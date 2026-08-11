//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "vallib.h"

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

  tstcase("arena growth keeps all strings intact") {
    enum { N = 100 };
    val_t vals[N];
    char expected[32];

    for (int i = 0; i < N; i++) {
      char buf[32];
      snprintf(buf, sizeof buf, "string-%03d", i);
      vals[i] = valstore(buf);
      tstcheck(valisstored(vals[i]));
    }

    // Every previously stored string must still recall intact (the arena
    // grows across the stores above, so this also exercises the realloc path).
    for (int i = 0; i < N; i++) {
      snprintf(expected, sizeof expected, "string-%03d", i);
      char *p = valrecall(vals[i]);
      tstcheck(p != NULL && strcmp(p, expected) == 0);
    }

    // And they must behave as strings
    tstcheck(valcmp(vals[0], val("string-000")) == 0);
    tstcheck(valcmp(vals[N-1], val("string-099")) == 0);
    tstcheck(valhash(vals[0]) == valhash(val("string-000")));
  }
}
