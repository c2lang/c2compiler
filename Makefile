C2C:=output/c2c/c2c

all: $(C2C)
	@$(C2C)

c2c: $(C2C)
	@$(C2C) --version

$(C2C):
	$(MAKE) -B -C bootstrap

output/tester/tester: tools/tester/*.c2 $(C2C)
	@$(C2C) tester

rebuild-bootstrap: $(C2C)
	@echo "generating bootstrap files for various systems/architectures"
	$(C2C) c2c -b bootstrap/build_linux_x86_64.yaml --test
	mv -f output/linux_x86_64/c2c/cgen/build.c bootstrap/bootstrap_linux_x86_64.c
	$(C2C) c2c -b bootstrap/build_darwin_x86_64.yaml --test
	( diff bootstrap/bootstrap_linux_x86_64.c output/darwin_x86_64/c2c/cgen/build.c > bootstrap/bootstrap_darwin_x86_64.patch ; true )
	$(C2C) c2c -b bootstrap/build_darwin_arm64.yaml --test
	( diff bootstrap/bootstrap_linux_x86_64.c output/darwin_arm64/c2c/cgen/build.c > bootstrap/bootstrap_darwin_arm64.patch ; true )

test: output/tester/tester
	@output/tester/tester -t test

warnings:
	grep -n '[[]-W' `find . -name build.log`

clean:
	rm -rf output c2_plugins
