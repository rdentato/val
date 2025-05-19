#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"

// Define custom pointer types before including val.h
#define valpointer_0_t FILE *

// Define custom constants
#define notfound valconst(1)
#define end_of_list valconst(2)
#define operation_failed valconst(3)
#define custom_success valconst(4)
#define custom_error valconst(5)

tstsuite("Val Library Value Extraction", value_extraction) {
    tstcase("Numeric Value Extraction") {
        double d = 3.14159;
        int si = -42;
        unsigned int ui = 42u;
        
        val_t doubleVal = val(d);
        val_t signedIntVal = val(si);
        val_t unsignedIntVal = val(ui);
        
        tstsection("Double Extraction") {
            tstcheck(valtodouble(doubleVal) == d, "Double value should be extracted correctly");
            // Testing conversion from int to double
            tstcheck(valtosignedint(signedIntVal) == (double)si, "Signed int should convert to double");
            tstcheck(valtodouble(unsignedIntVal) == (double)ui, "Unsigned int should convert to double");
        }
        
        tstsection("Signed Integer Extraction") {
            tstcheck(valtosignedint(signedIntVal) == si, "Signed int should be extracted correctly");
            // Testing conversion from double to int
            tstcheck(valtosignedint(doubleVal) == (int)d, "Double should convert to signed int");
            tstcheck(valtounsignedint(unsignedIntVal) == (unsigned int)ui, "Unsigned int should convert to signed int");
        }
        
        tstsection("Unsigned Integer Extraction") {
            tstcheck(valtounsignedint(unsignedIntVal) == ui, "Unsigned int should be extracted correctly");
            // Testing conversion to unsigned int
            tstcheck(valtounsignedint(doubleVal) == (unsigned int)d, "Double should convert to unsigned int");
            tstcheck((unsigned int)valtounsignedint(signedIntVal) == (unsigned int)si, 
                             "Signed int should convert to unsigned int %u %u", (unsigned int)valtounsignedint(signedIntVal), (unsigned int)si);
        }
    }
    
    tstcase("Boolean Value Extraction") {
        val_t trueVal = val(true);
        val_t falseVal = val(false);
        
        tstcheck(valtobool(trueVal) == true, "True value should be extracted correctly");
        tstcheck(valtobool(falseVal) == false, "False value should be extracted correctly");
        tstcheck(valtobool(valtrue) == true, "valtrue should extract to true");
        tstcheck(valtobool(valfalse) == false, "valfalse should extract to false");
    }
    
    tstcase("Pointer Value Extraction") {
        void *ptr = malloc(10);
        char *str = "hello";
        
        val_t ptrVal = val(ptr);
        val_t charPtrVal = val(str);
        
        tstsection("Raw Pointer Extraction") {
            tstcheck(valtopointer(ptrVal) == ptr, "Generic pointer should be extracted correctly");
            tstcheck(valtopointer(charPtrVal) == str, "Char pointer should be extracted correctly");
        }
        
        tstsection("Pointer Tag Extraction") {
            // Assuming pointer types are tagged based on their defined order
            tstcheck(valpointertag(charPtrVal) == 7, "Char pointer should have tag 7, (%d)",valpointertag(charPtrVal));
            tstcheck(valpointertag(ptrVal) == 8, "Generic pointer should have tag 8, (%d)",valpointertag(ptrVal));
        }
        
        free(ptr);
    }
    
    tstcase("Edge Cases in Value Extraction") {
        // Extract non-native types
        val_t nilVal = valnil;
        val_t constVal = notfound;
        
        tstsection("Nil Value Extraction") {
            // Assuming the implementation handles these conversions
            tstnote("Testing nil value conversions");
            // These are implementation-dependent but should not crash
            valtodouble(nilVal);
            valtosignedint(nilVal);
            valtounsignedint(nilVal);
            valtobool(nilVal);
        }
        
        tstsection("Constant Value Extraction") {
            // Assuming the implementation handles these conversions
            tstnote("Testing constant value conversions");
            // These are implementation-dependent but should not crash
            valtodouble(constVal);
            valtosignedint(constVal);
            valtounsignedint(constVal);
            valtobool(constVal);
        }
    }
}
