
Based on lua-5.3.4.

The binary was build on Ubuntu 16.04.

To make liblua.a be more portable, a '-fPIC' compile flag needs
to be added; modify the 'src/Makefile' to change the line

```c
MYCFLAGS=
```
into:
```c
MYCFLAGS=-fPIC
```

```bash
make linux
```

