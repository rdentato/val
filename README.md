# val — A NaN-Boxed Dynamic Value Library for C

[![Version](https://img.shields.io/badge/version-0.3.0%20Beta-blue.svg)](https://github.com/yourusername/val)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

`val` is a single-header, zero-dependency library that implements a dynamic **`val_t`** type for C programs via **NaN-boxing**. It can store:

* IEEE-754 double-precision floats
* Signed and unsigned 48-bit integers
* Typed pointers (void, char, custom payload)
* Booleans, `nil`, and user-defined constants

All in one 64-bit word, with efficient runtime checks, comparisons, and hashing.

### Tested on:

|   OS            | compiler        | notes                       |
|-----------------|-----------------|-----------------------------|
| Linux           | gcc 11.4.0      | 64-bits and 32-bits targets |
| Linux           | clang 14.0.0    | 64-bits and 32-bits targets |
| MS Windows 11   | Mingw64 15.1.0  | 64-bits and 32-bits targets |
| MS Windows 11   | MS cl 19.44     | /std:C11                    |
| MS Windows 11   | Pelles C 13.0   | 64-bits and 32-bits targets |

---

## Features

* **Compact representation**: 64-bit `val_t` uses NaN payload bits.
* **Fast type checks**: Macro utilities like `valisdouble()`, `valisinteger()`, `valisptr()`, `valisbool()`, `valisnil()`.
* **Generic constructors**:

  ```c
  val_t v1 = val(3.14);     // A double
  val_t v2 = val(-42);      // A signed integer
  val_t v3 = val("hello");  // A string
  ```
* **Generic extractor**:

  ```c
  double d  = valtodouble(v1);
  int64_t i = valtoint(v2);
  char *s   = valtoptr(v3);
  ```
* **Comparison & hashing**:

  * `valcmp(a,b)` returns –1, 0, or 1.
  * `valhash(a)` produces a 32-bit FNV1a or Murmur-style hash.
* **Zero dependencies**: Just include `val.h` in any C11/C17 project.
* **Extensible**: Define additional pointer-tags or constants via macros.

---

## Installation

Simply copy `val.h` into your project’s include directory:

```bash
curl -O https://raw.githubusercontent.com/yourusername/val/v0.3.0-beta/val.h
```

Then in your source:

```c
#include "val.h"
```

No build steps required.

---

## Quick Start

```c
#include <stdio.h>
#include "val.h"

int main(void) {
    // Create various values
    val_t d = val(2.71828);
    val_t i = val(-12345);
    val_t u = val((unsigned int)12345);
    val_t b = val((_Bool)true);
    val_t s = val("NaN-boxing");

    // Type checks
    assert(valisdouble(d));
    assert(valisinteger(i));
    assert(valisinteger(u));
    assert(valisbool(b));
    assert(valischarptr(s));

    // Conversions
    printf("double: %f\n", valtodouble(d));
    printf("signed: %lld\n", (long long)valtoint(i));
    printf("unsigned: %llu\n", (unsigned long long)valtounsignedint(u));
    printf("bool: %s\n", valtobool(b) ? "true" : "false");
    printf("string: %s\n", (char *)valtoptr(s));

    // Comparison
    val_t x = val(10);
    val_t y = val(20);
    printf("cmp(x,y)  = %d\n", valcmp(x,y));   // –1
    printf("cmp(x,10) = %d\n", valcmp(x,10));  // 0
    
    // Hashing
    printf("hash(\"NaN-boxing\") = 0x%x\n", valhash(s));
    return 0;
}
```

---

## Documentation

See the `docs` directory for full documentation.

The `test` directory is also a source of valuable information.

---

## References

* **NaN-Boxing Overview**
  Piotr Duperas, “NaN boxing or how to make the world dynamic”
  [https://piotrduperas.com/posts/nan-boxing](https://piotrduperas.com/posts/nan-boxing)
* **Nanobox Implementation**
  Viktor Söderqvist, nanbox.h (zuiderkwast)
  [https://github.com/zuiderkwast/nanbox/blob/master/nanbox.h](https://github.com/zuiderkwast/nanbox/blob/master/nanbox.h)
* **Interpreter Optimization**
  Robert Nystrom, *Crafting Interpreters*, Chapter “Optimization”
  [https://craftinginterpreters.com/optimization.html](https://craftinginterpreters.com/optimization.html)
* **IEEE 754 & Architecture Manual**
  Intel® 64 and IA-32 Architectures Software Developer’s Manual, Vol 1 §4.2.2, Table 4-3

---

## Changelog

* **0.3.0 Beta** (May 2025)

  * Introduced generic `_Generic` constructors.
  * Added pointer tags 0–7.
  * Improved `valcmp` and `valhash` performance.

---

## License

© 2025 Remo Dentato ‹[rdentato@gmail.com](mailto:rdentato@gmail.com)›
Released under the **MIT** License.
