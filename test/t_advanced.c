#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Define custom pointer types before including val.h
#define valpointer_1_t FILE *
#define PTRTAG_FILE VAL_PTRTAG_1

#include "val.h"

// Or after as long as it is a pointer to a structure (the most common case)
struct  valpointer_2_s { int x; int y; };
#define point_t valpointer_2_t 
#define PTRTAG_POINT VAL_PTRTAG_2

tstsuite("Val Library Advanced Use Cases", advanced) {
    tstcase("48-bit Integer Limits") {
        // 48-bit signed integer limit is 2^47 - 1 = 140,737,488,355,327
        int64_t max_48bit = ((int64_t)1 << 47) - 1;
        int64_t min_48bit = -((int64_t)1 << 47);
        
        val_t max_val = val(max_48bit);
        val_t min_val = val(min_48bit);
        
        tstcheck(valissignedint(max_val), "48-bit max should be a signed integer");
        tstcheck(valissignedint(min_val), "48-bit min should be a signed integer");
        tstcheck(valtosignedint(max_val) == max_48bit, "48-bit max should extract correctly");
        tstcheck(valtosignedint(min_val) == min_48bit, "48-bit min should extract correctly");
    }
    
    tstcase("Memory Size Verification") {
        // Verify that val_t is 8 bytes
        tstcheck(sizeof(val_t) == 8, "val_t should be exactly 8 bytes");
    }
}
