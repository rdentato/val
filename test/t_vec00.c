#include "tst.h"

#define VEC_MAIN
#include "vec.h"


void other_tests()
{
    tstcase("Checking something") {
        int j=0;
        tstcheck(0,"Surprise %d is false!",0);
        tstclk("Doing something %s useless.","almost") {
            for (int k=0; k < 0x007FFFFF; k++) j = k + tst_zero;
        }
        tstcheck(j>0);
        tstgroup(0,"No Data to check things") {
            tstcheck(j>0);
        }
        tstgroup(1,"No Files to check things") {
            tstcheck(j>0);
        }
        tstdata("Some unstructured data") {
            fprintf(stderr,"Asdk askd asd asdl aslds\nasskald asda sd\nsakdkasdèkas\n");
        }
        tstnote("This is quite important");
    }
}

int main(int argc, char *argv[])
{
  vec_t v=NULL;
  val_t x;
  tstrun() {

    tst_case("Create & Destroy vectors") {
      tstcheck((v = vecnew()) != NULL);
      tstcheck((v = vecfree(v)) == NULL);
      tstcheck((v = vecfree(v)) == NULL);
    }

    tst_case("Setting values as an arry") {
      tstcheck((v = vecnew()) != NULL);
      tstcheck(vecset(v,2,43) == 2);
      tstcheck(veccount(v) == 3);
      tstcheck(vecset(v,1,32) == 1);
      tstcheck(veccount(v) <= vecsize(v));
      tstcheck(vecset(v,223,100) == 223);
      tstcheck(veccount(v) <= vecsize(v));
      tstcheck(vecset(v,12,"100") == 12);
      v = vecfree(v);
      tstcheck(vecset(v,1,32) == VECNONDX && errno == EINVAL);
    }

    tstcase("Getting value from an array") {
      tstassert(v = vecnew());
      vecset(v,0,132);
      vecset(v,1,"pippo");
      vecset(v,3,valfalse);
      tstcheck(veccount(v) == 4);
      x = vecget(v,0); 
      tstcheck(!valisnil(x));
  
    }
    // other_tests();

  }
}