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

#include <stdarg.h>

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

    if (new_vec == NULL) {errno = ENOMEM; return 0; }

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
  vec_ptr->flg = 0;
  vec_ptr->del = 0;
  vec_ptr->dsz = 0;
  vec_ptr->dead = NULL;

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
    if (vec_ptr->dead != NULL) free(vec_ptr->dead);
    vec_ptr->vec = NULL;
    vec_ptr->dead = NULL;
    vec_ptr->end = 0;
    vec_ptr->fst = 0;
    vec_ptr->sze = 0;
    free(vec_ptr);
  }
  else if (valisbufptr(a)) {
    valptr_buf_t buf_ptr = valtoptr(a);
    if (buf_ptr->text != NULL) free(buf_ptr->text);
    buf_ptr->text = NULL;
    buf_ptr->len  = 0;
    buf_ptr->sze  = 0;
    free(buf_ptr);
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

  // Zero the gap between the previous end and ndx, so that reads of
  // previously-unset elements below the new end return 0.0. This is the only
  // region that can become visible, so it is zeroed exactly (not the whole
  // capacity): appending is O(1) even across reallocations.
  if (ndx > vec_ptr->end) {
    memset(&vec_ptr->vec[vec_ptr->end], 0, (ndx - vec_ptr->end) * sizeof(val_t));
  }

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
  
  if (vec_ptr->flg & VAL_VEC_MAP) return vec_ptr->sze/2;
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
// MAP
// The map is a sorted array of (key, value) pairs: entry i occupies the
// val_t slots [2i, 2i+1], sorted on the key via valcmp(). Keys are unique:
// inserting an existing key updates the value in place. Deleted entries are
// tombstoned (dead[] flag) and compacted away when they exceed 1/4 of the map.

// Binary search for the entry with the given key.
// Returns the entry index where the key is, or where it would be inserted.
static int32_t val_map_ndx(valptr_vec_t v, val_t key, int *found)
{
  int32_t lo = 0;
  int32_t hi = v->end / 2;

  while (lo < hi) {
    int32_t mid = lo + (hi - lo) / 2;
    if (valcmp(v->vec[2*mid], key) < 0) lo = mid + 1;
    else hi = mid;
  }

  *found = (lo < v->end/2) && (valcmp(v->vec[2*lo], key) == 0);
  return lo;
}

// Grow the per-entry deleted-flags array to hold at least `n` entries.
static int make_room_dead(valptr_vec_t v, int32_t n)
{
  if (n > v->dsz) {
    int32_t newdsz = v->dsz ? v->dsz : 8;
    while (newdsz < n) newdsz = mul_phi(newdsz);
    uint8_t *newdead = realloc(v->dead, newdsz);
    if (newdead == NULL) { errno = ENOMEM; return 0; }
    memset(newdead + v->dsz, 0, newdsz - v->dsz);
    v->dead = newdead;
    v->dsz  = newdsz;
  }
  errno = 0;
  return 1;
}

// Remove tombstoned entries in a single pass (amortized O(1) per delete).
static void val_map_compact(valptr_vec_t v)
{
  int32_t n = v->end / 2;
  int32_t dst = 0;

  for (int32_t src = 0; src < n; src++) {
    if (!v->dead[src]) {
      if (dst != src) {
        v->vec[2*dst]   = v->vec[2*src];
        v->vec[2*dst+1] = v->vec[2*src+1];
        v->dead[dst]    = 0;
      }
      dst++;
    }
  }

  v->end = 2*dst;
  v->del = 0;
  memset(&v->dead[dst], 0, n - dst);
}

int val_set_map(val_t a, val_t key, val_t elm)
{
  valptr_vec_t v = val_vecptr(a);
  if (v == NULL) return 0;

  if (!(v->flg & VAL_VEC_MAP)) { // Convert a fresh vec into a map
    if (v->end != 0 || v->fst != 0) { errno = EINVAL; return 0; }
    v->flg |= VAL_VEC_MAP;
  }

  int found;
  int32_t ndx = val_map_ndx(v, key, &found);

  if (found) { // Update the value in place (or revive a tombstoned entry)
    v->vec[2*ndx+1] = elm;
    if (v->dead[ndx]) { v->dead[ndx] = 0; v->del -= 1; }
    errno = 0;
    return 1;
  }

  // Insert a new entry at position ndx
  int32_t n = v->end / 2;
  if (!make_room_vec(v, v->end + 1)) return 0;
  if (!make_room_dead(v, n + 1)) return 0;

  memmove(&v->vec[2*(ndx+1)], &v->vec[2*ndx], 2*(n-ndx) * sizeof(val_t));
  memmove(&v->dead[ndx+1], &v->dead[ndx], n-ndx);
  v->vec[2*ndx]   = key;
  v->vec[2*ndx+1] = elm;
  v->dead[ndx]    = 0;
  v->end += 2;
  errno = 0;
  return 1;
}

val_t val_get_map(val_t a, val_t key)
{
  valptr_vec_t v = val_vecptr(a);
  if (v == NULL) return valnil;
  if (!(v->flg & VAL_VEC_MAP)) { errno = EINVAL; return valnil; }

  int found;
  int32_t ndx = val_map_ndx(v, key, &found);

  if (found && !v->dead[ndx]) { errno = 0; return v->vec[2*ndx+1]; }

  errno = ENOENT;
  return valnil;
}

int val_del_map(val_t a, val_t key)
{
  valptr_vec_t v = val_vecptr(a);
  if (v == NULL) return 0;
  if (!(v->flg & VAL_VEC_MAP)) { errno = EINVAL; return 0; }

  int found;
  int32_t ndx = val_map_ndx(v, key, &found);

  if (!found || v->dead[ndx]) { errno = ENOENT; return 0; }

  v->dead[ndx] = 1;
  v->del += 1;
  errno = 0;

  if (v->del * 4 >= v->end / 2) val_map_compact(v); // Too many tombstones
  return 1;
}

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

// ==== BUFFERS
// Growable NUL-terminated text buffers (see valptr_buf_s in val_lib_post.h).
// All functions return 0 (or valnullptr for valbufnew) on error with errno
// set (EINVAL/ENOMEM/ERANGE).

static int make_room_buf(valptr_buf_t b, int32_t need)
{
  if (b == NULL) {errno = EINVAL; return 0;}

  if (need < 0) {errno = ERANGE; return 0;}  // Overflow guard

  if (need <= b->sze) {errno = 0; return 1;} // Enough room

  int32_t new_size = b->sze;
  if (new_size == 0) new_size = 16;
  while (new_size < need) new_size = mul_phi(new_size);

  char *new_text = realloc(b->text, new_size);
  if (new_text == NULL) {errno = ENOMEM; return 0;}
  b->text = new_text;
  b->sze = new_size;
  errno = 0;
  return 1;
}

val_t valbufnew(int32_t size)
{
  valptr_buf_t b = malloc(sizeof(struct valptr_buf_s));
  if (b == NULL) {errno = ENOMEM; return valnullptr;}

  b->text = NULL;
  b->sze = 0;
  b->len = 0;

  if ((0 < size) && !make_room_buf(b, size)) {
    free(b);
    return valnullptr;
  }
  if (b->text != NULL) b->text[0] = '\0'; // Keep it a valid empty string

  return val(b);
}

int valbufcpy(val_t a, const char *s)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;
  if (s == NULL) {errno = EINVAL; return 0;}

  int32_t len = (int32_t)strlen(s);
  if (!make_room_buf(b, len+1)) return 0;

  memcpy(b->text, s, len+1);
  b->len = len;
  errno = 0;
  return 1;
}

