//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#define VEC_MAIN
#include "vec.h"

tsttags(large);

int main(int argc, char *argv[])
{
  val_t v=valnil;
  val_t x=valnil;

  tstsettags(argc,argv);

  tstrun() {

    tstcase("Setting values as a queue") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(veccount(v) == 1);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(veccount(v) == 2);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(veccount(v) == 3);
      v = vecfree(v);
      tstcheck(vecenq(v,240) == VECNONDX && errno == EINVAL);
    }

    tstcase("Getting value from a que") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(veccount(v) == 3,"Expected 3 got %d",veccount(v));
      tstcheck(valeq((x = vecdeq(v)),100)); 
      tstcheck(veccount(v) == 2,"Expected 2 got %d",veccount(v));
      tstcheck(valeq((x = vecdeq(v)),200)); 
      tstcheck(veccount(v) == 1);
      tstcheck(valeq((x = vecdeq(v)),300)); 
      tstcheck(veccount(v) == 0);
      tstcheck(!valisnil(x));
      
      tstcheck(valisnil((x = vecdeq(v)))); 
      v = vecfree(v);
      tstcheck(valisnil((x = vecdeq(v)))); 
    }

    tstcase("Getting value from a que") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(veccount(v) == 3,"Expected 3 got %d",veccount(v));
      tstcheck(valeq((x = vecdeq(v,2)),200)); 
      tstcheck(veccount(v) == 1, "Expected 1 got %d",veccount(v));
      v = vecfree(v);
    }

    tstcase("Getting value from a que") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(veccount(v) == 3,"Expected 3 got %d",veccount(v));
      tstcheck(valeq(vecdrop(v,2),200));
      v = vecfree(v);
    }

    tstcase("Getting value from a que") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(vecenq(v,400) == 3);
      tstcheck(valeq((x = vecdeq(v,2)),200),"Expected 200 Got: %ld",valtointeger(x)); 
      tstcheck(veccount(v) == 2, "Expected 1 got %d",veccount(v));
      tstcheck(valeq((x = vectail(v)),400),"Expected 400 Got: %ld",valtointeger(x)); 
      val_t y;
      y = vechead(v);
      x = vecdeq(v);
      tstcheck(valeq(x,y),"Head: %ld  Deq: %ld",valtointeger(x),valtointeger(y));
      v = vecfree(v);
    }


    tstcase("Getting value from a que") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(vecenq(v,400) == 3);
      tstcheck(valeq((x = vecdrop(v,2)),300),"Expected 300 Got: %ld",valtointeger(x)); 
      tstcheck(veccount(v) == 2, "Expected 1 got %d",veccount(v));
      tstcheck(valeq((x = vechead(v)),100),"Expected 100 Got: %ld",valtointeger(x)); 
      val_t y;
      y = vechead(v);
      x = vecdeq(v);
      tstcheck(valeq(x,y),"Head: %ld  Deq: %ld",valtointeger(y),valtointeger(x));
      v = vecfree(v);
    }

    tstcase("Counting (queue)") {
      vec_t vv;
      uint32_t n;
      tstcheck(veccount(v,7)==0 && errno == EINVAL);
      tstassert(!valisnil((v = vecnew())));
      vecenq(v,100);
      vecenq(v,200);
      vecenq(v,300);
      tstcheck(veccount(v) == 3);
      tstcheck(valeq(vecdeq(v),100));
      tstcheck(veccount(v) == 2);
      vv = valtovec(v);
      tstcheck(vv->fst == 1 && vv->cnt == 3);
      tstcheck(veccount(v,1) == 1);
      tstcheck(vv->fst == 1 && vv->cnt == 2);
      tstcheck(valeq(vecdeq(v),200));
      
      tstcheck(veccount(v,7) == 7);
      tstcheck((n=vecsize(v)) > 7);
      tstcheck(veccount(v,5) == 5);
      tstcheck(veccount(v) == 5);
      v = vecfree(v);      
      tstcheck(veccount(v,7)==0 && errno == EINVAL);
    }

    tstgroup(tsttag(large), "Large queue test disabled") {
      tstcase("large queue") {
        tstassert(!valisnil((v = vecnew())));
        for (int k=0; k<100; k++) vecenq(v,1000+k);
        tstcheck(veccount(v) == 100);
        for (int k=0; k<100; k++) 
          tstcheck(valeq((x=vecdeq(v)),1000+k)); 
        v = vecfree(v);
      }
    }
  }

  if (!valisnil(v)) v=vecfree(v);
}