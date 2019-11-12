all:
	 gcc -g -std=gnu17 -o main tag_type.c main.c
run:
	./main image.CR2
