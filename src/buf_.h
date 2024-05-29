//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#ifndef BUF_VERSION
#define BUF_VERSION 0x0001000B

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>

#ifndef VAL_VERSION
#include "val_.h"
#endif

#define BUF_cnt(x1,x2,x3,x4,xN, ...) xN
#define BUF_n(...)       BUF_cnt(__VA_ARGS__, 4, 3, 2, 1, 0)
#define BUF_join(x ,y)   x ## y
#define BUF_cat(x, y)    BUF_join(x, y)
#define BUF_vrg(f, ...)  BUF_cat(f, BUF_n(__VA_ARGS__))(__VA_ARGS__)
#define BUF_VRG(f, ...)  BUF_cat(f, BUF_n(__VA_ARGS__))(__VA_ARGS__)

typedef struct buf_s {
  char    *buf;
  uint32_t sze;
  uint32_t end;
  uint32_t pos;
  uint32_t aux;
} *buf_t;

extern int64_t bufallocatedmem;

#define BUFNONDX   0xFFFFFFFF
#define BUFLOADALL 0xFFFFFFF1
#define BUFLOADLN  0xFFFFFFF2
#define BUFMAXNDX  0xFFFFFFF0

#define bufnew(...) bufnew_(__VA_ARGS__ +0)
val_t bufnew_(uint32_t sze);
val_t buffree(val_t bb);

uint32_t bufsize(val_t bb, uint32_t sze);

#define bufpos(...) BUF_vrg(bufpos_,__VA_ARGS__)
uint32_t bufpos_1(val_t bb);
uint32_t bufpos_2(val_t bb, int32_t pos);

#define buflen(...) BUF_vrg(buflen_,__VA_ARGS__)
#define buflen_1(bb) buflen_2(bb,BUFMAXNDX)
uint32_t buflen_2(val_t bb, uint32_t len);

#define bufsize(...) BUF_vrg(bufsize_,__VA_ARGS__)
#define bufsize_1(bb) bufsize_2(bb,0)
uint32_t bufsize_2(val_t bb, uint32_t sze);

uint32_t bufprintf(val_t bb, const char *fmt, ...);

#define bufputs(...) BUF_vrg(bufputs_,__VA_ARGS__)
#define bufputs_2(bb,s) bufputs_3(bb,s,0)
uint32_t bufputs_3(val_t bb, const char *src, uint32_t len);

#define bufload(...) BUF_vrg(bufload_,__VA_ARGS__)
#define bufload_1(bb) bufload_3(bb,0,stdin)
#define bufload_2(bb, f) bufload_3(bb,0,f)
uint32_t bufload_3(val_t bb, uint32_t n, FILE *f);

uint32_t bufloadln(val_t bb, FILE *f);

#define bufsave(...) BUF_vrg(bufsave_,__VA_ARGS__)
#define bufsave_1(bb) bufsave_3(bb,0,stdout)
#define bufsave_2(bb,f) bufsave_3(bb,0,f)
uint32_t bufsave_3(val_t bb, uint32_t n, FILE *f);

uint32_t bufsaveln(val_t bb, FILE *f);

#define buf(...) BUF_vrg(buf_,__VA_ARGS__)
#define buf_1(b) buf_2(b,BUFMAXNDX)
char *buf_2(val_t bb, uint32_t start);

#define bufdel(...) BUF_vrg(bufdel_,__VA_ARGS__)
#define bufdel_1(bb) bufdel_2(bb,BUFMAXNDX)
uint32_t bufdel_2(val_t bb, uint32_t len);

uint32_t bufins_n(val_t bb,uint32_t len);
uint32_t bufins_s(val_t bb,char *str);

#define bufins(b,x) _Generic((x),          char *: bufins_s, \
                                  unsigned char *: bufins_s, \
                                           void *: bufins_s, \
                                              int: bufins_n, \
                                     unsigned int: bufins_n, \
                                            short: bufins_n, \
                                   unsigned short: bufins_n, \
                                             long: bufins_n, \
                                    unsigned long: bufins_n  \
                            ) (b,x)

#define bufsearch(...) BUF_vrg(bufsearch_,__VA_ARGS__)
#define bufsearch_2(b,s) bufsearch_3(b,s,BUFMAXNDX)
uint32_t bufsearch_3(val_t buf, char *str, uint32_t start);

#define BUF_STORES_NUM 4

extern val_t buf_stores[BUF_STORES_NUM];
extern uint8_t buf_stores_cnt[BUF_STORES_NUM];

#define bufstore(...) BUF_vrg(bufstore_,__VA_ARGS__)
#define bufstore_1(v) bufstore_2(0,v)
val_t bufstore_2(int sto, char *s);

#define bufclearstore(...) bufclearstore_(__VA_ARGS__ + 0)
void bufclearstore_(int sto);

#define bufclearstores() for (int sto=0; sto<BUF_STORES_NUM; sto++) bufclearstore_(sto)

#endif
