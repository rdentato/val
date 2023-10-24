

int valisinteger(val_t x) 
int valissigned(val_t x)  
int valisbool(val_t x)    
int valisdouble(val_t x)  
int valispointer(val_t x) 
int valisstring(val_t x)  
int valisvec(val_t x)     
int valeq(val_t x,val_t y)      
int valisnil(val_t x)     
int valiszero(val_t x)    

// Convert a basic value to a val_t value.
val_t val( x);

// Retrieve a value from a val_t variable
   void *valtopointer(val_t v) { return (void *)((uintptr_t)((v.v) & VAL_PAYLOAD));}
 double  valtodouble(val_t v)  { double d; memcpy(&d,&v,8); return d;}
  float  valtofloat(val_t v)   { return (float)valtodouble(v);}

  _Bool  valtobool(val_t v)    { return (_Bool)((v.v)&1);}

   long  valtointeger(val_t v)

#define valtostring(v) ((char *)valtopointer(v))
#define valtovec(v)    ((vec_t)valtopointer(v))

// Some constant
#define valfalse      ((val_t){0x7FFD000000000000})
#define valtrue       ((val_t){0x7FFD000000000001})
#define valnil        ((val_t){0x7FFDFFFFFFFFFFE0})
#define valundefined  ((val_t){0x7FFDFFFFFFFFFFD0})
#define valdeleted    ((val_t){0x7FFDFFFFFFFFFFC0})
#define valempty      ((val_t){0x7FFDFFFFFFFFFFB0})
#define valmarked     ((val_t){0x7FFDFFFFFFFFFFE1})
#define valnilpointer ((val_t){0xFFFF000000000000})
#define valnilstr     val_nilstr()

int valtype(val_t v) {
  if (valisdouble(v))  return VALDOUBLE;
  if (valisinteger(v)) return VALINTEGER;
  if (valisbool(v))    return VALBOOL;
  if (valisnil(v))     return VALNIL;
  if (valispointer(v)) return VALPOINTER;
  if (valisstring(v))  return VALSTRING;
  if (valisvec(v))     return VALVEC;
  return 0;
}
