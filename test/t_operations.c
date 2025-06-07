#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"

// Define custom constants
#define notfound valconst(1)
#define end_of_list valconst(2)

tstsuite("Val Library Operations", operations) {
    tstcase("Equality Comparison") {
        tstsection("Same Type Comparisons") {
            // Numeric equality
            tstcheck(valeq(val(42), val(42)), "Same integers should be equal");
            tstcheck(valeq(val(3.14), val(3.14)), "Same doubles should be equal");
            tstcheck(!valeq(val(42), val(43)), "Different integers should not be equal");
            tstcheck(!valeq(val(3.14), val(3.15)), "Different doubles should not be equal");
            
            // Boolean equality
            tstcheck(valeq(valtrue, valtrue), "valtrue should equal valtrue");
            tstcheck(valeq(valfalse, valfalse), "valfalse should equal valfalse");
            tstcheck(!valeq(valtrue, valfalse), "valtrue should not equal valfalse");
            
            // Pointer equality
            char str[] = "hello";
            tstcheck(valeq(val(str), val(str)), "Same pointers should be equal");
            tstcheck(!valeq(val(str), val("hello")), "Different pointers with same content should not be equal");
            tstcheck(valcmp(str,val("hello")) == 0,"Expected 0 got %d",valcmp(str,val("hello")));
            tstcheck(valcmp(str,"hello") == 0,"Expected 0 got %d",valcmp(str,val("hello")));
            
            // Special values
            tstcheck(valeq(valnil, valnil), "valnil should equal valnil");
            tstcheck(valeq(valnullptr, valnullptr), "valnullptr should equal valnullptr");
            tstcheck(!valeq(valnil, valnullptr), "valnil should not equal valnullptr");
            
            // Custom constants
            tstcheck(valeq(notfound, notfound), "notfound should equal notfound");
            tstcheck(valeq(end_of_list, end_of_list), "end_of_list should equal end_of_list");
            tstcheck(!valeq(notfound, end_of_list), "Different constants should not be equal");
        }
        
        tstsection("Auto-conversion in Equality") {
            // The API says functions auto-convert arguments
            tstcheck(valeq(val(42), 42), "val(42) should equal 42");
            tstcheck(valeq(val(3.14), 3.14), "val(3.14) should equal 3.14");
            tstcheck(valeq(valtrue, (_Bool)true), "valtrue should equal true");
            tstcheck(valeq(valfalse, (_Bool)false), "valfalse should equal false");
        }
    }
    
    tstcase("Comparison (valcmp)") {
        tstsection("Numeric Comparisons") {
            // Integer comparisons
            tstcheck(valcmp(val(10), val(20)) < 0, "10 should be less than 20");
            tstcheck(valcmp(val(30), val(20)) > 0, "30 should be greater than 20");
            tstcheck(valcmp(val(20), val(20)) == 0, "20 should be equal to 20");
            
            // Double comparisons
            tstcheck(valcmp(val(1.5), val(2.5)) < 0, "1.5 should be less than 2.5");
            tstcheck(valcmp(val(3.5), val(2.5)) > 0, "3.5 should be greater than 2.5");
            tstcheck(valcmp(val(2.5), val(2.5)) == 0, "2.5 should be equal to 2.5");
            
            // Mixed type comparisons
            tstcheck(valcmp(val(10), val(10.0)) == 0, "10 should be equal to 10.0");
            tstcheck(valcmp(val(10.5), val(10)) > 0, "10.5 should be greater than 10");
            tstcheck(valcmp(val(9.5), val(10)) < 0, "9.5 should be less than 10");
        }
        
        tstsection("Boolean Comparisons") {
            tstcheck(valcmp(valtrue, valfalse) > 0, "true should be greater than false");
            tstcheck(valcmp(valfalse, valtrue) < 0, "false should be less than true");
            tstcheck(valcmp(valtrue, valtrue) == 0, "true should be equal to true");
            tstcheck(valcmp(valfalse, valfalse) == 0, "false should be equal to false");
        }
        
        tstsection("Mixed Type Comparisons") {
            // Implementation-dependent but should be consistent
            tstnote("Testing mixed type comparisons (implementation-dependent)");
            valcmp(val(10), valtrue);
            valcmp(val(3.14), valfalse);
            valcmp(val("hello"), val(42));
        }
    }
    
    tstcase("Hashing") {
        tstsection("Hash Consistency") {
            // Same values should produce same hash
            uint64_t hash1 = valhash(val(42));
            uint64_t hash2 = valhash(val(42));
            tstcheck(hash1 == hash2, "Same value should produce same hash");
            
            // Different values should produce different hashes
            uint64_t hash3 = valhash(val(43));
            tstcheck(hash1 != hash3, "Different values should produce different hashes");
            
            // Consistent across types
            hash1 = valhash(valtrue);
            hash2 = valhash(valtrue);
            tstcheck(hash1 == hash2, "Same boolean should produce same hash");
            
            hash1 = valhash(val(3.14));
            hash2 = valhash(val(3.14));
            tstcheck(hash1 == hash2, "Same double should produce same hash");
            
            // Same pointer should produce same hash
            char *str = "hello";
            hash1 = valhash(val(str));
            hash2 = valhash(val(str));
            tstcheck(hash1 == hash2, "Same pointer should produce same hash");
            
            // Constants should have consistent hashes
            hash1 = valhash(notfound);
            hash2 = valhash(notfound);
            tstcheck(hash1 == hash2, "Same constant should produce same hash");
        }
    }
}
