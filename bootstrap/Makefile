# This makefile is auto-generated, any modifications will be lost

CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch -Wno-char-subscripts -Wno-zero-length-bounds -Wno-format-overflow -Wno-stringop-overflow
CFLAGS+=-pipe -O2 -std=c99 -g -Wno-missing-field-initializers

all: ../output/c2c/c2c

c2c_bootstrap: bootstrap.c
		@echo "---- compiling boostrap compiler ----"
		@$(CC) $(CFLAGS) bootstrap.c -o c2c_bootstrap
		@strip c2c_bootstrap

../c2_output/c2c/c2c: c2c_bootstrap
		@echo "---- running boostrap compiler ----"
		@./c2c_bootstrap -c

../output/c2c/c2c: ../c2_output/c2c/c2c
		@echo "---- running (boostrapped) c2c ----"
		@../c2_output/c2c/c2c -c

clean:
		@rm -rf c2c_bootstrap ../c2_output/ ../output/

