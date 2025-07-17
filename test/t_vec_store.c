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
    char *x;
    val_t y;
    y = valstore("pippo");
    tstcheck(valisstored(y));
    x = valrecall(y);
    tstcheck(x != NULL);
    tstcheck(x != NULL && x[0] == 'p' && errno == 0 );
    tstcheck(strcmp(x,"pippo") == 0);
    val_t z = valstore("pluto");
    x = valrecall(z);
    tstcheck(x != NULL);
    tstcheck(x != NULL && x[0] == 'p' && errno == 0 );
    tstcheck(strcmp(x,"pluto") == 0);
  }

  tstcase("structures") {
    point2D_t x1,x2;

    x1 = (point2D_t){.x = 0x11335577, .y = 0x99BBDDFF};
    x2 = (point2D_t){.x = 0x01030507, .y = 0x191B1D1F};

    val_t v1;
    v1 = valstore(&x1,sizeof(point2D_t));

    point2D_t *p;
    p = valrecall(v1);
    tstcheck(p != NULL && p->x == x1.x);
    tstcheck(p != NULL && p->y == x1.y);

    val_t v2;
    v2 = valstore(&x2,sizeof(point2D_t));

    p = valrecall(v2);
    tstcheck(p != NULL && p->x == x2.x);
    tstcheck(p != NULL && p->y == x2.y);

    point3D_t q1,q2;

    q1 = (point3D_t){.x = 0x11335577, .y = 0x99BBDDFF, .z=0x01234567};
    q2 = (point3D_t){.x = 0x01030507, .y = 0x191B1D1F, .z=0x01234567};

    val_t w1;
    w1 = valstore(&q1,sizeof(point3D_t));

    point3D_t *q;
    q = valrecall(w1);
    tstcheck(q != NULL && q->x == q1.x);
    tstcheck(q != NULL && q->y == q1.y);
    tstcheck(q != NULL && q->z == q1.z);

    val_t w2;
    w2 = valstore(&q2,sizeof(point3D_t));

    q = valrecall(w2);
    tstcheck(q != NULL && q->x == q2.x);
    tstcheck(q != NULL && q->y == q2.y);
    tstcheck(q != NULL && q->z == q2.z);

  }
}
