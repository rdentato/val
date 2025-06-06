//  SPDX-FileCopyrightText: © 2025 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.3.5 Beta

#ifndef VAL_VERSION
#define VAL_VERSION 0x0003005B

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <stdalign.h>
#include <stddef.h>

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
//  The value "Quiet NaN Indefinite" which is the value returend by invalid operations like
//  0/0, is usually 0xFFF8000000000000 (that's the case both for Intel and ARM 64 bits processors).
//  This library never uses or generates this value to avoid conflicts. This is not a guarantee as
//  the IEEE standard only requires that a NaN is produced, without specifying which one.
//  However, the assumption should work reasonably well for most modern computers.

typedef struct {uint64_t v;} val_t;

static_assert(sizeof(val_t) == sizeof(uint64_t), "Wrong size for val_t");
static_assert(sizeof(val_t) == 8, "Wrong size for uint64_t");

// ==== Numbers
// All numbers are stored as a double floating point.
// A value v is NaN-boxed if  (v & VAL_NAN_MASK) == VAL_NAN_MASK
// Otherwise is a double (i.e. is not a NaN).
#define valisdouble(x)  ((val(x).v & VAL_NAN_MASK) != VAL_NAN_MASK)

#define VAL_NAN_MASK      ((uint64_t)0x7FF8000000000000)
#define VAL_TYPE_MASK     ((uint64_t)0xFFFF000000000000)
#define VAL_PAYLOAD_MASK  ((uint64_t)0x0000FFFFFFFFFFFF)

