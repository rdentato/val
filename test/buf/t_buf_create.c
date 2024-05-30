#include "tst.h"
#include "val.h"

tstsuite("Buffer creation tests",nolarge) {

  val_t b = valnil;

  tstcase("Create & Destroy buffers") {
    tstcheck(!valisnil((b = bufnew())));
    tstcheck(valisnil((b = buffree(b))));
    tstcheck(valisnil((b = buffree(b))));
  }

  if (valisbuf(b)) b=buffree(b);
  tstcheck(valisnil(b));
}