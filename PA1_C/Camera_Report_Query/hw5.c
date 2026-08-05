// 1: Libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// 2: Constants
#define MAX_LENGTH_RZ 1000
#define SEARCH_ID 0
#define EXACT 0
#define PREV -1
#define NEXT 1
#define MODE 3
#define MONTH_STRING 3
#define MAX_MONTH 12

// 3: Data types
// A car report
typedef struct {
    int camera_ID;
    char RZ[MAX_LENGTH_RZ + 1 + 1];
    char mon[MONTH_STRING + 1 + 1];
    int day;
    int hour;
    int min;
} car_rp;

// An array of car report
typedef struct {
    car_rp* carArr;
    int size;
    int memory;
} car_arr;

// 4: Global variables
// Array of received reports
car_arr carReportArr;
// Months in a year
const char* month_char [MAX_MONTH] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// 5: Functions
// Check validity of RZ
bool validRZ (char* RZ) {
    // printf("Start validRZ\n");
    int length = strlen(RZ);
    // printf("End validRZ\n");
    return length > 0 && length <= 1000;
}

// Check validity of a month
bool validMonth (char* mon) {
    // printf("Start validMonth\n");
    for (int i = 0; i<MAX_MONTH;i++) {
        if (strcmp(mon, month_char[i]) == 0) {
            // printf("End validMonth\n");
            return true;
        }
    }
    // printf("End validMonth\n");
    return false;
}

// Return max_day of the month
int maxDay (char* mon)
{
    // printf("Start maxDay\n");
    const int day [MAX_MONTH] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int days = day[0];

    for (int i = 0; i<MAX_MONTH; i++) {
        if (strcmp(mon, month_char[i])==0) {
            days = day[i];
        }
    }
    // printf("End maxDay\n");
    return days;
}

// Validate a car_rp
bool validCar_rp(car_rp reportA) {
    return validRZ(reportA.RZ) && validMonth(reportA.mon)
        && reportA.day >= 1 && reportA.day <= maxDay(reportA.mon)
        && reportA.hour >= 0 && reportA.hour <= 23
        && reportA.min >= 0 && reportA.min <= 59;
}

// Initiate carReportArr
void initiate (void) {
    // printf("Start initiate\n");
    carReportArr.carArr = (car_rp*) malloc (sizeof(car_rp));
    carReportArr.size = 0;
    carReportArr.memory = 1;
    // printf("End initiate\n");
}

// Free carReportArr
void end(void) {
    free(carReportArr.carArr);
}

// Realloc the carReportArr
void reallocCarArr (void) {
    // printf("Start reallocCarArr\n");
    if (carReportArr.size == carReportArr.memory) {
        int newCapacity = carReportArr.memory*2 + 1;
        carReportArr.carArr = (car_rp*) realloc (carReportArr.carArr, newCapacity * sizeof(car_rp));
        carReportArr.memory = newCapacity;
    }
    // printf("End reallocCarArr\n");
}

// Update a new report to the current list of report
void updateReport (car_rp newRp) {
    // printf("Start updateReport\n");
    carReportArr.carArr[carReportArr.size] = newRp;
    carReportArr.size++;
    // printf("End updateReport\n");
}

// Invalid input
void invalid(void) {
    printf("Invalid input.\n");
}

// Handle initial report inputs
bool initial_input(void) {
    // printf("Start initial_input\n");
    // Check first bracket
    char first_br = '\0';
    if (scanf(" %c", &first_br) != 1 || first_br != '{') {
        return false;
    }
    // Scanf each report
    while (true) {
        car_rp newRp;
        char comma;
        if (scanf (" %d : %1001s %4s %d %d : %d %c", 
            &newRp.camera_ID, newRp.RZ, newRp.mon, 
            &newRp.day, &newRp.hour, &newRp.min, &comma) != 7) {
                //printf("End initial_input: %s\n", newRp.mon);
                return false;
            }
        // Validate the car_rp just received
        if (!validCar_rp(newRp)) {
            //printf("End initial_input\n");
            return false;
        }
        // Realloc main array for these reports as needed
        reallocCarArr();
        // Update to the main array
        updateReport(newRp);
        // Check final bracket
        char second_br = '}';
        if (comma == second_br) {
            break;
        }
        // Check invalid characters
        if (comma != ',') {
            //printf("End initial_input\n");
            return false;
        }
    }
     //printf("End initial_input\n");
    return true;
}

// Compare two reports based on RZ and time
bool matchRZ(car_rp A, car_rp B) {
    return (strcmp(A.RZ, B.RZ) == 0);
}

// Compare two reports based on exact time
bool exactTime(car_rp A, car_rp B) {
    // printf("Start matchTime\n");
    // printf("End matchTime\n");
    return (strcmp(A.mon, B.mon) == 0) && (A.day == B.day) 
    && (A.hour == B.hour) && (A.min == B.min);
}

