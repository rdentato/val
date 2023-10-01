#include "val.h"

#define true  ((_Bool)1)
#define false ((_Bool)0)
#define bool _Bool

#define check(x_) do { int x__ = !!(x_); fprintf(stderr, "%s: %s \xF:%s:%d\n", x__?"PASS":"FAIL", #x_, __FILE__, __LINE__);} while(0)

int main(int argc, char *argv[])
{

   val_t x;

   x = val(12);
   check(valisinteger(x));

   x = val(true);
   check(valisbool(x));

   x = val(1.3);
   check(valisdouble(x));

   float x_f = valtofloat(x);

   x = val(x_f);
   check(valisdouble(x));
   
   x = val("pippo");
   check(valisstring(x));

   x = val((long)-2);
   check(valisinteger(x));

   long r = valtointeger(x);
   check(r == -2);
 
   // This must be different because only 48 bits are allowed
   x = val((unsigned long)-2);
   unsigned long u = valtointeger(x);
   check(u != -2);

   x = valnil;
   check(valisnil(x));

   x = val(0.0);
   check(valisdouble(x));
   check(valtodouble(x) == 0.0);
   check(valiszero(x));

   x = val(NULL);
   check(valispointer(x));
   check(valtopointer(x) == NULL);
   check(valiszero(x));

   x = val("Hello!");
   check(strcmp("Hello!",valtostring(x)) == 0);
}