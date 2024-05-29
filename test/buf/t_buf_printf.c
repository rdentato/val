#include "tst.h"
#include "val.h"


tstsuite("Buffer printf") {

  val_t b = valnil;
  //uint32_t n;

  tstassert(!valisnil((b = bufnew())));

  tstcase("Invalid arguments") {
    tstcheck(bufprintf(val(3),"H: %d",2) == 0 && errno != 0);
  }

  tstcase("printf at the beginning") {
    bufpos(b,0);
    tstcheck(bufprintf(b,"s: %s!","X") == 5  && errno == 0 );
    tstcheck(strcmp("s: X!",buf(b, 0)) == 0, "Result: %s len:%d",buf(b, 0),buflen(b));
  }

  tstcase("printf at the end") {
    tstcheck(bufprintf(b," t: %d", 123) > 0  && errno == 0 );
    tstcheck(strcmp("s: X! t: 123",buf(b, 0)) == 0, "Result: %s len:%d",buf(b, 0),buflen(b));
  }

  tstcase("printf in the middle") {
    bufpos(b,3);
    tstcheck(bufprintf(b,"%d", 9) > 0  && errno == 0 );
    tstcheck(bufpos(b) == 4);
    tstcheck(strcmp("s: 9! t: 123",buf(b, 0)) == 0, "Result: '%s' len:%d",buf(b, 0),buflen(b));
  }


#if 0
  tstcase("Null insert") {
    tstcheck(bufins(b,"") == 0  && errno == 0 );
    tstcheck(strcmp("Hello World!",buf(b, 0)) == 0, "Result: %s len:%d",buf(b, 0),buflen(b));
    tstcheck(bufins(b,NULL) == 0  && errno == 0 );
    tstcheck(strcmp("Hello World!",buf(b, 0)) == 0, "Result: %s len:%d",buf(b, 0),buflen(b));
    tstcheck(bufins(b,0) == 0  && errno == 0 );
    tstcheck(strcmp("Hello World!",buf(b, 0)) == 0, "Result: %s len:%d",buf(b, 0),buflen(b));
  }

  tstcase("insert in the middle") {
    bufpos(b,4);
    tstcheck(bufins(b,2) == 2 && errno == 0);
    tstcheck(strcmp("Hell  o World!",buf(b, 0)) == 0);
    tstcheck(*buf(b) == 'o');

    tstcheck(bufins(b,"OO") == 2 && errno == 0);
    tstcheck(strcmp("Hell  OOo World!",buf(b, 0)) == 0);
    tstcheck(*buf(b) == 'o');
  }

  tstcase("ins at the beginning") {
    bufpos(b,0);
    tstcheck(bufins(b,"-- ") == 3  && errno == 0 );
    tstcheck(strcmp("-- Hell  OOo World!",buf(b, 0)) == 0);
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
#endif

  tstassert(valisnil(b=buffree(b)));

}