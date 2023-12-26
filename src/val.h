//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.1.0 Beta

#ifndef VAL_VERSION
#define VAL_VERSION 0x0001000B

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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
// Intel® 64 and IA-32 Architectures Software Developer’s Manual vol 1.
// https://xem.github.io/minix86/manual/intel-x86-and-64-manual-vol1/o_7281d5ea06a5b67a.html
// (see table 4.3 on pages 90)

//                  ╭─── Quiet NaN
//                  │╭── QNaN Floating-Point Indefinites has 0 fron here up to the end
//                  ▼▼ 
//   x111 1111 1111 1xxx FF FF FF FF FF FF
//    ╰──────┬──────╯    ╰───────┬───────╯
//           │                   │ 
//    12 bits set to 1   Payload (48 bits)
//          7FF8

typedef struct {uint64_t v;} val_t;
typedef struct {char *s;} chs_t;
typedef struct vec_s *vec_t;
typedef struct blk_s *blk_t;

typedef struct val_dta_s {
  void *ptr;
  uint32_t sze;
  uint32_t cnt;
} val_dta_t;


// Some bitmask
#define VAL_TYPE_MASK ((uint64_t)0xFFFF000000000000)
#define VAL_NAN_MASK  ((uint64_t)0x7FF0000000000000)
#define VAL_PAYLOAD   ((uint64_t)0x0000FFFFFFFFFFFF)

#define VAL_OWN_MASK  ((uint64_t)0xFFFE000000000001)
#define VAL_BLK_MASK  ((uint64_t)0xFFFE000000000000)
#define VAL_VEC_MASK  ((uint64_t)0xFFFF000000000000)

#define VAL_STR_MASK  ((uint64_t)0x7FFC000000000000)
#define VAL_PTR_MASK  ((uint64_t)0x7FFD000000000000)
#define VAL_UNS_MASK  ((uint64_t)0x7FFE000000000000)
#define VAL_INT_MASK  ((uint64_t)0x7FFF000000000000)
#define VAL_CST_MASK  ((uint64_t)0xFFF9000000000000)
#define VAL_BOL_MASK  ((uint64_t)0xFFF8FFFFFFFFFF00)

#define valisowned(x) ((val(x).v & VAL_OWN_MASK) == VAL_OWN_MASK)

// Check the type of a val_t variable
#define valisinteger(x) ((val(x).v & (uint64_t)0xFFFE000000000000) == VAL_UNS_MASK)
#define valissigned(x)  ((val(x).v & VAL_TYPE_MASK) == VAL_INT_MASK)
#define valisbool(x)    ((val(x).v & (uint64_t)0xFFFFFFFFFFFFFFFE) == VAL_BOL_MASK )
#define valisdouble(x)  ((val(x).v & VAL_NAN_MASK)  != VAL_NAN_MASK)
#define valispointer(x) ((val(x).v & VAL_TYPE_MASK) == VAL_PTR_MASK)
#define valisstring(x)  ((val(x).v & VAL_TYPE_MASK) == VAL_STR_MASK)
#define valisvec(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_VEC_MASK)
#define valisblk(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_BLK_MASK)

#define valeq(x,y)      (val(x).v == val(y).v)

#define valisnil(x)     valeq(x,valnil)
#define valiszero(x)    ((val(x).v & VAL_PAYLOAD) == 0)
#define valiserror(x)   valeq(x,valerror)

