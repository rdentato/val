# Questions & Answers for `val.h`

Below is a curated list of questions a programmer might ask to better understand the inner details of the `val.h` library, and the corresponding answers.

## 1. NaN-boxing scheme

**Question**: How exactly are Quiet NaNs (versus signaling NaNs) used to encode non-floating types, and what hardware or OS assumptions does this make?

**Answer**: The library exploits the IEEE‑754 standard where a NaN is any value with all exponent bits set to 1. Within NaNs, the most significant fraction bit (bit 51) distinguishes Quiet NaNs (QNaN, bit 51=1) from Signaling NaNs (SNaN, bit 51=0). `val.h` chooses to use only QNaNs to carry its own type tags and payloads, ensuring that hardware or OS signaling of SNaNs never overlaps with encoded values. This assumes the platform strictly adheres to IEEE‑754 double‑precision semantics and does not canonicalize QNaN payload bits (i.e., preserves the full 52‑bit mantissa) on arithmetic operations or transfers.


**Question**: Which specific bit patterns are reserved for “true” NaNs (FPU NaNs) versus the library’s payload-carrying NaNs, and how do we avoid collisions with compiler-generated NaNs?

**Answer**:
- **Reserved Ranges:** The library reserves the 16 patterns in the QNaN space: on the positive side, any 64‑bit word whose top 16 bits are `0x7FF8`–`0x7FFF` indicates a tagged value; on the negative side, `0xFFF8`–`0xFFFF`.
- **True NaNs:** Canonical FPU‑generated QNaNs (e.g., via `0.0/0.0`) typically use `0x7FF80000_00000000` (positive nan) and `0xFFF80000_00000000` (negative nan) with payload all zero. Since `val.h` encodes payloads with non‑zero bits in the high mantissa region, (i.e. ≥ `0x7FF90000_00000000`) these default FPU NaNs remain distinct.
The same goes for infiniies like `log(0.0)` whose representation are `0x7FF00000_00000000` (positive inf) and `0xFFF80000_00000000` (negative inf). Canonical NaNs are considered numbers by the `valisnumber()` function.

- **Avoiding Collisions:** By only interpreting words where the type‑tag bits (mantissa bits 50–48) are non‑zero, the library effectively ignores canonical NaNs with zero payload. All user‑visible NaNs from `val.h` are created through `val()`, which explicitly set the tag bits and payload; generic compiler/math library NaNs never match a valid tag pattern and are treated as floating‑point values.


## 2. Types layout

**Question**: How are the type bits (bit 63 and bits 48–50) organized in the 64-bit word, and how does each mask (`VAL_TYPE_MASK`, `VAL_F7_TYPE_MASK`, `VAL_PAYLOAD_MASK`) work to extract type or payload?

**Answer**: Each `val_t` is a 64-bit word laid out as follows:

```

       exponent   ╭─── Quiet NaN
    ╭──────┴────╮ ▼
   x111 1111 1111 1xxx FF FF FF FF FF FF
   │╰──────┬──────╯╰┬╯ ╰───────┬───────╯
 type      │      type         │ 
           │                   │ 
    12 bits set to 1   Payload (48 bits)
          7FF8


| bit 63 | bits 62–52 | bit 51 | bits 50–48 | bits 47–0      |
| sign   | exponent   | QNaN   | type       | payload (data) |
```

- **Sign (bit 63):** Used for types.
- **Exponent (bits 62–52):** all ones (`0x7FF`) signal NaN-boxing.
- **Quiet NaN bit (bit 51):** always 1 for tagged values.
- **Type bits (bits 50–48):** eight type values (0–7) indicating category (constant, pointer subtype, etc.).
- **Payload (bits 47–0):** stores value.

There are several Mask definitions to ease the identification of a boxed type. 
Thanks to the way the types are coded, it usually does need one or two bitwise and operations and few comparison to determine the type of a value.

For example, to check if a value `x` is a boxed  `void *` you simply do:

```c
 ((val(x).v & VAL_TYPE_MASK) == VALPTR_VOID)
```
Just a bitwise `AND` and a comparison.


