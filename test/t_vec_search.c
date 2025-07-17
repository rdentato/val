//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "vallib.h"

#define N 20                // total number of elements
#define MAX_STR_LEN 8       // maximum length of generated strings

char *list[] = {
"ah" , "dcto", "acgposs", "fpvausi", "ftc", "pippo",
"cwpusnz", "chqxjeb", "topolino", "uzww", "hqjjfgy", "pe"
};

#define list_len (sizeof(list)/sizeof(list[0]))

tstsuite("Searching") {

  val_t arr = valvec();

  // Fill array with strings
  for (int i = 0; i < list_len; i++) {
    valset(arr,i, list[i]);
  }

  tstcase("Search") {
    valsort(arr);

    tstnote("SIZE: %lu",sizeof(struct valptr_vec_s));
    tstcheck(valbsearch(arr,"pippo") >= 0);
    tstcheck(valbsearch(arr,"pluto") < 0 && errno == 0 );

    int32_t pos;
    pos = valbsearch(arr,"pippo");
    tstcheck(pos >= 0);
    tstcheck(valcmp(valget(arr,pos),"pippo") == 0);

    tstcheck(valbsearch(arr,"topolino") >= 0);
    tstcheck(valbsearch(valconst(3),"topolino") < 0);
    tstcheck(valbsearch(valnil,"topolino") < 0 && errno != 0);
  }

  tstcase("Stored") {
    char *x;
    val_t y;
    y = valstore("pippo");
    x = valrecall(valnil);
    tstcheck(x == NULL && errno == EINVAL );
  }
}
