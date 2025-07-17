//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

/*
To better understand how to use this data structure, refer to the diagaram below:

           push/enq ──╮ ╭─▷ pop
                      ▽ ╵ 
                    |    |◄─ end
      +(count-1) ─► │ ╴╴ │◄─ last/top
      +(count-2) ─► │ ╴╴ │    ╭╴first
                    │ ╴╴ │    ▼
        del/get  ◁─ │ ╴╴ ╰──────
        ins/set  ─▷ │  / ╎  ╎  ╎ ─▷ deq
                    ╰───────────
                      ...  +1 +0  

   - set/get/del/ins are used to handle it as an array. The indices range
     from `first` to `count-first-1`. If you don't use the the stack or queue
     functions, `first` is guaranteed to be 0 so you can consider it as a vector
     with indices ranging from `0` to `count-1`.
  
   - push/pop handle it as a stack. (Last-In / First-Out). `pop` returns the
     topmost element (the last inserted) and removes it from the stack. 
     `top` just returns the element without removing it. If the stack is empty
     both return `valnil`. 

   - enq/deq handle it as a list (First-In / First-Out). `deq` returns the
     next element (the first inserted) and removes it from the queue. 
     `first` just returns the element without removing it. If the queue is empty
     both return `valnil`. Using a valvec as a queue modifies the `first` index.

   - If you follow the push/pop and enq/deq policies, you will never have to worry
     about the `first` index. The same goes if you only use the set/get policy.
     However if you (for any unforeseeable reason) need to switch from a policy to
     another, you need to retrieve the first index using the a variant of 
     the `valfirst()` function.
     In the example below, the variable `first_index` will contain the value of
     the `first` index that you can use to scan the vector:
        
         int32_t first_index;
         val_t   first_value;
         val_t   vct = valvec();

         valenq(vct, 3);
         first_value = valfirst(vct, &first_index);
         for (int32_t k=0; k<valcount(vct); k++)
           printf("%d\n", valtoint(valget(vct,first_index+k));
*/


#define VAL_CODE
#include "vallib.h"

#ifdef VALNOTHREAD
#define VAL_THREAD_LOCAL
#endif

#ifndef VAL_THREAD_LOCAL
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
  #define VAL_THREAD_LOCAL _Thread_local        /* C11 compiler with threads */
#elif defined(_MSC_VER)
  #define VAL_THREAD_LOCAL __declspec(thread)   /* MSVC */
#elif defined(__GNUC__) || defined(__clang__)
  #define VAL_THREAD_LOCAL __thread             /* GCC/Clang */
#else
  #error "Cannot define VAL_THREAD_LOCAL on this compiler"
#endif
#endif

static VAL_THREAD_LOCAL val_t val_aux;
val_t valaux() {return val_aux;}

static void val_compact_queue(valptr_vec_t v, int always);
#define val_compact_q(v,x) for (valptr_vec_t v_ = v; v_ && (v_->fst != v_->end); v_=NULL) val_compact_queue(v_,x)
#define val_compact_queue_ifneeded(v) val_compact_q(v,0)
#define val_compact_queue_always(v)   val_compact_q(v,1)

