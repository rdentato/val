//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "val.h"

// Define custom constants
#define notfound         valconst(1)
#define end_of_list      valconst(2)
#define operation_failed valconst(3)
#define custom_success   valconst(4)
#define custom_error     valconst(5)
#define same_custom_error valconst(5)

tstsuite("Val Library Core Functionality", core) {
    tstcase("Value Creation") {
            val_t v1 = val(42);
            tstcheck(valisint(v1), "42 should be a signed integer");
            tstcheck(valtoint(v1) == 42, "Value should be 42");
            
            val_t v2 = val(3.14);
            tstcheck(valisnumber(v2), "3.14 should be a double");
            tstcheck(valtodouble(v2) == 3.14, "Value should be 3.14");
            
            val_t v3 = val((_Bool)true);
            tstcheck(valisbool(v3), "true should be a boolean (%016" PRIX64 ")",v3.v);
            tstcheck(valtobool(v3) == true, "Value should be true (%016" PRIX64 ")",v3.v);
            
                  v3 = val((_Bool)143);
            tstcheck(valisbool(v3), "true should be a boolean (%016" PRIX64 ")",v3.v);
            tstcheck(valtobool(v3) == true, "Value should be true");
            
            val_t v4 = val((_Bool)false);
            tstcheck(valisbool(v4), "false should be a boolean");
            tstcheck(valtobool(v4) == false, "Value should be false (%016" PRIX64 " ->%d)",v4.v,valtobool(v4));
            
                  v4 = val((_Bool)0);
            tstcheck(valisbool(v4), "false should be a boolean");
            tstcheck(valtobool(v4) == false, "Value should be false (%016" PRIX64 ")",v4.v);
            
            val_t v5 = val(NULL);
            tstcheck(valisptr(v5), "NULL should be a pointer (%016" PRIX64 ")",v5.v);
            tstcheck(valisnullptr(v5), "NULL should be a null pointer (%016" PRIX64 ")",v5.v);
            // unsigned 
            val_t v6 = val(42u);
            tstcheck(valisint(v6), "42u should be an unsigned integer");
            tstcheck(valtounsignedint(v6) == 42u, "Value should be 42u");
            
            // Character pointer
            char *test_str = "hello";
            val_t v7 = val(test_str);
            tstcheck(valischarptr(v7), "String pointer should be a char pointer");
            tstcheck(valtoptr(v7) == test_str, "Pointer values should match");
    }
    
    tstcase("Predefined Constants") {
            tstcheck(valisbool(valtrue), "valtrue should be a boolean");
            tstcheck(valtobool(valtrue) == true, "valtrue should equal true");
            
            tstcheck(valisbool(valfalse), "valfalse should be a boolean");
            tstcheck(valtobool(valfalse) == false, "valfalse should equal false");

            tstcheck(valisnil(valnil), "valnil should be nil");
            tstcheck(valisnullptr(valnullptr), "valnullptr should be a null pointer");

            tstcheck(valisconst(notfound), "notfound should be a custom constant");
            tstcheck(valisconst(end_of_list), "end_of_list should be a custom constant");
            tstcheck(valisconst(operation_failed), "operation_failed should be a custom constant");
            
            tstcheck(valeq(custom_error, custom_error));
            // Ensure constants with different values are not equal
            tstcheck(!valeq(notfound, end_of_list), "Different constants should not be equal");
            tstcheck(!valeq(notfound, operation_failed), "Different constants should not be equal");
            tstcheck(!valeq(end_of_list, operation_failed), "Different constants should not be equal");

    }
}
