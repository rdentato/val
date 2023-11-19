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
    free(((val_t *)(v->blk.dta)));
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

  vec_dbg("Making room %p (%p)[%d]->[%d]",(void *)v,(void *)((val_t *)(v->blk.dta)),v->blk.sze,n);

  // There should be at least n+1 slots:
  n += 1;

  if (n <= v->blk.sze) return 1; // There's enough room

  if (n > VECMAXNDX) {errno = ERANGE; return 0;}; // Max number of elments in a vector reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = v->blk.sze ? v->blk.sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1);
  
  if (new_sze <= n) { errno = ERANGE; return 0; }

  vec_dbg("MKROOM: realloc(%p,%d) [%d]",((val_t *)(v->blk.dta)),new_sze * sizeof(val_t),new_sze);
  new_vec = realloc(((val_t *)(v->blk.dta)), new_sze * sizeof(val_t));
  assert(((uintptr_t)new_vec & 1) == 0); // assume that all malloced pointers are at least even-aligned
  vec_dbg("MKROOM: got(%p,%d) [%d]",new_vec,new_sze * sizeof(val_t),new_sze);

  if (new_vec == NULL) { errno = ENOMEM; return 0; }
  
  v->blk.dta = new_vec;
  v->blk.sze = new_sze;
  
  return 1;
}


int vec_gap_3(val_t vv, uint32_t i, uint32_t l)
{
  vec_t v;
  int n;

  if (!valisvec(vv)) {errno = EINVAL; return 0;}
  v = valtovec(vv);

  if (i == VECNONDX) i = v->blk.cnt;
  if (l == VECNONDX) l = 1;
  vecdbg("GAP : %d %d",i,l);

  if (i < v->blk.cnt) {
    n = v->blk.cnt+l;
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
  vecdbg("GAP cnt:%d i:%d l:%d n:%d",v->blk.cnt,i,l,n);
  if (!makeroom(v, n)) return 0;
  if (i < v->blk.cnt)  memmove(&((val_t *)(v->blk.dta))[i+l],  &((val_t *)(v->blk.dta))[i],  (v->blk.cnt-i)*sizeof(val_t));
  v->blk.cnt = n;
  return 1;
}

uint32_t vecins_(val_t vv, uint32_t i, val_t x)
{
  vec_t v;
  if (vecmakegap(vv,i,1)) {
    v = valtovec(vv);
    ((val_t *)(v->blk.dta))[i] = x;
    return v->blk.sze;
  }
  return VECNONDX; 
}

val_t *vec(val_t v)
{
  if (!valisvec(v)) return NULL;
  return valtovec(v)->blk.dta;
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
  return v->blk.sze;
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
    v->blk.cnt = n;
  }

  return v->blk.cnt - fst;
}

uint32_t vecset_(val_t vv, uint32_t i, val_t x)
{
  if (!valisvec(vv)) return ((errno=EINVAL),VECNONDX);
  vec_t v = valtovec(vv);
  if (i == VECNONDX) i = v->blk.cnt;
  if (!makeroom(v,i)) return VECNONDX;
  ((val_t *)(v->blk.dta))[i] = x;
  if (i >= v->blk.cnt) v->blk.cnt = i +1;
  return i;
}

#define vec_data(v_) ((val_t *)((v_)->blk.dta))

val_t vecget(val_t vv, uint32_t i)
{
  vec_t v;
  i = vecindex(vv, i);
  if (i == VECERRORNDX) return valnil;

  v = valtovec(vv);
  if (i == VECNONDX) i = v->blk.cnt - 1; // Get the last element
  if (i >= v->blk.cnt) {errno = ERANGE; return valnil; }
  return vec_data(v)[i];
}

val_t vecdel_3(val_t vv, uint32_t i, uint32_t j)
{
  uint32_t l;
  vec_t v;
  if (!valisvec(vv) || (v = valtovec(vv))->blk.cnt == 0) {errno = EINVAL; return valnil; }
  vec_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->blk.cnt,v->blk.sze);
  if (i == VECNONDX) { i = v->blk.cnt-1; j = i; }
  else if (j == VECNONDX) j = i;
  if (i >= v->blk.cnt || j<i) { errno = EINVAL; return valnil; }
  if (j >= v->blk.cnt) {j = v->blk.cnt-1;}
  if (j+1 >= v->blk.cnt) {
    v->blk.cnt = i; // Just drop last elements
  }
  else {
    l = (j-i)+1;
    // ```
    //       __l__                                
    //    ABCdefghIJKLM              ABCIJKLM     
    //    |  |    |    |             |  |    |    
    //    0  i   i+l   cnt           0  i    cnt-l  
    // ```
    memmove(&((val_t *)(v->blk.dta))[i] , &((val_t *)(v->blk.dta))[i+l],  (v->blk.cnt-(i+l)) * sizeof(val_t));
    v->blk.cnt -= l;
  }
  vec_dbg("i = %d, j = %d, cnt = %d, sze = %d",i,j,v->blk.cnt,v->blk.sze);
  return(v->blk.cnt? ((val_t *)(v->blk.dta))[v->blk.cnt - 1] : valnil);
}

