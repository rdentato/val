//  SPDX-FileCopyrightText: © 2025 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.3.0 Beta

#ifndef VAL_VERSION
#define VAL_VERSION 0x0003000B

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

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

// We use Quiet NaN's (i.e. no trap or exception will be triggered: IEEE 754-2019 § 6.2.1)
// The tag that identifies which type the payload is, uses 4 bits:
//
//                  ╭─── Quiet NaN
//                  ▼
//   x111 1111 1111 1xxx FF FF FF FF FF FF
//   │╰──────┬──────╯╰┬╯ ╰───────┬───────╯
//  tag      │       tag         │ 
//           │                   │ 
//    12 bits set to 1   Payload (48 bits)
//          7FF8
//
//  The value "Quiet NaN Indefinite" which is the value returend by invalid operations like
//  0/0, is usually 0xFFF8000000000000 (that's the case both for Intel and ARM 64 bits processors).
//  This library never uses or generates this value to avoid conflicts. This is not a guarantee as
//  the IEEE standard only requires that a NaN is produced, but works well for most modern computers.

typedef struct {uint64_t v;} val_t;

static_assert(sizeof(val_t) == sizeof(uint64_t));
static_assert(sizeof(val_t) == 8);

// Only used in conjunction with vec.h. Assumes that the first field in the struct
// is a pointer to the buffer (as it is the case for `struct vec_s` in vec.h) so 
// that the C11 standard (§6.3.2.3) ensure we can convert from a pointer to the 
// structure to a pointer its first element. Only used in valcmp() and valhash().
// If you're using val.h as a standalone library, just ignore the following line.
typedef struct val_buf_s *val_buf_t;

// ==== FLOATING POINT DOUBLES
// A value v is NaN-boxed if  (v & VAL_NAN_MASK) == VAL_NAN_MASK
// Otherwise is a double (i.e. is not a NaN).
#define valisdouble(x)  ((val(x).v & VAL_NAN_MASK) != VAL_NAN_MASK)

#define VAL_NAN_MASK      ((uint64_t)0x7FF8000000000000)
#define VAL_TAG_MASK      ((uint64_t)0xFFFF000000000000)
#define VAL_PAYLOAD_MASK  ((uint64_t)0x0000FFFFFFFFFFFF)

// ==== INTEGERS 
#define VAL_UINT_48       ((uint64_t)0xFFFE000000000000)
#define VAL_INT_48        ((uint64_t)0xFFFF000000000000)
#define VAL_INT_SIGN      ((uint64_t)0x0001800000000000)

#define valissignedint(x)    ((val(x).v & VAL_TAG_MASK) == VAL_INT_48)
#define valisunsignedint(x)  ((val(x).v & VAL_TAG_MASK) == VAL_UINT_48)

// ==== POINTERS
#define VAL_PTR_MASK      ((uint64_t)0xFFF8000000000000)
#define VAL_PTRTAG_VOID   ((uint64_t)0x7FF8000000000000)
#define VAL_PTRTAG_CHAR   ((uint64_t)0x7FF9000000000000)
#define VAL_PTRTAG_BUF    ((uint64_t)0x7FFA000000000000)

// This is reserved for future expansions.
#define VAL_PTRTAG_5      ((uint64_t)0x7FFB000000000000)
#ifndef valpointer_5_t
typedef struct valpointer_5_s *valpointer_5_t; 
#endif

// These can be user defined.
#define VAL_PTRTAG_4      ((uint64_t)0x7FFC000000000000)
#define VAL_PTRTAG_3      ((uint64_t)0x7FFD000000000000)
#define VAL_PTRTAG_2      ((uint64_t)0x7FFE000000000000)
#define VAL_PTRTAG_1      ((uint64_t)0x7FFF000000000000)

#ifndef valpointer_4_t
typedef struct valpointer_4_s *valpointer_4_t; 
#endif
#ifndef valpointer_3_t
typedef struct valpointer_3_s *valpointer_3_t; 
#endif
#ifndef valpointer_2_t
typedef struct valpointer_2_s *valpointer_2_t; 
#endif
#ifndef valpointer_1_t
typedef struct valpointer_1_s *valpointer_1_t; 
#endif

#define valispointer(p,...) val_ispointer(val(p), __VA_ARGS__ +0)
static inline int val_ispointer(val_t v, uint64_t tag)  {
    if (tag == 0) return ((v.v & VAL_PTR_MASK)  == VAL_PTRTAG_VOID);
  else            return ((v.v & VAL_TAG_MASK)  == tag); 
}

#define valischarptr(x)  ((val(x).v & VAL_TAG_MASK) == VAL_PTRTAG_CHAR)
#define valisbufptr(x)   ((val(x).v & VAL_TAG_MASK) == VAL_PTRTAG_BUF)

