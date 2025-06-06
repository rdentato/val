## Overview

This library implements a **NaN-boxing** scheme on 64-bit platforms to pack multiple C data types—floating-point, signed/unsigned integers, pointers, booleans, user-defined constants, nil, and null pointers—into a single 64-bit value (`val_t`). NaN-boxing leverages unused bit patterns in the IEEE-754 double-precision format (the “quiet NaN” region) to encode non-floating data without extra memory overhead or tagged unions.

**Key benefits**

* **Uniform representation**: Different C types share the same `val_t` container.
* **Performance**: Inlineable macros and functions for zero-overhead conversions.
* **Generality**: Supports user-defined pointer tags and constants.
* **Portability**: Works on Intel 64 and ARM64 (assumes IEEE-754 double).

---

## Version & Licensing

* **Version**: 0.3.0 Beta
* **Author**: Remo Dentato `<rdentato@gmail.com>`
* **License**: MIT (see SPDX headers)

---

## `val_t` Definition

```c
typedef struct { uint64_t v; } val_t;
static_assert(sizeof(val_t) == 8);
```

* Internally, a `uint64_t` bit pattern.
* When used in expressions, `val_t` passes by value—no hidden pointers.

---

## Tag Layout

A quiet NaN in IEEE-754 has **all exponent bits =1** and the top bit of the mantissa =1. We reserve the high 16 bits for a **tag** and the low 48 bits for a **payload**:

```
63 62   ...  52 51       48 47                            0
┌─┬───────────────┬────────┬─────────────────────────────┐
│T│ 0x7FF8…0x7FFF │  Tag   │            Payload          │
└─┴───────────────┴────────┴─────────────────────────────┘
```

* **Quiet-NaN mask**:  `VAL_NAN_MASK = 0x7FF8_0000_0000_0000`
* **Tag mask**:        `VAL_TAG_MASK = 0xFFFF_0000_0000_0000`
* **Payload mask**:    `VAL_PAYLOAD_MASK = 0x0000_FFFF_FFFF_FFFF`

Anything *not* matching the quiet-NaN pattern is treated as a raw IEEE-754 double.

---

## Supported Types & Tags

| Type               | Tag value (hex)               | Macro predicates         |
| ------------------ | ----------------------------- | ------------------------ |
| **Double**         | *Not* matching `VAL_NAN_MASK` | `valisdouble(x)`         |
| **Signed int48**   | `0xFFFF_0000_0000_0000`       | `valisinteger(x)`      |
| **Unsigned int48** | `0xFFFE_0000_0000_0000`       | `valisinteger(x)`    |
| **Void pointer**   | `0x7FF8_0000_0000_0000`       | `valisptr(x)`        |
| **Char pointer**   | `0x7FF9_0000_0000_0000`       | `valischarptr(x)`        |
| **Ptr tag 1–4**    | `0x7FFF…0x7FFC_…`             | `valpointertag(x)`       |
| **Boolean**        | `0xFFF8_FFFF_0000_0000`       | `valisbool(x)`        |
| **Nil**            | `0xFFF8_00FF_0000_0000`       | `valisnil(x)`            |
| **Null pointer**   | `0x7FF8_0000_0000_0000`       | `valisnullptr(x)`        |
| **User constant**  | `0xFFF8_000F_0000_0000`       | `valisconst(x)`          |

---

## Constructing `val_t` Values

All constructors return by value, no dynamic allocation:

```c
// Floating-point
val_t v1 = val(3.14);
val_t v2 = val(2.71f);

// Integers
val_t i1 = val(-1234567LL);
val_t u1 = val(0xDEADBEEFull);

// Pointers
void *ptr = malloc(100);
val_t p1 = val(ptr);
val_t p2 = val(strdup("hello"));
val_t p3 = val(buf);

// Boolean
val_t btrue  = val((_Bool)true);
val_t bfalse = val((_Bool)false);

// Nil & nullptr
const val_t nil = valnil;
const val_t np  = valnullptr;

// User constant (32-bit payload)
val_t c = valconst(0x1234ABCD);
```

