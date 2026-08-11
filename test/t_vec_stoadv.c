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
    val_t y;
    tstnote("SIZE: %d CNT: %d",val_store_sze(0),val_store_cnt(0));
    tstcheck(val_store_cnt(0) == 0);
    y = valstore("pippo");
    tstcheck(val_store_cnt(0) == 1);
    tstcheck(val_store_len(0,1) == 6);
    tstcheck(val_store_offset(0,1) == 0);
    tstnote("SIZE: %d CNT: %d",val_store_sze(0),val_store_cnt(0));

    y = valstore("pippopippopippo");
    tstcheck(val_store_cnt(0) == 2);
    tstnote("SIZE: %d CNT: %d",val_store_sze(0),val_store_cnt(0));
    tstcheck(val_store_len(0,1) == 6);
    tstcheck(val_store_offset(0,1) == 0);
    tstcheck(val_store_len(0,2) == 16);
    tstcheck(val_store_offset(0,2) == 8);
    tstcheck(valisstored(y));
 

#if 0
    tstcheck(valisstored(y));
    tstcheck(valisstoredstr(y));
    x = valrecall(y);
    tstcheck(x != NULL);
    tstcheck(x != NULL && x[0] == 'p' && errno == 0 );
    tstcheck(strcmp(x,"pippo") == 0);
    val_t z = valstore("pluto");
    x = valrecall(z);
    tstcheck(x != NULL);
    tstcheck(x != NULL && x[0] == 'p' && errno == 0 );
    tstcheck(strcmp(x,"pluto") == 0);
    val_t w = valstore("topolino",4);
    x = valrecall(w);
    tstcheck(x != NULL);
    tstcheck(x != NULL && x[0] == 't' && errno == 0 );
    tstcheck(strcmp(x,"topo") == 0, "Got: %s",x);
#endif
  }
}
