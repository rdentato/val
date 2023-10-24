//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "vec.h"

int vecisowned(val_t vv)
{
  if (!valisvec(vv)) return 0;
  return ((valtovec(vv)->typ & 0x80) == 0x80);
}

int valisvecref(val_t vv)
{
  return (vecisowned(vv)) ? (vv.v & 1) : 0;
}

val_t vecfree(val_t vv) { 
  vec_t v;
  errno = 0;
  if (valisvecref(vv)) { 
    v = valtovec(vv); 
    free(v->vec);
    free(v); 
  } else errno = EINVAL;
  return valnil;
}

// Ensure the vector is large enough to store a value at index n
static int makeroom(vec_t v, uint32_t n)
{
  uint32_t new_sze;
  val_t   *new_vec;

  errno = 0;
  if (!v) {errno = EINVAL; return 0;}

  vec_dbg("Making room %p (%p)[%d]->[%d]",(void *)v,(void *)v->vec,v->sze,n);

  // There should be at least n+1 slots:
  n += 1;

  if (n <= v->sze) return 1; // There's enough room

  if (n > VECMAXNDX) {errno = ERANGE; return 0;}; // Max number of elments in a vector reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = v->sze ? v->sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1);
  
  if (new_sze <= n) { errno = ERANGE; return 0; }

  vec_dbg("MKROOM: realloc(%p,%d) [%d]",v->vec,new_sze * sizeof(val_t),new_sze);
  new_vec = realloc(v->vec, new_sze * sizeof(val_t));
  assert(((uintptr_t)new_vec & 1) == 0); // assume that all malloced pointers are at least even-aligned
  vec_dbg("MKROOM: got(%p,%d) [%d]",new_vec,new_sze * sizeof(val_t),new_sze);

  if (new_vec == NULL) { errno = ENOMEM; return 0; }
  
  v->vec = new_vec;
  v->sze = new_sze;
  
  return 1;
}


int vec_gap_3(val_t vv, uint32_t i, uint32_t l)
{
  vec_t v;
  int n;

  if (!valisvec(vv)) {errno = EINVAL; return 0;}
  v = valtovec(vv);

  if (i == VECNONDX) i = v->cnt;
  if (l == VECNONDX) l = 1;
  vecdbg("GAP : %d %d",i,l);

  if (i < v->cnt) {
    n = v->cnt+l;
    /*                    __l__
    **  ABCDEFGH       ABC-----DEFGH
    **  |  |    |      |  |    |    |
    **  0  i    cnt    0  i   i+l   cnt+l
    */
  }
  else {
    n = i+l;
    /*                                           __l__
    **  ABCDEFGH........          ABCDEFGH............
    **  |       |     |           |             |     |
    **  0       cnt   i           0             i     i+l
    */
  }
  vecdbg("GAP cnt:%d i:%d l:%d n:%d",v->cnt,i,l,n);
  if (!makeroom(v, n)) return 0;
  if (i < v->cnt)  memmove(&v->vec[i+l],  &v->vec[i],  (v->cnt-i)*sizeof(val_t));
  v->cnt = n;
  return 1;
}

val_t *vec(val_t v)
{
  if (!valisvec(v)) return NULL;
  return valtovec(v)->vec;
}

int vectype_2(val_t vv,int type)
{ 
  vec_t v;
  if (!valisvec(vv)) return VEC_ISNULL;
  v = valtovec(vv);
  if (type >=0 ) v->typ = (type & 0x7F) | (v->typ & 0x80);
  return (v->typ & 0x7F);
}

uint32_t vecsize_2(val_t vv, uint32_t n)
{
  vec_t v;
  if (!valisvec(vv)) {errno = EINVAL; return 0;}
  v = valtovec(vv);
  if (n != VECNONDX && !makeroom(v,n)) return 0;
  return v->sze;
}

uint32_t veccount_2(val_t vv, uint32_t n)
{
  uint32_t fst = 0;
  vec_t v;

  if (!valisvec(vv)) {errno = EINVAL; return 0;}
  v = valtovec(vv);
  if (vectype(vv) == VEC_ISQUEUE) fst = v->fst;

  if (n != VECNONDX)  {
    n += fst;
    if (!makeroom(v, n)) return 0;
    v->cnt = n;
  }

  return v->cnt - fst;
}

uint32_t vecset_(val_t vv, uint32_t i, val_t x)
{
  if (!valisvec(vv)) return ((errno=EINVAL),VECNONDX);
  vec_t v = valtovec(vv);
  if (i == VECNONDX) i = v->cnt;
  if (!makeroom(v,i)) return VECNONDX;
  v->vec[i] = x;
  if (i >= v->cnt) v->cnt = i +1;
  return i;
}

val_t vecget(val_t vv, uint32_t i)
{
  vec_t v;
  if (!valisvec(vv) || (v = valtovec(vv))->cnt == 0) {errno = EINVAL; return valnil; }
  if (i == VECNONDX) i = v->cnt - 1; // Get the last element
  if (i >= v->cnt) {errno = ERANGE; return valnil; }
  return v->vec[i];
}

