//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT


#ifndef VAL_VERSION
#define VAL_VERSION 0x0003008B

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <stdalign.h>
#include <inttypes.h>
#include <stddef.h>
#include <limits.h>

// ## REFERENCES
//
// "Crafting Interpreters" by Robert Nystrom 
// https://craftinginterpreters.com/optimization.html
//
// "NaN boxing or how to make the world dynamic"
// https://piotrduperas.com/posts/nan-boxing
//
// Nanobox implementation by Viktor Söderqvist 
// https://github.com/zuiderkwast/nanbox/blob/master/nanbox.h
//
// Intel® 64 and IA-32 Architectures Software Developer’s Manual vol 1 §4.2.2 table 4-3
// https://cdrdv2.intel.com/v1/dl/getContent/671436
//

// NaNs have all the 11 bits of the exponent set to 1.
// We use Quiet NaN's so thaat no trap or exception will be triggered: IEEE 754-2019 § 6.2.1.
// The type is identified by bit 63 and bits 48-50:
//
//       exponent   ╭─── Quiet NaN
//    ╭──────┴────╮ ▼
//   x111 1111 1111 1xxx FF FF FF FF FF FF
//   │╰──────┬──────╯╰┬╯ ╰───────┬───────╯
// type      │      type         │ 
//           │                   │ 
//    12 bits set to 1   Payload (48 bits)
//          7FF8
//
//  The value "Quiet NaN Indefinite" `nan`, which is the value returend by invalid operations like
//  0.0/0.0, is usually 0x7FF8000000000000 (and 0xFFF8000000000000 for `-nan`).
//  This library never uses or generates these value to avoid conflicts. This is not a guarantee as
//  the IEEE standard only requires that a NaN is produced, without specifying which one.
//  However, the assumption should work reasonably well for most modern computers.
// 
//  The encoding scheme is as follows:
//           |                     |                     | Ptr |                                |
//           |           7F_       |          FF_        | Tag |     Note                       |
//           |---------------------|---------------------|-----|--------------------------------|
//  _F8 1000 |         NAN         |        -NAN         | N/A | Reserved for FPU NAN           |
//  _F9 1001 |      constants      |    Symbolic const   | N/A |                                |
//  _FA 1010 |        void *       |        char *       | NO  | Predefined pointer types       |
//  _FB 1011 |        FILE *       |     valptr_buf_t    | YES | Predefined pointer types       |
//  _FC 1100 |      valptr_7_t     |      valptr_6_t     | YES | Library/User defined pointers  |
//  _FD 1101 |      valptr_5_t     |      valptr_4_t     | YES | Library/User defined pointers  |
//  _FE 1110 |      valptr_3_t     |      valptr_2_t     | YES | User defined pointer types     |
//  _FF 1111 |      valptr_1_t     |      valptr_0_t     | YES | User defined pointer types     |
//   
//        4       4         3         2         1
//        7       0         2         4         6         8         0
//        xxxx xYxx xxxY xxxx xYxx xxxY xxxx xYxx xxxY xxxx xYxx xxxY 
//      
//        0000 0011 1111 0000
//
//   7FF9 93F0 Boolean          1001 0011 1111 0000
//   7FF9 B3F0 Nil              1011 0011 1111 0000  
//   7FF9 C3F0 User defined     1100 0011 1111 0000
//   FFF9 xxxx Symbolic

