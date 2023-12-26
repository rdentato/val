#include <math.h>
#include <limits.h>
#include "tst.h"
#include "val.h"

tstsuite("Edge Cases and Limits") {
  tstcase("32 bits Integer Values") {
    int max = (unsigned int)LONG_MAX;
    int min = (unsigned int)LONG_MIN;
    tstnote("integers are limited to 48bits, not 64");
    tstcheck(valtointeger(val(max)) == (max), "Failed at LONG_MAX (0x%X instead of 0x%X)",(int)valtointeger(val(max)),max );
    tstcheck(valtointeger(val(min)) == (min), "Failed at LONG_MIN");
    tstcheck(valtointeger(val(max)) == (max), "Failed at LONG_MAX (0x%X instead of 0x%X)",(int)valtointeger(val(max)),(int)(max & VAL_PAYLOAD));
    tstcheck(valtointeger(val(min)) == (min), "Failed at LONG_MIN");

    unsigned int umax = (unsigned int)LONG_MAX;
    unsigned int umin = (unsigned int)LONG_MIN;
    tstcheck(valtointeger(val(umax)) == (umax), "Failed at LONG_MAX (0x%X instead of 0x%X)",(unsigned int)valtointeger(val(umax)),umax );
    tstcheck(valtointeger(val(umin)) == (umin), "Failed at LONG_MIN");
    tstcheck(valtointeger(val(umax)) == (umax), "Failed at LONG_MAX (0x%X instead of 0x%lX)",(unsigned int)valtointeger(val(umax)),(umax & VAL_PAYLOAD));
    tstcheck(valtointeger(val(umin)) == (umin), "Failed at LONG_MIN");
  }

  tstcase("Extreme Integer Values") {
    long max = LONG_MAX;
    long min = LONG_MIN;
    tstnote("integers are limited to 48bits, not 64");
    tstcheck(valtointeger(val(max)) != (max), "Failed at LONG_MAX (0x%lX instead of 0x%lX)",valtointeger(val(max)),max );
    tstcheck(valtointeger(val(min)) != (min), "Failed at LONG_MIN");
    tstcheck(valtointeger(val(max)) != (max & VAL_PAYLOAD), "Failed at LONG_MAX (0x%lX instead of 0x%lX)",valtointeger(val(max)),(max & VAL_PAYLOAD));
    tstcheck(valtointeger(val(min)) == (min & VAL_PAYLOAD), "Failed at LONG_MIN");

    unsigned long umax = (unsigned long)LONG_MAX;
    unsigned long umin = (unsigned long)LONG_MIN;
    tstcheck(valtointeger(val(umax)) != (umax), "Failed at LONG_MAX (0x%lX instead of 0x%lX)",valtointeger(val(umax)),umax );
    tstcheck(valtointeger(val(umin)) != (umin), "Failed at LONG_MIN");
    tstcheck(valtointeger(val(umax)) == (umax & VAL_PAYLOAD), "Failed at LONG_MAX (0x%lX instead of 0x%lX)",valtointeger(val(umax)),(umax & VAL_PAYLOAD));
    tstcheck(valtointeger(val(umin)) == (umin & VAL_PAYLOAD), "Failed at LONG_MIN");
  }

  tstcase("Floating Point Limits") {
    double inf = INFINITY;
    double nan = NAN;
    tstcheck(isinf(valtodouble(val(inf))), "Failed at INFINITY");
    tstcheck(isnan(valtodouble(val(nan))), "Failed at NAN");
  }

  tstcase("Null Pointer and Empty String") {
    char *null_str = NULL;
    tstcheck(valtopointer(val(null_str)) == NULL, "Failed at NULL pointer");
    tstcheck(strcmp(valtostring(val("")), "") == 0, "Failed at empty string");
  }

  // Add similar cases for other types
}
