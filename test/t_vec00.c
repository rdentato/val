#include "tst.h"

#define VEC_MAIN
#include "vec.h"


int main(int argc, char *argv[])
{
  vec_t v=NULL;
  tst("Create vector") {
    tstcheck((v = vecnew()) != NULL);
    tstcheck((v = vecfree(v)) == NULL);
    tstcheck((v = vecnew()) != NULL);
  }
  tst("Setting value as an arry") {
    tstcheck((v = vecnew()) != NULL);
    tstcheck(vecset(v,2,43) == 2);
    tstcheck(veccount(v) == 3);
    tstcheck(vecset(v,1,32) == 1);
    tstcheck(veccount(v) <= vecsize(v));
    tstcheck(vecset(v,223,100) == 223);
    tstcheck(veccount(v) <= vecsize(v));

  }
}