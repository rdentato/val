
#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"


tstsuite("Val Library labels") {
    valstr_t lbl_str;
    val_t lbl = valnil;

    tstcase("Create a label") {
      lbl = vallabel("Hello");
      lbl_str = valtostr(lbl);

      tstnote("Hello: %016" PRIX64 "  \"%s\"", lbl.v, lbl_str.str);
      tstnote("Hello: %016" PRIX64 "  \"%s\"", lbl.v, valtostr(lbl).str);

      tstcheck(strcmp(lbl_str.str,"Hello") == 0);
    }

    tstcase("Wrong chars") {
      lbl = vallabel("A!B");
      
      tstcheck(strcmp( (lbl_str = valtostr(lbl)).str,"A") == 0, "buf: %.8s",lbl_str.str);

      lbl = vallabel("A:B");
      lbl_str = valtostr(lbl);
      tstcheck(strcmp(lbl_str.str,"A") == 0, "buf: %.8s",lbl_str.str);

      lbl = vallabel("!AB");
      lbl_str = valtostr(lbl);
      tstcheck(strcmp(lbl_str.str,"") == 0, "buf: %.8s",lbl_str.str);
    }

    tstcase("Wrong length") {
      lbl_str = valtostr(vallabel(""));
      tstcheck(strcmp(lbl_str.str,"") == 0, "buf: %.8s",lbl_str.str);

      lbl_str = valtostr(vallabel("01234567"));
      tstcheck(strcmp(lbl_str.str,"01234567") == 0, "buf: %.8s",lbl_str.str);

      lbl_str = valtostr(vallabel("012345678"));
      tstcheck(strcmp(lbl_str.str,"01234567") == 0, "buf: %.8s",lbl_str.str);
    }

    tstcase("Wrong type") {
      val_t x = val(12);
      tstcheck(strcmp(vallabeltostr(x).str,"") == 0);
    }

    tstcase("Compare") {
      val_t lbl2 = vallabel("Hello");
      lbl = vallabel("Hello");

      tstcheck(valeq(lbl,lbl2));
      tstcheck(valcmp(lbl,lbl2) == 0);

      tstcheck(valcmp(lbl,"Hello") == 0);
      tstcheck(valcmp("Hello",lbl) == 0);

      tstcheck(valcmp(lbl,"pippo") != 0);

      tstcheck(valislabel(lbl,"Hello"));
      tstcheck(!valislabel(lbl,"pippo"));

      tstcheck(valhash(lbl) == valhash("Hello"));
    }

}
