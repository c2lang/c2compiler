## Attributes

C2 incorporates standardized __attributes__. This means that the *syntax* of attributes is
defined, but attribute types can be added (by plugins, etc). There can also be compiler-specific
attributes to do all sorts of funky things the compilers do.

The currently supported attributes are:

* __aligned__ (type, fn, var), requires a numeric argument
* __auto_expr__ (parameter)
* __auto_file__ (parameter)
* __auto_func__ (parameter)
* __auto_line__ (parameter)
* __cname__ (type, fn, var), interface
* __cdef__ (type, fn, var), only in interface files, string argument
* __constructor__ (fn)
* __deprecated__ (fn), requires a string argument
* __destructor__ (fn)
* __embed__ (var)
* __export__ (type, fn, var)
* __inline__ (fn)
* __noreturn__ (fn)
* __no_typedef__ (interface struct/union types)
* __opaque__ (public struct/union types)
* __packed__ (type)
* __printf_format__ (parameter)
* __pure__ (fn)
* __scanf_format__ (parameter)
* __section__ (fn, var), requires a string argument
* __unused__ (type, fn, var)
* __unused_params__ (fn)
* __weak__ (fn, var)

The standard syntax for all attributes is `@(  )`  (Hint: the @ (at) is for attributes... ;) )

Take a look at the following example showing their usage in various declarations:

```c
// variables
i32 counter @(unused);
i32[1024] bigdata @(section="data") = {};

// types
type Point struct @(packed, aligned=16) {
    i32 x;
    i32 y;
}

type Weird enum u32 @(unused) {
    FOO,
    BAR,
    FAA
}

// functions
public fn void init() @(export) {
    // ..
}
```

`NOTE: compiler-specific attributes will be required to start with an underscore,
like _c2_my_attribute_, so other compilers can recognize and ignore them`

### Embed attribute

The __embed__ attribute is used to embed external files into a variable:

```c
const char[] Data @(embed="data.txt");
```

The path is relative from the project root. The data will be 0-terminatated.


### Printf_format

__printf_format__ is C2's equivalent of C's
```__attribute__((format(printf, fmt_index, first_to_check)))```. Where C needs
you to count parameter positions (and keep that count in sync as the signature
changes), C2's attribute is simply placed directly on the parameter that holds
the format string:

```c
fn void log(const char* format @(printf_format), ...) {
    // ..
}
```

Any call to this function then has its format string checked against the
variadic arguments passed to it, giving errors like:

```c
fn void test() {
    log("%s", 10);  // error: format '%s' expects a string argument
}
```

See also [Printf specifiers](printf_specifiers.md).

### Scanf_format

__scanf_format__ works exactly like __printf_format__, but checks its
variadic arguments the way `scanf`-style functions expect: as pointers to
write into, rather than values to format.

```c
fn i32 my_scanf(const char* format @(scanf_format), ...) {
    // ..
}

fn void test(i32* value) {
    my_scanf("%d", value);   // OK
    my_scanf("%d", *value);  // error: conversion '%d' expects an integer
                              // pointer argument
}
```


### Opaque pointers

The __opaque__ attribute deserves some special attention. It is used to implement
the *opaque pointer* pattern in C2. See the Wikipedia article
[Opaque Pointer](https://en.wikipedia.org/wiki/Opaque_pointer) for more background info.

In short, opaque pointers are used to hide the implementation while giving the users
a *typed handle* to
pass to your library, maintaining type safety. The __opaque__ attribute can only
be used on *public struct/union types* and tells the compiler that *other*
modules can only use that type *by pointer* and are not allowed to dereference it.

```c
public type Handle struct @(opaque) {
    ..   // members are not visible outside of the module
}
```

When c2c generates an *interface file* (eg. module.c2i), it will only generate:
```c
type Handle struct @(opaque) {}
```

Note that it is allowed to put other non-public types as full members inside
a public opaque struct, since the members are not visible outside the module.


### Cname / No\_typedef
Some legacy C types/functions don't map really well to the C2 style. An example
of this is _stat.h_:

```c
struct stat {
    // ...
};

int stat(const char* pathname, struct stat* statbuf);
```

So both the struct and the function are called _stat_.

To solve this situation and offer a nice way to embed these calls into a C2 application,
C2 offers the attributes *cname* and *no_typedef*. In the C2 version of sys\_stat.h:

```c
type Stat struct @(cname="stat", no_typedef) {
    // ...
}

fn int stat(const char* pathname, Stat* buf);
```

This means C2 code can use 'Stat' instead of 'struct stat', so the spelling conventions
stay intact (types start with capital case). Also for the C backend, we cannot generate:
```c
typedef struct stat_ stat;

struct stat_ {
    // ...
};
```
...since that would clash with the function `stat`. So the attribute *no_typedef* tells c2c not
to generate the typedef, instead simply:
```c
struct stat {
    // ...
};
```

### Auto-arguments

There are four attributes for function parameters: *auto_file*, *auto_line*,
*auto_func* and *auto_expr*. What's special about them is that a parameter carrying one of these
attributes gets _auto-filled_ at every call site, instead of the caller having to
pass it explicitly — hence *auto-arguments*. They exist so C2 code never needs
macros to get at `__FILE__`, `__LINE__` and `__func__`.  *auto_expr* can be used in place of
the stringization operator used in C to convert expressions to string literals.

Example:
```c
fn void log(const char* file @(auto_file), u32 line @(auto_line),
            const char* func @(auto_func), const char* fmt, ...) {
    // ...
}

fn void test() {
    log("%p %d", nil, 10);  // <- file, line and func parameters are auto filled
}
```

#### Rules

- Auto-arguments come after the self-pointer for type-functions
- Auto-arguments come before other arguments
- The type for _auto\_file_, _auto\_func_ and _auto\_expr_ needs to be _const char*_
- The type for _auto\_line_ needs to be _u32_
- The filename that is generated is *project relative* (no more /home/bas/project_x/..)
- _auto\_func_ is filled with the name of the calling function
- Auto-arguments cannot be used with _pure_ functions
- Auto-arguments can be used with _template_ functions
- Auto-arguments can be used in the type-definition of Function type
- Auto-arguments cannot be used in functions used as Function pointers (see example below)


Example with type-function:
```c
fn void Foo.log(Foo* f, u32 line @(auto_line), void* ptr) {
    // ...
}

fn void test(Foo* f) {
    f.log(nil); // 'translates' into Foo.log(f, 123, nil);
    Foo.log(f, nil); // equivalent to the call above
}
```

Function pointer example:

```c
type Callback fn void (const char* file @(auto_file), u32 line @(auto_line), void* arg);

fn void test1(const char* file, u32 line, void* arg) {
    // ...
}
Callback f1 = test1; // ok

fn void test2(const char* file @(auto_file), u32 line @(auto_line), void* arg) {
    // ...
}
Callback f2 = test2; // error: test2 cannot have auto-arguments
```

Example for the _auto\_expr_ attribute:
```c
fn void trace(const char* s @(auto_expr), u64 e) {
    printf("%s -> %u\n", s, e);
}

fn void test() {
    trace(1 + 1); // outputs "1 + 1 -> 2"
}
```

#### Unit test framework ####
In combination with the *unit_test* plugin, _auto-arguments_ can be used to implement a unit test framework.
See the example code archive for a full implementation.

