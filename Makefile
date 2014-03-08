all:
	gcc -Wall -pedantic -ggdb -lm -lpthread main.c general.c sanet.c database.c crypt.c -o sabot -I .

