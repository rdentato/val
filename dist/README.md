
# `val` and `vallib` distribution file

Create the files with:

```
$ cd src/
$ make clean_dist
$ make dist
```

You will find three files in the `dist` directory:

  - `val.h` : If you want to just use the NaN-Boxing functions, `#include` this header 
  - `vallib.h`, `vallib.c`: If you want to use the vector, stack, map, etc functions you `#include` the `vallib.h` header (you don't need a separate copy of `val.h`), and compile+link `vallib.c` with the rest of your source files.
