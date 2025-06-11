
#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"


tstsuite("Val Library labels") {
    char lbl_buf[30];
    const char *lbl_str;
    val_t lbl = valnil;

    tstcase("Create a label") {
      lbl = vallabel("Hello");
      lbl_str = valtostring(lbl, lbl_buf);
      tstcheck(lbl_str == lbl_buf);
      tstnote("Hello: %016" PRIX64 "  \"%s\"", lbl.v, lbl_str);

      tstcheck(strcmp(lbl_str,"Hello") == 0);
    }

    tstcase("Wrong chars") {
      lbl = vallabel("A!B");
      tstcheck(strcmp(valtostring(lbl,lbl_buf),"A") == 0, "buf: %.8s",lbl_buf);

      lbl = vallabel("A:B");
      tstcheck(strcmp(valtostring(lbl,lbl_buf),"A") == 0, "buf: %.8s",lbl_buf);

      lbl = vallabel("!AB");
      tstcheck(strcmp(valtostring(lbl,lbl_buf),"") == 0, "buf: %.8s",lbl_buf);
    }

    tstcase("Wrong length") {
      lbl = vallabel("");
      tstcheck(strcmp(valtostring(lbl,lbl_buf),"") == 0, "buf: %.8s",lbl_buf);

      lbl = vallabel("01234567");
      tstcheck(strcmp(valtostring(lbl,lbl_buf),"01234567") == 0, "buf: %.8s",lbl_buf);

      lbl = vallabel("012345678");
      tstcheck(strcmp(valtostring(lbl,lbl_buf),"01234567") == 0, "buf: %.8s",lbl_buf);
    }

    tstcase("Compare") {
      val_t lbl2 = vallabel("Hello");
      lbl = vallabel("Hello");
      tstcheck(valeq(lbl,lbl2));

      tstcheck(valcmp(lbl,"Hello") == 0);
      tstcheck(valcmp("Hello",lbl) == 0);

      tstcheck(valcmp(lbl,"pippo") != 0);

      tstcheck(valislabel(lbl,"Hello"));
      tstcheck(!valislabel(lbl,"pippo"));

      tstcheck(valhash(lbl) == valhash("Hello"));
    }
}
