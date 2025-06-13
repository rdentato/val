//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT


#ifndef VAL_VERSION
#define VAL_VERSION 0x0003008B

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
//  0.0/0.0, is usually 0x7FF8000000000000 (and 0x7FF8000000000000 for `-nan`).
//  This library never uses or generates these value to avoid conflicts. This is not a guarantee as
//  the IEEE standard only requires that a NaN is produced, without specifying which one.
//  However, the assumption should work reasonably well for most modern computers.

#define VAL_NAN_MASK     ((uint64_t)0x7FF8000000000000)

#define VAL_DBLNAN_MASK  ((uint64_t)0x7FFFFFFFFFFFFFFF)
#define VAL_DBLNAN_NEG   ((uint64_t)0xFFF8000000000000)
#define VAL_DBLNAN_POS   ((uint64_t)0x7FF8000000000000)

typedef struct {uint64_t v;} val_t;

static_assert(sizeof(val_t) == sizeof(uint64_t), "Wrong size for val_t");
static_assert(sizeof(val_t) == 8, "Wrong size for uint64_t");

typedef struct {int32_t v;} val_dummy_t;

static_assert(sizeof(val_dummy_t) == sizeof(int32_t), "Wrong size for val_dummy_t");
static_assert(sizeof(val_dummy_t) == 4, "Wrong size for int32_t");

// Some auxiliary macros for optional parameters
#define val_x(...)     __VA_ARGS__
#define val_0(x,...)      x
#define val_1(y,x,...)    x
#define val_2(z,y,x,...)  x

// ==== Numbers
// All numbers are stored as a double floating point.
// A value v is NaN-boxed if  (v & VAL_NAN_MASK) == VAL_NAN_MASK
// Otherwise is a double (i.e. is not a NaN).

#define valisdouble(x) val_isdouble(val(x)) 
static inline int val_isdouble(val_t v) {
  return ((v.v & VAL_NAN_MASK) != VAL_NAN_MASK)
      || ((v.v & VAL_DBLNAN_MASK) == VAL_DBLNAN_POS);
}

#define VAL_TYPE_MASK     ((uint64_t)0xFFFF000000000000)
#define VAL_PAYLOAD_MASK  ((uint64_t)0x0000FFFFFFFFFFFF)

// By effect of the IEEE 754 standard, only integers up to 52 bits are representable.
// We'll check directly on the bit represantation of doubles. See IEEE754.md in the docs direcotory
#define valisint(x) val_isint(val(x))
static inline int val_isint(val_t v) {
 
  int exp_bits = (int)((v.v >> 52) & 0x7FF); // Extract the exponent (11 bits)
  uint64_t frac_bits = v.v & ((((uint64_t)1) << 52) - 1); // Extract the fraction field (52 bits)

  if (exp_bits == 0) return (frac_bits == 0); // If exponent is 0 and the fractional part is 0 then it's an integer.
  if (exp_bits == 0x7FF) return 0; // If exponent is all 1, then it's a NaN, hence not an integer

  int e = exp_bits - 1023; // Compute unbiased exponent e = exp_bits − bias (bias = 1023)
  if (e < 0) return 0; //  If exponent < 0 cannot be an integer
  if (e >= 52) return 1; // If exponet ≥ 52, all bits represent values ≥ 2^0, so no fractional part remains

  uint64_t frac_mask = (((uint64_t)1) << (52 - e)) - 1; // Get the actual fractional part
  return ( (frac_bits & frac_mask) == 0 );              // Iff it is 0, it's an integer.
}

// ==== CONSTANTS
#define VAL_CONST_MASK    ((uint64_t)0xFFFFFFFF00000000)
#define VAL_CONST_0       ((uint64_t)0x7FF91AA100000000)
#define VAL_FALSE         ((uint64_t)0x7FF9FFFF00000000)
#define VAL_32BIT_MASK    ((uint64_t)0x00000000FFFFFFFF)

