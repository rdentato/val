#include "tst.h"

#define VEC_MAIN
#include "vec.h"

tsttags(large,medium)

int main(int argc, char *argv[])
{
  val_t v = valnil;
  val_t x = valnil;

  tstsettags(argc,argv);

  tstrun() {

    tstcase("Create & Destroy vectors") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(valisnil((v = vecfree(v))));
      tstcheck(valisnil((v = vecfree(v))));
    }

    tstcase("Setting values as an array") {
      tstcheck(!valisnil((v = vecnew())));
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
      tstassert(!valisnil((v = vecnew())));
      vecset(v,0,132);
      vecset(v,1,"pippo");
      vecset(v,3,valfalse);
      tstcheck(veccount(v) == 4);
      x = vecget(v,0); 
      tstcheck(!valisnil(x));
      v = vecfree(v);
    }

    tstcase("insert value in an array") {
      tstassert(!valisnil((v = vecnew())));
      vecset(v,0,132);
      vecset(v,1,"pippo");
      vecset(v,2,"pluto");
      vecset(v,3,valtrue);
      vecset(v,4,valempty);
      vecset(v,5,valfalse);
      vecset(v,6,valtrue);
      tstcheck(veccount(v) == 7);
      tstcheck(valeq(vecget(v,4),valempty));

      vecins(v,4,valdeleted);

      tstcheck(veccount(v) == 8);
      tstcheck(valeq(vecget(v,4),valdeleted));
      tstcheck(valeq(vecget(v,5),valempty),"Explected %16lX got: %16lX",valempty.v,vecget(v,5).v);

      v = vecfree(v);
    }

    tstcase("delete value from an array") {
      tstassert(!valisnil((v = vecnew())));
      vecset(v,0,132);
      vecset(v,1,"pippo");
      vecset(v,2,"pluto");
      vecset(v,3,valtrue);
      vecset(v,4,valempty);
      vecset(v,5,valfalse);
      vecset(v,6,valmarked);
      tstcheck(veccount(v) == 7);
      tstcheck(valeq(vecget(v,4),valempty));

      vecdel(v,3,4);
      
      tstcheck(veccount(v) == 5);
      tstcheck(valeq(vecget(v,3),valfalse));
      tstcheck(valeq(vecget(v,4),valmarked),"Explected %16lX got: %16lX",valempty.v,vecget(v,5).v);

      v = vecfree(v);
    }

    tstcase("Deleting elements from the top") {
      v = vecnew();
      for (int k=0; k< 5; k++)
        vecset(v,k,100+k);
      tstcheck(veccount(v) == 5);
      tstcheck(valeq(102,vecget(v,2)));
      vecdel(v,veccount(v)-1);
      tstcheck(veccount(v) == 4, "Count: %d", veccount(v));
      v = vecfree(v);
    }

    tstcase("Size") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(vecsize(v)==0);
      vecset(v,10,3.4);
      tstcheck(veccount(v) == 11);
      tstcheck(vecsize(v) >= 11);
      tstcheck(vecsize(v,100) >= 100);
      tstcheck(veccount(v) == 11);
      v = vecfree(v);
    }
    
    tstcase("Counting") {
      uint32_t n;
      tstcheck(veccount(v,7)==0 && errno == EINVAL);
      tstassert(!valisnil((v = vecnew())));
      tstcheck(veccount(v) == 0);
      tstcheck(veccount(v,7) == 7);
      tstcheck((n=vecsize(v)) > 7);
      tstcheck(veccount(v,5) == 5);
      tstcheck(veccount(v) == 5);
      v = vecfree(v);      
      tstcheck(veccount(v,7)==0 && errno == EINVAL);
    }

    tstgroup(tsttag(large), "Large array test disabled") {
      tstcase("large array") {
        tstassert(!valisnil((v = vecnew())));
        for (int k=0; k<100; k++) vecset(v,k,1000+k);
        tstcheck(veccount(v) == 100);
        for (int k=0; k<100; k++) 
          tstcheck(valeq((x=vecget(v,k)),1000+k)); 
        v = vecfree(v);
      }
    }
  }
  if (valisvec(v)) v=vecfree(v);
  tstcheck(valisnil(v));
}