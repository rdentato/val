//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "vallib.h"


tstsuite("vectors") {

  tstcase("Access as a queue") {
    val_t vct = valvec();
    
    tstcheck(valisvecptr(vct));
    tstcheck(valcount(vct) == 0);

    tstcheck(valenq(vct,99));
    tstcheck(valcount(vct) == 1);
    
    for (int k=0; k< 100; k++)
      valenq(vct,k+1000);

    tstcheck(valcount(vct) == 101);
    
    val_t x;
    x = valfirst(vct);
    tstcheck(errno == 0);
    tstcheck(valeq(x,99));

    x = valdeq(vct);
    tstcheck(errno == 0);
    tstcheck(valeq(x,99));

    tstcheck(valeq((x=valdeq(vct)),1000),"Got x: %s",valtostr(x).str);
    tstcheck(valeq((x=valdeq(vct)),1001),"Got x: %s",valtostr(x).str);
    tstcheck(valeq((x=valdeq(vct)),1002),"Got x: %s",valtostr(x).str);

    tstcheck(valcount(vct) == 97);

    tstcheck(valeq(vallast(vct),1099));

    vct=valfree(vct);
  }

  tstcase("First movements") {
    val_t vct = valvec();
    valptr_vec_t vec_ptr = valtoptr(vct);

    for (int k=0; k<8; k++) 
      valenq(vct, k+1000);
    
    tstnote("fst: %d, end: %d, size: %d", vec_ptr->fst, vec_ptr->end, vec_ptr->sze);
    
    int32_t oldsize = valsize(vct);
    for (int k=valcount(vct); k < oldsize; k++) 
      valenq(vct, k+1000);
    
    tstnote("fst: %d, end: %d, size: %d", vec_ptr->fst, vec_ptr->end, vec_ptr->sze);

    int32_t oldcount = valcount(vct);
    int32_t oldfirst = -1;

    valfirst(vct,&oldfirst);
    tstcheck(oldfirst == 0);
    tstcheck(valeq(valdeq(vct),1000));
    
    tstcheck(valcount(vct) == oldcount -1 );
    vec_ptr = valtoptr(vct);
    tstcheck(vec_ptr->fst == oldfirst +1);

    int32_t newfirst = -1;

    valfirst(vct,&newfirst);
    tstcheck(newfirst == 0);
    vct = valfree(vct);
  }

  tstcase("Check sequence") {
    val_t vct = valvec();
    valptr_vec_t vec_ptr;

    for (int k=0; k<10; k++)  valenq(vct,1000+k);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);
    tstcheck(valcount(vct) == 10);

    for (int k=10; k<20; k++) valenq(vct,2000+k);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);
    tstcheck(valcount(vct) == 20);

    val_t x;
    x = valdeq(vct);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);
    tstcheck(valcount(vct) == 19);
    int32_t fst;
    valfirst(vct,&fst);
    tstcheck(fst == 0);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);
    tstcheck(valeq(x, 1000));

    tstcheck(valeq(valfirst(vct), 1001));

    int errs = 0;
    for (int k=1; k<6; k++) {
      if (!valeq(valdeq(vct),1000+k)) errs++;
    }
    tstcheck(errs == 0);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);
    
    for (int k=0; k<6; k++) valenq(vct,k);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);

    for (int k=0; k<6; k++) valdeq(vct);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);

    x = valfirst(vct);
    valptr_vec_t p;
    p = valtoptr(vct);

    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);
    val_t y = p->vec[p->fst];
    tstcheck(valeq(x, y));

    valenq(vct,5000);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);

    valenq(vct,5001);
    vec_ptr = valtoptr(vct);
    tstnote("first: %d last: %d count: %d size: %d",vec_ptr->fst,vec_ptr->end, valcount(vct),vec_ptr->sze);

  }

}
