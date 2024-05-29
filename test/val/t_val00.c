#include "tst.h"
#include "val.h"

#define true  ((_Bool)1)
#define false ((_Bool)0)
#define bool _Bool

tstsuite("Basic val tests")
{
     val_t x;

     tstcase("Check Type") {
       x = val(12);
       tstcheck(valisinteger(x));
  
       x = val(true);
       tstcheck(valisbool(x));
  
       x = val(1.3);
       tstcheck(valisdouble(x));
  
       tstcheck(valisinteger(3));
       tstcheck(!valisinteger(3.5));
       tstcheck(valisdouble(3.5));
     }
    
     tstcase("Check conversion") {
       float x_f = valtofloat(x);
  
       x = val(x_f);
       tstcheck(valisdouble(x));
     
       x = val("pippo");
       tstcheck(valisstring(x));
  
       x = val((long)-2);
       tstcheck(valisinteger(x));
  
       long r = valtointeger(x);
       tstcheck(r == -2);
       
       x = val((long)-2);
       long s = valtointeger(x);
       tstcheck(s == -2);

       tstcheck(valtointeger(3) == 3);
       tstcheck(valtodouble(3.5) == 3.5);

       // This must be different because only 48 bits are allowed
       x = val((unsigned long)-2);
       unsigned long u = valtointeger(x);
       tstcheck(u != -2);
     }
  
     tstcase("Check Constant") {
       x = valnil;
       tstcheck(valisnil(x));
    
       x = val(0.0);
       tstcheck(valisdouble(x));
       tstcheck(valtodouble(x) == 0.0);
       tstcheck(valiszero(x));
    
       x = val(NULL);
       tstcheck(valispointer(x));
       tstcheck(valtopointer(x) == NULL);
       tstcheck(valiszero(x));
    
       x = val("Hello!");
       tstcheck(strcmp("Hello!",valtostring(x)) == 0);
    }

    tstcase("Constants") {
      x = valconst(23);
      val_t y = val(23);

      tstcheck(!valeq(x,y));
      tstcheck(valtointeger(x) == valtointeger(y));
      tstcheck(valtobool(x) == valtobool(y));
      tstcheck(!valisinteger(x));
      tstcheck(valisinteger(y));
    }

    tstcheck("Function pointer") {
       val_t f = val(valcmp);
       tstcheck(valispointer(f));
    }

}