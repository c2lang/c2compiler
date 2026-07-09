
## Builtin functions

C2 has the following built-ins:

* `sizeof`
* `countof`
* `offsetof`
* `to_container`
* `assert`
* `cast<T>(..)`

### sizeof
`sizeof` is the same as in C: it returns a `u32` with the size of the type/variable.

```c
u32 size = sizeof(void*);
```

C2 does not allow the use of *[ ]* inside a sizeof expression, since this introduces
ambiguity between _arrays_ and _subscript expressions_.

### countof
For array types, C2 introduces a new operator, namely `countof`. This returns the number
of elements in an array or the number of constants in an enum and avoids C macros like:
```c
#define ARRAY_SIZE(x) ( sizeof(x) / sizeof(x[0]) )
```

So

```c
char[15] name;
const u32 Len = countof(name);   // will be 15

u32[10][20] multi;
const u32 Cols = countof(multi); // will be 10
const u32 Rows = countof(multi[0]); // will be 20

type Colors enum u8 { Red, Green, Blue }
const u32 Count = countof(Colors); // will be 3

```
Note that this also works for [incrementally declared arrays](variables.md#incrementally-declared-arrays).

The `countof` function can also be used on Enum types, to return the number of elements
in the Enum.


### offsetof
`offsetof` is the same as in C, only it comes with the language (no need to include something).
The syntax is offsetof(type, member). For example:

```c
type Struct struct {
    i32 a;
    struct sub {
        i32 b;
    }
}

u32 o1 = offsetof(Struct, a);
u32 o2 = offsetof(Struct, sub.b);
```

### to_container

`to_container` replaces a much used C macro, namely:
```c
#define to_container(type, member, ptr) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
```

Essentially, this macro converts a pointer to a member of a struct to
a pointer of the struct itself. Examples where this macro is used is the Linux kernel
linked list mechanism. This works by embedded a 'List' node inside the Element you want
in the list. The list then points to this member. When iterating, you need to
convert the list member to the containing struct.

The syntax is `to_container`(type, member, pointer), where:

- Type is a struct/union type
- member is a member of that struct/union
- pointer is a pointer to that member (and of that same type)

For example:

```c
type Struct struct {
    i32 a;
    i32 b;
}

fn void example(i32* i) {
    Struct* s = to_container(Struct, b, i); // so i points to member b
}

fn void example(const i32* i) {
    const Struct* s = to_container(Struct, b, i); // const status is maintained
}
```

Unlike the macro version, the _const_ status of the original pointer is maintained.


### assert

C2 has builtin assert functionality. It looks similar to C, namely:

```c
fn void test1() {
    assert(2 == 2);
}
```

But this doesn't use the macro preprocessor. Asserts are enabled by default, but can be disabled
in the [recipe](../build_system/recipe_file.md) with the *$disable-asserts* option. Note that they are always parsed, to avoid getting
errors when you do re-enable them after some time.

### cast

A new cast-syntax was added to make casts more explicit and enforce the fact that cast is a
unary operator. The C-style cast is also allowed.

Please note that in C (and C2), the cast is a *unary* operator.

```c
   // new style
   i32 a = cast<i32>(b);
   Foo* f = cast<Foo*>(bar);

   // old style
   i32 c = (i32)b;
   Foo* g = (Foo*)bar;
```

