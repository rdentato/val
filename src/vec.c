//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "vec.h"
#include <stdalign.h>

val_t vecnew() {
  errno = 0;
  vec_t v = aligned_alloc(4,sizeof(struct vec_s));
  if (v == NULL) { 
    errno = ENOMEM;
    return valnil; }
  else {
    memset(v,0,sizeof(struct vec_s));
    return val(v);
  }
}

static val_t vec_free(val_t vv);

static void freevecs(val_t *v,uint32_t from, uint32_t to)
{
  for (int k=from; k<to; k++) {
      vecdbg("free: %016lX %d %d",v[k].v,k,valisref(v[k]));
    if (valisref(v[k])) {
      vec_free(v[k]);
      v[k] = valnil;
    }
  }
}

static val_t vec_free(val_t vv) 
{ 
  vec_t v;
  errno = 0;
  v = val_torealvec(vv); 
  freevecs(v->vec,v->fst,v->cnt);
  free(v->vec);
  free(v); 
  return valnil;
}

val_t vecfree(val_t vv) {
  errno = 0;
  if (!valisvec(vv) || valisref(vv) ) { errno=EINVAL; return vv; }
  else return vec_free(vv);
}

val_t vecreffree(val_t vv) {return vec_free(vv);}

val_t vecsetfree_3(val_t vv, uint32_t i, int32_t delta) {
  vec_t v;
  val_t x;
  
  i = vecindex(vv,i,delta);

  if (i == VECERRORNDX) return valnil;

  v = val_torealvec(vv);
  x = v->vec[i];
  if (!valisvec(x)) return x;

  x = vecfree(x);

  v->vec[i] = x;
  
  return x;
}

// Ensure the vector is large enough to store a value at index n
static int makeroom(vec_t v, uint32_t n)
{
  uint32_t new_sze;
  val_t   *new_vec;

  errno = 0;
  if (!v) {errno = EINVAL; return 0;}

  vec_dbg("Making room %p (%p)[%d]->[%d]",(void *)v,(void *)(v->vec),v->sze,n);

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
  
  new_sze += (new_sze & 1); // ensure is even
  
  if (new_sze <= n) { errno = ERANGE; return 0; }

  vec_dbg("MKROOM: realloc(%p,%d) [%d]",(v->vec),new_sze * sizeof(val_t),new_sze);
  new_vec = realloc(v->vec, new_sze * sizeof(val_t));
  vec_dbg("MKROOM: got(%p,%d) [%d]",new_vec,new_sze * sizeof(val_t),new_sze);

  if (new_vec == NULL) { errno = ENOMEM; return 0; }
  
  // set the newly allocated area to 0;
  memset(&(new_vec[v->cnt]),0,(new_sze-v->sze)*sizeof(val_t));

  v->vec = new_vec;
  v->sze = new_sze;
  
  return 1;
}

int vec_gap_3(val_t vv, uint32_t i, uint32_t l)
{
  vec_t v;
  int n;
  errno = 0;
  if (!valisvec(vv)) {errno = EINVAL; return 0;}
  v = val_torealvec(vv);

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
  if (i < v->cnt) {
    memmove(&(v->vec[i+l]),  &(v->vec[i]),  (v->cnt-i)*sizeof(val_t));
    memset(&(v->vec[i]),0, l * sizeof(val_t));
  }
  v->cnt = n;
  return 1;
}

static val_t setval(vec_t v, uint32_t i, val_t x)
{
  val_t ret = x;
  if (valeq(x, valnewvec)) {
    x = vecnew();
    ret = x;
    x.v |= 1; // owned
    vecdbg("Set %p[%d] at %016lX",(void *)v,i,x.v);
  }
  if (valisref(v->vec[i])) vec_free(v->vec[i]);
  v->vec[i] = x;
  return ret;
}

uint32_t vecins_(val_t vv, uint32_t i, val_t x)
{
  vec_t v;
  if (vecmakegap(vv,i,1)) {
    v = val_torealvec(vv);
    x = setval(v, i, x);
    return v->sze;
  }
  return VECNONDX; 
}

val_t *vec(val_t v)
{
  if (!valisvec(v)) return NULL;
  return val_torealvec(v)->vec;
}

