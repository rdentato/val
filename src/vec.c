//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "vec_.h"
#include <stdalign.h>

#define RETURN_IF valreturnif

#ifdef DEBUG
int64_t vecallocatedmem = 0;
#define add_mem(m) (vecallocatedmem += (m))
#define sub_mem(m) (vecallocatedmem -= (m))
#else 
#define add_mem(m) ((void)0)
#define sub_mem(m) ((void)0) 
#endif

#define VALS_PER_MAPNODE 4

typedef struct vec_mapnode_s {
  val_t    value;
  val_t    key;
  uint32_t level;
  uint32_t parent;
  uint32_t left;
  uint32_t right;
} vec_mapnode_t;

static_assert(sizeof(vec_mapnode_t) == VALS_PER_MAPNODE * sizeof(val_t));

#define getnode(v,n)   ((vec_mapnode_t *) &((v)->vec[n]))
#define setleft(n,l)   ((n)->left = l)
#define getleft(n)     ((n)->left)
#define setright(n,r)  ((n)->right = r)
#define getright(n)    ((n)->right)
#define getroot(t)     ((t)->fst)
#define setroot(t,n)   ((t)->fst = n)
#define getkey(n)      ((n)->key)
#define getvalue(n)    ((n)->value)
#define setkey(n,k)    ((n)->key = k)
#define setvalue(n,v)  ((n)->value = v)

#define setlevel(n,l)  ((n)->level = l)

#define getlevel(...) VEC_vrg(getlevel_,__VA_ARGS__)
#define getlevel_1(n)    ((n)->level)
#define getlevel_2(v,n)  (getnode(v,n)->level)

#define getparent(...) VEC_vrg(getparent_,__VA_ARGS__)
#define getparent_1(n)   ((n)->parent)
#define getparent_2(v,n)   (getnode(v,n)->parent)

#define setparent(...) VEC_vrg(setparent_,__VA_ARGS__)
#define setparent_2(n,p)   ((n)->parent = p)
#define setparent_3(v,n,p) (getnode(v,n)->parent = p)

#define getnumnodes(v) (((v)->end) / VALS_PER_MAPNODE)
#define incnumnodes(v) ((v)->end += VALS_PER_MAPNODE)
#define decnumnodes(v) ((v)->end -= VALS_PER_MAPNODE)
#define lastnodendx(v) ((v)->end - VALS_PER_MAPNODE)

#define is_emptytree(v) ((v)->end == 0)

#define marknode(n)     ((n)->level |= 0x80000000)
#define unmarknode(n)   ((n)->level &= 0x7FFFFFFF)
#define ismarkednode(n) ((n)->level &  0x80000000)

#define NULLNODE       VECNONDX

static int volatile ZERO = 0;

val_t vecnew() {
  errno = 0;
  vec_t v = aligned_alloc(4,sizeof(struct vec_s));
  
  RETURN_IF(v == NULL, valnil, ENOMEM);
  
  add_mem(sizeof(struct vec_s));
  memset(v,0,sizeof(struct vec_s));
  return val(v);
}

static val_t vec_free(val_t vv);

