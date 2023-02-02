# This makefile is auto-generated, any modifications will be lost

CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch -Wno-char-subscripts -Wno-zero-length-bounds -Wno-format-overflow -Wno-stringop-overflow
CFLAGS+=-pipe -O2 -std=c99 -g -Wno-missing-field-initializers

c2c_bootstrap: bootstrap.c
		$(CC) $(CFLAGS) bootstrap.c -o c2c_bootstrap
		strip c2c_bootstrap

clean:
		rm -f c2c_bootstrap

