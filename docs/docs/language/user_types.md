# User defined types

Just like C, C2 allows the programmer to define new types.

The syntax to define new types is very standard and uniform: `type <name> <definition>`.
Like all global declarations, they may also be specified as `public`. Note that the `public`
specifier is required when the type is used as a parameter, return type or a member of a public symbol.

The type can define:

* an `enum`
* a `struct`
* a `union`
* an alias for another type
* a `fn`

## Enum types

Enum (enumerated) types use the following syntax:
```c
type State enum i8 {
    Begin = 0,
    Middle,
    End,
}
```

Note that all enum constants are in only available through the enum type's namespace
(eg. State.Begin, not Begin). This allows the names to be much shorter (not STATE_BEGIN,
but just Begin).

When the enum-constant values start at 0 and dont have any gaps, the enum type is called
a __regular enum__.

### Enum init
When *initializing* an enum variable, the enum prefix can be left out:
```c
Color c1 = Color.Red; // allowed
Color c2 = Green; // also allowed, inferred from left-hand side

Color[] cs = { Red, Green, Blue }  // ok
```

Whenever possible the RHS (right-hand side) is inferred from the LHS (Left-hand side).
So even if two different enum types have the same Constant, it still works.
```c
c = Red;
line.draw(Green);
```

### Enum min/max
Enum types can be used as array size specifications for variables:

```c
i32[Color] my_vars;
```

Only __regular enums__ can be used for this.


### Enum min/max
A pattern often seen in C is to add a constant 'Max' at the end to be able to interate.
In C2 every Enum type has automatic *min/max* constants for this:

```c
type Color enum u8 {
    Red=0,
    Green=1,
    Blue=2,
}

fn void test() {
    for (i32 i = Color.min; i <= Color.max; i++) {   // will run from [0, 2]
    }
}
```


### Incremental Enums

Just like [incremental arrays](variables/#incremental-arrays) C2 allows enums to be
incremental. This can be used when the software has a lot of compile-time configurations.
The additional enum constants can even be added from separate files (from the same module).

The syntax looks like:

```c
type Colors enum u8 { + }

Colors += Red;
Colors += Green;
Colors += Blue;
```

Enums must always have at least one constant.


### Enum-associated values

Often it's useful to associate other information to an enum-constant, for example a string name.
The code often ends up like:
```c
fn const char* color2name(Color c) {
    switch (c) {
    case Red:    return "red";
    case Green:  return "green";
    case Blue:   return "blue";
    }
    return "?";
}
```

When adding/removing enum-constants, this code must be kept in sync, which is a pain.

To automatically keep these __associated values__ in sync, C2 allows associated other values
to that enum:

```c
type State enum u8 (const char* name, State next, const bool is_active) {
    Start : { "start", Run,  false },
    Run   : { "run",   Stop, true  },
    Stop  : { "stop",  Done, false },
    Done  : { "done",  Done, false },
}
```

Since the syntax doesn't allow specifying the values of the constanst, enums with associated-values
are always __regular enums__.

Associated values can be const or not.

There are 2 ways to access these associated values:

* by Type
* by Constant

#### By Type
```c
const State* states = State.next;   // use Type here, not instance
State next = states[cur_state];
```

#### By Enum-Constant
```c
State cur_state = Run;
bool is_active = cur_state.is_active;   // use instance here
cur_state.next = Start; // assign to non-const values
```


## Struct types
Structs are defined like so:
```c
type Person struct {
    u8 age;
    char* name;
}
```

A struct's members may be accessed using dot notation:
```c
Person p;
p.age = 21;
p.name - "John Doe";

io.printf("%s is %d years old.", p.age, p.name);
```

Structs may also contain *bit-fields*. These are the same as C.
```c
type MyStruct struct {
    u32 first : 1;
    u32 : 8;
    u32 more : 4;
}
```

Unnamed bit-fields are allowed.


## Union types

Union types are defined just like structs:
```c
type Integral union {
    u8 as_u8;
    u16 as_u16;
    u32 as_u32;
    u64 as_u64;
}
```

Unions, unlike structs, may only have one active member at a given time. See below:
```c
Integral i;
i.as_u8 = 40; // Setting the active member to as_u8

i.as_u32 = 500; // Changing the active member to as_u16

io.printf("%d", i.as_u8); // Undefined behaviour: as_u8 is not the active member, so this will probably print garbage.
```

Note that unions only take up as much space as their largest member, so `sizeof(Integral)` is equivalent to `sizeof(u64)`.

C2 also features (anonymous) sub-structs/unions:
```c
type Person struct {
    u8 age;
    char* name;
    union {
        i32 employee_nr;
        u32 other_nr;
    }
    union subname {
        bool b;
        Callback cb;
    }
```


## Alias types
Alias types are used to give an alias to a different type, like:

```c
type CharPtr char*;
type Numbers i32[10];
```

Instead of the obscure C syntax used to alias a function pointer type, C2 uses the
following:
```c
public type Callback fn void(i32 a, bool b);
```
This defines an alias to function pointer type of a function that returns nothing and requires two
arguments: an `i32` and a `bool`.



## Function types
Function types are used to pass a function as an argument to another function or to store
it in a variable:

```c
type Callback fn i32(i32, void*);
```

A usage example is given below:
```c
// in some function body
Callback cb = my_callback
cb(10, nil);
```

For struct-members that are types, defining an explicit type is not required:
```c
type Callbacks struct {
    fn void (void* arg, i32 a) func1;
    fn void (void* arg) func2;
}
```



