all:
	gcc -Wall -msse4.2 -fno-strict-aliasing -O2 -ggdb -lm -pthread main.c general.c sanet.c database.c crypt.c -o sabot -I .

