
//  (C) by Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.4.0 Beta

#include "vec.h"

vec_t vecnew()
{
  vec_t v;
  v = calloc(1,sizeof(struct vec_s));
  if (v == NULL) errno = ENOMEM;
  return v;
}

vec_t vecfree(vec_t v)
{
  free(v);
  return NULL;
}

// Ensure the vector is large enough to store a value at index n
static int makeroom(vec_t v, uint32_t n)
{
  uint32_t new_sze;
  val_t   *new_vec;

  errno = 0;
  if (!v) {errno = EINVAL; return 0;}

 _vecdbg("Making room %p (%p)[%d]->[%d]",(void *)v,(void *)v->vec,v->sze,n);

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

 _vecdbg("MKROOM: realloc(%p,%d) [%d]",v->vec,new_sze * sizeof(val_t),new_sze);
  new_vec = realloc(v->vec, new_sze * sizeof(val_t));
 _vecdbg("MKROOM: got(%p,%d) [%d]",new_vec,new_sze * sizeof(val_t),new_sze);

  if (new_vec == NULL) { errno = ENOMEM; return 0; }
  
  v->vec = new_vec;
  v->sze = new_sze;
  
  return 1;
}


uint32_t vec_set(vec_t v, uint32_t i, val_t x)
{
  if (i == VECNONDX) i = v->cnt;
  if (!makeroom(v,i)) return VECNONDX;
  v->vec[i] = x;
  if (i >= v->cnt) v->cnt = i +1;
  return i;
}

val_t vec_get_2(vec_t v, uint32_t i)
{
  _vecdbg("v=%p",(void*)v);
  if (!v || v->cnt == 0) {errno = EINVAL; return valnil; }
  if (i == VECNONDX) i = v->cnt - 1; // Get the last element
  if (i >= v->cnt) {errno = ERANGE; return valnil; }
  return v->vec[i];
}

