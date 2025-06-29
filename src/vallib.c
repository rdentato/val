//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT


#define VAL_CODE
#include "vallib.h"

// Multiply ndx by the golden ratio phi approximated
// through early elements Fibonacci sequence (13/8)
static inline int32_t mul_phi(int32_t ndx) {
  int64_t t = ndx;
  t = (t * 13) / 8;
  return (int32_t)t;
}

#define VAL_NDX_OVERFLOW ((int32_t)0x0D79435D)

static int make_room_vec(valptr_vec_t v, int32_t ndx)
{
  if (v == NULL) {errno = EINVAL; return 0;}

  if (ndx >= VAL_VEC_MAXNDX) {errno = ERANGE; return 0;}

  //fprintf(stderr,">>> Making room ndx: %d size: %d\n",ndx,v->sze);
 
  if (ndx >= v->sze) { // Not enough room!
    int32_t new_size = v->sze;
    if (new_size == 0) new_size = 2;

    if (ndx > VAL_NDX_OVERFLOW) new_size = ndx + 1; // Avoid overflow!
    else while (new_size <= ndx) {
      new_size = mul_phi(new_size);
      new_size += (new_size & 1);
    }
    
    //fprintf(stderr,">>> about to realloc ndx: %d new size: %d (%d) vec: %p\n",ndx,new_size,new_size * (int)sizeof(val_t),v->vec);

    val_t *new_vec = realloc(v->vec, new_size * sizeof(val_t));

    // fprintf(stderr,">>> realloc (vec: %p)\n", new_vec);

    if (new_vec == NULL) {errno = ENOMEM; return 0; }

    // set the newly allocated area to 0;
    // fprintf(stderr,">>> about to memset vec: %p size: %d\n", &(new_vec[v->sze]), (new_size - v->sze) * (int)sizeof(val_t));
    memset(&(new_vec[v->sze]), 0, (new_size - v->sze) * sizeof(val_t));

    v->vec = new_vec;
    v->sze = new_size;
  }
  errno = 0;
  return 1;
}

val_t val_vec(int32_t initial_sze)
{
  valptr_vec_t vec_ptr = malloc(sizeof(struct valptr_vec_s));

  if (vec_ptr == NULL) {
    errno = ENOMEM;
    return valnullptr;
  }

  vec_ptr->vec = NULL;
  vec_ptr->end = 0;
  vec_ptr->fst = 0;
  vec_ptr->sze = 0;

  if ((0 < initial_sze) && (initial_sze < VAL_VEC_MAXNDX)) {
    if (!make_room_vec(vec_ptr,initial_sze)) {
      if (vec_ptr->vec) free(vec_ptr->vec);
      return valnullptr;
    }
  }
  
  return val(vec_ptr);
}

val_t valfree(val_t a)
{
  if (valisvecptr(a)) {
    valptr_vec_t vec_ptr = valtoptr(a);
    if (vec_ptr->vec != NULL) free(vec_ptr->vec);
    vec_ptr->vec = NULL;
    vec_ptr->end = 0;
    vec_ptr->fst = 0;
    vec_ptr->sze = 0;
    free(vec_ptr);
  }
  else errno = EINVAL;

  return valnullptr;
}

static void compact_queue(valptr_vec_t v, int always)
{
  if (v->fst == 0) return;
  if (v->fst >= v->end) {
    v->fst = 0;
    v->end = 0;
    return;
  }
  if (always || ((v->end >= v->sze) && (v->fst > (v->sze / 2)))) {
    void *src = v->vec + (v->fst * sizeof(val_t));
    memmove(v->vec,src,(v->end - v->fst) * sizeof(val_t));
    v->end -= v->fst;
    v->fst  = 0;
  }
}

#define compact_queue_ifneeded(v) compact_queue(v,0)
#define compact_queue_always(v)   compact_queue(v,1)

int val_set_vec(val_t a, int32_t ndx, val_t elm)
{
  if (!valisvecptr(a)) {errno = EINVAL; return 0;}
  
  valptr_vec_t vec_ptr = valtoptr(a);
  if (vec_ptr == NULL) {errno = EINVAL; return 0;}

  if (ndx == VAL_VEC_TOPNDX) {
    compact_queue_ifneeded(vec_ptr);
    ndx = vec_ptr->end;
  }

  if (!make_room_vec(vec_ptr, ndx)) {return 0;}

  vec_ptr->vec[ndx] = elm;

  if (ndx >= vec_ptr->end) vec_ptr->end = ndx+1;

  errno = 0;
  return 1;
}

val_t val_get_vec(val_t a, int32_t ndx)
{
  if (!valisvecptr(a)) {errno = EINVAL; return valnil;}
  
  valptr_vec_t vec_ptr = valtoptr(a);
  if (vec_ptr == NULL || (vec_ptr->end == vec_ptr->fst)) {
    errno = EINVAL; return valnil;
  }

  if (ndx == VAL_VEC_TOPNDX) ndx = vec_ptr->end-1;
  else if (ndx == VAL_VEC_FIRSTNDX) ndx = vec_ptr->fst;

  if (ndx >= vec_ptr->end) {errno = ERANGE; return valnil; }

  errno = 0;
  return vec_ptr->vec[ndx];
}


static int val_del(valptr_vec_t vec_ptr, int32_t ndx_from, int32_t ndx_to)
{
  if (vec_ptr == NULL) {errno = ERANGE; return 1;}

  if (vec_ptr->end == vec_ptr->fst) return val_clear(vec_ptr);

  errno = 0;
   
  if (ndx_from == VAL_VEC_TOPNDX) { ndx_from = vec_ptr->end-1; ndx_to = ndx_from; }
  else if (ndx_to == VAL_VEC_NONDX) ndx_to = ndx_from;

  if ((ndx_from >= vec_ptr->end) || (ndx_to < ndx_from)) {errno = ERANGE; return 0;}

  if (ndx_to >= vec_ptr->end-1) {
    vec_ptr->end = ndx_from; // Just drop last elements
  }
  else {
    //       __l__
    //    ABCdefghIJKLM              ABCIJKLM
    //    |  |    |    |             |  |    |    
    //    0  i   i+l   end           0  i    end-l  

    memmove( &(vec_ptr->vec[ndx_from]), &(vec_ptr->vec[ndx_to + 1]), (vec_ptr->end-(ndx_to + 1)) * sizeof(val_t));
    vec_ptr->end -= (ndx_to - ndx_from) + 1;
  }

  if (vec_ptr->end <= vec_ptr->fst) return val_clear(vec_ptr);

  return 1;
}

int val_del_vec(val_t a, int32_t ndx_from, int32_t ndx_to)
{
  if (!valisvecptr(a)) {errno = EINVAL; return 0;}
  
  valptr_vec_t vec_ptr = valtoptr(a);
  if (vec_ptr == NULL) {errno = EINVAL; return 0;}
  return val_del(vec_ptr, ndx_from, ndx_to);
}

int val_set_map(val_t a, val_t key, val_t elm)
{ return 0; }

val_t val_get_map(val_t a, val_t key)
{ return valnil; }

