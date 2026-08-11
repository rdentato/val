//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "vallib.h"

tstsuite("buffers") {
  tstcase("Create and destroy") {
    val_t vb = valbufnew(0);
    tstcheck(valisbufptr(vb));
    tstcheck(valbuflen(vb) == 0);
    tstcheck(valbufsze(vb) == 0);

    vb = valfree(vb);
    tstcheck(!valisbufptr(vb));
    tstcheck(valisnullptr(vb));
  }

  tstcase("Create with initial capacity") {
    val_t vb = valbufnew(100);
    tstcheck(valisbufptr(vb));
    tstcheck(valbufsze(vb) >= 100);
    tstcheck(valbuflen(vb) == 0);
    tstcheck(valbuftxt(vb) != NULL);
    tstcheck(valbuftxt(vb)[0] == '\0');

    vb = valfree(vb);
  }

  tstcase("valbufcpy and valbufcat") {
    val_t vb = valbufnew(0);

    tstcheck(valbufcpy(vb, "Hello"));
    tstcheck(valbuflen(vb) == 5);
    tstcheck(strcmp(valbuftxt(vb), "Hello") == 0);

    tstcheck(valbufcat(vb, " World"));
    tstcheck(valbuflen(vb) == 11);
    tstcheck(strcmp(valbuftxt(vb), "Hello World") == 0);

    tstcheck(valbufcpy(vb, ""));
    tstcheck(valbuflen(vb) == 0);
    tstcheck(valbuftxt(vb)[0] == '\0');

    vb = valfree(vb);
  }

  tstcase("valbufadd") {
    val_t vb = valbufnew(0);

    tstcheck(valbufadd(vb, "Hello", 5));
    tstcheck(valbuflen(vb) == 5);
    tstcheck(strcmp(valbuftxt(vb), "Hello") == 0);

    tstcheck(valbufadd(vb, "!", 1));
    tstcheck(strcmp(valbuftxt(vb), "Hello!") == 0);

    vb = valfree(vb);
  }

  tstcase("valbuffmt and valbufcatf") {
    val_t vb = valbufnew(0);

    tstcheck(valbuffmt(vb, "%s %d %x", "value", 42, 0x1F));
    tstcheck(valbuflen(vb) == (int32_t)strlen("value 42 1f"));
    tstcheck(strcmp(valbuftxt(vb), "value 42 1f") == 0);

    // valbuffmt overwrites
    tstcheck(valbuffmt(vb, "%s", "abc"));
    tstcheck(strcmp(valbuftxt(vb), "abc") == 0);

    // valbufcatf appends
    tstcheck(valbufcatf(vb, "-%d", 7));
    tstcheck(strcmp(valbuftxt(vb), "abc-7") == 0);

    vb = valfree(vb);
  }

  tstcase("valbufins") {
    val_t vb = valbufnew(0);
    valbufcpy(vb, "ac");

    tstcheck(valbufins(vb, 1, "b"));
    tstcheck(strcmp(valbuftxt(vb), "abc") == 0);

    tstcheck(valbufins(vb, 0, "_"));
    tstcheck(strcmp(valbuftxt(vb), "_abc") == 0);

    tstcheck(valbufins(vb, 4, "!"));
    tstcheck(strcmp(valbuftxt(vb), "_abc!") == 0);

    // Negative index: count from the end
    tstcheck(valbufins(vb, -1, "X"));
    tstcheck(strcmp(valbuftxt(vb), "_abcX!") == 0);

    // Out of range
    errno = 0;
    tstcheck(!valbufins(vb, 99, "z"));
    tstcheck(errno == ERANGE);

    vb = valfree(vb);
  }

  tstcase("valbufdel") {
    val_t vb = valbufnew(0);
    valbufcpy(vb, "Hello World");

    tstcheck(valbufdel(vb, 5, 5));   // remove the space
    tstcheck(strcmp(valbuftxt(vb), "HelloWorld") == 0);

    tstcheck(valbufdel(vb, 0, 4));   // remove range at the start
    tstcheck(strcmp(valbuftxt(vb), "World") == 0);

    valbufcpy(vb, "Hello World");
    tstcheck(valbufdel(vb, 6, -1));  // negative to: to the end
    tstcheck(strcmp(valbuftxt(vb), "Hello ") == 0);

    // Invalid range
    errno = 0;
    tstcheck(!valbufdel(vb, 5, 2));
    tstcheck(errno == ERANGE);

    vb = valfree(vb);
  }

  tstcase("valbufclear and valbufreserve") {
    val_t vb = valbufnew(0);
    valbufcpy(vb, "Hello");
    tstcheck(valbufreserve(vb, 100));
    tstcheck(valbufsze(vb) >= 100);

    tstcheck(valbufclear(vb));
    tstcheck(valbuflen(vb) == 0);
    tstcheck(strcmp(valbuftxt(vb), "") == 0);

    vb = valfree(vb);
  }

  tstcase("String semantics") {
    val_t vb = valbufnew(0);
    valbufcpy(vb, "Hello");

    tstcheck(valcmp(vb, "Hello") == 0);
    tstcheck(valcmp(vb, "Hello!") != 0);
    tstcheck(valhash(vb) == valhash(val("Hello")));
    tstcheck(strcmp(((valptr_buf_t)valtoptr(vb))->text, "Hello") == 0);

    vb = valfree(vb);
  }

  tstcase("Growth beyond initial capacity") {
    val_t vb = valbufnew(8);
    char big[2000];
    for (int k = 0; k < 1999; k++) big[k] = (char)('a' + (k % 26));
    big[1999] = '\0';

    tstcheck(valbufcpy(vb, big));
    tstcheck(valbuflen(vb) == 1999);
    tstcheck(strcmp(valbuftxt(vb), big) == 0);

    tstcheck(valbufcat(vb, big));
    tstcheck(valbuflen(vb) == 3998);
    tstcheck(valbuftxt(vb)[3998] == '\0');

    vb = valfree(vb);
  }

  tstcase("Error handling") {
    val_t vb = valbufnew(0);

    errno = 0;
    tstcheck(!valbufcpy(valnil, "x"));
    tstcheck(errno == EINVAL);

    errno = 0;
    tstcheck(!valbufcpy(vb, NULL));
    tstcheck(errno == EINVAL);

    tstcheck(valbuflen(valnil) == -1);
    tstcheck(valbufsze(valnil) == -1);
    tstcheck(valbuftxt(valnil) == NULL);

    vb = valfree(vb);
  }
}
