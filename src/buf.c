//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "buf_.h"
#include <stdalign.h>

#ifndef RETURN_IF
#define RETURN_IF valreturnif
#endif

val_t buf_stores[BUF_STORES_NUM] = {valnil, valnil, valnil, valnil};
uint8_t buf_stores_cnt[BUF_STORES_NUM] = {0,0,0,0} ;

val_t bufstore_2(int sto, char *s)
{
  val_t v;

  sto &= 3;

  if (!valisbuf(buf_stores[sto])) buf_stores[sto] = bufnew();
  RETURN_IF(!valisbuf(buf_stores[sto]),valnil,EINVAL);

  bufputs(buf_stores[sto],"\0\0\0\0",4);   // Add 4 bytes for the aux

  v.v = VAL_STO_MASK | ((uint64_t)(buf_stores_cnt[sto]) << 40) |  ((uint64_t)sto) << 32 | bufpos(buf_stores[sto]);
  
  bufputs(buf_stores[sto],s);

  uint32_t n = bufpos(buf_stores[sto])+1;
  n = (4 - n % 4) % 4;
  val_dbg("n: %d",n);
  bufputs(buf_stores[sto],"\0\0\0\0",1+n); // Add \0 until next pos will be aligned with 4 bytes.

  return v;
}

void bufclearstore_(int sto)
{
  sto &= 3;
  buf_stores[sto] = buffree(buf_stores[sto]);
  buf_stores_cnt[sto] += 1;
}

// Ensure the buftor is large enough to store a value at index n
static int buf_makeroom(buf_t b, uint32_t n)
{
  uint32_t new_sze;
  char *new_buf;

  errno = 0;
  val_dbg("buf Making room %p (%p)[%d]->[%d]",(void *)b,(void *)(b->buf),b->sze,n);

  RETURN_IF(!b, 0, EINVAL);
  RETURN_IF(n == 0, 1, 0); // success for sure!

  // There should be at least n+1 bytes free:
  n += 1;

  if (n <= b->sze) return 1; // There's enough room

  RETURN_IF(n > BUFMAXNDX,0,ERANGE); // Max number of elments in a buftor reached.
  
  if (n >= 0xD085FAF0) new_sze = n; // an n higher than that would make the new size overflow
  else {
    new_sze = b->sze ? b->sze : 4;
    while (new_sze <= n) { 
      new_sze = (new_sze * 13) / 8; // (new_sze + (new_sze/2) + (new_sze/8));  
    }
  }
  
  new_sze += (new_sze & 1); // ensure is even
  
  RETURN_IF(new_sze <= n,0,ERANGE);

  val_dbg("MKROOM: realloc(%p,%lu) [%u]->[%u]",(b->buf),new_sze ,b->sze, new_sze);
  new_buf = realloc(b->buf, new_sze);
  val_dbg("MKROOM: got(%p,%d) [%d]",new_buf,new_sze ,new_sze);

  RETURN_IF(new_buf == NULL,0,ENOMEM);

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
    if (!buf_makeroom(b,sze)) {free(b); b = NULL;}
  }
  RETURN_IF(b == NULL, valnil, ENOMEM);
  
  memset(b,0,sizeof(struct buf_s));
  return val(b);
}

val_t buffree(val_t bb)
{
  buf_t b;
  errno = 0;
  RETURN_IF(!valisbuf(bb),bb,EINVAL);
  b = valtocleanpointer(bb); 

  free(b->buf);
  free(b); 
  return valnil;
}

uint32_t bufpos_1(val_t bb)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  return b->pos;    
}

uint32_t bufpos_2(val_t bb, int32_t pos)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (pos >= 0) {
    b->pos =  pos >  b->end ? b->end : pos;
  } else {
    b->pos = -pos >= b->end ? 0 : b->end + pos;
  }
  return b->pos;
}

uint32_t bufputs_3(val_t bb, const char *src, uint32_t len)
{

  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  RETURN_IF(src == NULL, 0, 0);

  buf_t b = valtocleanpointer(bb);

  if (len == 0) len = strlen(src);
  
  if (len > 0) {
    RETURN_IF(!buf_makeroom(b,b->pos+len), 0, ENOMEM);

    memmove(b->buf + b->pos, src, len);

    b->pos += len;
    if (b->pos >= b->end) {
      b->end = b->pos;
      b->buf[b->end] = '\0';
    }
  }

  return len;
}

uint32_t bufprintf(val_t bb, const char *fmt, ...)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  va_list args;
  int len;

  // Determine how much space is needed
  va_start(args, fmt);
  len = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  if (len > 0) {
    RETURN_IF(!buf_makeroom(b,b->pos+len), 0, ENOMEM);
    val_dbg("POS[end]: %d", b->buf[b->pos+len]);
    int oldend = b->buf[b->pos+len];
    va_start(args, fmt);
    len = vsnprintf(b->buf+b->pos,len+1,fmt,args);
    va_end(args);
    b->pos += len;
    b->buf[b->pos] = oldend;
    if (b->pos >= b->end) {
      b->end = b->pos;
      b->buf[b->end] = '\0';
    }
  }

  return len;
}

