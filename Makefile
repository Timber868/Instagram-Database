igdb: igdb.o database.o
	gcc -o igdb igdb.o database.o

igdb.o: igdb.c database.h
	gcc -c igdb.c

database.o: database.c database.h
	gcc -c database.c