static void freevecs(val_t *v,uint32_t from, uint32_t to)
{
  if (v) for (int k=from; k<to; k++) {
    //val_dbg("free: %016lX %d %d",v[k].v,k,valisownedvec(v[k]));
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
  //freevecs(v->vec,v->fst,v->end);
  sub_mem(v->sze * sizeof(val_t));
  free(v->vec);
  sub_mem(sizeof(struct vec_s));
  free(v); 
  return valnil;
}

val_t vecfree(val_t vv) {
  errno = 0;
  
  RETURN_IF(!valisvec(vv) || valisownedvec(vv), vv, EINVAL);
  
  return vec_free(vv);
}

// Ensure the vector is large enough to store a value at index n
static int makeroom(vec_t v, uint32_t n)
{
  uint32_t new_sze;
  val_t   *new_vec;

  errno = 0;
  RETURN_IF(!v,0, EINVAL);

  val_dbg("vec Making room %p (%p)[%d]->[%d]",(void *)v,(void *)(v->vec),v->sze,n);

  // There should be at least n+1 slots:
  n += 1;

  if (n <= v->sze) return 1; // There's enough room

  RETURN_IF(n > VECMAXNDX,0,ERANGE); // Max number of elments in a vector reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = v->sze ? v->sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1); // ensure is even
  
  RETURN_IF(new_sze <= n,0,ERANGE);

  val_dbg("MKROOM: realloc(%p,%lu) [%u]->[%u]",(v->vec),new_sze * sizeof(val_t),v->sze, new_sze);
  new_vec = realloc(v->vec, new_sze * sizeof(val_t));
  val_dbg("MKROOM: got(%p,%d) [%d]",new_vec,new_sze * sizeof(val_t),new_sze);

  RETURN_IF(new_vec == NULL,0,ENOMEM);
  add_mem((new_sze - v->sze) * sizeof(val_t));
  // set the newly allocated area to 0;
  memset(&(new_vec[v->end]),0,(new_sze-v->sze)*sizeof(val_t));

  v->vec = new_vec;
  v->sze = new_sze;
  
  return 1;
}

int vec_gap_3(val_t vv, uint32_t i, uint32_t l)
{
  vec_t v;
  int n;
  errno = 0;

  RETURN_IF(!valisvec(vv),0,EINVAL);

  v = (vec_t)valtovec(vv);

  if (i == VECNONDX) i = v->end;
  if (l == VECNONDX) l = 1;
  val_dbg("GAP : %d %d",i,l);

  if (i < v->end) {
    n = v->end+l;
    /*                    __l__
    **  ABCDEFGH       ABC-----DEFGH
    **  |  |    |      |  |    |    |
    **  0  i    end    0  i   i+l   end+l
    */
  }
  else {
    n = i+l;
    /*                                           __l__
    **  ABCDEFGH........          ABCDEFGH............
    **  |       |     |           |             |     |
    **  0       end   i           0             i     i+l
    */
  }
  val_dbg("GAP end:%d i:%d l:%d n:%d",v->end,i,l,n);
  if (!makeroom(v, n)) return 0;
  if (i < v->end) {
    memmove(&(v->vec[i+l]),  &(v->vec[i]),  (v->end-i)*sizeof(val_t));
    memset(&(v->vec[i]),0, l * sizeof(val_t));
  }
  v->end = n;
  return 1;
}

val_t *vec(val_t v)
{
  RETURN_IF(!valisvec(v),NULL,EINVAL);
  return ((vec_t)valtocleanpointer(v))->vec;
}

int vectype_2(val_t vv,int type)
{ 
  vec_t v;
  RETURN_IF(!valisvec(vv),VEC_ISNOT,EINVAL);
  v = (vec_t)valtovec(vv);
  if (type >=0 ) v->typ = (type & 0x0F) | (v->typ & 0xF0);
  return (v->typ & 0x0F);
}

uint32_t vecsize_2(val_t vv, uint32_t n)
{
  vec_t v;
  errno = 0;
  RETURN_IF(!valisvec(vv),0,EINVAL); 
  v = (vec_t)valtovec(vv);
  RETURN_IF(n != VECNONDX && !makeroom(v,n),0,ENOMEM);
  return v->sze;
}

uint32_t veccount_2(val_t vv, uint32_t n)
{
  vec_t v;
  errno = 0;

  RETURN_IF(!valisvec(vv),0,EINVAL);
  
  v = (vec_t)valtovec(vv);

  if (n == 0) {
    v->end = 0;
    v->fst = 0;
    v->flg &= VEC_NOTYPE;
    return 0;
  }

  if (vectype(vv) == VEC_ISMAP) return getnumnodes(v);

  uint32_t first = 0;
  if (vectype(vv) == VEC_ISQUEUE) first = v->fst;

  if (n != VECNONDX && (n < (VECMAXNDX - first)))  {
    n += first;
    RETURN_IF(!makeroom(v, n ), 0, ENOMEM);
    if (n > v->end) v->flg &= ~VEC_SORTED;
    v->end = n;
    if (v->fst > n) v->fst = n;
  }

  return v->end - first;
}

// ADDING VALUES TO VEC

static val_t setval(vec_t v, uint32_t i, val_t x)
{
  // I'm about to overwrite an owned vector. Free it! (unless it is the same!)
  if ((v->fst <= i && i < v->end) && valisownedvec(v->vec[i]))
    if (v->vec[i].v != (x.v | 1)) vec_free(v->vec[i]);
  v->vec[i] = x;
  v->flg &= ~VEC_SORTED;
  return x;
}

val_t vecins_(val_t vv, uint32_t i, val_t x)
{
  vec_t v;

  RETURN_IF(!vecmakegap(vv,i,1),valerror,ENOMEM);

  v = (vec_t)valtocleanpointer(vv);
  return setval(v, i, x);
}

val_t vecset_(val_t vv, uint32_t i, val_t x)
{
  errno = 0;
  RETURN_IF(!valisvec(vv), valerror, EINVAL);

  vec_t v = (vec_t)valtocleanpointer(vv);

  if (i == VECNONDX) i = v->end;
  
  RETURN_IF(!makeroom(v,i), valerror, ENOMEM);

  x=setval(v, i, x);

  if (i >= v->end) v->end = i+1;
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
    //val_dbg("disowned");
    x.v &= ((uint64_t)0xFFFFFFFFFFFFFFFE);
    ((vec_t)valtocleanpointer(vv))->vec[i] = x;
    return x;
  }
  else return x;
}