val_t vecdel_3(val_t vv, uint32_t i, uint32_t j)
{
  uint32_t l;
  vec_t v;
  if (!valisvec(vv) || (v = valtovec(vv))->cnt == 0) {errno = EINVAL; return valnil; }
  vec_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->cnt,v->sze);
  if (i == VECNONDX) { i = v->cnt-1; j = i; }
  else if (j == VECNONDX) j = i;
  if (i >= v->cnt || j<i) { errno = EINVAL; return valnil; }
  if (j >= v->cnt) {j = v->cnt-1;}
  if (j+1 >= v->cnt) {
    v->cnt = i; // Just drop last elements
  }
  else {
    l = (j-i)+1;
    // ```
    //       __l__                                
    //    ABCdefghIJKLM              ABCIJKLM     
    //    |  |    |    |             |  |    |    
    //    0  i   i+l   cnt           0  i    cnt-l  
    // ```
    memmove(&v->vec[i] , &v->vec[i+l],  (v->cnt-(i+l)) * sizeof(val_t));
    v->cnt -= l;
  }
  vec_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->cnt,v->sze);
  return(v->cnt? v->vec[v->cnt - 1] : valnil);
}

val_t vecdrop_2(val_t vv, uint32_t n) {
  vec_t v;
  uint32_t cnt;
  if (!valisvec(vv)) {errno = EINVAL; return valnil; }
  v = valtovec(vv);
  cnt = veccount(vv);
  if (n == 0 || cnt == 0) return ((errno=ERANGE),valnil);
  if (n >= cnt) v->cnt = 0;
  else v->cnt -= n;
  return(v->vec[v->cnt]);
}

uint32_t vecpush_(val_t vv, val_t x) 
{ 
    if (!valisvec(vv)) return ((errno=EINVAL),VECNONDX);
    vectype(vv, VEC_ISSTACK); 
    return vecadd(vv,x); 
}

uint32_t vecenq_(val_t vv, val_t x)
{
  vec_t v;
  if (!valisvec(vv)) {errno = EINVAL; return VECNONDX; }
  vectype(vv,VEC_ISQUEUE);
  v = valtovec(vv);
  if ((v->fst > (v->cnt / 2)) && (v->cnt >= v->sze)) {
    memmove(v->vec, &v->vec[v->fst] , (v->cnt - v->fst) * sizeof(val_t));
    v->cnt -= v->fst;
    v->fst = 0;
  }
  return vecadd(vv,x); 
}

// Deque (get the next element and removes it from the que)
val_t vecdeq_2(val_t vv, uint32_t n) 
{
  val_t x;
  vec_t v;
  if (!valisvec(vv)) {errno = EINVAL; return valnil; }
  if (veccount(vv) == 0) return valnil;
  if (vectype(vv) != VEC_ISQUEUE) { errno = EINVAL; return valnil; }
  v = valtovec(vv);
  if (v->cnt < (v->fst + n) ) {
    v->fst = v->cnt = 0; v->typ = VEC_ISVEC;
    x = valnil;
  }
  else {
    v->fst += n;
    x = v->vec[v->fst-1];
    if (v->fst >= v->cnt) {
      v->fst = 0; v->cnt = 0; v->typ = VEC_ISVEC;
    }
  }
  return x;
}

val_t vechead(val_t vv)
{
  vec_t v;
  if (!valisvec(vv)) { errno = EINVAL; return valnil; }
  v = valtovec(vv);
  if (v->typ != VEC_ISQUEUE) { errno = EINVAL; return valnil; }
  if (v->cnt <= v->fst) {return valnil;}
  return v->vec[v->fst];   
}

val_t vectail(val_t vv)
{
  vec_t v;
  if (!valisvec(vv)) {errno = EINVAL; return valnil; }
  v = valtovec(vv);
  if (v->typ != VEC_ISQUEUE) { errno = EINVAL; return valnil; }
  if (v->cnt <= v->fst) {return valnil;}
  return v->vec[v->cnt-1];   
}

// val_t vecfirst(vec_t v)
// {
//   uint32_t n;
//   n = veccount(v);
//   if (n == 0) {return valnil;}
//   if (v->typ == VEC_ISQUEUE) return v->vec[v->fst];
//   return v->vec[n-1];   
// }

val_t vecown(val_t vv)
{
  vec_t v;
  if (!valisvec(vv)) { errno=EINVAL; return  valnil; }
  v = valtovec(vv);
  if (v->typ & VEC_ISOWNED) {errno = ERANGE; return vv; }
  v->typ |= VEC_ISOWNED;
  vv.v |= 1;
  return vv;
}


uint32_t vecfirst(val_t v);
uint32_t veclast(val_t v);
uint32_t vecnext(val_t v);
uint32_t vecprev(val_t v);
uint32_t vec_cur_2(val_t v, uint32_t i);

uint32_t vecindex(val_t v,uint32_t ndx)
{

  
}