// Booleans
static const val_t valfalse = {VAL_FALSE};
static const val_t valtrue  = {VAL_FALSE | 1};

#define valisbool(x) ((val(x).v & VAL_CONST_MASK) == VAL_FALSE)

// A nil value to signal a void value
#define VAL_NIL       ((uint64_t)0x7FF900FF00000000)
static const val_t    valnil = {VAL_NIL};

#define valisnil(x)   ((val(x).v == VAL_NIL))

// Labels (8 chars strings of upper/lower case, digits and '_')
#define VAL_LABEL_NULL     ((uint64_t)0x7FFAFFFFFFFFFFFF)
#define VAL_LABEL_0        ((uint64_t)0x7FFA000000000000)
#define valislabelnull(x)  (val(x) == vallabelnull)

static const val_t vallabelnull = {VAL_LABEL_NULL};

  //             1         2         3     3   4         5         6
  //   0         0         0         0     67  0         0         0
  //  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
  // Note that character `\0` is mapped to 63

static inline val_t vallabel(char * restrict lbl_str) {
  val_t lbl_val = {VAL_LABEL_NULL};
  uint64_t lbl = 0;
  static const uint8_t ascii_to_lbl[128] = {
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x3F, 0x3F, 0x3F, 0x3F, 0x24,
    0x3F, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F
  };
  uint64_t c = 0;

  if (lbl_str == NULL || *lbl_str == '\0') {
    lbl_val.v = VAL_LABEL_0 | (uint64_t)0x3F;
    return lbl_val;
  }

  int shift = 0;
                   c = ascii_to_lbl[*lbl_str++ & 0x7F];  lbl |= (c << shift);  shift =  6;
  if (c != 0x3F) { c = ascii_to_lbl[*lbl_str++ & 0x7F];  lbl |= (c << shift);  shift = 12;
  if (c != 0x3F) { c = ascii_to_lbl[*lbl_str++ & 0x7F];  lbl |= (c << shift);  shift = 18;
  if (c != 0x3F) { c = ascii_to_lbl[*lbl_str++ & 0x7F];  lbl |= (c << shift);  shift = 24;
  if (c != 0x3F) { c = ascii_to_lbl[*lbl_str++ & 0x7F];  lbl |= (c << shift);  shift = 30;
  if (c != 0x3F) { c = ascii_to_lbl[*lbl_str++ & 0x7F];  lbl |= (c << shift);  shift = 36;
  if (c != 0x3F) { c = ascii_to_lbl[*lbl_str++ & 0x7F];  lbl |= (c << shift);  shift = 42;
  if (c != 0x3F) { c = ascii_to_lbl[*lbl_str   & 0x7F];  lbl |= (c << shift);  shift = 48;
  }}}}}}} // UNROLLED LOOP


  lbl |= ((uint64_t)0x3F << shift);
  lbl_val.v = VAL_LABEL_0 | (lbl & VAL_PAYLOAD_MASK);
  return lbl_val;
}

#define VAL_STR_MAX_LEN 32
typedef struct { char str[VAL_STR_MAX_LEN]; } valstr_t;

#define vallabeltostr(v) val_label_to_str(val(v))
static inline valstr_t val_label_to_str(val_t v) {
  valstr_t lbl_str_s;
  uint64_t lbl;
  int c;
  char *lbl_str = lbl_str_s.str;

  if (((v.v & VAL_TYPE_MASK) == VAL_LABEL_0)) {
                              //           1         2         3     3   4         5         6
                              // 0         0         0         0     67  0         0         0
    static const char *lbl_to_ascii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    
    lbl = (v.v & VAL_PAYLOAD_MASK) | (uint64_t)0x003F000000000000 ;
    
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    if ((c = lbl_to_ascii[lbl & 0x3F])) { *lbl_str++ = c;  lbl >>= 6;
    }}}}}}}} // UNROLLED LOOP
  }
  *lbl_str = '\0';
  return lbl_str_s;
}


