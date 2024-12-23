CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length
CFLAGS+=-pipe -std=c99 -O0 -g

all: ../output/c2c/c2c

# NOTE: XXX should check which bootstrap to take
c2c_bootstrap: bootstrap_linux_x86_64.c
		@echo "---- compiling bootstrap compiler ----"
		@$(CC) $(CFLAGS) bootstrap_linux_x86_64.c -o c2c_bootstrap -ldl

../output/c2c/c2c: c2c_bootstrap
		@echo "---- running (bootstrapped) c2c ----"
		@./c2c_bootstrap c2c --fast --noplugins
		@echo "---- running c2c (no plugins) ----"
		@../output/c2c/c2c --noplugins --fast
		@echo "---- running c2c (optimized with plugins) ----"
		@../output/c2c/c2c
		@(cd .. && ./install_plugins.sh )

clean:
		@rm -rf c2c_bootstrap ../output/

