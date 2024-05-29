#include "tst.h"
#include "val.h"

tstsuite("Printing values") {
  val_t a;
  val_t b;
  FILE *f = stdout;
  tstcase("Printing signed integer") {
    fprintf(f,"signed integer " ); 
    valprintf(val(3),f);
    fprintf(f,"\n"); 
  }
  tstcase("Printing double") {
    fprintf(f,"double " ); 
    valprintf(val(3.13),f);
    fprintf(f,"\n"); 
    valfmt(DOUBLE,"%E");
    fprintf(f,"double " ); 
    valprintf(val(3.13),f);
    fprintf(f,"\n"); 
  }
}


