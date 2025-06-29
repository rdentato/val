//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "vallib.h"


tstsuite("vectors") {
  tstcase("Create and destroy (0)") {
    val_t vct = valvec();
    
    tstcheck(valisvecptr(vct));
    vct = valfree(vct);
    tstcheck(!valisvecptr(vct));
    tstcheck(valisnullptr(vct));
 
    tstcheck(valsize(vct) == 0);
    tstcheck(valcount(vct) == 0);
  }

  tstcase("Create and destroy (100)") {
    val_t vct = valvec(100);
    
    tstcheck(valisvecptr(vct)); 

    tstcheck(valtoptr(vct) != NULL);
 
    tstcheck(((valptr_vec_t)valtoptr(vct))->vec != NULL );
    tstnote("Size 100 -> %d", valsize(vct));
    tstcheck(valsize(vct) >= 100, "Instead of 100, size is: %" PRId32,valsize(vct));
    tstcheck(valcount(vct) == 0);

    vct = valfree(vct);
    tstcheck(!valisvecptr(vct));
    tstcheck(valisnullptr(vct));
  }

  tstcase("Access as an array") {
    val_t vct = valvec();
    
    tstcheck(valisvecptr(vct));
    tstcheck(valcount(vct) == 0);

    tstcheck(valset(vct,0,99));
    tstcheck(valcount(vct) == 1);
    
    tstcheck(valset(vct,13,98));
    tstcheck(valcount(vct) == 14);
    
    tstcheck(valset(vct,130,98));
    tstcheck(valcount(vct) == 131);
    
    val_t elm = valget(vct,13);

    tstcheck(!valisnil(elm));
    tstcheck(valisint(elm));
    tstcheck(valtoint(elm) == 98);

    elm = valget(vct,0);

    tstcheck(!valisnil(elm));
    tstcheck(valisint(elm));
    tstcheck(valtoint(elm) == 99);

    elm = valget(vct, 10000);
    tstcheck(valisnil(elm));

    vct=valfree(vct);
  }

  tstcase("Delete from array") {
    val_t vct = valvec(100);
    
    tstcheck(valisvecptr(vct));
    tstcheck(valcount(vct) == 0);

    tstcheck(valset(vct,0,99));
    tstcheck(valcount(vct) == 1);
    
    for (int k=0; k< 100; k++)
      valset(vct,k,k+1000);

    tstcheck(valcount(vct) == 100);

    tstcheck(valeq(valget(vct,99), 1099));
    tstcheck(valeq(valget(vct,98), 1098));

    tstcheck("Delete the last element" && valdel(vct));

    tstcheck(valcount(vct) == 99);
    tstcheck(valeq(valget(vct,99), valnil));
    tstcheck(valeq(valget(vct,98), 1098));

    tstcheck(valeq(valget(vct,0), 1000),"v[0] = %d",(int)valtoint(valget(vct,0)));
    tstcheck("Delete the first element" && valdel(vct,0));
    tstcheck(valeq(valget(vct,0), 1001),"v[0] = %d",(int)valtoint(valget(vct,0)));
    tstcheck(valcount(vct) == 98);
    tstcheck(valeq(valget(vct,98), valnil));
    tstcheck(valeq(valget(vct,97), 1098));


    tstcheck(valeq(valget(vct,50), 1051),"v[0] = %d",(int)valtoint(valget(vct,0)));
    tstcheck("Delete in the middle" && valdel(vct,50,59));
    tstcheck(valeq(valget(vct,50), 1061),"v[0] = %d",(int)valtoint(valget(vct,0)));
    tstcheck(valcount(vct) == 88);
    tstcheck(valeq(valget(vct,87), 1098));
    

    vct=valfree(vct);
  }

}
