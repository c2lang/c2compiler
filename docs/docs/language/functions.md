# Functions

Functions declarations always start with the `fn` keyword; otherwise they look very similar
to C. Since there are no forward declarations of any kind in C2, there is just one form
of a function declaration, which is the definition:

```c
public fn i32 main(i32 argc, char** argv) {
    return 0;
}
```

Functions may also have attributes. More information on attributes can be found [here](attributes.md).

## Arrays
In C2 arrays cannot be used as function arguments, so pointers must be used instead. This is done
because in C passing 'int numbers[20]' is not a copy, but a pointer to an array, which is confusing
and could lead to bugs.


## Arguments

### Default arguments
Default arguments are also allowed in C2.

```c
fn void test(i32 a = 10, i32 b = 20) {}
```

### Named arguments
Named arguments can be used when calling a function where many arguments are the same type,
and re-ordering arguments would not cause a compilation error. For example:

```c
fn void foo(bool a, bool b, bool c, bool d) { .. }


fn void bar() {
    foo(true, false, true, false);
}
```

In these cases it can be handy to name calling arguments:
    foo(a: true, b: false, c: true, d: false);
```

The *order* of the arguments must still be correct.

C2 allows combining __named__ arguments with __default__ arguments as long as there is
no ambiguity.


### Function pointer arguments

Function pointer types can be defines inline as function arguments:

```c
fn void my_func(void* arg,
        fn i32(void* arg, bool b) a,
        // even nested
        fn void(void* arg, fn bool(i32, i32), i32 a) b,
        i32 c) {
}
```

