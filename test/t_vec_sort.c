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

// Comparator for qsort: uses valcmp
static int cmp_inv(const void *a, const void *b) {
    const val_t *va = (const val_t*)a;
    const val_t *vb = (const val_t*)b;
    return -valcmp(*va, *vb);
}

// Generate a random lowercase ASCII string of length 1..MAX_STR_LEN
static char* random_string() {
    static char s[2*MAX_STR_LEN];
    int len = 1 + rand() % MAX_STR_LEN;

    for (int i = 0; i < len; i++) {
        s[i] = 'a' + (rand() % 26);
    }
    s[len] = '\0';
    return s;
}

tstsuite("Sorting") {
    srand((unsigned)time(NULL));

    val_t arr = valvec();
    val_t arr_inv = valvec();

    // Fill array with random ints or strings
    for (int i = 0; i < N; i++) {
        if (rand() % 2 == 0) {
            // random integer 0..99
            int n = rand() % 100;
            valset(arr,i, n);
            valset(arr_inv,i, n);
        } else {
            val_t n = valconst(random_string());
            valset(arr,i, n);
            valset(arr_inv,i, n);
        }
    }

    valsort(arr);
    valsort(arr_inv,cmp_inv);

    for (int i = 1; i < N; i++) {
      tstcheck(valcmp(valget(arr,i),valget(arr,i)) <= 0);
      tstcheck(valcmp(valget(arr_inv,i),valget(arr_inv,i)) >= 0);
    }

    for (int i = 0; i < N; i++) {
      tstcheck(valeq(valget(arr,i),valget(arr_inv,N-i-1)),"%2d - arr: '%s' arr_inv: '%s'",i,valtostr(valget(arr,i)).str,valtostr(valget(arr_inv,N-i-1)).str);
    }
}
