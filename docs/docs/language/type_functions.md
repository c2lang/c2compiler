# Type functions
__Type functions__ are another new feature in C2.

Type functions are _syntactic-sugar_ that makes code more readable.

The example below shows how it works:

```c
type Point struct {
    i32 x;
    i32 y;
}

fn void Point.add(Point* p, i32 x) {
    p.x = x;
}

fn void example() {
    Point p = { 1, 2 }

    // with type-functions
    p.add(10);

    // Non-sugared call
    Point.add(&p, 10);
}
```

## Rules
* Type functions only work on struct/union/enum types
* Type functions are defined in the same module as the struct (not necessarily the same file!)
* For a type type named `Foo`, type-functions must start with `Foo.` prefix.
* There cannot be a struct member `x` and a type-function `Foo.x`.
* For struct/union: a (non-static) type-function is required to have '(const) Type\*' as its first argument.
* For enum: a (non-static) type-function is required to have '(const) Type\*' or '(const) Type' as its first argument.
* A static type-function has no argument requirements and can only be called on the type itself,
    not an instance of the type. `Foo.add()` and `f.add()` cannot both exist.
* A type-function may also be assigned to variables of type Function with the correct prototype:
    _callback = Foo.add;_
* Sub-structs cannot have type-functions


Extra notes:

* It's possible to have public or non-public type-functions for a public struct.
* A type-function itself is a regular function and can also be used as such
* It's also possible to use a type-function call if the variable of struct type is a member of
    another struct
```c
type Outer struct {
    Inner inner;
}

type Inner struct {
    // ...
}

fn void Inner.modify(Inner* inner, /* ... */) {
    // ...
}

fn void example() {
    Outer outer;
    outer.inner.modify(/* ... */);
    // translates to Inner.modify(&outer.inner);
}
```

* Calling through a pointer works the same way as calling through a value — the
  compiler inserts whichever of `&`/`*` is needed to match the type-function's
  declared receiver, per the [Auto conversions](#auto-conversions) table below:
```c
    Type* t = get_type();
    t.init();   // fine, whether Type.init takes 'Type*' or 'Type'
```

for more examples, see the tests in _c2compiler/test/functions/struct_functions/_


### Auto conversions

Calling a type-functions in the short form (eg. f.init()) can auto-convert f from Type -> Type* or vice versa.

Examples:
```c

fn void Foo.func1(Foo f) { ... }

fn void Foo.func2(Foo* f) { ... }

fn void test(Foo f) {
    f.func1();  // no conversion needed
    f.func2();  // auto-converted Foo -> Foo*
}

fn void test(Foo* f) {
    f.func1();  // auto-converted Foo* -> Foo
    f.func2();  // no conversion needed
}
```

Rules

| Function \ Arg       | Foo   | Foo*  | const Foo | const Foo* |
|----------------------|-------|-------|-----------|------------|
| Foo.func()           | error | error | error     | error      |
| Foo.func(Foo*)       | auto  |  ok   | error     | error      |
| Foo.func(const Foo*) | auto  |  ok   | auto      | ok         |
| Foo.func(Foo)        | ok    | auto  |  ok       |     auto   |
| Foo.func(const Foo)  | ok    | auto  |  ok       |     auto   |

### Init calls

Type-function can also be used in a variable declaration, so combine
the _definition_ and _initialization_ in one line:

```c
fn void test1() {
    Foo f.init(1);
}
```

which is equivalent to:
```c
fn void test1() {
    Foo f;
    f.init(1);
}
```

The called function should be a regular type function with `Foo*` as first argument.
This concept is called *init calls* and cannot be used when a declaration
is used inside a *condition*, eg

```c
   while (Foo f.init(1)) {} // ERROR not allowed
```


### Enum example

```c
type Color enum u8 { Red, Green, Blue }

fn const char* Color.str(Color c) {
    switch (c) {
    case Red:   return "Red";
    case Green: return "Green";
    case Blue:  return "Blue";
    }
    return "";
}

fn void test1(Color c) {
    printf("color = %s\n", c.str());
}
```


### Complex example

See below a more complex example, which uses opaque pointers. The module `inner` offers an API:

```c
module inner;

import stdlib;
import stdio;

public type Shape struct @(opaque) {
    u8 sides;
    // ..
}

// a non-public type-function
fn void Shape.init(Shape* shape, u8 sides) {
   shape.sides = sides;
}

// a static type-function, called as Shape.create(..)
public fn Shape* Shape.create(u8 sides) {
    Shape* shape = stdlib.malloc(sizeof(Shape));
    shape.init(sides);
    return shape;
}

// a public, const type-function, first argument: const Shape*
public fn void Shape.print(const Shape* shape) {
    stdio.printf("shape with %d sides\n", shape.sides);
}

// a public, type-function, first argument: Shape*
public fn void Shape.free(Shape* shape) {
    stdlib.free(shape);
}
```

which is used by the module `outer`:

```c
module outer;

import inner { * }

fn void example() {
    Shape* s = Shape.create(3);
    s.print();
    s.free();
}
```

_NOTE_: `outer` is not allowed to access Shape's regular members directly.

