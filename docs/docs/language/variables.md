# Variables

Variable definitions in C2 look a lot like variable definitions in C (on purpose):

```c
i32 counter = 0;
public bool hasBool = false;
Point* p = nil;
```

The __public__ keyword may only be used with `global` variables.

## Multiple variable declarations

Like in C, C2 allows multiple variables to be declared in one line:
```c
i32 a, b, c;
```

However, to avoid the C pitfall of
```
int* a, b;
```
where b would be an 'int' and not an 'int*', defining multiple variables is not allowed
for pointer and array types.


## The `static` keyword

The __static__ keyword has the same meaning as the __static__ keyword in C when used on local
(as in non-global) variables in C; their lifetime is bigger then that of the function.
For example calling the function below 3 times:

```c
fn void increment() {
    static i32 counter = 0;
    counter++;
    printf("%d\n", counter);
}
```
will result in:
```
1
2
3
```

The __static__ keyword may only be used on non-global variables. The 'static' variable is
automatically zero-initialized (like global variables). If explicitly initialized, the
init expression must be a compile-time constant/value.

## The `tlocal` keyword

The __tlocal__ keyword is usede to indicate a variable has __thread local__ storage.


## Initialization

C2 has some convenient variable initialization syntax:

```c
type Data struct {
    i32 a;
    char* text;
    f32 f;
}

Data[] mydata = {
    { 1, "first",  1.11 },
    { 2, "second", 2.22 },
    { 3, "third",  3.33 },
}
```

In C2, all global variables are automatically initialized with a default value
if no explicit initialization is done.

The examples below show some C2 initialization options.

### Array index designators
```c
i32[] array = {
    [10] = 0,
    [11] = 3,
}

// mixing index designators with default (incremental) initialization
i32[4] array2 = {
    0,
    [3] = 3,
    4,          // error: access elements in array initializer
}

// using enum constant as index designator value
type E enum i8 {
    FOO = 2,
    BAR = 5,
}

i32[] array = {
    [E.BAR] = 5,
    0,          // index 6
    [E.FOO] = 2
    3,          // index 3
    4,
    5,          // error: duplicate initialization of array index
}

// using non-compile-time constant as index value is not allowed
i32 a = 1;
const i32 b = 2;

i32 array2 = {
    [a] = 1,    // error: initializer element is not a compile-time constant
    [b] = 2,
}
```

### Field designators
Field designators initialize struct members by name.
```c
// basic struct fields

type Point struct {
    i32 x;
    const u8* name;
}

Point[] array = {
    { 1, "one" },   // basic struct initialization

    { .x = 3, .name = "three" },    // using field designators

    { 4, .name = "four" },  // error: mixing field designator with non-field designators
}
```

Rules:

* Array and Field designators may not be combined to initialize members of a single struct

* Sub-structs may use different initializers from their parent struct

* Union can only be initialized using Field-designators


## Incrementally declared arrays
Incrementally declared arrays are a special feature in C2. These can be used to avoid messy macros when
it is required to have some elements of the array present depending on some external condition (__#ifdef__'ed).

To incrementally declare array, use the `[+]` array subscript in the initial declaration. Entries can then be added from
different points in the code.
```c
type Point struct {
    i32 x;
    i32 y;
}

Point[+] points;

points += { 10, 11 }

// ... other code

points += { 20, 22 }

// ... other code

points += { 30, 31 }
```

Note that incrementally defined arrays can only be used at the __global__ level (not inside functions), as their length must be
known at compile time.