**Question**: What’s the full mapping from the high-nibble patterns (`7FF8`–`7FFF` / `FFF8`–`FFFF`) to doubles, constants, pointers, and reserved slots?

**Answer**: The top 16 bits (`bits 63–48`) form a hex pattern in the range `7FF9-FFFF`:

|                  |      7F_         |      FF_       | Tag |     Note                       |
|------------------|------------------|----------------|-----|--------------------------------|
| **_F8** (`1000`) |    NAN           |    -NAN        | N/A | Reserved for FPU NAN           |
| **_F9** (`1001`) |   constants      |  (reserved)    | N/A | FFF9 is kept for extensions    |
| **_FA** (`1010`) |   `void *`       |    `char *`    | NO  | Predefined pointer types       |
| **_FB** (`1011`) | `valptr_vec_t`   | `valptr_buf_t` | YES | Predefined pointer types       |
| **_FC** (`1100`) |   `FILE *`       |  `valptr_6_t`  | YES | Library/User defined pointers  |
| **_FD** (`1101`) | `valptr_5_t`     |  `valptr_4_t`  | YES | Library/User defined pointers  |
| **_FE** (`1110`) | `valptr_3_t`     |  `valptr_2_t`  | YES | User defined pointer types     |
| **_FF** (`1111`) | `valptr_1_t`     |  `valptr_0_t`  | YES | User defined pointer types     |
  
If the first 16 bits are not in this range, it is considered a number (in IEEE754 format).

The constants have a *sub-type* which is stored in the next 16 bits:

|  top 32 bits  | payload type   | 
|---------------|----------------|
| **7FF9 93F0** | Boolean        |
| **7FF9 B3F0** | Nil            |  
| **7FF9 C3F0** | User defined   |
| **7FF9 xxxx** | Symbolic const |   


## 3. Macro vs inline function mechanics

**Question**: How does the `val(x)` macro, using C11 `_Generic` and `val_from*` functions, ensure each argument is only evaluated once, and what pitfalls remain for side-effectful expressions?


**Answer**:
The `val(x)` macro is defined using C11 `_Generic` to select a specific `val_from*()` inline constructor based on the compile-time type of `x`. Because `_Generic` performs type-based selection without evaluating its controlling expression, the expansion `val(x)` becomes a single call—e.g., `val_fromint((x))` or `val_fromptr((x))`—ensuring `x` is evaluated exactly once.

Note that `val()` is defined so that if the argument is a `val_t`, it is passed "as is" with no further processing. This ensures that calling `val(x)` does not incur in any undesired overhead.

**Question**: When should we rely on the macro wrappers (e.g., `valisnumber(x)`) versus calling the underlying `static inline` functions (`val_isnumber(val_t)`) directly?

**Answer**: Functions, macro and variable starting with `val` or `VAL` NOT followed by an underscore (`_`) are *public* and part of the library's API.
Everything starting with `val_` (or `VAL_`) is considered *private* and can be used only within the library.
The public functions often takes care of converting the argument to `val_t` or handling optional parameters. Private functions can only be used within the library, for example you can use `val_isconst_1(val_t v)` if you know already that your value is already boxed and you don't need to check for the actual value. 

  > **Never, use `val_` and `VAL_` functions outside the library code!**


## 4. Pointer tagging & alignment

**Question**: What minimum alignment (`VAL_MIN_ALIGN`) is required to safely tag pointers, and how does the library behave (via `VALNOTAGPTR`) on platforms that don’t meet it?

**Answer**: Pointer tagging happens in the lower bits of the address. For this to be possible, we need to be sure that those bits are normally zero. This is usually the case for pointers returned by `malloc()` since the allocator needs to to ensure that the returned address is properly aligned for any object it might want to point at. 

Depending on the architecture, if a pointer is misaligned (for example, an `int` starting on an odd address) there might be issues ranging from poor performance to program crash.

The most common scenario is that `malloc()` returns at least an 8-bytes aligned address so that doubles are properly aligned. This is true also for 32-bits architectures.

The library assumes that this is the case and sets `VAL_MIN_ALIGN` to 8 and checks this assumption with a static assert:

