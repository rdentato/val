
[val API Documentation](#val-api-documentation)
- [val API Documentation](#val-api-documentation)
  - [What is NaN Boxing?](#what-is-nan-boxing)
  - [Core Type](#core-type)
    - [`val_t`](#val_t)
  - [Constants](#constants)
    - [`valnil`](#valnil)
    - [`valfalse`, `valtrue`](#valfalse-valtrue)
    - [`valnullptr`](#valnullptr)
  - [Value Creation](#value-creation)
    - [`val_t val(x)`](#val_t-valx)
    - [`val_t valconst(int32_t x)`](#val_t-valconstint32_t-x)
  - [Type Checking](#type-checking)
    - [`int valisdouble(val_t x)`](#int-valisdoubleval_t-x)
    - [`int valisint(val_t x)`](#int-valisintval_t-x)
    - [`int valisbool(val_t x)`](#int-valisboolval_t-x)
    - [`int valisnil(val_t x)`](#int-valisnilval_t-x)
    - [`int valisconst(val_t x [, int32_t c])`](#int-valisconstval_t-x--int32_t-c)
    - [`int valisptr(val_t p [, int64_t type])`](#int-valisptrval_t-p--int64_t-type)
  - [Value Extraction](#value-extraction)
    - [`double valtodouble(val_t v)`](#double-valtodoubleval_t-v)
    - [`int64_t valtoint(val_t v)`](#int64_t-valtointval_t-v)
    - [`uint64_t valtounsignedint(val_t v)`](#uint64_t-valtounsignedintval_t-v)
    - [`_Bool valtobool(val_t v)`](#_bool-valtoboolval_t-v)
    - [`void *valtoptr(val_t v)`](#void-valtoptrval_t-v)
  - [Pointer Operations](#pointer-operations)
    - [`uint64_t valptrtype(val_t v)`](#uint64_t-valptrtypeval_t-v)
    - [`val_t valtagptr(val_t p, int tag)`, `int valtagptr(val_t v)`](#val_t-valtagptrval_t-p-int-tag-int-valtagptrval_t-v)
    - [Custom pointers](#custom-pointers)
  - [Comparison and Hashing](#comparison-and-hashing)
    - [`int valeq(val_t x, val_t y)`](#int-valeqval_t-x-val_t-y)
    - [`int valcmp(val_t a, val_t b)`](#int-valcmpval_t-a-val_t-b)
    - [`uint32_t valhash(val_t a)`](#uint32_t-valhashval_t-a)
  - [Example Usage](#example-usage)

# val API Documentation

A C library for NaN boxing and pointer tagging that stores different types of values in a single 64-bit structure.

## What is NaN Boxing?

**NaN Boxing** exploits the special "Not-a-Number" (NaN) representations defined by IEEE 754 floating-point numbers. Since a double-precision floating-point number uses 64 bits, certain NaN patterns can be safely repurposed to encode other data types (integers, pointers, booleans, etc.) without ambiguity.

The library encodes these various types inside a `val_t`, making type handling seamless and efficient.

## Core Type 
### `val_t`
The main value type that can hold doubles, booleans, constants, pointers, or nil values using NaN boxing.

```c
val_t v1 = val(42);           // integer
val_t v2 = val(3.14);         // double
val_t v3 = val(true);         // boolean
val_t v4 = val("hello");      // char pointer
```

## Constants

### `valnil`
Represents a nil/void value.

### `valfalse`, `valtrue`
Boolean constants for false and true values.

### `valnullptr`
Represents a NULL pointer value.

## Value Creation

### `val_t val(x)`
**Usage**: Converts a C value to a `val_t`. This is the primary way to create `val_t` values.
```c
val_t v1 = val(42);           // integer
val_t v2 = val(3.14);         // double
val_t v3 = val(true);         // boolean
val_t v4 = val("hello");      // char pointer
```

### `val_t valconst(int32_t x)`
**Usage**: Creates a constant from a 32-bit value which is guaranteed to be different from any other value.
```c
val_t my_const = valconst(0x12345678);
```

## Type Checking

### `int valisdouble(val_t x)`
**Usage**: Returns non-zero if the value is a double (not NaN-boxed).
```c
if (valisdouble(v)) {
    printf("It's a number\n");
}
```

### `int valisint(val_t x)`
**Usage**: Returns non-zero if the value is an integer (no fractional part).
```c
if (valisint(v)) {
    printf("It's an integer\n");
}
```

### `int valisbool(val_t x)`
**Usage**: Returns non-zero if the value is a boolean.
```c
if (valisbool(v)) {
    printf("It's a boolean\n");
}
```

### `int valisnil(val_t x)`
**Usage**: Returns non-zero if the value is nil.
```c
if (valisnil(v)) {
    printf("It's nil\n");
}
```

### `int valisconst(val_t x [, int32_t c])`
**Usage**: Returns non-zero if the value is a user-defined constant. If the second argument
is specified, checks that the user constant `x` has the value `c`.

```c
if (valisconst(v)) {
    printf("It's a user constant\n");
}

#define ITEM_LOST 0x123
if (valisconst(v,ITEM_LOST)) {
    printf("It's a lost item\n");
}

```

### `int valisptr(val_t p [, int64_t type])`
**Usage**: Checks if value is a pointer, optionally of a specific type.
```c
if (valisptr(v)) {           // any pointer
    printf("It's a pointer\n");
}
if (valisptr(v, VAL_PTR_CHAR)) {  // specific pointer type
    printf("It's a char pointer\n");
}
```
Pointer `type` can be one of the following:

`VAL_PTR_VOID   `
`VAL_PTR_CHAR   `
`VAL_PTR_CHARPTR`
`VAL_PTR_5`
`VAL_PTR_4`
`VAL_PTR_3`
`VAL_PTR_2`
`VAL_PTR_1`

as a shortened form, the following functions are also available:

| short form          | long form                     |
|---------------------|-------------------------------|
|`valisvoidptr(x)`    | `valisptr(x,VAL_PTR_VOID)`    |
|`valischarptr(x)`    | `valisptr(x,VAL_PTR_CHAR)`    |
|`valischarptrptr(x)` | `valisptr(x,VAL_PTR_CHARPTR)` |
|`valisnullptr(x)`    | `valtoptr(x) == NULL`       |

**Usage**: Check for specific pointer types.
```c
if (valischarptr(v)) {
    char *str = valtoptr(v);
    printf("String: %s\n", str);
}
```

## Value Extraction

### `double valtodouble(val_t v)`
**Usage**: Extracts double value. Returns 0.0 if not a double.
```c
double d = valtodouble(v);
```

### `int64_t valtoint(val_t v)`
**Usage**: Converts value to int64_t. Also converts from Booleans and user defined constants. Note that only 52-bits integers can be represented.
```c
int64_t i = valtoint(v);
```

### `uint64_t valtounsignedint(val_t v)`
**Usage**: Converts value to uint64_t.
```c
uint64_t ui = valtounsignedint(v);
```

### `_Bool valtobool(val_t v)`
**Usage**: Converts value to boolean (C type _Bool). When converted to/from integers, zero is false, non-zero is true.
```c
_Bool b = valtobool(v);
if (valtobool(v)) {
    printf("Value is truthy\n");
}
```

### `void *valtoptr(val_t v)`
**Usage**: Extracts pointer from value. Returns NULL if not a pointer.
```c
void *ptr = valtoptr(v);
if (ptr != NULL) {
    // use pointer
}
```

## Pointer Operations

### `uint64_t valptrtype(val_t v)`
**Usage**: Returns the pointer type, or 0 if not a pointer.
```c
uint64_t type = valptrtype(v);
if (type == VAL_PTR_CHAR) {
    printf("It's a char pointer\n");
}
```
Pointer `type` will be one of the following:

`VAL_PTR_VOID   `
`VAL_PTR_CHAR   `
`VAL_PTR_CHARPTR`
`VAL_PTR_5`
`VAL_PTR_4`
`VAL_PTR_3`
`VAL_PTR_2`
`VAL_PTR_1`


### `val_t valtagptr(val_t p, int tag)`, `int valtagptr(val_t v)`
**Usage**: Return a tagged pointer (if a tag is specified) or get the current tag.
```c
// Set tag
val_t tagged;
int tag; 
char *s = "Hello";

tagged = valtagptr(&s, 2);  // tag a char ** with value 2
tag = valtagptr(tagged);    // Will return 2
```
Tags are 3-bits number (values 0-7) requiring a pointer alignment of 8 bytes. The tag 0 is the default one for pointers. Tags can be used to mark a pointer as "visited" or to signal it is not to be disposed yet, etc.

### Custom pointers
The pointers `VAL_PTR_1` through `VAL_PTR_5` can be use to add custom pointers to the set of values that can be stored in a `val_t` variable.

**Usage**: 
```c
// Define custom pointer types before including val.h
#define valptr_1_t FILE *
#define PTRTAG_FILE VAL_PTR_1

#include "val.h"

// Or after as long as it is a pointer to a structure (the most common case)
struct  valptr_2_s { int x; int y; };
#define point_t valptr_2_t 
#define PTRTAG_POINT VAL_PTR_2
```

## Comparison and Hashing

The function `valcmp()` and `valhash()` are provided for convenience, real application might have their own criteria on how values should be sorted and hashed.

### `int valeq(val_t x, val_t y)`
**Usage**: Checks for exact identity (bit-wise equality) of two val_t values.
```c
val_t v1 = val(10);
val_t v2 = val(20);

int check_1 = valeq(v1, v2); // returns false
int check_2 = valeq(v1, val(10)); // returns true
int check_3 = valeq(v1, 10); // returns true
```
Note that for `check_3` the value `10` is automatically converted to its corresponding `val_t` value.

### `int valcmp(val_t a, val_t b)`
**Usage**: Compares two values. Returns -1, 0, or 1. Strings are compared lexicographically, numbers numerically.
```c
int result = valcmp(v1, v2);
if (result < 0) {
    printf("v1 < v2\n");
} else if (result > 0) {
    printf("v1 > v2\n");
} else {
    printf("v1 == v2\n");
}
```

### `uint32_t valhash(val_t a)`
**Usage**: Computes a hash value for the val_t. Uses FNV1a for strings, MurmurHash3 for others.
```c
uint32_t hash = valhash(v);
// use hash for hash tables, etc.
```

## Example Usage

```c
#include "val.h"

int main() {
    // Create values
    val_t num = val(42);
    val_t str = val("hello");
    val_t flag = val(true);
    
    // Type checking
    if (valisdouble(num)) {
        printf("Number: %g\n", valtodouble(num));
    }
    
    if (valischarptr(str)) {
        printf("String: %s\n", (char*)valtoptr(str));
    }
    
    if (valisbool(flag)) {
        printf("Boolean: %s\n", valtobool(flag) ? "true" : "false");
    }
    
    // Comparison
    if (valeq(num, 42)) {
        printf("Numbers are equal\n");
    }
    
    return 0;
}
```

