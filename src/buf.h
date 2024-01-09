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

#include "val.h"

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
} *buf_t;

extern int64_t bufallocatedmem;

#define BUFNONDX  0xFFFFFFFFFFFFFFFF
#define BUFMAXNDX 0xFFFFFFFFFFFFFFF0

#define bufnew(...) bufnew_(__VA_ARGS__ +0)
val_t bufnew_(uint32_t sze);
val_t buffree(val_t b);

uint32_t bufsize(val_t b, uint32_t sze);

#define buftell(...) BUF_vrg(buftell_,__VA_ARGS__)
#define buftell_1(b) buftell_2(b,SEEK_CUR)
uint32_t buftell_2(val_t b, int start);

uint32_t bufseek(val_t b, int32_t pos);
uint32_t bufcut(val_t b, uint32_t n);

uint32_t bufwrite(val_t b, void *src, uint32_t n);
uint32_t bufprintf(val_t b, const char *fmt, ...);
uint32_t bufputs(val_t b, const char *src);
uint32_t bufputc(val_t b, int c);

uint32_t bufload(val_t b, FILE *f, uint32_t n);
uint32_t bufloadln(val_t b, FILE *f);

static inline char *buf(val_t b) { return (valisbuf(b)) ? ((buf_t)valtocleanpointer(b))->buf : NULL;}


#endif
