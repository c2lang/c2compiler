# This makefile is auto-generated, any modifications will be lost

CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch -Wno-char-subscripts
ifneq (,$(shell $(CC) --version | grep clang))
CFLAGS+=-Wno-incompatible-library-redeclaration
CFLAGS+=-Wno-parentheses-equality
CFLAGS+=-Wno-sometimes-uninitialized
CFLAGS+=-Wno-unused-parameter
CFLAGS+=-Wno-incompatible-library-redeclaration
CFLAGS+=-Wno-unknown-warning-option
CFLAGS+=-Wno-parentheses-equality
CFLAGS+='-Dfallthrough=__attribute__((fallthrough))'
else
#CFLAGS+=-Wno-zero-length-bounds -Wno-format-overflow -Wno-stringop-overflow
CFLAGS+=-Wno-unused-parameter
CFLAGS+=-Wno-main
CFLAGS+='-Dfallthrough='
endif
CFLAGS+=-pipe -O0 -std=c99 -g -Wno-missing-field-initializers -Wno-format-zero-length
TARGET_ARCH:=$(shell uname -s)-$(shell uname -m)
ifeq (Darwin-arm64,$(TARGET_ARCH))
CFLAGS+=-DDARWIN_ARM64
endif

all: ../output/c2c/c2c

c2c_bootstrap: bootstrap.c external.h
		@echo "---- compiling bootstrap compiler ----"
		$(CC) $(CFLAGS) bootstrap.c -o c2c_bootstrap -ldl

c2c_bootstrap2: c2c_bootstrap bootstrap.c
		@echo "---- running (bootstrapped) c2c ----"
		./c2c_bootstrap -c c2c --fast --noplugins
		mv -f ../output/c2c/c2c c2c_bootstrap2

../output/c2c/c2c: c2c_bootstrap2
		@echo "---- running c2c (no plugins) ----"
		./c2c_bootstrap2 --noplugins

globals: globals.c
		$(CC) -E - < globals.c > globals.i
		$(CC) globals.c -o globals
		./globals

clean:
		rm -rf c2c_bootstrap c2c_bootstrap2 globals.i globals ../output/
