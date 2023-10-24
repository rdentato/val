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

     tstcase("Setting values as a stack") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecpush(v,100) == 0);
      tstcheck(veccount(v) == 1);
      tstcheck(vecpush(v,200) == 1);
      tstcheck(veccount(v) == 2);
      tstcheck(vecpush(v,300) == 2);
      tstcheck(veccount(v) == 3);
      tstcheck(valeq(vectop(v),300));
      v = vecfree(v);
      tstcheck(vecpush(v,400) == VECNONDX && errno == EINVAL);
    }

    tstcase("Getting value from a stack") {
      tstassert(!valisnil((v = vecnew())));
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(!vecisempty(v));
      tstcheck(valeq(vecpop(v),300));
      tstcheck(valeq(vecpop(v),200));
      tstcheck(valeq(vecpop(v),100));
      tstcheck(vecisempty(v));
      tstcheck(valisnil(vecpop(v)));
      v = vecfree(v);
      tstcheck(vecisempty(v));
      tstcheck(valisnil(vecpop(v)));
    }

    tstcase("Getting value from a stack") {
      tstassert(!valisnil((v = vecnew())));
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(veccount(v) == 3);
      tstcheck(!vecisempty(v));
      vecclear(v);
      tstcheck(vecisempty(v));
      tstcheck(valisnil(vectop(v)));
      v = vecfree(v);
      tstcheck(vecisempty(v));
    }

    tstcase("Deleting elements from the top (single)") {
      v = vecnew();
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(veccount(v) == 3);
      x = vecpop(v);
      tstcheck(veccount(v) == 2);
      tstcheck(valeq(x,300));
      x = vecpop(v);
      tstcheck(veccount(v) == 1);
      tstcheck(valeq(x,200));
      vecclear(v);
      tstcheck(vecisempty(v));
      tstcheck(valisnil(vectop(v)));
      x = vecpop(v);
      tstcheck(valisnil(x));
      v = vecfree(v);
      tstcheck(vecisempty(v));
      x = vecpop(v);
      tstcheck(valisnil(x));
    }
    
    tstcase("Deleting elements from the top (multiple)") {
      v = vecnew();
      vecpush(v,100);
      vecpush(v,200);
      vecpush(v,300);
      tstcheck(veccount(v) == 3);
      x = vecdrop(v,2);
      tstcheck(veccount(v) == 1);
      tstcheck(valeq(x,200));
  
      x = vecdrop(v,2);
      tstcheck(veccount(v) == 0);
      tstcheck(valeq(x,100));

      x = vecdrop(v,2);
      tstcheck(veccount(v) == 0);
      tstcheck(valisnil(x));

      // Drop from an empty stack
      x = vecdrop(v,2);
      tstcheck(veccount(v) == 0);
      tstcheck(valisnil(x));

      v = vecfree(v);

      // Drop from an freed stack
      x = vecdrop(v,2);
      tstcheck(veccount(v) == 0);
      tstcheck(valisnil(x));
    }

    tstgroup(tsttag(large)) {
      tstcase("large stack") {
        tstassert(!valisnil((v = vecnew())));
        for (int k=1000; k< 1100; k++)
           vecpush(v,k);
        tstcheck(!vecisempty(v));
        tstcheck(veccount(v) == 100);
  
        for (int k=1099; k>= 1000; k--)
           tstcheck(valeq((x=vecpop(v)),k), "expected %d got %d",k,(int)valtointeger(x));
        tstcheck(vecisempty(v));
  
        v = vecfree(v);
      }
    }

  }

  if (!valisnil(v)) v=vecfree(v);
}