//                                      |   |   |   |   |
#define VAL_NAN_MASK       ((uint64_t)0x7FF0000000000000)
//                                      |   |   |   |   |
#define VAL_DBLNAN_MASK    ((uint64_t)0x7FFF000000000000)
#define VAL_DBLNAN_NEG     ((uint64_t)0xFFF8000000000000)
#define VAL_DBLNAN_POS     ((uint64_t)0x7FF8000000000000)
//                                      |   |   |   |   |
#define VAL_TYPE_MASK      ((uint64_t)0xFFFF000000000000)
#define VAL_F7_TYPE_MASK   ((uint64_t)0x7FFF000000000000)
#define VAL_PAYLOAD_MASK   ((uint64_t)0x0000FFFFFFFFFFFF)
#define VAL_32BIT_MASK     ((uint64_t)0x00000000FFFFFFFF)
//                                      |   |   |   |   |
#define VALPTR_VOID        ((uint64_t)0x7FFA000000000000)
#define VALPTR_CHAR        ((uint64_t)0xFFFA000000000000)
#define VALPTR_FILE        ((uint64_t)0x7FFB000000000000)
#define VALPTR_BUF         ((uint64_t)0xFFFB000000000000)
#define VALPTR_7           ((uint64_t)0x7FFC000000000000)
#define VALPTR_6           ((uint64_t)0xFFFC000000000000)
#define VALPTR_5           ((uint64_t)0x7FFD000000000000)
#define VALPTR_4           ((uint64_t)0xFFFD000000000000)
#define VALPTR_3           ((uint64_t)0x7FFE000000000000)
#define VALPTR_2           ((uint64_t)0xFFFE000000000000)
#define VALPTR_1           ((uint64_t)0x7FFF000000000000)
#define VALPTR_0           ((uint64_t)0xFFFF000000000000)
//                                      |   |   |   |   |
#define VAL_CONSTTYPE_MASK ((uint64_t)0xFFFFFFFF00000000)
#define VAL_CONST_ANY      ((uint64_t)0x7FF9000000000000)
//                                      |   |   |   |   |
#define VAL_CONST_0        ((uint64_t)0x7FF9C3F000000000)
#define VAL_SYM_0          ((uint64_t)0xFFF9000000000000)
#define VAL_SYM_NULL       ((uint64_t)0xFFF9FFFFFFFFFFFF)
//                                      |   |   |   |   |
#define VAL_FALSE          ((uint64_t)0x7FF993F000000000)
#define VAL_NIL            ((uint64_t)0x7FF9B3F000000000)
#define VAL_VALCONST_MASK  ((uint64_t)0xFFFFDFF000000000)
#define VAL_VALCONST       ((uint64_t)0x7FF993F000000000)
//                                      |   |   |   |   |

// =========

typedef struct {uint64_t v;} val_t;

static_assert(sizeof(val_t) == sizeof(uint64_t), "Wrong size for val_t");
static_assert(sizeof(val_t) == 8, "Wrong size for uint64_t");

// Some auxiliary macros for optional parameters

#define VAL_cnt(x1,x2,x3,x4,xN, ...) xN
#define VAL_n(...)       VAL_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define VAL_join(x ,y)   x ## y
#define VAL_cat(x, y)    VAL_join(x, y)
#define VAL_vrg(f, ...)  VAL_cat(f, VAL_n(__VA_ARGS__))(__VA_ARGS__)

// ==== Numbers
// All numbers are stored as a double floating point.

// All non-NaN numbers are doubles except VAL_DBLNAN_NEG and VAL_DBLNAN_POS
#define valisnumber(x) val_isnumber(val(x)) 
static inline int val_isnumber(val_t v) {
  return ((v.v & VAL_NAN_MASK) != VAL_NAN_MASK)
      || ((v.v & VAL_DBLNAN_MASK) == VAL_DBLNAN_POS); // The result of expressions like 0.0/0.0
}

// By effect of the IEEE 754 standard, only integers up to 52 bits are representable.
// We'll check directly on the bit represantation of doubles. See IEEE754.md in the docs direcotory
#define valisint(x) val_isint(val(x))
static inline int val_isint(val_t v) {
  int      exp_bits  = (int)((v.v >> 52) & 0x7FF);          // Extract the exponent (11 bits)
  uint64_t frac_bits = v.v & ((((uint64_t)1) << 52) - 1);   // Extract the fraction field (52 bits)

  if (exp_bits == 0)     return (frac_bits == 0);           // If exponent is 0 and the fractional part is 0 then it's an integer.
  if (exp_bits == 0x7FF) return 0;                          // If exponent is all 1, then it's a NaN, hence not an integer

  int e = exp_bits - 1023;                                  // Compute unbiased exponent e = exp_bits − bias (bias = 1023)
  if (e < 0)   return 0;                                    //  If exponent < 0 cannot be an integer
  if (e >= 52) return 1;                                    // If exponet ≥ 52, all bits represent values ≥ 2^0, so no fractional part remains

  uint64_t frac_mask = (((uint64_t)1) << (52 - e)) - 1;     // Get the actual fractional part
  return ( (frac_bits & frac_mask) == 0 );                  // Iff it is 0, it's an integer.
}

