# Loops + Flow control

C2 has the same loop constructs as C, namely: for, while and do..while.

## For

```c
for (u32 i=0; i<10; i++) printf("%d\n", i);
```

## While

```c
while (i<10) i++;

while (Point* p = get_point()) { .. }
```

Note that C2 does allow a declaration to be used as condition (see example above)


## Do-while

Do-while statements have been removed from C2, since they were very error prone
and often only used for macro expansions.


## If

If statements are exactly the same as C.

```c
if (x < 10 && y >= 10 && ptr != nil) { .. }
```

It can be used with and without braces just like in C.

## Label / Goto

Labels and goto exist just like in C.

```c
start:
    i++;
    if (i < 10) goto start;
```

Labels *must* start with a lower-case character.

## Labelled break/continue

To jump out of or continue an outer loop __labelled break/continue__ exists:
```c
outer: for (i32 i=0; i<100; i++) {
  inner: for (i32 j=0; j<100; j++) {
      if (..) break outer;
      if (..) continue outer;
  }
}
```

Note that these labels can also be used for regular goto statements.


