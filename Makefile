all:
	 gcc -std=gnu17 -o main tag_type.c main.c -lfort && ./main image.CR2
