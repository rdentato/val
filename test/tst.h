
//  (C) by Remo Dentato (rdentato@gmail.com)
//  SPDX-License-Identifier: MIT
//  PackageVersion: 0.1.0 Beta

#ifndef TST_VERSION // 0.1.0-beta
#define TST_VERSION    0x0001000B

#include <stdio.h>
#include <errno.h>

static volatile int tst_zero = 0;
#define tstprintf(...)     (fprintf(stderr, __VA_ARGS__), fprintf(stderr, " \xF:%s:%d\n", __FILE__, __LINE__), tst_zero = 0)

#define tstcheck(x_,...)   do { errno = !(x_); tstprintf("%s %s", errno?"FAIL│":"PASS│", #x_);\
                                if (errno) {fprintf(stderr,"    │ ╰─ " __VA_ARGS__); fputc('\n',stderr);} \
                           } while(0)

#define tstmust(x_,...)    do { tstcheck(x_, __VA_ARGS__); if (errno) abort();} while(0)

#define tst(...)           for (int tst = !tstprintf("TEST┬── " __VA_ARGS__);  tst ; tst = 0, fprintf(stderr,"    ╰──\n")) 

#define tst_check(...)
#define tst_must(...)
#define tst_(...) 

#define tstblk   if (tst_zero) ; else
#define tst_blk  if (!tst_zero) ; else

#endif // TST_VERSION
