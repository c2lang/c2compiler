
## Primitive types

C2 has the following built-in primitive types:

* `bool`: either `true` or `false`.
* `i8`, `i16`, `i32`, `i64`: signed integral types.
* `u8`, `u16`, `u32`, `u64`: unsigned integral types.
* `isize`, `usize`: architecture dependent, either i32/i64 or u32/u64.
* `f32`, `f64`: single and double precision floating point types, respectively.
* `reg8`, `reg16`, `reg32`, `reg64`: register types (volatile, unsigned).
* `void`: Same as in C.

For convenience, the __char__ keyword is also available and is identical to the __u8__ type.
The idea is to only use this for strings.

Note that C2 does __not__ have any type specifiers like __signed__, __unsigned__, __long__ or __short__.

### Built-in types fields ###
Built-in primitive types have fields for easy access to type properties such as minimum and maximum value, bit width and for floating point types NoN and Infinity.

Each integral type has a minimum value, a maximum value and a bit width:

* `char.min` is 0, `char.max` is 255, `char.width` is 8;
* `i8.min` is -127, `i8.max` is 128, `i8.width` is 8;
* `i16.min` is -32767, `i16.max` is 32768, `i16.width` is 16;
* `i32.min` is -2147483647, `i32.max` is 2147483648, `i32.width` is 32;
* `i64.min` is -9223372036854775807, `i64.max` is 9223372036854775808, `i64.width` is 64;
* `u8.min` is 0, `u8.max` is 255, `u8.width` is 8;
* `u16.min` is 0, `u16.max` is 65535, `u16.width` is 16;
* `u32.min` is 0, `u32.max` is 4294967295, `u32.width` is 32;
* `u64.min` is 0, `u64.max` is 18446744073709551615, `u64.width` is 64;
* `isize.min`, `isize.max` and `isize.width` depend on the target architecture;
* same for `usize.min`, `usize.max` and `usize.width`


Each floating point type has a minimum value, a maximum value and a bit width and representations of NaN and Infinity:

* `f32.min` is -3.40282347E+38F, `f32.max` is 3.40282347E+38F, `f32.width` is 32;
* `f64.min` is -1.7976931348623157E+308, `f64.max` is 1.7976931348623157E+308, `f64.width` is 64;
* `f32.nan` is a NaN (not a value) of type `f32`
* `f64.nan` is a NaN (not a value) of type `f64`
* `f32.inf` and `f64.inf` represent infinity in their respective type.

```c
module foo;

i32 highest = i32.max;
```

## Pointer types

Pointer types are created by adding an asterix (`*`) after the type they refer to, like

```c
void* a;
i8* b;
Point* c;
char** d; // Pointer to a pointer to a char
```

The `nil` keyword may be used to assign a null value to a pointer:
```c
char* name = nil;
```

## Array types

Arrays in C2 differ from C arrays in that `[]` always comes right after the element type, e.g:

```c
void*[]  a;
Point[4] b;
```

For array types, C2 introduces a new operator, namely [elemsof](../language/builtin_functions/#elemsof) This returns the number
of elements in an array and avoids C macros like:
```c
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))
```
The `sizeof()` operator is also still available.

