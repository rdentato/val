#include "tst.h"
#include "val.h"


tstsuite("Buffer search tests") {

  val_t b = valnil;
  uint32_t n;

  tstcase("Search in a non-buffer") {
    tstcheck((bufsearch(val(9),"ll") == BUFNONDX) && (errno == EINVAL));
  }

  tstassert(!valisnil((b = bufnew())));

  tstcase("Search in a freshly created buffer") {
    tstcheck((bufsearch(b,"ll") == BUFNONDX) && (errno == 0));
  }

  bufputs(b,"Hello World");

  tstcase("Search a non-existing string from the current position") {
    tstcheck((bufsearch(b,"XX") == BUFNONDX) && (errno == 0));
  }

  tstcase("Search an existing string with current position at the end") {
    tstcheck((bufsearch(b,"XX") == BUFNONDX) && (errno == 0));
  }

  tstcase("Search a non-existing string from the start") {
    uint32_t p = bufpos(b);
    tstcheck(((n=bufsearch(b,"XX",0)) == BUFNONDX) && (errno == 0),"found: %d errno: %d",n,errno);
    tstcheck(p == bufpos(b));
  }

   tstcase("Search an existing string from the start") {
    tstcheck(strcmp("Hello World",buf(b,0))==0);
    tstcheck(((n=bufsearch(b,"ll",0)) == 2) && (errno == 0),"found: %d errno: %d",n,errno);
    tstcheck(n == bufpos(b));
  }

 if (valisbuf(b)) b=buffree(b);
  tstcheck(valisnil(b));
}