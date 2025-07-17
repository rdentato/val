//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "vallib.h"


tstsuite("vectors") {

  tstcase("Insert") {
    val_t vct = valvec();
    
    tstcheck(valisvecptr(vct));
    tstcheck(valcount(vct) == 0);

    #define N 50
    for (int k=0; k< 50; k++)
      valenq(vct,k+1000);

    tstcheck(valcount(vct) == N);

    valins(vct,3,2003);
    tstcheck(valeq(valget(vct,3),2003));
    tstcheck(valeq(valget(vct,4),1003));
    
    tstcheck(valcount(vct) == N+1);

    tstcheck(valeq(valget(vct,-1),1049));
    tstcheck(valeq(valget(vct,N),1049));

    tstcheck(valins(vct,4,3001,6));
    tstcheck(valcount(vct) == N+1+6);

    tstcheck(valeq(valget(vct,4),3001));
    tstcheck(valeq(valget(vct,5),3001));
    tstcheck(valeq(valget(vct,6),3001));
    tstcheck(valeq(valget(vct,7),3001));
    tstcheck(valeq(valget(vct,8),3001));
    tstcheck(valeq(valget(vct,9),3001));
    tstcheck(valeq(valget(vct,10),1003));
    
    tstcheck(valeq(valget(vct,-1),1049));
    tstcheck(valeq(valget(vct,N+1+6-1),1049));

    tstcheck(valins(vct, 999, 999));
    tstcheck(valcount(vct) == 1000,"Count is: %d",valcount(vct));
    
    tstcheck(valeq(valget(vct,999),999));
    tstcheck(valeq(valget(vct,998),0.0));
    tstcheck(valeq(valget(vct,N+1+6-1),1049));

    vct=valfree(vct);
  }


}
