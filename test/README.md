# Test Directory

This directory contains all of the unit and integration tests for the project. Each test source file follows the naming convention:

```
t_<name>.c

```

When you run `make t_<name>`, the `t_<name>.c` file is compiled into a corresponding test executable:

```
t_<name>

````

For example, to compile and run the tests in `t_core.c`, execute:

```sh
make t_core
./t_core
````

---

## Prerequisites

* A POSIX‐compatible environment (Linux, macOS, WSL, etc.)
* A C compiler supporting C99 or later (e.g., `gcc`, `clang`)
* GNU Make

---

## Makefile Targets

| Target          | Description                                                     |
| --------------- | --------------------------------------------------------------- |
| `make t_<name>` | Compile the test in `t_<name>.c` into the executable `t_<name>` |
| `make runtest`  | Build **all** `t_*.c` files and run each resulting test binary  |
| `make clean`    | Remove all object files, executables, and temporary files       |

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
  3. Execute each test binary and report the results

* **Clean up**

  ```sh
  make clean
  ```

  This will delete:

  * All object files
  * All test executables
  * Any log files (`*.log`)

---

## Adding New Tests

1. Create a new C source file in this directory named `t_<feature>.c`.
2. Implement your tests in that file.
3. The provided `makefile` will pick up your new file automatically.
4. Run your test:

   ```sh
   make t_<feature>
   ./t_<feature>
   ```

---

## Test Output

Each test executable should return:

* **0** on success (all checks passed)
* **Non-zero** on failure (one or more checks failed)

Make’s `runtest` target will stop and report which test failed if any return a non-zero exit code.

---

