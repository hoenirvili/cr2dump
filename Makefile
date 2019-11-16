CC = gcc
CFLAGS += -Wall -Wextra -Wpedantic \
          -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs

# GCC warnings that Clang doesn't provide:
ifeq ($(CC),gcc)
    CFLAGS += -Wjump-misses-init -Wlogical-op
endif

CFLAGS += "-O2"

all:
	 $(CC) $(CFLAGS) main.c cr2_header.c file.c ifd.c tag_type.c tiff_header.c -o main
run:
	./main image.CR2
