#include "tst.h"
#include "val.h"


tstsuite("Buffer delete") {

  val_t b = valnil;
  //uint32_t n;

  tstassert(!valisnil((b = bufnew())));
  tstassert(bufputs(b,"Hello World!") == 12);

  tstcase("Invalid arguments") {
    tstcheck(bufdel(val(3),2) == 0 && errno != 0);
  }

  tstcase("pos at the end") {
    tstcheck(bufpos(b) == buflen(b));
    tstcheck(bufdel(b,3) == 0 && errno == 0);
  }

  tstcase("Delete from the middle") {
    bufpos(b,1);
    tstcheck(bufdel(b,2) == 2 && errno == 0);
    tstcheck(strcmp("Hlo World!",buf(b, 0)) == 0);
    tstcheck(*buf(b) == 'l');
  }

  tstcase("Delete a single character from the middle") {
    bufpos(b,4);
    tstcheck(bufdel(b,1) == 1 && errno == 0);
    tstcheck(strcmp("Hlo orld!",buf(b, 0)) == 0);
  }

  tstcase("Delete the first character") {
    bufpos(b,0);
    tstcheck(bufdel(b,1) == 1 && errno == 0);
    tstcheck(strcmp("lo orld!",buf(b, 0)) == 0);
  }

  tstcase("Delete from the current position") {
    bufpos(b,2);
    tstcheck(bufdel(b) > 0 && errno == 0);
    tstcheck(strcmp("lo",buf(b, 0)) == 0);
  }

  tstcase("Delete the entire buffer") {
    bufpos(b,0);
    tstcheck(bufdel(b) > 0 && errno == 0);
    tstcheck(*buf(b, 0) == '\0' && bufpos(b) == 0 && buflen(b) == 0);
  }

  tstassert(valisnil(b=buffree(b)));

}