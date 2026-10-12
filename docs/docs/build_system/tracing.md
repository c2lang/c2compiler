## Tracing

The c2c compiler support adding call tracing code natively with minimal
overhead.

To enable tracing for a target run:
```sh
$ c2c <target> --trace-calls -o target_trace
```

This will produce an *instrumented* version in *output/target_trace/*
The tracing can be configured and enabled at runtime by setting the *C2_TRACE*
environment variable:

```sh
C2_TRACE="min=10;min2=1;mode=3;name=*;fd=2" output/target_trace/target_trace 2> log
```

Options for C2_TRACE are separated by semi-colons:

* *min* - the minimal number of total calls to a function (otherwise not printed)
* *min2* - the minimal number of calls from a caller to a function (otherwise not printed)
* *mode* - 1 (print only callees), 2 (print only callers), 3 (print callers and callees)
* *indent* - indentation of callers during printing
* *fd* - the file-descriptor where output is printed on
* *name* - name of functions to be traced (eg my_mod.*)
* *filename* - the filenames of caller locations
* *caller* - the symbol names of callers (eg my_mod.*)

### Example

Tracing all calls to *malloc*, *calloc* and *free*, would be done as follows:

```sh
C2_STRACE="min2=500;mode=3;name=stdlib.malloc,stdlib.calloc,stdlib.free;fd=2" ./output/target_trace/target_trace
```

The output looks something like below and is sorted already.

```
stdlib.free: 9508 calls
  parser/stmt_list.c2:53:28: stdlib.free: 1111 calls from stmt_list.StmtList.free
  analyser/name_vector.c2:35:17: stdlib.free: 641 calls from name_vector.NameVector.free
  ast/function_decl_list.c2:46:13: stdlib.free: 582 calls from ast.FunctionDeclList.add
  ast_utils/string_buffer.c2:60:18: stdlib.free: 549 calls from string_buffer.StringBuf.free
  ast_utils/string_buffer.c2:61:5: stdlib.free: 549 calls from string_buffer.StringBuf.free

stdlib.malloc: 8378 calls
  parser/stmt_list.c2:43:24: stdlib.malloc: 1511 calls from stmt_list.StmtList.add
  ast/function_decl_list.c2:42:24: stdlib.malloc: 990 calls from ast.FunctionDeclList.add
  analyser/name_vector.c2:31:28: stdlib.malloc: 584 calls from name_vector.NameVector.init
  ast_utils/string_buffer.c2:35:16: stdlib.malloc: 549 calls from string_buffer.create
  ast_utils/string_buffer.c2:38:17: stdlib.malloc: 549 calls from string_buffer.create
  ast/expr_list.c2:47:28: stdlib.malloc: 536 calls from ast.ExprList.add
```

