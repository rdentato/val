
#include "tst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "val.h"

// Make up a buffer type
typedef struct valptr_buf_s { char *buf; char buf_str[120]; } *buf_t;

buf_t bufnew()
{ 
  buf_t b = calloc(1,sizeof(*b));
  if (b)  b->buf = b->buf_str;
  return b;
}

void buffree(buf_t b)
{
  free(b);
}

tstsuite("Val Library convert to string") {

  tstcase("Create with a buffer") {
      val_t v = valnil;

      tstcheck(strcmp(valtostring(v),"nil") == 0);

      v = val((_Bool)300);
      tstcheck(strcmp(valtostring(v),"true") == 0);

      v = val((_Bool)0);
      tstcheck(strcmp(valtostring(v),"false") == 0);

      v = val("pippo");
      tstcheck(strcmp(valtostring(v),"pippo") == 0);

      v = val((char *)NULL);
      tstcheck(strcmp(valtostring(v),"") == 0);

      v = val(432);
      tstnote("432 -> %s",valtostring(v));
      tstcheck(strcmp(valtostring(v),"432.000000") == 0);
      tstcheck(strcmp(valtostring(v,NULL,"%.2f"),"432.00") == 0);
      tstcheck(strcmp(valtostring(v,"%.2f"),"432.00") == 0);
      
      tstcheck(strcmp(valtostring(v,"%.0f"),"432") == 0);

      v = val(3.26);
      tstnote("3.26 -> %s",valtostring(v));
      tstcheck(strcmp(valtostring(v),"3.260000") == 0);
      tstcheck(strcmp(valtostring(v,NULL,"%.2f"),"3.26") == 0);
      tstcheck(strcmp(valtostring(v,"%.2f"),"3.26") == 0);

      v = val(0);
      tstcheck(strlen(valtostring(v,"%20f")) == 20);
      tstcheck(strlen(valtostring(v,"%50f")) == 30);

      char dbl_buf[60]; dbl_buf[0] = '\0';
      tstcheck(strlen(valtostring(v,dbl_buf,"%50f")) == 50);

      v = val(valconst(42));
      tstcheck(strcmp(valtostring(v),"0000002A") == 0);

      v = val(vallabel("Hello"));
      tstcheck(strcmp(valtostring(v),"Hello") == 0);

      buf_t b = bufnew();
      tstassert(b != NULL);
      strcpy(b->buf,"Good Morning");
      v = val(b);
      tstcheck(valisbufptr(b));

      tstcheck(strcmp(valtostring(v),"Good Morning") == 0);

      buffree(b);
    }
}
