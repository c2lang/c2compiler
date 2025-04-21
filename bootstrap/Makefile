# CFLAGS are used to compile the bootstrap C code
CFLAGS:=-Wall -Wextra -Wno-unused -Wno-switch
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length -Wno-string-plus-int
# silence clang warning on %lb on macOS
CFLAGS+=-Wno-format
CFLAGS+=-pipe -std=c99 -funsigned-char -O0 -g
C2FLAGS:=

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
ifeq (arm64,$(ARCH))
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-darwin-arm64.c
else
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-darwin-x86_64.c
endif
endif

ifeq (FreeBSD,$(SYSTEM))
CC:=clang
LDFLAGS:=
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-freebsd-amd64.c
endif

ifeq (OpenBSD,$(SYSTEM))
CC:=clang
LDFLAGS:=
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-openbsd-amd64.c
endif

MSG:=

ifdef ASAN
MSG+= asan
C2FLAGS+= --asan
endif

ifdef MSAN
MSG+= msan
C2FLAGS+= --msan
endif

ifdef UBSAN
MSG+= ubsan
C2FLAGS+= --ubsan
endif

all: ../output/c2c/c2c

../output/bootstrap/bootstrap: $(BOOTSTRAP_FILE)
		@echo "---- compiling bootstrap compiler ----"
		@mkdir -p ../output/bootstrap
		@$(CC) $(CFLAGS) -o $@ $(BOOTSTRAP_FILE) $(LDFLAGS)

../output/c2c/c2c: ../output/bootstrap/bootstrap
		@echo "---- running (bootstrapped$(MSG)) c2c ----"
		@../output/bootstrap/bootstrap c2c $(C2FLAGS) --fast --noplugins
		@echo "---- running c2c (no plugins$(MSG)) ----"
		@../output/c2c/c2c $(C2FLAGS) --noplugins --fast
		@(cd .. && ./install_plugins.sh )
		@echo "---- running c2c (optimized with plugins$(MSG)) ----"
		@../output/c2c/c2c $(C2FLAGS)
		@(cd .. && ./install_plugins.sh )

../output/bootstrap/%.c: %.patch bootstrap.c
		@echo "---- patching bootstrap code for $(SYSTEM)-$(ARCH) ----"
		@mkdir -p ../output/bootstrap
		@patch -s -o $@ bootstrap.c $<

clean:
		@rm -rf ../output/
