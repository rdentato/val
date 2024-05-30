//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.1.0 Beta

#ifndef VAL_VERSION
#define VAL_VERSION 0x0001000B

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// DBG
#if defined(NDEBUG) && defined(DEBUG)
#undef DEBUG
#endif

#ifdef DEBUG
  #define valdbg(...) (fprintf(stderr,"      INFO|  "),fprintf(stderr, __VA_ARGS__),fprintf(stderr," [%s:%d]\n",__FILE__,__LINE__))
#else
  #define valdbg(...)
#endif
#define val_dbg(...)

#define VAL_cnt(x1,x2,x3,x4,xN, ...) xN
#define VAL_n(...)       VAL_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define VAL_join(x ,y)   x ## y
#define VAL_cat(x, y)    VAL_join(x, y)
#define VAL_vrg(f, ...)  VAL_cat(f, VAL_n(__VA_ARGS__))(__VA_ARGS__)
#define VAL_VRG(f, ...)  VAL_cat(f, VAL_n(__VA_ARGS__))(__VA_ARGS__)

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

typedef struct vec_s *vec_t;
typedef struct buf_s *buf_t;

// Some bitmask
#define VAL_TYPE_MASK ((uint64_t)0xFFFF000000000000)
#define VAL_NAN_MASK  ((uint64_t)0x7FF0000000000000)
#define VAL_PAYLOAD   ((uint64_t)0x0000FFFFFFFFFFFF)

#define VAL_BUF_MASK  ((uint64_t)0xFFFE000000000000)
#define VAL_VEC_MASK  ((uint64_t)0xFFFF000000000000)

#define VAL_STR_MASK  ((uint64_t)0x7FFC000000000000)
#define VAL_PTR_MASK  ((uint64_t)0x7FFD000000000000)
#define VAL_UNS_MASK  ((uint64_t)0x7FFE000000000000)
#define VAL_INT_MASK  ((uint64_t)0x7FFF000000000000)
#define VAL_CST_MASK  ((uint64_t)0xFFF9000000000000)
#define VAL_BOL_MASK  ((uint64_t)0xFFF8FFFFFFFFFF00)
#define VAL_STO_MASK  ((uint64_t)0x7FF9000000000000)

#define VALDOUBLE   0x0001
#define VALBOOL     0x0002
#define VALNIL      0x0004
#define VALSTORED   0x7FF9
#define VALSTRING   0x7FFC
#define VALPOINTER  0x7FFD
#define VALINTEGER  0x7FFF
#define VALCONST    0xFFF9
#define VALBUF      0xFFFE
#define VALVEC      0xFFFF
#define VALPREDEF   0xFFF8


// Check the type of a val_t variable
#define val_isbool(x)    (((x).v & (uint64_t)0xFFFFFFFFFFFFFFFE) == VAL_BOL_MASK )
#define val_isdouble(x)  (((x).v & VAL_NAN_MASK)  != VAL_NAN_MASK)
#define val_isnil(x)     ((x).v == valnil.v)

#define valisinteger(x) ((val(x).v & (uint64_t)0xFFFE000000000000) == VAL_UNS_MASK)
#define valissigned(x)  ((val(x).v & VAL_TYPE_MASK) == VAL_INT_MASK)
#define valisbool(x)    ((val(x).v & (uint64_t)0xFFFFFFFFFFFFFFFE) == VAL_BOL_MASK )
#define valisdouble(x)  ((val(x).v & VAL_NAN_MASK)  != VAL_NAN_MASK)
#define valispointer(x) ((val(x).v & VAL_TYPE_MASK) == VAL_PTR_MASK)
#define valisstring(x)  ((val(x).v & VAL_TYPE_MASK) == VAL_STR_MASK)
#define valisvec(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_VEC_MASK)
#define valisbuf(x)     ((val(x).v & VAL_TYPE_MASK) == VAL_BUF_MASK)
#define valisstored(x)  valisstored_(val(x))
int valisstored_(val_t x);

#define valisownedvec(x)  ((val(x).v & (VAL_TYPE_MASK | 1)) == (VAL_VEC_MASK | 1))
#define valisownedbuf(x)  ((val(x).v & (VAL_TYPE_MASK | 1)) == (VAL_BUF_MASK | 1))

#define valisnil(x)     (val(x).v == valnil.v)
#define valiszero(x)    ((val(x).v & VAL_PAYLOAD) == 0)
#define valiserror(x)   (val(x).v == valerror.v)

