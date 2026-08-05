// Libraries
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Constants
#define BUFFER_SIZE 100 // Arbitrary value, more than 12 digit
#define MAX_DIGITS 12
#define MAX_EXPR 25 //12 digits + 11 operators + 1 '\0' + 1 (safety margin)

// Structs
// A string of math expression. 
// It has a string, size and memory to dynamic allocate, currentSum and lastTerm to keep track of value
typedef struct {
    char expr [MAX_EXPR];
} string;

// An array of strings
typedef struct {
    string* strArr;
    size_t size;
    size_t memory;
} stringArr;

// A global variable to hold all solutions 
stringArr masterplan;
// Functions
// Warn user of invalid input
void invalid (void) {
    printf("Invalid input.\n");
}
// Get input
char* input (char* buffer, size_t size) {
    return fgets(buffer, size, stdin);
}
// Validate operator
int validOpe (char* buffer, int* position) {
    char ope;
    if (sscanf(buffer, "%c %n", &ope, position) != 1) {
        return -1;
    }
    if (ope == '?') return 1;
    if (ope == '#') return 0;
    return -1;
}
// Validate digit
bool valiDigit (char* buffer) {
    // Check empty string
    if (buffer == NULL || *buffer == '\0') {
        return false;
    }
    // Check for optional sign
    size_t i = 0;
    if(buffer[i] == '-') {
        i++;
    }
    // No digit after the sign
    if (buffer[i] == '\0') {
        return false;
    }
    // If there is digit, loop thru each, validate each char to be a digit
    for (; buffer[i] != '\0'; i++) {
        if (buffer[i] < '0' || buffer[i] > '9') {
            return false;
        }
    }
    return true; // Only digits found
}
// Replace '\n' with '\0'
void newlineTerm (char* buffer) {
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
}
// Replace the end of string with '\0'
void nullTerm(char str[], size_t len) {
    str[len] ='\0';
}
// Validate length
bool validLen (const char* buffer) {
    size_t len = strlen(buffer);
    return (len<=MAX_DIGITS && len>=1);
}
// Print all solutions
void printSol (int ope) {
    if (ope == 1) {
        for (size_t i = 0; i < masterplan.size; i++) {
        printf("= %s\n", masterplan.strArr[i].expr);
        }
    }
    printf("Total: %zu\n", masterplan.size);
}
// Initiate, realloc, add new string, and free - For the global var masterPlan (carry all strings)
void initPlan (void) {
    masterplan.strArr = (string*) malloc (sizeof(string));
    masterplan.size = 0;
    masterplan.memory = 1;
}
void reallocPlan (void) {
    if (masterplan.size + 1 >= masterplan.memory) {
        size_t newMem = masterplan.memory*2 + 1;
        masterplan.strArr = (string*) realloc (masterplan.strArr, newMem * sizeof(string));
        masterplan.memory = newMem;
    }
}
void freePlan (void) {
    free(masterplan.strArr);
}
// Add a new solution string to plan
void addPlan(char expr[]) {
    strcpy(masterplan.strArr[masterplan.size].expr,expr);
    masterplan.size++;
}

