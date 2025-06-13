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

// Checks if NAN (which is the value returned by expressions 
// like 0.0/0.0) is what we expect.
// This is critical, if the test fail the val library is unusable

tstsuite("Val Library NaN check", core) {
  tstcase("Check positive and negative NaN") {
    uint64_t pos_u;
    uint64_t neg_u;
    double   d;

    int pos_NaN_OK;
    int neg_NaN_OK;

    tstassert(sizeof(d) == sizeof(pos_u));

    d = (double)NAN;
    memcpy(&pos_u,&d,sizeof(d));
    pos_u &= VAL_TYPE_MASK;
    pos_NaN_OK = tst(pos_u == VAL_DBLNAN_POS);
    tstcheck(pos_NaN_OK,"Positive NaN: %016" PRIX64 "", pos_u);
 
    d = -(double)NAN;
    memcpy(&neg_u,&d,sizeof(d));
    neg_u &= VAL_TYPE_MASK;
    neg_NaN_OK = tst(neg_u == VAL_DBLNAN_NEG);
    tstcheck(neg_NaN_OK,"Negative NaN: %016" PRIX64 "", neg_u);

    if (!(pos_NaN_OK && neg_NaN_OK)) {
      tstnote("************************************************");
      tstnote("*                                              *");
      tstnote("*     CRITICAL ERROR. NaN values mismatch      *");
      tstnote("*                                              *");
      tstnote("*           This arch          Required        *");
      tstnote("*    NaN %016" PRIX64"   %016" PRIX64 "   *", pos_u,VAL_DBLNAN_POS);
      tstnote("*   -NaN %016" PRIX64"   %016" PRIX64 "   *", neg_u,VAL_DBLNAN_NEG);
      tstnote("*                                              *");
      tstnote("*  Check latest version or contact the author. *");
      tstnote("************************************************");
    }
  }
}