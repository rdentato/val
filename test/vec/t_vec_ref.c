//  SPDX-FileCopyrightText: © 2023 Remo Dentato <rdentato@gmail.com>
//  SPDX-License-Identifier: MIT

#include "tst.h"

#include "val.h"


tstsuite("Check owning reference") {
    val_t v = valnil;
    val_t x = valnil;
    val_t y = valnil;

    tstcase("Check that owned vectors get freed up") {
        v = vecnew();
        tstassert(!valisnil(v));

        // Set an owned vector
        x = vecnew();
        tstassert(!valisnil(x));
        y = vecown(v,0,x);
        tstcheck((x.v | 1) == y.v );

        v = vecfree(v);
    }

    tstcase("Check that not owned vectors are not freed up") {
        v = vecnew();
        tstassert(!valisnil(v));

        // Set a non owned vector
        x = vecnew();
        tstassert(!valisnil(x));
        y = vecset(v,0,x);
        tstcheck(x.v == y.v);

        v = vecfree(v);
        x = vecfree(x);
    }

    tstcase("Check that not owned vectors can be owned") {
        v = vecnew();
        tstassert(!valisnil(v));

        // Set an owned vector
        x = vecnew();
        y = vecset(v,1,x);
        tstcheck(x.v == y.v);

        y = vecown(v,1);
        tstassert((x.v|1) == y.v);

        v = vecfree(v);
    }

    tstcase("Check that owned vectors can be disowned") {
        v = vecnew();
        tstassert(!valisnil(v));

        // Set an owned vector
        x = vecnew();
        tstassert(!valisnil(x));

        y = vecown(v,0,x);
        tstcheck((x.v | 1) == y.v );

        y = vecdisown(v,0);
        tstcheck(x.v == y.v);

        v = vecfree(v);
        x = vecfree(x);
    }

    tstcase("Check that an owned vec can't be freed with vecfree()") {
        v = vecnew();
        tstassert(!valisnil(v));

        // Set an owned vector
        x = vecnew(); y = x;
        tstassert(!valisnil(x));
        x = vecown(v,0,x);
        tstcheck(x.v == (y.v | 1));
        x = vecfree(x);
        tstcheck(!valisnil(x));

        v = vecfree(v);
    }

    tstcase("Check that overwriting will result in a free") {
        v = vecnew();
        tstassert(!valisnil(v));
        x = vecnew(); y = x;
        tstassert(!valisnil(x));
        x = vecown(v,0,x);

        vecset(v,0,42); // OVERWRITE!

        v = vecfree(v);
    }

    tstcase("Check that delete will result in a free") {
        v = vecnew();
        tstassert(!valisnil(v));
        x = vecnew();
        tstassert(!valisnil(x));
        x = vecown(v,0,x);
        x = vecnew();
        tstassert(!valisnil(x));
        x = vecown(v,1,x);

        vecdel(v,1); // 

        vecdel(v,0); // +
        v = vecfree(v);
    }

    tstcase("Check that delete will result in a free") {
        v = vecnew();
        tstassert(!valisnil(v));
        x = vecnew();
        tstassert(!valisnil(x));
        x = vecown(v,0,x);
        x = vecnew();
        tstassert(!valisnil(x));
        x = vecown(v,1,x);

        vecdel(v,0,1); // 

        v = vecfree(v);
    }

    tstcase("Check that owning twice is armless") {
        v = vecnew();
        tstassert(!valisnil(v));
        x = vecnew();
        tstassert(!valisnil(x));
        x = vecown(v,0,x);
        x = vecown(v,0,x); // This is harmless
        v = vecfree(v);
    }

        //if (valisvec(v)) v=vecfree(v);
    tstcheck(valisnil(v));
}