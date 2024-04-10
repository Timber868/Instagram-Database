#ifndef DB_H
#define DB_H

#include <stddef.h>

//Define the Record type as we will need it in our C file
typedef struct{
	char handle[32];
	char comment[64];
	long unsigned int followerCount;
	long unsigned int dateLastModified;
}Record;

//Define the Database type as we will need it in our C file
typedef struct{
	Record* array;
	int capacity;
	int size;
}Database;

//Function declarations to layout the skeleton of our C program
Database db_create();

void db_append(Database *db, Record const * item);

Record * db_index(Database * db, int index);

Record * db_lookup(Database * db, char const * handle);

void db_free(Database * db);


void db_load_csv(Database * db, char const * path);

void db_write_csv(Database * db, char const * path);

//My helper functions, declare them here so they are available everywhere in database.c
int db_resize(Database *db);

#endif
