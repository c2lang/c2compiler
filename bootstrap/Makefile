CFLAGS:=-Wall -Wextra -Wno-unused -Wno-switch
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length
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

ifdef ASAN
DEBUG_SUFFIX:=_asan
C2FLAGS+= --asan
CFLAGS+= -fsanitize=address -g -D__ASAN__
endif

ifdef MSAN
DEBUG_SUFFIX:=_msan
C2FLAGS+= --msan
CFLAGS+= -fsanitize=memory -g -D__MSAN__
endif

ifdef UBSAN
DEBUG_SUFFIX:=_ubsan
C2FLAGS+= --ubsan
CFLAGS+= -fsanitize=undefined -g -D__UBSAN__
endif

all: ../output/c2c/c2c

../output/bootstrap/bootstrap: $(BOOTSTRAP_FILE)
		@echo "---- compiling bootstrap compiler ----"
		@mkdir -p ../output/bootstrap
		@$(CC) $(CFLAGS) -o $@ $(BOOTSTRAP_FILE) $(LDFLAGS)

../output/c2c/c2c: ../output/bootstrap/bootstrap
		@echo "---- running (bootstrapped) c2c ----"
		@../output/bootstrap/bootstrap c2c $(C2FLAGS) --fast --noplugins
		@echo "---- running c2c (no plugins) ----"
		@../output/c2c/c2c $(C2FLAGS) --noplugins --fast
		@(cd .. && ./install_plugins.sh )
		@echo "---- running c2c (optimized with plugins) ----"
		@../output/c2c/c2c $(C2FLAGS)
		@(cd .. && ./install_plugins.sh )

../output/bootstrap/%.c: %.patch bootstrap.c
		@echo "---- patching bootstrap code for $(SYSTEM)-$(ARCH) ----"
		@mkdir -p ../output/bootstrap
		patch -s -o $@ bootstrap.c $<

clean:
		@rm -rf ../output/
