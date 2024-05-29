#include "tst.h"
#include "val.h"

tstsuite("Hashing") {
    uint32_t a,b;

    tstcase("Integers") {
      a = valhash(val(3)); b = valhash(val(3));
      tstcheck(a != 0 && a == b,"a: %08X  b: %08X",a,b);
    }

  }


