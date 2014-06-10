all:
	gcc -Wall -lsqlite3 -msse4.2 -fno-strict-aliasing -ggdb -lm -pthread main.c regex.c general.c sanet.c database.c crypt.c -o sabot -I .