int valbufcat(val_t a, const char *s)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;
  if (s == NULL) {errno = EINVAL; return 0;}

  int32_t len = (int32_t)strlen(s);
  if (!make_room_buf(b, b->len + len + 1)) return 0;

  memcpy(b->text + b->len, s, len+1);
  b->len += len;
  errno = 0;
  return 1;
}

int valbufadd(val_t a, const char *s, int32_t len)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;
  if ((s == NULL) || (len < 0)) {errno = EINVAL; return 0;}

  if (!make_room_buf(b, b->len + len + 1)) return 0;

  memcpy(b->text + b->len, s, len);
  b->len += len;
  b->text[b->len] = '\0'; // Always NUL-terminate
  errno = 0;
  return 1;
}

int valbuffmt(val_t a, const char *fmt, ...)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;
  if (fmt == NULL) {errno = EINVAL; return 0;}

  va_list ap;
  va_start(ap, fmt);
  int need = vsnprintf(NULL, 0, fmt, ap);
  va_end(ap);
  if (need < 0) {errno = EINVAL; return 0;}

  if (!make_room_buf(b, need+1)) return 0;

  va_start(ap, fmt);
  vsnprintf(b->text, (size_t)(need+1), fmt, ap);
  va_end(ap);
  b->len = need;
  errno = 0;
  return 1;
}

