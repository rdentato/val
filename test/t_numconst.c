//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"

typedef enum color_e {COLOR = 0x10000000, RED, GREEN, BLUE } color_t;
typedef enum direction_e {DIRECTION = 0x20000000, NORTH, SOUTH, EAST, WEST} direction_t;

#define iscolor(v)     (valtouint32(v) & COLOR)
#define isdirection(v) (valtouint32(v) & DIRECTION)

tstsuite("Val Library syms") {
    valstr_t num_vstr;
    val_t num = valnil;
    uint32_t n;

    tstcase("Create a num") {
      num = valconst(721);
      tstcheck(valisconst(num,721));
      num_vstr = valtostr(num);

      n = valtounsignedint(num);
      tstcheck(n == 721);
      tstcheck(valtounsignedint(num) == 721);
      tstcheck(valisconst(num));
      tstcheck(valisnumconst(num));
      tstcheck(!valisbool(num));
      tstcheck(!valisnil(num));
      tstcheck(!valissymconst(num));

      tstnote("Hello: %016" PRIX64 "  \"%s\"", num.v, num_vstr.str);
      tstnote("Hello: %016" PRIX64 "  \"%s\"", num.v, valtostr(num).str);

      tstcheck(strcmp(num_vstr.str,"<2D1>") == 0, "str: %s",num_vstr.str);
      tstcheck(strcmp(valtostr(num,"[%d]").str,"[721]") == 0, "str: %s",num_vstr.str);

    }
   
    tstcase("Cross some val const") {
      tstcheck(valisconst(valnil));
      tstcheck(valisconst(valtrue));
      tstcheck(valisconst(valfalse));

      tstcheck(valtoint(valnil) == 0);
      tstcheck(valtoint(valtrue) == 1);
      tstcheck(valtoint(valfalse) == 0);

      val_t notconst = val(12);

      tstcheck(valeq(valnil,valconst(valnil)));
      tstcheck(valeq(valtrue,valconst(valtrue)));
      tstcheck(valeq(valfalse,valconst(valfalse)));
      tstcheck(valisnil(valconst(notconst)));
    }

   tstcase("Create from Enums") {
      val_t directions[4] = { valconst(NORTH), valconst(SOUTH), valconst(EAST), valconst(WEST) };

      tstcheck(valisconst(directions[0]));
      tstcheck(valisconst(directions[1]));
      tstcheck(valisconst(directions[2]));
      tstcheck(valisconst(directions[3]));

      tstcheck(valisconst(directions[0], NORTH));
      tstcheck(valisconst(directions[1], SOUTH));
      tstcheck(valisconst(directions[2], EAST));
      tstcheck(valisconst(directions[3], WEST));
      
      tstcheck(!valisconst(directions[0], SOUTH));
      tstcheck(!valisconst(directions[1], EAST ));
      tstcheck(!valisconst(directions[2], WEST ));
      tstcheck(!valisconst(directions[3], NORTH));
      
      tstcheck(isdirection(directions[1]));
      tstcheck(iscolor(valconst(RED)));
      tstcheck(!iscolor(valconst(NORTH)));
   }

}
