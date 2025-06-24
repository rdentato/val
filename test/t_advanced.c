//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>

#include "val.h"

tstsuite("Val Library Advanced Use Cases", advanced) {
    tstcase("48-bit Integer Limits") {
        // 48-bit signed integer limit is 2^47 - 1 = 140,737,488,355,327
        int64_t max_48bit =  ((int64_t)1 << 47) - 1;
        int64_t min_48bit = -((int64_t)1 << 47);
        
        int64_t max_52bit =  ((int64_t)1 << 52) - 1;
        int64_t min_52bit = -((int64_t)1 << 52);
        
        val_t max_val = val(max_48bit);
        val_t min_val = val(min_48bit);
        
        tstcheck(valisint(max_val), "48-bit max should be a signed integer");
        tstcheck(valisint(min_val), "48-bit min should be a signed integer");
        tstcheck(valtoint(max_val) == max_48bit, "48-bit max should extract correctly");
        tstcheck(valtoint(min_val) == min_48bit, "48-bit min should extract correctly");
        
        max_val = val(max_52bit);
        min_val = val(min_52bit);
        
        tstcheck(valisint(max_val), "52-bit max should be a signed integer");
        tstcheck(valisint(min_val), "52-bit min should be a signed integer");
        tstcheck(valtoint(max_val) == max_52bit, "52-bit max should extract correctly");
        tstcheck(valtoint(min_val) == min_52bit, "52-bit min should extract correctly");
    }
    
    tstcase("Doubles edge cases") {
        double d;
        val_t d_val;

        #ifndef _MSC_VER
        d = 0.0/0.0;
        tstcheck(isnan(d));

        d_val = val(d);
        tstnote("0.0/0.0 d: %f d_val: %016" PRIX64,d,d_val.v);
        tstcheck(valisnumber(d_val));

        d_val = val(-d);
        tstnote("0.0/0.0 -d: %f d_val: %016" PRIX64,-d,d_val.v);
        tstcheck(valisnumber(d_val));
        #endif

        d = log(0.0);
        tstcheck(isinf(d));
        d_val = val(d);
        tstnote("log(0.0) d: %f d_val: %016" PRIX64,d,d_val.v);
        tstcheck(valisnumber(d_val));

        d_val = val(-d);
        tstcheck(isinf(-d));
        tstcheck(isinf(valtodouble(d_val)));
        tstcheck(valtodouble(d_val) == -d);
        tstnote("log(0.0) d: %f d_val: %016" PRIX64,-d,d_val.v);
        tstcheck(valisnumber(d_val));

        double inf = -log(0.0);
        tstcheck(isinf(inf));

        d = inf - inf;
        tstcheck(isnan(d));
        d_val = val(d);
        tstnote("inf-inf d: %f d_val: %016" PRIX64,d,d_val.v);
        tstcheck(valisnumber(d_val));

        d_val = val(-d);
        tstnote("inf-inf -d: %f d_val: %016" PRIX64,-d,d_val.v);
        tstcheck(valisnumber(d_val));

    }

    tstcase("Memory Size Verification") {
        // Verify that val_t is 8 bytes
        tstcheck(sizeof(val_t) == 8, "val_t should be exactly 8 bytes");
    }
}
