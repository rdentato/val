# `val` C Library: NaNboxing Made Accessible in C

Welcome to `val`, a C library designed to bring the wonders of NaNboxing to the fingertips of C programmers, enabling the efficient storage of multiple data types in a single variable.

## 🧰 What is NaNboxing?

NaNboxing is a technique that leverages the NaN (Not a Number) space in IEEE 754 floating-point numbers to encode and represent various data types, such as integers, pointers, and floating-point numbers, in a single 64-bit float. This approach has been vital for languages like JavaScript, providing them with a way to optimize memory usage and enhance performance. `val` aims to bestow these advantages upon C, a language that doesn’t natively offer NaNboxing.

## 🚀 Features

- **Versatile Storage:** Handle signed/unsigned integers (up to 48 bits), double-precision floats, generic pointers, and string pointers all within a single `val_t` variable.
- **Type Discovery:** Identify stored data types effortlessly.
- **Smooth Conversion:** Retrieve your original data seamlessly with type conversion functions.
- **Helpful Constants:** Utilize predefined constants for common values like true, false, and nil.

## 💻 Usage

### Storing a Value

Store various types of data using the `val()` function. Example:

```c
val_t f = val(3.2);          // Stores a double
val_t s = val("a string");   // Stores a pointer to a string
```

### Retrieving Values

Retrieve stored data by converting `val_t` variable using the appropriate function:

```c
double num = valtodouble(f);
char * str = valtostring(s);
```

### Type Checking

Check the type of data stored in `val_t` using:

```c
int type = valtype(f);
```

Or utilize the provided helper functions for context-specific checks:

```c
valisinteger(x);
valisdouble(x);
// ... and more
```

## 🧪 Constants

The library defines several handy constants of type `val_t`:

- `valfalse`
- `valtrue`
- `valnil`
- `valnilpointer`
- `valnilstr`

## 📚 Documentation

For a detailed guide on how to integrate and utilize `val` in your C projects, please refer to the [documentation](docs/).

## 🔧 Installation & Building

Clone the repository and use your preferred method to integrate the `val` library into your project. 

```shell
git clone https://github.com/[username]/val.git
```

## 🙋‍♂️ Contributing

Contributions, issues, and feature requests are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for how to get started.

## 📜 License

Distributed under the MIT License. See [LICENSE](LICENSE) for more information.

## 🤝 Acknowledgments

- JavaScript engines like V8, SpiderMonkey, and JavaScriptCore for inspiring NaNboxing implementation.
```
Note: Make sure to replace placeholders like `[username]` with actual GitHub usernames or relevant links. Similarly, if additional documents like `CONTRIBUTING.md` or `docs/` directory are mentioned, ensure they are available in the repository or adjust the README.md accordingly.
