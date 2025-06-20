//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef int (*mono_f)(int);
#define valptr_0_t mono_f
#define ismono_f(v) valisptr(v,VALPTR_0)

typedef int (*couple_f)(int,int);
#define valptr_1_t couple_f
#define iscouple_f(v) valisptr(v,VALPTR_1)

int f1(int a, int b) {return a;}
int f2(int a, int b) {return b;}

#include "val.h"

// Or after inclusion to define your own struct (the most common case)
typedef struct  valptr_2_s { int x; int y; } *point_t;
#define PTRTAG_POINT VALPTR_2


tstsuite("Val Library pointers", advanced) {
    tstcase("48-bit Pointer Storage") {
        // We can't directly test 48-bit pointer storage, but we can verify basics
        void *ptr = malloc(10);
        val_t ptr_val = val(ptr);
        
        tstcheck(valisptr(ptr_val), "Pointer should be identified as a pointer");
        tstcheck(valtoptr(ptr_val) == ptr, "Pointer should be extracted correctly");
        tstcheck(valisptr(ptr_val, VALPTR_VOID), "Pointer should be identified as a void pointer");
        tstcheck(!valisptr(ptr_val, VALPTR_CHAR), "Pointer should not be identified as a char pointer");
        
        free(ptr);
    }

    tstcase("A non pointer is a NULL pointer") {
        val_t int_val = val(3);
        tstcheck(valtoptr(int_val) == NULL);
        tstcheck(!valisptr(int_val));
    }
    
    tstcase("Custom Pointer Types (via define)") {

        FILE *file = tmpfile();
        val_t file_val = val(file);
        
        tstcheck(valisptr(file_val), "FILE* should be identified as a pointer");
        tstcheck(valisptr(file_val, VALPTR_FILE), "FILE* should be identified as a pointer 1");
        tstcheck(valtoptr(file_val) == file, "FILE* should be extracted correctly");
        tstcheck(valisptr(stdout,VALPTR_FILE));
        fclose(file);
    }
    
    tstcase("Custom Pointer Types (via struct typedef)") {

        point_t point = malloc(sizeof(*point));
        val_t point_val = val(point);
        
        tstcheck(valisptr(point_val), "point_t should be identified as a pointer");
        tstcheck(valisptr(point_val, PTRTAG_POINT), "point_t should be identified as a pointer 1");
        tstcheck(valtoptr(point_val) == point, "point_t should be extracted correctly");
        free(point);
    }

    tstcase("Popinter tagging") {
        point_t point = malloc(sizeof(*point));
        val_t point_val = val(point);
        
        tstcheck((valtagptr(point_val) == 0), "Expect 0 got %d",valtagptr(point_val));
        point_val = valtagptr(point_val,2);
        tstcheck((valtagptr(point_val) == 2), "Expect 2 got %d",valtagptr(point_val));

        point_val = valtagptr(point_val,6);
        tstcheck((valtagptr(point_val) == 6), "Expect 6 got %d",valtagptr(point_val));

        point_val = valtagptr(point_val,0);
        tstcheck((valtagptr(point_val) == 0), "Expect 0 got %d",valtagptr(point_val));

        // Untaggable pointer (char *)
        val_t charptr_val = val("Hello");
        tstcheck((valtagptr(charptr_val) == 0), "Expect 0 got %d",valtagptr(charptr_val));
        charptr_val = valtagptr(charptr_val,2);
        tstcheck((valtagptr(charptr_val) == 0), "Expect 0 got %d",valtagptr(charptr_val));

        // Untaggable pointer (char *)
        val_t void_val = val((void *)&void_val);
        tstcheck((valtagptr(void_val) == 0), "Expect 0 got %d",valtagptr(void_val));
        void_val = valtagptr(void_val,2);
        tstcheck((valtagptr(void_val) == 0), "Expect 0 got %d",valtagptr(void_val));

        free(point);
    }

    tstcase("function pointers") {
        val_t v_arr[2];

        v_arr[0] = val(f1);
        v_arr[1] = val(f2);

        tstcheck(iscouple_f(v_arr[1]));
        tstcheck(valeq(f1,v_arr[0]));
    }
}
