C2C:=output/c2c/c2c

C2FLAGS:=
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

C2C_DIRS:= {analyser,analyser_utils,ast,ast_utils,common,compiler,generator,ir,libs,parser}
C2C_DEPS:= $(wildcard $(C2C_DIRS)/*.c2* $(C2C_DIRS)/*/*.c2*) recipe.txt Makefile

all: $(C2C)

c2c: $(C2C)
	@$(C2C) --version

$(C2C): output/bootstrap/bootstrap $(C2C_DEPS)
	@echo "---- running (bootstrapped$(C2FLAGS)) c2c ----"
	@output/bootstrap/bootstrap c2c $(C2FLAGS) --fast --noplugins
	@mv output/c2c/c2c output/bootstrap/c2c
	@echo "---- running c2c (no plugins$(C2FLAGS)) ----"
	@output/bootstrap/c2c $(C2FLAGS) --noplugins --fast
	@./install_plugins.sh
	@echo "---- running c2c (optimized with plugins$(C2FLAGS)) ----"
	@output/c2c/c2c $(C2FLAGS)
	@./install_plugins.sh

output/bootstrap/bootstrap:
	@$(MAKE) -B -C bootstrap

san:;	@$(MAKE) -B ASAN=1 UBSAN=1
asan:;	@$(MAKE) -B ASAN=1
msan:;	@$(MAKE) -B MSAN=1
ubsan:;	@$(MAKE) -B UBSAN=1
debug:;	@$(MAKE) -B DEBUG=1

output/tester/tester: tools/tester/*.c2 $(C2C_DEPS) $(C2C)
	@$(C2C) tester

rebuild-bootstrap: $(C2C)
	@echo "generating bootstrap files for various systems/architectures"
	$(C2C) c2c -b bootstrap/build-linux-x86_64.yaml  --test
	mv -f output/linux-x86_64/c2c/cgen/build.c bootstrap/bootstrap.c
	$(C2C) c2c -b bootstrap/build-darwin-x86_64.yaml  --test
	( diff bootstrap/bootstrap.c output/darwin-x86_64/c2c/cgen/build.c > bootstrap/bootstrap-darwin-x86_64.patch ; true )
	$(C2C) c2c -b bootstrap/build-darwin-arm64.yaml  --test
	( diff bootstrap/bootstrap.c output/darwin-arm64/c2c/cgen/build.c > bootstrap/bootstrap-darwin-arm64.patch ; true )
	$(C2C) c2c -b bootstrap/build-freebsd-amd64.yaml  --test
	( diff bootstrap/bootstrap.c output/freebsd-amd64/c2c/cgen/build.c > bootstrap/bootstrap-freebsd-amd64.patch ; true )
	$(C2C) c2c -b bootstrap/build-openbsd-amd64.yaml  --test
	( diff bootstrap/bootstrap.c output/openbsd-amd64/c2c/cgen/build.c > bootstrap/bootstrap-openbsd-amd64.patch ; true )

test: output/tester/tester
	@output/tester/tester -t test

testv: output/tester/tester
	@output/tester/tester -v test

output/c2c_trace/c2c_trace: $(C2C_DEPS)
	$(C2C) c2c --trace-calls -o c2c_trace --fast

trace_calls: $(C2C) output/c2c_trace/c2c_trace
	C2_TRACE="min=10;min2=1;mode=3;name=*;fd=2" output/c2c_trace/c2c_trace c2c --test 2> output/c2c/calls

trace: trace_calls
	cat output/c2c/calls

alloc_trace: $(C2C) output/c2c_trace/c2c_trace
	C2_TRACE="min=10;min2=1;mode=3;name=stdlib.malloc,stdlib.calloc;fd=2" output/c2c_trace/c2c_trace c2c --test 2> output/c2c/calls
	cat output/c2c/calls

errors:
	@( grep -n 'error:' `find output -name build.log` | sed -E 's/build.log:[0-9]+://' ; true )

warnings:
	@( grep -n '[[]-W' `find output -name build.log` | sed -E 's/build.log:[0-9]+://' ; true )

rebuild: clean all test trace_calls warnings

clean:
	rm -rf output