// ==== POINTERS

// Buffers are structures whose first field is a char *
typedef struct valptr_buf_s *valptr_buf_t; 

// These can be user defined.

#ifndef valptr_7_t
typedef struct valptr_7_s *valptr_7_t; 
#endif
#ifndef valptr_6_t
typedef struct valptr_6_s *valptr_6_t; 
#endif
#ifndef valptr_5_t
typedef struct valptr_5_s *valptr_5_t; 
#endif
#ifndef valptr_4_t
typedef struct valptr_4_s *valptr_4_t; 
#endif
#ifndef valptr_3_t
typedef struct valptr_3_s *valptr_3_t; 
#endif
#ifndef valptr_2_t
typedef struct valptr_2_s *valptr_2_t; 
#endif
#ifndef valptr_1_t
typedef struct valptr_1_s *valptr_1_t; 
#endif
#ifndef valptr_0_t
typedef struct valptr_0_s *valptr_0_t; 
#endif

#define val_is_any_ptr(v) (((v).v & VAL_F7_TYPE_MASK) >= VALPTR_VOID)

#define valisptr(...) VAL_vrg(val_isptr_v,__VA_ARGS__)

#define val_isptr_v1(v)   val_isptr_1(val(v))
#define val_isptr_v2(v,t) val_isptr_2(val(v),t)

static inline int val_isptr_1(val_t v)  {
  return val_is_any_ptr(v);
}

static inline int val_isptr_2(val_t v, uint64_t ptr_type)  {
  return (val_is_any_ptr(v) && (v.v & VAL_TYPE_MASK) == ptr_type); 
}

#define valisvoidptr(x)     ((val(x).v & VAL_TYPE_MASK) == VALPTR_VOID)
#define valischarptr(x)     ((val(x).v & VAL_TYPE_MASK) == VALPTR_CHAR)
#define valisfileptr(x)     ((val(x).v & VAL_TYPE_MASK) == VALPTR_FILE)
#define valisbufptr(x)      ((val(x).v & VAL_TYPE_MASK) == VALPTR_BUF)

// The val_t corresponding of the C pointer NULL
static const val_t valnullptr = {VALPTR_VOID};

#define valptrtype(v) val_ptrtype(val(v))
static inline uint64_t val_ptrtype(val_t v) {
  return  val_is_any_ptr(v) ? (v.v & VAL_TYPE_MASK) : 0;
}

#define valisnullptr(x)  (valtoptr(x) == NULL)

// ==== Pointer tagging
// Except for `void *` and `char *`, any pointer can be *tagged* with 
// a number between 0 and 7 (0 is the default tag for pointers).
// This can be used, for example, to mark a pointer as "visited" or
// "to be disposed" without having to resort to additional memory.


// Define VALNOTAGPTR if pointers are not properly aligned.
#ifdef VALNOTAGPTR
  #define VAL_TAG_MASK  ((uint64_t)0)
#else
  #ifdef _MSC_VER
    // Make up for lack of `max_align_t` in Microsoft cl
    #define val_align_t double
  #else 
    #define val_align_t max_align_t
  #endif

  // We expect room for VAL_MIN_ALIGN values
  #define VAL_MIN_ALIGN 8
  static_assert(alignof(val_align_t) >= VAL_MIN_ALIGN, "Alignment requirements not met");
  #define VAL_TAG_MASK  ((uint64_t)(VAL_MIN_ALIGN-1))

#endif

static inline int val_check_taggable_ptr(val_t v) {
  // Not a pointer
  if (!val_is_any_ptr(v)) return -1; 

  // It's either a char or void pointer can't be tagged
  if ((v.v & VAL_TYPE_MASK) <= VALPTR_CHAR)  return 0;

  // Any other pointer is taggable
  return 1;
}