```c
   static_assert(alignof(val_align_t) >= VAL_MIN_ALIGN)
```
The type `val_align_t` is either `max_align_t` (as specified by the C11 standard), or `double` (to accomodate Microsoft `cl` lack of `max_align_t`).

If the platform you're using does not meet this assumption, or you just want to remove pointer tagging altogether, you can define the symbol `VALNOTAGPTR` during compilation.
The effect is that the function `val_check_taggable_ptr()` will always return false and, hence, no pointer can be tagged.


**Question**: Why are `void*` and `char*` pointers explicitly excluded from tagging, and how does `val_check_taggable_ptr()` determine taggability?

**Answer**: `void*` and `char*` pointers are designed to point to any arbitrary memory address, including potentially unaligned ones. Tagging relies on the lower bits of an aligned pointer, which would conflict with the general-purpose nature of `void*` and `char*` if they were to point to misaligned data.

Since a `void *` could be any pointer, including a `char *`, it can't be tagged either.

All other pointer are taggable.

**Question**: What are the implications of `valtagptr()` returning a new `val_t` rather than modifying in-place?

**Answer**:  `valtagptr()` returns a *new* `val_t` value with the updated tag because `val_t` is a value type (a `struc`), not a pointer. Modifying it in-place would violate C's pass-by-value semantics for `struct` parameters. Callers must assign the returned `val_t` to capture the tagged value.

## 5. Custom pointer types & buffers.

**Question**: How do you define and register new `valptr_X_t`?
**Answer**: There two separate ways.

***Pointer to a struct***

The most common case is when the new type is a pointer to a structure. In this case, you define your new type as in this example:

```c
// Just complete the definition of struct vlaptr_1_s
typedef struct valptr_1_s {
  double x;
  double y;
} *Point_t;

// Define a meainingful macro to check if a boxed value is a Point
#define isPoint(v) valisptr(v,VALPTR_1)

// Include val.h AFTER having defined the new type
#include "val.h"

// Let's assume you have constructors/destructors:
Point_t newPoint(double x, double y);
void freePoint(Point_t pt);

int main()
{
  val_t pt;
  pt = val(newPoint(4.3, -2.1)); // You can now box Point_t values
  if (isPoint(pt))
    freePoint(valtoptr(pt));
}
```
***Pointer to other types***
If the new type is a pointer to a union or to another exisiting type, you can redefine `valptr_X_t` instead:

```c
typedef int *ptrtoint_t;
#define valptr_0_t ptrtoint_t
#define isptrtoint(v) valisptr(v,VALPTR_0);

#include "val.h"

int main() 
{
   int x = 0;
   int l[10] = {0};
   ptrtoint_t p = NULL;

   val_t values[5];

   p = malloc(100 * sizeof(int));
   values[0] = val(p);  // Boxed as an int *
   values[1] = val(l);  // Boxed as an int *
   values[2] = val[&x]; // Boxed as an int *

   free(p);
}
```
You have to use `#define xxx valptr_X_t`, you can't directly define the type `valptr_X_t` with a `typedef`.

> **Note**: Remember that `typedef` does NOT define a new type, it only gives names to existing type. This means that, for example, in this code:
```c
  typedef int *ptrtoint_1;
  typedef int *ptrtoint_2;
```
>`ptrtoint_1` and `ptrtoint_2` are the SAME type (a pointer to an `int`) and they are treated accordingly. If you try to assign them to two different `valptr_X_t`, you will get an error.

**Question**: How are growable text buffers managed?
**Answer**: Growable text buffers are managed by the library (when using `vallib`).
The library defines `struct valptr_buf_s` and provides the API to create, modify and free buffers
(`valbufnew()`, `valbufcpy()`, `valbufcat()`, `valbufadd()`, `valbuffmt()`, `valbufcatf()`, `valbufins()`,
`valbufdel()`, `valbufclear()`, `valbufreserve()`, `valbuflen()`, `valbufsze()`, `valbuftxt()`, `valfree()`):

