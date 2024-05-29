//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "val_.h"
#include "vec_.h"
#include "buf_.h"

char *valemptystr="\0\0\0";

int valisstored_(val_t v)
{
  if ((v.v & VAL_TYPE_MASK) != VAL_STO_MASK) return 0;

  return ((v.v & 0x0000FF0000000000) >> 40) == buf_stores_cnt[(v.v & 0x0000000300000000) >> 32];
}

int valtype(val_t v)
{
#if 0
  if (valisdouble(v))  return VALDOUBLE;
  if (valisbool(v))    return VALBOOL;
  if (valisnil(v))     return VALNIL;
  if (valispointer(v)) return VALPOINTER;
  if (valisstring(v))  return VALSTRING;
  if (valisvec(v))     return VALVEC; 
  if (valisbuf(v))     return VALBUF; 
  if (valisconst(v))   return VALCONST; 
  if (valisstored(v))  return VALSTORED;
#endif
#if 0
  uint64_t t = v.v & VAL_TYPE_MASK;
  switch (t) {
    case VAL_BUF_MASK: return VALBUF;
    case VAL_VEC_MASK: return VALVEC;
    case VAL_STR_MASK: return VALSTRING;
    case VAL_PTR_MASK: return VALPOINTER;
    case VAL_UNS_MASK: return VALINTEGER;
    case VAL_INT_MASK: return VALINTEGER;
    case VAL_CST_MASK: return VALCONST;
    case VAL_STO_MASK: return VALSTORED;
    case ((uint64_t)0xFFF8000000000000): return (t & 0xF0) == 0xE0 ? VALNIL : VALBOOL;
  }
  if (valisdouble(v))  return VALDOUBLE;
 #endif

  if (val_isdouble(v))  return VALDOUBLE;
  if (val_isnil(v))  return VALNIL;
  if (val_isbool(v)) return VALBOOL;
  return (v.v >> 48);

}

char *valtostring(val_t v)
{
  char *s ;
  uint64_t n;
  val_t b;
  int sto;
  errno = 0;
  switch (valtype(v)) {
    case VALSTRING: s = ((char *)valtopointer(v));  val_dbg("STRING IS STRING OR POINTER (%p) [%s]",(void *)s,s); return s; 
    case VALBUF:    s = buf(v,0); val_dbg("STRING IS BUF (%p) [%s]",(void *)s,s); return s;
    case VALSTORED: n = v.v & VAL_PAYLOAD;
                    sto = (n & 0x0000000300000000) >> 32;
                    b = buf_stores[sto];
                    if ((n >> 40) == buf_stores_cnt[sto])
                       return buf(b, (n & 0xFFFFFFFF));
  } 
  val_dbg("STRING IS NULL");
  errno = EINVAL;
  return valemptystr;
}

#if 0
int valcmp(val_t a, val_t b)
{
  double d_a, d_b;
  char *s_a, *s_b;

  int t_a = valtype(a);
  int t_b = valtype(b);

  switch (t_a << 4 | t_b) {

    case VALDOUBLE   << 4 | VALDOUBLE  :   d_a = valtodouble(a);          d_b = valtodouble(b);          goto cmpdbl;
    case VALDOUBLE   << 4 | VALINTEGER :   d_a = valtodouble(a);          d_b = (double)valtointeger(b); goto cmpdbl;
    case VALINTEGER  << 4 | VALDOUBLE  :   d_a = (double)valtointeger(a); d_b = valtodouble(b);          goto cmpdbl;
    cmpdbl:
      return d_a == d_b ? 0 : d_a > d_b ? 1 : -1;
          
    case VALSTRING  << 4 | VALSTRING  :
    case VALSTRING  << 4 | VALBUF     :
    case VALSTRING  << 4 | VALSTORED  :
    case VALBUF     << 4 | VALSTRING  :
    case VALBUF     << 4 | VALBUF     :
    case VALBUF     << 4 | VALSTORED  :
    case VALSTORED  << 4 | VALSTRING  :
    case VALSTORED  << 4 | VALBUF     :
    case VALSTORED  << 4 | VALSTORED  :
      s_a = valtostring(a); s_b = valtostring(b);
      val_dbg("[%s] [%s]",s_a,s_b);
      if (s_a == s_b)  return  0;
      if (s_a == NULL) return -1;
      if (s_b == NULL) return  1;
      return strcmp(s_a, s_b);

    case VALNIL << 4 | VALNIL:  return 0;

    default:
      if (t_a == VALNIL) return -1;
      return a.v == b.v ? 0 : a.v > b.v ? 1 : -1;
  }
  return 1;
}
#endif