val_t vecpush_(val_t vv, val_t x, int own) 
{ 
  errno = 0;
  RETURN_IF(!valisvec(vv), valerror, EINVAL);
  vectype(vv, VEC_ISSTACK); 
  if (own) return vecown_3_(vv,VECNONDX,x);
  return vecset_(vv,VECNONDX,x); 
}

val_t vecenq_(val_t vv, val_t x,int own)
{
  vec_t v;
  errno = 0;
  RETURN_IF(!valisvec(vv), valerror, EINVAL);

  vectype(vv,VEC_ISQUEUE);
  v = (vec_t)valtocleanpointer(vv);
  if ((v->fst > (v->end / 2)) && (v->end >= v->sze)) {
    memmove(v->vec, &(v->vec[v->fst]), (v->end - v->fst) * sizeof(val_t));
    v->end -= v->fst;
    v->fst = 0;
  }
  
  if (own) return vecown_3_(vv,VECNONDX,x);
  return vecset_(vv,VECNONDX,x); 
}

#define vec_data(v_) ((v_)->vec)

uint32_t vec_search_map(vec_t v, val_t key, uint32_t *parent_ndx, int *right, int *depth);

val_t vecget_(val_t vv, val_t ii)
{
  vec_t v;
  errno = 0;
  val_t x;
  uint32_t parent_ndx;
  int right;
  int depth;

  RETURN_IF(!valisvec(vv), valnil, EINVAL);
  v = (vec_t)valtovec(vv);

  if (vectype(vv) == VEC_ISMAP) {
    val_dbg("Retrieve from map");
    uint32_t node_ndx = vec_search_map(v,ii,&parent_ndx,&right,&depth);
    RETURN_IF(node_ndx == NULLNODE, valnil,0);
    vec_mapnode_t *node = getnode(v,node_ndx);
    return getvalue(node);
  }
  else {
    uint32_t i = valtointeger(ii);

    if (i == VECNONDX) i = v->end - 1; // Get the last element
    RETURN_IF(i >= v->end, valnil, ERANGE);

    x = v->vec[i];
  }
  return x;
}

val_t vecgetfirst(val_t vv)
{
  vec_t v;
  errno = 0;
  val_t x;
  uint32_t first = 0;
  int vtype;

  RETURN_IF(!valisvec(vv), valnil, EINVAL);
  v = (vec_t)valtovec(vv);

  RETURN_IF(v->end == 0, valnil, 0);

  vtype = v->flg & 0x0F;
  if (vtype == VEC_ISMAP) {
    return valnil;
  }
  else {
    if (vtype == VEC_ISQUEUE) first = v->fst;
    else if (vtype == VEC_ISSTACK) first = v->end - 1;
    x = v->vec[first];
  }
  return x;
}

val_t vecgetnext(val_t v);
val_t vecgetmin(val_t vv);
val_t vecgetmax(val_t vv);

