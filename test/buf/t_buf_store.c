#include "tst.h"
#include "val.h"


tstsuite("Buffer store") {

  val_t a;
  val_t b;
  val_t c;
  //uint32_t n;

  tstcheck("Not a string is an empty string")
  {
    tstcheck(valtostring(val(3))[0] == '\0');
  }

  a = bufstore("pippo");
  b = bufstore("pluto");

  tstcase("Default store") {

    tstcheck(valisstored(a));
    tstcheck(!valisbuf(a));
    tstcheck((((uintptr_t)valtostring(a)) & 3) == 0);
    tstcheck(valtointeger(a) == 4);
    tstcheck(strcmp(valtostring(a),"pippo") == 0)

    tstcheck(valisstored(b));
    tstcheck(!valisbuf(b));
    tstcheck((((uintptr_t)valtostring(b)) & 3) == 0,"ptr: %016lX",(((uintptr_t)valtostring(b))));
    tstcheck(valtointeger(b) == 16,"int: %ld",valtointeger(b));
    tstcheck(strcmp(valtostring(a),"pippo") == 0)
    tstcheck(strcmp(valtostring(b),"pluto") == 0)

    c = bufstore("topo");
    tstcheck(valisstored(c));
    tstcheck(!valisbuf(c));
    tstcheck((((uintptr_t)valtostring(c)) & 3) == 0,"ptr: %016lX",(((uintptr_t)valtostring(c))));
    tstcheck(valtointeger(c) == 28,"int: %ld",valtointeger(c));
    tstcheck(strcmp(valtostring(a),"pippo") == 0)
    tstcheck(strcmp(valtostring(b),"pluto") == 0)
    tstcheck(strcmp(valtostring(c),"topo") == 0)

    tstcheck(bufpos(buf_stores[0]) == 36);

    //            1         2         3     
    //  0123456789012345678901234567890123456
    //  ....pippo000....pluto000....topo0000
  }

  tstcase("Clear store") {
    bufclearstore(0);

    tstcheck(!valisstored(a));
    tstcheck(valtointeger(a) == 4);
    tstcheck(valtostring(a)[0] == '\0');
    tstcheck(valtostring(b)[0] == '\0');

    a = bufstore("qui");

    tstcheck(valisstored(a));
    tstcheck(!valisbuf(a));
    tstcheck((((uintptr_t)valtostring(a)) & 3) == 0);
    tstcheck(valtointeger(a) == ((((uint64_t)1)<<40) | 4));
    tstcheck(strcmp(valtostring(a),"qui") == 0)
  }

  tstcase("aux values") {
    a = bufstore("pippo");

    tstcheck(valisstored(a));
    tstcheck(valaux(a) == 0);

    tstcheck(valaux(a,32) == 32);
    tstcheck(valaux(a) == 32);

    bufclearstore();

    tstcheck(!valisstored(a));
    tstcheck(valaux(a) == 0);
  }


  tstcase("other stores") {
    a = bufstore("pippo");
    b = bufstore(1,"pippo");
    tstcheck(valcmp(a,b) == 0,"[%s] [%s]",valtostring(a), valtostring(b));
    tstcheck(!valeq(a,b));

    bufclearstore(1);
    tstcheck(!valisstored(b));
    
  }
}