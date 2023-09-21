.PHONY: all clean default

default: out/example

all: out/example out/pal out/pal-debug

CFLAGS = \
	-std=c2x \
	-O2 \
	-Wall \
	-Wconversion \
	-Wdouble-promotion \
	-Werror \
	-Weverything \
	-Wextra \
	-Wno-c++98-compat \
	-Wno-constant-logical-operand \
	-Wno-declaration-after-statement \
	-Wno-fixed-enum-extension \
	-Wno-gnu-statement-expression-from-macro-expansion \
	-Wno-missing-prototypes \
	-Wno-pre-c2x-compat \
	-Wno-shadow \
	-Wno-shift-op-parentheses \
	-Wno-unsafe-buffer-usage \
	-ffp-contract=off \
	-fno-math-errno \
	-fno-omit-frame-pointer \
	-fno-slp-vectorize \
	-pedantic

LDFLAGS = \
	-lSystem \
	-syslibroot `xcrun -sdk macosx --show-sdk-path` \
	-arch arm64

clean:
	rm -f out/*

out/example: a64/runtime.c a64/example.S
	clang -o out/example a64/runtime.c a64/example.S $(CFLAGS) -DNDEBUG

out/pal: src/*.c
	clang -o pal src/pal.c $(CFLAGS) -DNDEBUG

out/pal-debug: src/*.c
	clang -o pal-debug src/pal.c $(CFLAGS) -fsanitize=address -fsanitize=undefined
