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

    uint32_t key;
    val_t n;

    #define N 25
    tstcase("many keys") {
      veccount(v,0);
      for (int i=1000; i<1000+N; i++) {
        key = i;
        //tstnote("key: %d",key);
        vecmap(v,key,100+i);
        //tstassert(errno == 0);
      }
    }

    tstcase("traversing the key/value pairs") {

      n = vecfirst(v);
      tstcheck(valeq(n,1000+100),"first: %016lX",n.v);

      val_t k;
      n = vecfirst(v,&k);
      tstcheck(valeq(k,1000),"first key: %016lX",k.v);
      tstcheck(valeq(n,1000+100),"first: %016lX",n.v);

      n = vecnext(v,&k);
      tstcheck(valeq(k,1001),"next key: %016lX",k.v);
      tstcheck(valeq(n,1001+100),"next: %016lX",n.v);

      n = vecnext(v,&k);
      tstcheck(valeq(k,1002),"next key: %016lX",k.v);
      tstcheck(valeq(n,1002+100),"next: %016lX",n.v);

      uint32_t d = 1003;
      while (1) {
        n = vecnext(v,&k);
        if (valisnil(n)) break;
        tstcheck(valeq(k,d),"next key: %016lX (%ld %d) %016lX %d",k.v, valtointeger(k), d, val(d).v, k.v == val(d).v);
        tstcheck(valeq(n,d+100),"next: %016lX (%ld %d)",n.v, valtointeger(n), d+100);
        d++;
      }

    }


    if (valisvec(v)) v=vecfree(v);
    tstassert(valisnil(v));
}