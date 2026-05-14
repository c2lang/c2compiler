## Build file

A build file is optional, but can used to specify:

* Target-triple (eg. arm-linux-gnueabi-gcc, x86\_64-unknown-linux-gnu)
* Compiler/Build options for the C backend (eg. specify the cross-compiler + flags)
* A list of library search paths
* Change the default output directory


### Specifying a build file
Telling the C2 compiler to use a build file can be done in several ways:

* By default, C2C looks for a file named *build.yaml* in the same directory as the recipe file
* it can be specified on the command-line with the *-b <build_file\>* option


### Format
The build file uses the [YAML format](https://yaml.org) and use
the *.yaml* filename extension.

```yaml
target: "arm-linux-gnueabi"

output_dir: "output_arm"

toolchain:
    cc: "arm-linux-gnueabi-gcc"
    cflags: "-march=armv7-a -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a9"
    ldflags: "-Wl,-O1"

libdir:
    - "$ARM_SYSROOT"

```

All entries are optional, but when specified will override the default values.

#### Value expansion

Instead of specifying a fixed value in the build file, all values that start with a *$* are
instead interpreted as *environment variables*. For example, the *build_generic.yaml* file

```yaml
target: "$TARGET_PREFIX"

output_dir: "output_cross"

toolchain:
    cc: "$CC"
    cflags: "$CFLAGS"
    ldflags: "$LDFLAGS"

libdir:
    - "$C2_LIBDIR"

```

can be used with many different cross-compile toolchains that just export the right
environment variables. This can also be used to re-add the *C2_LIBDIR* environement
variable to the search path.

Please note that the entire value after the *$* is interpreted as the
environment variable name, so the following is NOT allowed:

```yaml
key: "$ENV1 $ENV2"
```


### Library search paths
A build file changes the way C2C searches for libraries. C2C uses the following rules:

* If no build file is used, it **only** uses the *C2\_LIBDIR* environment variable.
* If a build file is used, the *C2\_LIBDIR* environment variable is not used.
* The build file can specify a list of paths that are searched in the given order.

