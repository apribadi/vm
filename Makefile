.PHONY: all clean default

default: out/example

all: pal pal-debug

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
	rm -f pal pal-debug out/*.o out/combined

out/example: a64/runtime.c a64/example.S
	clang -o out/example a64/runtime.c a64/example.S $(CFLAGS) -DNDEBUG

pal: src/*.c
	clang -o pal src/pal.c \
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
		-Wno-pre-c2x-compat \
		-Wno-shadow \
		-Wno-shift-op-parentheses \
		-Wno-unsafe-buffer-usage \
		-ffp-contract=off \
		-fno-math-errno \
		-fno-omit-frame-pointer \
		-fno-slp-vectorize \
		-pedantic \
		-DNDEBUG

pal-debug: src/*.c
	clang -o pal-debug src/pal.c \
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
		-Wno-pre-c2x-compat \
		-Wno-shadow \
		-Wno-shift-op-parentheses \
		-Wno-unsafe-buffer-usage \
		-ffp-contract=off \
		-fno-math-errno \
		-fno-omit-frame-pointer \
		-fno-slp-vectorize \
		-pedantic \
		-fsanitize=address \
		-fsanitize=undefined