#define valtagptr_vrg(...) VAL_vrg(val_tagptr_,__VA_ARGS__)
#define valtagptr(...) valtagptr_vrg(__VA_ARGS__)
static inline val_t val_tagptr_2(val_t v, int tag) {
  if (val_check_taggable_ptr(v) > 0) 
    v.v = (v.v & ~VAL_TAG_MASK) | (tag & VAL_TAG_MASK);
  return v;
}

static inline int val_tagptr_1(val_t v) {
  if (val_check_taggable_ptr(v) <= 0) return 0;
  return (v.v & VAL_TAG_MASK);
}


// This is used for convenience
static char *val_emptystr = "\0\0\0"; // FOUR nul bytes

// Convert a C value to a val_t
// Note that these functions return a structure (NOT a pointer).VAL_FALSE

// We only store floating point numbers as doubles since it is guaranteed by the
// C Standard (C11/C17 § 6.3.1.5 [Real floating types]), that any float (4-byte
// floating point real number) can be exactly represented as a double (8 bytes number).

static inline val_t val_fromdouble(double v)    {val_t ret; memcpy(&ret,&v,sizeof(val_t)); return ret;}
static inline val_t val_fromfloat(float f)      {return val_fromdouble((double)f);}
static inline val_t val_fromint(int64_t v)      {return val_fromdouble((double)v);}
static inline val_t val_fromuint(uint64_t v)    {return val_fromdouble((double)v);}

// POINTERS

