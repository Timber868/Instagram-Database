#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "database.h"

Database db_create() {
	Database database;
	database.size = 0;
	database.capacity =4;
	database.array = malloc(database.capacity * sizeof(Record));
	if(database.array == NULL){
		fprintf(stderr, "Error: Could not allocate memory for the database array.\n");
		exit(EXIT_FAILURE);
	}
	return database;
}

//My database resizing helper function
int db_resize(Database *db){
        //Use a temporary pointer to then test if realloc was successful
        Record *newArray = realloc(db->array, db->capacity * 2 * sizeof(Record));

        //Throw an error if realloc was not successful to notify the user
        if(newArray == NULL){
                fprintf(stderr, "Error: Could not allocate memory for the database array resizing.\n");
		return 1;
		//Returns 1 to let the functions using it that it failed
        } else {
                db->capacity *= 2; //The resizing of the database array is only done in the else to avoid doing it if there was an error
                db->array = newArray;
		return 0;
		//Returns 0 to let the functions using it that it passed
        }
}

void db_append(Database * db, Record const * item){
	//We do an if because we don't want to try to add to unexisting memory
	if(db->size >= db->capacity){
		//We need to look at the error to not continue if there was one
		int error = db_resize(db);
		if(error == 1){
			//Make sure to exit so that we dont add to a list we couldnt expand
			exit(EXIT_FAILURE);
		}
	}
	//Add the array to our database
	db->array[db->size] = *item;
	db->size++;
}

Record * db_index(Database * db, int index){
	//Check that the index is valid to not get code failures later
	if(index > db->size || index < 0){
		fprintf(stderr, "Error: Invalid index.\n");
		//Return null to show that the function has failed
		return NULL;
	} else {
		//Return a pointer to the record so that it can later be used
		return &(db->array[index]);
	}
}

Record * db_lookup(Database * db, char const * handle){
	//Makes sure our database is valid to avoid errors
	if (db == NULL || db->array == NULL || db->size == 0 || db->size > db->capacity) {
		//Return null to interrupt the function
		fprintf(stderr, "Error: Invalid database\n");
		return NULL;
	}
	for(int i = 0; i < db->size; i++){
		//Check if handle at indec is the same as handle given to see if record matches to handle
		if(strcmp(db->array[i].handle, handle) == 0){
			return &(db->array[i]);
		}
	}
	//In case handle wasnt found we return null so main function knows it failed
	return NULL;
}

void db_free(Database * db){
	//If db does not exist theres nothing to free so return
	if (db == NULL) {
		return;
	}

	//If the database array is not empty we also need to free it to avoid memory leaks
	if (db->array != NULL) {
		free(db->array);
		db->array = NULL;
	}
	
	//Finally free the databaee as the database can no longer be used
	free(db);
}

Record parse_record ( char const * line ){
	Record record; //Record we want to get from the line
	
	//We need to copy line into something mutable as strtok needs to modify it and line here is a const and
	//immutable
	char mutableLine[strlen(line) + 1]; //+1 for \0
	strcpy(mutableLine, line);

	//Split the line into fields by spliting with , so that we get the info needed
	char *field = strtok(mutableLine, ",");

	//Ifs check that were not just adding a null pointer
	if(field != NULL) {
		//Copy the content into record handle for the first field, size-1 so that we have space for \0
		strncpy(record.handle, field, sizeof(record.handle) -1);
		record.handle[sizeof(record.handle) - 1] = '\0'; //We need to finish the handle with a \0
	}

	//Continue from NULL aka where we were until the next , to get the second field
	field = strtok(NULL, ",");
	if(field != NULL) {
		strncpy(record.comment, field, sizeof(record.comment)-1);
		record.comment[sizeof(record.comment)-1] = '\0'; 
	}
	
	//Now onto the third field
	field = strtok(NULL, ",");
	if(field != NULL) {
		//We convert the whole thing into long so no need for end part
		record.followerCount = strtoul(field, NULL, 10); //Strtoul as unsigned long integers
	}

	//Fourth field
	field = strtok(NULL, ",");
	if(field != NULL) {
		record.dateLastModified = strtoul(field, NULL, 10);
	}

	return record;
}	

void db_load_csv(Database * db, char const * path){

        FILE *myCSV = fopen(path, "rt");
        //Makes sure we actually loaded the file so we dont try to get info from nothing
        if(myCSV == NULL){
                fprintf(stderr, "Error: Could not open the csv file\n");
                return;
        }

        //Initialize our buffer and line length to use it in get line
        char *data = NULL;
        size_t length = 0;
        int count = 0; // Variable to keep track of the number of records that we go throguh

        while (getline(&data, &length, myCSV) != -1) {
                if (count != 0) { //We dont read the first one its the column info
                        // Parse the line into a Record using my parse_record so that we can later add it to the database
                        Record record = parse_record(data);
                        db_append(db, &record); //Add it to the database 
                }
                count++; // Increment the record count each time
        }

        //Free the buffer
        free(data);
        fclose(myCSV); //Close the files or else well have issues

        printf("Loaded %d records.\n", count-1); // Print the number of records loaded, -1 as the first one is the info
}

void db_write_csv(Database *db, char const *path) {
        FILE *myCSV = fopen(path, "wt");
        if (myCSV == NULL) {
                fprintf(stderr, "Error: Could not open the csv file\n");
                return;
        }
	// Write the first csv line with the info of what each column is
        fprintf(myCSV, "Handle,Comment,FollowerCount,DateLastModified\n");
   	int count = 0;

        // Write each record's information into a line
        for (int i = 0; i < db->size; i++) {
                fprintf(myCSV, "%s,", db->array[i].handle);
                fprintf(myCSV, "%s,", db->array[i].comment);
                fprintf(myCSV, "%lu,", db->array[i].followerCount);
                fprintf(myCSV, "%lu\n", db->array[i].dateLastModified);
                count++;
        }
        
        printf("Wrote %d records.\n", count); // Print the number of records we wrote
        fclose(myCSV);
}

