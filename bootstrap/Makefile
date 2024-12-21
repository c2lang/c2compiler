# This makefile is auto-generated, any modifications will be lost

CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch -Wno-char-subscripts
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length
CFLAGS+=-pipe -std=c99 -O0 -g

all: ../output/c2c/c2c

c2c_bootstrap: bootstrap.c external.h
		@echo "---- compiling bootstrap compiler ----"
		@$(CC) $(CFLAGS) bootstrap.c -o c2c_bootstrap -ldl

../output/c2c/c2c: c2c_bootstrap
		@echo "---- running (bootstrapped) c2c ----"
		@./c2c_bootstrap c2c --fast --noplugins
		@echo "---- running c2c (no plugins) ----"
		@../output/c2c/c2c --noplugins
		@echo "---- running c2c (with plugins) ----"
		@(cd .. && ./install_plugins.sh )
		@../output/c2c/c2c c2c

clean:
		@rm -rf c2c_bootstrap ../output/

