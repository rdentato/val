
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

tstsuite("Val Library Type Checking", type_checking) {
    tstcase("Numeric Type Checking") {
        val_t doubleVal = val(3.14);
        val_t signedIntVal = val(-42);
        val_t unsignedIntVal = val(42u);
        
        tstsection("Double Type Checking") {
            tstcheck(valisdouble(doubleVal), "3.14 should be identified as double");
            tstcheck(!valisdouble(signedIntVal), "Integer should not be identified as double");
            tstcheck(!valisdouble(unsignedIntVal), "Unsigned integer should not be identified as double");
        }
        
        tstsection("Signed Integer Type Checking") {
            tstcheck(!valissignedint(doubleVal), "Double should not be identified as signed integer");
            tstcheck(valissignedint(signedIntVal), "-42 should be identified as signed integer");
            tstcheck(!valissignedint(unsignedIntVal), "Unsigned integer should not be identified as signed integer");
        }
        
        tstsection("Unsigned Integer Type Checking") {
            tstcheck(!valisunsignedint(doubleVal), "Double should not be identified as unsigned integer");
            tstcheck(!valisunsignedint(signedIntVal), "Signed integer should not be identified as unsigned integer");
            tstcheck(valisunsignedint(unsignedIntVal), "42u should be identified as unsigned integer");
        }
    }
    
    tstcase("Boolean Type Checking") {
        val_t boolVal = val((_Bool)true);
        val_t intVal = val(1);
        
        tstcheck(valisboolean(boolVal), "true should be identified as boolean");
        tstcheck(!valisboolean(intVal), "1 should not be identified as boolean");
        tstcheck(valisboolean(valtrue), "valtrue should be identified as boolean");
        tstcheck(valisboolean(valfalse), "valfalse should be identified as boolean");
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
    
    tstcase("Pointer Type Checking") {
        void *ptr = malloc(10);
        val_t ptrVal = val(ptr);
        char *str = "hello";
        val_t charPtrVal = val(str);
        
        tstsection("Generic Pointer Checking") {
            tstcheck(valispointer(ptrVal), "Generic pointer should be identified as a pointer");
            tstcheck(valispointer(charPtrVal), "Char pointer should be identified as a pointer");
            tstcheck(valispointer(valnullptr), "valnullptr should be identified as a pointer");
            tstcheck(!valispointer(valtrue), "valtrue should not be identified as a pointer");
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