// Store a value into a val_t variable
static inline val_t val_fromchar(char v)             {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromuchar(unsigned char v)   {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromint(int v)               {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromuint(unsigned int v)     {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromshort(short v)           {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromushort(unsigned short v) {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromlong(long v)             {val_t ret; ret.v = VAL_INT_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_fromulong(unsigned long v)   {val_t ret; ret.v = VAL_UNS_MASK | ((uint64_t)(v) & VAL_PAYLOAD);  return ret;}
static inline val_t val_frombool(_Bool v)            {val_t ret; ret.v = VAL_BOL_MASK | ((uint64_t)((v) & 1));          return ret;}
static inline val_t val_fromptr(void *v)             {val_t ret; ret.v = VAL_PTR_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_fromstr(void *v)             {val_t ret; ret.v = VAL_STR_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_fromvec(void *v)             {val_t ret; ret.v = VAL_VEC_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}
static inline val_t val_frombuf(void *v)             {val_t ret; ret.v = VAL_BUF_MASK | ((uintptr_t)(v) & VAL_PAYLOAD); return ret;}

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
                            buf_t: val_frombuf,    \
                  unsigned char *: val_fromstr,    \
                           char *: val_fromstr,    \
                           void *: val_fromptr,    \
                          default: val_fromptr) (x)

#define val(x) _Generic((x), val_t: x, default: val_(x))

#define valeq(x,y)      valeq_(val(x),val(y))
static inline int valeq_(val_t x, val_t y)
{
  if (x.v == y.v) return 1;
  if (valisinteger(x))
    return (x.v & ((uint64_t)0xFFFEFFFFFFFFFFFF)) == (y.v & ((uint64_t)0xFFFEFFFFFFFFFFFF));
  return 0;
}

#define valpayload(x) (val(x).v & VAL_PAYLOAD)

// Retrieve a value from a val_t variable
#define valtodouble(v) val_todouble(val(v))
static inline   double val_todouble(val_t v)  { double d; memcpy(&d,&v,8); return d;}

#define valtofloat(v) val_tofloat(val(v))
static inline  float  val_tofloat(val_t v)    { return (float)val_todouble(v);}

#define valtobool(v) val_tobool(val(v))
static inline  _Bool val_tobool(val_t v)      { return (_Bool)((v.v)&1);}

#define VAL_SIGNED_MASK ((uint64_t)0x0001800000000000)
#define valtointeger(x) val_tointeger(val(x))
static inline      int64_t val_tointeger(val_t v) \
                                  { int64_t ret = ((v.v) & VAL_PAYLOAD); 
                                    if ( ((v.v) & VAL_SIGNED_MASK) == VAL_SIGNED_MASK) // if signed and negative
                                       ret |= (uint64_t)0xFFFF000000000000;            // then extend sign
                                    return ret;
                                  }

static inline   void *val_topointer(val_t v, uint64_t mask) { return (void *)((uintptr_t)((v.v) & mask));}

#define val_to_pointer(v_, m_) ((void *)((uintptr_t)((val(v_).v) & (m_))))

#define valtopointer(v) val_to_pointer(v,VAL_PAYLOAD)
#define valtocleanpointer(v) ((void *)val_to_pointer(v,(uint64_t)0x0000FFFFFFFFFFFE))

#define valtovec(v)    ((vec_t)valtocleanpointer(v))
#define valtobuf(v)    ((buf_t)valtocleanpointer(v))

char *valtostring(val_t v);

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


#define val_first(x,...) x
#define val_rest(x,...) __VA_ARGS__

#define valconst(...) valconst_(val_first(__VA_ARGS__),val_rest(__VA_ARGS__) +0)
#define valconst_(x,y)  ((val_t){((0xFFF9000000000000 + (y) * 0x1000000000000)) | ((uint64_t)((uintptr_t)(x)) & VAL_PAYLOAD)})

#define valisconst(...) valisconst_(val_first(__VA_ARGS__),val_rest(__VA_ARGS__) +0)
#define valisconst_(x,y) ((val(x).v & (uint64_t)0xFFFF000000000000) == ((uint64_t)0xFFF9000000000000 + (y) * 0x1000000000000))

//#define valconst(x)   ((val_t){0xFFF9000000000000 | ((uint64_t)((uintptr_t)(x)) & VAL_PAYLOAD)})
//#define valisconst(x) ((val(x).v & (uint64_t)0xFFFF000000000000) == ((uint64_t)0xFFF8000000000000))

extern char *valemptystr;

#define valnilstr   val_nilstr()
static inline val_t val_nilstr() {return val_fromstr(valemptystr);}

extern char *VALDOUBLE_fmt  ;
extern char *VALSIGNED_fmt  ;
extern char *VALUNSIGNED_fmt;
extern char *VALSTRING_fmt  ;
extern char *VALCONST_fmt   ;
#define valfmt(t,s) (VAL##t##_fmt = s)

void valprintf(val_t v, FILE *f);

int  valtype(val_t v);
int  valcmp(val_t a, val_t b);

#define valreturnif(x,r,e) if (!(x)) errno=0; else return (errno = (e), r)

#define valaux(...) VAL_vrg(valaux_,__VA_ARGS__)
#define valaux_1(v)   val_getaux(v)
#define valaux_2(v,n) val_setaux(v,n)

uint32_t val_getaux(val_t v);
uint32_t val_setaux(val_t v, uint32_t n);

#define valhash(x) valhash_(val(x))
uint32_t valhash_(val_t v);

#endif
