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

tstsuite("Val Library pointers", advanced) {
    tstcase("48-bit Pointer Storage") {
        // We can't directly test 48-bit pointer storage, but we can verify basics
        void *ptr = malloc(10);
        val_t ptr_val = val(ptr);
        
        tstcheck(valispointer(ptr_val), "Pointer should be identified as a pointer");
        tstcheck(valtopointer(ptr_val) == ptr, "Pointer should be extracted correctly");
        tstcheck(valispointer(ptr_val, VAL_PTRTAG_VOID), "Pointer should be identified as a void pointer");
        tstcheck(!valispointer(ptr_val, VAL_PTRTAG_CHAR), "Pointer should not be identified as a char pointer");
        
        free(ptr);
    }

    tstcase("A non pointer is a NULL pointer") {
        val_t int_val = val(3);
        tstcheck(valtopointer(int_val) == NULL);
        tstcheck(!valispointer(int_val));
    }
    
    tstcase("Custom Pointer Types (via define)") {

        FILE *file = tmpfile();
        val_t file_val = val(file);
        
        tstcheck(valispointer(file_val), "FILE* should be identified as a pointer");
        tstcheck(valispointer(file_val, PTRTAG_FILE), "FILE* should be identified as a pointer 1");
        tstcheck(valtopointer(file_val) == file, "FILE* should be extracted correctly");
        tstcheck(valispointer(stdout,PTRTAG_FILE));
        fclose(file);
    }
    
    tstcase("Custom Pointer Types (via struct typedef)") {

        point_t point = malloc(sizeof(*point));
        val_t point_val = val(point);
        
        tstcheck(valispointer(point_val), "point_t should be identified as a pointer");
        tstcheck(valispointer(point_val, PTRTAG_POINT), "point_t should be identified as a pointer 1");
        tstcheck(valtopointer(point_val) == point, "point_t should be extracted correctly");
        free(point);
    }

    tstcase("Memory Size Verification") {
        // Verify that val_t is 8 bytes
        tstcheck(sizeof(val_t) == 8, "val_t should be exactly 8 bytes");
    }
}
