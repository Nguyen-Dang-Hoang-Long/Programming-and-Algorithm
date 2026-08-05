// 1. Libaries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

// 2. Constants
#define DNA_SUBSTRING 3
#define DNA_SEARCH 0.0
#define MAX_LIST 50

// 3. Structs
// A DNA Sample
typedef struct {
    double frq;
    char* sqArr;
    int size;
    int memory;
} DNA;

// A DNA Database
typedef struct {
    DNA* sampleArr;
    int size;
    int memory;
} DNADatabase;

// 4. Global variables
bool empty_line = true;
bool empty_search = true;
char validDNA[4] = {'A', 'T', 'C', 'G'};
DNADatabase unsortedDatabase;
DNADatabase sortedDatabase;

// 5. Custom Functions
// Return the min value of the two
int min (int a, int b) {
    if (a<b) return a;
    else return b;
}
// Read one line from stdin
bool readLine (char** buffer) {
    // Initialize variables to read the input
    *buffer = NULL;
    size_t size = 0;
    ssize_t char_count; 
    // If the input is not empty, read the line
    if ((char_count = getline(buffer, &size, stdin)) != -1) {
        // Stop reading if only '\n' is found
        if (char_count == 1 && (*buffer)[0] == '\n') {
            return false;
        }
        // Detect signs
        if ((*buffer)[0] == '+' || (*buffer)[0] == '-'){
            return false;
        }
    }
    // Return once the line is read
    // If the input is empty
    if (char_count == -1) {
        return false;
    }
    // Else
    return true;
}
// Fully malloc an DNA
void initiateDNA(DNA** newDNA, size_t len) {
    (*newDNA)->sqArr = (char*) malloc ((len+1) * sizeof(char));
    (*newDNA)->memory = len + 1;
    (*newDNA)->size = len;
}
// Malloc an DNA
//void initiateDNA(DNA** newDNA) {
//    (*newDNA)->sqArr = (char*) malloc (sizeof(char));
//    (*newDNA)->memory = 1;
//    (*newDNA)->size = 0;
//}