int valbufcatf(val_t a, const char *fmt, ...)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;
  if (fmt == NULL) {errno = EINVAL; return 0;}

  va_list ap;
  va_start(ap, fmt);
  int need = vsnprintf(NULL, 0, fmt, ap);
  va_end(ap);
  if (need < 0) {errno = EINVAL; return 0;}

  if (!make_room_buf(b, b->len + need + 1)) return 0;

  va_start(ap, fmt);
  vsnprintf(b->text + b->len, (size_t)(need+1), fmt, ap);
  va_end(ap);
  b->len += need;
  errno = 0;
  return 1;
}

int valbufins(val_t a, int32_t ndx, const char *s)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;
  if (s == NULL) {errno = EINVAL; return 0;}

  if (ndx < 0) ndx = b->len + ndx;      // Negative: count from the end
  if ((ndx < 0) || (ndx > b->len)) {errno = ERANGE; return 0;}

  int32_t len = (int32_t)strlen(s);
  if (!make_room_buf(b, b->len + len + 1)) return 0;

  memmove(b->text + ndx + len, b->text + ndx, (size_t)(b->len - ndx + 1));
  memcpy (b->text + ndx, s, (size_t)len);
  b->len += len;
  errno = 0;
  return 1;
}

int valbufdel(val_t a, int32_t from, int32_t to)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;

  if (from < 0) from = b->len + from;   // Negative: count from the end
  if (to   < 0) to   = b->len + to;
  if ((from < 0) || (from > to) || (from >= b->len)) {errno = ERANGE; return 0;}
  if (to >= b->len) to = b->len - 1;

  memmove(b->text + from, b->text + to + 1, (size_t)(b->len - to));
  b->len -= (to - from) + 1;
  b->text[b->len] = '\0'; // Keep it NUL-terminated
  errno = 0;
  return 1;
}

int valbufclear(val_t a)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;

  b->len = 0;
  if (b->text != NULL) b->text[0] = '\0';
  errno = 0;
  return 1;
}

int valbufreserve(val_t a, int32_t extra)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return 0;
  if (extra < 0) {errno = ERANGE; return 0;}

  return make_room_buf(b, b->len + extra + 1);
}

int32_t valbuflen(val_t a)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return -1;
  return b->len;
}

int32_t valbufsze(val_t a)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return -1;
  return b->sze;
}

char *valbuftxt(val_t a)
{
  valptr_buf_t b = val_bufptr(a);
  if (b == NULL) return NULL;
  return b->text; // May be NULL if no room has been allocated yet
}

