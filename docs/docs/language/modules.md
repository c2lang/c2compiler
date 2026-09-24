# Modules

Like many modern languages, C2 organizes code around __modules__ rather than files.
A module groups related functions, types and variables under a single name. Every
`.c2` file must start with a `module` declaration naming the module it belongs to.
A single module can span several files:

`file1.c2`
```c
module foo;

// ..
```

`file2.c2`
```c
module foo;

// ..
```

`file3.c2`
```c
module bar;

// ..
```

`file1.c2` and `file2.c2` both belong to module `foo`, while `file3.c2` belongs to
module `bar`. The compiler determines this purely from the `module` declaration —
file names and directory layout are irrelevant to where a declaration lives.

Splitting a module across multiple files lets you organize a large amount of
functionality without paying any runtime cost or adding artificial complexity: from
the compiler's point of view, all files of a module are simply concatenated into one
translation unit.

Modules add exactly one layer of namespacing; they are not nested like packages in
some other languages (eg. Java). Nested namespaces turn out not to be necessary — C
managed for decades with only a single global namespace. For large codebases, prefer
descriptive, longer module names instead (eg. `network_utils`) to keep names
meaningful without nesting.

## Importing modules
To use declarations from another module, a file must `import` it. `import` has
**file scope**: it only affects the file it appears in. If `file1.c2` imports
`stdio`, `file2.c2` of the same module still cannot use `stdio`'s symbols unless it
imports `stdio` itself.

`file1.c2`
```c
module foo;

// file and storage imported here
import file;
import storage;
// ..
```

`file2.c2`
```c
module foo;

// file and net imported here, but not storage
import file;
import net;
// ..
```

So `file1.c2` can use symbols from `file` and `storage`, while `file2.c2` can only
use symbols from `file` and `net` — even though both files belong to module `foo`.

A module cannot import itself; `import foo;` inside a file of module `foo` is
rejected with `cannot import own module 'foo'` (own-module symbols are always
visible without an import). Importing the same module twice in one file is also
an error: `duplicate import of module 'x'`.

### Named imports (aliasing)
A module with a long or generic name can be given a shorter, file-local alias with
`as`:

```c
module foo;

import extended_filesystem_io as fs;
```

External symbols are now reached through the `fs` alias. Aliasing is purely a
convenience for the importing file — it does not rename or affect the module
anywhere else.

The alias name must be different from the module's own name (`import foo as foo;`
is rejected as redundant), and, like other module names, must start with a
lowercase letter. Two imports in the same file can never resolve to the same local
name; whichever imports second is rejected with `duplicate import name 'x'`,
whether the clash comes from two aliases, an alias matching another module's real
name, or vice versa.

Once a module has been aliased, only the alias may be used to qualify its symbols
in that file — falling back to the original module name is an error:

```c
import stdio as io;

fn void test() {
    io.puts("hello");     // OK
    stdio.puts("hello");  // error: module 'stdio' is imported with alias 'io'
}
```

### Importing all symbols
When a file uses many symbols from the same module, prefixing every use can get
tedious. Following the import an import-list '{ * }' makes all of that module's
external symbols usable **without** a prefix. This combines with aliasing, making
the prefix entirely optional:

```c
import networking as net { * }
import filesystem { * }

// Equivalent
filesystem.doSomething();
doSomething();

// Equivalent
net.connect();
networking.connect();
connect();
```

Symbols from both `networking` and `filesystem` are now reachable unprefixed. An
unprefixed name only resolves this way when it is unambiguous across the current
module and its set of imports: if both `networking` and `filesystem` declared an
`open()` function, calling `open()` unprefixed would fail with `symbol 'open' is
ambiguous` and both call sites would need their module prefix restored.

### Import lists
Import lists allow cherry-picking of symbols you want to import.
When only a handful of symbols are needed unprefixed, list them explicitly in
braces after the import:

```c
import stdio { printf }

public fn i32 main() {
    printf("Hello world!\n");
    return 0;
}
```

Here only `printf` becomes usable unprefixed. The module is still imported
normally otherwise, so any public symbol of `stdio` — listed or not — remains
reachable with the regular `stdio.xxx` prefix. List several symbols by separating
them with commas, and give any of them its own local alias with `as`:

```c
import stdio { printf, fprintf as fp }
```

An unknown or non-public symbol in the list is a compile error (`module 'stdio' has
no symbol 'xxx'` / `symbol 'x' is not public`), and the same ambiguity rule as
import-list imports applies once a listed symbol is used unprefixed alongside other
imports.

Writing `*` instead of a symbol list imports every symbol unprefixed.

```c
import stdio { * }
```

As with any import, each name — the import itself, or an individual entry in its
list — must actually be used somewhere in the file, otherwise the compiler warns
about it (eg. `unused import 'stdio'`, or `unused import 'stdio.scanf'` for one
unused entry in an otherwise-used list).

### Why this matters
The combination of __modules__ and `import .. (as ..) ({ .. })` means C2
code never has to constantly re-prefix the same declarations, while still letting
each file choose how verbose it wants to be. In C, libraries commonly prefix every
symbol themselves to avoid clashes, e.g.:

`networking.h`
```c
void net_open();

void net_close();

struct net_data {
  ..
};
```

In C2 this is simply:
```c
module networking;

public fn void open() { .. }

public fn void close() { .. }

public type NetData struct {
    ..
}

```
Callers then pick whichever prefix suits them — the module's own name, a shorter
alias (`import networking as net;`), or no prefix at all (an import
list) — without the module itself needing to know or care.

One further advantage over C-style header includes: filenames never appear in the
code. Renaming or moving the files that make up a module requires **no** change to
any code that imports it, since imports refer to the module name, not a path.

## Symbol visibility

All files belonging to the same module see every declaration in that module,
public or not. Files in *other* modules can only see declarations explicitly
marked `public`.

`file.c2`
```c
module foo;

public fn void init() { .. }

fn void open() { .. }
```

Other modules can call `init()` once they import `foo`, but only files within
module `foo` itself can call `open()`, since it isn't `public`.

## Symbol resolution
Symbols from an imported module are normally reached with dot notation:
`module.symbol`.

```c
module foo;

import one;
import two;

fn void test() {
    one.test();
    two.test();
    open();
}
```

Global symbol names __must__ be unique within their own module, whether `public`
or not — this guarantees `module.symbol` always names exactly one declaration. In
the example above, `one`, `two` and `foo` itself can each declare a `test` symbol
without clashing, because the module prefix (or the enclosing module, for `open`)
always disambiguates which one is meant.

When an import list makes a symbol reachable unprefixed, this uniqueness guarantee
only holds per module — so an unprefixed reference is accepted only if exactly one
visible import list, `local` import, or the current module contributes that name.