uint32_t bufsize_2(val_t bb, uint32_t sze)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (sze > b->sze) {
    RETURN_IF(!buf_makeroom(b,sze), 0, ENOMEM);
    b->sze = sze;
  }
  return b->sze;
}

char *buf_2(val_t bb, uint32_t start)
{
  RETURN_IF(!valisbuf(bb), NULL, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (start == BUFMAXNDX) start = b->pos;

  return b->buf+start;
}

uint32_t buflen_2(val_t bb, uint32_t n)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (n < BUFMAXNDX) {
    RETURN_IF(!buf_makeroom(b,n), 0, ENOMEM);
    b->end = n;
    b->pos = b->end;
    b->buf[b->end] = '\0';
  }

  return b->end;
}

uint32_t bufload_3(val_t bb, uint32_t n, FILE *f)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (n == 0) n = UINT32_MAX;
   
  uint32_t blksize = 128;
 
  if (n<blksize) blksize = n;

  uint32_t l = 0;
  uint32_t r;
  while (!feof(f) && l <= n) {
    RETURN_IF(!buf_makeroom(b,b->pos+blksize), 0, ENOMEM);

    r = fread(b->buf+b->pos,1,blksize,f);
    if (r == 0) break;
    l += r;
    b->pos += r;
    if (b->pos > b->end) {
      b->end = b->pos;
      b->buf[b->pos] = '\0';
    }
  }
  return l;
}


uint32_t bufloadln(val_t bb, FILE *f)
{
  int c;
  char last_c = '\0';
  
  RETURN_IF(!f || feof(f), 0, EINVAL);

  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  uint32_t l = 0;

  while ((c=fgetc(f)) != EOF) {

    if (c == '\n') break;

    RETURN_IF(!buf_makeroom(b,b->pos+32), 0, ENOMEM);

    last_c = b->buf[b->pos];
    b->buf[b->pos] = c;
    b->pos += 1;
    l += 1;
  }
  if (b->pos>0 && b->buf[b->pos-1] == '\r') {
    b->buf[b->pos-1] = last_c;
    b->pos -= 1;
    l -= 1;
  }

  if (b->pos >= b->end) {
    b->end = b->pos;
    b->buf[b->pos] = '\0';
  }
  return l;
}

uint32_t bufsave_3(val_t bb, uint32_t n, FILE *f)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (n == 0 || (b->pos + n) > b->end) n = b->end - b->pos;
  if (n > 0) {
    n = fwrite(b->buf + b->pos, 1, n, f);
    b->pos += n;
  }

  return n;
}

uint32_t bufdel_2(val_t bb, uint32_t len)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);
  
  if (b->pos == b->end) return 0;

  if (len == 0) return 0;
  
  if (len >= b->end - b->pos) {
    len = b->end - b->pos;
    b->end = b->pos;
    b->buf[b->end] = '\0';
    return len;
  }

  char *ptr = b->buf + b->pos;
  uint32_t len_to_move = b->end - (b->pos + len);
  memmove(ptr, ptr+len, len_to_move+1);

  b->end -= len;
  b->buf[b->end] = '\0';

  return len;
}

static int makegap(buf_t b, uint32_t l)
{
  uint32_t i = b->pos;

  val_dbg("entering makegap(%d,%d)",i,l);
  RETURN_IF(!buf_makeroom(b, i+l),0,ENOMEM);

  if (i < b->end) {
    /*                     __l__
    **  ABCDEFGH       ABC/-----\DEFGH
    **  |  |    |      |  |      |    |
    **  0  i    len    0  i     i+l   len+l */  
    memmove(b->buf+(i+l), b->buf+i, b->end-i);
  }
  b->pos += l;
  b->end += l;
  b->buf[b->end] = '\0';
  return 1;
}

uint32_t bufins_x(val_t bb, uint32_t len, char *fill)
{
  RETURN_IF(!valisbuf(bb), 0, EINVAL);
  buf_t b = valtocleanpointer(bb);

  uint32_t p = b->pos;

  RETURN_IF(!makegap(b,len), 0, ENOMEM);

  uint32_t k=0;
  while (p < b->pos) {
    b->buf[p++] = fill[k++];
    if (fill[k] == '\0') k = 0;
  }
  return(len);
}

uint32_t bufins_n(val_t bb, uint32_t n)
{
  if (n == 0) return 0;
  else return bufins_x(bb,n,"                                ");
}

uint32_t bufins_s(val_t bb, char *s)
{
  if (s == NULL || *s == '\0') return 0;
  else return bufins_x(bb,strlen(s),s);
}

uint32_t bufsearch_3(val_t bb, char *str, uint32_t start)
{
  RETURN_IF(!valisbuf(bb), BUFNONDX, EINVAL);
  buf_t b = valtocleanpointer(bb);

  if (start == BUFMAXNDX) start = b->pos;
 
  RETURN_IF(start >= b->end, BUFNONDX, 0);

  char *heystack = b->buf+start;
  char *found = strstr(heystack,str);

  RETURN_IF(found == NULL, BUFNONDX, 0);

  b->pos = (uint32_t)(found-heystack);
  return b->pos;
}