uint32_t vecdel_3_(val_t vv, val_t ii, uint32_t j)
{
  uint32_t l,i;
  vec_t v;
  errno = 0;
  
  RETURN_IF(!valisvec(vv), 0, EINVAL);
  RETURN_IF(vecisempty(vv), 0, ERANGE);

  v = (vec_t)valtocleanpointer(vv);

  if (vectype(vv) == VEC_ISMAP) return vecunmap_(vv,ii);

  i = valtointeger(ii);

  val_dbg("i = %d, j = %d, end = %d, sze = %d",i,j,v->end,v->sze);
  if (i == VECNONDX) { i = v->end-1; j = i; }
  else if (j == VECNONDX) j = i;
  if (i >= v->end || j<i) return 0;

  if (j >= v->end-1) {
    freevecs(v->vec,i,v->end);
    v->end = i; // Just drop last elements
  }
  else {
    freevecs(v->vec,i,j+1);

    //       __l__
    //    ABCdefghIJKLM              ABCIJKLM
    //    |  |    |    |             |  |    |    
    //    0  i   i+l   end           0  i    end-l  

    l = (j-i)+1;
    memmove(&(v->vec[i]) , &(v->vec[i+l]),  (v->end-(i+l)) * sizeof(val_t));
    v->end -= l;
  }
  val_dbg("i = %d, j = %d, end = %d, sze = %d",i,j,v->end,v->sze);
  return(v->end);
}

uint32_t vecdrop_2(val_t vv, uint32_t n) {
  vec_t v;
  uint32_t cnt;
  errno = 0;

  RETURN_IF(!valisvec(vv), 0, EINVAL);

  cnt = veccount(vv);

  if (n != 0 && cnt != 0) {
    v = (vec_t)valtocleanpointer(vv);
    if (n >= cnt) n = cnt;

    freevecs(v->vec,v->end-n,v->end);

    v->end -= n;
    if (v->fst >= v->end) {
       v->fst = 0; v->end = 0; v->typ = VEC_ISVEC;
    }
    cnt = (v->end);
  }
  return cnt;
}

// Deque (removes the next n elements form the queue)
uint32_t vecdeq_2(val_t vv, uint32_t n) 
{
  uint32_t ret = 0;
  vec_t v;
  errno = 0;

  RETURN_IF(!valisvec(vv), 0, EINVAL);
  RETURN_IF(vectype(vv) != VEC_ISQUEUE, 0, EINVAL);
  
  ret = veccount(vv);

  if (ret > 0) {
    v = (vec_t)valtocleanpointer(vv);
    n += v->fst;
    if (n > v->end) n = v->end;

    freevecs(v->vec, v->fst, n);
    v->fst = n;
    if (v->fst >= v->end) {
      v->fst = 0; v->end = 0;
      v->typ = VEC_ISVEC;
    }
    ret = v->end - v->fst;
  }
  return ret;
}

uint32_t vecindex_3(val_t vv,uint32_t ndx, int32_t delta)
{
  uint32_t ret = VECERRORNDX;
  errno = 0;
  
  RETURN_IF(!valisvec(vv), VECERRORNDX, EINVAL);
  RETURN_IF(vecisempty(vv), VECERRORNDX, EINVAL);

  vec_t v = (vec_t)valtovec(vv);

  if (ndx < VECNONDX)          ret = ndx;
  else if (ndx == VECNONDX)    ret = ndx;
  else if (ndx == VECCOUNTNDX) ret = v->end;
  else if (ndx == VECSIZENDX)  ret = v->sze;
  else switch(vectype(vv)) {
    case VEC_ISSTACK: switch(ndx) {
                        case VECNEXTNDX:
                        case VECFIRSTNDX:
                        case VECTOPNDX:  ret = v->end-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ret) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = 0; 
                                         if (delta < 0) delta = -delta;
                                         if (delta > v->end-1) ret = VECERRORNDX;
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
                                         if (delta > ((v->end-1) - v->fst)) ret = VECERRORNDX;
                                         else ret += delta;
                                         break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = v->end-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ((v->end-1) - v->fst)) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;
    } 
    break;

    default: switch(ndx) {
                        case VECNEXTNDX:  ret = v->end;  break;

                        case VECHEADNDX:  
                        case VECFIRSTNDX: ret = 0;  break;

                        case VECTAILNDX:
                        case VECLASTNDX: ret = v->end-1;
                                         if (delta < 0) delta = -delta;
                                         if (delta > ret) ret = VECERRORNDX;
                                         else ret -= delta;
                                         break;

                        case VECCURNDX:  ret = v->cur; break;
    }
  }

  return ret;
}