// Create the newDNA one char at a time
//void updateDNA (DNA* newDNA, char newChar) {
//    int size = newDNA->size;
//    newDNA->sqArr[size] = newChar;
//    newDNA->size++;
//}
// Realloc an DNA
//void reallocDNA(DNA** newDNA) {
//    if ((*newDNA)->size + 1 >= (*newDNA)->memory) {
//        int newCap = ((*newDNA)->memory)*2 + 1;
//        (*newDNA)->sqArr = (char*) realloc ((*newDNA)->sqArr, newCap*sizeof(char));
//        (*newDNA)->memory = newCap;
//    }
//}
// Free an DNA
void freeDNASqArr (DNA** newDNA) {
    if ((*newDNA) != NULL && (*newDNA)->sqArr != NULL) {
    free((*newDNA)->sqArr);
    (*newDNA)->sqArr = NULL;
    }
}
// Validate the sequence, one char at a time
bool validateDNA (char DNA) {
    for (int i = 0; i < 4; i++) {
        if (DNA == validDNA[i]) {
            return true;
        }
    }
    //printf("False - %c\n", DNA);
    return false;
}
// Process the DNA part of the string
bool processDNA(char* string, size_t len, DNA** newDNA) {
    // Scan through the DNA part, one char at a time
    //printf("String is %s\n", string);
    for (size_t i = 0; i < len; i++) {
            char c = string[i];
            
            // Validate it
            if (!validateDNA(c)) {
                //printf("Wrong c is %c", c);
                //if (c == '\0') {
                //   printf("null terminator ");
                //}
                //printf("\n");
                return false;
            }
            // Update the DNA
            (*newDNA)->sqArr[i] = c;
    }
    // Add the final null terminator
    (*newDNA)->sqArr[len] = '\0';
    return true;
}
// Read the frequency head at the start of the string
bool processFrq(char* string, double* frq, int* ptr, DNA** newDNA) {
    // Initialize variables to parse the start of input
    char colon;
    // Check for a double and a colon, return false if not
    if (sscanf(string, "%lf%c%n", frq, &colon, ptr) != 2 || colon != ':') {
        return false;
    }
    // Update it to the new DNA
    (*newDNA)->frq = *frq;
    // Else return true
    return true;
}
// See if the string length is a multiple of 3
bool multipleOf3 (char* buffer) {
    //printf("%ld is multiple of 3?", strlen(buffer));
    return (strlen(buffer) % 3 == 0);
}
// Malloc any database
void mallocDatabase (void) {
    unsortedDatabase.sampleArr = (DNA*) malloc (sizeof(DNA));
    unsortedDatabase.memory = 1;
    unsortedDatabase.size = 0;
}
// Malloc the database
//void mallocDatabase (void) {
//    database.sampleArr = (DNA*) malloc (sizeof(DNA));
//    database.memory = 1;
//    database.size = 0;
//}
// Realloc the database
void reallocDatabase (void) {
    size_t size = (size_t) unsortedDatabase.size;
    size_t memory = (size_t) unsortedDatabase.memory;
    if (size + 1 >= memory) {
        size_t newCap = (memory * 2) + 1;
        unsortedDatabase.sampleArr = (DNA*) realloc (unsortedDatabase.sampleArr, newCap * sizeof(DNA));
        unsortedDatabase.memory = (int) newCap;
    }
}
// Free the database
void freeDatabase (DNADatabase database) {
    if (database.sampleArr != NULL) {
        for (int i = 0; i < database.size; i++) {
            free(database.sampleArr[i].sqArr);
        }
        free(database.sampleArr);
    }
}
// Update the database with the new DNA
void updateDatabase (DNA* newDNA) {
    // Transfer frq
    int size = unsortedDatabase.size;
    unsortedDatabase.sampleArr[size].frq = newDNA->frq;
    // Transfer sqArr
    int memory = newDNA->memory;
    unsortedDatabase.sampleArr[size].sqArr = (char*) malloc (memory * sizeof(char));
    memcpy(unsortedDatabase.sampleArr[size].sqArr, newDNA->sqArr, memory);
    // Transfer size and memory
    unsortedDatabase.sampleArr[size].size = newDNA->size;
    unsortedDatabase.sampleArr[size].memory = newDNA->memory;
    // Update database size
    unsortedDatabase.size++;
}
// Printf invalid input and a string to indicate the type of problem
void invalid(void) {
    printf("Invalid input.\n");
}
// Lookup the database, return number of match, also update an array of matched indexes
int lookup (DNA* substring, size_t search_len, int* matchedIdx) {
    //printf("Search_len: %d\n", search_len);
    int found = 0;
    char* searchStr = substring->sqArr;
    // Cycle through the databse
    for (int i = 0; i < sortedDatabase.size; i++) {
        size_t sample_len = strlen(sortedDatabase.sampleArr[i].sqArr);
        size_t cycle = sample_len - search_len;
        if (sample_len < search_len) {
            continue;
        }
        //double databaseFrq = database.sampleArr[i].frq;
        //printf("Find %s in %s.\n", searchStr, database.sampleArr[i].sqArr);
        //printf("DatabaseStr_len: %d\n", sample_len);
        // Cycle through each DNA Sample
        for (size_t j = 0; j <= cycle; j+=3) {
            char* databaseStr = sortedDatabase.sampleArr[i].sqArr;
            if (strncmp(searchStr, databaseStr + j, search_len) == 0) {
                //printf("Found: %s match %g:%s in database.\n", searchStr, databaseFrq, databaseStr);
                matchedIdx[found++]=i;
                break;
            }
        }
    }
    return found;
}
// Function to compare frequencies
int compareFrqUnsorted (const void* a, const void* b) {
    // Cast int ptr
    const int idx1 = *(const int *)a;
    const int idx2 = *(const int *)b;
    // Get frq from database using matchedIdx
    const double frq1 = unsortedDatabase.sampleArr[idx1].frq;
    const double frq2 = unsortedDatabase.sampleArr[idx2].frq;
    // Return the value in descending order
    if (frq1 < frq2) {
        return 1;
    }
    else if (frq1 > frq2) {
        return -1;
    } else
    {
        return 0;
    }
}
// QSort the matchedIdx
//void qSortMatchedIdx (int* matchedIdx, int found) {
//    qsort(matchedIdx, found, sizeof(int), compareFrq);
//}

// Qsort an Index from unsortedDatabase
void qSortIdx (int* IdxList, int found) {
    qsort(IdxList, found, sizeof(int), compareFrqUnsorted);
}
// Sort the matchedIdx
//void sortMatchedIdx (int* matchedIdx, int found){
//    for (int j = 0; j < found; j++) {
//        for (int i = 0; i < found-1-j; i++) {
//            int idx1 = matchedIdx[i];
//            int idx2 = matchedIdx[i+1];
//            if (database.sampleArr[idx1].frq < database.sampleArr[idx2].frq) {
//                matchedIdx[i] = idx2;
//                matchedIdx[i+1] = idx1;
//            }
//        }
//    }
//}

// Copy one DNA to another
void transferDNA(DNA* des, const DNA* src) {
    // Transfer frq, size, memory of each DNA
    des->frq = src->frq;
    des->size = src->size;
    des->memory = src->memory;
    // Malloc the new string
    des->sqArr = (char*) malloc(src->memory * sizeof(char));
    // Transfer string
    memcpy(des->sqArr, src->sqArr, src->memory);
}
// Copy the unsortedDatabase to the sortedDatabase
void transferDatabase(void) {
    // Transfer the unsorted database to the sorted one
    int dtbSize = unsortedDatabase.size;
    int* sortedIdx;
    sortedIdx = (int*) malloc (dtbSize * sizeof(int));
    // Populate the index array
    for (int i = 0; i < dtbSize; i++) {
        sortedIdx[i] = i;
    }
    // Sort the index array based on the unsortedDatabase
    qSortIdx(sortedIdx, dtbSize);
    // Populate the sortedDatabase with the sorted index array
    int memory = unsortedDatabase.memory;
    sortedDatabase.memory = memory;
    sortedDatabase.sampleArr = (DNA*) malloc (memory * sizeof(DNA));
    sortedDatabase.size = dtbSize;
    for (int i = 0; i < dtbSize; i++) {
        int idx = sortedIdx[i];
        transferDNA(&sortedDatabase.sampleArr[i], &unsortedDatabase.sampleArr[idx]);
    }
    free(sortedIdx);
}

