# This makefile is auto-generated, any modifications will be lost

CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch -Wno-char-subscripts -Wno-zero-length-bounds -Wno-format-overflow -Wno-stringop-overflow
CFLAGS+=-pipe -O0 -std=c99 -g -Wno-missing-field-initializers -Wno-format-zero-length

all: ../output/c2c/c2c

c2c_bootstrap: bootstrap.c external.h
		@echo "---- compiling bootstrap compiler ----"
		@$(CC) $(CFLAGS) bootstrap.c -o c2c_bootstrap -ldl

../output/c2c/c2c: c2c_bootstrap
		@echo "---- running (bootstrapped) c2c ----"
		@./c2c_bootstrap -c c2c --fast --noplugins
		@echo "---- running c2c (no plugins) ----"
		@../output/c2c/c2c --noplugins

clean:
		@rm -rf c2c_bootstrap ../output/

