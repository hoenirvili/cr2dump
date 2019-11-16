
all:
	 gcc -g -Wall -std=gnu17 -o main main.c cr2_header.c file.c ifd.c tag_type.c tiff_header.c
run:
	./main image.CR2