val_t vecdrop_2(val_t vv, uint32_t n) {
  vec_t v;
  uint32_t cnt;
  if (!valisvec(vv)) {errno = EINVAL; return valnil; }
  v = valtovec(vv);
  cnt = veccount(vv);
  if (n == 0 || cnt == 0) return ((errno=ERANGE),valnil);
  if (n >= cnt) v->blk.cnt = 0;
  else v->blk.cnt -= n;
  return(((val_t *)(v->blk.dta))[v->blk.cnt]);
}

uint32_t vecpush_(val_t vv, val_t x) 
{ 
    if (!valisvec(vv)) {(errno=EINVAL); return VECNONDX; }
    vectype(vv, VEC_ISSTACK); 
    return vecadd(vv,x); 
}

uint32_t vecenq_(val_t vv, val_t x)
{
  vec_t v;
  if (!valisvec(vv)) {errno = EINVAL; return VECNONDX; }
  vectype(vv,VEC_ISQUEUE);
  v = valtovec(vv);
  if ((v->fst > (v->blk.cnt / 2)) && (v->blk.cnt >= v->blk.sze)) {
    memmove(((val_t *)(v->blk.dta)), &((val_t *)(v->blk.dta))[v->fst] , (v->blk.cnt - v->fst) * sizeof(val_t));
    v->blk.cnt -= v->fst;
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
  if (v->blk.cnt < (v->fst + n) ) {
    v->fst = v->blk.cnt = 0; v->typ = VEC_ISVEC;
    x = valnil;
  }
  else {
    v->fst += n;
    x = ((val_t *)(v->blk.dta))[v->fst-1];
    if (v->fst >= v->blk.cnt) {
      v->fst = 0; v->blk.cnt = 0; v->typ = VEC_ISVEC;
    }
  }
  return x;
}

val_t vechead(val_t vv)
{
  return vecget(vv,VECHEADNDX);

//  vec_t v;
//  if (!valisvec(vv)) { errno = EINVAL; return valnil; }
//  v = valtovec(vv);
//  if (v->typ != VEC_ISQUEUE) { errno = EINVAL; return valnil; }
//  if (v->blk.cnt <= v->fst) {return valnil;}
//  return ((val_t *)(v->blk.dta))[v->fst];   
}

val_t vectail(val_t vv)
{
  vec_t v;
  if (!valisvec(vv)) {errno = EINVAL; return valnil; }
  v = valtovec(vv);
  if (v->typ != VEC_ISQUEUE) { errno = EINVAL; return valnil; }
  if (v->blk.cnt <= v->fst) {return valnil;}
  return ((val_t *)(v->blk.dta))[v->blk.cnt-1];   
}

// val_t vecfirst(vec_t v)
// {
//   uint32_t n;
//   n = veccount(v);
//   if (n == 0) {return valnil;}
//   if (v->typ == VEC_ISQUEUE) return ((val_t *)(v->blk.dta))[v->fst];
//   return ((val_t *)(v->blk.dta))[n-1];   
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

val_owner_t vecowner(val_t vv) 
{
  val_owner_t vo;
  vec_t v;
  vo.v = vv;
  if (!valisvec(vv)) {errno=EINVAL; return  vo;}
  v = valtovec(vv);
  if (v->typ & VEC_ISOWNED) {errno = ERANGE; return vo;}
  v->typ |= VEC_ISOWNED;
  vo.v.v |= 1;
  return vo;
}

uint32_t vecfirst(val_t v);
uint32_t veclast(val_t v);
uint32_t vecnext(val_t v);
uint32_t vecprev(val_t v);
uint32_t vec_cur_2(val_t v, uint32_t i);

uint32_t vecindex(val_t vv,uint32_t ndx)
{
  errno = 0;
  if (!valisvec(vv)) {errno = EINVAL; return VECERRORNDX; }

  vec_t v = valtovec(vv);

  if (ndx <= VECNONDX)    return ndx;
  if (ndx == VECCOUNTNDX) return v->blk.cnt;
  if (ndx == VECSIZENDX)  return v->blk.sze;

  if (vecisempty(vv)) return VECERRORNDX;
  
  switch(vectype(vv)) {
    case VEC_ISSTACK: switch(ndx) {
                        case VECNEXTNDX:
                        case VECFIRSTNDX:
                        case VECTOPNDX:  return v->blk.cnt-1;
                                         break;

                        case VECLASTNDX: return 0;
                                         break;
    } 
    break;

    case VEC_ISQUEUE: switch(ndx) {
                        case VECNEXTNDX:
                        case VECFIRSTNDX:
                        case VECHEADNDX: return v->fst;
                                         break;

                        case VECTAILNDX:
                        case VECLASTNDX: return v->blk.cnt-1;
                                         break;
    } 
    break;

    default: switch(ndx) {
                        case VECNEXTNDX:  return v->blk.cnt;

                        case VECHEADNDX:  
                        case VECFIRSTNDX: return 0;
                                          break;

                        case VECTAILNDX:
                        case VECLASTNDX: return v->blk.cnt-1;
                                         break;

                        case VECCURNDX:  return v->cur;
                                         break;
    }
  }

  return VECERRORNDX;
}
