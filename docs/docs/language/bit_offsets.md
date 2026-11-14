# Bit selections

__Bit selection__ are a feature new to C2 — don't confuse them with __bit fields__,
which are struct members that are only *x* bits wide, used to pack data tightly
into memory.

Bit selections instead let you pull a range of bits directly out of an unsigned
integer value, which is convenient in code that works with hardware registers,
wire protocols, or other bit-packed data.

The syntax is `value[<highest bit>:<lowest bit>]`, so the resulting width is
`highest - lowest + 1`. This mirrors how hardware datasheets typically describe
bit ranges within a hardware register.

```c
fn void demo() {
    u32 value = 0x1234;
    u8 a = value[15:8]; // 0x12: the high byte
    u8 b = value[11:4]; // 0x23
    u8 c = value[4:0];  // the lowest 5 bits

    // The statements below are equivalent: C style, then C2 style
    i32 counter1 = (value >> 10) & 0x1F;
    i32 counter2 = value[14:10];
}
```

## Rules

* The base value must have an *unsigned* integer type (`u8`/`u16`/`u32`/`u64`, or
  a `type` alias of one) — signed integers, `bool`, pointers and functions are all
  rejected with `bit selections are only allowed on unsigned integer type`.
* The two indices must themselves be integers; the high index may not be lower
  than the low index (`left selection index is smaller than right index`), and
  neither may be negative or exceed the base value's bit width (`selection index
  value 'N' too large for type 'uN'`).
* A bit selection is a read-only expression: it cannot appear on the left-hand side
  of an assignment (`bit selections cannot be used as left hand side expression`).
* For consistency, the type of a bit selection is the type of the base value, but
  when both indices are compile-time constants, the result range is known and the
  selection expression can be stored directly into a smaller type. When either index
  value is only known at run time, the compiler cannot determine if the value fits
  so an explicit narrowing conversion is needed when assigning to a smaller type:

```c
fn void demo2(u32 value, u8 lo, u8 hi) {
    u8  a = value[15:8];    // OK: width (8 bits) is known at compile time
    u32 b = value[hi:lo];   // OK: all selections fit in destination type
    u8  c = value[hi:lo];   // error: implicit conversion loses integer precision: 'u32' to 'u8'
    u8  d = value[15:4];    // the value has a knwon width of 12 bits so an cast is needed to prevent the
                            // error: implicit conversion loses integer precision: 'u32' to 'u8'
}
```

Implicit narrowing conversions and constant range checks apply here exactly like
they do elsewhere in C2:

```c
u32 value1 = 0xffff;
u8 a = value1[15:0];    // error: implicit conversion loses integer precision: 'u16' to 'u8'

const u32 Value2 = 0x1234;
i8 b = Value2[6:0] + 100;  // error: constant value 152 out-of-bounds for type 'i8', range [-128, 127]
```
