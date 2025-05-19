# Val Library API Cheat Sheet

## Value Creation

| Function | Description |
|----------|-------------|
| `val(x)` | Create a val_t from any supported C type (auto-selects conversion method) |
| `val_fromdouble(d)` | Create a val_t from a double |
| `val_fromfloat(f)` | Create a val_t from a float |
| `val_fromint(i)` | Create a val_t from a signed integer |
| `val_fromuint(u)` | Create a val_t from an unsigned integer |
| `val_frombool(b)` | Create a val_t from a boolean |
| `val_frompvoidtr(ptr)` | Create a val_t from a void pointer |
| `val_fromcharptr(str)` | Create a val_t from a character pointer (string) |
| `val_frombufptr(buf)` | Create a val_t from a buffer pointer |
| `val_fromptr_0-4(ptr)` | Create a val_t from a custom pointer (types 0-4) |

## Predefined Constants

| Constant | Description |
|----------|-------------|
| `valtrue` | Boolean true value |
| `valfalse` | Boolean false value |
| `valnil` | Nil/null value |
| `valnullptr` | Null pointer value |
| `valconst(n)` | Create a custom constant with value n |

## Type Checking

| Function | Description |
|----------|-------------|
| `valisdouble(v)` | Check if value is a double |
| `valisinteger(v)` | Check if value is any integer (signed or unsigned) |
| `valissignedint(v)` | Check if value is a signed integer |
| `valisunsignedint(v)` | Check if value is an unsigned integer |
| `valisboolean(v)` | Check if value is a boolean |
| `valisnil(v)` | Check if value is nil |
| `valisconst(v)` | Check if value is a custom constant |
| `valispointer(v)` | Check if value is any pointer type |
| `valischarptr(v)` | Check if value is a character pointer |
| `valisbufptr(v)` | Check if value is a buffer pointer |
| `valisnullptr(v)` | Check if value is a null pointer |

## Value Extraction

| Function | Description |
|----------|-------------|
| `valtodouble(v)` | Extract double value |
| `valtosignedint(v)` | Extract signed integer value |
| `valtounsignedint(v)` | Extract unsigned integer value |
| `valtobool(v)` | Extract boolean value |
| `valtopointer(v)` | Extract raw pointer value |
| `valpointertag(v)` | Get the tag of a pointer value (to determine pointer type) |

## Operations

| Function | Description |
|----------|-------------|
| `valeq(a, b)` | Compare values for equality |
| `valcmp(a, b)` | Compare values (-1: a<b, 0: a=b, 1: a>b) |
| `valhash(v)` | Generate a hash value |

## Memory Considerations

- Integers limited to 48 bits
- Only lower 48 bits of pointers stored
- All values consume exactly 8 bytes

## Types Overview

| Type | Bit Pattern |
|------|-------------|
| Double | [IEEE-754 double that's not a NaN] |
| Signed Int | 0x7FFF000000000000 | [48-bit value] |
| Unsigned Int | 0x7FFE000000000000 | [48-bit value] |
| Boolean | 0x7FF8FFFF0000000[0/1] |
| Nil | 0x7FF800FF00000000 |
| Custom Constant | 0x7FF8000F00000000 | [32-bit ID] |
| Void Pointer | 0xFFF8000000000000 | [48-bit address] |
| Char Pointer | 0xFFF9000000000000 | [48-bit address] |
| Buffer Pointer | 0xFFFA000000000000 | [48-bit address] |
| Custom Pointer 0-4 | 0xFFFF-0xFFFB000000000000 | [48-bit address] |

## Define Custom Pointer Types

```c
// Before including val.h:
#define valpointer_0_t FILE *
#define valpointer_1_t YourType *
// ... and so on for types 2-4
#include "val.h"
```

## Define Custom Constants

```c
#define notfound valconst(1)
#define end_of_list valconst(2)
// Custom constant functions
if (valeq(value, notfound)) { /* handle not found */ }
```

**Remember:** All val functions automatically convert their arguments to val_t!
