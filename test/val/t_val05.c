#include <math.h>
#include <limits.h>
#include "tst.h"
#include "val.h"

tstsuite("Constants") {
  val_t x;
  tstcase("predefined constants") {
    tstcheck(!valisconst(valnil),"x: 0x%08lX (0x%08lX)",valnil.v,(valnil.v & (uint64_t)0xFFFF000000000000) );
    tstcheck(!valisconst(valtrue),"x: 0x%08lX (0x%08lX)",valnil.v,(valnil.v & (uint64_t)0xFFFF000000000000) );
  }

  tstcase("Constants (default group)") {
    tstcheck(valconst(431).v == (VAL_CST_MASK | 431));
    tstcheck(valconst(&x).v != (VAL_PTR_MASK | (uintptr_t)(&x)));
    tstcheck((valconst(&x).v & VAL_PAYLOAD) == (uintptr_t)(&x));
    tstcheck(valtopointer(valconst(&x)) == &x);
    int y=32;
    tstcheck(valconst(y*2).v == (VAL_CST_MASK | (y*2)));
  }

  tstcase("Constants (group 1)") {
    tstcheck(valconst(431,1).v == ((VAL_CST_MASK + 0x1000000000000) | 431) ,"x: 0x%08lX",valconst(431,1).v);
    tstcheck((valconst(&x,1).v & VAL_PAYLOAD) == (uintptr_t)(&x));
    tstcheck(valtopointer(valconst(&x,1)) == &x);
    int y=32;
    tstcheck(valconst(y*2,1).v == ((VAL_CST_MASK + 0x1000000000000) | (y*2)));
    x = valconst(y*2,1);
    tstcheck(valisconst(x,1));
    tstcheck(!valisconst(x));
  }
    
}
