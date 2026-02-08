# CFLAGS are used to compile the bootstrap C code
CFLAGS:=
#CFLAGS+=-Wall -Wextra
CFLAGS+=-Wno-unused -Wno-switch
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length
# silence clang warning on %lb on macOS
CFLAGS+=-Wno-format
CFLAGS+=-pipe -std=c99 -funsigned-char -O0 -g

SYSTEM:=$(shell uname -s)
ARCH:=$(shell uname -m)

ifeq (Linux,$(SYSTEM))
CC:=gcc
LDFLAGS:=-ldl
BOOTSTRAP_FILE:=bootstrap.c
endif

ifeq (Darwin,$(SYSTEM))
CC:=clang
LDFLAGS:=-ldl
CFLAGS+=-Wno-string-plus-int
ifeq (arm64,$(ARCH))
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-darwin-arm64.c
else
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-darwin-x86_64.c
endif
endif

ifeq (FreeBSD,$(SYSTEM))
CC:=clang
LDFLAGS:=-lpthread
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-freebsd-amd64.c
endif

ifeq (OpenBSD,$(SYSTEM))
CC:=clang
LDFLAGS:=
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-openbsd-amd64.c
endif

all: ../output/bootstrap/bootstrap

../output/bootstrap/bootstrap: $(BOOTSTRAP_FILE)
	@echo "---- compiling bootstrap compiler ----"
	@mkdir -p ../output/bootstrap
	@$(CC) $(CFLAGS) -o $@ $(BOOTSTRAP_FILE) $(LDFLAGS)

../output/bootstrap/%.c: %.patch bootstrap.c
	@echo "---- patching bootstrap code for $(SYSTEM)-$(ARCH) ----"
	@mkdir -p ../output/bootstrap
	@patch -s -o $@ bootstrap.c $<

clean:
	@rm -rf ../output/
