//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include "val.h"

// Checks if a pointer is less than 48 bits.
// This is an heurisitic and does not really guarantee that a pointer will fit in 48 bits.
// It's just better than nothing.
// This is critical, if the test fail the val library is unusable

int a00;
int fits48(void *p) { return ((uint64_t)((uintptr_t)p) >> 48) == 0; }

tstsuite("Val Library ptr check", core) {
  tstcase("Check static objects") {
    int ptr_exceed_48 = 0;
    ptr_exceed_48 |= !fits48(&a00);    // a global variavle
    ptr_exceed_48 |= !fits48((void*)(&valnil));    // a global variavle
    ptr_exceed_48 |= !fits48(&ptr_exceed_48);    // a local variavle
    ptr_exceed_48 |= !fits48(fits48);  // a user function
    ptr_exceed_48 |= !fits48(fprintf); // a library function
    ptr_exceed_48 |= !fits48(atoi); // a library function
    ptr_exceed_48 |= !fits48(strcpy); // a library function

    char *p = malloc(1024);
    ptr_exceed_48 |= !fits48(p); // an allocated pointer
    free(p);

    tstcheck(!ptr_exceed_48);
  }
}