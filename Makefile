all:
	gcc -Wall -pedantic -ggdb -lm -lpthread main.c general.c sanet.c -o sabot -I .

