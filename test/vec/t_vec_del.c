//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#include "val.h"


tstsuite("Check delete") {
    val_t v = valnil;
    // uint32_t n;
    // val_t x;

    v = vecnew();
    tstassert(!valisnil(v));

    tstcheck(vectype(v) == VEC_ISVEC);
    tstcheck(veccount(v) == 0);

    int key;
    val_t n;

    tstcase("many keys") {
      veccount(v,0);
      #define N 25
      for (int i=00; i<N; i++) {
        key = i;
        valdbg("key: %d",key);
        vecmap(v,i,100+i);
        //tstassert(errno == 0);
      }
    }

    tstcase("deleting a non existent key") {
      tstcheck(vecunmap(v,145) == VECNONDX);
      tstcheck(veccount(v) == N);
    }

    tstcase("deleting an existent key") {
      int deleted_key = 16;
      vecprinttree(v,stdout);
      tstcheck(vecdel(v,deleted_key) == 0);
      // vecprinttree(v,stdout);

      tstcheck(veccount(v) == (N-1));
      tstcheck(valisnil(vecget(v,deleted_key)));

      for (int i = 0; i< N; i++) {
        key = i;
        n = vecget(v,i);
        if (key == deleted_key) {
          tstassert(valisnil(n));
        }
        else {
          tstassert(!valisnil(n),"Key: %d not found",key);
          tstcheck(valeq(n,100+i),"key: %d val: %lu (%016lX)",i,valtointeger(n),n.v);
        }
      }
   }


    if (valisvec(v)) v=vecfree(v);
    tstassert(valisnil(v));
}