// 6. Main
int main (void) {
    // Initiate the DNA unsortedDatabase
    mallocDatabase();
    // Main Function 1: Parsing inputs
    printf("DNA database:\n");
    // Line buffer
    char* buffer;
    while (readLine(&buffer)) {
        // Update line count
        empty_line = false;
        // DNA buffer
        DNA* newDNA;
        newDNA = (DNA*) malloc (sizeof(DNA));
        // Frq buffer 
        double frq = 0.0;
        //printf("Line read: %s", buffer);
        // Parsing frequency head of string
        int ptr;
        if (!processFrq(buffer, &frq, &ptr, &newDNA))
        {
            invalid();
            free(buffer);
            free(newDNA);
            freeDatabase(unsortedDatabase);
            return EXIT_FAILURE;
        }
        //printf("Frq read: %lf\n", newDNA->frq);
        buffer[strlen(buffer) - 1] = '\0';
        size_t DNALen = strlen(buffer + ptr);
        // Check if the DNA string is empty
        if (DNALen == 0) {
            invalid();
            free(buffer);
            free(newDNA);
            return EXIT_FAILURE;
        }
        // Check if the DNA string is of multiples of 3
        if (!multipleOf3(buffer + ptr)) {
            invalid();
            free(buffer);
            free(newDNA);
            freeDatabase(unsortedDatabase);
            return EXIT_FAILURE;
        }
        // Repeatedly read, validate, and add the DNA string to a newDNA, one char at a time
        //printf("DNA:");
        initiateDNA(&newDNA, DNALen);
        if (!processDNA(buffer + ptr, DNALen, &newDNA)) {
            free(buffer);
            //printf("Process DNA input\n");
            invalid();
            freeDNASqArr(&newDNA);
            free(newDNA);
            freeDatabase(unsortedDatabase);
            return EXIT_FAILURE;
        }
        // Add the new sample to the database, realloc as needed
        reallocDatabase();
        updateDatabase(newDNA);
    // Free malloc memories
        free(buffer);
        buffer = NULL;
        freeDNASqArr(&newDNA);
        free(newDNA);
    }
    free(buffer);
    // If no line exists except the empty line
    if (empty_line) {
        invalid();
        freeDatabase(unsortedDatabase);
        return EXIT_FAILURE;
    }
    // Check
    //printf("Database has %d DNA Samples.\n", database.size);
    //for (int i = 0; i < database.size; i++) {
    //    printf("DNA %d: %g:%s. Size: %d Memory: %d\n", i, 
    //        database.sampleArr[i].frq, database.sampleArr[i].sqArr, 
    //        database.sampleArr[i].size, database.sampleArr[i].memory);
    //}

    // Transfer the unsortedDatabase to the sortedDatabase
    transferDatabase();
    freeDatabase(unsortedDatabase);

    // Main Function 2: Search algorithm
    printf("Searches:\n");
    // Read search input
    char* search;
    while (true) {
        if (!readLine(&search)) {
            break;
        }
        // Update search count
        empty_search = false;
        // Validate the search
        search[strlen(search) - 1] = '\0';
        size_t search_len = strlen(search);
        DNA* substring;
        substring = (DNA*) malloc (sizeof(DNA));
        initiateDNA(&substring, search_len);
        substring->frq = DNA_SEARCH;
        //printf("Search string is %s\n", search);
        if (!multipleOf3(search) || !processDNA(search, search_len, &substring)) {
            invalid ();
            //printf("Process DNA search\n");
            free(search);
            freeDNASqArr(&substring);
            free(substring);
            freeDatabase(sortedDatabase);
            return EXIT_FAILURE;
        }
        // Check
        //printf("Search: %s\n", substring->sqArr);
        search_len = strlen(substring->sqArr);
        // Look through the database the desired substring
        int* matchedIdx;
        matchedIdx = (int*) malloc (sortedDatabase.size * sizeof(int));
        int found = lookup(substring, search_len, matchedIdx);
        printf("Found: %d\n", found);
        //qSortIdx(matchedIdx, found);
        //printf("MatchedIdx for %s:\n", substring->sqArr);
        int tmp = min(MAX_LIST, found);
        for (int i = 0; i < tmp; i++) {
            int idx = matchedIdx[i];
            printf("> %s\n", sortedDatabase.sampleArr[idx].sqArr);
        }

        // Free malloc memories
        free(matchedIdx);
        free(search);
        freeDNASqArr(&substring);
        free(substring);
    }
    // If no search was input
    if (empty_search) {
        invalid();
        free(search);
        freeDatabase(sortedDatabase);
        return EXIT_FAILURE;
    }
    // Exit success
    free(search);
    freeDatabase(sortedDatabase);
    return EXIT_SUCCESS;
}