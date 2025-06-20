//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"


tstsuite("Val Library syms") {
    valstr_t num_vstr;
    val_t num = valnil;
    uint32_t n;

    tstcase("Create a num") {
      num = valconst(721);
      num_vstr = valtostr(num);

      n = valtounsignedint(num);
      tstcheck(n == 721);
      tstnote("Hello: %016" PRIX64 "  \"%s\"", num.v, num_vstr.str);
      tstnote("Hello: %016" PRIX64 "  \"%s\"", num.v, valtostr(num).str);

      tstcheck(strcmp(num_vstr.str,"<2D1>") == 0, "str: %s",num_vstr.str);
      tstcheck(strcmp(valtostr(num,"<%d>").str,"<721>") == 0, "str: %s",num_vstr.str);
    }


}
