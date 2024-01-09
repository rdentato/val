//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "vec.h"
#include <stdalign.h>

#ifdef DEBUG
int64_t vecallocatedmem = 0;
#define add_mem(m) (vecallocatedmem += (m))
#define sub_mem(m) (vecallocatedmem -= (m))
#else 
#define add_mem(m) ((void)0)
#define sub_mem(m) ((void)0) 
#endif

static int volatile ZERO = 0;

val_t vecnew() {
  errno = 0;
  vec_t v = aligned_alloc(4,sizeof(struct vec_s));
  
  valreturnif(v == NULL, valnil, ENOMEM);
  
  add_mem(sizeof(struct vec_s));
  memset(v,0,sizeof(struct vec_s));
  return val(v);
}

static val_t vec_free(val_t vv);

static void freevecs(val_t *v,uint32_t from, uint32_t to)
{
  if (v) for (int k=from; k<to; k++) {
    //valdbg("free: %016lX %d %d",v[k].v,k,valisownedvec(v[k]));
    if (valisownedvec(v[k])) {
      v[k] = vec_free(v[k]);
    }
  }
}

static val_t vec_free(val_t vv) 
{ 
  vec_t v;
  errno = 0;
  v = (vec_t)valtocleanpointer(vv); 
  freevecs(v->vec,v->fst,v->cnt);
  sub_mem(v->sze * sizeof(val_t));
  free(v->vec);
  sub_mem(sizeof(struct vec_s));
  free(v); 
  return valnil;
}

val_t vecfree(val_t vv) {
  errno = 0;
  
  valreturnif(!valisvec(vv) || valisownedvec(vv), vv, EINVAL);
  
  return vec_free(vv);
}


// Ensure the vector is large enough to store a value at index n
static int makeroom(vec_t v, uint32_t n)
{
  uint32_t new_sze;
  val_t   *new_vec;

  errno = 0;
  valreturnif(!v,0, EINVAL);

  val_dbg("vec Making room %p (%p)[%d]->[%d]",(void *)v,(void *)(v->vec),v->sze,n);

  // There should be at least n+1 slots:
  n += 1;

  if (n <= v->sze) return 1; // There's enough room

  valreturnif(n > VECMAXNDX,0,ERANGE); // Max number of elments in a vector reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = v->sze ? v->sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1); // ensure is even
  
  valreturnif(new_sze <= n,0,ERANGE);

  val_dbg("MKROOM: realloc(%p,%lu) [%u]->[%u]",(v->vec),new_sze * sizeof(val_t),v->sze, new_sze);
  new_vec = realloc(v->vec, new_sze * sizeof(val_t));
  val_dbg("MKROOM: got(%p,%d) [%d]",new_vec,new_sze * sizeof(val_t),new_sze);

  valreturnif(new_vec == NULL,0,ENOMEM);
  add_mem((new_sze - v->sze) * sizeof(val_t));
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

  valreturnif(!valisvec(vv),0,EINVAL);

  v = (vec_t)valtocleanpointer(vv);

  if (i == VECNONDX) i = v->cnt;
  if (l == VECNONDX) l = 1;
  valdbg("GAP : %d %d",i,l);

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
  valdbg("GAP cnt:%d i:%d l:%d n:%d",v->cnt,i,l,n);
  if (!makeroom(v, n)) return 0;
  if (i < v->cnt) {
    memmove(&(v->vec[i+l]),  &(v->vec[i]),  (v->cnt-i)*sizeof(val_t));
    memset(&(v->vec[i]),0, l * sizeof(val_t));
  }
  v->cnt = n;
  return 1;
}

val_t *vec(val_t v)
{
  valreturnif(!valisvec(v),NULL,EINVAL);
  return ((vec_t)valtocleanpointer(v))->vec;
}

int vectype_2(val_t vv,int type)
{ 
  vec_t v;
  valreturnif(!valisvec(vv),VEC_ISNULL,EINVAL);
  v = (vec_t)valtocleanpointer(vv);
  if (type >=0 ) v->typ = (type & 0x7F) | (v->typ & 0x80);
  return (v->typ & 0x7F);
}

uint32_t vecsize_2(val_t vv, uint32_t n)
{
  vec_t v;
  errno = 0;
  valreturnif(!valisvec(vv),0,EINVAL); 
  v = (vec_t)valtocleanpointer(vv);
  valreturnif(n != VECNONDX && !makeroom(v,n),0,0);
  return v->sze;
}

uint32_t veccount_2(val_t vv, uint32_t n)
{
  vec_t v;
  errno = 0;

  valreturnif(!valisvec(vv),0,EINVAL);
  
  v = (vec_t)valtocleanpointer(vv);

  if (n != VECNONDX && (n < (VECMAXNDX - v->fst)))  {
    n += v->fst;
    valreturnif(!makeroom(v, n), 0, ENOMEM);
    v->cnt = n;
  }

  return v->cnt - v->fst;
}

// ADDING VALUES TO VEC

static val_t setval(vec_t v, uint32_t i, val_t x)
{
  // I'm about to overwrite an owned vector. Free it! (unless it is the same!)
  if ((v->fst <= i && i < v->cnt) && valisownedvec(v->vec[i]))
    if (v->vec[i].v != (x.v | 1)) vec_free(v->vec[i]);
  v->vec[i] = x;
  return x;
}