int valcmp(val_t a, val_t b)
{
  double d_a, d_b;
  char *s_a, *s_b;

  if (a.v == b.v) return 0;
  if (a.v == valnil.v) return -1;
  if (b.v == valnil.v) return 1;

  uint32_t t_a = valtype(a);
  uint32_t t_b = valtype(b);

  switch (t_a << 16 | t_b) {

    case VALINTEGER  << 16 | VALINTEGER: 
      return valtointeger(a) - valtointeger(b);
    
    case VALSTRING  << 16 | VALSTRING:
    case VALSTRING  << 16 | VALBUF:
    case VALSTRING  << 16 | VALSTORED:
    case VALBUF     << 16 | VALSTRING:
    case VALBUF     << 16 | VALBUF:
    case VALBUF     << 16 | VALSTORED:
    case VALSTORED  << 16 | VALSTRING:
    case VALSTORED  << 16 | VALBUF:
    case VALSTORED  << 16 | VALSTORED:
      s_a = valtostring(a);
      s_b = valtostring(b);
      if (s_a == s_b)  return  0;
      if (s_a == NULL) return -1;
      if (s_b == NULL) return  1;
      return strcmp(s_a, s_b);

    case VALDOUBLE   << 16 | VALDOUBLE:  d_a = valtodouble(a);           d_b = valtodouble(b);   break;
    case VALDOUBLE   << 16 | VALINTEGER: d_a = valtodouble(a);           d_b = (double)valtointeger(b);  break;
    case VALINTEGER  << 16 | VALDOUBLE:  d_a = (double)valtointeger(a);  d_b = valtodouble(b);  break;

    default:
      return a.v > b.v ? 1 : -1;
  }

  return d_a == d_b ? 0 : d_a > d_b ? 1 : -1;
}

char *VALDOUBLE_fmt   = "%f";
char *VALSIGNED_fmt   = "%ld";
char *VALUNSIGNED_fmt = "%lu";
char *VALSTRING_fmt   = "%s";
char *VALCONST_fmt    = "%016X"; 
    
void valprintf(val_t v, FILE *f) 
{
   switch (valtype(v)) {
    case VALBOOL:    fprintf(f,"%s",valeq(v,valtrue)? "true" : "false") ; break;
    case VALNIL:     fprintf(f,"nil") ; break;
    case VALVEC:     fprintf(f,"%p[]",(void *)valtopointer(v)) ; break;
    case VALPOINTER: fprintf(f,"%p",(void *)valtopointer(v)) ; break;
    case VALBUF:
    case VALSTORED:
    case VALSTRING:  fprintf(f,VALSTRING_fmt,valtostring(v)); break;
    case VALDOUBLE:  fprintf(f,VALDOUBLE_fmt,valtodouble(v)); break;
    case VALINTEGER: if (valissigned(v)) fprintf(f,VALSIGNED_fmt,(int64_t)valtointeger(v)); 
                     else fprintf(f,VALUNSIGNED_fmt,(uint64_t)valtointeger(v));
                     break;
    case VALCONST:   fprintf(f,VALCONST_fmt,v.v); break;
    default:         fprintf(f,"%016lX",v.v); break;
   }
}

#define FNV_32_PRIME ((uint32_t)0x01000193)
#define FNV1_32_INIT ((uint32_t)0x811c9dc5)
#define hash fnv_1a
static uint32_t fnv_1a(void *buf, size_t len)
{
    unsigned char *bp = (unsigned char *)buf;
    uint32_t hval = FNV1_32_INIT;
    if (len > 0) {
      while (len--) {
        hval ^= (uint32_t)*bp++;
        hval *= FNV_32_PRIME;
      }
    }
    else {
      while (*bp) {
        hval ^= (uint32_t)*bp++;
        hval *= FNV_32_PRIME;
      }
    }
    return hval;
}


uint32_t valhash_(val_t v)
{
  void *s = valtostring(v);
  uint32_t h;
  if (errno == 0)
    h = hash(s, 0);
  else
    h = hash((void*)&v, sizeof(val_t));

 	h ^= h << 13;
	h ^= h >> 17;
	h ^= h << 5;
  return h;
}

uint32_t val_getaux(val_t v)
{
  errno = 0;
  uint32_t *aux;

  switch(valtype(v)) {
    case VALVEC:    return valtovec(v)->aux;
    case VALBUF:    return valtobuf(v)->aux;
    case VALSTORED: aux = (uint32_t *)valtostring(v);
                    if (aux != (uint32_t *)valemptystr) return aux[-1];
  }
  errno = EINVAL;
  return 0;
}

uint32_t val_setaux(val_t v, uint32_t n)
{
  uint32_t *aux;
  errno = 0;
  switch(valtype(v)) {
    case VALVEC:    return valtovec(v)->aux = n;
    case VALBUF:    return valtobuf(v)->aux = n;
    case VALSTORED: aux = (uint32_t *)valtostring(v);
                    if (aux != (uint32_t *)valemptystr) return aux[-1] = n;
  }
  errno = EINVAL;
  return 0;
}