int vec_cmp(const void *a, const void *b)
{
  int ret;
  ret = valcmp(*((val_t *)a),*((val_t *)b));
  val_dbg("comparing %016lX %016lX = %d",((val_t *)a)->v,((val_t *)b)->v, ret);
  return ret;
}

int vecsort_3(val_t vv, vec_cmp_t cmp, val_t aux)
{
  int ret = 0;
  errno = 0;

  RETURN_IF(!valisvec(vv), 1, EINVAL);
  RETURN_IF(vecisempty(vv), 0, 0);

  vec_t v = valtovec(vv);
  val_dbg("About to sort %p[%u]",(void *)v,v->end);

  val_dbg("[0] = %16lX",(v->vec[0]).v);
  qsort(v->vec,v->end,sizeof(val_t),vec_cmp);
  v->flg |= VEC_SORTED;
  return ret;
}

int vecissorted_(val_t vv)
{
  RETURN_IF(!valisvec(vv), 0, EINVAL);
  RETURN_IF(vecisempty(vv), 0, 0);

  vec_t v = valtovec(vv);

  return v->flg & VEC_SORTED;

}

uint32_t vec_search_sorted(vec_t v, val_t key)
{
  val_dbg("Searching in sorted");
  val_t *found = bsearch(&key, v->vec,v->end, sizeof(val_t), vec_cmp);
  RETURN_IF(!found, VECNONDX, 0);
  return (uint32_t)(found - v->vec);
}

uint32_t vec_search_linear(vec_t v, val_t key)
{
  val_dbg("Searching linear");
  for (uint32_t found = 0; found < v->end; found++)
    if (valcmp(v->vec[found], key) == 0) return found;
  return VECNONDX;
}

uint32_t vec_search_map(vec_t v, val_t key, uint32_t *parent_ndx, int *right,int *depth)
{
  uint32_t node_ndx = 0;
  vec_mapnode_t *node;
  *parent_ndx = VECNONDX;
  *right = 0;
  *depth = 0;
  if (!is_emptytree(v)) {
    node_ndx = getroot(v);
    val_dbg("SEARCHIG root: %d",node_ndx);
    while (node_ndx != NULLNODE) {
      node = getnode(v,node_ndx);
      val_dbg("node_ndx: %d left: %d right: %d",node_ndx, getleft(node), getright(node));
      int cmp = valcmp(key, getkey(node));

      if (cmp == 0) {
        val_dbg("FOUND! (%d)",*depth);
        return node_ndx;
      }

      *parent_ndx = node_ndx;
      *depth += 1;

      if (cmp < 0) {
        *right = 0;
        node_ndx = getleft(node);
      } else {
        *right = 1;
        node_ndx = getright(node);
      }
    }
  }
  
  val_dbg("NOT FOUND! (%d)",*depth);
  return VECNONDX;
}

uint32_t vecsearch_(val_t vv, val_t key)
{
  
  RETURN_IF(!valisvec(vv), VECNONDX, EINVAL);
  RETURN_IF(vecisempty(vv), VECNONDX, EINVAL);

  vec_t v = valtovec(vv);
  uint32_t parent_ndx;
  int right;
  int depth;
 
  if (v->flg & VEC_SORTED)
    return vec_search_sorted(v,key);
  else if (vectype(vv) == VEC_ISMAP)
    return vec_search_map(v,key, &parent_ndx, &right, &depth);
 else 
    return vec_search_linear(v,key);
}

#define MAXLEVEL 0xFFFFFFFF

#include <time.h>
#if 1
// Function to generate a random 32-bit number using Xorshift
static uint32_t RND() {
    static uint32_t state = 0; // A non-zero seed value
    if (state == 0) state = 2463534242 * (uint32_t)time(0);
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state = x;
    // if (x == MAXLEVEL) x -= 1;
    return x & 0x7FFFFFFF;
}
#endif

#if 0
// RNG by burtleburtle http://burtleburtle.net/bob/rand/smallprng.html

#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
#define RND_() \
    e = a - rot(b, 27); \
    a = b ^ rot(c, 17);\
    b = c + d;\
    c = d + e;\
    d = e + a;

