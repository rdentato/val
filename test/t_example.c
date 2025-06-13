//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"

// Define custom constants
#define notfound valconst(1)


val_t lookup(const char *key) {
    if (strcmp(key, "int") == 0) {
        return val(42);
    } else if (strcmp(key, "double") == 0) {
        return val(3.14);
    } else if (strcmp(key, "bool") == 0) {
        return valtrue;
    } else if (strcmp(key, "null") == 0) {
        return valnullptr;
    } else if (strcmp(key, "nil") == 0) {
        return valnil;
    } else {
        return notfound;
    }
}
        
tstsuite("Val Library Real-world Usage", usage) {
    tstcase("Dictionary Key Example") {
        // Simulating use of val_t as dictionary keys
        val_t keys[5] = {
            val(42),
            val("hello"),
            val(3.14),
            valtrue,
            notfound
        };
        
        // Verify we can hash all types
        uint64_t hashes[5];
        for (int i = 0; i < 5; i++) {
            hashes[i] = valhash(keys[i]);
            tstcheck(hashes[i] != 0, "Hash should not be zero");
        }
        
        // Verify key equality
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                if (i == j) {
                    tstcheck(valeq(keys[i], keys[j]), "Same keys should be equal");
                } else {
                    tstcheck(!valeq(keys[i], keys[j]), "Different keys should not be equal");
                }
            }
        }
    }
    
    tstcase("Function Return Value Example") {
        // Simulating a function that returns different types
        val_t result;
        
        result = lookup("int");
        tstcheck(valisint(result), "Should return an integer");
        tstcheck(valtoint(result) == 42, "Should return 42");
        
        result = lookup("double");
        tstcheck(valisdouble(result), "Should return a double");
        tstcheck(valtodouble(result) == 3.14, "Should return 3.14");
        
        result = lookup("bool");
        tstcheck(valisbool(result), "Should return a boolean");
        tstcheck(valtobool(result) == true, "Should return true");
        
        result = lookup("null");
        tstcheck(valisnullptr(result), "Should return a null pointer");
        
        result = lookup("nil");
        tstcheck(valisnil(result), "Should return nil");
        
        result = lookup("unknown");
        tstcheck(valisconst(result), "Should return a constant");
        tstcheck(valeq(result, notfound), "Should return notfound constant");
    }
    
    tstcase("Type Conversion Flow") {
        // Test the auto-conversion flow
        val_t start = val(42);
        
        // Extract as different types
        double d = valtodouble(start);
        int i = (int)valtoint(start);
        unsigned int u = (unsigned int)valtounsignedint(start);
        bool b = valtobool(start);
        
        // Convert back to val_t
        val_t back_d = val(d);
        val_t back_i = val(i);
        val_t back_u = val(u);
        val_t back_b = val(b);
        
        // Verify types
        tstcheck(valisdouble(back_d), "Should be a double");
        tstcheck(valisint(back_i), "Should be a signed integer");
        tstcheck(valisint(back_u), "Should be an unsigned integer");
        tstcheck(valisbool(back_b), "Should be a boolean");
        
        // Verify values
        tstcheck(valtodouble(back_d) == 42.0, "Value should be 42.0");
        tstcheck(valtoint(back_i) == 42, "Value should be 42");
        tstcheck(valtounsignedint(back_u) == 42u, "Value should be 42u");
        tstcheck(valtobool(back_b) == true, "Value should be true (%d)",valtobool(back_b));
    }
}
