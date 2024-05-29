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
        tstcheck(vecallocatedmem == 0);
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
        tstcheck(vecallocatedmem == sizeof(struct vec_s));
        x = vecfree(x);
        tstcheck(vecallocatedmem == 0);
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
        tstcheck(vecallocatedmem == 0);
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
        tstcheck(vecallocatedmem == sizeof(struct vec_s), "Expectd %lu, got %lu",sizeof(struct vec_s),vecallocatedmem);
        x = vecfree(x);
        tstcheck(vecallocatedmem == 0);
    }

    tstcase("Check that an owned vec can't be freed with vecfree()") {
        v = vecnew();
        tstassert(!valisnil(v));

        // Set an owned vector
        x = vecnew(); y = x;
        tstassert(!valisnil(x));
        x = vecown(v,0,x);
        tstcheck(x.v == (y.v | 1));
        int64_t old_mem = vecallocatedmem;
        x = vecfree(x);
        tstcheck(!valisnil(x));
        tstcheck(old_mem == vecallocatedmem);

        v = vecfree(v);
        tstcheck(vecallocatedmem == 0);
    }

    tstcase("Check that overwriting will result in a free") {
        v = vecnew();
        tstassert(!valisnil(v));
        x = vecnew(); y = x;
        tstassert(!valisnil(x));
        x = vecown(v,0,x);
        int64_t old_mem = vecallocatedmem;

        vecset(v,0,42); // OVERWRITE!
        tstcheck(old_mem == vecallocatedmem + sizeof(struct vec_s));

        v = vecfree(v);
        tstcheck(vecallocatedmem == 0);
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
        int64_t old_mem = vecallocatedmem;

        vecdel(v,1); // 
        tstcheck(old_mem == vecallocatedmem + sizeof(struct vec_s));
        old_mem = vecallocatedmem;

        vecdel(v,0); // +
        tstcheck(old_mem == vecallocatedmem + sizeof(struct vec_s));
        v = vecfree(v);
        tstcheck(vecallocatedmem == 0);
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
        int64_t old_mem = vecallocatedmem;

        vecdel(v,0,1); // 
        tstcheck(old_mem == vecallocatedmem + 2*sizeof(struct vec_s));

        v = vecfree(v);
        tstcheck(vecallocatedmem == 0);
    }

    tstcase("Check that owning twice is armless") {
        v = vecnew();
        tstassert(!valisnil(v));
        x = vecnew();
        tstassert(!valisnil(x));
        x = vecown(v,0,x);
        int64_t old_mem = vecallocatedmem;
        x = vecown(v,0,x); // This is harmless
        tstcheck(old_mem == vecallocatedmem,"Expect %lu, got %lu",old_mem,vecallocatedmem);
        v = vecfree(v);
        tstcheck(vecallocatedmem == 0);       
    }

        //if (valisvec(v)) v=vecfree(v);
    tstcheck(valisnil(v));
}