//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#include "val.h"


tstsuite("sort tests")
{
    val_t v=valnil;
    // val_t x=valnil;

    tstassert(!valisnil((v = vecnew())));

    tstcase("sort integers") {
      vecadd(v,20);
      vecadd(v,10);
      vecadd(v,40);
      vecadd(v,30);

      tstcheck(veccount(v)==4);

      tstcheck(valeq(vecget(v,0),val(20)),"[0]: %ld",valtointeger(vecget(v,0)));
      tstcheck(vecsort(v) == 0);
      tstcheck(valeq(vecget(v,0),val(10)),"[0]: %ld",valtointeger(vecget(v,0)));
      tstcheck(valeq(vecget(v,1),val(20)),"[1]: %ld",valtointeger(vecget(v,0)));
      tstcheck(valeq(vecget(v,2),val(30)),"[2]: %ld",valtointeger(vecget(v,0)));
      tstcheck(valeq(vecget(v,3),val(40)),"[3]: %ld",valtointeger(vecget(v,0)));

    }

    tstcase("Search integers") {
      uint32_t ndx;
      tstcheck((ndx=vecsearch(valnil,20)) == VECNONDX && (errno != 0));
      tstcheck((ndx=vecsearch(v,20)) == 1);
      tstcheck((ndx=vecsearch(v,1)) == VECNONDX && (errno == 0));
    }

    tstcase("sort doubles") {
      veccount(v,0); // clear up the vector
      vecadd(v,20.2);
      vecadd(v,10.1);
      vecadd(v,40.4);
      vecadd(v,30.3);

      tstcheck(veccount(v)==4);

      tstcheck(valeq(vecget(v,0),val(20.2)),"[0]: %f",valtodouble(vecget(v,0)));
      tstcheck(vecsort(v) == 0);
      tstcheck(valeq(vecget(v,0),val(10.1)),"[0]: %f",valtodouble(vecget(v,0)));
      tstcheck(valeq(vecget(v,1),val(20.2)),"[1]: %f",valtodouble(vecget(v,1)));
      tstcheck(valeq(vecget(v,2),val(30.3)),"[2]: %f",valtodouble(vecget(v,2)));
      tstcheck(valeq(vecget(v,3),val(40.4)),"[3]: %f",valtodouble(vecget(v,3)));
    }

    tstcase("sort strings") {
      veccount(v,0); // clear up the vector
      vecadd(v,"20.2");
      vecadd(v,"10.1");
      vecadd(v,"40.4");
      vecadd(v,"30.3");

      tstcheck(veccount(v)==4);

      tstcheck(valeq(vecget(v,0),val("20.2")),"[0]: %s",valtostring(vecget(v,0)));
      tstcheck(vecsort(v) == 0);
      tstcheck(valeq(vecget(v,0),val("10.1")),"[0]: %s",valtostring(vecget(v,0)));
      tstcheck(valeq(vecget(v,1),val("20.2")),"[1]: %s",valtostring(vecget(v,1)));
      tstcheck(valeq(vecget(v,2),val("30.3")),"[2]: %s",valtostring(vecget(v,2)));
      tstcheck(valeq(vecget(v,3),val("40.4")),"[3]: %s",valtostring(vecget(v,3)));
    }
 
    tstcase("Search strings") {
      uint32_t ndx;
      tstcheck((ndx=vecsearch(valnil,"30.3")) == VECNONDX && (errno != 0));
      tstcheck((ndx=vecsearch(v,"30.3")) == 2);
      tstcheck((ndx=vecsearch(v,"1")) == VECNONDX && (errno == 0));

      char xx[] = "30.3";
      void *x = xx;

      tstcheck((ndx=vecsearch(v,x)) == VECNONDX && (errno == 0),"ndx: %u errno: %d",ndx,errno); // because a pointer is not considered a string!
      tstcheck((ndx=vecsearch(v,(char *)x)) == 2 && (errno == 0),"ndx: %u errno: %d",ndx,errno); // because is a string!

    }

    tstcase("unsorted") {
      uint32_t ndx;
      veccount(v,0);
      srand(time(0));
      for (int k=0; k<9999; k++)
        vecadd(v,rand() & 0xFFFF);
      tstassert(veccount(v) == 9999);
      tstcheck(!vecissorted(v))
      val_t n = vecget(v,40); // n is surely in the vector
      tstclock("Unsorted") {
        tstcheck((ndx=vecsearch(v,n)) != VECNONDX);
      }
      vecsort(v);
      tstcheck(vecissorted(v))
      tstclock("Sorted") {
        tstcheck((ndx=vecsearch(v,n)) != VECNONDX);
      }
      vecadd(v,rand());
      tstcheck(!vecissorted(v))

    }

    if (!valisnil(v)) v=vecfree(v);
}