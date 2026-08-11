//  SPDX-FileCopyrightText: © 2026 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vallib.h"

// Stored strings: val_store() copies a NUL-terminated string into the
// library-managed arena and returns a val_t handle that must behave as a
// string in every val function (valcmp, valhash, valtostr, valtoptr, valtoint).

tstsuite("Stored strings") {

  tstcase("store and recall") {
    val_t s = valstore("pippo");
    tstcheck(valisstored(s));
    char *p = valrecall(s);
    tstcheck(p != NULL);
    tstcheck(p != NULL && strcmp(p,"pippo") == 0);
  }

  tstcase("stored values behave as strings everywhere") {
    val_t s = valstore("pippo");
    // valcmp by content, against literals and other stored values
    tstcheck(valcmp(s, val("pippo")) == 0);
    tstcheck(valcmp(val("pippo"), s) == 0);
    tstcheck(valcmp(s, valstore("pippo")) == 0);
    tstcheck(valcmp(s, valstore("pluto")) != 0);
    // valhash consistent with a plain string
    tstcheck(valhash(s) == valhash(val("pippo")));
    // valtostr prints the content
    tstcheck(strcmp(valtostr(s).str, "pippo") == 0);
    // valtoptr returns the content pointer
    tstcheck(valtoptr(s) != NULL && strcmp((char *)valtoptr(s), "pippo") == 0);
    // valtoint parses the content
    tstcheck(valtoint(valstore("123")) == 123);
    tstcheck(valtoint(valstore("-45")) == -45);
    tstcheck(valtoint(valstore("pippo")) == 0);
  }

  tstcase("empty and edge cases") {
    val_t e = valstore("");
    tstcheck(valisstored(e));
    tstcheck(strcmp(valrecall(e), "") == 0);
    tstcheck(valcmp(e, val("")) == 0);
    tstcheck(valtoint(e) == 0);
    // NULL input is rejected
    val_t n = valstore(NULL);
    tstcheck(valisnil(n) && errno == EINVAL);
  }

  tstcase("recall at scale") {
    // Exercises the offset binary search in val_recall_1() with a large
    // registry, including interleaved tombstones (valdel keeps the slots).
    enum { N = 10000 };
    val_t vals[N];
    char buf[32];

    for (int i = 0; i < N; i++) {
      snprintf(buf, sizeof buf, "s-%05d", i);
      vals[i] = valstore(buf);
      tstcheck(valisstored(vals[i]));
    }

    for (int i = 0; i < N; i++) {
      char expected[32];
      snprintf(expected, sizeof expected, "s-%05d", i);
      char *p = valrecall(vals[i]);
      tstcheck(p != NULL && strcmp(p, expected) == 0);
    }

    // Delete every third entry: the registry keeps their slots, so the
    // offsets stay sorted and the search must still find the survivors.
    for (int i = 0; i < N; i += 3) tstcheck(valdel(vals[i]) == 1);

    for (int i = 0; i < N; i++) {
      char expected[32];
      snprintf(expected, sizeof expected, "s-%05d", i);
      char *p = valrecall(vals[i]);
      if (i % 3 == 0) { tstcheck(p == NULL); }
      else { tstcheck(p != NULL && strcmp(p, expected) == 0); }
    }

    // Content semantics still hold at scale
    tstcheck(valcmp(vals[1], val("s-00001")) == 0);
    tstcheck(valcmp(vals[9998], val("s-09998")) == 0); // 9998 % 3 == 2: alive
  }
}