```c
  #include "vallib.h"

  int main()
  {
    val_t a_string;
    val_t a_buffer;

    a_buffer = valbufnew(0);

    a_string = val("Hello!!");
    valbufcpy(a_buffer, "Hello!!");

    if (valcmp(a_string,a_buffer) == 0)
      printf("They are equal!!\n");

    valfree(a_buffer);
  }

```
The library guarantees that the buffer content is always a `\0`-terminated string, so `valcmp()` and `valhash()`
can treat it as a string without any user code.

**Note**: The library owns the buffer struct: when using `vallib` do **not** define `struct valptr_buf_s` yourself.
In header-only mode (just `val.h`, without `vallib`) the library cannot manage buffers: you may define your own
buffer struct whose **first field is a `char *`** holding a `\0`-terminated string so that `valcmp()`/`valhash()`
treat it as a string, but allocation and growth are your responsibility.

**Question**: How are `valptr_buf_t` buffers treated for hashing and comparison,
**Answer**: They are treated as strings, by their content.

## 6. Constants encoding

**Question**: How are numeric versus symbolic constants distinguished at bit-level (using `VAL_CONSTTYPE_MASK`, `VAL_CONST_0`, `VAL_SYM_MASK`, etc.), 
**Answer**: Numeric constants are a way to create `val_t` values that are guaranteed to be different from any other val_t value.
This can be useful to simplify the logic of some algorithm by introducing special values like `end_of_list` or `empty_slot`.
You can create up to 2^32 numeric constant using the `valconst()` function, and convert them back to integers via the `valtoint()` function.

```c
  int main()
  {
    val_t end_data = valconst(10);
    val_t data[100];

    // ... some other code here ....
    for (int i=0; i<100 && !valeq(data[i],end_data))  {
      // Use the special value end to stop the loop earlier
    }
  }
```

**Question**: What is a "*symbolic constants*", what is the maximum length and the permitted character set?
**Answer**: Sometimes it's useful to create constants that could be print back as string instead of numbers.
You can pass a string to `valconst()` to create such constant with the following limitations:

  - No more than 8 characters
  - Only the following characters are permitted:
     -  any lowercase letters: `a-z` (uppercase letters will be converted to lowercase)
     -  any digit: `0-9`
     -  some special characters: `!#$*+-./:<=>?@[]_~`


```c
  int main()
  {
    val_t end_data = valconst("end_data");
    val_t empty_slot = vaconst(31);
    val_t data[100];

    // ... some other code here ....
    int i = 0;
    for (i = 0; i<100 && !valeq(data[i],end_data) && !valeq(data[i],empty_slot))  {
      // Use the special value end to stop the loop earlier
    }
    // Will print "end_data" or "31" depending on the value of data[i]
    if (i<100) printf("found: %s",valtostr(data[i]).str); 
  }
```

From the example is clear that (especially for debugging), having a symbolic string makes it easier to interpret a message.

Note that for the sake of comparison and hashing, **stored strings** are treated as strings (by content), while symbolic constants are **not**: they compare by their raw encoded value.

```c
int main()
{
  val_t s = val("help!");
  val_t h = valconst("help!");
  val_t st = valstore("help!");

  if (valeq(s,h)) print("Error! they are two different val_t values\n");

  if (valcmp(s,st) == 0) print("Correct! Stored strings are the same!\n");

  if (valcmp(s,h) != 0) print("Correct! Symbolic constants compare by value\n");
}
```

***Internal encoding***

All constants use the NaN space with top bits `7FF9`. For the predefined constants `valnil`, `valtrue` and `valfalse`, the next 16 bits hold the values:

  |   top bits  | Binary sub-type       |  type        |
  |-------------|-----------------------|--------------|
  | `7FF9 93F0` | `1001 00`**`11 1111`** `0000` | Boolean      |
  | `7FF9 B3F0` | `1011 00`**`11 1111`** `0000` | Nil          |  
  | `7FF9 C3F0` | `1100 00`**`11 1111`** `0000` | User defined |

The nil family occupies the adjacent patterns `7FF9 B3F1`–`7FF9 B3F4` for
`valnone`, `valundefined`, `valtombstone` and `valdeleted`. They are all
distinct constants, and only `7FF9 B3F0` satisfies `valisnil()`.

