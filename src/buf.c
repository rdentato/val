//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "buf.h"
#include <stdalign.h>

#ifdef DEBUG
int64_t bufallocatedmem = 0;
#define add_mem(m) (bufallocatedmem += (m))
#define sub_mem(m) (bufallocatedmem -= (m))
#else 
#define add_mem(m) ((void)0)
#define sub_mem(m) ((void)0) 
#endif

// Ensure the buftor is large enough to store a value at index n
static int makeroom(buf_t b, uint32_t n)
{
  uint32_t new_sze;
  char *new_buf;

  errno = 0;
  val_dbg("buf Making room %p (%p)[%d]->[%d]",(void *)b,(void *)(b->buf),b->sze,n);

  valreturnif(!b, 0, EINVAL);
  valreturnif(n == 0, 1, 0); // success for sure!

  // There should be at least n+1 slots:
  n += 1;

  if (n <= b->sze) return 1; // There's enough room

  valreturnif(n > BUFMAXNDX,0,ERANGE); // Max number of elments in a buftor reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = b->sze ? b->sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1); // ensure is even
  
  valreturnif(new_sze <= n,0,ERANGE);

  val_dbg("MKROOM: realloc(%p,%lu) [%u]->[%u]",(b->buf),new_sze ,b->sze, new_sze);
  new_buf = realloc(b->buf, new_sze);
  val_dbg("MKROOM: got(%p,%d) [%d]",new_buf,new_sze ,new_sze);

  valreturnif(new_buf == NULL,0,ENOMEM);
  add_mem((new_sze - b->sze));
  // set the newly allocated area to 0;
  new_buf[b->end] = '\0';

  b->buf = new_buf;
  b->sze = new_sze;
  
  return 1;
}

val_t bufnew_(uint32_t sze) {
  errno = 0;
  buf_t b = aligned_alloc(4,sizeof(struct buf_s));
  if (b) {
    b->pos = 0;
    b->end = 0;
    b->sze = sze;
    b->buf = NULL;
    if (!makeroom(b,sze)) {free(b); b = NULL;}
  }
  valreturnif(b == NULL, valnil, ENOMEM);
  
  add_mem(sizeof(struct buf_s));
  memset(b,0,sizeof(struct buf_s));
  return val(b);
}

val_t buffree(val_t bb)
{
  buf_t b;
  errno = 0;
  valreturnif(!valisbuf(bb),bb,EINVAL);
  b = valtocleanpointer(bb); 
  sub_mem(b->sze);
  free(b->buf);
  sub_mem(sizeof(struct buf_s));
  free(b); 
  return valnil;
}

uint32_t buftell(val_t bb, int start)
{
  valreturnif(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  switch(start) {
    case SEEK_CUR: return b->pos;
    case SEEK_END: return b->end;
  }
  return 0;
}

uint32_t bufsize(val_t b, uint32_t sze);
uint32_t bufseek(val_t b, int32_t pos);
uint32_t bufcut(val_t b, uint32_t n);
