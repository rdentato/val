//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "vallib.h"


tstsuite("vectors") {

  tstcase("Access as a stack") {
    val_t vct = valvec();
    
    tstcheck(valisvecptr(vct));
    tstcheck(valcount(vct) == 0);

    tstcheck(valpush(vct,99));
    tstcheck(valcount(vct) == 1);
    
    for (int k=0; k< 100; k++)
      valpush(vct,k+1000);

    tstcheck(valcount(vct) == 101);
    
    val_t x;
    x = valtop(vct);
    tstcheck(errno == 0);
    tstcheck(valeq(x,1099));

    tstcheck(valeq(valpop(vct),1099));
    tstcheck(valeq(valpop(vct),1098));
    tstcheck(valeq(valpop(vct),1097));

    tstcheck(valcount(vct) == 98);

    valdel(vct,-3,-1);

    tstcheck(valcount(vct) == 95);
    tstcheck(valeq(valtop(vct),1093));

    vct=valfree(vct);
  }

  tstcase("Empty Stack") {
    val_t vct = valvec(100);
    
    tstcheck(valisempty(vct));
    val_t x;

    x = valtop(vct);
    tstcheck(valisnil(x) && errno != 0);

    errno =0;
    x = valpop(vct);
    tstcheck(valisnil(x) && errno != 0);
    vct=valfree(vct);
  } 

  tstcase("Not a Stack") {
    val_t vct = valconst(100);
    
    tstcheck(valisempty(vct) && errno != 0);
    val_t x;

    x = valtop(vct);
    tstcheck(valisnil(x) && errno != 0);

    errno =0;
    x = valpop(vct);
    tstcheck(valisnil(x) && errno != 0);


    tstcheck(valpush(vct,3) == 0);
    tstcheck(valcount(vct) == 0 && errno != 0);

    vct=valfree(vct);
  } 

}
