all:
	gcc -Wall -pedantic -fno-strict-aliasing -ggdb -lm -lpthread main.c general.c sanet.c database.c crypt.c -o sabot -I .

