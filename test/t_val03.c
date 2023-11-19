#include "tst.h"
#include "val.h"

tstsuite("Type Checking Functions") {
  tstcase("Check Integer Type") {
    val_t x = val(10);
    tstcheck(valisinteger(x), "Failed to identify integer");
    tstcheck(valissigned(x), "Failed to identify signed integer");
    x = val((unsigned char)0x40);
    tstcheck(valisinteger(x), "Failed to identify integer");
    tstcheck(!valissigned(x), "Failed to identify signed integer");
  }

  tstcase("Check Double Type") {
    val_t x = val(5.5);
    tstcheck(valisdouble(x), "Failed to identify double");
  }

  tstcase("Check String Type") {
    val_t x = val("hello");
    tstcheck(valisstring(x), "Failed to identify string");
  }

  // Add similar cases for boolean, pointer, nil, vec
}
