.PHONY: all clean default

default: pal

all: pal pal-debug

clean:
	rm -f pal pal-debug

pal: src/*.c
	clang -o pal src/pal.c \
		-std=c17 \
		-O2 \
		-Wall \
		-Wconversion \
		-Wdouble-promotion \
		-Werror \
		-Wextra \
		-Wno-fixed-enum-extension \
		-Wstrict-prototypes \
		-fno-math-errno \
		-fno-omit-frame-pointer \
		-fno-rounding-math \
		-pedantic \
		-DNDEBUG

pal-debug: src/*.c
	clang -o pal-debug src/pal.c \
		-std=c17 \
		-O2 \
		-Wall \
		-Wconversion \
		-Wdouble-promotion \
		-Werror \
		-Wextra \
		-Wno-fixed-enum-extension \
		-Wstrict-prototypes \
		-fno-math-errno \
		-fno-omit-frame-pointer \
		-fno-rounding-math \
		-pedantic \
		-fsanitize=address \
		-fsanitize=undefined