// Reconstruct the expr such that only + and - are allowed
void newExpr(char expr[], size_t exprLen) {
    int ptr=0;
    int ptrStart = 0, ptrEnd = 0;
    int ptrArr1[MAX_EXPR];
    int ptrArr2[MAX_EXPR];
    int mult[MAX_EXPR];
    int ptrAll = ptr;
    char* ptrSum = expr;
    long long num1;
    char ope;
    size_t i = 0;
    int j = 0;
    long long multRes = 1;
    while(i<exprLen) {
        while (sscanf(ptrSum+ptrAll, "%lld%c%n", &num1, &ope, &ptr) == 2 && ope == '*'){
            ptrAll+=ptr;
            if (ptrStart == 0) {
                ptrStart=ptrAll;
            }
            multRes *= num1;
            i+=ptr;
        }
        ptrEnd = ptrAll;
        mult[ptrStart] = multRes;
        ptrArr1[j]=ptrStart;
        ptrArr2[j]=ptrEnd;
        j++;
        multRes=1;
    }
    ptrSum=expr;
    int k = 0;
    int l = 0;
    while(sscanf(ptrSum, "%lld%c%n", &num1, &ope, &ptr) == 2) {
        if (ope != '*') {
            expr[k++]=num1;
            expr[k++]=ope;
            ptrSum+=(ptr);
        }
        else {
            expr[k++]=mult[ptrArr1[l]];
            ptrSum+=ptrArr2[l];
            l++;
        }
    }
    if (sscanf(ptrSum, "%lld", &num1) == 1) {
        expr[k]=num1;
    }
}
long long nextNum(const char **ptr) {
    long long num = 0;
    int chars_read = 0;
    const char *start = *ptr;

    // Scan for the number, tracking how many characters it consumed
    if (sscanf(start, "%lld%n", &num, &chars_read) == 1) {
        *ptr += chars_read; // Advance the pointer past the number
        return num;
    }
    return 0; 
}
long long evalExpr1(char expr[]) {
    const char *ptr = expr;
    long long final_sum = 0;
    long long current_term = 0;
    long long factor = 0;

    // 1. Get the first number, which starts the first term
    if (isdigit(*ptr)) {
        factor = nextNum(&ptr);
        current_term = factor;
    } else {
        // Empty expression, return 0 (should not happen)
        return 0;
    }
    
    // 2. Process the rest of the expression
    while (*ptr != '\0') {
        char op = *ptr;
        ptr++; // Advance past the operator
        
        // Ensure the next character is a digit before attempting to read a number
        if (!isdigit(*ptr)) break; 

        factor = nextNum(&ptr); // Read the next factor
        
        if (op == '+' || op == '-') {
            // Addition/Subtraction: The previous 'current_term' is complete.
            // Apply 'next_op' to finalize the previous term and add it to final_sum.
            final_sum += current_term;
            
            // Start a new term: current_term is now just the new factor
            current_term = (op == '-') ? -factor : factor;

        } else if (op == '*') {
            // Multiplication: Continue the current term.
            // Multiply the current_term by the new factor.
            current_term *= factor;
            // The last factor used for multiplication is now tracked by the current_term value
        } else {
            // Unknown operator (should not happen)
            break;
        }
    }

    // 3. Add the final term to the sum
    final_sum += current_term;
    
    // printf("Evaluated Sum: %lld\n", final_sum);
    return final_sum;
}
// Evaluate the given string of expression
long long evalExpr(char expr[]) {
    int ptr = 0;
    char* ptrSum = expr+ptr;
    long long num;
    long long sum = 0;
    char ope;
    if (sscanf(ptrSum, "%lld%n", &num, &ptr) == 1){
        sum += num;
        ptrSum+=ptr;
    }
    while(sscanf(ptrSum, "%c%lld%n", &ope, &num, &ptr) == 2) {
        if (ope == '+') {
            sum += num;
        }
        else if (ope == '-') {
            sum -= num;
        }
        ptrSum+=ptr;
    }
    //printf("Sum:%lld\n", sum);
    return sum;
}
// Const input: The string of digits (set[]), length of string (n), target value (target)
// Variable inout - Tracking: The number of digits used (idx)
// Variable input - String: The string we must build (expr[]), length of string (exprLen)
// Variable input - Value: Sum of all expr so far(currentSum), next term to be combined (currentTerm)
// Return value: Void, the results are printed at base case (if target value is matched)
// Base case: If all digits is used => if target value matched => null terminate string => print the expression
// Recursive case: If there exist unused digits => Go through 4 different options
// Four options: '+', '-', '*', and no operation
// Order of operation: Build the expr => Calculate currentSum and currentTerm => Apply to the next recursion call
void rcsOpe (const char set[], const size_t n, const long long target, char expr[], 
            size_t exprLen, size_t idx, long long currentSum, long long currentTerm, 
            long long lastFactor) {
    // Base case: All digits used
    if (idx == n) {
        //for (size_t  i = 0; i<exprLen+1;i++) {
        //printf("%c", expr[i]);
        //}
        //printf("\n");
        //printf("Sum:%lld.Term:%lld.Last:%lld\n", currentSum, currentTerm, lastFactor);
        //printf("Base! %lld+%lld=%lld=?=%lld .\n", currentSum, currentTerm, currentSum + currentTerm, target);
        if (currentSum + currentTerm == target) {
        //    printf("Found! %lld+%lld=%lld=%lld .\n", currentSum, currentTerm, currentSum + currentTerm, target);
            nullTerm(expr, exprLen); // This add one extra size to exprLen
            reallocPlan();
            addPlan(expr);
        //    printf("[%s]\n", expr);
        }
        return;
    }
    // Recursive case: Still digits to be used
    long long digit = set[idx] - '0';
    // No operator
    if (idx>0) {
        if (lastFactor != 0 || digit == 0) {
            long long newFactor = (lastFactor >= 0) ? (lastFactor*10 + digit) 
                                                    : (lastFactor*10 - digit);
            long long newTerm = currentTerm - lastFactor + newFactor;
            expr[exprLen]=set[idx];
            //for (size_t  i = 0; i<exprLen+1;i++) {
            //printf("%c", expr[i]);
            //}
            //printf("\n");
            //printf("Sum:%lld.Term:%lld.Last:%lld\n", currentSum, newTerm, newFactor);
            rcsOpe(set, n, target, expr, exprLen+1, idx+1, currentSum, newTerm, newFactor);
        }
    }
    // Add operator '+'
    expr[exprLen]='+';
    expr[exprLen+1]=set[idx];
    //for (size_t  i = 0; i<exprLen+2;i++) {
    //    printf("%c", expr[i]);
    //}
    //printf("\n");
    //printf("Sum:%lld.Term:%lld.Last:%lld\n", currentSum + currentTerm, digit, digit);
    rcsOpe(set, n, target, expr, exprLen+2, idx+1, currentSum + currentTerm, digit, digit);
    // Minus operator '-'
    expr[exprLen]='-';
    expr[exprLen+1]=set[idx];
    //for (size_t  i = 0; i<exprLen+2;i++) {
    //    printf("%c", expr[i]);
    //}
    //printf("\n");
    //printf("Sum:%lld.Term:%lld.Last:%lld\n", currentSum + currentTerm, -digit, -digit);
    rcsOpe(set, n, target, expr, exprLen+2, idx+1, currentSum + currentTerm, -digit, -digit);
    // Multiplication operator '*'
    expr[exprLen]='*';
    expr[exprLen+1]=set[idx];
    //for (size_t i = 0; i<exprLen+2;i++) {
    //    printf("%c", expr[i]);
    //}
    //printf("\n");
    //printf("Sum:%lld.Term:%lld.Last:%lld\n", currentSum, currentTerm*digit, digit);
    rcsOpe(set, n, target, expr, exprLen+2, idx+1, currentSum, currentTerm*digit, digit);
}
void rcsExpr (const char set[], const size_t n, const long long target, char expr[], 
            size_t exprLen, size_t idx) {
    // Base case: All digits used
    if (idx == n) {
        //for (size_t i = 0; i < exprLen; i++) {
        //    printf("%c", expr[i]);
        //}
        //printf("\n");
        if (evalExpr1(expr) == target) {
            nullTerm(expr, exprLen); // This add one extra size to exprLen
            reallocPlan();
            addPlan(expr);
        }
        return;
    }
    // Recursive case: Still digits to be used
    // No operator (concatenate)
    if (idx>0) {
            expr[exprLen]=set[idx];
            rcsExpr(set, n, target, expr, exprLen+1, idx+1);
        }
    // With operator
    expr[exprLen+1]=set[idx];
    // Add operator '+'
    expr[exprLen]='+';
    rcsExpr(set, n, target, expr, exprLen+2, idx+1);
    // Minus operator '-'
    expr[exprLen]='-';
    rcsExpr(set, n, target, expr, exprLen+2, idx+1);
    // Multiplication operator '*'
    expr[exprLen]='*';
    rcsExpr(set, n, target, expr, exprLen+2, idx+1);
    }
