//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"


tstsuite("Val Library valtostr") {
    val_t v = valnil;
    valstr_t v_str;

    tstcase("Default formatters") {
      v = vallabel("Hello");
      tstcheck(strcmp(valtostr(v).str,"Hello") == 0);

      v = val(7);
      tstcheck(strcmp(valtostr(v).str,"7") == 0);

      v = val(4.32);
      tstcheck(strcmp(valtostr(v).str,"4.320000") == 0);

      char *s = "World";
      v = val(s);
      tstcheck(strcmp(valtoptr(v),"World") == 0); 
      tstcheck(strtoull(valtostr(v).str,NULL,16) == (uintptr_t)s);

      v = valconst(43);
      tstcheck(strcmp(valtostr(v).str,"<2B>") == 0,"Got: %s", valtostr(v).str);

      tstcheck(strcmp(valtostr(valnil).str,"nil") == 0,"Got: %s", valtostr(v).str);
      tstcheck(strcmp(valtostr(valtrue).str,"true") == 0,"Got: %s", valtostr(v).str);
      tstcheck(strcmp(valtostr(valfalse).str,"false") == 0,"Got: %s", valtostr(v).str);

    }

    tstcase("Custom formatters") {
      v = val(7); v_str = valtostr(v,"%03d");
      tstcheck(strcmp(v_str.str,"007") == 0,"Got: %s", v_str.str);

      v = val(4.32); v_str = valtostr(v,"|%.03f|");
      tstcheck(strcmp(v_str.str,"|4.320|") == 0,"Got: %s", v_str.str);

      v = valconst(43); v_str = valtostr(v,"[%0X]");
      tstcheck(strcmp(v_str.str,"[2B]") == 0,"Got: %s", valtostr(v).str);
    }

    tstcase("Empty formatter") {
      v_str = valtostr(valnil,"");
      tstnote("valnil: %s",v_str.str);
      tstcheck(strtoull(v_str.str,NULL,16) == valnil.v);
    }
}

