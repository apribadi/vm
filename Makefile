.PHONY: all clean default

default: pal

all: pal pal-debug

clean:
	rm -f pal pal-debug

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