// By effect of the IEEE 754 standard, only integers up to 52 bits are representable.
// We'll check directly on the bit represantation of doubles. See IEEE754.md in the docs direcotory
#define valisint(x) val_isinteger(val(x))
static inline int val_isinteger(val_t v) {
 
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
#define VAL_CONST_0       ((uint64_t)0xFFF81AA100000000)
#define VAL_FALSE         ((uint64_t)0xFFF8FFFF00000000)
#define VAL_32BIT_MASK    ((uint64_t)0x00000000FFFFFFFF)

// Booleans
static const val_t valfalse = {VAL_FALSE};
static const val_t valtrue  = {VAL_FALSE | 1};

#define valisbool(x) ((val(x).v & VAL_CONST_MASK) == VAL_FALSE)

// A nil value to signal a void value
#define VAL_NIL       ((uint64_t)0xFFF800FF00000000)
static const val_t    valnil = {VAL_NIL};

#define valisnil(x)   ((val(x).v == VAL_NIL))

// User defined constants
#define valconst(x)   ((val_t){ VAL_CONST_0 | ((x) & VAL_32BIT_MASK)})

#define valisconst(x) ((val(x).v & VAL_CONST_MASK) == VAL_CONST_0)

// ==== POINTERS
// Checks that there are at least three zero bits in memory aligned pointers
static_assert(alignof(double) >= 8, "Need 8-bytes pointer alignment");

#define VAL_PTR_MASK    ((uint64_t)0xFFF8000000000000)

#define VAL_PTR_VOID    ((uint64_t)0x7FF8000000000000)
#define VAL_PTR_CHAR    ((uint64_t)0x7FF9000000000000)
#define VAL_PTR_CHARPTR ((uint64_t)0x7FFA000000000000)

// These are reserved for interactions with other libraries.
#define VAL_PTR_5       ((uint64_t)0x7FFB000000000000)

#ifndef valptr_5_t
typedef struct valptr_5_s *valptr_5_t; 
#endif

// These can be user defined.
#define VAL_PTR_4      ((uint64_t)0x7FFC000000000000)
#define VAL_PTR_3      ((uint64_t)0x7FFD000000000000)
#define VAL_PTR_2      ((uint64_t)0x7FFE000000000000)
#define VAL_PTR_1      ((uint64_t)0x7FFF000000000000)

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

#define val_is_any_ptr(v) ((v.v & VAL_PTR_MASK)  == VAL_PTR_VOID)
#define valisptr(p,...) val_isptr(val(p), __VA_ARGS__ +0)
static inline int val_isptr(val_t v, uint64_t ptr_type)  {
    if (ptr_type == 0) return val_is_any_ptr(v);
  else            return ((v.v & VAL_TYPE_MASK)  == ptr_type); 
}

#define valisvoidptr(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_PTR_VOID)
#define valischarptr(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_PTR_CHAR)
#define valischarptrptr(x)  ((val(x).v & VAL_TYPE_MASK) == VAL_PTR_CHARPTR)

// The val_t corresponding of the C pointer NULL
static const val_t valnullptr = {VAL_PTR_VOID};

#define valisnullptr(x)  (val(x).v  == VAL_PTR_VOID)

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

static inline val_t val_frompvoidtr(void *v)    {val_t ret; ret.v = VAL_PTR_VOID     | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromcharptr(void *v)    {val_t ret; ret.v = VAL_PTR_CHAR     | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromcharptrptr(void *v) {val_t ret; ret.v = VAL_PTR_CHARPTR  | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_5(void *v)      {val_t ret; ret.v = VAL_PTR_5        | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_4(void *v)      {val_t ret; ret.v = VAL_PTR_4        | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_3(void *v)      {val_t ret; ret.v = VAL_PTR_3        | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_2(void *v)      {val_t ret; ret.v = VAL_PTR_2        | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_1(void *v)      {val_t ret; ret.v = VAL_PTR_1        | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}

// BOOLEAN
static inline val_t val_frombool(_Bool v)       {val_t ret; ret.v = VAL_FALSE | ((uint64_t)(!!(v))); return ret;}

// VALUES (identity)
static inline val_t val_fromval(val_t v)        {return v;}

#define val_from(x) _Generic((x), int: val_fromint,        \
                                 char: val_fromint,        \
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
                      unsigned char *: val_fromcharptr,    \
                        signed char *: val_fromcharptr,    \
                               char *: val_fromcharptr,    \
                     unsigned char **: val_fromcharptrptr, \
                       signed char **: val_fromcharptrptr, \
                              char **: val_fromcharptrptr, \
                       valptr_5_t: val_fromptr_5,          \
                       valptr_4_t: val_fromptr_4,          \
                       valptr_3_t: val_fromptr_3,          \
                       valptr_2_t: val_fromptr_2,          \
                       valptr_1_t: val_fromptr_1,          \
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

// ==== Pointer tagging
// Except for `void *` and `char *`, any pointer can be *tagged* with 
// a number between 0 and 7 (0 is the default tag for pointers).
// This can be used, for example, to mark a pointer as "visited" or
// "to be disposed" without having to resort to additional memory.

static inline int val_check_taggable_ptr(val_t v) {
  // Not a pointer
  if ((v.v & VAL_PTR_MASK) != VAL_PTR_VOID) return -1; 

  // It's either a char or void pointer can't be tagged
  if ((v.v & VAL_TYPE_MASK) <= VAL_PTR_CHAR)  return 0;

  // A taggable pointer
  return 1;
}

// Tags are 3 bits long
#define VAL_TAG_MASK  ((uint64_t)0x07)

#define valtoptr(v) val_toptr(val(v))
static inline   void *val_toptr(val_t v) {
  uint64_t mask = VAL_PAYLOAD_MASK;

  switch (val_check_taggable_ptr(v)) {
    case -1: return NULL; // Not a pointer
    case  1: mask &= ~VAL_TAG_MASK; // taggable ptr. Need to eliminate the tag!
  }

  return (void *)((uintptr_t)(v.v & mask));
}

#define valptrtype(v) val_ptrtype(val(v))
static inline uint64_t val_ptrtype(val_t v) {
  return  valisptr(v) ? (v.v & VAL_TYPE_MASK) : 0;
}

#define val_x(...)     __VA_ARGS__
#define val_0(x,...)     x
#define val_1(_,x,...)   x

#define valtagptr(...)  val_tagptr_(val_x(val_0(__VA_ARGS__)),\
                                    val_x(val_1(__VA_ARGS__,(signed char)'4')))

#define val_tagptr_(v,t) _Generic((t), \
                             int: val_tagptr_set(val(v),(int)t), \
                     signed char: val_tagptr_get(val(v)) \
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

// This compares two val_t values. Like the hash function below, it is provide just for convenience 
// since your criteria for comparison and hashing might be different.
#define valcmp(a,b) val_cmp(val(a),val(b))
static inline int val_cmp(val_t a, val_t b) {
  char *sa = val_emptystr;
  char *sb = val_emptystr;

  if (valischarptr(a)) sa = valtoptr(a);
  else if (valischarptrptr(a)) {
    char **sa_ptr = valtoptr(a);
    sa = sa_ptr ? *sa_ptr : NULL;
  }
  
  if (sa != val_emptystr) {
    if (valischarptr(b)) sb = valtoptr(a);
    else if (valischarptrptr(b)) {
      char **sb_ptr = valtoptr(b);
      sb = sb_ptr ? *sb_ptr : NULL;
    }

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
  
  if (valisdouble(a)) return  1; // / Numbers are lower than
  if (valisdouble(b)) return -1; // \ any other type

  return (a.v > b.v)? 1 : (a.v < b.v) ? -1 : 0 ;
}

#define valhash(a) val_hash(val(a))
static inline uint32_t val_hash(val_t v) {
  uint32_t hash = (uint32_t)0x811c9dc5; // FNV1a INIT

  char *s = val_emptystr;

  if (valischarptr(v)) s = valtoptr(v);
  else if (valischarptrptr(v)) {
    char **s_ptr = valtoptr(v);
    s = s_ptr ? *s_ptr : NULL;
  }

  if (s != val_emptystr) {
    if (s != NULL) {
      // FNV1a abridged from http://www.isthe.com/chongo/tech/comp/fnv/index.html
      while (*s) {
        hash ^= (uint32_t)(*s++);
        hash *= (uint32_t)0x01000193; // FNV1a PRIME
      }
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

// This is needed to avoid warnings about unused static variables.
static inline uint64_t val_usestatic()
{
  return (valnil.v | valtrue.v | valfalse.v | valnullptr.v);
}

#endif