uint32_t RND( ) {
    static uint32_t a = 0;
    static uint32_t b,c,d;
    uint32_t e;

    if (a == 0) {
      a = 0xf1ea5eed, b = c = d = (uint32_t)time(0);
      for (int i=0; i<20; i++) {
        RND_();
      }
    }
    RND_();
    return d;
}
#endif

#if 0
// Generate 1 random bit (for 50% choices)
static int RNDBIT() {
  static int n = 0;
  static uint32_t state = 0;

  if (n==0) { state = RND(); n = 32; }
  n -= 1;
  return (state & (1<<n));
}
#endif

static uint32_t vec_newmapnode(vec_t v)
{
  uint32_t newnode;
  vec_mapnode_t *node;

  newnode = v->end;
  RETURN_IF(!makeroom(v, newnode+VALS_PER_MAPNODE), VECNONDX, ENOMEM);
  v->end += VALS_PER_MAPNODE;
  node = getnode(v,newnode);

  setleft(node, NULLNODE);
  setright(node, NULLNODE);
  setparent(node, NULLNODE);
  setkey(node, valnil);
  setvalue(node, valnil);
  setlevel(node, RND());

  return newnode;
}

static void moveup(vec_t v, uint32_t node_ndx, uint32_t parent_ndx, int is_right_child)
{
  vec_mapnode_t *parent_node;
  vec_mapnode_t *node;
  uint32_t grandparent;
  vec_mapnode_t *grandparent_node;
  uint32_t chld_ndx;
  int right_next = 0;

  node = getnode(v, node_ndx);
  parent_node = getnode(v,parent_ndx);

  while (parent_ndx != NULLNODE && getlevel(node) >= getlevel(parent_node)) {
    grandparent = getparent(parent_node);
    setparent(node, grandparent);

    if (grandparent != NULLNODE) {
      grandparent_node = getnode(v,grandparent);
      if (getleft(grandparent_node) == parent_ndx) {
        setleft(grandparent_node, node_ndx);
        right_next = 0;
      }
      else {
        setright(grandparent_node, node_ndx);
        right_next = 1;
      }
    }
    else {
      grandparent_node = NULL;
      setroot(v, node_ndx);
      val_dbg("New root: %d", node_ndx);
    }

    if (is_right_child) { // node is parent's right children ROTATE LEFT!
        chld_ndx = getleft(node);
        setright(parent_node, chld_ndx); val_dbg("Setting right children to %d",chld_ndx);
        setleft(node, parent_ndx);
        val_dbg("ROTATED LEFT");
    } else {
        chld_ndx = getright(node); 
        setleft(parent_node, chld_ndx); val_dbg("Setting left children to %d",chld_ndx);
        setright(node, parent_ndx);
        val_dbg("ROTATED RIGHT");
    }
    if (chld_ndx != NULLNODE) setparent(v,chld_ndx,parent_ndx);
    setparent(parent_node, node_ndx);
    parent_ndx = grandparent;
    parent_node = grandparent_node;
    is_right_child = right_next;
  }
}

uint32_t vecmap_(val_t vv, val_t key, val_t value)
{
  RETURN_IF(!valisvec(vv), NULLNODE, EINVAL);
  vec_t v = valtovec(vv);
  uint32_t node_ndx = NULLNODE;
  uint32_t parent_ndx = NULLNODE;
  int is_right_child = 0;
  int depth = 0;
  vec_mapnode_t *node;
  vec_mapnode_t *parent_node;

  int t = vectype(vv);

  if (t == VEC_ISMAP && valisnil(value)) return vecunmap_(vv,key);

  vectype(vv,VEC_ISMAP);

  if (!is_emptytree(v)) {
    node_ndx = vec_search_map(v,key,&parent_ndx,&is_right_child,&depth);
  }
  else val_dbg("INSERT INTO EMPTY TREE");
  
  if (node_ndx == NULLNODE) {
    val_dbg("Adding a new node");
    node_ndx = vec_newmapnode(v);
    RETURN_IF(node_ndx == NULLNODE, NULLNODE, ENOMEM);
  }

  node = getnode(v,node_ndx);
  setkey(node, key);
  setvalue(node, value);
  setparent(node, parent_ndx);

  // Insert the node as a leaf
  if (parent_ndx != NULLNODE) {
    parent_node = getnode(v,parent_ndx);
    if (is_right_child) {
      setright(parent_node, node_ndx);
      val_dbg("MAP INSERT RIGHT parent: %d depth:%d (lvl: %d))",parent_ndx, depth, node_lvl);
    }
    else {
      setleft(parent_node, node_ndx);
      val_dbg("MAP INSERT LEFT parent: %d depth:%d (lvl: %d))",parent_ndx, depth, node_lvl);
    }
  }
  else {
    val_dbg("MAP INSERT ROOT parent: %d depth:%d (lvl: %d))",parent_ndx, depth, node_lvl);
    return node_ndx;
  }

  moveup(v, node_ndx, parent_ndx, is_right_child);

  return node_ndx;
}
      