// Store a value into a val_t variable
static inline val_t val_fromchar(char v)             {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)v);                  return ret;}
static inline val_t val_fromuchar(unsigned char v)   {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)v);                  return ret;}
static inline val_t val_fromint(int v)               {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)v);                  return ret;}
static inline val_t val_fromuint(unsigned int v)     {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)v);                  return ret;}
static inline val_t val_fromshort(short v)           {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)v);                  return ret;}
static inline val_t val_fromushort(unsigned short v) {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)v);                  return ret;}
static inline val_t val_fromlong(long v)             {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromulong(unsigned long v)   {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_frombool(_Bool v)            {val_t ret; ret.v = VAL_BOL_MASK | ((uint64_t)((v) & 1));          return ret;}
static inline val_t val_fromptr(void *v)             {val_t ret; ret.v = VAL_PTR_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_fromstr(void *v)             {val_t ret; ret.v = VAL_STR_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_fromvec(void *v)             {val_t ret; ret.v = VAL_VEC_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_fromblk(void *v)             {val_t ret; ret.v = VAL_BLK_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}

static inline val_t val_fromdouble(double v)         {val_t ret; memcpy(&ret,&v,sizeof(val_t)); return ret;}
static inline val_t val_fromfloat(float f)           {return val_fromdouble((double)f);}

static inline val_t val_fromval(val_t v)             {return v;}

#define val_(x) _Generic((x), int: val_fromint,    \
                             char: val_fromchar,   \
                            short: val_fromshort,  \
                             long: val_fromlong,   \
                            _Bool: val_frombool,   \
                     unsigned int: val_fromuint,   \
                    unsigned char: val_fromuchar,  \
                   unsigned short: val_fromushort, \
                    unsigned long: val_fromulong,  \
                           double: val_fromdouble, \
                            float: val_fromfloat,  \
                            val_t: val_fromval,    \
                            vec_t: val_fromvec,    \
                            blk_t: val_fromblk,    \
                  unsigned char *: val_fromstr,    \
                           char *: val_fromstr,    \
                           void *: val_fromptr) (x)

#define val(x) _Generic((x), val_t: x, default: val_(x))

// Retrieve a value from a val_t variable
#define valtodouble(v) val_todouble(val(v))
static inline   double val_todouble(val_t v)  { double d; memcpy(&d,&v,8); return d;}

#define valtofloat(v) val_tofloat(val(v))
static inline  float  val_tofloat(val_t v)    { return (float)val_todouble(v);}

#define valtobool(v) val_tobool(val(v))
static inline  _Bool val_tobool(val_t v)      { return (_Bool)((v.v)&1);}

#define VAL_SIGNED_MASK ((uint64_t)0x0001800000000000)
#define valtointeger(x) val_tointeger(val(x))
static inline      long val_tointeger(val_t v) \
                                  { long ret = ((v.v) & VAL_PAYLOAD); 
                                    if ( ((v.v) & VAL_SIGNED_MASK) == VAL_SIGNED_MASK) // if signed and negative
                                       ret |= (uint64_t)0xFFFF000000000000;            // then extend sign
                                    return ret;
                                  }

static inline   void *val_topointer(val_t v, uint64_t mask) { return (void *)((uintptr_t)((v.v) & mask));}

#define val_to_pointer(v_, m_) ((void *)((uintptr_t)((val(v_).v) & (m_))))

#define valtopointer(v) val_to_pointer(v,VAL_PAYLOAD)
#define valtostring(v) ((char *)valtopointer(v))
#define valtovec(v)    ((vec_t)valtopointer(v))
#define valtoblk(v)    ((blk_t)valtopointer(v))

// Some constant
#define valfalse      ((val_t){0xFFF8FFFFFFFFFF00})
#define valtrue       ((val_t){0xFFF8FFFFFFFFFF01})
#define valnil        ((val_t){0xFFF80FFFFFFFFFE0})
#define valundefined  ((val_t){0xFFF80FFFFFFFFFD0})
#define valerror      ((val_t){0xFFF80FFFFFFFFFD1})
#define valdeleted    ((val_t){0xFFF80FFFFFFFFFC0})
#define valempty      ((val_t){0xFFF80FFFFFFFFFB0})
#define valmarked     ((val_t){0xFFF80FFFFFFFFFE1})
#define valnewvec     ((val_t){0xFFF80AAAAAAAAA01})
#define valnewbuf     ((val_t){0xFFF80AAAAAAAAA02})
#define valown        ((val_t){0xFFF80AAAAAAAAAA1})
#define valdisown     ((val_t){0xFFF80AAAAAAAAAA2})
#define valnilpointer ((val_t){0xFFFD000000000000})

#define valconst(x)   ((val_t){0x7FF9000000000000 | (uint32_t)(x)})
#define valisconst(x) ((val(x).v & (uint64_t)0xFFFFFFFF00000000) == ((uint64_y)0x7FF800000000000))

#define valnilstr   val_nilstr()
static inline val_t val_nilstr() {static char *s=""; return val_fromstr(s);}

#define VALDOUBLE   1
#define VALINTEGER  2
#define VALBOOL     3
#define VALNIL      4
#define VALPOINTER  5
#define VALSTRING   6
#define VALVEC      7
#define VALBLK      8

static inline int valtype(val_t v) {
  if (valisdouble(v))  return VALDOUBLE;
  if (valisinteger(v)) return VALINTEGER;
  if (valisbool(v))    return VALBOOL;
  if (valisnil(v))     return VALNIL;
  if (valispointer(v)) return VALPOINTER;
  if (valisstring(v))  return VALSTRING;
  if (valisvec(v))     return VALVEC; 
  if (valisblk(v))     return VALBLK; 
  return 0;
}

#endif
