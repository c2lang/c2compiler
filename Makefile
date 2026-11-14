C2C:=output/c2c/c2c

C2FLAGS=
ifdef ASAN
C2FLAGS+= --asan
endif
ifdef MSAN
C2FLAGS+= --msan
endif
ifdef UBSAN
C2FLAGS+= --ubsan
endif
ifdef DEBUG
# should be --debug
C2FLAGS+= --fast
endif
ifdef CONFIG_32BIT
MAKEFLAGS+= CONFIG_32BIT=y
endif
QUIET=--quiet

C2C_DIRS:= {analyser,analyser_utils,ast,ast_utils,common,compiler,generator,ir,libs,parser}
C2C_DEPS:= $(wildcard $(C2C_DIRS)/*.c2* $(C2C_DIRS)/*/*.c2*) recipe.txt Makefile

PLUGINS= deps_generator refs_generator git_version unit_test shell_cmd

all: $(C2C)~

c2c: $(C2C)~
	@$(C2C) --version

$(C2C)~: output/bootstrap/bootstrap $(C2C_DEPS)
	@echo "---- running bootstrap c2c ----"
	@output/bootstrap/bootstrap c2c --fast $(QUIET) --noplugins
	@rm -rf output/bootstrap/cgen
	@mv -f output/c2c/* output/bootstrap
	@echo "---- running c2c --noplugins $(C2FLAGS)) ----"
	@output/bootstrap/c2c $(C2FLAGS) --noplugins --fast $(QUIET) c2c $(PLUGINS)
	@rm -rf output/c2c_fast
	@mv output/c2c output/c2c_fast
	@./install_plugins.sh
	@echo "---- running c2c (optimized with plugins$(C2FLAGS)) ----"
	@output/c2c_fast/c2c $(C2FLAGS) $(QUIET)
	@./install_plugins.sh

output/bootstrap/bootstrap:
	@$(MAKE) -B -C bootstrap $(MAKEFLAGS)

san:;	@$(MAKE) -B ASAN=1 UBSAN=1 $(MAKEFLAGS)
asan:;	@$(MAKE) -B ASAN=1 $(MAKEFLAGS)
msan:;	@$(MAKE) -B MSAN=1 $(MAKEFLAGS)
ubsan:;	@$(MAKE) -B UBSAN=1 $(MAKEFLAGS)
debug:;	@$(MAKE) -B DEBUG=1 $(MAKEFLAGS)

output/tester/tester: tools/tester/*.c2 $(C2C_DEPS) $(C2C)
	@$(C2C) $(C2FLAGS) $(QUIET) tester

test-bootstrap: $(C2C)
	@$(C2C) c2c -o c2c_bootstrap --bootstrap $(QUIET)

rebuild-bootstrap: $(C2C)
	@echo "generating bootstrap files for various systems/architectures"
	$(C2C) c2c -b bootstrap/build-linux-x86_64.yaml  --no-build --bootstrap $(QUIET)
	mv -f output/linux-x86_64/c2c/cgen/build.c bootstrap/bootstrap.c
	$(C2C) c2c -b bootstrap/build-darwin-x86_64.yaml --no-build --bootstrap $(QUIET)
	( diff bootstrap/bootstrap.c output/darwin-x86_64/c2c/cgen/build.c > bootstrap/bootstrap-darwin-x86_64.patch ; true )
	$(C2C) c2c -b bootstrap/build-darwin-arm64.yaml  --no-build --bootstrap $(QUIET)
	( diff bootstrap/bootstrap.c output/darwin-arm64/c2c/cgen/build.c > bootstrap/bootstrap-darwin-arm64.patch ; true )
	$(C2C) c2c -b bootstrap/build-freebsd-amd64.yaml --no-build --bootstrap $(QUIET)
	( diff bootstrap/bootstrap.c output/freebsd-amd64/c2c/cgen/build.c > bootstrap/bootstrap-freebsd-amd64.patch ; true )
	$(C2C) c2c -b bootstrap/build-openbsd-amd64.yaml --no-build --bootstrap $(QUIET)
	( diff bootstrap/bootstrap.c output/openbsd-amd64/c2c/cgen/build.c > bootstrap/bootstrap-openbsd-amd64.patch ; true )

globals:
	@$(MAKE) -C bootstrap globals $(MAKEFLAGS)

test: output/tester/tester
	@echo "---- running tests ----"
	@output/tester/tester -q test

testv: output/tester/tester
	@echo "---- running verbose tests ----"
	@output/tester/tester -v test

output/c2c_trace/c2c_trace: $(C2C_DEPS)
	$(C2C) c2c --trace-calls -o c2c_trace --fast $(QUIET)

trace_calls: $(C2C) output/c2c_trace/c2c_trace
	C2_TRACE="min=10;min2=1;mode=3;name=*;fd=2" output/c2c_trace/c2c_trace c2c -o c2c_calls $(QUIET) --test 2> output/c2c/calls

trace: trace_calls
	cat output/c2c/calls

alloc_trace: $(C2C) output/c2c_trace/c2c_trace
	C2_TRACE="min=10;min2=1;mode=3;name=stdlib.malloc,stdlib.calloc,stdlib.realloc;fd=2" output/c2c_trace/c2c_trace $(QUIET) c2c -o c2c_alloc --test 2> output/c2c/calls
	cat output/c2c/calls

show-diags:;	@./tools/show_diags.sh output/*/cgen/build.log
show-errors:;	@./tools/show_errors.sh output/*/cgen/build.log
show-warnings:;	@./tools/show_warnings.sh output/*/cgen/build.log

c-warnings:;    $(MAKE) clean all test examples QUIET='' C2FLAGS='--convert-c-warnings --show-c-warnings -Wconversion'

rebuild: clean all test trace_calls examples

.PHONY: examples
examples: c2c
	@$(MAKE) -C examples $(MAKEFLAGS)

clean:
	rm -rf output examples/output