uint32_t vecunmap_(val_t vv, val_t key)
{
  RETURN_IF(!valisvec(vv), NULLNODE, EINVAL);
  RETURN_IF(vectype(vv) != VEC_ISMAP, NULLNODE, EINVAL);

  vec_t v = valtovec(vv);
  uint32_t node_ndx = NULLNODE;
  uint32_t parent_ndx = NULLNODE;
  int is_right_child = 0;
  int depth = 0;
  vec_mapnode_t *node;
  vec_mapnode_t *parent_node;
  uint32_t left_level = 0, right_level = 0;

  errno = 0;
  
  if (is_emptytree(v)) return NULLNODE;

  node_ndx = vec_search_map(v,key,&parent_ndx,&is_right_child,&depth);

  if (node_ndx == NULLNODE) return NULLNODE;

  // If there is only one node, just remove it
  if (getnumnodes(v) == 1) {
    v->end = 0;
    setroot(v,NULLNODE);
    return NULLNODE;
  }

  node = getnode(v,node_ndx);

#if 0
  if (node_ndx != getroot(v)) {
    setlevel(node,MAXLEVEL);
    moveup(v, node_ndx, parent_ndx, is_right_child);
    val_dbg("Moved up %d",node_ndx);
    vec_printtree(v,stdout);
  }
  else val_dbg("Already at root");
#endif


  // starting from node 
  // if the node is a leaf, remove it and break
  // if the node has a single child, remove it and attach child to its grandparent and breal
  // else, rotate the node (moving it down) so that the children with highest level moves up

  // take the last node and copy it in the freed node.
  uint32_t grandparent_ndx;
  vec_mapnode_t *grandparent_node;
  uint32_t child_ndx;
  vec_mapnode_t *child_node;

  #define set_grandparent() \
      if (grandparent_ndx != NULLNODE) { \
        grandparent_node = getnode(v,grandparent_ndx); \
        if (getright(grandparent_node) == node_ndx) \
          setright(grandparent_node, child_ndx); \
        else \
          setleft(grandparent_node, child_ndx); \
      } \
      else { \
        setroot(v,child_ndx); \
      } \

  while (1) {
    val_dbg("Moving down %d",node_ndx);
    grandparent_ndx = getparent(node);
    
    child_ndx = getleft(node);
    if (child_ndx == NULLNODE) {  // either it's a leaf or only has a right child
      val_dbg("[ No left children");
      child_ndx = getright(node);

      set_grandparent();      

      if (child_ndx != NULLNODE) {  // Node only has a right child
        child_node = getnode(v, child_ndx);
        setparent(child_node, grandparent_ndx);
      }
      else val_dbg("No right children ]");

      break;
    }
    else {
      // Either it has two children or only has the left child
      if (getright(node) == NULLNODE) {
         val_dbg("[ No right children");
        // Only has a left child
  
        set_grandparent();      
  
        child_node = getnode(v, child_ndx);
        setparent(child_node, grandparent_ndx);
        break;
      }
      else {
        // Has both left and right child
        val_dbg("Both children");
        // rotate so that the children with highest level moves up
        left_level = getlevel(v,getleft(node));
        right_level = getlevel(v,getright(node));
        
        if (right_level > left_level)  child_ndx = getright(node); // Rotate left
        else  child_ndx = getleft(node);           // Rotate right

        set_grandparent();      

        child_node = getnode(v,child_ndx);
        setparent(child_node,grandparent_ndx);
        if (right_level > left_level) {
          setright(node,getleft(child_node));
          setleft(child_node,node_ndx);
        }
        else {
          setleft(node,getright(child_node));
          setright(child_node,node_ndx);
        }
        setparent(node,child_ndx);
      }
    }
  }

  // Fix the last node
  child_ndx = v->end - 4; // index of last element
  if (node_ndx != child_ndx) {
    child_node = getnode(v,child_ndx);
    *node = *child_node;
    parent_ndx = getparent(child_node);
    if (parent_ndx == NULLNODE) {
      setroot(v,node_ndx);
    }
    else {
      parent_node = getnode(v,parent_ndx);
      if (getleft(parent_node) == child_ndx)
        setleft(parent_node, node_ndx);
      else
        setright(parent_node, node_ndx);
    }
  }
  
  decnumnodes(v); // Decrement the number of nodes by one

  return 0;
}

