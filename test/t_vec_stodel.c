//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "vallib.h"

typedef struct {
  int x;
  int y;
} point2D_t;

typedef struct {
  int x;
  int y;
  int z;
} point3D_t;

tstsuite("Store") {

  tstcase("Stored strings") {
    val_t x;
    val_t y;
    val_t z;
    tstnote("SIZE: %d CNT: %d",val_store_sze(0),val_store_cnt(0));
    tstcheck(val_store_cnt(0) == 0);
    x = valstore("pippopippo");
    y = valstore("topolinotopolino");
    z = valstore("plutopluto");
    tstcheck(val_store_cnt(0) == 3);
    tstnote("SIZE: %d CNT: %d TOP: %d",val_store_sze(0),val_store_cnt(0),val_store_top(0));


    int32_t x_old_offset = val_store_offset(0,1);
    int32_t y_old_offset = val_store_offset(0,2);
    int32_t z_old_offset = val_store_offset(0,3);

    tstnote("x: %d y: %d z: %d",x_old_offset, y_old_offset, z_old_offset);

    char *x_p = valrecall(x);
    char *y_p = valrecall(y);
    char *z_p = valrecall(z);

    tstcheck(strcmp(x_p,"pippopippo") == 0);
    tstcheck(strcmp(y_p,"topolinotopolino") == 0);
    tstcheck(strcmp(z_p,"plutopluto") == 0);

    valdel(y);
    tstnote("SIZE: %d CNT: %d TOP: %d",val_store_sze(0),val_store_cnt(0),val_store_top(0));
    tstcheck(strcmp(x_p,"pippopippo") == 0);
    tstcheck(valrecall(y) == NULL);
    tstcheck(strcmp(z_p,"plutopluto") == 0);

    tstcheck(val_store_offset(0,2) == y_old_offset,"new_z: %d old_y: %d",val_store_offset(0,2),y_old_offset);
    tstcheck(val_store_cnt(0) == 3);

    val_t w = valstore("minnieminnie");
    tstcheck(val_store_cnt(0) == 4);
    tstnote("w: %d",val_store_offset(0,4));
  }
}
