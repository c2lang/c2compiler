## Struct / Union types

### Naming

User Type names _must_ always start with a capital letter (A-Z) and be less than 32 characters
in length. Valid non-start characters are: [A-Z a-z 0-9 _]

### Syntax

```c
<public> type [Name] struct/union <attributes> {
    [members]
}
```

### Members / Sub-structs

Member syntax for sub-structs is:
```c
struct/union <name> {
    [members]
}
```

Member syntax for non-substructs is:
```c
[type specification] [name] ;
```

Rules:

- member types may not be _const_
- member names adhere to VarDecl naming rules
- substructs may be anonymous (no name)
- if substructs are named, the name has the same rules as other members
- member names must be unique, including member names of anonymous sub-structs

### Attributes

Structs/Unions can have the following _attributes_:

- _export_
- _packed_ (structs only)
- _unused_
- _aligned_
- _opaque_

### Examples

```c
public type Point struct @(packed, aligned=8) {
    i32 x;
    i32 y;
}

type Outer struct {
    struct inner {
        i32 x;
    }
    Other o;
    union {
        void* p;
        u32* u;
        bool* b;
    }
}
```