They have been chosen so that the value `3F` (`0011 1111`) appears eaxctly in the middle of a character break in a symbolic constant.
Now, `3F` is the encoding of `\0` for symbolic contstant and it is used as a terminator. If a symbolic constant had those bit sets, it means that it is 6 characters long, but it this is the case, also the last bits would be `1` (you can see this in the `valsymconst()` function)! So by checking that just `3F` appears in that poisition, one can discriminate between symbolic constants ant the others.


**Question**: How to use the constant sub-types reserved for future extensions (`FFF9`)?
**Answer**: You don't. The encoding is left there for the library. Should we need an encoding, we have some free space there.

## 7. Numeric range & precision

**Question**: Given that all numbers are stored as IEEE-754 doubles, how does `valisint` detect 52-bit integer values exactly, and what happens for integers beyond that range?

**Answer**: `val_isint(v)` examines separatly each group of bits in the IEE754 representation to identifiy integer mumbers.
The best way to understand it is to look at the code, where each step is explicitly called out.

Since there are only 52 bits in the mantissa, only the integers in the range [0 , 2^52-] (or [–2^51, 2^52-1] for signed integers) can be represented exactly.
Values outside this range either lose precision in conversion to double or fail the test and are treated as non-integer floats.


**Question**: How are special IEEE-754 values (`+nan`, `-nan`, `-inf`, `inf`) treated differently by `val_isnumber` and related predicates?
**Answer**: The values `+nan` and `-nan` are explictly called out in the `val_isnumber()` function. Actually, they are folded as the sign bit is ignored.
The values `-inf`, `inf` are considered numbers as they have the Signaling bit cleared while the `VAL_NAN_MASK` is set to `0x7FF8_0000_0000_0000` which requires that bit to be set.

## 8. Unboxing & error-handling

**Question**: What error codes are set by `valtodouble`, `valtoint`, or `valtoptr` on type-mismatch, and how should a caller reliably detect and recover from such errors?
**Answer**: On error, those three functions return, respecively, `0.0`, `0` and `NULL`. They also set the `errno` value to `EINVAL`. You should check `errno` after each call. 


**Question**: Why does `valtoint()` returns a different number of payload bits for pointers and numeric constants?
**Answer**: Beside numbers, `valtoint()` also operates on constants converting them to an integer (e.g. `valnil` is converted to `0`). User defined numeric constants operate with a 32 bits number, symbolic constants use all the 48 bits of payload and so do pointers. `valtoint()` must stay consistent with this behaviour.
Note that regardless the type, `valtoint()` returns an 64 bit integer.

**Question**: What does `valtoint()` return for a stored string?
**Answer**: It parses the stored content with `strtoll()` (base 10);
non-numeric content yields `0`. This makes a stored string behave like the
string value it is (`valtoint(valstore("123")) == 123`).

## 9. String conversion

**Question**: How does `valtostr()` choose formatting based on type. 
**Answer**: By inspecting the type of the `val_t` value. If no formatting specifier has been passed to valtostr, a default one is chosen, otherwise the specified one is used.
Note that passing `""` (the empty string) as specified, produce the string representation of the `val_t` value. This may be useful to inspect unexpected behaviours

**Question**: How should the returned `valtostr_t` be used?
**Answer**: The value returned by `valtostr()` is a structure containing an array of 32 characters. Being a structure, this is copied back to the caller.
You have to access the string itself thoroug the field `.str`. The safest way is to assign it to a local variable:

```c
   valtostr_t ret;
   val_t v = valnil;
   
   ret = valtostr(v,""); // Ask for the internal representation
   printf("nil: %s",ret.str); // Prints 7FF9B3F000000000
```

once the function ends, the variable is discarded and the string is no longer accessible.

**Question**: How are pairs and stored strings formatted?
**Answer**: Pairs print as `<L,R>` in hexadecimal (or with a custom format,
e.g. `valtostr(p,"<%PRIu16,%PRIu32>")`). Stored strings print their content,
truncated to fit the 32-character buffer. Note that the content is read from
the arena, so a deleted stored string prints as an empty string.

**Question**: Could concurrent stringification cause issues?
**Answer**: No, because no global data is involved, each call has its own `valtostr_t` value to work with.


## 10. Comparison & hashing

