#include "tst.h"
#include "val.h"

tstsuite("Comparison Function `valeq`") {
  tstcase("Integer Equality") {
    val_t x = val(100);
    val_t y = val(100);
    tstcheck(valeq(x, y), "Integer equality failed");
  }

  tstcase("Double Equality") {
    val_t x = val(3.1415);
    val_t y = val(3.1415);
    tstcheck(valeq(x, y), "Double equality failed");
  }

  tstcase("String Equality") {
    char *test_str = "test";
    val_t x = val(test_str);
    val_t y = val(test_str);
    tstcheck(valeq(x, y), "String equality failed");
  }

  tstcase("Pointer Equality") {
    void *ptr = malloc(10);
    val_t x = val(ptr);
    val_t y = val(ptr);
    tstcheck(valeq(x, y), "Pointer equality failed");
    free(ptr);
  }

  tstcase("Mixed Type Inequality") {
    val_t int_val = val(42);
    val_t str_val = val("42");
    tstcheck(!valeq(int_val, str_val), "Different types should not be equal");
  }

}


