// #1: Libraries
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// #2: Constants
// There should be at least 2 sections and at most 10000 sections
#define MIN_SECTION 2
#define MAX_SECTION 10000
// The array to take everything in the first time is 10x size of the number of sections (+1 for '\0')
#define MAX_ARRAY (MAX_SECTION*3)
// At most, there can be 300000 updates
#define MAX_UPDATE 300000

// #3: Extra Data Structures
// Date
typedef struct {
    long long unsigned year;
    long long unsigned month;
    long long unsigned day;
} DATE;

// A cost-date update info pair of a certain update
typedef struct {
    DATE iniDate;
    long long unsigned newCost;
} FUTURE_COST;

// The cost profile of a certain section: With original cost, update_count for number of updates
// memory capacity (self-explain), and an array of updated cost
typedef struct {
    long long unsigned origin_cost;
    long long unsigned update_count;
    long long unsigned memory_capacity;
    FUTURE_COST* futureCostArr;
} COST_PROFILE;

// Date-based structure with info of the section affected and cost changed
typedef struct {
    DATE upDate;
    long long unsigned section;
    long long unsigned newCost;
} FUTURE_UPDATE;

// Array of date-based updates, keeping track of all updates
typedef struct {
    FUTURE_UPDATE updateArr[MAX_UPDATE];
    long long unsigned max_update;
} UPDATE_PROFILE;

// A supersection to be given to a certain company A or B
typedef struct {
    long long unsigned start;
    long long unsigned end;
} SUPER_SECTION;

// An option to give which company which sections
typedef struct {
    long long unsigned difference;
    SUPER_SECTION A;
    SUPER_SECTION B;
} OPTION;

// #4: Global variables
// Count of sections
long long unsigned x = 0;
long long unsigned* cnt = &x;
// Array of sections costs
long long unsigned SECTION_ARRAY[MAX_SECTION] = {0};
// The masterplan array of sections
COST_PROFILE* masterArr;
// The masterplan array of updates
UPDATE_PROFILE masterUpdate;
// Update checking array
long long unsigned y = 0;
long long unsigned* updateCheck = &y;
// Array of options
OPTION* optionArr;

// #5: Functions

void free_masterArr (void) {
    for (long long unsigned i = 0; i < *cnt; i++) {
        if (masterArr[i].futureCostArr != NULL) {
            free(masterArr[i].futureCostArr);
            masterArr[i].futureCostArr = NULL;
        }
    } 
    if (masterArr != NULL) {
        free(masterArr);
        masterArr = NULL;
    }
}
// Clear input buffer
void clear_input_buffer (void) {
    int a;
    while((a = getchar()) != '\n' && a != EOF){}
}

// makeDate
DATE makeDate ( long long unsigned y,
                 long long unsigned m,
                 long long unsigned d )
{
  DATE res = { y, m, d };
  return res;
}

// Check whether the first date is later than the second date
bool greaterDate (DATE later,
                  DATE earlier)
{
  return ((later . year >  earlier . year)
      || ((later . year >= earlier . year) && ((later . month > earlier . month)))
      || ((later . year >= earlier . year) && (later . month >= earlier. month) && (later . day > earlier . day)));
}

// Opposite of greaterDate, usage: for better code comprehension
bool lesserDate (DATE earlier, DATE later) {
    return greaterDate(later, earlier);
}

// Check whether the two date are equal
bool  equalDate ( DATE a,
                  DATE b )
{
  return a . year == b . year
         && a . month == b . month
         && a . day == b . day;
}

// Calculate leapyear
bool leapYear  (DATE date)
{
    if (date . year % 4000 == 0) {
      return false;
    }

    if (date . year % 400 == 0){
      return true;
    }

    if (date . year % 100 == 0){
      return false;
    }

    if (date . year % 4 == 0){
      return true;
    }

    return false;
}

