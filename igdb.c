#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>

#include "database.h" // Include the database header file


//helper function for list
void list(Database *db) {   
    //Print the top of the table
    printf("HANDLE          | FOLLOWERS | LAST MODIFIED    | COMMENT\n");
    printf("----------------|-----------|------------------|--------------------------------\n");
    for (int i=0; i < db->size; i++) {
        Record record = db->array[i]; //Get the record at the current index

        char time[30]; //Makes a time container to store the formatted time string into so that we can reuse it later
        strftime(time, 30, "%Y-%m-%d %H:%M", localtime(&record.dateLastModified)); // Convert the record.time into a string using strftime()

        //Print out the information row, formatting the strings to fit the table
        printf("%-15s | %-9ld | %16s | %-30s\n", record.handle, record.followerCount, time, record.comment);
    }
}

void add_record(Database *db, char *handle, char *followerCount) {
    // Convert followers to unsigned long int
    char *endptr;
    unsigned long long followers = strtoull(followerCount, &endptr, 10);
    
    //If not a number or if its a negative number return error message
    if (*followerCount == '\0' || *endptr != '\0' || followers < 0) {
        printf("Error: Follower count must be a positive number.\n");
        return;
    }
    
    //Stop if the handle is too long because our databaase can only handle strings of 31 characters
    if(strlen(handle) > 31) {
        printf("Error : handle too long.\n");
        return;
    }

    //Make sure the handle is properly formatted
    if(handle[0] != '@') {
        printf("Error: Handle must start with @\n");
        return;
    }


    printf("Comment> ");//Ask the user for the comment

    
    char *comment = NULL;
    size_t length = 0;

    // Get the input from the command line
    getline(&comment, &length, stdin);

    // Find the end of the comment and save it in a variable so that we can use it in our loop
    int endLine = strcspn(comment, "\n");

    if(endLine > 64) {
        printf("Error : comment too long.\n");
        return;
    }

    // Remove the '\n' character from the comment
    comment[endLine] = '\0';
    
    for(int i = 0; i < endLine; i++) {
        if(comment[i] == ',' || comment[i] == '\n' || comment[i] == '\0') {
            printf("Error : comment cannot contain commas.\n");
            return;
        }
    }

    // Create a new record with the given handle and followers count so that we can later add it to db
    Record record;
    strcpy(record.handle, handle);
    record.followerCount = followers;
    record.dateLastModified = time(NULL); // Date last modified is now
    strcpy(record.comment, comment);
    
    db_append(db, &record); //Add the record to the database
}

void update_record(Database *db, char *handle, char *followerCount) {
    
    //Get our record so we can modify it
    Record *record;
    record = db_lookup(db, handle);

    // Convert followers to unsigned long int
    char *endptr;
    unsigned long long followers = strtoull(followerCount, &endptr, 10);
    
    //If not a number or if its a negative number return error message
    if (*followerCount == '\0' || *endptr != '\0' || followers < 0) {
        printf("Error: Follower count must be a positive number.\n");
        return;
    }

    printf("Comment> ");//Ask the user for the comment

    char *comment = NULL;
    size_t length = 0;

    // Get the input from the command line
    getline(&comment, &length, stdin);

    // Find the end of the comment and save it in a variable so that we can use it in our loop
    int endLine = strcspn(comment, "\n");

    if(endLine > 64) {
        printf("Error : comment too long.\n");
        return;
    }

    // Remove the '\n' character from the comment
    comment[endLine] = '\0';
    
    for(int i = 0; i < endLine; i++) {
        if(comment[i] == ',' || comment[i] == '\n' || comment[i] == '\0') {
            printf("Error : comment cannot contain commas.\n");
            return;
        }
    }

    strcpy(record->handle, handle);
    record->followerCount = followers;
    record->dateLastModified = time(NULL); // Date last modified is now
    strcpy(record->comment, comment);

    free(comment); //Make sure to free the buffer to avoid memory leaks

}

int main_loop(Database * db)
{
    int saved = 0; // Flag indicating whether we saved our progress
    while (1) {
        printf("> ");

        char *line = NULL;
        size_t length = 0;

        // Get the input from the command line
        getline(&line, &length, stdin);
        
	int endLine = strcspn(line, "\n");
        line[endLine] = '\0'; // Remove the newline character from the input

        char *inputs[3]; // Split the input by spaces
	inputs[0] = inputs[1] = inputs[2] = NULL; // Initialize the inputs to NULL or else i get segmentation errors
						  // (That bug was almost the end of me)
	
        int i = 0;

        char *input = strtok(line, " ");
        while (input != NULL && i < 3) {
            inputs[i] = input; // Store the input tokens in an array
            input = strtok(NULL, " ");
            i++;
        }

        if (strcmp(inputs[0], "list") == 0) {
            list(db);
	    free(line); //We need to free after each action or else we might get a memory leak

        } else if (strncmp(inputs[0], "add", 3) == 0) {
            
            //Wrong inputs so stop to avoid errors
            if(inputs[1] == NULL || inputs[2] == NULL) {
                printf("Error: Usage: add HANDLE FOLLOWERS\n");
		free(line);
                continue;
            }
            
            //Now check that its not already in the database to avoid said error
            if(db_lookup(db, inputs[1]) != NULL) {
                printf("Error: handle %s already exists.\n", inputs[1]);
		free(line);
                continue;
            }		

            add_record(db, inputs[1], inputs[2]);
            saved = 1; //new unsaved changes
	    free(line);
        } else if (strncmp(inputs[0], "update", 6) == 0) {
		
	    //Wrong inputs so stop to avoid errors
            if(inputs[1] == NULL || inputs[2] == NULL) {
                printf("Error: Usage: update HANDLE FOLLOWERS\n");
		free(line);
                continue;
            }

            //Now check that its not already in the database to avoid said error
            if(db_lookup(db, inputs[1]) == NULL) {
                printf("Error : no entry with handle %s\n", inputs[1]);
		free(line);
                continue;
       	    }

            update_record(db, inputs[1], inputs[2]);
            saved = 1; //new unsaved changes
	    free(line);

        } else if (strcmp(inputs[0], "save") == 0) {
	    //Save is set to database.csv but i want to give the user to save it elsewhere
            if (inputs[1] != NULL) {
                db_write_csv(db, inputs[1]);
		free(line);
            } else {
                db_write_csv(db, "database.csv");
		free(line);
            }

            saved = 0;
        } else if (strcmp(inputs[0], "exit") == 0) {
            if ((saved == 1 && ( inputs[1] == NULL || strcmp(inputs[1], "fr") != 0)) ) {
                printf("Error : you did not save your changes. Use `exit fr ` to force exiting anyway.\n");
		free(line);
            } else {
		free(line);
                break;
            }
        } else {
            printf("Unknown input. Available inputs: list, add, update, save, exit\n");
	    free(line);
        }
    } 
  
    return 0;
}

int main()
{
    Database db = db_create();
    db_load_csv(&db, "database.csv");
    return main_loop(&db);
}

