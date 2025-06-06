# Test Directory

- [Test Directory](#test-directory)
  - [Linux](#linux)
    - [Prerequisites](#prerequisites)
    - [Makefile Targets](#makefile-targets)
    - [Examples](#examples)
    - [Adding New Tests](#adding-new-tests)
  - [Windows](#windows)
    - [Mingw64 \& MSYS2](#mingw64--msys2)
    - [Microsft C (cl)](#microsft-c-cl)
    - [Pelles C (pocc)](#pelles-c-pocc)


Linux is the development platform for the library. 

We also provide scripts to test on MS Windows with the most commonly used C compilers.

## Linux

This directory contains all of the unit and integration tests for the project. Each test source file follows the naming convention:

```c
t_<name>.c

```

When you run `make t_<name>`, the `t_<name>.c` file is compiled into a corresponding test executable:

```c
t_<name>
```

For example, to compile and run the tests in `t_core.c`, execute:

```sh
make t_core
./t_core
```

### Prerequisites

* A POSIX‐compatible environment (Linux, macOS, WSL, etc.)
* A C compiler supporting C11 or later (e.g., `gcc`, `clang`)
* GNU Make

---

### Makefile Targets

| Target          | Description                                                     |
| --------------- | --------------------------------------------------------------- |
| `make t_<name>` | Compile the test in `t_<name>.c` into the executable `t_<name>` |
| `make runtest`  | Build **all** `t_*.c` files and run each resulting test binary  |
| `make clean`    | Remove all object files, executables, and temporary files       |

The default compiler `cc` is used. To ensure compilation with `gcc` use:

```sh
make CC=gcc runtest
```
for `clang`:
```sh
make CC=clang runtest
```

If you want to test for 32-bit:

```sh
make ARCH=-m32 runtest
```


### Examples

* **Compile a single test**

  ```sh
  make t_memory
  ```

  This will compile `t_memory.c` into `t_memory`.

* **Run the full test suite**

  ```sh
  make runtest
  ```

  This will:

  1. Discover all `t_*.c` files in this directory
  2. Compile each one into its corresponding `t_*` executable
  3. Execute each test binary and report the results n the `test.log` file

* **Clean up**

  ```sh
  make clean
  ```

  This will delete:

  * All object files
  * All test executables
  * Any log files (`*.log`)

---

### Adding New Tests

1. Create a new C source file in this directory named `t_<feature>.c`.
2. Implement your tests in that file.
3. The provided `makefile` will pick up your new file automatically.
4. Run your test:

   ```sh
   make t_<feature>
   ./t_<feature>
   ```

---

## Windows

The library has been tested to work with three compilers:

### Mingw64 & MSYS2

To use this `gcc` port to windows, just follow the same steps as Linux

### Microsft C (cl)

To test with `cl`, open the Developer's environment from Visual Studio 22 (CMD or PowerShell) and:
  - move into the `microsoft_cl` directory
  - execute the `runtest.bat` script
  - the tests will be compiled and executed, the results will be in the `test.log` file.

### Pelles C (pocc)

Open the "Pelles C Command Prompt" that comes installed with the Pelles C IDE and:
  - move into the `pelles_c` directory
  - execute the `runtest.bat` script
  - the tests will be compiled and executed, the results will be in the `test.log` file.

