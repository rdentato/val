//  SPDX-FileCopyrightText: © 2026 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <string.h>
#include "vallib.h"

// valmap: a vec used as a map. Sorted array of (key, value) pairs; keys are
// unique (inserting an existing key updates the value), lookups use valcmp
// (so 1 and 1.0, and stored strings vs literals, are the same key), deletes
// are tombstoned and compacted away.
// Note: valget/valdel dispatch int keys to the array, so integer map keys
// must be wrapped: valget(m, val(42)).

tstsuite("valmap") {

  tstcase("insert and get") {
    val_t m = valvec(0);
    tstcheck(!valisnil(m));
    tstcheck(valmap(m, "pippo", "pluto") == 1);
    tstcheck(valcount(m) == 1);
    tstcheck(valcmp(valget(m, "pippo"), "pluto") == 0);
    tstcheck(valcmp(valget(m, val("pippo")), "pluto") == 0); // val_t key
    // number keys (wrapped in val_t)
    tstcheck(valmap(m, 42, "answer") == 1);
    tstcheck(valcount(m) == 2);
    tstcheck(valcmp(valget(m, val(42)), "answer") == 0);
    // double value
    tstcheck(valmap(m, "pi", val(3.14)) == 1);
    tstcheck(valeq(valget(m, "pi"), val(3.14)));
    tstcheck(valcount(m) == 3);
  }

  tstcase("update in place") {
    val_t m = valvec(0);
    valmap(m, "k", "v1");
    tstcheck(valcount(m) == 1);
    valmap(m, "k", "v2");
    tstcheck(valcount(m) == 1); // no duplicate entry
    tstcheck(valcmp(valget(m, "k"), "v2") == 0);
  }

  tstcase("key equivalence via valcmp") {
    val_t m = valvec(0);
    valmap(m, 1, "one"); // int key
    tstcheck(valcmp(valget(m, val(1.0)), "one") == 0); // double key finds it
    valmap(m, "s", "sv");
    val_t skey = valstore("s"); // stored string key
    tstcheck(valcmp(valget(m, skey), "sv") == 0); // stored key finds literal
    valmap(m, skey, "sv2");     // stored key updates the literal-keyed entry
    tstcheck(valcount(m) == 2);
    tstcheck(valcmp(valget(m, "s"), "sv2") == 0);
  }

  tstcase("delete") {
    val_t m = valvec(0);
    valmap(m, "a", 1);
    valmap(m, "b", 2);
    valmap(m, "c", 3);
    tstcheck(valcount(m) == 3);
    tstcheck(valdel(m, "b") == 1);
    tstcheck(valcount(m) == 2);
    errno = 0;
    tstcheck(valisnil(valget(m, "b")) && errno == ENOENT);
    tstcheck(valdel(m, "b") == 0);  // already deleted
    tstcheck(valdel(m, "zz") == 0); // never existed
    tstcheck(valcmp(valget(m, "a"), 1) == 0);
    tstcheck(valcmp(valget(m, "c"), 3) == 0);
    // reinsert after delete
    valmap(m, "b", 20);
    tstcheck(valcount(m) == 3);
    tstcheck(valcmp(valget(m, "b"), 20) == 0);
  }

  tstcase("many entries, tombstones and compaction") {
    val_t m = valvec(0);
    for (int i = 0; i < 1000; i++) valmap(m, i, i*2);
    tstcheck(valcount(m) == 1000);
    for (int i = 0; i < 1000; i += 10) tstcheck(valeq(valget(m, val(i)), val(i*2)));
    // delete 300 entries (compaction triggers when tombstones reach 1/4)
    for (int i = 0; i < 300; i++) tstcheck(valdel(m, val(i)) == 1);
    tstcheck(valcount(m) == 700);
    errno = 0;
    tstcheck(valisnil(valget(m, val(100))) && errno == ENOENT);
    tstcheck(valeq(valget(m, val(300)), val(600)));
    tstcheck(valeq(valget(m, val(999)), val(1998)));
    // add more entries after compaction
    for (int i = 1000; i < 1500; i++) valmap(m, i, i*3);
    tstcheck(valcount(m) == 1200);
    tstcheck(valeq(valget(m, val(1499)), val(4497)));
    errno = 0;
    tstcheck(valisnil(valget(m, val(299))) && errno == ENOENT);
    tstcheck(valeq(valget(m, val(300)), val(600)));
  }

  tstcase("array ops unaffected") {
    val_t v = valvec(0);
    tstcheck(valset(v, 0, 99) == 1);
    tstcheck(valeq(valget(v, 0), 99));
    tstcheck(valcount(v) == 1);
    tstcheck(valdel(v, 0) == 1);
    tstcheck(valcount(v) == 0);
    // converting a non-empty vec into a map is refused
    valset(v, 0, 1);
    errno = 0;
    tstcheck(valmap(v, "k", "v") == 0 && errno == EINVAL);
    // valget with a val_t key on a plain array vec -> EINVAL
    errno = 0;
    tstcheck(valisnil(valget(v, val("k"))) && errno == EINVAL);
  }
}