// ==== CONSTANTS
#define VAL_CONST_MASK    ((uint64_t)0xFFFFFFFF00000000)
#define VAL_CONST_0       ((uint64_t)0xFFF8000F00000000)
#define VAL_BOOL          ((uint64_t)0xFFF8FFFF00000000)
#define VAL_32BIT_MASK    ((uint64_t)0x00000000FFFFFFFF)

// Booleans
#define valfalse        ((val_t){VAL_BOOL })
#define valtrue         ((val_t){VAL_BOOL | 1})
#define valisboolean(x) ((val(x).v & VAL_CONST_MASK) == VAL_BOOL)


// A nil value to signal a void value
#define VAL_NIL     ((uint64_t)0xFFF800FF00000000)
static const val_t valnil = {VAL_NIL};
#define valisnil(x)     ((val(x).v == VAL_NIL))

// The val_t corresponding of the C pointer NULL
static const val_t valnullptr = (val_t){VAL_PTRTAG_VOID};
#define valisnullptr(x)  (val(x).v  == VAL_PTRTAG_VOID)

// User defined constants
#define valconst(x) ((val_t){ VAL_CONST_0 | ((x) & VAL_32BIT_MASK)})
#define valisconst(x)   ((val(x).v & VAL_CONST_MASK) == VAL_CONST_0)

// Convert a C value to a val_t
// Note that these functions return a structure (NOT a pointer).

// We only store floating point numbers as doubles since it is guaranteed by the
// C Standard (C11/C17 § 6.3.1.5 [Real floating types]), that any float (4-byte
// floating point real number) can be exactly represented as a double (8 bytes number).

static inline val_t val_fromdouble(double v)    {val_t ret = valnil; memcpy(&ret,&v,sizeof(val_t)); return ret;}
static inline val_t val_fromfloat(float f)      {return val_fromdouble((double)f);}