void rcsOpe2(const char *digits,
            int n,
            long long target,
            char *expr,
            int exprLen,
            int idx,
            long long currentSum,
            long long lastTerm)
{
    if (idx == n) {
        if (currentSum + lastTerm == target) {
            expr[exprLen] = '\0';
            printf("= %s\n", expr);
        }
        return;
    }

    long long num = 0;
    int startLen = exprLen;

    for (int i = idx; i < n; i++) {
        num = num * 10 + (digits[i] - '0');

        expr[exprLen++] = digits[i];

        if (idx == 0) {
            /* first number */
            rcsOpe2(digits, n, target,
                   expr, exprLen,
                   i + 1,
                   0,
                   num);
        } else {
            /* + */
            expr[startLen - 1] = '+';
            rcsOpe2(digits, n, target,
                   expr, exprLen,
                   i + 1,
                   currentSum + lastTerm,
                   num);

            /* - */
            expr[startLen - 1] = '-';
            rcsOpe2(digits, n, target,
                   expr, exprLen,
                   i + 1,
                   currentSum + lastTerm,
                   -num);

            /* * */
            expr[startLen - 1] = '*';
            rcsOpe2(digits, n, target,
                   expr, exprLen,
                   i + 1,
                   currentSum,
                   lastTerm * num);
        }
    }
}
          
