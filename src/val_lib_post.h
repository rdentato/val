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
};

#define VAL_VEC_EMPTY 0x0000
#define VAL_VEC_MAP   0x0001
#define VAL_VEC_ARRAY 0x0002
#define VAL_VEC_STACK 0x0004
#define VAL_VEC_QUEUE 0x0008

#define VAL_VEC_MAXNDX   ((int32_t)0x1FFFFFF0)
#define VAL_VEC_NONDX    ((int32_t)-1)
#define VAL_VEC_TOPNDX   ((int32_t)-2)
#define VAL_VEC_FIRSTNDX ((int32_t)-3)

#define valvec(...) val_vec(__VA_ARGS__ +0)
val_t val_vec(int32_t n);
val_t valfree(val_t a);

static inline int32_t valcount(val_t a) { 
  if (!valisvecptr(a)) {errno = EINVAL; return 0;}
  valptr_vec_t vec_ptr = valtoptr(a);
  int32_t cnt = vec_ptr->end - vec_ptr->fst;
  // fprintf(stderr,">>> cnt: %d (%d >> %d)\n",cnt,(vec_ptr->end - vec_ptr->fst) , (vec_ptr->typ & VAL_VEC_MAP));
  return cnt;
}

static inline int32_t valsize(val_t a) {
  if (!valisvecptr(a)) {errno = EINVAL; return 0; }
  return ((valptr_vec_t)valtoptr(a))->sze;
}

int val_set_vec(val_t a, int32_t ndx, val_t elm);
int val_set_map(val_t a, val_t key, val_t elm);

#define valset(a,i,v) _Generic((i), \
                           char *: val_set_chrptr_ndx, \
                            val_t: val_set_map, \
                          default: val_set_vec) (a,i,val(v))

static inline int val_set_chrptr_ndx(val_t a, char *ndx, val_t v) {
  return val_set_map(a, val(ndx), v);
}

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

#define val_del_1(a)      val_del_vec(a,VAL_VEC_TOPNDX,VAL_VEC_NONDX)
#define val_del_3(a,f,t)  val_del_vec(a,f,t)
#define val_del_2(a,ndx)  _Generic((ndx), \
                            char *: val_del_chrptr_ndx, \
                             val_t: val_del_map, \
                           default: val_del_int_ndx)(a, ndx)

static inline int val_del_chrptr_ndx(val_t a, char *ndx) {
  return val_del_map(a, val(ndx));
}

static inline int val_del_int_ndx(val_t a, int32_t ndx) {
  return val_del_vec(a,ndx,ndx);
}

// STACK

#define valpush(a,e)  val_set_vec(a, VAL_VEC_TOPNDX, val(e))

#define valtop(a) val_get_vec(a, VAL_VEC_TOPNDX);

#define valisempty(a)  (valcount(a) == 0)

static inline int val_clear(valptr_vec_t vec_ptr) {
  vec_ptr->end = vec_ptr->fst = 0;
  errno = 0;
  return 1;
}

static inline int valclear(val_t a) {
  valptr_vec_t vec_ptr;
  if (!valisvecptr(a)) {errno = EINVAL; return 0;}
  if ((vec_ptr = valtoptr(a)) == NULL) {errno = ERANGE; return 1;}
  return val_clear(vec_ptr);
}

/*

valset(a,i,v)
valget(a,i)
valins(a,i,v)
valdel(a[,i[,j]])

valdel(a)
valpush(a,v)
valtop(a)

valenq(a,v)
valdeq(a)
valdel(a)
valfirst(a)
vallast(a)

valset(a,k,v)
valget(a,k)
valdel(a,k)

val_t myvec = valvec(100);

valset(myvec,30,"pippo")

valfree(myvec);
*/

#endif