static inline val_t val_frompvoidtr(void *v)    {val_t ret; ret.v = VAL_PTRTAG_VOID  | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromcharptr(void *v)    {val_t ret; ret.v = VAL_PTRTAG_CHAR  | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_frombufptr(void *v)     {val_t ret; ret.v = VAL_PTRTAG_BUF   | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_5(void *v)      {val_t ret; ret.v = VAL_PTRTAG_5 | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_4(void *v)      {val_t ret; ret.v = VAL_PTRTAG_4 | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_3(void *v)      {val_t ret; ret.v = VAL_PTRTAG_3 | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_2(void *v)      {val_t ret; ret.v = VAL_PTRTAG_2 | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}
static inline val_t val_fromptr_1(void *v)      {val_t ret; ret.v = VAL_PTRTAG_1 | ((uintptr_t)(v) & VAL_PAYLOAD_MASK); return ret;}

static inline val_t val_frombool(_Bool v)       {val_t ret; ret.v = VAL_BOOL | ((uint64_t)((v) & 1)); return ret;}

static inline val_t val_fromint(int64_t v)      {val_t ret; ret.v = VAL_INT_48  | ((v) & VAL_PAYLOAD_MASK);  return ret;}
static inline val_t val_fromuint(uint64_t v)    {val_t ret; ret.v = VAL_UINT_48 | ((v) & VAL_PAYLOAD_MASK);  return ret;}

static inline val_t val_fromval(val_t v)        {return v;}

#define val_(x) _Generic((x), int: val_fromint,     \
                             char: val_fromint,     \
                            short: val_fromint,     \
                             long: val_fromint,     \
                        long long: val_fromint,     \
                     unsigned int: val_fromuint,    \
                    unsigned char: val_fromuint,    \
                   unsigned short: val_fromuint,    \
                    unsigned long: val_fromuint,    \
               unsigned long long: val_fromuint,    \
                            _Bool: val_frombool,    \
                           double: val_fromdouble,  \
                            float: val_fromfloat,   \
                           void *: val_frompvoidtr, \
                  unsigned char *: val_fromcharptr, \
                    signed char *: val_fromcharptr, \
                           char *: val_fromcharptr, \
                        val_buf_t: val_frombufptr,  \
                   valpointer_5_t: val_fromptr_5,   \
                   valpointer_4_t: val_fromptr_4,   \
                   valpointer_3_t: val_fromptr_3,   \
                   valpointer_2_t: val_fromptr_2,   \
                   valpointer_1_t: val_fromptr_1,   \
                            val_t: val_fromval  ,   \
                          default: val_frompvoidtr  \
                          ) (x)

// The C11 standard (§6.5.1.1) guarantess that x is only evaluated once.
// The first occurence will not be evaluated as only its type is considered
// and then only one of the branches are included in the translation unit.
// This improves performance as val_t values pass through val() with no computation.
#define val(x) _Generic((x), val_t: x, default: val_(x))

// ====== Retrieve values from a val_t variable
#define valtobool(v) val_tobool(val(v))
static inline  _Bool val_tobool(val_t v)  {return ((v.v & VAL_32BIT_MASK) != 0);}

static inline uint64_t val_tounsignedint(val_t v);
static inline int64_t val_tosignedint(val_t v);

#define valtodouble(v) val_todouble(val(v))
static inline double val_todouble(val_t v)  
{
  double d=0.0;
       if (valisdouble(v))  memcpy(&d,&v,sizeof(double)); 
  else if (valissignedint(v)) d = (double)val_tosignedint(v);
  else if (valisunsignedint(v)) d = (double)val_tounsignedint(v);
  return d;
}

#define valtosignedint(v)  val_tosignedint(val(v))
static inline int64_t val_tosignedint(val_t v)
{ int64_t ret = 0;
  if (valissignedint(v)) {
    ret =((v.v) & VAL_PAYLOAD_MASK); 
    ret |= ((v.v & VAL_INT_SIGN) == VAL_INT_SIGN) ? (uint64_t)0xFFFF000000000000 : (uint64_t)0x0;
  }
  else if (valisunsignedint(v))  ret = (int64_t)val_tounsignedint(v); 
  else if (valisdouble(v)) ret = (int64_t)val_todouble(v);
  return ret;
}

#define valtounsignedint(v)  val_tounsignedint(val(v))
static inline uint64_t val_tounsignedint(val_t v) {
  uint64_t ret = 0;
  if (valisunsignedint(v))     ret = (v.v & VAL_PAYLOAD_MASK);
  else if (valissignedint(v))  ret = (int64_t)(v.v & VAL_PAYLOAD_MASK);
  else if (valisdouble(v))     ret = (uint64_t)val_todouble(v);
  return ret;
}

#define valtopointer(v) val_topointer(val(v))
static inline   void *val_topointer(val_t v) {return (void *)((uintptr_t)((v.v) & VAL_PAYLOAD_MASK));}

#define valpointertag(v) val_pointertag(val(v))
static inline uint64_t val_pointertag(val_t v) 
{
  uint64_t ret = 0;
  if (valispointer(v)) {
    ret = (v.v) & VAL_TAG_MASK;
  }
  return ret ;
}

// This checks for val_t values IDENTITY
#define valeq(x,y) (val(x).v == val(y).v)

// This compares two val_t values. LIke the hash function below, It is provide just for convenience 
// since your criteria for comparison and hashing might be different.
#define valcmp(a,b) val_cmp(val(a),val(b))
static inline int val_cmp(val_t a, val_t b)
{
  char *sa = NULL, *sb = NULL;
  
       if (valischarptr(a)) sa = valtopointer(a);
  else if (valisbufptr(a) && ((sa = valtopointer(a)) != NULL)) sa = *((char **)sa);

  if (sa) {
         if (valischarptr(b)) sb = valtopointer(b);
    else if (valisbufptr(b) && ((sb = valtopointer(b)) != NULL)) sb = *((char **)sb);
  }

  if (sb) return strcmp(sa,sb);
  
  double da = 0.0,  db = 0.0;
  int    is_da = 0, is_db = 0;
  
  // We compare all numbers as floating points value. This works because a 48-bit integer can
  // be exactly represented by a double (IEEE 754-2019 §6.2.6 and §3.2.2)
       if ((is_da = valisdouble(a)))      da = valtodouble(a);
  else if ((is_da = valissignedint(a)))   da = (double)valtosignedint(a);
  else if ((is_da = valisunsignedint(a))) da = (double)valtounsignedint(a);

  if (is_da) {
         if ((is_db = valisdouble(b)))      db = valtodouble(b);
    else if ((is_db = valissignedint(b)))   db = (double)valtosignedint(b);
    else if ((is_db = valisunsignedint(b))) db = (double)valtounsignedint(b);

    if (is_db) return (da > db)? 1 : (da < db) ? -1 : 0 ;
  }

  return (a.v > b.v)? 1 : (a.v < b.v) ? -1 : 0 ;
}

#define valhash(a) val_hash(val(a))
static inline uint32_t val_hash(val_t v)
{
  uint32_t hash = (uint32_t)0x811c9dc5; // FNV1a INIT
  char *s = NULL;

       if (valischarptr(v)) s = valtopointer(v);
  else if (valisbufptr(v) && ((s = valtopointer(v)) != NULL)) s = *(char **)s;
  if (s) {
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

#endif
