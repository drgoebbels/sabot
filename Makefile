all:
	gcc -Wall -msse4.2 -pedantic -fno-strict-aliasing -O2 -ggdb -lm -lpthread main.c general.c sanet.c database.c crypt.c -o sabot -I .

