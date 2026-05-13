
# Switch statement

The switch statement is similar to C, except for the changes below:

    * no auto-fallthrough
    * auto-scoping
    * default (if present) must be last

## Auto-fallthrough

Unexpected fallthrough is a big source of issues in C programs, so in C2
this implicit behaviour has been removed. Any case statement must end in
either: *break* | *fallthrough* | *return* | *continue* | *noreturn-func*.
The fallthrough statement can only appear at the top-level of the case body,
not in some sub-expression (eg. _if (x) { fallthrough; }_ is not allowed).
The only exception to this is for empty case statements, there the default
behaviour is fallthrough.

```c
   switch (i) {
      case 0:  // this gives an error, last statement of a case must be one of
               // break/fallthrough/continue/return.
      case 2:
         fallthrough;    // will fallthrough
      case 3: // implicit fallthrough allowed, since empty case
      case 4:
         break;
      default:
         fallthrough; // not allowed in last case
    }
```

## Case auto-scope

```c
    switch (i) {
        case 1:
            i32 a = 10;     <- in C, user would have to add {} around case body.
            return calc(a);
        case 2:
            i32 a = 20;     <- no clash, since it's another scope
            ...
    }
```

## Default last

Finally, the default statement must be last, to increase code-uniformity.

## Automatic Enum scoping

C2 places enum constants in the Enum type namespace, for example:

```
State s = State.Begin;
State s2 = Begin;   // shorthand, also allowed, but only for Initializations
```

For a switch statement using an enum type, the scope is automatic, so it's possible
to do:

```
fn void demo(State s) {
    switch (s) {
    case Begin:
        break;
    case Middle:
        break;
    case End:
        break;
    }
}
```

So no need to add `State.` to every case.


## Multi-condition case statements

C2 adds a new feature to C that allows multiple conditions to be specified in a single case statement. This lowers
the needs for *fallthrough* and increases code readability. This feature can only be used when switching an *enum* type.

Rules for multi-condition cases:

* Multi-conditions contain one or more single conditions, comma separated
* A single condition can be either an *identifier* or *identifier* .. *identifier*
* Default cannot be combined with other conditions
* Can also be used with incremental enums

Example:

```
type Foo enum u8 { A, B, C, D, E, F, G, H, I }

fn void test(Foo f) {
    switch (f) {
    case A:     // single case
        break;
    case B..C, H:   // range of enums + single
        break;
    case D..F, G, I:    // range + single cases
        break;
    }
}

```

# String switch statement

C2 introduces a new option for the switch statement: the ability to switch on strings.

```c
fn void handleCommand(const char* cmd) {
   switch (cmd) {
   case nil:
      return;
   case "":
    break;
   case "start":
      return;
   case "stop":
   case "move":
       break;
   default:
       return 10;
   }
```

Rules for using switch for strings:

* the case conditions must be: nil or a string literal
* the case strings must be < 255 bytes (including 0)
* there may be no duplicate cases


## Decls

C2 allows declaring a declaration inside a switch condition:
```c
switch (i32 a = getNum()) {
    ...
}
```

