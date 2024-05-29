#include "tst.h"
#include "val.h"

tstsuite("Array tests",nolarge) {

    val_t v = valnil;
    val_t x = valnil;

    tstcase("Create & Destroy vectors") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(valisnil((v = vecfree(v))));
      tstcheck(valisnil((v = vecfree(v))));
    }

    tstcase("Setting values as an array") {
      tstcheck(!valisnil((v = vecnew())));
      tstcheck(valeq(vecset(v,2,43),43));  // vecset will return the "slot indexx" were the value has been stored.
      tstcheck(veccount(v) == 3);
      tstcheck(valeq(vecset(v,1,32),32));
      tstcheck(veccount(v) <= vecsize(v));
      tstcheck(valeq(vecset(v,223,100),100));
      tstcheck(vecsize(v) >= 223);
      tstcheck(veccount(v) == 224);
      tstcheck(veccount(v) < vecsize(v));
      tstcheck(strcmp(valtostring(vecset(v,12,"100")),"100") == 0);
      v = vecfree(v);
      tstcheck(valeq(vecset(v,1,32), valerror) && errno == EINVAL);
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

    tstcase("Set will zero previusly unused memory") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(veccount(v) == 0);

      tstcheck(valisnil(vecget(v,0)));

      tstcheck(valeq(vecset(v,2,6.5),6.5));
      tstcheck(veccount(v) == 3);

      tstcheck(valisdouble(vecget(v,0)));
      tstcheck(valeq(vecget(v,0),0.0));

      tstcheck(valisdouble(vecget(v,1)));
      tstcheck(valeq(vecget(v,1),0.0));

      v = vecfree(v);      
    }

    tstcase("makegap will zero the created gap") {
      tstassert(!valisnil((v = vecnew())));
      tstcheck(veccount(v) == 0);

      tstcheck(valeq(vecset(v,0,100),100));
      tstcheck(valeq(vecset(v,1,101),101));
      tstcheck(valeq(vecset(v,2,102),102));
      tstcheck(valeq(vecset(v,3,103),103));
      tstcheck(veccount(v) == 4);

      tstcheck(vecmakegap(v,1,2));
      tstcheck(veccount(v) == 6, "Expected 6 got: %d",veccount(v));

      tstcheck(valeq(vecget(v,3),101));
      tstcheck(valeq(vecget(v,4),102));
      tstcheck(valeq(vecget(v,5),103));

      tstcheck(valisdouble(vecget(v,1)));
      tstcheck(valeq(vecget(v,1),0.0));

      tstcheck(valisdouble(vecget(v,2)));
      tstcheck(valeq(vecget(v,2),0.0));

      tstcheck(vecmakegap(v,10)); // extend the vec with 10 elements
      tstcheck(veccount(v) == 16);

      tstcheck(valisdouble(veclast(v)));
      tstcheck(valeq(veclast(v),0.0));
      tstcheck(vecindex(v,VECLASTNDX) == veccount(v)-1);
      tstcheck(vecindex(v,VECLASTNDX,-2) == veccount(v)-3);

      tstcheck(vecindex(v,VECLASTNDX,-30) == VECERRORNDX);

      v = vecfree(v);      
    }

    tstcase("large array") {
      tstskipif(tsttag(nolarge)) {
        tstassert(!valisnil((v = vecnew())));
        for (int k=0; k<100 && !errno ; k++) vecset(v,k,1000+k);
        tstcheck(veccount(v) == 100);
        for (int k=0; k<veccount(v); k++) 
          tstcheck(valeq((x=vecget(v,k)),1000+k)); 
        v = vecfree(v);
      }
    }
    if (valisvec(v)) v=vecfree(v);
    tstcheck(valisnil(v));

}