// Multiply ndx by the golden ratio phi approximated
// through early elements Fibonacci sequence (13/8)
static inline int32_t mul_phi(int32_t ndx) 
{
  int64_t t = ndx;
  t = (t * 13) / 8;
  ndx = (int32_t)t;
  ndx = (ndx + 7) & ~((uint32_t)7); // by Multiples of 8
  return ndx;
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

int32_t val_ndx(valptr_vec_t a, int32_t ndx)
{
  errno = 0; 

       if (ndx == VAL_VEC_FIRSTNDX) return a->fst;
  else if (ndx == VAL_VEC_ENDNDX)  {
    val_compact_queue_ifneeded(a);
    return a->end;
  }

  if (ndx < 0) ndx = a->end + ndx;
  
  if (ndx < a->fst) {
    val_compact_queue_always(a);
    errno = ERANGE; 
    return 0; 
  }
  
  if (a->end <= ndx) errno = ERANGE;

  return ndx;
}


int val_set_vec(val_t a, int32_t ndx, val_t elm)
{
 
  valptr_vec_t vec_ptr = val_vecptr(a);
  if (vec_ptr == NULL) return 0;

  ndx = val_ndx(vec_ptr, ndx);

  if (!make_room_vec(vec_ptr, ndx)) return 0;

  vec_ptr->vec[ndx] = elm;

  if (ndx >= vec_ptr->end) vec_ptr->end = ndx+1;

  return 1;
}

val_t val_get_vec_p(valptr_vec_t vec_ptr, int32_t ndx)
{
  if ((vec_ptr->fst >= vec_ptr->end)) {
    vec_ptr->end = vec_ptr->fst = 0;
    errno = ERANGE; return valnil;
  }

  ndx = val_ndx(vec_ptr, ndx);

  if (ndx >= vec_ptr->end) {errno = ERANGE; return valnil; }

  errno = 0;
  return vec_ptr->vec[ndx];
}
 
val_t val_get_vec(val_t a, int32_t ndx)
{
  valptr_vec_t vec_ptr = val_vecptr(a);

  if (vec_ptr == NULL) return valnil;
  else return val_get_vec_p(vec_ptr, ndx);
}

int32_t valsize(val_t a) {
  valptr_vec_t vec_ptr = val_vecptr(a); 
  if (vec_ptr == NULL) return 0;

  // This is a side effect useful to ensure that the queue will
  // beahave well if treated as a vector from now on.
  val_compact_queue_always(vec_ptr);
  
  return vec_ptr->sze;
}

int val_del_vec(val_t a, int32_t ndx_from, int32_t ndx_to)
{
  valptr_vec_t vec_ptr = val_vecptr(a);

  if (vec_ptr == NULL) return 0;
  if (vec_ptr->end == vec_ptr->fst) return val_clear(vec_ptr);

  // The idea is that if you call valdel() on a queue (rather than using deq),
  // You deserve the extra time needed to campact the queue as you may probably 
  // want to use it as a vector from that point on. When fst == 0, it will 
  // return immediately.
  val_compact_queue_always(vec_ptr);

  ndx_from = val_ndx(vec_ptr, ndx_from);
  
  if (ndx_to == VAL_VEC_NONDX) ndx_to = ndx_from;
  
  ndx_to = val_ndx(vec_ptr, ndx_to);
  // fprintf(stderr,">>> DEL %d %d\n",ndx_from, ndx_to);

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

static int val_insgap(valptr_vec_t vec_ptr, int32_t ndx, int32_t len)
{
  int32_t n;
  errno = 0;
  
  ndx = val_ndx(vec_ptr,ndx);
  
  // vec_dbg("GAP : %d %d",i,l);

  if (ndx < vec_ptr->end) { // Gap inside the current array
    n = vec_ptr->end + len;
    /*                    _len_
    **  ABCDEFGH       ABC-----DEFGH
    **  |  |    |      |  |    |    |
    **  0 ndx  end     0 ndx   | end+len
    **                      ndx+len
    */
  }
  else { // Gap happens past the last element
    n = ndx+len;
    /*                                           _len_
    **  ABCDEFGH........          ABCDEFGH............
    **  |       |       |         |             |     |
    **  0       end    ndx        0            ndx ndx+len
    */
  }
//  vec_dbg("GAP end:%d i:%d l:%d n:%d",v->end,i,l,n);
  if (!make_room_vec(vec_ptr, n)) return 0;

  if (ndx < vec_ptr->end) {
    // Move the elements at the end of the array
    void *dst = &(vec_ptr->vec[ndx+len]);
    void *src = &(vec_ptr->vec[ndx]);
    int nbytes = (vec_ptr->end - ndx) * sizeof(val_t);
    memmove(dst, src, nbytes);
    memset(src, 0, len * sizeof(val_t));
  }
  else {
    void *dst = &(vec_ptr->vec[vec_ptr->end]);
    int nbytes = (n - vec_ptr->end) * sizeof(val_t);
    memset(dst, 0, nbytes);
  }

  vec_ptr->end = n;
  return 1;
}

int val_ins(val_t a, int32_t ndx, val_t elm, int32_t rep)
{
  if (rep <= 0) rep = 1;

  valptr_vec_t vec_ptr = val_vecptr(a);
  if (vec_ptr == NULL) return 0;

  if (!val_insgap(vec_ptr, ndx, rep)) return 0;

  for (int k=ndx; k<ndx+rep; k++)
    vec_ptr->vec[k] = elm;

  return 1;
}

// HASH
int val_set_map(val_t a, val_t key, val_t elm)
{ return 0; }

val_t val_get_map(val_t a, val_t key)
{ return valnil; }

// QUEUE

static void val_compact_queue(valptr_vec_t v, int always)
{
  if (v->fst == 0) return;
  if (v->fst >= v->end) {
    v->fst = 0;
    v->end = 0;
    return;
  }
  if (always || ((v->end >= v->sze) && (v->fst >= (v->sze / 4)))) {
    void *src = &(v->vec[v->fst]);
    memmove(v->vec,src,(v->end - v->fst) * sizeof(val_t));
    v->end -= v->fst;
    v->fst  = 0;
  }
}

val_t val_first_or_last(val_t a, int32_t *retndx, int32_t ndx) 
{
  valptr_vec_t vec_ptr = val_vecptr(a);
  if (vec_ptr == NULL) return valnil;
  if (retndx != NULL) {
    val_compact_queue_always(vec_ptr);
    *retndx = 0; // vec_ptr->fst would be 0 anyway
  }
  return val_get_vec_p(vec_ptr, ndx);  
}

int val_veccmp(const void *a, const void *b) {
    const val_t *va = (const val_t*)a;
    const val_t *vb = (const val_t*)b;
    return valcmp(*va, *vb);
}

int val_sort(val_t a, val_cmp_t cmp, val_t aux)
{
  valptr_vec_t vec_ptr = val_vecptr(a);
  if (vec_ptr == NULL) return 0;
  val_aux = valisnil(aux) ? a : aux;

  int32_t num_elm = valcount(a);
  if (num_elm == 0) return 0;

  val_compact_queue_always(vec_ptr);

  qsort(vec_ptr->vec,num_elm,sizeof(val_t),cmp);
  return 1;
}

int32_t val_bsearch(val_t a, val_t key, val_cmp_t cmp, val_t aux)
{
  valptr_vec_t vec_ptr = val_vecptr(a);
  if (vec_ptr == NULL) return -1;
  val_aux = valisnil(aux) ? a : aux;

  int32_t num_elm = valcount(a);
  if (num_elm == 0) return -1;

  val_t *base = vec_ptr->vec;
  val_t *elm  = bsearch(&key,base,num_elm,sizeof(val_t),cmp);

  return elm? (elm-base) : -1;
}

// store
val_store_t val_stores[VAL_NUM_STORES] = { 0 };

val_t val_store(void *elm, int32_t len, int32_t isstring)
{
  int nstore = 0;
  // nstore = valhash(elm) % VAL_NUM_STORES;
  //  fprintf(stderr, ">>> nstore: %d\n",nstore);
  
  if (len == 0 && isstring) len = strlen(elm) +1;
  len += isstring;

  val_t ret = (val_t) {VAL_STORED_0 | ((int64_t)nstore << 32) | val_stores[nstore].end};
  int32_t newend = val_stores[nstore].end + len;
  newend = (newend + 7) & ~((int32_t)7); // Align to eight!

  if (newend >= val_stores[nstore].sze) {
    int32_t newsze = val_stores[nstore].sze;
    if (newsze == 0) { newsze = 16; val_stores[nstore].data = NULL; }
    while (newsze <= newend) {
      newsze = mul_phi(newsze);
    }
    uint8_t *newdata = realloc(val_stores[nstore].data,newsze);
    if (newdata == NULL) {
      errno = ENOMEM;
      return valnil;
    }
    val_stores[nstore].data = newdata;
  }
  memcpy(val_stores[nstore].data+val_stores[nstore].end,elm,len);
  if (isstring) {
    val_stores[nstore].data[val_stores[nstore].end + len - 1] = '\0';
  }
  errno = 0;
  val_stores[nstore].end = newend;
  fprintf(stderr, ">>> ret: %s\n",valtostr(ret).str);
  return ret;
}

void *valrecall(val_t x) 
{
  if (valisstored(x)) { 
    errno = 0;
    uint32_t offset = x.v & VAL_32BIT_MASK;
    int32_t nstore = (x.v >> 32) & 0x0F;
    if (offset < val_stores[nstore].end)
      return val_stores[nstore].data + offset;
  }

  errno = EINVAL;
  return NULL; 
}
