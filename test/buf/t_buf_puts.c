#include "tst.h"
#include "val.h"


tstsuite("Buffer puts() tests") {

  val_t b = valnil;

  tstassert(!valisnil((b = bufnew())));

  tstcase("Write a string into a buffer") {
    tstcheck(bufpos(b) == 0);
    tstcheck(bufputs(b,"Hello") == 5);

    tstcheck(buflen(b) == 5,"Buf size: %d",buflen(b));

    tstcheck(strcmp("Hello",buf(b, 0)) == 0);
  }

  tstcase("Continue writing into a buffer") {
    tstcheck(bufputs(b," World!") == 7);
    tstcheck(buflen(b) == 12,"Buf size: %d",buflen(b));

    tstcheck(strcmp("Hello World!",buf(b, 0)) == 0);

    tstcheck(bufpos(b,-2) == 10);
    tstcheck(bufpos(b) == 10);

    tstcheck(bufsize(b) > 10);

    tstcheck(bufputs(b,"xs?") == 3);
    tstcheck(strcmp("Hello Worlxs?",buf(b, 0)) == 0);
  }

  tstcase("Cut a string") {
    tstcheck(strcmp("Hello Worlxs?",buf(b, 0)) == 0);
    tstcheck(buflen(b,5) == 5);
    tstcheck(strcmp("Hello",buf(b, 0)) == 0);
  }

  tstcase("write in the middle") {
    tstcheck(strcmp("Hello",buf(b, 0)) == 0);
    bufpos(b,2);
    tstcheck(bufputs(b,"gg") == 2);
    tstcheck(strcmp("Heggo",buf(b, 0)) == 0);
    tstcheck(bufpos(b)==4);
  }

  tstcase("Append a NULL pointer") {
    tstcheck(bufputs(b,NULL) == 0);
    tstcheck(errno == 0);
  }

  tstcase("Append a empty string") {
    int l = buflen(b);
    tstcheck(bufputs(b,"") == 0);
    tstcheck(l == buflen(b));
  }

  tstcase("Embedded '\\0'") {
    buflen(b,0);
    tstcheck(bufputs(b,"ab\0cd",6) == 6);
    tstcheck(buflen(b) == 6);
    tstcheck(strcmp("ab",buf(b, 0)) == 0);
    tstcheck(buf(b, 0)[2] == '\0');
    tstcheck(buf(b, 0)[3] == 'c');
    tstcheck(strcmp("cd",buf(b, 0)+3) == 0);
  }

  tstassert(valisnil(b=buffree(b)));

  tstcase("Buffer is null") {
    tstcheck(bufputs(b,"Hello") == 0);
    tstcheck(errno == EINVAL);
  }

  if (valisbuf(b)) b=buffree(b);
  tstcheck(valisnil(b));
}