void vec_printtree(vec_t v, FILE *f)
{
  fprintf(f,"graph BST {\nnode [shape=circle];\n");
  vec_mapnode_t *node;
  for (int k=0; k < v->end; k+=4) {
    node = getnode(v,k);

    fprintf(f,"%d [label=\"",k);
    valprintf(getkey(node),f);
    fprintf(f,"\n%08X",getlevel(node));
    fprintf(f,"\"];\n");

    fprintf(f,"%d -- ",k);

    if (getleft(node) != NULLNODE) fprintf(f,"%d",getleft(node));
    else fprintf(f,"L%d",k);

    fprintf(f," [label=\"L\"];\n");
    
    fprintf(f,"%d -- ",k);

    if (getright(node) != NULLNODE) fprintf(f,"%d",getright(node));
    else fprintf(f,"R%d",k);

    fprintf(f," [label=\"R\"];\n");

    if (getleft(node) == NULLNODE) fprintf(f,"L%d [shape=point];\n",k);
    if (getright(node) == NULLNODE) fprintf(f,"R%d [shape=point];\n",k);

  }

  fprintf(f,"}\n");
  fflush(f);

}

void vecprinttree(val_t vv, FILE *f)
{
  if (!valisvec(vv)) {errno = EINVAL; return ;};
  vec_t v = valtovec(vv);
  vec_printtree(v,f);
}

val_t vecmapfirst_(val_t vv, val_t *key)
{
  if (!valisvec(vv)) {errno = EINVAL; return valnil ;};
  vec_t v = valtovec(vv);
  
  uint32_t node_ndx = getroot(v);
  vec_mapnode_t *node;
  uint32_t child_ndx;
  while (1) {
    node = getnode(v, node_ndx);
    unmarknode(node);
    child_ndx = getleft(node);
    if (child_ndx == NULLNODE) {
      v->cur = node_ndx;
      marknode(node);
      if (key) *key = getkey(node);
      return (getvalue(node));
    }
    node_ndx = child_ndx;
  }
  return valnil;
}

val_t vecmapnext_(val_t vv, val_t *key)
{
  if (!valisvec(vv)) {errno = EINVAL; return valnil;};

  if (vectype(vv) != VEC_ISMAP) return valnil;

  vec_t v = valtovec(vv);

  if (getnumnodes(v) == 0) return valnil;

  uint32_t node_ndx = getroot(v);
  vec_mapnode_t *node;

  *key = valnil;

  if (v->end == 0 || v->cur >= v->end || v->cur & 0x03) return valnil;
  node_ndx = v->cur;
  node = getnode(v,node_ndx);
  if (getright(node) != NULLNODE) {
    node_ndx = getright(node);
    do {
      v->cur = node_ndx;
      node = getnode(v,node_ndx);
      unmarknode(node);
      node_ndx = getleft(node);
    } while (node_ndx != NULLNODE);
    marknode(node);
    *key = getkey(node);
    return getvalue(node);
  }
  else {
    node_ndx = getparent(node);
    do {
      node = getnode(v, node_ndx);
      if (!ismarkednode(node)) {
        v->cur = node_ndx;
        marknode(node);
        *key = getkey(node);
        return getvalue(node);
      }
      node_ndx = getparent(node);
    } while (node_ndx != NULLNODE);
  }

  return valnil;
}
