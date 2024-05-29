//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include "val.h"


tstsuite("Queue test cases",nolarge)
{
    val_t v=valnil;
    val_t x= valnil;
    uint32_t n;

    tstcase("Setting values as a queue") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(valisvec(v) && !vecisqueue(v));
      tstcheck(valtointeger(vecenq(v,100)) == 100);
      tstcheck(vecisqueue(v));
      tstcheck(veccount(v) == 1);
      tstcheck(valtointeger(vecenq(v,200)) == 200);
      tstcheck(veccount(v) == 2);
      tstcheck(valtointeger(vecenq(v,300)) == 300);
      tstcheck(veccount(v) == 3);
      v = vecfree(v);
      tstcheck(valiserror(vecenq(v,240)) && errno == EINVAL);
    }

    // An empty vec has no type.
    tstcase("Deleting and type") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(valisvec(v));
      tstcheck(!vecisqueue(v));
      tstcheck(valtointeger(vecenq(v,100)) == 100);
      tstcheck(vecisqueue(v));
      tstcheck(vecdeq(v) == 0);
      tstcheck(!vecisqueue(v));
      tstcheck(valtointeger(vecenq(v,200)) == 200);
      tstcheck(vecisqueue(v));
      tstcheck(vecdrop(v) == 0);
      tstcheck(!vecisqueue(v));
      v = vecfree(v);
    }

    tstcase("Getting value from a que") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(valtointeger(vecenq(v,100)) == 100);
      tstcheck(valtointeger(vecenq(v,200)) == 200);
      tstcheck(valtointeger(vecenq(v,300)) == 300);
      tstcheck(veccount(v) == 3,"Expected 3 got %d",veccount(v));
      tstcheck(valeq((x = vecfirst(v)),100), "got: %016lX",x.v); 
      tstcheck((n = vecdeq(v)) == 2); 
      tstcheck(veccount(v) == 2,"Expected 2 got %d",veccount(v));
      tstcheck(valeq((x = vecfirst(v)),200)); 
      tstcheck((n = vecdeq(v)) == 1); 
      tstcheck(veccount(v) == 1);
      tstcheck(valeq((x = vecfirst(v)),300)); 
      tstcheck((n = vecdeq(v)) == 0); 
      tstcheck(veccount(v) == 0);
      tstcheck(!valisnil(x));
      
      tstcheck(valisnil((x = vecfirst(v)))); 
      v = vecfree(v);
      tstcheck(valisnil((x = vecfirst(v)))); 
    }

    tstcase("Peeking value from a que") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(valtointeger(vecenq(v,100)) == 100);
      tstcheck(valtointeger(vecenq(v,200)) == 200);
      tstcheck(valtointeger(vecenq(v,300)) == 300);
      tstcheck(veccount(v) == 3,"Expected 3 got %d",veccount(v));

      tstcheck(valeq((x=vecfirst(v)),100), "got: %016lX",x.v); 

      tstcheck(valeq((x=vecfirst(v,-1)),200), "got: %016lX",x.v); 
      tstcheck(valeq((x=vecfirst(v,1)),200), "got: %016lX",x.v); 

      tstcheck(valeq((x=vecfirst(v,-2)),300), "got: %016lX",x.v); 
      tstcheck(valeq((x=vecfirst(v,2)),300), "got: %016lX",x.v); 

      tstcheck(valeq((x=veclast(v)),300), "got: %016lX",x.v); 

      tstcheck(valeq((x=veclast(v,1)),200), "got: %016lX",x.v); 
      tstcheck(valeq((x=veclast(v,-1)),200), "got: %016lX",x.v); 

      tstcheck(valeq((x=veclast(v,2)),100), "got: %016lX",x.v); 
      tstcheck(valeq((x=veclast(v,-2)),100), "got: %016lX",x.v); 

      tstcheck(valisnil(vecfirst(v,3)));
      tstcheck(valisnil(veclast(v,3)));

      tstcheck(valisnil(vecfirst(v,130)));
      tstcheck(valisnil(veclast(v,130)));

      tstcheck((n = vecdeq(v,3)) == 0); 
      
      tstcheck(valisnil((x = vecfirst(v)))); 
      tstcheck(valisnil((x = veclast(v)))); 
    
      tstcheck(valisnil((x = vecfirst(v,2)))); 
      tstcheck(valisnil((x = veclast(v,2)))); 

      v = vecfree(v);

      tstcheck(valisnil((x = vecfirst(v)))); 
      tstcheck(valisnil((x = veclast(v)))); 

      tstcheck(valisnil((x = vecfirst(v,2)))); 
      tstcheck(valisnil((x = veclast(v,2)))); 
    }

#if 0
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
      uint32_t n;
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(veccount(v) == 3,"Expected 3 got %d",veccount(v));
      tstcheck(vecdrop(v,2) == 1,"Expected 1 got %d",n);
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
      uint32_t n;
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecenq(v,100) == 0);
      tstcheck(vecenq(v,200) == 1);
      tstcheck(vecenq(v,300) == 2);
      tstcheck(vecenq(v,400) == 3);
      tstcheck((n=vecdrop(v,2) == 2), "Expected %d got %d",veccount(v),n);
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
#endif

    tstcase("large queue") {
      tstskipif(tsttag(nolarge)) {
        tstassert(!valisnil((v = vecnew())));
        tstcheck(errno == 0);
        for (int k=0; k<100 && !errno; k++) vecenq(v,1000+k);
        tstcheck(veccount(v) == 100);
        int k=0; 
        do {
          tstcheck(valeq((x=vecfirst(v)),1000+k)); 
          k++;
        } while (vecdeq(v));
        tstcheck(k == 100);
        v = vecfree(v);
      }
    }
    if (!valisnil(v)) v=vecfree(v);
}
