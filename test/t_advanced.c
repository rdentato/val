#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Define custom pointer types before including val.h
#define valpointer_1_t FILE *

#include "val.h"

// Define custom constants
#define notfound valconst(1)
#define end_of_list valconst(2)
#define operation_failed valconst(3)
#define custom_success valconst(4)
#define custom_error valconst(5)

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
    
    tstcase("48-bit Pointer Storage") {
        // We can't directly test 48-bit pointer storage, but we can verify basics
        void *ptr = malloc(10);
        val_t ptr_val = val(ptr);
        
        tstcheck(valispointer(ptr_val), "Pointer should be identified as a pointer");
        tstcheck(valtopointer(ptr_val) == ptr, "Pointer should be extracted correctly");
        
        free(ptr);
    }
    
    tstcase("Custom Pointer Types") {

        FILE *file = tmpfile();
        val_t file_val = val(file);
        
        tstcheck(valispointer(file_val), "FILE* should be identified as a pointer");
        tstcheck(valpointertag(file_val) == 1, "FILE* should have tag 1 (%d) (%016lx)",valpointertag(file_val), file_val.v);
        tstcheck(valtopointer(file_val) == file, "FILE* should be extracted correctly");
        
        fclose(file);
    }
    
    tstcase("Memory Size Verification") {
        // Verify that val_t is 8 bytes
        tstcheck(sizeof(val_t) == 8, "val_t should be exactly 8 bytes");
    }
}
