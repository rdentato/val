//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "val.h"

#define N 20                // total number of elements
#define MAX_STR_LEN 8       // maximum length of generated strings

// Comparator for qsort: uses valcmp
static int cmp_val(const void *a, const void *b) {
    const val_t *va = (const val_t*)a;
    const val_t *vb = (const val_t*)b;
    return valcmp(*va, *vb);
}

// Generate a random lowercase ASCII string of length 1..MAX_STR_LEN
static char* random_string() {
    int len = 1 + rand() % MAX_STR_LEN;
    char *s = malloc(len + 1);
    if (!s) {
        perror("malloc");
        exit(1);
    }
    for (int i = 0; i < len; i++) {
        s[i] = 'a' + (rand() % 26);
    }
    s[len] = '\0';
    return s;
}

tstsuite("Sorting") {
    srand((unsigned)time(NULL));

    val_t arr[N];

    // Fill array with random ints or strings
    for (int i = 0; i < N; i++) {
        if (rand() % 2 == 0) {
            // random integer 0..99
            arr[i] = val(rand() % 100);
        } else {
            arr[i] = val(random_string());
        }
    }

    // Sort
    qsort(arr, N, sizeof(val_t), cmp_val);

    for (int i = 1; i < N; i++) {
      tstcheck(valcmp(arr[i-1],arr[i]) <= 0);
      if (valisint(arr[i-1]) && valisint(arr[i]))
        tstcheck(valtoint(arr[i-1]) <= valtoint(arr[i]));

      if (valischarptr(arr[i-1]) && valischarptr(arr[i]))
        tstcheck(strcmp(valtoptr(arr[i-1]),valtoptr(arr[i])) <= 0);
    }

        // Free allocated strings
    for (int i = 0; i < N; i++) {
        if (valischarptr(arr[i])) {
            free(valtoptr(arr[i]));
        }
    }


}