static inline val_t val_frompvoidtr(void *v)    {val_t ret; ret.v = VALPTR_VOID | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromcharptr(void *v)    {val_t ret; ret.v = VALPTR_CHAR | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromfileptr(void *v)    {val_t ret; ret.v = VALPTR_FILE | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_frombufptr(void *v)     {val_t ret; ret.v = VALPTR_BUF  | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_7(void *v)      {val_t ret; ret.v = VALPTR_7    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_6(void *v)      {val_t ret; ret.v = VALPTR_6    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_5(void *v)      {val_t ret; ret.v = VALPTR_5    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_4(void *v)      {val_t ret; ret.v = VALPTR_4    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_3(void *v)      {val_t ret; ret.v = VALPTR_3    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_2(void *v)      {val_t ret; ret.v = VALPTR_2    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_1(void *v)      {val_t ret; ret.v = VALPTR_1    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_0(void *v)      {val_t ret; ret.v = VALPTR_0    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}

// BOOLEAN
static inline val_t val_frombool(_Bool v)       {val_t ret; ret.v = VAL_FALSE | ((uint64_t)(!!(v))); return ret;}

// VALUES (identity)
static inline val_t val_fromval(val_t v)        {return v;}

#define val_from(x) _Generic((x), int: val_fromint,        \
                                 char: val_fromint,        \
                          signed char: val_fromint,        \
                                short: val_fromint,        \
                                 long: val_fromint,        \
                            long long: val_fromint,        \
                         unsigned int: val_fromuint,       \
                        unsigned char: val_fromuint,       \
                       unsigned short: val_fromuint,       \
                        unsigned long: val_fromuint,       \
                   unsigned long long: val_fromuint,       \
                                _Bool: val_frombool,       \
                               double: val_fromdouble,     \
                                float: val_fromfloat,      \
                                char*: val_fromcharptr,    \
                         signed char*: val_fromcharptr,    \
                       unsigned char*: val_fromcharptr,    \
                               FILE *: val_fromfileptr,    \
                         valptr_buf_t: val_frombufptr,     \
                           valptr_7_t: val_fromptr_7,      \
                           valptr_6_t: val_fromptr_6,      \
                           valptr_5_t: val_fromptr_5,      \
                           valptr_4_t: val_fromptr_4,      \
                           valptr_3_t: val_fromptr_3,      \
                           valptr_2_t: val_fromptr_2,      \
                           valptr_1_t: val_fromptr_1,      \
                           valptr_0_t: val_fromptr_0,      \
                               void *: val_frompvoidtr,    \
                                val_t: val_fromval  ,      \
                              default: val_frompvoidtr     \
                    ) (x)

// In val(x), the C11 standard (§6.5.1.1) guarantess that x is only evaluated once.
// The first occurence will not be evaluated as only its type is considered
// and then only one of the branches are included in the translation unit.
// This improves performance as val_t values pass through val() with no computation.
#define val(x) _Generic((x), val_t: x, default: val_from(x))


// ==== CONSTANTS


// Booleans

static const val_t valfalse = {VAL_FALSE};
static const val_t valtrue  = {VAL_FALSE | 1};

#define valisbool(x) ((val(x).v & VAL_CONSTTYPE_MASK) == VAL_FALSE)

// A nil value to signal a void value
static const val_t    valnil = {VAL_NIL};
#define valisnil(x)   ((val(x).v == VAL_NIL))


// User defined constants
#define valconst(x) _Generic((x), val_t: val_valconst, char *: valsymconst, default: valnumconst)(x)
static inline val_t valnumconst(uint32_t x)  { return ((val_t){ VAL_CONST_0 | x }); }

// This checks if val is any numeric or symbolic const, including valnil, valtrue and valfalse
#define val_is_any_const(x) ((v.v & VAL_F7_TYPE_MASK) == VAL_CONST_ANY)

// This checks if val is a numeric const or any of valnil, valtrue and valfalse
#define val_is_any_NV_const(x) ((v.v & VAL_TYPE_MASK) == VAL_CONST_ANY)

// Booleans and valnil
static inline val_t val_valconst(val_t v) { return val_is_any_const(v) ? v : valnil;}

#define valisconst_vrg(...) VAL_vrg(val_isconst_,__VA_ARGS__)
#define valisconst(...) valisconst_vrg(__VA_ARGS__)

static inline int val_isconst_1(val_t v) {return val_is_any_const(v); }

#define val_isconst_2(v,c) _Generic((c), \
                                   char *: val_issymconst_2, \
                                  default: val_isnumconst_2 \
                           ) (valconst(v),c)

// Num

#define valisnumconst(...) VAL_vrg(val_isnumconst_,__VA_ARGS__)

static inline int val_isnumconst_1(val_t v) { 
  return ((v.v & VAL_CONSTTYPE_MASK) == VAL_CONST_0);
}

static inline int val_isnumconst_2(val_t v, uint32_t x) { 
  return val_isnumconst_1(v) && ((v.v & VAL_32BIT_MASK) == x);
}

static inline val_t valsymconst(char * restrict sym_str) ;

#define valissymconst(...)  VAL_vrg(val_issymconst_,__VA_ARGS__)

static inline int val_issymconst_1(val_t v) {
  return ((v.v & VAL_TYPE_MASK) == VAL_SYM_0);
}

static inline int val_issymconst_2(val_t v, char * s) {
  return (val_issymconst_1(v) && (valsymconst(s).v == v.v));
}

#define VAL_STR_MAX_LEN 32
typedef struct { char str[VAL_STR_MAX_LEN]; } valstr_t;

// Syms (8 chars strings)

  //             1         2         3     3   4         5         6  6
  //   0         0         0         0     67  0         0         0  3
  //  "!#$*+-./0123456789:<=>?@ABCDEFXYZ[]_abcdefghijklmnopqrstuvwxyz~"
  // Note that character `\0` is mapped to 63 (0x3F)

static inline val_t valsymconst(char * restrict sym_str) {
  val_t sym_val = {0};
  uint64_t sym = 0;
  static const uint8_t ascii_to_sym[128] = {
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x00, 0x3F, 0x01, 0x02, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x03, 0x04, 0x3F, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x3F, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x1E, 0x1F, 0x20, 0x21, 0x3F, 0x22, 0x3F, 0x23,
    0x3F, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3F, 0x3F, 0x3F, 0x3E, 0x3F
  };
  uint64_t c = 0;

  if (sym_str == NULL || *sym_str == '\0') {
    sym_val.v = VAL_SYM_NULL;
    return sym_val;
  }

  int shift = 0;
                   c = ascii_to_sym[*sym_str++ & 0x7F];  sym |= (c << shift);  shift =  6;
  if (c != 0x3F) { c = ascii_to_sym[*sym_str++ & 0x7F];  sym |= (c << shift);  shift = 12;
  if (c != 0x3F) { c = ascii_to_sym[*sym_str++ & 0x7F];  sym |= (c << shift);  shift = 18;
  if (c != 0x3F) { c = ascii_to_sym[*sym_str++ & 0x7F];  sym |= (c << shift);  shift = 24;
  if (c != 0x3F) { c = ascii_to_sym[*sym_str++ & 0x7F];  sym |= (c << shift);  shift = 30;
  if (c != 0x3F) { c = ascii_to_sym[*sym_str++ & 0x7F];  sym |= (c << shift);  shift = 36;
  if (c != 0x3F) { c = ascii_to_sym[*sym_str++ & 0x7F];  sym |= (c << shift);  shift = 42;
  if (c != 0x3F) { c = ascii_to_sym[*sym_str   & 0x7F];  sym |= (c << shift);  shift = 48;
  /* UNROLLED */ }}}}}}}

  sym |= (VAL_SYM_NULL << shift); // Fill with FF
  sym_val.v = VAL_SYM_0 | (sym & VAL_PAYLOAD_MASK);
  return sym_val;
}

static inline valstr_t valsymtostr(val_t v) {
  valstr_t sym_str_s;
  uint64_t sym;
  int c;
  char *sym_str = sym_str_s.str;

  if (((v.v & VAL_TYPE_MASK) == VAL_SYM_0)) {
                                    //           1         2         3     33  4         5         6  6
                                    // 0         0         0         0     67  0         0         0  3
    static const char *sym_to_ascii = "!#$*+-./0123456789:<=>?@ABCDEFXYZ[]_abcdefghijklmnopqrstuvwxyz~";
    
                                     // Ensure there's a terminator
    sym = (v.v & VAL_PAYLOAD_MASK) | (uint64_t)0x003F000000000000 ;
    
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    if ((c = sym_to_ascii[sym & 0x3F])) { *sym_str++ = c;  sym >>= 6;
    /* UNROLLED */                      }}}}}}}}
  }
  *sym_str = '\0';
  return sym_str_s;
}

// ====== Retrieve values from a val_t variable
#define valtodouble(v) val_todouble(val(v))
static inline double val_todouble(val_t v) {
  double d = 0.0; 
  if (val_isnumber(v)) memcpy(&d,&v,sizeof(double));
  else errno = EINVAL;
  return d;
}

#define valtoint(v)  val_toint(val(v))
static inline int64_t val_toint(val_t v) {
  if (val_isnumber(v)) {
    double d;
    memcpy(&d,&v,sizeof(double));
    return (int64_t)d;
  }
  if (val_is_any_NV_const(v)) { // Only the lower 32bits
    return (v.v & VAL_32BIT_MASK);  
  }
  // Pointers and symbolic const are returned on 48 bits.
  return (v.v & VAL_PAYLOAD_MASK);
}

#define valtounsignedint(v)  ((uint64_t)valtoint(v))

#define valtoint32(x)  ((int32_t)valtoint(x))
#define valtouint32(x) ((uint32_t)valtoint(x))

#define valtobool(v) val_tobool(val(v))
static inline  _Bool val_tobool(val_t v)  {
  if (valisnumber(v)) return (valtodouble(v) != 0.0);
  else return ((v.v & VAL_32BIT_MASK) != 0);
}

#define valtoptr(v) val_toptr(val(v))
static inline   void *val_toptr(val_t v) {
  uint64_t mask = VAL_PAYLOAD_MASK;

  switch (val_check_taggable_ptr(v)) {
    case -1: errno = EINVAL; return NULL; // Not a pointer
    case  1: mask &= ~VAL_TAG_MASK; // taggable ptr. Need to eliminate the tag!
  }
  void * ret = (void *)((uintptr_t)(v.v & mask));
  return ret;
}

#define valtostr(...) VAL_vrg(val_tostr_,__VA_ARGS__)
#define val_tostr_1(v) val_tostr_2(v,NULL)
static inline valstr_t val_tostr_2(val_t v, char *fmt) {
  valstr_t ret;

  if (fmt && (*fmt == '\0'))
         snprintf(ret.str, VAL_STR_MAX_LEN, "%016" PRIX64, v.v);
  else if (val_issymconst_1(v)) 
         ret = valsymtostr(v);
  else if (val_isnumconst_1(v))
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "<%" PRIX32 ">", (uint32_t)valtoint(v));
  else if (val_is_any_ptr(v))
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "%p", valtoptr(v));
  else if (valisbool(v))
         strcpy(ret.str,(v.v & 1)? "true" : "false");
  else if (valisnil(v))
         strcpy(ret.str,"nil");
  else if (valisint(v))
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "%" PRId64, valtoint(v));
  else if (valisnumber(v)) 
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "%f" , valtodouble(v));
  else
         snprintf(ret.str, VAL_STR_MAX_LEN, "%016" PRIX64, v.v);

  return ret;
}

