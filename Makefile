.PHONY: all clean default

default: out/exe

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
	rm -f pal pal-debug out/*.o

out/exe: out/run.o out/shm.o out/foo.o
	ld out/run.o out/shm.o out/foo.o -o $@ $(LDFLAGS)

out/run.o: a64/run.c
	clang -c a64/run.c -o $@ $(CFLAGS) -DNDEBUG

out/shm.o: a64/shm.s
	as a64/shm.s -o $@

out/foo.o: a64/foo.s
	as a64/foo.s -o $@

out/bar.o: a64/bar.s
	as a64/bar.s -o $@

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
