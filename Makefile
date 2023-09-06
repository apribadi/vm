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
		-Wextra \
		-Wno-c++98-compat \
		-Wno-constant-logical-operand \
		-Wno-fixed-enum-extension \
		-Wno-gnu-statement-expression-from-macro-expansion \
		-Wno-pre-c2x-compat \
		-ffp-contract=off \
		-fno-math-errno \
		-fno-omit-frame-pointer \
		-pedantic \
		-Weverything \
		-Wno-declaration-after-statement \
		-Wno-unsafe-buffer-usage \
		-DNDEBUG

pal-debug: src/*.c
	clang -o pal-debug src/pal.c \
		-std=c2x \
		-O2 \
		-Wall \
		-Wconversion \
		-Wdouble-promotion \
		-Werror \
		-Wextra \
		-Wno-c++98-compat \
		-Wno-constant-logical-operand \
		-Wno-fixed-enum-extension \
		-Wno-gnu-statement-expression-from-macro-expansion \
		-Wno-pre-c2x-compat \
		-ffp-contract=off \
		-fno-math-errno \
		-fno-omit-frame-pointer \
		-pedantic \
		-Weverything \
		-Wno-declaration-after-statement \
		-Wno-unsafe-buffer-usage \
		-fsanitize=address \
		-fsanitize=undefined
