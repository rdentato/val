//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#define VEC_MAIN
#include "vec.h"

tst_suite("Check owning reference") {

    val_t v = valnil;
    val_t x = valnil;

    tstcase("Creating a ref") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(vectype(v) == VEC_ISVEC);
      x = vecown(v);
      tstcheck(vectype(v) == VEC_ISVEC,"v: %016lX (%02X) owned: %016lX (%02X)", v.v, vectype(v), x.v, vectype(x));
      tstcheck(vectype(v) == vectype(x));
      tstcheck(!valeq(v,x));
      tstcheck(valtovec(v) == valtovec(x));
      v = vecfree(v);
      tstcheck(valisnil(v) && (errno == EINVAL));
      x = vecfree(x);
      tstcheck(valisnil(x) && (errno == 0));
    }

    if (valisvec(v)) v=vecfree(v);
    tstcheck(valisnil(v));
}