> **Note**: The functions `valcmp()` and `valhash()` are only provided for your convenience. Your application might have different needs. For example, the `valcmp()` function treats stored strings and buffers as strings by content, while you may want to keep them separate.
> Always consider whether the provided functions are right for you or you need to write your own.

**Question**: What ordering does `valcmp` impose between numbers, strings (char\*/buffers), constants, and other pointers, and is this total order consistent across platforms?

**Answer**: `valcmp(v1, v2)` use the following criteria:

- **Strings, buffers and stored strings:** Using `strcmp` on the content. A stored string and a `char*` (or buffer) with the same text compare equal.
- **Pairs**: compared by their (left, right) components, lexicographically.
- **Numbers**: compared by numerical ordering.
- **Anything else** (including symbolic and numeric constants): compared by their raw value (order is governed by their type encoding).

  This scheme is consistent across platforms provided identical pointer ordering and endianness.

**Question**: Do `valcmp()` and `valhash()` have side effects on `errno`?
**Answer**: No. They are pure functions and never touch `errno`, so they can be
used freely inside comparators and hash-table code without disturbing the
caller's error state. (Some other functions, like `valtoptr()` and
`valtodouble()`, do set `errno` to report conversion errors.)

## 11. Versioning

**Question**: What’s the meaning of the `VAL_VERSION` hex value (0x0004009C), and what bump strategy should be used when modifying the header?
**Answer**: This is a way to keep track of the version, should there be any need to conditional compile some piece of code depending on the version.
number encodes the semantic versioning scheme. For example this is the encoding for `1.2.032-beta`:

```
       0x0102032B
         \/\/\_/\__ A:alpha, B:beta, C:Release Candidate, F:Final
          \ \ \____ patch
           \ \_____ minor
            \______ major
```
  
## 12. Testing & validation

- **Question**: What existing test suite or example programs cover the edge-cases of NaN boxing, pointer tagging, constant encoding, and error paths, and where should new tests be added for untested scenarios (e.g., reserved patterns, misaligned pointers)?

- **Answer**: Existing tests in the `tests/` directory cover a wide range of features. Also they serve as examples for the library usage.

## 13. Maps & stored strings

**Question**: How is the map implemented, and why a sorted array rather than a hash table?
**Answer**: A map is a vec whose (key, value) pairs are kept sorted on the key; lookup is a binary search via `valcmp()`, insertion of a new key shifts the tail, and inserting an existing key just updates the value in place. The sorted-array approach was chosen because:

- `valcmp()` is content-based, so `1` and `1.0` are the same key, and a stored string and a `char*` with the same text are the same key. A hash table would need hash values consistent with that equality (hash-normalization for numbers), which the sorted array sidesteps entirely.
- Lookups are cache-friendly and, for the sizes this library targets (tens to low thousands of entries), faster than hashing.
- Deletion is O(log n): entries are tombstoned and compacted away when they exceed a quarter of the map.

Should a workload ever demand it, the same API can be backed by an overflow buffer or blocked (B-tree-like) array without changing callers.

**Question**: Why do stored strings live in an arena with a registry, and why is deletion a tombstone?
**Answer**: The arena lets a `val_t` own its string without the caller managing lifetime, and the registry (offset/len per entry) gives O(1)-ish introspection (`val_store_len()`, `val_store_offset()`, ...) and stable handles: deleting marks the entry (tombstone) so other handles and offsets stay valid, and the bytes are reclaimed when the arena is redesigned/reclaimed later. Handles are by offset, so they remain valid across growth; only pointers returned by `valrecall()`/`valtoptr()` are invalidated by the next `valstore()` (the arena may reallocate).

**Question**: How does the library find a stored string given its handle?
**Answer**: Handles encode the arena offset, and the registry keeps one entry per stored string. Because entries are appended with monotonically increasing offsets, and tombstones keep their slot, the registry is always **sorted by offset** — so `valrecall()`, `valcmp()`, `valhash()` and friends look up a handle with a **binary search (O(log n))** instead of a linear scan. That invariant is load-bearing: future arena changes (e.g. reclaiming tombstone bytes) must preserve the sorted-by-offset order or switch to an index.

