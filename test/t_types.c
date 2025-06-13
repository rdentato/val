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
#define end_of_list valconst(2)

tstsuite("Val Library Type Checking", type_checking) {
    tstcase("Numeric Type Checking") {
        val_t doubleVal = val(3.14);
        val_t signedIntVal = val(-42);
        val_t unsignedIntVal = val(42u);
        
        tstsection("Double Type Checking") {
            tstcheck(valisdouble(doubleVal), "3.14 should be identified as double");
            tstcheck(valisdouble(signedIntVal), "Integer should not be identified as double");
            tstcheck(valisdouble(unsignedIntVal), "Unsigned integer should not be identified as double");
        }
        
        tstsection("Signed Integer Type Checking") {
            tstcheck(!valisint(doubleVal), "Double should not be identified as signed integer");
            tstcheck(valisint(signedIntVal), "-42 should be identified as signed integer");
            tstcheck(valisint(unsignedIntVal), "Unsigned integer should not be identified as signed integer");
        }
        
        tstsection("Unsigned Integer Type Checking") {
            tstcheck(!valisint(doubleVal), "Double should not be identified as unsigned integer");
            tstcheck(valisint(signedIntVal), "Signed integer should not be identified as unsigned integer");
            tstcheck(valisint(unsignedIntVal), "42u should be identified as unsigned integer");
        }
    }
    
    tstcase("Boolean Type Checking") {
        val_t boolVal = val((_Bool)true);
        val_t intVal = val(1);
        
        tstcheck(valisbool(boolVal), "true should be identified as boolean");
        tstcheck(!valisbool(intVal), "1 should not be identified as boolean");
        tstcheck(valisbool(valtrue), "valtrue should be identified as boolean");
        tstcheck(valisbool(valfalse), "valfalse should be identified as boolean");
    }
    
    tstcase("Nil Type Checking") {
        tstcheck(valisnil(valnil), "valnil should be identified as nil");
        tstcheck(!valisnil(valtrue), "valtrue should not be identified as nil");
        tstcheck(!valisnil(val(0)), "0 should not be identified as nil");
        tstcheck(!valisnil(valnullptr), "valnullptr should not be identified as nil");
    }
    
    tstcase("Constant Type Checking") {
        tstcheck(valisconst(notfound), "notfound should be identified as a constant");
        tstcheck(valisconst(end_of_list), "end_of_list should be identified as a constant");
        tstcheck(!valisconst(valtrue), "valtrue should not be identified as a custom constant");
        tstcheck(!valisconst(valnil), "valnil should not be identified as a custom constant");
    }
    
    #define ITEM_LOST 123
    #define ITEM_FOUND 435
    tstcase("Chec exact constants") {
      val_t item_lost = valconst(ITEM_LOST);
      val_t item_found = valconst(ITEM_FOUND);

      tstcheck( valisconst(item_lost));
      tstcheck( valisconst(item_lost,ITEM_LOST));

      tstcheck( !valeq(item_lost,item_found));
      tstcheck( !valisconst(item_lost,ITEM_FOUND));

      tstcheck( valeq(item_lost,valconst(ITEM_LOST)));
      tstcheck( !valeq(item_lost,ITEM_LOST));
    }

    tstcase("Pointer Type Checking") {
        void *ptr = malloc(10);
        char *str = "hello";

        val_t ptrVal = val(ptr);
        val_t charPtrVal = val(str);

        tstsection("Generic Pointer Checking") {
            tstcheck(valisptr(ptrVal), "Generic pointer should be identified as a pointer");
            tstcheck(valisptr(charPtrVal), "Char pointer should be identified as a pointer");
            tstcheck(valisptr(valnullptr), "valnullptr should be identified as a pointer");
            tstcheck(!valisptr(valtrue), "valtrue should not be identified as a pointer");
        }
        
        tstsection("Char Pointer Checking") {
            tstcheck(valischarptr(charPtrVal), "String should be identified as char pointer");
            tstcheck(!valischarptr(ptrVal), "Generic pointer should not be identified as char pointer");
        }
       
        tstsection("Null Pointer Checking") {
            tstcheck(valisnullptr(valnullptr), "valnullptr should be identified as null pointer");
            tstcheck(valisnullptr(val(NULL)), "NULL should be identified as null pointer");
            tstcheck(!valisnullptr(ptrVal), "Non-null pointer should not be identified as null pointer");
        }
        
        free(ptr);
    }
}
