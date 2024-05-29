#include "tst.h"
#include "val.h"

tstsuite("Comparison Function `valcmp`") {
    int cmp;
    val_t a;
    val_t b;
    tstcase("Two integers") {
      a = val(3); b = val(3);
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(32); b = val(5);
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(3); b = val(5);
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
    }

    tstcase("Two doubles") {
      a = val(3.14); b = val(3.14);
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(32.6); b = val(5.9);
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(3.1); b = val(5.7);
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
    }

    tstcase("An integer and a double") {
      a = val(3); b = val(3.0);
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(3); b = val(2.4);
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(3); b = val(5.7);
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
    }

    tstcase("A double and an integer") {
      a = val(3.0); b = val(3);
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(3.14); b = val(2);
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val(3.14); b = val(5);
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
    }

    tstcase("Two strings") {
      a = val("abc"); b = val("abc");
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val("bcd"); b = val("abc");
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
      a = val("Abc"); b = val("abc");
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);
    }

    tstcase("Two buffers") {
      a = bufnew();
      b = bufnew();
      bufputs(a,"abc"); bufputs(b,"abc");
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (%d) (a= '%s' (%s) b='%s' (%s))",cmp,strcmp(buf(a),buf(b)),buf(a,0),buf(a),buf(b,0),buf(b));
      bufpos(a,0); bufpos(b,0); bufputs(a,"bcd"); bufputs(b,"abc");
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= '%s' (%s) b='%s' (%s))",cmp,buf(a,0),buf(a),buf(b,0),buf(b));
      bufpos(a,0); bufpos(b,0); bufputs(a,"Xbc"); bufputs(b,"abc");
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);

      a = buffree(a);
      b = buffree(b);
    }

    tstcase("A buffer and a string") {
      a = bufnew();
      b = val("abc");
      bufputs(a,"abc");
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (%d) (a= '%s' (%s) b='%s' (%s))",cmp,strcmp(buf(a),buf(b)),buf(a,0),buf(a),buf(b,0),buf(b));
      b = val("Abc");
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= '%s' (%s) b='%s' (%s))",cmp,buf(a,0),buf(a),buf(b,0),buf(b));
      b = val("bbc");
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);

      a = buffree(a);
    }

    tstcase("A string and a buffer") {
      a = val("abc");
      b = bufnew();
      bufputs(b,"abc");
      tstcheck((cmp = valcmp(a,b)) == 0, "cmp: %d (%d) (a= '%s' (%s) b='%s' (%s))",cmp,strcmp(buf(a),buf(b)),buf(a,0),buf(a),buf(b,0),buf(b));
      a = val("bbc");
      tstcheck((cmp = valcmp(a,b)) > 0, "cmp: %d (a= '%s' (%s) b='%s' (%s))",cmp,buf(a,0),buf(a),buf(b,0),buf(b));
      a = val("Abc");
      tstcheck((cmp = valcmp(a,b)) < 0, "cmp: %d (a= %016lX b=%016lX)",cmp,a.v,b.v);

      b = buffree(b);
    }

  }


