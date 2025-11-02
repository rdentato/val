//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"

tstsuite("Nil Values and Constants", nil_values) {
    tstcase("Nil Constants Distinctness") {
        val_t nil = valnil;
        val_t none = valnone;
        val_t undefined = valundefined;
        val_t tombstone = valtombstone;
        val_t deleted = valdeleted;
        
        tstsection("All nil constants are distinct") {
            tstcheck(nil.v != none.v, "valnil and valnone should be different");
            tstcheck(nil.v != undefined.v, "valnil and valundefined should be different");
            tstcheck(nil.v != tombstone.v, "valnil and valtombstone should be different");
            tstcheck(nil.v != deleted.v, "valnil and valdeleted should be different");
            
            tstcheck(none.v != undefined.v, "valnone and valundefined should be different");
            tstcheck(none.v != tombstone.v, "valnone and valtombstone should be different");
            tstcheck(none.v != deleted.v, "valnone and valdeleted should be different");
            
            tstcheck(undefined.v != tombstone.v, "valundefined and valtombstone should be different");
            tstcheck(undefined.v != deleted.v, "valundefined and valdeleted should be different");
            
            tstcheck(tombstone.v != deleted.v, "valtombstone and valdeleted should be different");
        }
    }
    
    tstcase("Nil Constants Recognition") {
        tstsection("valisnil() recognizes valnil") {
            tstcheck(valisnil(valnil), "valisnil(valnil) should be true");
            tstcheck(!valisnil(valnone), "valisnil(valnone) should be false");
            tstcheck(!valisnil(valundefined), "valisnil(valundefined) should be false");
            tstcheck(!valisnil(valtombstone), "valisnil(valtombstone) should be false");
            tstcheck(!valisnil(valdeleted), "valisnil(valdeleted) should be false");
        }
    }
    
    tstcase("Nil Constants vs Other Values") {
        tstsection("Nil constants differ from numbers") {
            val_t zero = val(0);
            val_t one = val(1);
            val_t pi = val(3.14159);
            
            tstcheck(valnil.v != zero.v, "valnil should differ from val(0)");
            tstcheck(valnone.v != one.v, "valnone should differ from val(1)");
            tstcheck(valundefined.v != pi.v, "valundefined should differ from val(3.14159)");
            tstcheck(valtombstone.v != zero.v, "valtombstone should differ from val(0)");
            tstcheck(valdeleted.v != one.v, "valdeleted should differ from val(1)");
        }
        
        tstsection("Nil constants differ from booleans") {
            val_t true_val = val((_Bool)true);
            val_t false_val = val((_Bool)false);
            
            tstcheck(valnil.v != true_val.v, "valnil should differ from val(true)");
            tstcheck(valnone.v != false_val.v, "valnone should differ from val(false)");
            tstcheck(valundefined.v != true_val.v, "valundefined should differ from val(true)");
            tstcheck(valtombstone.v != false_val.v, "valtombstone should differ from val(false)");
            tstcheck(valdeleted.v != true_val.v, "valdeleted should differ from val(true)");
        }
        
        tstsection("Nil constants differ from pointers") {
            void *ptr = (void *)0x1234;
            char *str = "hello";
            val_t ptr_val = val(ptr);
            val_t str_val = val(str);
            
            tstcheck(valnil.v != ptr_val.v, "valnil should differ from val(ptr)");
            tstcheck(valnone.v != str_val.v, "valnone should differ from val(str)");
            tstcheck(valundefined.v != ptr_val.v, "valundefined should differ from val(ptr)");
            tstcheck(valtombstone.v != str_val.v, "valtombstone should differ from val(str)");
            tstcheck(valdeleted.v != ptr_val.v, "valdeleted should differ from val(ptr)");
        }
    }
    
    tstcase("Nil Constants Bit Pattern") {
        tstsection("Nil constants have correct bit patterns") {
            tstcheck(valnil.v == 0x7FF9B3F000000000ULL, "valnil should have correct bit pattern");
            tstcheck(valnone.v == 0x7FF9B3F100000000ULL, "valnone should have correct bit pattern");
            tstcheck(valundefined.v == 0x7FF9B3F200000000ULL, "valundefined should have correct bit pattern");
            tstcheck(valtombstone.v == 0x7FF9B3F300000000ULL, "valtombstone should have correct bit pattern");
            tstcheck(valdeleted.v == 0x7FF9B3F400000000ULL, "valdeleted should have correct bit pattern");
        }
    }
    
    tstcase("Nil Constants Immutability") {
        val_t nil1 = valnil;
        val_t nil2 = valnil;
        
        tstsection("Nil constants are immutable and consistent") {
            tstcheck(nil1.v == nil2.v, "Two copies of valnil should be identical");
            tstcheck(valisnil(nil1), "First copy of valnil should still be recognized as nil");
            tstcheck(valisnil(nil2), "Second copy of valnil should still be recognized as nil");
        }
    }
    
    tstcase("Nil Constants Comparison") {
        tstsection("Nil constants compare correctly") {
            tstcheck(valcmp(valnil, valnil) == 0, "valnil should equal itself");
            tstcheck(valcmp(valnone, valnone) == 0, "valnone should equal itself");
            tstcheck(valcmp(valundefined, valundefined) == 0, "valundefined should equal itself");
            tstcheck(valcmp(valtombstone, valtombstone) == 0, "valtombstone should equal itself");
            tstcheck(valcmp(valdeleted, valdeleted) == 0, "valdeleted should equal itself");
            
            // Comparisons between different nil constants
            tstcheck(valcmp(valnil, valnone) != 0, "valnil and valnone should not compare equal");
            tstcheck(valcmp(valnil, valundefined) != 0, "valnil and valundefined should not compare equal");
            tstcheck(valcmp(valnone, valdeleted) != 0, "valnone and valdeleted should not compare equal");
        }
    }
    
    tstcase("Nil Constants Hashing") {
        tstsection("Nil constants produce distinct hashes") {
            uint32_t hash_nil = valhash(valnil);
            uint32_t hash_none = valhash(valnone);
            uint32_t hash_undefined = valhash(valundefined);
            uint32_t hash_tombstone = valhash(valtombstone);
            uint32_t hash_deleted = valhash(valdeleted);
            
            tstcheck(hash_nil != hash_none, "valnil and valnone should have different hashes");
            tstcheck(hash_nil != hash_undefined, "valnil and valundefined should have different hashes");
            tstcheck(hash_nil != hash_tombstone, "valnil and valtombstone should have different hashes");
            tstcheck(hash_nil != hash_deleted, "valnil and valdeleted should have different hashes");
            
            tstcheck(hash_none != hash_undefined, "valnone and valundefined should have different hashes");
            tstcheck(hash_none != hash_tombstone, "valnone and valtombstone should have different hashes");
            tstcheck(hash_none != hash_deleted, "valnone and valdeleted should have different hashes");
            
            tstcheck(hash_undefined != hash_tombstone, "valundefined and valtombstone should have different hashes");
            tstcheck(hash_undefined != hash_deleted, "valundefined and valdeleted should have different hashes");
            
            tstcheck(hash_tombstone != hash_deleted, "valtombstone and valdeleted should have different hashes");
        }
        
        tstsection("Nil constants have consistent hashing") {
            uint32_t hash1 = valhash(valnil);
            uint32_t hash2 = valhash(valnil);
            
            tstcheck(hash1 == hash2, "valnil should always hash to the same value");
        }
    }
    
    tstcase("Nil Constants Type Checking") {
        tstsection("Nil constants are recognized as constants") {
            tstcheck(valisconst(valnil), "valnil should be recognized as a constant");
            tstcheck(valisconst(valnone), "valnone should be recognized as a constant");
            tstcheck(valisconst(valundefined), "valundefined should be recognized as a constant");
            tstcheck(valisconst(valtombstone), "valtombstone should be recognized as a constant");
            tstcheck(valisconst(valdeleted), "valdeleted should be recognized as a constant");
        }
        
        tstsection("Nil constants are not numbers") {
            tstcheck(!valisnumber(valnil), "valnil should not be a number");
            tstcheck(!valisnumber(valnone), "valnone should not be a number");
            tstcheck(!valisnumber(valundefined), "valundefined should not be a number");
            tstcheck(!valisnumber(valtombstone), "valtombstone should not be a number");
            tstcheck(!valisnumber(valdeleted), "valdeleted should not be a number");
        }
        
        tstsection("Nil constants are not pointers") {
            tstcheck(!valisptr(valnil), "valnil should not be a pointer");
            tstcheck(!valisptr(valnone), "valnone should not be a pointer");
            tstcheck(!valisptr(valundefined), "valundefined should not be a pointer");
            tstcheck(!valisptr(valtombstone), "valtombstone should not be a pointer");
            tstcheck(!valisptr(valdeleted), "valdeleted should not be a pointer");
        }
        
        tstsection("Nil constants are not booleans") {
            tstcheck(!valisbool(valnil), "valnil should not be a boolean");
            tstcheck(!valisbool(valnone), "valnone should not be a boolean");
            tstcheck(!valisbool(valundefined), "valundefined should not be a boolean");
            tstcheck(!valisbool(valtombstone), "valtombstone should not be a boolean");
            tstcheck(!valisbool(valdeleted), "valdeleted should not be a boolean");
        }
    }
}
