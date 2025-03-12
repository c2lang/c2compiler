C2C:=output/c2c/c2c

all: $(C2C)
	@$(C2C)
	./install_plugins.sh

c2c: $(C2C)
	@$(C2C) --version

$(C2C):
	@$(MAKE) -B -C bootstrap

san:
	@$(MAKE) -B -C bootstrap ASAN=1 UBSAN=1

asan:
	@$(MAKE) -B -C bootstrap ASAN=1

msan:
	@$(MAKE) -B -C bootstrap MSAN=1

ubsan:
	@$(MAKE) -B -C bootstrap UBSAN=1

output/tester/tester: tools/tester/*.c2 $(C2C)
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

errors:
	@( grep -n 'error:' `find . -name build.log` | sed -E 's/build.log:[0-9]+://' ; true )

warnings:
	@( grep -n '[[]-W' `find . -name build.log` | sed -E 's/build.log:[0-9]+://' ; true )

clean:
	rm -rf output
