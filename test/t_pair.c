//  SPDX-FileCopyrightText: © 2025 Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT

#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "val.h"

tstsuite("Val Library Core Functionality", core) {
    tstcase("Value Creation") {
        val_t v = valpair(0xA1A2,0xB1B2B3B4);
        tstnote("pair: %s", valtostr(v,"").str);
        tstnote("pair: %s", valtostr(v).str);
        tstnote("pair: %s", valtostr(v,"<%" PRIu16 ",%" PRIu32 ">").str);
        tstcheck(valleft(v) == 0xA1A2);
        tstcheck(valright(v) == 0xB1B2B3B4);
    }
}
