CFLAGS:=-Wall -Wextra -Wno-unused -Wno-switch
CFLAGS+=-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-zero-length
CFLAGS+=-pipe -std=c99 -O0 -g
C2FLAGS:=

SYSTEM:=$(shell uname -s)
ARCH:=$(shell uname -m)

ifeq (Linux,$(SYSTEM))
CC:=gcc
LDFLAGS:=-ldl
BOOTSTRAP_FILE:=bootstrap_linux_x86_64.c
endif

ifeq (Darwin,$(SYSTEM))
CC:=clang
LDFLAGS:=-ldl
ifeq (arm64,$(ARCH))
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap_darwin_arm64.c
else
BOOTSTRAP_FILE:=../output/bootstrap/bootstrap_darwin_x86_64.c
endif
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
		@mkdir -p ../c2_plugins
		@(cd .. && ./install_plugins.sh )
		@echo "---- running c2c (optimized with plugins) ----"
		@../output/c2c/c2c $(C2FLAGS)
		@(cd .. && ./install_plugins.sh )

../output/bootstrap/%.c: %.patch bootstrap_linux_x86_64.c
		@echo "---- patching bootstrap code for $(SYSTEM)-$(ARCH) ----"
		@mkdir -p ../output/bootstrap
		patch -s -o $@ bootstrap_linux_x86_64.c $<

clean:
		@rm -rf ../output/ ../c2_plugins/
