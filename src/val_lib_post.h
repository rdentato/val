//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT
#line 4 "val_lib_post.h"


#ifndef VAL_LIB
#define VAL_LIB
#include "val.h"
#endif

#include <stdlib.h>

struct valptr_vec_s {
  val_t   *vec;
  int32_t  sze;
  int32_t  end;
  int32_t  fst;
  int32_t  buf;
};

val_t valaux();

#define VAL_VEC_EMPTY 0x0000
#define VAL_VEC_MAP   0x0001
#define VAL_VEC_ARRAY 0x0002
#define VAL_VEC_STACK 0x0004
#define VAL_VEC_QUEUE 0x0008

#define VAL_VEC_MAXNDX   ((int32_t)0x1FFFFFF0)
#define VAL_VEC_NONDX    ((int32_t)0x40000001)
#define VAL_VEC_ENDNDX   ((int32_t)0x40000002)
#define VAL_VEC_FIRSTNDX ((int32_t)0x40000003)
#define VAL_VEC_LASTNDX  (-1)
#define VAL_VEC_ENQNDX   ((int32_t)0x40000005)

#define valvec(...) val_vec(__VA_ARGS__ +0)
val_t val_vec(int32_t n);
val_t valfree(val_t a);

int32_t valsize(val_t a);
int32_t val_ndx(valptr_vec_t a, int32_t ndx);

static inline valptr_vec_t val_vecptr(val_t a)
{
  valptr_vec_t vec_ptr = NULL;
  errno = 0;
  if (!valisvecptr(a) || ((vec_ptr = valtoptr(a)) == NULL) ) errno = EINVAL; 
  return vec_ptr;
}

static inline val_t *valvecbase(val_t a) {
  valptr_vec_t vec_ptr = val_vecptr(a);
  if (vec_ptr == NULL) {errno = EINVAL; return NULL; }
  errno = 0;
  return vec_ptr->vec;
}

static inline int32_t val_count(valptr_vec_t vec_ptr) {
  return (vec_ptr != NULL) ? (vec_ptr->end - vec_ptr->fst) : 0;
}
#define valcount(a) val_count(val_vecptr(a))

int val_set_vec(val_t a, int32_t ndx, val_t elm);
int val_set_map(val_t a, val_t key, val_t elm);

#define valset(a,i,v) _Generic((i), \
                           char *: val_set_chrptr_ndx, \
                            val_t: val_set_map, \
                          default: val_set_vec) (a,i,val(v))

static inline int val_set_chrptr_ndx(val_t a, char *ndx, val_t v) {
  return val_set_map(a, val(ndx), v);
}

#define valins(a,i,e,...) val_ins(a,i,val(e),__VA_ARGS__+0)
int val_ins(val_t a, int32_t ndx, val_t elm, int32_t rep);

val_t val_get_vec_p(valptr_vec_t a, int32_t i);
val_t val_get_vec(val_t a, int32_t i);
val_t val_get_map(val_t a, val_t k);

#define valget(a,ndx) _Generic((ndx), \
                         char *: val_get_chrptr_ndx, \
                          val_t: val_get_map, \
                        default: val_get_vec)(a, ndx)

static inline val_t val_get_chrptr_ndx(val_t a, char *ndx) {
  return val_get_map(a, val(ndx));
}

int val_del_vec(val_t a, int32_t from, int32_t to);
int val_del_map(val_t a, val_t k);

#define valdel(...) VAL_vrg(val_del_,__VA_ARGS__)

#define val_del_1(a)      val_del_vec(a, -1, -1)
#define val_del_3(a,f,t)  val_del_vec(a,f,t)
#define val_del_2(a,ndx)  _Generic((ndx), \
                            char *: val_del_chrptr_ndx, \
                             val_t: val_del_map, \
                           default: val_del_int_ndx)(a, ndx)

static inline int val_del_chrptr_ndx(val_t a, char *ndx) {
  return val_del_map(a, val(ndx));
}

static inline int val_del_int_ndx(val_t a, int32_t ndx) {
  return val_del_vec(a, ndx, ndx);
}

// STACK

#define valpush(a,e)  val_set_vec(a, VAL_VEC_ENDNDX, val(e))
#define valtop(a)     val_get_vec(a, -1)
#define valisempty(a)  (valcount(a) == 0)

static inline val_t valpop(val_t a) {
  val_t top = valtop(a);
  if (errno == 0) ((valptr_vec_t)valtoptr(a))->end -= 1;
  return top;
}

static inline int val_clear(valptr_vec_t vec_ptr) {
  vec_ptr->end = vec_ptr->fst = 0;
  return 1;
}

static inline int valclear(val_t a) {
  valptr_vec_t vec_ptr = val_vecptr(a);
  if (vec_ptr == NULL)  return 0;
  return val_clear(vec_ptr);
}

// Queue

#define valenq(a,e) valpush(a,e)

static inline val_t valdeq(val_t a) {
  val_t first_val = val_get_vec(a, VAL_VEC_FIRSTNDX);
  if (errno == 0) ((valptr_vec_t)valtoptr(a))->fst += 1;
  return first_val;
}

val_t val_first_or_last(val_t a, int32_t *ndx, int32_t retndx);
#define val_lf_ndx(p) _Generic((p), int: NULL, default: p)

#define vallast(a,...) val_last(a, __VA_ARGS__ +0)
#define val_last(a,p) val_first_or_last(a, val_lf_ndx(p), -1) 

#define valfirst(a,...) val_first(a, __VA_ARGS__ +0)
#define val_first(a,p) val_first_or_last(a, val_lf_ndx(p), VAL_VEC_FIRSTNDX) 

/*
valset(a,k,v)
valget(a,k)
valdel(a,k)
*/

// SORT & SEARCH
typedef int (*val_cmp_t)(const void *, const void *);

int val_veccmp(const void *a,const void *b);

#define valsort(...) VAL_vrg(val_sort_,__VA_ARGS__)
#define val_sort_1(a)      val_sort(a,val_veccmp,valnil)
#define val_sort_2(a,c)    val_sort(a,c,valnil)
#define val_sort_3(a,c,x)  val_sort(a,c,x)
int val_sort(val_t a, val_cmp_t cmp, val_t aux);

#define valbsearch(...) VAL_vrg(val_bsearch_,__VA_ARGS__)
#define val_bsearch_2(a,e)     val_bsearch(a,val(e),val_veccmp,valnil)
#define val_bsearch_3(a,e,c)   val_bsearch(a,val(e),c,valnil)
#define val_bsearch_4(a,e,c,x) val_bsearch(a,val(e),c,x)
int32_t val_bsearch(val_t a, val_t e, val_cmp_t cmp, val_t aux);

#endif // VAL_LIB

// Stores

#define VAL_NUM_STORES 1

typedef struct val_store_s {
  uint8_t *data;
  int32_t  sze;
  int32_t  end;
} val_store_t;

extern val_store_t val_stores[VAL_NUM_STORES];

// If you use threads, all these three function are mutex-protected!
#define valstore(s,...) val_store_(s,__VA_ARGS__+0)
#define val_store_(s,l) val_store(s,l,_Generic((s), char *: 1, default: 0))
val_t val_store(void *elm, int32_t len, int32_t isstring);

void *valrecall(val_t x);
void  valwipe(val_t vstr);