// Compare months
// If A is later => positive. A is earlier => negative. A B same => 0.
int compMonth (char* monA, char* monB) {
    // Value monA
    int A = 0, B = 0;
    for (int i = 0; i<MAX_MONTH;i++) {
        if (strcmp(monA, month_char[i])==0) {
            A = i;
            break;
        }
    }
    for (int i = 0; i<MAX_MONTH;i++) {
        if (strcmp(monB, month_char[i])==0) {
            B = i;
            break;
        }
    }
    return A-B;
}

// See if report A is found earlier than report B
bool prevTime (car_rp A, car_rp B) {
    // printf("Start prevTime\n");
    // printf("End prevTime\n");
    return (compMonth(A.mon, B.mon) < 0) 
    || ((compMonth(A.mon, B.mon) <= 0) && (A.day < B.day)) 
    || ((compMonth(A.mon, B.mon) <= 0) && (A.day <= B.day) && (A.hour < B.hour)) 
    || ((compMonth(A.mon, B.mon) <= 0) && (A.day <= B.day) && (A.hour <= B.hour) && (A.min < B.min));
}

// See if report A is found later than report B
bool nextTime (car_rp A, car_rp B) {
    // printf("Start nextTime\n");
    // printf("End nextTime\n");
    return (!prevTime(A, B) && !exactTime(A, B));
}

// Modes of search
bool (*ptr[MODE]) (car_rp, car_rp) = {exactTime, prevTime, nextTime};

// Print the list of reports found
void printList (int numRpFound, car_rp* rpList) {
    printf("%s %d %02d:%02d, %dx [", 
            rpList[0].mon, rpList[0].day, 
            rpList[0].hour, rpList[0].min, numRpFound);
    for (int i = 0; i < numRpFound; i++) {
        if (i == 0) {
            printf("%d", rpList[i].camera_ID);
        }
        else {
            printf(", %d", rpList[i].camera_ID);
        }
    }
    printf("]\n");
}

// Lookup to see if RZ match anywhere, put matched indexes in a list
int RZLookup (car_rp targetRp, int** matchRZIdx) {
    // printf("Start RZLookup\n");
    int cnt = 0;
    int cap = 1;
    for (int i=0; i<carReportArr.size; i++) {
        car_rp currentRp = carReportArr.carArr[i];
        if (matchRZ(currentRp, targetRp)) {
            if (cnt == cap) {
                cap = cap*2 + 1;
                *matchRZIdx = (int*) realloc(*matchRZIdx, cap * sizeof(int));
            }
            (*matchRZIdx)[cnt++] = i; 
        }
    }
    // printf("End RZLookup\n");
    return cnt;
}

// Sort a list of car_rp based on the camera IDs
void cameraIDSort (int size, car_rp* rpList, car_rp* finalList) {
    // Create a new list
    for (int i = 0; i< size; i++) {
        finalList[i] = rpList[i];
    }
    // Sort the new list
    for (int j = 0; j<size - 1 ; j++) {
        for (int i=0;i<size - 1 ;i++) {
            if (finalList[i].camera_ID > finalList[i+1].camera_ID) {
                int tmp = finalList[i].camera_ID;
                finalList[i].camera_ID = finalList[i+1].camera_ID;
                finalList[i+1].camera_ID = tmp;
            }
        }
    }

}

// Sort from earliest to latest or latest to earliest depend on mode
// If mode = 1 => find earliest => use nextTime, if mode = 2 => find latest => use nextTime 
// Overwrite the original arrays with the new values
// Return number of values of same value that is closest
int timeSort (car_rp** rpList, int length, int mode) {
    int cnt = 0;
    car_rp currentRp = *rpList[0];
    
    // Find the earliest/latest report
    for (int i=0; i<length; i++) {
        if (ptr[mode]((*rpList)[i], currentRp)) {
            currentRp = (*rpList)[i];
        }
    }

    // Find all reports with the same closest date
    // Overwrite the old list with car_rp of closese dates
    for (int i=0; i<length;i++) {
        if (exactTime((*rpList)[i], currentRp)) {
            (*rpList)[cnt] = (*rpList)[i]; 
            cnt++;
        }
    }
    //B=currentRp;
    //printf("List 4 - Same Value to the Closest Date: %s %d %s %d %d:%d\n", B.RZ, B.camera_ID, B.mon, B.day, B.hour, B.min);
    //for (int i=0; i<cnt; i++) {
    //    car_rp A = (*rpList)[i];
    //    printf("Car Report %d: %s %d %s %d %d:%d\n", i, A.RZ, A.camera_ID, A.mon, A.day, A.hour, A.min);
    //}

    // Return number of values of same value that is closest
    return cnt;
}

