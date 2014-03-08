all:
	gcc -Wall -pedantic -ggdb -lm main.c general.c sanet.c -o sabot -I .

