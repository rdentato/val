//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#define VEC_MAIN
#include "vec.h"

tstsuite("Check owning reference") {
    val_t v = valnil;
    val_t x = valnil;
    val_t y = valnil;

    tstcase("Creating a ref") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(vectype(v) == VEC_ISVEC);
      tstcheck(valisvec(x = vecref(vecnew())));

      tstcheck((x.v & 1) == 1);
      tstcheck(valisref(x));

      tstcheck(!valisnil(x=vecfree(x))); // Won't free anything

      vecset(v,1,x);

      v = vecfree(v); // will also free x
    }

    tstcase("unref of a variable") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(vectype(v) == VEC_ISVEC);
      tstcheck(valisvec(x = vecref(vecnew())));

      tstcheck((x.v & 1) == 1);
      tstcheck(valisref(x));

      tstcheck(!valisnil(x=vecfree(x))); // Won't free anything
      y = x;
      tstcheck(!valeq((y=vecunref(x)),x));

      tstcheck((y.v | 1) == x.v);

      tstcheck(valisnil(y=vecfree(y))); // will free y (former x)

      v = vecfree(v); // will also free x
    }
    
    
    tstcase("unref of an element in the vector") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(vectype(v) == VEC_ISVEC);

      tstcheck(valisvec((x = vecset(v,1,vecref(vecnew())))));
      tstcheck(valisref(x));
      
      vecsetfree(v,1);

      tstassert(valisref(vecget(v,1)));

      x = vecfree(vecget(v,1));
      tstcheck(!valisnil(x));

      tstassert(valisref(vecget(v,1)));

      v = vecfree(v); // will also free x
    }
    
    tstcase("unref of an element in the vector") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(vectype(v) == VEC_ISVEC);

      tstcheck(valisvec((x = vecset(v,1,vecref(vecnew())))));
      tstcheck(valisref(x));
      
      vecsetfree(v,1);

      tstassert(valisref(vecget(v,1)));

      x = vecfree(vecget(v,1));
      tstcheck(!valisnil(x));

      tstassert(valisref(vecget(v,1)));

      v = vecfree(v); // will also free x
    }
        //if (valisvec(v)) v=vecfree(v);
    tstcheck(valisnil(v));
}