// Collect the reports from an input list to an output list
// Based on 3 mode: exact, prev, or next
// Return number of duplicate reports
int collect (car_rp targetRp, int input_cnt, int* matchedRZIdx, car_rp** rpList, bool (*ptr)(car_rp, car_rp)) {
    //printf("Start collect\n");
    int output_cnt = 0;
    int cap = 1;
    for (int i=0; i<input_cnt;i++) {
        car_rp currentRp = carReportArr.carArr[matchedRZIdx[i]];
            if (ptr(currentRp, targetRp)) {
                if (output_cnt == cap) {
                    cap = cap*2 + 1;
                    *rpList = (car_rp*) realloc(*rpList, cap * sizeof(car_rp));
                }
                (*rpList)[output_cnt++] = currentRp;
            }
    }
    //car_rp A = targetRp;
    //printf("List 3 - Exact/Earlier/Later to %d %s %s %d %d:%d\n", A.camera_ID, A.RZ, A.mon, A.day, A.hour, A.min);
    //for (int i=0; i<output_cnt;i++) {
    //    A = (*rpList)[i];
    //    printf("Car Report %d: %d %s %s %d %d:%d\n", i, A.camera_ID, A.RZ, A.mon, A.day, A.hour, A.min);
    //}
    //printf("End collect\n");
    return output_cnt;
}

// Lookup a target car_rp by searching through the list of car_rp with matched RZ
// If a rp with an exact date is found, call collect() to grab all reports with the same date
// Then, sort the report list by cameraIDs through cameraIDSort()
// Then call printList() to print all list elements
// On the other hand, if an exact date is not found, call collect() twice, grab all reports with prev and next dates
// Use timeSort() to find the closest dates and collect all reports with the same closest date
// Then use cameraIDSort() and printList() as usual
void lookup (car_rp targetRp, int matchedCnt, int* matchRZIdx) {
    //printf("Start Lookup\n");
    bool prev = false, next = false;
    const char* phrase[MODE] = {"> Exact: ", "> Previous: ", "> Next: "};
    for (int j = 0; j<MODE; j++) {
        for (int i=0; i<matchedCnt;i++) {
            car_rp* rpList;
            rpList = (car_rp*) malloc (sizeof(car_rp));
            car_rp currentRp = carReportArr.carArr[matchRZIdx[i]];
            if (ptr[j](currentRp, targetRp)) {
                if (j==2 && !prev) {printf("> Previous: N/A\n");}
                printf("%s", phrase[j]);
                int size = collect(targetRp, matchedCnt, matchRZIdx, &rpList, ptr[j]);
                if (j == 1 || j == 2) {
                    size = timeSort(&rpList, size, 3-j);
                }
                car_rp* finalList = (car_rp*) malloc (size * sizeof(car_rp));
                cameraIDSort(size, rpList, finalList);
                printList(size, finalList);
                free(finalList);
                if (j==0) {
                //    printf("End lookup\n");
                    free(rpList);
                    return;
                }
                if (j==1) {
                    prev = true;
                    free(rpList);
                    break;
                }
                if (j==2) {
                    next = true;
                    free(rpList);
                    break;
                }
            }
            free(rpList);
        }
    }
    if (prev && !next) {printf("> Next: N/A\n");}
    return;
}

// Print no result is found
void printNone(void) {
    printf("> Car not found.\n");
}

// Handle search report input
bool search_input (void) {
    // printf("Start search_input\n");
    // Get the user input
    car_rp targetRp;
    targetRp.camera_ID = SEARCH_ID;
    if (scanf (" %1001s %4s %d %d : %d ", 
                targetRp.RZ, targetRp.mon, 
                &targetRp.day, &targetRp.hour, &targetRp.min) != 5) {
                return false;
            }

    // Validate the receive car_rp
    if (!validCar_rp(targetRp)) {
        invalid();
        return false;
    }

    // Search up the RZ from the database, put matches in a list
    int* matchRZIdx;
    matchRZIdx = (int*) malloc (sizeof(int));
    int matchedCnt = 0;
    if (!(matchedCnt = RZLookup(targetRp, &matchRZIdx))) {
        free(matchRZIdx);
        printNone();
        return true;
    }
    //printf("List 2 - Matched RZ: %d\n", matchedCnt);
    //for (int i=0; i<matchedCnt; i++) {
    //    car_rp currentRp = carReportArr.carArr[matchRZIdx[i]];
    //    int camera_ID = currentRp.camera_ID;
    //    char* RZ = currentRp.RZ;
    //    char* mon = currentRp.mon;
    //    int day = currentRp.day;
    //    int hour = currentRp.hour;
    //    int min = currentRp.min;
    //    printf("Car Report %d: %i %s %s %d %d:%d\n", 
    //            i, camera_ID, RZ, mon, day, hour, min);
    //}
    // Search up to see if there's reports with exact date
    // If not the prev and next dates
    lookup(targetRp, matchedCnt, matchRZIdx);
    free(matchRZIdx);
    return true;
}

// 6: Main
int main () {
    // Getting user input
    printf("Camera reports:\n");
    initiate();
    if (!initial_input()) {
        invalid();
        end();
        return EXIT_FAILURE;
    }
    //printf("List 1 - Total input: %d\n", carReportArr.size);
    //for (int i=0; i<carReportArr.size; i++) {
    //    car_rp A = carReportArr.carArr[i];
    //    printf("Car Report %d: %d: %s %s %d %d:%d\n", 
    //    i, A.camera_ID, A.RZ, A.mon, A.day, A.hour, A.min);
    //}
    // Searching user input
    printf("Search:\n");
    while (true) {
        if (!search_input()) {
            end();
            return EXIT_FAILURE;
        }
    }
    end();
    return EXIT_SUCCESS;
}