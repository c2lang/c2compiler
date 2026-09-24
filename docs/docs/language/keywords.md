## Keywords

C2 has the following keywords:

### Module related

* `module`
* `import`
* `as`
* `public`

### Type related

* `bool`
* `char`
* `const`
* `elemsof`
* `enum`
* `f32`
* `f64`
* `false`
* `fn`
* `i8`
* `i16`
* `i32`
* `i64`
* `isize`
* `nil`
* `offsetof`
* `reg8`
* `reg16`
* `reg32`
* `reg64`
* `sizeof`
* `static`
* `struct`
* `template`
* `tlocal`
* `to_container`
* `true`
* `type`
* `u8`
* `u16`
* `u32`
* `u64`
* `union`
* `usize`
* `void`
* `volatile`

### Control flow related

* `break`
* `case`
* `continue`
* `default`
* `else`
* `fallthrough`
* `for`
* `goto`
* `if`
* `return`
* `switch`
* `while`

### Other

* `asm`
* `assert`
* `static_assert`

### Feature selection

* `#if`
* `#ifdef`
* `#ifndef`
* `#elif`
* `#else`
* `#endif`
* `#error`
* `#warning`

C2 has no C-style macros or `#include`, but it does keep a small, tokenizer-level
mechanism for conditional compilation, using the directives above. Instead of
testing macro definitions, `#ifdef`/`#ifndef` test named *features* — boolean-ish
flags that a build target enables with `$config <name>` in its recipe (see
[Recipe file](../build_system/recipe_file.md)) or the user passes on the command
line as `-Dname`, plus a few c2c defines automatically: `ARCH_32BIT`/`ARCH_64BIT`,
`LONG_32BIT`/`LONG_64BIT`, `BOOTSTRAP` (while building the bootstrap compiler), and
`__ASAN__`/`__MSAN__`/`__UBSAN__` when the corresponding sanitizer is enabled.
`#if`/`#elif` take a small integer expression instead (`0`, `1`, comparisons,
`&&`/`||`, etc.), and `#error`/`#warning` emit a diagnostic with the rest of the
line as its message:

```c
#if 0
const u32 Limit = 1;
#elif 0
const u32 Limit = 2;
#else
const u32 Limit = 3;
#endif

#ifdef ARCH_64BIT
const usize WordSize = 8;
#else
const usize WordSize = 4;
#endif

// enabled through `$config ExtraLogging` in the recipe, or `-DExtraLogging`
#ifdef ExtraLogging
#warning building with ExtraLogging enabled
#endif
```

A disabled branch (like the `#if 0` one above) is skipped entirely: c2c only scans
it for nested `#if`/`#elif`/`#else`/`#endif` structure, so it doesn't need to be
valid C2 at all.

## C keywords

In addition to the C2 keywords, there are a number of C keywords that can be used in interface files with their C semantics:

* `double`
* `extern`
* `float`
* `int`
* `long`
* `short`
* `signed`
* `size_t`
* `ssize_t`
* `typedef`
* `unsigned`

Other C keywords are reserved to avoid compatibility problems:

* `alignas`
* `alignof`
* `auto`
* `constexpr`
* `do`
* `inline`
* `nullptr`
* `register`
* `restrict`
* `thread_local`
* `typeof`
* `typeof_unqual`
