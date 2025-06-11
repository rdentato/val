#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"

typedef struct valptr_buf_s { char *buf; int len; int size; } *buf_t;

buf_t bufnew(int size) 
{
  buf_t b = malloc(sizeof(*b));
  if (b) {
    b->buf = malloc(size);
    if (b->buf == NULL) {
      free(b); b = NULL;
    }
    else {
      b->size = size;
      b->len  = 0;
    }
  }
  return b;
}

void buffree(buf_t b)
{
  if (b) {
    if (b->buf != NULL) free(b->buf);
    free(b);
  }
}

tstsuite("Buffers") {
    tstcase("Numeric Value Extraction") {
      buf_t b = bufnew(100);
      tstcheck(b != NULL);
      val_t bufVal = val(b);

      buf_t b2 = bufnew(40);
      tstcheck(b2 != NULL);
      val_t buf2Val = val(b2);


      tstcheck(valisbufptr(bufVal));
      tstcheck(valisbufptr(buf2Val));

      tstcheck(valeq(buf2Val,b2));

      tstcheck(valtagptr(buf2Val) == 0);
      buf2Val = valtagptr(buf2Val,2);
      tstcheck(valtagptr(buf2Val) == 2);

      tstcheck(!valeq(buf2Val,b2));
      tstcheck(valtoptr(buf2Val) == valtoptr(b2));

      tstcheck(valischarptr("Hello"),"val: %016" PRIX64 "",val("Hello").v);

      strcpy(b->buf,"Hello"); b->len=5;
      strcpy(b2->buf,"World"); b2->len=5;
      
      tstcheck(strcmp(b->buf,"Hello") == 0);
      tstcheck(strlen(b->buf) == b->len);

      tstcheck(strcmp(b2->buf,"World") == 0);
      tstcheck(strlen(b2->buf) == b->len);

      tstcheck(valcmp(bufVal,buf2Val) != 0);
      tstcheck(valcmp(bufVal,"Hello") == 0); // Mix buffer and strings
      tstcheck(valcmp("Hello", bufVal) == 0); // Mix buffer and strings

      

      buffree(b);
      buffree(b2);
    }
}