// Main
int main(void) {
    // Part 1: Get user input
    printf("Digits:\n");
    char buffer[BUFFER_SIZE];
    if (input(buffer, sizeof(buffer)) == NULL){
        invalid();
        return EXIT_FAILURE;
    } 
    // Replace '\n' with '\0'
    newlineTerm(buffer);
    //printf("Buffer:%s. Size:%ld.\n", buffer, len);
    // Validate length and digit
    if (!validLen(buffer) || !valiDigit(buffer)) {
        invalid();
        return EXIT_FAILURE;
    }

    // Part 2: Get user problem input
    printf("Problems:\n");
    char search[BUFFER_SIZE];
    int position;
    initPlan();
    // Continuously read user input
    while (input(search, sizeof(search)) != NULL) {
        
        // Check valid operator ? and #
        int ope;
        if((ope = validOpe(search, &position)) == -1) {
            invalid();
            return EXIT_FAILURE;
        }
        // Null terminate '\n'
        newlineTerm(search);
        // Skip whitespaces
        char* digitPtr = search+position;
        while(digitPtr[0] == ' ') {
            digitPtr++;
        }
        // Check for validLen and digit
        if (!validLen(digitPtr) || digitPtr[0] == '\0' || !valiDigit(digitPtr)) {
            invalid();
            return EXIT_FAILURE;
        }
        // Convert to long long
        char* convert;
        long long target = strtoll(digitPtr, &convert, 10);
        if (convert[0] != '\0') {
            invalid();
            return EXIT_FAILURE;
        }
        //printf("Correct: %lld. Len: %ld.\n", target, strlen(search+position));

        // Part 3: Parsing for recursive calculation
        masterplan.size = 0;
        //printf("Value of buffer %s\n", buffer);
        //printf("Length of buffer: %zu\n", strlen(buffer));
        //printf("Value of target: %lld\n", target);   
        char expr[MAX_EXPR];
        expr[0] = buffer[0];
        rcsExpr(buffer, strlen(buffer), target, expr, 1, 1);
        printSol(ope);
    }
    freePlan();
    // Exit success
    return EXIT_SUCCESS;
}