---

## Querying & Predicates

```c
if (valisdouble(x))      // true if IEEE-754 double
if (valisinteger(x))   // true if signed int48
if (valisinteger(x)) // true if unsigned int48
if (valisptr(x[,t])) // true for pointers with tag t (or any pointer if t is omitted)
if (valischarptr(x))     // specifically a char *
if (valisbool(x))     // true for bool
if (valisconst(x))       // true for user constants
if (valisnil(x))         // true if nil
if (valisnullptr(x))     // true if C NULL pointer
```

Use one onf the predefined constants to check the pointer tags:

| Type               | Tag               |
| ------------------ | ----------------- |
|  void *            |  VAL_PTRTAG_VOID  |       
|  char *            |  VAL_PTRTAG_CHAR  |       
|  buf_t             |  VAL_PTRTAG_BUF   |       
|  val_pointer_4_t   |  VAL_PTRTAG_4     |       
|  val_pointer_3_t   |  VAL_PTRTAG_3     |
|  val_pointer_2_t   |  VAL_PTRTAG_2     |
|  val_pointer_1_t   |  VAL_PTRTAG_1     |

---

## Extracting Payloads

```c
double      d  = valtodouble(x);        // double/int → double
int64_t     si = valtoint(x);     // any numeric → signed int64
uint64_t    ui = valtounsignedint(x);   // any numeric → unsigned int64
_Bool       bb = valtobool(x);          // bool/const → C _Bool
void       *pv = valtoptr(x);       // pointer tags → void*
int        tag = valpointertag(x);      // a PTRTAG constant if it's a pointer, 0 otherwise
```

* Integers up to 48 bits map exactly to `double` (per IEEE-754 guarantees).
* Non-matching extra types coerce naturally (e.g. int → double, double → int).

---

## Comparison & Hashing

* **Identity**:

  ```c
  if (valeq(a,b)) { … }
  ```
* **Three-way compare**:

  ```c
  int cmp = valcmp(a,b);
  // <0 if a<b, 0 if equal, >0 if a>b
  ```

  * String pointers compare via `strcmp` if either operand is a C-string pointer.
  * Otherwise numeric values compare as doubles (safe for 48-bit ints).
  
* **Hashing**:

  ```c
  uint32_t h = valhash(x);
  ```

  * C-strings hash with FNV-1a on bytes.
  * Non-string values hashed via a 64→32 MurmurHash3 finalizer.

---

## Usage Example

```c
#include "val.h"
#include <stdio.h>

int main(void) {
    val_t a = val(42);    // signed integer
    val_t b = val(3.14);  // double
    val_t s = val("Hi");
    val_t n = valnil;

    // Print numeric values
    printf("a = %lld, b = %.2f\n",
           (long long)valtoint(a),
           valtodouble(b));

    // Compare (note that 100 will be autoconverted into a val_t value)
    if (valcmp(a, 100) < 0)
        puts("a < 100");

    // Hash & equality
    if (valhash(s) == valhash(val("Hi")))
        puts("Strings match by hash");

    // Pointer tags
    void *p = malloc(10);
    val_t vp = val(p);
    assert(valisptr(vp,VAL_PTRTAG_VOID));

    return 0;
}
```

---

## Limitations & Notes

* **48-bit payload only**: Pointers and integers wider than 48 bits lose high bits. Ensure payload fits.
* **NaN conflicts**: The quiet-NaN pattern `0xFFF8_0000_0000_0000` (Indefinite) is avoided but not guaranteed on all hardware.
* **Thread safety**: Purely functional, no globals, thread-safe by design.

---

## References

* Robert Nystrom, *Crafting Interpreters* (Optimization chapter)
* “NaN boxing or how to make the world dynamic” – Piotr Duperas
* Viktor Söderqvist’s nanobox implementation
* Intel® 64 and IA-32 Architectures SDM, vol 1, §4.2.2 Table 4-3
