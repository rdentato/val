# Compiling

The source files are set up so that `val` and `vallib` can be used separately.

- **`val.h`** — the header-only core: `val_t`, boxing, predicates, comparison/hashing, string conversion. No `.c` to compile or link.
- **`vallib.h` + `vallib.c`** — the library layer (containers, store arena, buffers): `vallib.h` amalgamates `val_lib_pre.h` + `val.h` + `val_lib_post.h`; `vallib.c` is the single TU that instantiates the non-inline code.

See "Core vs. Library" in `docs/reference.md` for when to use which.

After having modified one of the files, here in the `src` directory, run the command `make dist` to create the files that can be used in projects.
