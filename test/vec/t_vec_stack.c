//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include "val.h"


tstsuite("Stack tests",nolarge)
{
     val_t v=valnil;
     val_t x=valnil;

     tstcase("Setting values as a stack") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(valtointeger(vecpush(v,100)) == 100);
      tstcheck(veccount(v) == 1);
      tstcheck(valtointeger(vecpush(v,200)) == 200);
      tstcheck(veccount(v) == 2);
      tstcheck(valtointeger(vecpush(v,300)) == 300);
      tstcheck(veccount(v) == 3);
      tstcheck(valeq(vectop(v),300));
      v = vecfree(v);
      tstcheck(valeq(vecpush(v,400),valerror) && errno == EINVAL);
    }

    tstcase("Getting value from a stack") {
      tstassert(!valisnil((v = vecnew())));
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(!vecisempty(v));
      tstcheck(valeq(vectop(v),300));
      tstcheck(vecpop(v) == 2);
      tstcheck(valeq(vectop(v),200));
      tstcheck(vecpop(v) == 1);
      tstcheck(valeq(vectop(v),100));
      tstcheck(vecpop(v) == 0);
      tstcheck(vecisempty(v));
      tstcheck(valisnil(vectop(v)));
      v = vecfree(v);
      tstcheck(vecisempty(v));
      tstcheck(valisnil(vectop(v)));
    }

    tstcase("Peeking values from a stack") {
      tstassert(!valisnil((v = vecnew())));
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(veccount(v) == 3);
      tstcheck(!vecisempty(v));
      tstcheck(valeq(vectop(v),300));
      tstcheck(valeq(vectop(v,-1),200));
      tstcheck(valeq(vectop(v,1),200));
      tstcheck(valeq(vectop(v,-2),100));
      tstcheck(valeq(vectop(v,2),100));

      tstcheck(valisnil(vectop(v,-3)));
      tstcheck(valisnil(vectop(v,3)));

      tstcheck(valisnil(vectop(v,-332)));
      tstcheck(valisnil(vectop(v,332)));

      v = vecfree(v);
      tstcheck(vecisempty(v));
      tstcheck(valisnil(vectop(v)));
      tstcheck(valisnil(vectop(v,-1)));
      tstcheck(valisnil(vectop(v,-2)));
    }

    tstcase("Deleting elements from the top (single)") {
      v = vecnew();
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(veccount(v) == 3);
      x = vectop(v);
      tstcheck(valeq(x,300));
      tstcheck(vecpop(v) == 2);
      tstcheck(veccount(v) == 2);
      x = vectop(v);
      tstcheck(valeq(x,200));
      tstcheck(vecpop(v) == 1);
      tstcheck(veccount(v) == 1);

      v = vecfree(v);
      tstcheck(vecisempty(v));
      x = vectop(v);
      tstcheck(valisnil(x));
    }
    
    tstcase("Deleting elements from the top (multiple)") {
      v = vecnew();
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(veccount(v) == 3);
      vecpop(v);
      x = vectop(v);
      vecpop(v);
      tstcheck(veccount(v) == 1);
      tstcheck(valeq(x,200));
  
      x = vectop(v);
      vecpop(v);
      tstcheck(veccount(v) == 0);
      tstcheck(valeq(x,100));
      vecpop(v,2);
      x = vectop(v);
      tstcheck(veccount(v) == 0);
      tstcheck(valisnil(x));

      // Drop from an empty stack
      vecpop(v,2);
      tstcheck(veccount(v) == 0);

      v = vecfree(v);

      // Drop from an freed stack
      tstcheck(vecpop(v,2) == 0 && errno != 0);
      tstcheck(veccount(v) == 0);
      tstcheck(valisnil(x));
    }

    tstcase("large stack") {
      tstskipif(tsttag(nolarge)) {
        tstassert(!valisnil((v = vecnew())));
        for (int k=1000; k< 1100 && !errno; k++) vecpush(v,k);
        tstcheck(!vecisempty(v));
        tstcheck(veccount(v) == 100);

        for (int k=1099; !vecisempty(v); k--) {
           tstcheck(valeq((x=vectop(v)),k), "expected %d got %d",k,(int)valtointeger(x));
           vecpop(v);
        }
        tstcheck(vecisempty(v));
  
        v = vecfree(v);
      }
    }

    if (!valisnil(v)) v=vecfree(v);
}