val_t vecins_(val_t vv, uint32_t i, val_t x)
{
  vec_t v;

  valreturnif(!vecmakegap(vv,i,1),valerror,ENOMEM);

  v = (vec_t)valtocleanpointer(vv);
  return setval(v, i, x);
}

val_t vecset_(val_t vv, uint32_t i, val_t x)
{
  errno = 0;
  valreturnif(!valisvec(vv), valerror, EINVAL);

  vec_t v = (vec_t)valtocleanpointer(vv);

  if (i == VECNONDX) i = v->cnt;
  
  valreturnif(!makeroom(v,i), valerror, ENOMEM);

  x=setval(v, i, x);

  if (i >= v->cnt) v->cnt = i+1;
  return x;
}

val_t vecown_3_(val_t vv, uint32_t i, val_t x)
{
  if (valeq(x,valown)) x = vecget(vv,i);
  if (valisvec(x)) x.v |= 1;
  return vecset_(vv,i,x);
}

val_t vecdisown(val_t vv, uint32_t i)
{
  val_t x = vecget(vv,i);
 
  if (valisownedvec(x)) {
    //valdbg("disowned");
    x.v &= ((uint64_t)0xFFFFFFFFFFFFFFFE);
    ((vec_t)valtocleanpointer(vv))->vec[i] = x;
    return x;
  }
  else return x;
}

val_t vecpush_(val_t vv, val_t x, int own) 
{ 
  errno = 0;
  valreturnif(!valisvec(vv), valerror, EINVAL);
  vectype(vv, VEC_ISSTACK); 
  if (own) return vecown_3_(vv,VECNONDX,x);
  return vecset_(vv,VECNONDX,x); 
}

val_t vecenq_(val_t vv, val_t x,int own)
{
  vec_t v;
  errno = 0;
  valreturnif(!valisvec(vv), valerror, EINVAL);

  vectype(vv,VEC_ISQUEUE);
  v = (vec_t)valtocleanpointer(vv);
  if ((v->fst > (v->cnt / 2)) && (v->cnt >= v->sze)) {
    memmove(v->vec, &(v->vec[v->fst]), (v->cnt - v->fst) * sizeof(val_t));
    v->cnt -= v->fst;
    v->fst = 0;
  }
  
  if (own) return vecown_3_(vv,VECNONDX,x);
  return vecset_(vv,VECNONDX,x); 
}

#define vec_data(v_) ((v_)->vec)

val_t vecget_2(val_t vv, uint32_t i)
{
  vec_t v;
  errno = 0;
  val_t x;

  valreturnif(!valisvec(vv), valnil, EINVAL);
  v = (vec_t)valtocleanpointer(vv);

  if (i == VECNONDX) i = v->cnt - 1; // Get the last element
  valreturnif(i >= v->cnt, valnil, ERANGE);

  x = v->vec[i];
  return x;
}

uint32_t vecdel_3(val_t vv, uint32_t i, uint32_t j)
{
  uint32_t l;
  vec_t v;
  errno = 0;
  
  valreturnif(!valisvec(vv), 0, EINVAL);
  valreturnif(vecisempty(vv), 0, ERANGE);

  v = (vec_t)valtocleanpointer(vv);

  val_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->cnt,v->sze);
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
  val_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->cnt,v->sze);
  return(v->cnt);
}

uint32_t vecdrop_2(val_t vv, uint32_t n) {
  vec_t v;
  uint32_t cnt;
  errno = 0;

  valreturnif(!valisvec(vv), 0, EINVAL);

  cnt = veccount(vv);

  if (n != 0 && cnt != 0) {
    v = (vec_t)valtocleanpointer(vv);
    if (n >= cnt) n = cnt;

    freevecs(v->vec,v->cnt-n,v->cnt);

    v->cnt -= n;
    if (v->fst >= v->cnt) {
       v->fst = 0; v->cnt = 0; v->typ = VEC_ISVEC;
    }
    cnt = (v->cnt);
  }
  return cnt;
}

// Deque (removes the next n elements form the queue)
uint32_t vecdeq_2(val_t vv, uint32_t n) 
{
  uint32_t ret = 0;
  vec_t v;
  errno = 0;

  valreturnif(!valisvec(vv), 0, EINVAL);
  valreturnif(vectype(vv) != VEC_ISQUEUE, 0, EINVAL);
  
  ret = veccount(vv);

  if (ret > 0) {
    v = (vec_t)valtocleanpointer(vv);
    n += v->fst;
    if (n > v->cnt) n = v->cnt;

    freevecs(v->vec, v->fst, n);
    v->fst = n;
    if (v->fst >= v->cnt) {
      v->fst = 0; v->cnt = 0;
      v->typ = VEC_ISVEC;
    }
    ret = v->cnt - v->fst;
  }
  return ret;
}

uint32_t vecindex_3(val_t vv,uint32_t ndx, int32_t delta)
{
  uint32_t ret = VECERRORNDX;
  errno = 0;
  
  valreturnif(!valisvec(vv), VECERRORNDX, EINVAL);
  valreturnif(vecisempty(vv), VECERRORNDX, EINVAL);

  vec_t v = (vec_t)valtocleanpointer(vv);

  if (ndx < VECNONDX)          ret = ndx;
  else if (ndx == VECNONDX)    ret = ndx;
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