// ==== POINTERS

#define VALPTR_VOID   ((uint64_t)0xFFF9000000000000)
#define VALPTR_CHAR   ((uint64_t)0xFFFA000000000000)

// Buffers are structures whose first field is a char *
#define VALPTR_BUF    ((uint64_t)0xFFFB000000000000)
typedef struct valptr_buf_s *valptr_buf_t; 

// These can be user defined.
#define VALPTR_4      ((uint64_t)0xFFFC000000000000)
#define VALPTR_3      ((uint64_t)0xFFFD000000000000)
#define VALPTR_2      ((uint64_t)0xFFFE000000000000)
#define VALPTR_1      ((uint64_t)0xFFFF000000000000)

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

#define val_is_any_ptr(v) (((v).v & VAL_TYPE_MASK) >= VALPTR_VOID)

#define valisptr(p,...) val_isptr(val(p), __VA_ARGS__ +0)
static inline int val_isptr(val_t v, uint64_t ptr_type)  {
    if (!ptr_type) return val_is_any_ptr(v);
  else             return ((v.v & VAL_TYPE_MASK) == ptr_type); 
}

#define valisvoidptr(x)     ((val(x).v & VAL_TYPE_MASK) == VALPTR_VOID)
#define valischarptr(x)     ((val(x).v & VAL_TYPE_MASK) == VALPTR_CHAR)
#define valisbufptr(x)      ((val(x).v & VAL_TYPE_MASK) == VALPTR_BUF)

// The val_t corresponding of the C pointer NULL
static const val_t valnullptr = {VALPTR_VOID};

#define valisnullptr(x)  (valtoptr(x) == NULL)

// This is used for convenience
static char *val_emptystr = "\0\0\0"; // FOUR nul bytes

// Convert a C value to a val_t
// Note that these functions return a structure (NOT a pointer).VAL_FALSE

// We only store floating point numbers as doubles since it is guaranteed by the
// C Standard (C11/C17 § 6.3.1.5 [Real floating types]), that any float (4-byte
// floating point real number) can be exactly represented as a double (8 bytes number).

static inline val_t val_fromdouble(double v)    {val_t ret = valnil; memcpy(&ret,&v,sizeof(val_t)); return ret;}
static inline val_t val_fromfloat(float f)      {return val_fromdouble((double)f);}
static inline val_t val_fromint(int64_t v)      {return val_fromdouble((double)v);}
static inline val_t val_fromuint(uint64_t v)    {return val_fromdouble((double)v);}

// POINTERS

