#include "tst.h"
#include "val.h"

tstsuite("Basic Type Conversions") {
  tstcase("Integer Conversion") {
    int x = 42;
    val_t vx = val(x);
    tstcheck(valtointeger(vx) == x, "Integer conversion failed");
  }

  tstcase("Double Conversion") {
    double x = 3.14;
    val_t vx = val(x);
    tstcheck(valtodouble(vx) == x, "Double conversion failed");
  }

  tstcase("String Conversion") {
    char *x = "Test String";
    val_t vx = val(x);
    tstcheck(strcmp(valtostring(vx), x) == 0, "String conversion failed");
  }

  tstcase("Pointer Conversion") {
    void *x = malloc(10);  // Example pointer
    val_t vx = val(x);
    tstcheck(valtopointer(vx) == x, "Pointer conversion failed");
    free(x);
  }

  tstcase("Boolean Conversion") {
    _Bool x = 1;  // true
    val_t vx = val(x);
    tstcheck(valtobool(vx) == x, "Boolean conversion failed");
  }
}