// store
val_store_t val_stores[VAL_NUM_STORES] = { 0 };
val_t val_store(const char *str)
{
  int nstore = 0;
  val_store_t *st = &val_stores[nstore];
  // nstore = valhash(str) % VAL_NUM_STORES;

  if (str == NULL) { errno = EINVAL; return valnil; }

  int32_t len = strlen(str) + 1; // Include the NUL terminator

  // Make room in the data arena
  int32_t newend = st->end + len;
  newend = (newend + 7) & ~((int32_t)7); // Align to eight!
  if (newend >= st->sze) {
    int32_t newsze = st->sze;
    if (newsze == 0) { newsze = 16; st->data = NULL; }
    while (newsze <= newend) {
      newsze = mul_phi(newsze);
    }
    uint8_t *newdata = realloc(st->data,newsze);
    if (newdata == NULL) {
      errno = ENOMEM;
      return valnil;
    }
    st->data = newdata;
    st->sze = newsze; // Remember the new capacity
  }

  // Make room in the entries registry
  if (st->cnt >= st->cap) {
    int32_t newcap = st->cap;
    if (newcap == 0) newcap = 8;
    while (newcap <= st->cnt) {
      newcap = mul_phi(newcap);
    }
    val_store_ent_t *newent = realloc(st->ent,newcap*sizeof(val_store_ent_t));
    if (newent == NULL) {
      errno = ENOMEM;
      return valnil;
    }
    st->ent = newent;
    st->cap = newcap;
  }

  memcpy(st->data+st->end,str,len);
  st->data[st->end + len - 1] = '\0'; // Ensure termination

  st->ent[st->cnt] = (val_store_ent_t){.offset = st->end, .len = len, .deleted = 0};
  st->cnt += 1;

  val_t ret = (val_t) {VAL_STORED_0 | ((int64_t)nstore << 32) | st->end};
  st->end = newend;
  errno = 0;
  return ret;
}

int val_del_store(val_t v)
{
  if (valisstored(v)) {
    uint32_t nstore = (uint32_t)((v.v >> 32) & 0x0F);
    uint32_t offset = (uint32_t)(v.v & VAL_32BIT_MASK);
    if (nstore < VAL_NUM_STORES) {
      val_store_t *st = &val_stores[nstore];
      int32_t ndx = val_store_ndx_1(st, offset);
      if (ndx >= 0) {
        st->ent[ndx].deleted = 1; // Tombstone: bytes and slot are kept
        errno = 0;
        return 1;
      }
    }
  }
  errno = EINVAL;
  return 0;
}

int32_t val_store_cnt(int32_t nstore)
{
  if (nstore < 0 || nstore >= VAL_NUM_STORES) { errno = EINVAL; return -1; }
  return val_stores[nstore].cnt;
}

int32_t val_store_sze(int32_t nstore)
{
  if (nstore < 0 || nstore >= VAL_NUM_STORES) { errno = EINVAL; return -1; }
  return val_stores[nstore].sze;
}

int32_t val_store_top(int32_t nstore)
{
  if (nstore < 0 || nstore >= VAL_NUM_STORES) { errno = EINVAL; return -1; }
  return val_stores[nstore].cnt;
}

int32_t val_store_len(int32_t nstore, int32_t ndx)
{
  if (nstore < 0 || nstore >= VAL_NUM_STORES) { errno = EINVAL; return -1; }
  val_store_t *st = &val_stores[nstore];
  if (ndx < 1 || ndx > st->cnt) { errno = EINVAL; return -1; }
  return st->ent[ndx-1].len;
}

int32_t val_store_offset(int32_t nstore, int32_t ndx)
{
  if (nstore < 0 || nstore >= VAL_NUM_STORES) { errno = EINVAL; return -1; }
  val_store_t *st = &val_stores[nstore];
  if (ndx < 1 || ndx > st->cnt) { errno = EINVAL; return -1; }
  return st->ent[ndx-1].offset;
}

void *valrecall(val_t x) 
{
  if (valisstored(x)) {
    void *ret = val_recall_1(x);
    if (ret != NULL) { errno = 0; return ret; }
  }

  errno = EINVAL;
  return NULL; 
}
