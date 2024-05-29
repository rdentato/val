//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#include "val.h"


tstsuite("Check mapping") {
    val_t v = valnil;
    uint32_t n;
    val_t x;

    v = vecnew();
    tstassert(!valisnil(v));

    tstcheck(vectype(v) == VEC_ISVEC);
    tstcheck(veccount(v) == 0);

    tstcase("Map a number") {
        vecmap(v,3,30);
        tstcheck(vectype(v) == VEC_ISMAP);
        tstassert(errno == 0);
        tstcheck((n = veccount(v)) == 1, "count: %d (instead of 1)",n);
        x = vecget(v,3);
        tstcheck("Map retrieve" && (errno == 0));
        tstcheck(valeq(x,val(30)));
        tstcheck(errno == 0);
    }

    tstcase("Map a string") {
        vecmap(v,"3",300);
        tstcheck((n = veccount(v)) == 2, "count: %d (instead of 1)",n);
        x = vecget(v,"3");
        tstcheck("Map retrieve string key" && (errno == 0));
        tstcheck(valeq(x,val(300)));
        tstcheck(errno == 0);
    }

    tstcase("Change a value") {
      vecmap(v,"3",330);
      tstcheck((n = veccount(v)) == 2, "count: %d (instead of 2)",n);
      x = vecget(v,"3");
      tstcheck("Map retrieve string key" && (errno == 0));
      tstcheck(valeq(x,val(330)));
      tstcheck(errno == 0);
    }

    tstcase("many keys") {
      int key;
      veccount(v,0);
      #define N 30
      for (int i=00; i<N; i++) {
        key = i;
        valdbg("key: %d",key);
        vecmap(v,i,100+i);
        //tstassert(errno == 0);
      }
      vecprinttree(v,stdout);

      val_t n;
      tstclock("Search with tests") {
      for (int i = 0; i< N; i++) {
        key = i;
        n = vecget(v,i);
        tstassert(!valisnil(n));
        tstcheck(valeq(n,100+i),"key: %d val: %lu (%016lX)",i,valtointeger(n),n.v);
      }
      }
      tstclock("Search with no tests") {
      for (int i = 0; i< N; i++) {
        key = i;
        n = vecget(v,i);
      }
      }
    }
/*
    int vec_log2(int n);
    tstcase("logs") {
      tstcheck(vec_log2(0) == 0);
      tstcheck(vec_log2(1) == 0);
      tstcheck(vec_log2(2) == 1);
      tstcheck(vec_log2(3) == 1);
      tstcheck(vec_log2(4) == 2);
      tstcheck(vec_log2(5) == 2);
      tstcheck(vec_log2(0xFFFFFFFF) == 31);

    }
*/
    if (valisvec(v)) v=vecfree(v);
    tstassert(valisnil(v));
}