// Calculate maxday of a month
long long unsigned maxDay (DATE date)
{
    long long unsigned day [12] = {31, (28 + (unsigned long long)leapYear (date)), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    long long unsigned days = 31;
    for (long long unsigned i = 0; i < 12; i++)
    {
        if (date . month == i + 1)
            days = day[i];
    }
    return days;
}

// Validate a date
bool validate (DATE date)
{
  return (date . day >= 1 && date . day <= maxDay (date) 
  && date . month >= 1 && date . month <= 12 && date . year >= 1900); 
}

// Increase a date by 1 day
void increDate (DATE *date)
{
    if (date->day < maxDay(*date)) {
    date->day++;
  }
  else if (date->day == maxDay(*date)) {
    if (date->month < 12) {
    date->month++;
    }
    else {
        date->month = 1;
        date->year++;
    }
    date->day = 1;
  }
}

// Decrease a date by 1 day
void decreDate (DATE *date)
{
    if (date->day > 1) {
    date->day--;
  }
  else if (date->day == 1) {
    if (date->month > 1) {
        date->month--;
    }
    else {
        date->month = 12;
        date->year--;
    }
    date->day = maxDay(*date);
  }
}

// Count dates
long long unsigned countDate (DATE from, DATE to) {
    long long unsigned buffer = 0;
    DATE now, future;
    now = from;
    future = to;
    while (lesserDate(now, future) == true || equalDate(now,future) == true) {
        buffer++;
        increDate(&now);
    }
    return buffer;
}

// Check for both brackets
bool bracketCheck (char * original_arr) {
    //printf("start bracketCheck\n");
    if (sscanf(original_arr, " { %100000[^}]", original_arr) != 1) {
    //    printf("end bracketCheck\n");
        return false;
    }
    //printf("end bracketCheck\n");
    return true;
}

// Count sections, check commas, check invalid characters, update *cnt and INITIAL_COST of sections
bool countSection (char* original_arr, long long unsigned* cnt, long long unsigned SECTION_ARRAY[]) {
    //printf("start countSection\n");
    // Set up buffers for brackets, commas, sections, invalid chars, and pointer variables to track sscanf
    char comma = '\0', random = '\0';
    long long unsigned section = 0;
    const char * ptr = original_arr;
    int ptr_position = 0;

    // Sscanf for pairs of comma - section
    while (sscanf(ptr, " %llu %c%n", &section, &comma, &ptr_position) == 2) {
        if (comma != ',') {
            printf("Invalid input.\n");
            fflush(stdout);
            return false;
        }
        if (*cnt >= MAX_SECTION) {
            printf("Invalid input.\n");
            fflush(stdout);
            return false;
        }
        (*cnt)++;
        SECTION_ARRAY[*cnt - 1] = section;
        comma = '\0';
        ptr += ptr_position;
    }

    //Sscanf for the last section
    if (sscanf(ptr, "  %llu%n", &section, &ptr_position) != 1) {
        char random = '\0';
        if ((sscanf(ptr, " %c%n", &random, &ptr_position) == 1) 
        && random != ' ' && random !=  '\n' && random != '\t' && random != '\r') {
            printf("Invalid input.\n");
            fflush(stdout);
            return false;
        }
    }
    (*cnt)++;
    SECTION_ARRAY[*cnt - 1] = section;
    ptr+=ptr_position;

    // Check for random char characters within the string
    if (sscanf(ptr, " %c%n", &random, &ptr_position) == 1) {
        printf("Invalid input.\n");
        fflush(stdout);
        return false;
   }
   //printf("end countSection\n");
   return true;
}

// From an array of values, generate all possible OPTIONS - Including 2 SUPERSECTION (start and end) to 2 companies
// such that the start and end are contiguous
void variableSubset (long long unsigned inputArr[], const long long unsigned *cnt, OPTION * outputArr) {
    long long unsigned n = *cnt;
    long long unsigned* circularArr;
    circularArr = (long long unsigned* ) malloc ((n*2)*sizeof(long long unsigned));
    if (circularArr == NULL) {
        return;
    }
    for (long long unsigned k=0; k<2*n; k++) {
        circularArr[k] = inputArr[k % n]; 
    }
    //printf("Circular Arr: ");
    //for (long long unsigned k=0; k<2*n; k++) {
    //    printf("%i ", circularArr[k]);
    //}
    //printf("\n");
    long long unsigned a=0;
    long long unsigned total = 0;
    for (long long unsigned k=0; k<n;k++) {
        total += inputArr[k];
    }
    for (long long unsigned k=0; k<n;k++) {
        for (long long unsigned i = k; i<n+k;i++) {
    //        printf("i: %i\n", i);
            long long unsigned zero = 0;
            long long unsigned startA = k%n;
            long long unsigned endA = k;
            long long unsigned sum1 = 0;
    //        printf("[ ");
            for (long long unsigned j = k; j < i; j++) {
                endA = j%n;
                sum1 += circularArr[j];
    //            printf("%i", j%n);
                zero=1;
            }
    //        printf(" ] - ");
            
            long long unsigned startB = (endA + 1) % n;
            long long unsigned endB = (startA + n - 1) % n;
    //        long long unsigned sum2 = total - sum1;
            long long unsigned diff = llabs((long long)(sum1*2 - total));
            if (zero==1) {
    //            printf("A[%i%i] - B[%i%i] - ", startA, endA, startB, endB);
    //            printf("sum1: %i - sum2: %i - diff: %i\n", sum1, sum2, diff);
                outputArr[a].A.start = startA;
                outputArr[a].A.end = endA;
                outputArr[a].B.start = startB;
                outputArr[a].B.end = endB;
                outputArr[a].difference = diff;
                a++;
            }
        }
    }
    free(circularArr);
    return;
}

// Find min value
long long unsigned min (long long unsigned a, long long unsigned b) {
return (a<b) ? a: b;
}

// Print only the best options
void printOptions(OPTION* optionArr, long long unsigned optionLen) {
    if (optionLen == 0) {
        return;
    }
    long long unsigned min = 0;
    OPTION* minOption = (OPTION*) malloc (sizeof(OPTION));
    if (minOption == NULL) {
        return;
    }
    // Look for the smallest value
    for (long long unsigned i = 0; i < optionLen; i++) {
        if (optionArr[i].difference <= optionArr[min].difference) {
            min = i;
        }
    }
    // Look for values with the same smallest value, put it in an array
    long long unsigned count = 0;
    for (long long unsigned i = 0; i < optionLen; i++) {
        if (optionArr[i].difference == optionArr[min].difference) {
            count++;
            OPTION* reallocMinOption = (OPTION*) realloc (minOption, (count+1)*sizeof(OPTION));
            if (reallocMinOption == NULL) {
                free(minOption);
                return;
            }
            minOption = reallocMinOption;
            minOption[count-1].difference = optionArr[i].difference;
            minOption[count-1].A.start = optionArr[i].A.start;
            minOption[count-1].A.end = optionArr[i].A.end;
            minOption[count-1].B.start = optionArr[i].B.start;
            minOption[count-1].B.end = optionArr[i].B.end;

        }
    }
    // Print only values that are not a permutation of other values
    if (count == 0) {
        return;
    }
    long long unsigned diff = minOption[0].difference;
    printf("Difference: %llu, options: %llu\n", diff, count/2);
    fflush(stdout);
    for (long long unsigned i=0; i < count; i++) {
        bool copy = false;
        // Ignore values that are in a permutation of other
        for (long long unsigned j = i+1; j < count; j++) {
            if (minOption[i].A.start == minOption[j].B.start && minOption[i].A.end == minOption[j].B.end) {
                copy = true;
            }
        }
        if (copy == false) {
            printf("* %llu - %llu, %llu - %llu\n", minOption[i].A.start, minOption[i].A.end, 
                                           minOption[i].B.start, minOption[i].B.end);
            fflush(stdout);
        }
    }
    free(minOption);
    return;
}

// #6: Main
int main() {
    // Prompt user
    printf("Daily cost:\n");
    fflush(stdout);

    // Get the entire array
    char original_arr [MAX_ARRAY] = "";

    // Invalid if lack brackets, or empty input, or first input is invalid
    if (scanf("%100000[^}] }" , original_arr) != 1) {
        printf("Invalid input.\n");
        fflush(stdout);
        return EXIT_FAILURE;
    }
    //printf("String in buffer: %s\n", original_arr);

    // Check brackets
    if (!bracketCheck(original_arr)) {
        printf("Invalid input.\n");
        fflush(stdout);
        return EXIT_FAILURE;
    }
    //printf("String in adjusted buffer: %s\n", original_arr);

    // Count sections and update cnt and INITIAL_COST array
    if (!countSection(original_arr, cnt, SECTION_ARRAY)) {
        return EXIT_FAILURE;
    }

    // Check count of sections, must be at least 2 and at most 10000
    if (*cnt < MIN_SECTION || *cnt > MAX_SECTION ) {
        printf("Invalid input.\n");
        fflush(stdout);
        return EXIT_FAILURE;
    }

    // Check values
    //printf("Success 1\n");
    //for (long long unsigned i = 0; i < *cnt; i++) {
    //    printf("Section %llu:  %llu\n", i, SECTION_ARRAY[i]);
    //}
    //printf("Cnt: %d\n", *cnt);

    //Update the section profiles for each section
    masterArr = (COST_PROFILE*) malloc(*cnt * sizeof(COST_PROFILE));
    if (masterArr == NULL) {
        return EXIT_FAILURE;
    }
    for (long long unsigned i=0; i<*cnt;i++){
        masterArr[i].origin_cost = SECTION_ARRAY[i];
        masterArr[i].futureCostArr = (FUTURE_COST*) malloc(sizeof(FUTURE_COST));
        if (masterArr[i].futureCostArr == NULL) {
            free_masterArr();
            return EXIT_FAILURE;
        }
        masterArr[i].memory_capacity = 1;
        masterArr[i].update_count = 0;
    //    printf("Section  %llu:  %llu\n", i, masterArr[i].origin_cost);
    }

    // Check values
    //printf("Success 2\n");
    //for (long long unsigned i=0; i<*cnt;i++){
    //    printf("Section  %llu:  %llu\n", i, masterArr[i].origin_cost);
    //} 
    DATE firstDate = makeDate(1900,1,1);
    while (true) {
        //printf("start loop\n");
        // Update cost command
        // Buffers and scanf to check input
        DATE upDate; 
        long long unsigned section = 0, cost = 0;
        DATE from, to;
        int scanf_result;
        //printf("start\n");
        scanf_result = scanf(" = %llu-%llu-%llu %llu: %llu", &upDate.year, &upDate.month, &upDate.day, &section, &cost);
        if (scanf_result == 5) {

            // Check other conditions
            if (validate(upDate) != true || section >= *cnt || 
                (greaterDate(upDate, firstDate) != true && equalDate(upDate, firstDate) != true) ) {
                printf("Invalid input.\n");
                fflush(stdout);
                return EXIT_FAILURE;
            }

            // Check    
        //    printf("Data Collected:  %llu- %llu- %llu  Section: %llu  Cost: %llu\n", upDate.year, upDate.month, upDate.day, section, cost);

            // Update the current date
            firstDate = upDate;

            // Update number of updates
            (*updateCheck)++;
            if (*updateCheck >= MAX_UPDATE) {
                printf("Invalid input.\n");
                fflush(stdout);
                return EXIT_FAILURE;
            }
            masterArr[section].update_count++;
            

            // Assign memory to update the new cost-profile of the section
            long long unsigned update_count = masterArr[section].update_count;
            long long unsigned memory = masterArr[section].memory_capacity;
            if (update_count >= memory) {
                FUTURE_COST* reallocMasterArr = (FUTURE_COST*) realloc(masterArr[section].futureCostArr, (memory*2 + 1) * sizeof(FUTURE_COST));
                if (reallocMasterArr == NULL) {
                    free_masterArr();
                    return EXIT_FAILURE;
                }
                masterArr[section].futureCostArr = reallocMasterArr;
                masterArr[section].memory_capacity = masterArr[section].memory_capacity*2 + 1; 
            }
            masterArr[section].futureCostArr[update_count-1].newCost=cost;  
            masterArr[section].futureCostArr[update_count-1].iniDate=upDate;  

            // Update FUTURE_UPDATE AND UPDATE_PROFILE arrays for computation
            masterUpdate.max_update++;
            masterUpdate.updateArr[masterUpdate.max_update - 1].upDate = upDate;
            masterUpdate.updateArr[masterUpdate.max_update - 1].section = section;
            masterUpdate.updateArr[masterUpdate.max_update - 1].newCost = cost;
            
            // Check
            //printf(" - - - - - Data in masterArr - - - - -\n");
            //for (long long unsigned i = 0; i < *cnt; i++) {
                //printf("Section  %llu. Cost:  %llu.  %llu updates.  %llu memory allocated.\n", 
                //    i, masterArr[i].origin_cost, masterArr[i].update_count, masterArr[i].memory_capacity);
                //for (long long unsigned j = 0; j < masterArr[i].update_count; j++) {
                //    printf("Date:  %llu- %llu- %llu. New Cost:  %llu\n", 
                //    masterArr[i].futureCostArr[j].iniDate.year, 
                //    masterArr[i].futureCostArr[j].iniDate.month, 
                //    masterArr[i].futureCostArr[j].iniDate.day, 
                //    masterArr[i].futureCostArr[j].newCost);
                //}
            //}
        //    printf(" - - - - - Data in masterUpdate - - - - - \n");
        //    for (long long unsigned i = 0; i < masterUpdate.max_update; i++) {
        //        DATE dateA = masterUpdate.updateArr[i].upDate; 
        //        long long unsigned sectionA = masterUpdate.updateArr[i].section;
        //        long long unsigned newCostA = masterUpdate.updateArr[i].newCost;
        //        printf("Update  %llu:\nDate:  %llu- %llu- %llu. Section:  %llu. New Cost:  %llu.\n", 
        //            i, dateA.year, dateA.month, dateA.day, sectionA, newCostA);
        //    }
        //    printf(" - - - - - End - - - - - \n");
        continue;
        }

        // Assignment Cost Command: Part 1 - Data Extraction
        // Check correct user input
        scanf_result = scanf(" ? %llu-%llu-%llu  %llu-%llu-%llu", &from.year, &from.month, &from.day, &to.year, &to.month, &to.day);
        if (scanf_result == 6) {
        // Check other conditions
            if (validate(from) != true || validate(to) != true || greaterDate(from,to) == true)
            {
                printf("Invalid input.\n");
                fflush(stdout);
                return EXIT_FAILURE;
            }
            long long unsigned* total_cost;
            total_cost = (long long unsigned*) malloc((*cnt)*sizeof(long long unsigned));
            if (total_cost == NULL) {
                free_masterArr();
                return EXIT_FAILURE;
            }
            // Count one section at a time
            for (long long unsigned i = 0; i< *cnt; i++) {
                total_cost[i] = 0;
                DATE current = from;
                long long unsigned currentCost = masterArr[i].origin_cost;
                long long unsigned j = 0;
                
                // Skip to first relevant update (on or after from date)
                while (j < masterArr[i].update_count && 
                    lesserDate(masterArr[i].futureCostArr[j].iniDate, from)) {
                    currentCost = masterArr[i].futureCostArr[j].newCost;
                    j++;
                }
                
                while (lesserDate(current, to) || equalDate(current, to)) {
                    // Determine the end of this cost period
                    DATE next = to;
                    
                    if (j < masterArr[i].update_count && 
                        (lesserDate(masterArr[i].futureCostArr[j].iniDate, to) || 
                        equalDate(masterArr[i].futureCostArr[j].iniDate, to))) {
                        // Next update happens within our range - period ends day before update
                        next = masterArr[i].futureCostArr[j].iniDate;
                        decreDate(&next);
                    }
                    
                    // Calculate cost for this period
                    long long unsigned days = countDate(current, next);
                    total_cost[i] += currentCost * days;
                    
                    // Move to next period (day after next)
                    current = next;
                    increDate(&current);
                    
                    // Update cost if we reached an update date
                    if (j < masterArr[i].update_count && 
                        equalDate(current, masterArr[i].futureCostArr[j].iniDate)) {
                        currentCost = masterArr[i].futureCostArr[j].newCost;
                        j++;
                    }
                }
            }

            // Assignment Cost Command - Part 2: Computation
            long long unsigned a = 0;
            for (long long unsigned i = 0; i < *cnt; i++) {
                a += total_cost[i];
            }
            // Array length of the option array
            long long unsigned optionLen = (*cnt) * (*cnt-1);

            // Dynamically allocate memory for the array of options
            optionArr = (OPTION*) malloc (optionLen*sizeof(OPTION));
            if (optionArr == NULL) {
                free_masterArr();
                return EXIT_FAILURE;
            }

            // Calculate all possible options
            variableSubset(total_cost, cnt, optionArr);

            // Print only the optimal options
            printOptions(optionArr, optionLen);
            free(total_cost);
            free(optionArr);
            continue;
        }
        else {
            if (scanf_result == EOF) {
                break;
            }
            else if (scanf_result == 1) {
                clear_input_buffer();
                continue;
            }
            else {
                printf("Invalid input.\n");
                fflush(stdout);
                free_masterArr();
                return EXIT_FAILURE;
            }

        }
    }
    free_masterArr();
    return EXIT_SUCCESS;
}