#include "tst.h"
#include "val.h"


tstsuite("Buffer read/write tests") {

  val_t b = valnil;
  uint32_t n;

  tstassert(!valisnil((b = bufnew())));
  tstassert(bufputs(b,"Hello") == 5);

  tstcase("writing on stdout the entire buffer") {
    bufpos(b,0); // start from 0;
    tstcheck(bufpos(b) == 0);
    tstcheck((n=bufsave(b)) == buflen(b),"len: %d. Wrote %d bytes",buflen(b),n);
    tstcheck(bufpos(b) == buflen(b));
  }

  tstcase("writing 2 chars starting from the second one") {
    bufpos(b,1); // start from 1;
    tstcheck(bufpos(b) == 1);
    tstcheck((n=bufsave(b,2,stdout)) == 2,"requested: 2. Wrote %d bytes",n);
    tstcheck(bufpos(b) == 3);
  }

  FILE *f;

  tstcase("Writing on a file") {
    f = fopen("buf_rw.log","wb");
    assert(f);
    bufpos(b,0);
    tstcheck(bufputs(b,"World") == 5);
    
    bufpos(b,0);
    tstcheck(bufsave(b,f) == 5);
    fclose(f);
  }

  tstcase("Reading from a file") {
    f = fopen("buf_rw.log","rb");
    assert(f);
    bufpos(b,0);

    tstcheck(bufputs(b,"Hello ") == 6);
    tstcheck(bufload(b,f) == 5);

    tstcheck(strcmp(buf(b, 0),"Hello World") == 0);
    fclose(f);
  }

  tstcase("reading lines") {
    f = fopen("buf_rw.log","wb");
    assert(f);

    fprintf(f,"ln1\nln2\r\nln3");
    fclose(f);
    
    f = fopen("buf_rw.log","rb");
    assert(f);

    buflen(b,0);
    tstcheck(buflen(b) == 0);
    tstcheck(bufpos(b) == 0);

    bufputs(b,"[");
    tstcheck((n=bufloadln(b,f)) == 3, "Read: %d",n);
    bufputs(b,"]");
    tstcheck(strcmp(buf(b, 0),"[ln1]") == 0, "Read string: \"%s\"",buf(b, 0));

    bufputs(b,"[");
    tstcheck((n=bufloadln(b,f)) == 3, "Read: %d",n);
    bufputs(b,"]");
    tstcheck(strcmp(buf(b, 0),"[ln1][ln2]") == 0, "Read string: \"%s\"",buf(b, 0));

    bufputs(b,"[");
    tstcheck((n=bufloadln(b,f)) == 3, "Read: %d",n);
    bufputs(b,"]");
    tstcheck(strcmp(buf(b, 0),"[ln1][ln2][ln3]") == 0, "Read string: \"%s\"",buf(b, 0));

    fclose(f);

    f = fopen("buf_rw.log","rb");
    assert(f);

    bufpos(b,6);
    tstcheck((n=bufloadln(b,f)) == 3, "Read: %d",n);
    tstcheck(strcmp(buf(b, 0),"[ln1][ln1][ln3]") == 0, "Read string: \"%s\"",buf(b, 0));

    fclose(f);
 }

  tstassert(valisnil(b=buffree(b)));

}