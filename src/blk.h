//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#ifndef BLK_VERSION
#define BLK_VERSION 0x0001000B

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

typedef struct blk_s {
  uint8_t *blk;
  uint32_t cnt;
  uint32_t sze;
} *blk_t;

val_t blknew(uint32_t sze)
{
  blk_t blk;
  blk = 
  return val(blk);
}
#endif
