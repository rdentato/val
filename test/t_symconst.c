//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"


tstsuite("Val Library syms") {
    valstr_t sym_vstr;
    val_t sym = valnil;

    tstcase("Check") {
      char *s;
      s = "87654321";
      for (int i = 8; i>=0; i--) {
        sym = valconst(s+i);
        sym_vstr = valtostr(sym);
        tstnote("%d %d %10s: %016" PRIX64 "  \"%s\"", 8-i, valissymconst(sym), s+i, sym.v, sym_vstr.str);
      }
      s = "987654321";
      sym = valconst(s);
      sym_vstr = valtostr(sym);
      tstnote("%d %d %10s: %016" PRIX64 "  \"%s\"", 9, valissymconst(sym), s, sym.v, sym_vstr.str);

    }

    tstcase("Create a sym") {
      sym = valconst("Hello");
      sym_vstr = valtostr(sym);

      tstnote("Hello: %016" PRIX64 "  \"%s\"", sym.v, sym_vstr.str);
      tstnote("Hello: %016" PRIX64 "  \"%s\"", sym.v, valtostr(sym).str);

      tstcheck(strcmp(sym_vstr.str,"hello") == 0);
    }

    tstcase("Wrong chars") {
      sym = valconst("A,B");
      
      tstcheck(strcmp( (sym_vstr = valtostr(sym)).str,"A") == 0, "buf: %.8s",sym_vstr.str);

      sym = valconst("A^B");
      sym_vstr = valtostr(sym);
      tstcheck(strcmp(sym_vstr.str,"A") == 0, "buf: %.8s",sym_vstr.str);

      sym = valconst("'AB");
      sym_vstr = valtostr(sym);
      tstcheck(strcmp(sym_vstr.str,"") == 0, "buf: %.8s",sym_vstr.str);
    }

    tstcase("Wrong length") {
      sym_vstr = valtostr(valconst(""));
      tstcheck(strcmp(sym_vstr.str,"") == 0, "buf: %.8s",sym_vstr.str);

      sym_vstr = valtostr(valconst("01234567"));
      tstcheck(strcmp(sym_vstr.str,"01234567") == 0, "buf: %.8s",sym_vstr.str);

      sym_vstr = valtostr(valconst("012345678"));
      tstcheck(strcmp(sym_vstr.str,"01234567") == 0, "buf: %.8s",sym_vstr.str);
    }

    tstcase("Wrong type") {
      val_t x = val(12);
      tstcheck(strcmp(valsymtostr(x).str,"") == 0);
    }

    tstcase("Compare") {
      val_t sym2 = valconst("Hello");
      sym = valconst("Hello");

      tstcheck(valeq(sym,sym2));
      tstcheck(valcmp(sym,sym2) == 0);

      tstcheck(valcmp(sym,"hello") != 0);
      tstcheck(valcmp("hello",sym) != 0);

      tstcheck(valisconst(sym,"hello"));
      tstcheck(!valisconst(sym,"pippo"));

      tstcheck(valhash(sym) != valhash("hello"));
    }

    tstcase("toint") {
      sym = valconst("87654321");
      tstcheck(valtoint(sym) > ((uint64_t)1 << 40));
      tstnote("%016" PRIX64 " sym: '%s'",sym.v,valtostr(sym).str);

      sym = valconst(">>");
      tstcheck(valtoint(sym) > ((uint64_t)1 << 40));
      tstnote("%016" PRIX64 " sym: '%s'",sym.v,valtostr(sym).str);
    }

}
