# This makefile is auto-generated, any modifications will be lost

CC=gcc
CFLAGS=-Wall -Wextra -Wno-unused -Wno-switch -Wno-char-subscripts
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length
CFLAGS+=-pipe -std=c99 -O0 -g
TARGET_ARCH:=$(shell uname -s)-$(shell uname -m)

C2C:=../output/c2c/c2c

ifeq (Darwin-arm64,$(TARGET_ARCH))
BOOTSTRAP_FILE=bootstrap-darwin-arm64.c
else
BOOTSTRAP_FILE=bootstrap.c
endif

all: $(C2C)

c2c_bootstrap: $(BOOTSTRAP_FILE) external.h
	@echo "---- compiling bootstrap compiler ----"
	@$(CC) $(CFLAGS) $(BOOTSTRAP_FILE) -o c2c_bootstrap -ldl

c2c_bootstrap2: c2c_bootstrap bootstrap.c
	@echo "---- running (bootstrapped) c2c ----"
	@./c2c_bootstrap c2c --fast --noplugins
	@mv -f ../output/c2c/c2c c2c_bootstrap2

$(C2C): c2c_bootstrap2
	@echo "---- running c2c (no plugins) ----"
	@./c2c_bootstrap2 --noplugins
	@echo "---- running c2c (with plugins) ----"
	@( cd .. && ./install_plugins.sh )
	@../output/c2c/c2c c2c

globals: globals.c
	$(CC) -E - < globals.c > globals.i
	$(CC) globals.c -o globals
	./globals

bootstrap-darwin-arm64.c: bootstrap-darwin-arm64.patch bootstrap.c
	@patch -o $@ bootstrap.c $*.patch

rebuild:
	@echo "---- rebuilding bootstrap compiler ----"
	$(C2C) --target x86_64-unknown-linux-gnu --test c2c
	mv -f ../output/c2c/cgen/build.c bootstrap.c
	$(C2C) --target arm64-apple-darwin-macho --test c2c
	mv -f ../output/c2c/cgen/build.c bootstrap-darwin-arm64.c
	( diff bootstrap.c bootstrap-darwin-arm64.c > bootstrap-darwin-arm64.patch ; true )
	rm bootstrap-darwin-arm64.c

clean:
	rm -f c2c_bootstrap c2c_bootstrap2 globals.i globals
	rm -f bootstrap-darwin-arm64.c
	rm -rf ../output/ c2c_bootstrap.dSYM