// This checks for val_t values IDENTITY
#define valeq(x,y) (val(x).v == val(y).v)

static inline char *val_get_charptr(val_t v, valstr_t *str) {
  char *ret = val_emptystr;
  if (valischarptr(v)) ret = valtoptr(v);
  else if (valisbufptr(v)) {
    char **v_ptr = valtoptr(v);
    ret = v_ptr ? *v_ptr : NULL;
  }
  else if (val_issymconst_1(v)) {
    *str = valsymtostr(v);
    ret = str->str;
  }
  return ret;
}

// This compares two val_t values. Like the hash function below, it is provide just for convenience 
// since your criteria for comparison and hashing might be different.
#define valcmp(a,b) val_cmp(val(a),val(b))
static inline int val_cmp(val_t a, val_t b) {
  char *sa = val_emptystr;
  char *sb = val_emptystr;
  valstr_t sym_str_a;
  valstr_t sym_str_b;

  sa = val_get_charptr(a,&sym_str_a);
  
  if (sa != val_emptystr) {
    sb = val_get_charptr(b,&sym_str_b);

    if (sb != val_emptystr) {
      if (sa == NULL) sa = val_emptystr; // / To avoid calling strcmp
      if (sb == NULL) sb = val_emptystr; // \ with NULL arguments

      return strcmp(sa,sb);
    }
  }

  if (valisnumber(a) && valisnumber(b)) {
    double da = valtodouble(a);
    double db = valtodouble(b);
    return (da > db)? 1 : (da < db) ? -1 : 0 ;
  }
  
  if (valisnumber(a)) return -1; // / Numbers are lower than
  if (valisnumber(b)) return  1; // \ any other type

  return (a.v > b.v)? 1 : (a.v < b.v) ? -1 : 0 ;
}

#define valhash(a) val_hash(val(a))
static inline uint32_t val_hash(val_t v) {
  uint32_t hash = (uint32_t)0X811C9DC5; // FNV1a INIT

  char *s = val_emptystr;
  valstr_t sym_str;

  s = val_get_charptr(v,&sym_str);

  if (s != val_emptystr && s != NULL) {
    // FNV1a abridged from http://www.isthe.com/chongo/tech/comp/fnv/index.html
    while (*s) {
      hash ^= (uint32_t)(*s++);
      hash *= (uint32_t)0x01000193; // FNV1a PRIME
    }
  }
  else {
    uint64_t h = v.v;
    /* 64→64-bit MurmurHash3 “fmix” finalizer */
    h ^= h >> 33;
    h *= (uint64_t)0XFF51AFD7ED558CCD;
    h ^= h >> 33;
    h *= (uint64_t)0XC4CEB9FE1A85EC53;
    h ^= h >> 33;

    hash = (uint32_t)(h >> 32);
  }
  return hash;
}

// This is needed to avoid warnings about unused static variables.
static inline uint64_t val_usestatic()
{
  return (valnil.v | valtrue.v | valfalse.v | valnullptr.v | (uintptr_t)val_emptystr);
}
#endif