int vectype_2(val_t vv,int type)
{ 
  vec_t v;
  if (!valisvec(vv)) return VEC_ISNULL;
  v = val_torealvec(vv);
  if (type >=0 ) v->typ = (type & 0x7F) | (v->typ & 0x80);
  return (v->typ & 0x7F);
}

uint32_t vecsize_2(val_t vv, uint32_t n)
{
  vec_t v;
  errno = 0;
  if (!valisvec(vv)) {errno = EINVAL; return 0;}
  v = val_torealvec(vv);
  if (n != VECNONDX && !makeroom(v,n)) return 0;
  return v->sze;
}

uint32_t veccount_2(val_t vv, uint32_t n)
{
  uint32_t fst = 0;
  vec_t v;
  errno = 0;

  if (!valisvec(vv)) {errno = EINVAL; return 0;}
  v = val_torealvec(vv);
  if (vectype(vv) == VEC_ISQUEUE) fst = v->fst;

  if (n != VECNONDX)  {
    n += fst;
    if (!makeroom(v, n)) return 0;
    v->cnt = n;
  }

  return v->cnt - fst;
}

val_t vecset_(val_t vv, uint32_t i, val_t x)
{
  errno = 0;
  if (!valisvec(vv)) { 
    errno=EINVAL; 
    return valerror; 
  }

  vec_t v = val_torealvec(vv);

  if (i == VECNONDX) i = v->cnt;
  
  if (!makeroom(v,i)) 
    return valerror;

  x=setval(v, i, x);

  if (i >= v->cnt) v->cnt = i+1;
  return x;
}

#define vec_data(v_) ((v_)->vec)

val_t vecget_3(val_t vv, uint32_t i, int32_t delta)
{
  vec_t v;
  errno = 0;
  val_t x;

  i = vecindex(vv, i, delta);
  if (i == VECERRORNDX) 
    return valnil;

  v = val_torealvec(vv);
  if (i == VECNONDX) i = v->cnt - 1; // Get the last element
  if (i >= v->cnt) {errno = ERANGE; return valnil; }
  x = v->vec[i];
  return x;
}

uint32_t vecdel_3(val_t vv, uint32_t i, uint32_t j)
{
  uint32_t l;
  vec_t v;
  errno = 0;
  
  if (!valisvec(vv)) {errno = EINVAL; return VECERRORNDX; }
  if (vecisempty(vv)) return 0;

  v = val_torealvec(vv);

  vec_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->cnt,v->sze);
  if (i == VECNONDX) { i = v->cnt-1; j = i; }
  else if (j == VECNONDX) j = i;
  if (i >= v->cnt || j<i) return 0;

  if (j >= v->cnt-1) {
    freevecs(v->vec,i,v->cnt);
    v->cnt = i; // Just drop last elements
  }
  else {
    freevecs(v->vec,i,j+1);

    //       __l__
    //    ABCdefghIJKLM              ABCIJKLM
    //    |  |    |    |             |  |    |    
    //    0  i   i+l   cnt           0  i    cnt-l  

    l = (j-i)+1;
    memmove(&(v->vec[i]) , &(v->vec[i+l]),  (v->cnt-(i+l)) * sizeof(val_t));
    v->cnt -= l;
  }
  vec_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->cnt,v->sze);
  return(v->cnt);
}

uint32_t vecdrop_2(val_t vv, uint32_t n) {
  vec_t v;
  uint32_t cnt;
  errno = 0;
  if (!valisvec(vv)) {errno = EINVAL; return 0; }
  cnt = veccount(vv);

  if (n == 0 || cnt == 0) return cnt;

  v = val_torealvec(vv);
  if (n >= cnt) n = cnt;

  freevecs(v->vec,v->cnt-1,v->cnt-n+1);

  v->cnt -= n;
  if (v->fst >= v->cnt) {
     v->fst = 0; v->cnt = 0; v->typ = VEC_ISVEC;
  }
  return(v->cnt);
}

val_t vecpush_(val_t vv, val_t x) 
{ 
  errno = 0;
  if (!valisvec(vv)) {(errno=EINVAL); return valerror; }
  vectype(vv, VEC_ISSTACK); 
  return vecset_(vv,VECNONDX,x); 
}

