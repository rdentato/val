# Val Library API Cheat Sheet

## Value Creation

| Function | Description |
|----------|-------------|
| `val(x)` | Create a val_t from any supported C type (auto-selects conversion method) |

## Constants

### Predefined Constants

| Constant | Description |
|----------|-------------|
| `valtrue` | Boolean true value |
| `valfalse` | Boolean false value |
| `valnil` | Nil/null value |
| `valnullptr` | Null pointer value |
| `valconst(n)` | Create a custom constant with value n |

### User Defined Constants
Custom constants can be defined using the `valconst()` macro with a 32 bits numeric identifier:

```c
// Define application-specific special values
#define notfound    valconst(1)
#define end_of_list valconst(2)
#define retry       valconst(3)
#define not_ready   valconst(4)
```
All these constants are guaranteed to be different from any other val_t value.

## Type Checking

| Function | Description |
|----------|-------------|
| `valisdouble(v)` | Check if value is a double |
| `valisinteger(v)` | Check if value is any integer (signed or unsigned) |
| `valisinteger(v)` | Check if value is a signed integer |
| `valisinteger(v)` | Check if value is an unsigned integer |
| `valisbool(v)` | Check if value is a boolean |
| `valisnil(v)` | Check if value is nil |
| `valisconst(v)` | Check if value is a custom constant |
| `valisptr(v)` | Check if value is any pointer type |
| `valischarptr(v)` | Check if value is a character pointer |
| `valisnullptr(v)` | Check if value is a null pointer |

## Value Extraction

| Function | Description |
|----------|-------------|
| `valtodouble(v)` | Extract double value |
| `valtoint(v)` | Extract signed integer value |
| `valtounsignedint(v)` | Extract unsigned integer value |
| `valtobool(v)` | Extract boolean value |
| `valtoptr(v)` | Extract raw pointer value |
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


## Define Custom Pointer Types

```c
// Before including val.h:
#define valpointer_0_t FILE *
#define valpointer_1_t YourType *
// ... and so on for types 2-4
#include "val.h"
```

**Remember:** All val functions automatically convert their arguments to val_t!
