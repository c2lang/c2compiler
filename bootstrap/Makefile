# CFLAGS are used to compile the bootstrap C code
CFLAGS:=
#CFLAGS+=-Wall -Wextra
CFLAGS+=-Wno-unused -Wno-switch
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length
# silence clang warning on %lb on macOS
CFLAGS+=-Wno-format
CFLAGS+=-pipe -std=c99 -funsigned-char -O0

SYSTEM:=$(shell uname -s)
ARCH:=$(shell uname -m)

ifeq (Linux,$(SYSTEM))
CC:=gcc
LDFLAGS:=-ldl
ifeq (y,$(CONFIG_M32))
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-linux-i686.c
ARCH:=i686
else
BOOTSTRAP_FILE:=bootstrap.c
endif
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
LDFLAGS:=
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-freebsd-amd64.c
endif

ifeq (OpenBSD,$(SYSTEM))
CC:=clang
LDFLAGS:=
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap-openbsd-amd64.c
endif

ifeq (y,$(CONFIG_M32))
CFLAGS += -m32
LDFLAGS += -m32
endif

all: ../output/bootstrap/bootstrap

../output/bootstrap/bootstrap: $(BOOTSTRAP_FILE)
	@echo "---- compiling bootstrap compiler ----"
	@mkdir -p ../output/bootstrap
	@$(CC) $(CFLAGS) -o $@ $(BOOTSTRAP_FILE) $(LDFLAGS)

../output/bootstrap/%.c: %.patch bootstrap.c
	@echo "---- patching bootstrap code for $(SYSTEM)-$(ARCH) ----"
	@mkdir -p ../output/bootstrap
	@cp -f bootstrap.c $@
	@patch -s $@ $<

../output/bootstrap/globals: globals.c
	@echo "---- compiling header extractor ----"
	@mkdir -p ../output/bootstrap
	$(CC) $(CFLAGS) -o $@ globals.c $(LDFLAGS)

globals: ../output/bootstrap/globals
	../output/bootstrap/globals
	rm ../output/bootstrap/globals

clean:
	@rm -rf ../output/
