C2C:=output/c2c/c2c

all: $(C2C)
	@$(C2C)

c2c: $(C2C)
	@$(C2C) c2c

$(C2C): bootstrap/bootstrap.c
	make -B -C bootstrap

output/tester/tester: tools/tester/*.c2 $(C2C)
	@$(C2C) tester

rebuild-bootstrap: $(C2C)
	@$(C2C) --test c2c
	mv -f output/c2c/cgen/build.c bootstrap/bootstrap.c

test: output/tester/tester
	@output/tester/tester -t test

warnings:
	grep -n '[[]-W' `find . -name build.log`

clean:
	make -C bootstrap clean
	rm -rf output
