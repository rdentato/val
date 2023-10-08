//  (C) by Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.4.0 Beta

#ifndef VEC_VERSION
#define VEC_VERSION 0x0004000B

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>

#include "val.h"

#define VEC_ISVEC   0
#define VEC_ISARRAY 1
#define VEC_ISQUEUE 2
#define VEC_ISSTACK 3
#define VEC_ISLIST  4
#define VEC_ISMAP   5

#define VECNONDX  0xFFFFFFFF
#define VECMAXNDX 0xFFFFFFF0

#define VEC_cnt(x1,x2,x3,x4,xN, ...) xN
#define VEC_n(...)       VEC_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define VEC_join(x ,y)   x ## y
#define VEC_cat(x, y)    VEC_join(x, y)
#define VEC_vrg(f, ...)  VEC_cat(f, VEC_n(__VA_ARGS__))(__VA_ARGS__)

typedef struct vec_s {
  val_t   *vec;  // Pointer to the array containing values
  uint32_t sze;  // How many values the array can contain
  uint32_t cnt;  // How many values are in the vector
  uint32_t fst;  // Pointer to the first element
  uint32_t lst;  // Pointer to the last  element
  uint32_t cur;  // Pointer to the current element
  uint16_t flg;  // Flags
  uint8_t  typ;  // Type of structure
  uint8_t  off;  // offset for extra elements
} *vec_t;

   vec_t vecnew();
   vec_t vecfree(vec_t v);
uint32_t veccount(vec_t v);
uint32_t vecclear(vec_t v);

#define vecadd(v,x) vec_set(v,VECNONDX,x)

#define vecset(v,i,x) vec_set(v,i,val(x))
uint32_t vec_set(vec_t v, uint32_t i, val_t x);

#define vecget(...) vec_get(vec_get_,__VA_ARGS__)
#define vec_get_1(v) vec_get_2(v,VECNONDX)
   val_t vec_get_2(vec_t v, uint32_t i);

   val_t vecdel(vec_t v, uint32_t i);

#define vecins(v,i,x) vec_ins(v,i,val(x))
uint32_t vec_ins(vec_t v, uint32_t i, val_t x);

#define vecpush(v,i,x) vec_push(v,i,val(x))
uint32_t vec_push(vec_t v, val_t x);
   val_t vectop(vec_t v);
   val_t vecpop(vec_t v);
   
   val_t vecdrop(vec_t v);

#define vecenq(v,x) vec_enq(v,val(x))
uint32_t vec_enq(vec_t v, val_t x);
   val_t vecdeq(vec_t v);

   val_t vechead(vec_t v);
   val_t vectail(vec_t v);

uint32_t vecfirst(vec_t v);
uint32_t veclast(vec_t v);
uint32_t vecnext(vec_t v);
uint32_t vecprev(vec_t v);

#define veccur(...) VEC_vrg(vec_cur_,__VA_ARGS__)
#define vec_cur_1(v) vec_cur_2(v,VECNONDX)
uint32_t vec_cur_2(vec_t v, uint32_t i);

uint32_t vecsearch(vec_t v, val_t x);
uint32_t vecsort(vec_t v, int (*cmp)(val_t a, val_t b, val_t aux), val_t aux);

#ifndef NDEBUG
  #define vecdbg(...) (fprintf(stderr,"INFO: "),fprintf(stderr, __VA_ARGS__),fprintf(stderr," \xF%s:%d\n",__FILE__,__LINE__))
#else
  #define vecdbg(...)
#endif
#define _vecdbg(...)


#ifdef VEC_MAIN
#include "vec.c"
#endif

#endif
