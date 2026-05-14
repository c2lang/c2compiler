# Printf specifiers

One anti-pattern in C is that format-specifiers for printf. These require
a lot of effort when creating multi-platform code (ie 32-bit and 64-bit)

### Base type specifiers
C2 changes the specifiers to allow only the following types:

* `%c` - print character
* `%d` - print decimal number
* `%x`/`%X` - print hexadecimal number
* `%o` - print octal number
* `%f` - print floating-point number
* `%p` - print pointer
* `%s` - print string
* `%%` - print %

The `%d` specifier is used for all integer numbers (__i8__, __i64__, __u8__, __u64__, __bool__, etc). The C2
compiler will automatically use the correct one for the type. Likewise the `%f`
specifier can be used for both __f32__ and __f64__.

Never worry about `%llu`, `%ld` or `%"PRIu64"` again!

### Other options
Next to that, the printf specifier format still allows all other C options like
width, precision and alignment. It supports the full format of:

__%[flags][width][.precision]type__

for example:

```c
printf("%-4s  %06d  %7.3f", text, number, float_number);
```

### Attribute

To allow checking of the format, a function must be marked with the [printf_format attribute](../language/attributes/#printf_format).

