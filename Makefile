pal: src/*.c
	clang -o pal src/pal.c \
		-std=c17 \
		-O2 \
		-Wall \
		-Wdouble-promotion \
		-Werror \
		-Wextra \
		-Wno-fixed-enum-extension \
		-Wno-unused-function \
		-Wno-unused-parameter \
		-Wstrict-prototypes \
		-fno-math-errno \
		-fno-omit-frame-pointer \
		-fno-rounding-math \
		-pedantic

# -fsanitize=address \
# -fsanitize=undefined \
