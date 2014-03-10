all:
	gcc -Wall -msse4.2 -fno-strict-aliasing -ggdb -lm -lpthread main.c general.c sanet.c database.c crypt.c -o sabot -I .