static inline val_t val_frompvoidtr(void *v)    {val_t ret; ret.v = VALPTR_VOID | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromcharptr(void *v)    {val_t ret; ret.v = VALPTR_CHAR | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_frombufptr(void *v)     {val_t ret; ret.v = VALPTR_BUF  | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_4(void *v)      {val_t ret; ret.v = VALPTR_4    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_3(void *v)      {val_t ret; ret.v = VALPTR_3    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_2(void *v)      {val_t ret; ret.v = VALPTR_2    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_1(void *v)      {val_t ret; ret.v = VALPTR_1    | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}

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
                         valptr_buf_t: val_frombufptr,     \
                           valptr_4_t: val_fromptr_4,      \
                           valptr_3_t: val_fromptr_3,      \
                           valptr_2_t: val_fromptr_2,      \
                           valptr_1_t: val_fromptr_1,      \
                               void *: val_frompvoidtr,    \
                                val_t: val_fromval  ,      \
                              default: val_frompvoidtr     \
                    ) (x)

// In val(x), the C11 standard (§6.5.1.1) guarantess that x is only evaluated once.
// The first occurence will not be evaluated as only its type is considered
// and then only one of the branches are included in the translation unit.
// This improves performance as val_t values pass through val() with no computation.
#define val(x) _Generic((x), val_t: x, default: val_from(x))

// ====== Retrieve values from a val_t variable
#define valtodouble(v) val_todouble(val(v))
static inline double val_todouble(val_t v) {
  double d = 0.0;
  if (valisdouble(v)) memcpy(&d,&v,sizeof(double)); 
  return d;
}

#define valtoint(v)  val_toint(val(v))
static inline int64_t val_toint(val_t v) {
  if (valisdouble(v))    return (int64_t)val_todouble(v);
  if (val_is_any_ptr(v)) return (v.v & VAL_PAYLOAD_MASK);
  return (v.v & VAL_32BIT_MASK);
}

#define valtounsignedint(v)  ((uint64_t)valtoint(v))

#define valtobool(v) val_tobool(val(v))
static inline  _Bool val_tobool(val_t v)  {
  if (valisdouble(v)) return (valtodouble(v) != 0.0);
  else return ((v.v & VAL_32BIT_MASK) != 0);
}

// User defined constants
#define valconst(x)   ((val_t){ VAL_CONST_0 | ((x) & VAL_32BIT_MASK)})

#define valisconst(...) val_isconst_(val_x(val_0(__VA_ARGS__)),\
                                     val_x(val_1(__VA_ARGS__,val_emptystr)))

#define val_isconst_(v,c) _Generic((c), \
                                 default: val_isconst(v),  \
                                     int: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                                    char: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                             signed char: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                                   short: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                                    long: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                               long long: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                            unsigned int: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                           unsigned char: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                          unsigned short: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                           unsigned long: valeq(v,valconst((int32_t)((uintptr_t)(c)))), \
                      unsigned long long: valeq(v,valconst((int32_t)((uintptr_t)(c))))  \
                          )

static inline int val_isconst(val_t x) { return ((val(x).v & VAL_CONST_MASK) == VAL_CONST_0);}

#define valislabel(...)  val_islabel(val(val_0(__VA_ARGS__)), val_1(__VA_ARGS__,NULL))
#define val_is_label(v) ((v.v & VAL_TYPE_MASK) == VAL_LABEL_0)
static inline int val_islabel(val_t v, char * s) {
  if (!val_is_label(v)) return 0;
  if (s != NULL)  return (vallabel(s).v == v.v);
  return 1;
}

// ==== Pointer tagging
// Except for `void *` and `char *`, any pointer can be *tagged* with 
// a number between 0 and 7 (0 is the default tag for pointers).
// This can be used, for example, to mark a pointer as "visited" or
// "to be disposed" without having to resort to additional memory.

// Checks that there are at least three zero bits in memory aligned pointers
#ifdef _MSC_VER
  // Make up for lack of `max_align_t` in Microsoft cl
  #define val_align_t double
#else 
  #define val_align_t max_align_t
#endif

#define VAL_MIN_ALIGN 8
static_assert(alignof(val_align_t) >= VAL_MIN_ALIGN, "Alignment requirements not met");

static inline int val_check_taggable_ptr(val_t v) {
  // Not a pointer
  if (!val_is_any_ptr(v)) return -1; 

  // It's either a char or void pointer can't be tagged
  if ((v.v & VAL_TYPE_MASK) <= VALPTR_CHAR)  return 0;

  // A taggable pointer
  return 1;
}

// We expect room for VAL_MIN_ALIGN values
#define VAL_TAG_MASK  ((uint64_t)(VAL_MIN_ALIGN-1))

#define valtoptr(v) val_toptr(val(v))
static inline   void *val_toptr(val_t v) {
  uint64_t mask = VAL_PAYLOAD_MASK;

  switch (val_check_taggable_ptr(v)) {
    case -1: return NULL; // Not a pointer
    case  1: mask &= ~VAL_TAG_MASK; // taggable ptr. Need to eliminate the tag!
  }
  void * ret = (void *)((uintptr_t)(v.v & mask));
  return ret;
}

#define valptrtype(v) val_ptrtype(val(v))
static inline uint64_t val_ptrtype(val_t v) {
  return  val_is_any_ptr(v) ? (v.v & VAL_TYPE_MASK) : 0;
}

#define valtagptr(...)  val_tagptr_(val_x(val_0(__VA_ARGS__)),\
                                    val_x(val_1(__VA_ARGS__,val_emptystr)))

#define val_tagptr_(v,t) _Generic((t), \
                             int: val_tagptr_set(val(v),(int)((uintptr_t)(t))), \
                         default: val_tagptr_get(val(v)) \
                         )

static inline val_t val_tagptr_set(val_t v, int tag) {
  if (val_check_taggable_ptr(v) > 0) 
    v.v = (v.v & ~VAL_TAG_MASK) | (tag & VAL_TAG_MASK);
  return v;
}

static inline int val_tagptr_get(val_t v) {
  if (val_check_taggable_ptr(v) <= 0) return 0;
  return (v.v & VAL_TAG_MASK);
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
  else if (val_is_label(v)) {
    *str = val_label_to_str(v);
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
  valstr_t lbl_str_a;
  valstr_t lbl_str_b;

  sa = val_get_charptr(a,&lbl_str_a);
  
  if (sa != val_emptystr) {
    sb = val_get_charptr(b,&lbl_str_b);

    if (sb != val_emptystr) {
      if (sa == NULL) sa = val_emptystr; // / To avoid calling strcmp
      if (sb == NULL) sb = val_emptystr; // \ with NULL arguments

      return strcmp(sa,sb);
    }
  }

  if (valisdouble(a) && valisdouble(b)) {
    double da = valtodouble(a);
    double db = valtodouble(b);
    return (da > db)? 1 : (da < db) ? -1 : 0 ;
  }
  
  if (valisdouble(a)) return -1; // / Numbers are lower than
  if (valisdouble(b)) return  1; // \ any other type

  return (a.v > b.v)? 1 : (a.v < b.v) ? -1 : 0 ;
}

#define valhash(a) val_hash(val(a))
static inline uint32_t val_hash(val_t v) {
  uint32_t hash = (uint32_t)0x811c9dc5; // FNV1a INIT

  char *s = val_emptystr;
  valstr_t lbl_str;

  s = val_get_charptr(v,&lbl_str);

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
    h *= (uint64_t)0xff51afd7ed558ccd;
    h ^= h >> 33;
    h *= (uint64_t)0xc4ceb9fe1a85ec53;
    h ^= h >> 33;

    hash = (uint32_t)(h >> 32);
  }
  return hash;
}

#define valtostr(...) val_tostr(val(val_0(__VA_ARGS__)),val_1(__VA_ARGS__,NULL))
static inline valstr_t val_tostr(val_t v, char *fmt) {
  valstr_t ret;

  if (fmt && (*fmt == '\0'))
         snprintf(ret.str, VAL_STR_MAX_LEN, "%016" PRIX64, v.v);
  else if (val_is_label(v)) 
         ret = val_label_to_str(v);
  else if (valisconst(v))
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "<%" PRIX64 ">", valtoint(v));
  else if (val_is_any_ptr(v))
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "%p", valtoptr(v));
  else if (valisbool(v))
         strcpy(ret.str,(v.v & 1)? "true" : "false");
  else if (valisnil(v))
         strcpy(ret.str,"nil");
  else if (valisint(v))
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "%" PRId64, valtoint(v));
  else if (valisdouble(v)) 
         snprintf(ret.str, VAL_STR_MAX_LEN, fmt? fmt : "%f" , valtodouble(v));
  else
         snprintf(ret.str, VAL_STR_MAX_LEN, "%016" PRIX64, v.v);

  return ret;
}


// This is needed to avoid warnings about unused static variables.
static inline uint64_t val_usestatic()
{
  return (valnil.v | valtrue.v | valfalse.v | valnullptr.v | (uintptr_t)val_emptystr);
}
#endif