val_t vecenq_(val_t vv, val_t x)
{
  vec_t v;
  errno = 0;
  if (!valisvec(vv)) {errno = EINVAL; return valerror; }
  vectype(vv,VEC_ISQUEUE);
  v = val_torealvec(vv);
  if ((v->fst > (v->cnt / 2)) && (v->cnt >= v->sze)) {
    memmove(v->vec, &(v->vec[v->fst]), (v->cnt - v->fst) * sizeof(val_t));
    v->cnt -= v->fst;
    v->fst = 0;
  }
  return vecset_(vv,VECNONDX,x); 
}

// Deque (removes the next n elements form the queue)
uint32_t vecdeq_2(val_t vv, uint32_t n) 
{
  uint32_t x = 0;
  vec_t v;
  errno = 0;

  if (!valisvec(vv)) {errno = EINVAL; return 0; }
  if (veccount(vv) == 0) return 0;
  if (vectype(vv) != VEC_ISQUEUE) { errno = EINVAL; return 0; }
  v = val_torealvec(vv);
  if (v->cnt < (v->fst + n) ) {
    freevecs(v->vec,0,v->cnt);
    v->fst = v->cnt = 0;
    v->typ = VEC_ISVEC;
  }
  else {
    freevecs(v->vec,v->fst,v->fst+n);

    v->fst += n;
    if (v->fst >= v->cnt) {
      v->fst = 0; v->cnt = 0;
      v->typ = VEC_ISVEC;
    }
    x = v->cnt - v->fst;
  }
  return x;
}

uint32_t vecindex_3(val_t vv,uint32_t ndx, int32_t delta)
{
  uint32_t ret = VECERRORNDX;
  errno = 0;
  if (!valisvec(vv)) {errno = EINVAL; return VECERRORNDX; }

  if (vecisempty(vv)) return VECERRORNDX;

  vec_t v = val_torealvec(vv);

  if (ndx <= VECNONDX)         ret = ndx;
  else if (ndx == VECCOUNTNDX) ret = v->cnt;
  else if (ndx == VECSIZENDX)  ret = v->sze;
  else switch(vectype(vv)) {
    case VEC_ISSTACK: switch(ndx) {
                        case VECNEXTNDX:
                        case VECFIRSTNDX:
                        case VECTOPNDX:  ret = v->cnt-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ret) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = 0; 
                                         if (delta < 0) delta = -delta;
                                         if (delta > v->cnt-1) ret = VECERRORNDX;
                                         else ret += delta;
                                         break;
    } 
    break;

    case VEC_ISQUEUE: switch(ndx) {
                        case VECNEXTNDX:
                        case VECFIRSTNDX:
                        case VECTOPNDX:
                        case VECHEADNDX: ret = v->fst;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ((v->cnt-1) - v->fst)) ret = VECERRORNDX;
                                         else ret += delta;
                                         break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = v->cnt-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ((v->cnt-1) - v->fst)) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;
    } 
    break;

    default: switch(ndx) {
                        case VECNEXTNDX:  ret = v->cnt;  break;

                        case VECHEADNDX:  
                        case VECFIRSTNDX: ret = 0;  break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = v->cnt-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ret) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;

                        case VECCURNDX:  ret = v->cur; break;
    }
  }

  return ret;
}

val_t vecref(val_t v)
{
  if (!valisvec(v)) return v;
  v.v |= 1;
  return v;
}

val_t vecunref_1(val_t v)
{
  if (!valisvec(v)) return v;
  v.v &= 0xFFFFFFFFFFFFFFFE;
  return v;
}

val_t vecunref_2(val_t v, uint32_t i)
{
  if (!valisvec(v)) return v;

  val_t x = vecget(v,i);
  if (valisvec(x))  x.v &= 0xFFFFFFFFFFFFFFFE; 

  return x;
}

val_t vecown_1_(val_t *vp)
{
  return (*vp = vecref(*vp));
}

val_t vecown_3(val_t vv, uint32_t i, int32_t delta)
{
  vec_t v = val_torealvec(vv);
  if (!valisvec(v)) return valnil;
  i = vecindex_3(v,i,delta);
  if (i == VECERRORNDX) return valnil;
  if (!valisvec(v->vec[i])) return valnil;

  v->vec[i].v != 1;
  return v->vec[i];
}

val_t vecdisown_1_(val_t *vp)
{
  return (*vp = vecunref(*vp));
}

val_t vecdisown_3(val_t v, uint32_t i, int32_t delta)
{
  
}