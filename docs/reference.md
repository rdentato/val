# val C Library Reference Manual

A comprehensive C library for NaN boxing and pointer tagging that efficiently stores different types of values in a single 64-bit structure.

## Table of Contents

- [val C Library Reference Manual](#val-c-library-reference-manual)
  - [Table of Contents](#table-of-contents)
  - [Introduction](#introduction)
    - [Key Features](#key-features)
  - [Core Concepts](#core-concepts)
    - [NaN Boxing](#nan-boxing)
    - [Pointer Tagging](#pointer-tagging)
  - [Data Types](#data-types)
    - [Primary Type](#primary-type)
    - [Supported Value Types](#supported-value-types)
  - [Boxing Operations](#boxing-operations)
  - [Numbers](#numbers)
    - [Type Checking](#type-checking)
    - [Number Extraction](#number-extraction)
    - [Example](#example)
  - [Pointers](#pointers)
    - [Type Checking](#type-checking-1)
    - [Example](#example-1)
    - [Pointer Extraction](#pointer-extraction)
    - [Pointer Tagging](#pointer-tagging-1)
    - [Buffer Pointers](#buffer-pointers)
    - [Custom Pointer Types](#custom-pointer-types)
  - [Booleans](#booleans)
    - [Example](#example-2)
  - [Nil Values](#nil-values)
    - [Example](#example-3)
  - [Constants](#constants)
    - [Example](#example-4)
  - [Comparison and Hashing](#comparison-and-hashing)
    - [Equality and Comparison](#equality-and-comparison)
    - [Hashing](#hashing)
  - [String Representation](#string-representation)
    - [String Conversion Type](#string-conversion-type)
    - [Default Formatters](#default-formatters)
    - [Examples](#examples)
  - [Pairs](#pairs)
    - [Example](#example-5)
  - [Stored Strings](#stored-strings)
    - [Example](#example-6)
  - [Data Structures](#data-structures)
    - [Vectors](#vectors)
    - [Stacks](#stacks)
    - [Queues](#queues)
    - [Maps](#maps)
    - [Sorting and Searching](#sorting-and-searching)
    - [Example](#example-7)
  - [Performance Considerations](#performance-considerations)
    - [Optimization Features](#optimization-features)
  - [Examples](#examples-1)
    - [Complete Example: Mixed Data Container](#complete-example-mixed-data-container)

---

## Introduction

The **val library** is a high-performance C library that implements NaN boxing and pointer tagging techniques to store multiple data types efficiently within a single 64-bit structure. This approach eliminates the overhead of traditional tagged unions while maintaining type safety and providing O(1) operations for boxing, type checking, and unboxing.

### Key Features

- **Unified Type System**: Store doubles, integers, pointers, booleans, and constants in a single `val_t` type
- **High Performance**: All operations are O(1) and branch-free where possible
- **Memory Efficient**: 64-bit storage for all value types
- **Type Safe**: Runtime type checking with dedicated predicates
- **Extensible**: Support for custom pointer types and constants

---

## Core Concepts

### NaN Boxing

**NaN Boxing** exploits the IEEE 754 floating-point standard's "Not-a-Number" (NaN) representations. In a 64-bit double-precision floating-point number, certain NaN bit patterns can be safely repurposed to encode other data types without ambiguity.

The technique works by:
1. Using normal IEEE 754 representation for actual floating-point numbers
2. Utilizing specific NaN bit patterns to encode other types
3. Maintaining type information within the 64-bit value itself

### Pointer Tagging

The library supports 3-bit tagging for most pointer types, allowing you to store additional metadata (0-7) alongside pointer values. This is useful for marking pointers during operations (e.g., `visited`, `marked_for_deletion`, etc.).

---

## Data Types

### Primary Type

```c
typedef struct val_s val_t;
```

The fundamental data type that can hold any of the supported value types in NaN-boxed format.

### Supported Value Types

- **64-bit IEEE-754 doubles** (including NaN and -NaN)
- **52-bit signed integers** 
- **Pointers** (`void*`, `char*`, `FILE*`, custom struct pointers)
- **Booleans** (`valtrue`, `valfalse`)
- **Nil** (`valnil`) and the nil-family constants (`valnone`, `valundefined`, `valtombstone`, `valdeleted`)
- **Numeric constants** (32-bit integers)
- **Symbolic constants** (up to 8-character strings)
- **Pairs** (a 16-bit and a 32-bit value in one word)
- **Stored strings** (library-owned strings, see [Stored Strings](#stored-strings))

---

## Boxing Operations

```c
val_t val(x);
```

**Purpose**: Generic conversion of any supported C type `x` into a `val_t` boxed value.

**Parameters**:
- `x`: Value of any supported C type (double, int, pointer, bool, etc.)

**Returns**: NaN-boxed `val_t` representation of the input value

**Example**:
```c
#include "val.h"

int main() {
    val_t d = val(3.14159);          // Box a double
    val_t i = val(42);               // Box an integer  
    val_t s = val("Hello");          // Box a string pointer
    val_t b = val(true);             // Box a boolean
    val_t f = val(stdout);           // Box a FILE pointer
    
    return 0;
}
```

---

## Numbers

### Type Checking

```c
int valisnumber(val_t v);
int valisint(val_t v);
```

**`valisnumber(val_t v)`**
- **Purpose**: Check if value represents any numeric type
- **Returns**: Non-zero if `v` is a number (including NaN), zero otherwise
- **Note**: Both `NAN` and `-NAN` from `math.h` are considered numbers

**`valisint(val_t v)`**
- **Purpose**: Check if value represents an integer (no fractional part)
- **Returns**: Non-zero if `v` is an integer, zero otherwise

### Number Extraction

```c
double   valtodouble(val_t v);
int64_t  valtoint(val_t v);
uint64_t valtounsignedint(val_t v);
```

**`valtodouble(val_t v)`**
- **Purpose**: Extract double value from boxed number
- **Returns**: Double value, or 0.0 if not a number
- **Error Handling**: Sets `errno` to `EINVAL` if `v` is not a number

**`valtoint(val_t v)`**
- **Purpose**: Extract signed integer value
- **Returns**: 64-bit signed integer
- **Note**: Also converts numeric constants to their integer representation
- **Range**: Exact representation for integers up to 52 bits

**`valtounsignedint(val_t v)`**
- **Purpose**: Extract unsigned integer value
- **Returns**: 64-bit unsigned integer
- **Behavior**: Identical to `valtoint()` but returns unsigned type

### Example

```c
#include "val.h"
#include <stdio.h>

int main() {
    val_t v1 = val(3.14);
    val_t v2 = val(100);
    val_t v3 = val(true);

    if (valisnumber(v1)) {
        printf("v1 is a number: %f\n", valtodouble(v1));
    }
    
    if (valisint(v2)) {
        printf("v2 is an integer: %" PRId64 "\n", valtoint(v2));
    }
    
    printf("v3 is %s a number\n", valisnumber(v3) ? "" : "not");
    
    return 0;
}
```

---

## Pointers

### Type Checking

```c
int valisvoidptr(val_t v);
int valischarptr(val_t v);
int valisfileptr(val_t v);
int valisbufptr(val_t v);
int valisnullptr(val_t v);
int valisptr(val_t v [, uint64_t type]);
```

**`valisvoidptr(val_t v)`**
- **Purpose**: Check if value is a boxed `void*`
- **Returns**: Non-zero if `v` contains a `void*`, zero otherwise

**`valischarptr(val_t v)`**
- **Purpose**: Check if value is a boxed `char*`
- **Returns**: Non-zero if `v` contains a `char*`, zero otherwise

**`valisfileptr(val_t v)`**
- **Purpose**: Check if value is a boxed `FILE*`
- **Returns**: Non-zero if `v` contains a `FILE*`, zero otherwise

**`valisbufptr(val_t v)`**
- **Purpose**: Check if value is a boxed buffer pointer
- **Returns**: Non-zero if `v` contains a `valptr_buf_s*`, zero otherwise

**`valisnullptr(val_t v)`**
- **Purpose**: Check if value is a NULL pointer of any type
- **Returns**: Non-zero if `v` contains NULL, zero otherwise

**`valisptr(val_t v [, uint64_t type])`**
- **Purpose**: Check if value is a pointer, optionally of specific type
- **Parameters**: 
  - `v`: Value to check
  - `type`: Optional type constant (if omitted, checks for any pointer)
- **Returns**: Non-zero if `v` is a pointer of the specified type

The valid pointer types are:

- `VALPTR_VOID` - void pointer
- `VALPTR_CHAR` - char pointer  
- `VALPTR_FILE` - FILE pointer
- `VALPTR_BUF` - buffer pointer
- `VALPTR_0` through `VALPTR_6` - custom pointer types

### Example

```c
#include "val.h"
#include <stdio.h>

int main() {
    char *str = "Hello, World!";
    val_t boxed_str = val(str);
    FILE *fp = stdout;
    val_t boxed_file = val(fp);
    
    // Type checking
    if (valischarptr(boxed_str)) {
        printf("String: %s\n", (char*)valtoptr(boxed_str));
    }
    
    if (valisfileptr(boxed_file)) {
        printf("FILE pointer: %p\n", valtoptr(boxed_file));
    }
    
    // Pointer tagging (not available for char* and void*)
    if (valisfileptr(boxed_file)) {
        val_t tagged_ptr = valtagptr(boxed_file, 3);  // Set tag to 3
        printf("Previous tag: %d\n", valtagptr(boxed_file));
        printf("Current tag: %d\n", valtagptr(tagged_ptr));
    }
    
    return 0;
}
```

### Pointer Extraction

```c
void *valtoptr(val_t v);
```

**Purpose**: Extract raw pointer value from boxed pointer
**Returns**: Untagged pointer value. If `v` is not a pointer, returns `NULL` and sets `errno` to `EINVAL`
**Note**: Returns the pointer without any tag information

### Pointer Tagging

```c
int   valtagptr(val_t v);
val_t valtagptr(val_t v, int tag);
```

**Purpose**: Returns the tag of the boxed pointer `v` or, if the second paramater is provided, returns the boxed pointer tagged with the specified tag.
**Parameters**:
- `v`: Boxed pointer value
- `tag`: Optional new tag value (0-7)

**Behavior**:
- **Get mode** (single parameter): Returns pointer tag value
- **Set mode** (two parameters): Returns the boxed pointer with the new tag.

**Note**: Tags are not supported for `void*` and `char*` pointers as those pointers are supposed to be free to point to any unaligned address.

**Note**: `valtagptr()` does **NOT** change the tag of a boxed pointer, it returns a **new** boxed pointer with the same address and the new tag.

### Buffer Pointers

Growable **text buffer** is a common data structure and the library provides direct support for it. For the purpose of hashing or comparing, they are considered strings exactly as a `char *`. It is important to ensure that the first field is a `char *` containing a zero terminated string.
For example:

```c
// Complete the definition of valptr_buf_s for your buffer structure
typedef struct valptr_buf_s {
    char *text;    // MUST BE THE FIRST FIELD
    int  size;     // Buffer capacity
    int  curlen;   // Current used length
} *buf_t;

buf_t bufnew(int size);         // Create a new buffer with an initial size
void  buffree(buf_t b);         // Free up a buffer
int   bufcpy(buf_t b, char *s); // copy a string in the buffer

int main()
{
  val_t vb = val(bufnew(100));

  if (valisbufferptr(vb)) {
     bufcpy(valtoptr(vb),"Hello!");
     printf("Buffer: %p\n", valtoptr(vb));
     printf("Text: %s\n", valtoptr(vb)->text);
     printf("Is greeting?: %d\n", valcmp(vb,"Hello!")); // Direct comparison as string
  }
}

```
 **Note**: For a structure to be correctly recognized and handled as a buffer by the `val` library, its `char *` field **MUST BE THE FIRST FIELD** in the structure definition. This is critical for the library's internal mechanisms to correctly extract the text pointer of the buffer.

### Custom Pointer Types

The types that can be stored in a `val_t` variable, can be extended through these pointer types:

```c
typedef struct valptr_6_s *valptr_6_t;
typedef struct valptr_5_s *valptr_5_t;
typedef struct valptr_4_s *valptr_4_t;
typedef struct valptr_3_s *valptr_3_t;
typedef struct valptr_2_s *valptr_2_t;
typedef struct valptr_1_s *valptr_1_t;
typedef struct valptr_0_s *valptr_0_t;
```

The most common case is when you want to add a pointer to a structure. 
You put the structure definition *before* including `val.h`:

```c
// Define the valptr_2_s structure:
typedef struct valptr_2_s {
    int x;
    int y; 
} *point_t; // Now point_t values can be stored in a `val_t` variable.

// For convenience let's define a macro to distinguish point_t values from the others.
#define ispointptr(p) valisptr(p, VALPTR_2)

// Only now we include val.h
#include "val.h"

// Let's pretend we have these two functions:
point_t new_point(double x, double y);
void free_point(point_t);

int main()
{
  val_t vpoint;

  vpoint = val(new_point(-1.0,1.0));
  if (ispoint(vpoint)) printf("It's a point!\n");
  free_point(valtoptr(vpoint));
}
```

You can also add pointers to different types. For example, if you want to add function pointers:

```c
// Redefine valptr_X_t types

typedef int (*mono_f)(int);
#define valptr_0_t mono_f
#define ismono_f(v) valisptr(v,VALPTR_0)

typedef int (*couple_f)(int,int);
#define valptr_1_t couple_f
#define iscouple_f(v) valisptr(v,VALPTR_1)

#define valptr_2_t int *
#define isintptr(v) valisptr(v,VALPTR_2)

// REMEMBER: You need to redefine the types *BEFORE* including `val.h`
#include "val.h"

int f0(int a)        {return a;}
int f1(int a, int b) {return a;}
int f2(int a, int b) {return b;}

int main()
{
   val_t vlist[10];
   int int_array[10];
   int int_var;

   vlist[0] = val(f0);
   vlist[1] = val(f1);
   vlist[2] = val(f2);
   vlist[3] = val(int_array); // The base address of an array
   vlist[4] = val(&int_var);  // A pointer to int
   vlist[5] = valnil;

   // compare the type of an element with the previous one in the vlist array
   for (int i = 1; !valisnil(vlist[i]); i++) {
      printf( "%d %d ", , i-1, i);
      if (valptrtype(vlist[i]) == valptrtype(vlist[i-1]))
        printf("Same type!\n");
      else 
        printf("Different type!\n");
   }
}
```

**Summary**: To add a new pointer type to be stored in a `val_t` variable, you either complete the definition of the struct `valptr_X_s` for pointer to structures (the most common case), or you redefine the type `valptr_X_t` for general pointers.


---

## Booleans
The val library provides specific val_t constants for boolean true and false values: 

```c
static const val_t valfalse;  // Boolean false value
static const val_t valtrue;   // Boolean true value
```
and a function to check if a val_t holds a boolean:

```c
int valisbool(val_t v);
```

**Purpose**: Check if value represents a boolean
**Returns**: Non-zero if `v` is `valtrue` or `valfalse`, zero otherwise

### Example

```c
#include "val.h"
#include <stdio.h>

int main() {
    val_t b1 = val(true);
    val_t b2 = valfalse;
    val_t b3 = valtrue;
    
    printf("b1 is boolean: %s\n", valisbool(b1) ? "yes" : "no");
    printf("b2 is boolean: %s\n", valisbool(b2) ? "yes" : "no");
    printf("b3 is boolean: %s\n", valisbool(b3) ? "yes" : "no");
    
    if (valtoint(b2)) { /* This will NOT be executed */ }
    if (valtoint(b3)) { /* This will be executed */ }

    return 0;
}
```

---

## Nil Values
The val library includes a special val_t constant to represent a "nil" or "void" state (for example the lack of a result, rather that a '0'):

```c
static const val_t valnil;    // Nil/void value
```

and a function to check for it.

```c
int valisnil(val_t v);
```

**Purpose**: Check if value represents nil
**Returns**: Non-zero if `v` is `valnil`, zero otherwise

### Nil-family constants

Besides `valnil`, the library defines four sibling constants for the classic
"sentinel" values used by data structures:

```c
valnil        // "no value at all"
valnone       // "no result / nothing"
valundefined  // "value not (yet) defined"
valtombstone  // "slot freed / deleted"
valdeleted    // "entry deleted"
```

All five are distinct `val_t` values (different bit patterns), so they can be
used as unambiguous sentinels, map markers, or internal flags. Only `valnil`
satisfies `valisnil()`; the others are regular constants (see
[Constants](#constants)):

```c
valisnil(valnil)        // true
valisnil(valnone)       // false
valisconst(valdeleted)  // true
```

### Example

```c
#include "val.h"
#include <stdio.h>

int main() {
    val_t v_nil = valnil;
    val_t v_null_ptr = val(NULL); // A NULL void pointer, not valnil
    val_t v_false = valfalse;
    val_t v_zero = val(0);

    printf("valnil is nil: %s\n", valisnil(v_nil) ? "true" : "false");         // true
    printf("val(NULL) is nil: %s\n", valisnil(v_null_ptr) ? "true" : "false"); // false
    printf("valfalse is nil: %s\n", valisnil(v_false) ? "true" : "false");     // false
    printf("val(0) is nil: %s\n", valisnil(v_zero) ? "true" : "false");        // false
    return 0;
}
```
---

## Constants

In addition to the predefined constants `valnil`, `valtrue`, and `valfalse`, the `val` library allows you to define custom constant `val_t` values.

These custom constants are guaranteed to be unique and different from any other val_t value (numbers, pointers, booleans, or other constants unless explicitly defined as identical).

There are two types of custom constants:
 - Numeric constants: Defined with a 32-bit integer number. These are primarily for giving unique constant identifiers rather than representing numeric data.
 - Symbolic constants: Defined by a short string (maximum 8 characters). These are useful for creating labels, names, distinct constant values.
  Symbolic constants can only contain digits (`0`-`9`), lowercase letters (`a-z`), and characters from the set: `!#$*+-./:<=>?@[]_`. Upper case letter will be converted into lowercase.
`

You can create a constant with the `valconst()` function:

```c
val_t valconst(x);           // Create numeric or symbolic constant
```
**Purpose**: Creates a numeric/symbolic constant depending on `x`
**Returns**: The `val_t` boxing of the constant `x`


You can check if a value is a constant with:

```c
int valisconst(val_t v [, x]);
int valisnumconst(val_t v[, x]);
int valissymconst(val_t v[, x]);
```

**Purpose**: Check if value represents a constant and, if it ie equal to `x` (if specified)
**Returns**: Non-zero if `v` is constant, zero otherwise


### Example

```c
#include "val.h"
#include <stdio.h>

int main() {
    val_t num_const = valconst(100);
    val_t sym_const = valconst("start");
    val_t start_block = valconst("blk_[[");
    val_t end_block = valconst("]]_blk");
    val_t regular_int = val(100);
    
    printf("num_const is constant: %s\n", valisconst(num_const) ? "yes" : "no");
    printf("sym_const is constant: %s\n", valisconst(sym_const) ? "yes" : "no");
    printf("regular_int is constant: %s\n", valisconst(regular_int) ? "yes" : "no");
    
    printf("num_const is numeric constant: %s\n", valisnumconst(num_const) ? "yes" : "no");
    printf("sym_const is symbolic constant: %s\n", valissymconst(sym_const) ? "yes" : "no");
    printf("num_const is 100: %s\n", valisconst(num_const,valconst(100)) ? "yes" : "no");
    printf("num_const is 100: %s\n", valisconst(num_const,100) ? "yes" : "no");
    
    return 0;
}
```

---

## Comparison and Hashing

### Equality and Comparison

```c
int valeq(val_t a, val_t b);     // Test for identical values
int valcmp(val_t a, val_t b);    // Total ordering comparison
```

**`valeq(val_t a, val_t b)`**
- **Purpose**: Test if two `val_t` values are identical
- **Returns**: Non-zero if equal, zero if different

**`valcmp(val_t a, val_t b)`**
- **Purpose**: Compare two values with total ordering
- **Returns**: 
  - Negative if `a < b`
  - Zero if `a == b`  
  - Positive if `a > b`
- **Note**: Buffers are compared as strings.
- **Note**: Stored strings are compared as strings, by content (so a stored string and a `char*` with the same text compare equal).
- **Note**: Pairs compare lexicographically, first by their left component then by the right one.
- **Note**: Numeric constants are NOT compared as numbers and symbolic constants are NOT compared as strings.

### Hashing

```c
int valhash(val_t v);
```

**Purpose**: Generate 32-bit hash value
**Returns**: Hash code suitable for hash table implementations
**Note**: Buffers are hashed as strings, Symbolic constants are NOT hashed as string.
**Note**: Stored strings are hashed as strings, by content (same hash as a `char*` with the same text).

---
## String Representation
  The function `valtostr()` make it easier to print and examone `val_t` values. 

### String Conversion Type
The result of conversation is returned in a structure of type `valtostr_t`:

```c
typedef struct { char str[32]; } valtostr_t;
```

```c
valtostr_t valtostr(val_t v [, char* format]);
```

**Purpose**: Convert any `val_t` value to its string representation
**Parameters**:
- `v`: Value to convert
- `format`: Optional custom format string (max 30 characters)

**Returns**: `valtostr_t` object containing string representation

### Default Formatters
Each type is converted using a predefined formatter. You can specify your own using the printf format specifiers.

| Type | Default Format |
|------|----------------|
| double | `%f` |
| integer | `%" PRId64 "` |
| pointers | `%p` |
| boolean | "`false`"/"`true`" |
| nil | "`nil`" |
| symbolic constant | string |
| numeric constant | `[%" PRIX32 "]` |
| pair | `<%" PRIX16 ",%" PRIX32 ">` |
| stored string | the string content |

Stored strings print their content (truncated to fit the 32-character buffer);
pairs print as `<L,R>` in hexadecimal. The `valtoint()` conversion on a stored
string parses the content with `strtoll` (base 10), returning `0` for
a non-numeric content.

### Examples

**Temporary Usage**:
```c
val_t value = val(42);
printf("%s\n", valtostr(value).str);  // Direct usage
```

**Stored Usage**:
```c
val_t value = val(42);
valtostr_t str_obj = valtostr(value);   // Keep object in scope
char *str_ptr = str_obj.str;          // Safe to use while str_obj exists
printf("%s\n", str_ptr);
```

**Custom Formatting**:
```c
val_t number = val(7);
printf("%s\n", valtostr(number, "%03d").str);  // Outputs: "007"
```

⚠️ **Warning**: Do not store the `.str` pointer without keeping the `valtostr_t` object in scope!

**Incorrect storage**:
```c
// INCORRECT - creates dangling pointer
val_t value = val(42);
char *bad_ptr = valtostr(value).str;  // bad_ptr becomes invalid immediately after
```

**Correct Storage**:
```c
// CORRECT - keep the object alive
val_t value = val(42);
valtostr_t str_obj = valtostr(value);
char *good_ptr = str_obj.str;         // valid while str_obj is in scope
```

---

## Pairs

A pair is a `val_t` value that packs two integers into the payload bits of a
single NaN-boxed word:

```c
val_t valpair(l, r);   // Create the pair <l,r>
int   valispair(x);    // Check if x is a pair
int64_t valleft(x);    // The left component (16 bits)
int64_t valright(x);   // The right component (32 bits)
```

**Purpose**: Compact storage of two small integers (for example a 16-bit
index and a 32-bit value) in one `val_t`.

- `l` is masked to 16 bits and `r` to 32 bits; larger values are truncated.
- Pairs are compared by `valcmp()` lexicographically: first the left
  component, then the right one, and consistently with `valeq()` (two pairs
  are equal iff both components are equal).
- `valtostr()` prints a pair as `<L,R>` in hexadecimal, or with a custom
  `printf` format: `valtostr(p, "<%PRIu16,%PRIu32>")`.

### Example

```c
#include "val.h"
#include <stdio.h>

int main(void) {
    val_t p = valpair(0xA1A2, 0xB1B2B3B4);

    printf("pair: %s\n", valtostr(p).str);        // <A1A2,B1B2B3B4>
    printf("left: %d right: %u\n",
           (int)valleft(p), (unsigned)valright(p));

    val_t q = valpair(0xA1A2, 0x0);
    printf("cmp: %d\n", valcmp(p, q));            // > 0 (same left, right differs)
    return 0;
}
```

---

## Stored Strings

A stored string is a `val_t` value that **owns** a NUL-terminated string whose
memory is managed by the library. The string bytes live in a library-managed
arena; the `val_t` handle encodes the position of the string inside the arena
(see [String conversion](#string-representation) for how it prints).

```c
val_t    valstore(str);          // Copy str into the arena, return a handle
void    *valrecall(x);           // Return a pointer to the stored string
int      valdel(x);              // Delete the stored string (tombstone)
int      valisstored(x);         // Check if x is a stored string
```

**Purpose**: Hand off ownership of a string to the library, so a `val_t` can be
stored in a container without the caller having to keep the string alive or
free it. `valstore(NULL)` is rejected with `errno = EINVAL`.

A stored value **behaves as a string in every val function**:

- `valcmp()` compares it by content (`strcmp`), so `valstore("x")` and
  `val("x")` compare equal.
- `valhash()` hashes the content (same hash as a `char*` with the same text).
- `valtostr()` prints the content.
- `valtoptr()` returns a pointer to the content.
- `valtoint()` parses the content with `strtoll` (base 10); non-numeric
  content yields `0`.

Because the bytes live in an arena, any pointer returned by `valrecall()` or
`valtoptr()` is only valid until the *next* `valstore()` call (the arena may
reallocate). Keep the handles, not the pointers.

### Deleting stored strings

`valdel(x)` deletes a stored string with tombstone semantics: the slot and
bytes are kept, but `valrecall()`/`valtoptr()`/`valget` return `NULL`/`valnil`
with `errno = ENOENT` for it afterwards, and the handle can be re-inserted
with `valstore()`.

### Inspecting the arena

Each store (there is currently one, index 0) keeps a registry of its entries:

```c
int32_t val_store_cnt(n);      // Number of entries (tombstones included)
int32_t val_store_sze(n);      // Arena capacity in bytes
int32_t val_store_top(n);      // Index of the last entry (== cnt)
int32_t val_store_len(n, i);   // Length in bytes (NUL included) of entry i
int32_t val_store_offset(n, i);// Arena offset of entry i (1-based indices)
```

### Example

```c
#include "vallib.h"
#include <stdio.h>

int main(void) {
    val_t s = valstore("pippo");

    printf("stored: %s\n", valisstored(s) ? "yes" : "no");
    printf("content: %s\n", (char *)valrecall(s));
    printf("cmp: %d\n", valcmp(s, val("pippo")));   // 0: same content

    valdel(s);
    printf("after del: %s\n", valrecall(s) ? "still there" : "gone");
    return 0;
}
```

---

## Data Structures

The companion module (`vallib.h` + `vallib.c`) provides dynamic data
structures built on `val_t`. There is a single container type — the **vec** —
a dynamically resized array of `val_t` values, that can be used as an array, a
stack, a queue, or a map depending on the functions you call on it.

```c
val_t valvec(n);   // Create a vec (n = optional initial capacity)
val_t valfree(a);  // Free a vec and its storage; returns valnullptr
```

### Vectors

```c
int32_t valsize(a);     // Capacity
int32_t valcount(a);    // Number of elements
int valset(a, i, v);    // Set element at index i (integer index)
val_t valget(a, i);     // Get element at index i
int valdel(a, f, t);    // Delete the elements in [f, t]
int valins(a, i, e);    // Insert element e at index i
int valclear(a);        // Remove all elements
```

Indices are zero-based; negative indices count from the end (`-1` is the last
element). `valset()` grows the vec as needed. `valset()` is **array-only**: the
index must be an integer (for key/value maps see
[Maps](#maps)).

### Stacks

```c
val_t valpush(a, e);  // Push e on top of the stack
val_t valtop(a);      // Peek at the top element (without removing it)
val_t valpop(a);      // Pop the top element and remove it
int   valisempty(a);  // True if the stack is empty
```

### Queues

```c
val_t valenq(a, e);   // Enqueue e at the back of the queue
val_t valdeq(a);      // Dequeue the first element and remove it
val_t valfirst(a);    // Peek at the first element (without removing it)
val_t vallast(a);     // Peek at the last element (without removing it)
```

The queue is compacted automatically as elements are dequeued.

### Maps

A vec can be used as a **map**: a sorted array of (key, value) pairs. Keys are
unique — inserting an existing key updates the value in place.

```c
int valmap(m, k, v);  // Insert or update the pair key k -> value v
val_t valget(m, k);   // Look up the value for key k
int valdel(m, k);     // Delete the entry for key k
```

- **Keys**: any `val_t` value. `char*` and `val_t` keys are accepted directly;
  **integer keys must be wrapped** in `val()` because `valget()`/`valdel()`
  dispatch integer arguments to the array functions (e.g. `valget(m, val(42))`).
- **Equality**: keys are compared with `valcmp()`, so `1` and `1.0` are the
  same key, and a stored string and a `char*` with the same text are the same
  key.
- **Not found**: `valget()` returns `valnil` with `errno = ENOENT` (distinct
  from a stored `nil` value, which returns `errno = 0`).
- **Conversion**: the first `valmap()` call converts a fresh, empty vec into a
  map. Converting a vec that already contains elements fails with
  `errno = EINVAL`. Mixing array and map operations on the same vec is
  undefined.
- **Deletion**: `valdel()` marks the entry as deleted (tombstone) — O(log n);
  tombstones are compacted away automatically when they exceed a quarter of
  the map.
- **Counts**: for maps, `valcount()` and `valsize()` report the number of
  entries (not val_t slots).

### Sorting and Searching

```c
int    valsort(a [, cmp [, aux]]);     // Sort in place (default: valcmp)
int32_t valbsearch(a, e [, cmp [, aux]]); // Binary search for e
```

`valsort()` sorts the vec in place (quicksort); `valbsearch()` finds an element
in a sorted vec and returns its index, or -1. Both accept an optional
comparator and an auxiliary value passed to it (`valaux()`).

### Example

```c
#include "vallib.h"
#include <stdio.h>

int main(void) {
    val_t m = valvec(0);

    valmap(m, "pippo", "pluto");
    valmap(m, 42, "answer");
    valmap(m, "pippo", "paperino");   // updates the existing entry

    printf("count: %d\n", (int)valcount(m));       // 2
    printf("pippo: %s\n", (char *)valtoptr(valget(m, "pippo")));
    printf("42:    %s\n", (char *)valtoptr(valget(m, val(42))));

    valdel(m, 42);
    printf("after del: %s\n", valget(m, val(42)) ? "found" : "gone");

    m = valfree(m);
    return 0;
}
```

---

## Performance Considerations

### Optimization Features

- **Branch-free operations**: Most type checks and conversions avoid conditional branches
- **O(1) complexity**: All operations execute in constant time
- **Cache-friendly**: 64-bit values fit in single cache lines
- **Minimal overhead**: No separate type tags or metadata

---

## Examples

### Complete Example: Mixed Data Container

```c
#include "val.h"
#include <stdio.h>
#include <stdlib.h>

// Simple dynamic array of val_t values
typedef struct {
    val_t *data;
    size_t size;
    size_t capacity;
} val_array_t;

val_array_t* val_array_create(size_t initial_capacity) {
    val_array_t *arr = malloc(sizeof(val_array_t));
    arr->data = malloc(sizeof(val_t) * initial_capacity);
    arr->size = 0;
    arr->capacity = initial_capacity;
    return arr;
}

void val_array_push(val_array_t *arr, val_t value) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, sizeof(val_t) * arr->capacity);
    }
    arr->data[arr->size++] = value;
}

void val_array_print(val_array_t *arr) {
    for (size_t i = 0; i < arr->size; i++) {
        val_t v = arr->data[i];
        
        if (valisnumber(v)) {
            if (valisint(v)) {
                printf("[%zu] Integer: %" PRId64 "\n", i, valtoint(v));
            } else {
                printf("[%zu] Double: %f\n", i, valtodouble(v));
            }
        } else if (valischarptr(v)) {
            printf("[%zu] String: %s\n", i, (char*)valtoptr(v));
        } else if (valisbool(v)) {
            printf("[%zu] Boolean: %s\n", i, valeq(v, valtrue) ? "true" : "false");
        } else if (valisnil(v)) {
            printf("[%zu] Nil\n", i);
        } else if (valisconst(v)) {
            printf("[%zu] Constant: %s\n", i, valtostr(v).str);
        } else {
            printf("[%zu] Unknown type\n", i);
        }
    }
}

void val_array_destroy(val_array_t *arr) {
    free(arr->data);
    free(arr);
}

int main() {
    val_array_t *container = val_array_create(4);
    
    // Add various types
    val_array_push(container, val(42));
    val_array_push(container, val(3.14159));
    val_array_push(container, val("Hello, val!"));
    val_array_push(container, valtrue);
    val_array_push(container, valnil);
    val_array_push(container, valconst("TOKEN"));
    val_array_push(container, valconst(999));
    
    printf("Container contents:\n");
    val_array_print(container);
    
    // Demonstrate comparison
    val_t search_value = val(42);
    printf("\nSearching for value 42:\n");
    for (size_t i = 0; i < container->size; i++) {
        if (valeq(container->data[i], search_value)) {
            printf("Found at index %zu\n", i);
            break;
        }
    }
    
    val_array_destroy(container);
    return 0;
}
```

---

This reference manual provides comprehensive documentation for the val C library. For additional examples and advanced usage patterns, refer to the library's test suite and example programs.