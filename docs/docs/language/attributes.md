## Attributes

C2 incorporates standardized __attributes__. This means that the *syntax* of attributes is
defined, but attribute types can be added (by plugins, etc). There can also be compiler-specific
attributes to do all sorts of funky things the compilers do.

The currently supported attributes are:

* __aligned__ (type, fn, var), requires number argument
* __ auto_file__ (parameter)
* __ auto_func__ (parameter)
* __ auto_line__ (parameter)
* __cname__ (type, fn, var), interface
* __ cdef__ (type, fn, var), only in interface files, string argument
* __constructor__ (fn)
* __deprecated__ (fn), requires string argument
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
* __section__ (fn, var), requires string argument
* __unused_params__ (fn)
* __unused__ (type, fn, var)
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

Printf_format is the C2 equivalent of C:
```__attribute__((format=(printf, 1, 2)));``` and is used like:

```c
fn void log(const char* format @(printf_format), ...) {
 // ..
}

```

Where the argument points to the (1-based) index of the format argument.

Any call to this function can than have its format checked and possibly give errors like:

```c
fn void test() {
    log("%s", 10);  // error: "format '%s' expects a string argument"
              ^
}

```

See also [Printf specifiers](../language/printf_specifiers/)


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

int stat(const char *pathname, struct stat *statbuf);
```

So both the struct and the function are called _stat_.

To solve this situation and offer a nice way to embed these calls into a C2 application,
C2 offers the attributes *cname* and *no_typedef*. In the C2 version of sys\_stat.h:

```c
type Stat struct @(cname="stat", no_typedef) {
    // ...
}

fn c_int stat(const c_char* pathname, Stat* buf);
```

This means C2 code can use 'Stat' instead of 'struct stat', so the spelling conventions
stay intact (types start with capital case). Also for the C-backend, we cannot generate:
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

There are two attributes for function parameters: *auto_file* and *auto_line*.
These are special is that you define them with a function parameter and that causes
them to be _auto-filled_ when a call to that function is made and are called *auto-arguments*.
*Auto-arguments* are part of the work to avoid using macros to do \_\_FILE\_\_, \_\_LINE\_\_ and
\_\_FUNC\_\_.

Example:
```c
fn void log(const char* file @(auto_file), u32 line @(auto_line), const char* fmt, ...) {
    // ...
}

fn void test() {
    log("%p %d", nil, 10);  // <- file + line parameters are auto filled
}
```

#### Rules

- Auto-arguments come after the self-pointer for type-functions
- Auto-arguments come before other arguments
- The type for _auto\_file_ needs to be _const char*_
- The type for _auto\_line_ needs to be _u32_
- The filename that is generated is *project relative* (no more /home/bas/project_x/..)
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

#### Unit test framework ####
In combination with the *unit_test* plugin, _auto-arguments_ can be used to implement a unit test framework.
See the example code archive for a full implementation.

