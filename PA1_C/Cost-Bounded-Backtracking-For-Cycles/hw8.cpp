
#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

constexpr size_t CITY_NAME_MAX = 100;

typedef struct TTrip
{
  struct TTrip * m_Next;
  char         * m_Desc;
  int            m_Cities;
  int            m_Cost;
} TTRIP;

TTRIP * makeTrip ( char    desc[],
                   int     cities,
                   int     cost,
                   TTRIP * next )
{
  TTRIP * res = (TTRIP *) malloc ( sizeof ( *res ) );
  res -> m_Next = next;
  res -> m_Desc = desc;
  res -> m_Cities = cities;
  res -> m_Cost = cost;
  return res;
}

void freeTripList ( TTRIP * x )
{
  while ( x )
  {
    TTRIP * tmp = x -> m_Next;
    free ( x -> m_Desc );
    free ( x );
    x = tmp;
  }
}
#endif /* __PROGTEST__ */

// New constants
#define MIN_TRIP 2

// New structs
// Define a "route"
typedef struct {
  char from[CITY_NAME_MAX + 1];
  char to[CITY_NAME_MAX + 1];
  int cost;
} TRoute;

// An array to store all routes
typedef struct {
  TRoute* routes;
  int size;
  int memory;
} TRouteArr;

// New global variables
TRouteArr routeArr;

// New functions
// Check functions (to be turned off when I/O testing - submisssion)
// Check for data insertion into R is correctly done
void checkR () {
  for (int i = 0; i < routeArr.size; i++) {
    printf("%d: %s -> %s\n",  routeArr.routes[i].cost,
                              routeArr.routes[i].from,
                              routeArr.routes[i].to);
  }
}
// Check if linked list is correctly formed
void printList(TTRIP* list) {
    for (TTRIP* ptr = list; ptr != nullptr; ptr=ptr->m_Next) {
    printf("%d: %s\n", ptr->m_Cost, ptr->m_Desc);
  }
}

// Build a string based on path explored so far
void buildExpr (char* currentExpr, const char* newExpr) {
  strcat(currentExpr, " -> ");
  strcat(currentExpr, newExpr);
}
// Check if a city is visited (search an array)
bool isVisited (const char** visitedArr, const int visitedCnt, const char* city) {
  for (int i = 0; i < visitedCnt; i++) {
    if (strcmp(visitedArr[i], city) == 0) {
      return true;
    }
  }
  return false;
}
// Insert into a sorted linked list
void sortedInsert (TTRIP** list, TTRIP* tmp) {
  // If this is the first node, prepend
  if ((*list) == nullptr) {
    tmp->m_Next=*list;
    *list=tmp;
  }
  // If the total cost is lowest so far, prepend
  else if (tmp->m_Cost < (*list)->m_Cost) {
    tmp->m_Next=*list;
    *list=tmp;
  }
  // Else if its belong far into the linked list
  else {
    // Traverse the linked list
    for (TTRIP* ptr = *list; ptr != nullptr; ptr=ptr->m_Next) {
      // If at the end of list, append at end of list
      if (ptr->m_Next == nullptr) {
        tmp->m_Next = ptr->m_Next;
        ptr->m_Next = tmp;
        break;
      }
      // Else if its belong in the middle of list
      else {
        if (tmp->m_Cost < ptr->m_Next->m_Cost) {
          tmp->m_Next = ptr->m_Next;
          ptr->m_Next = tmp;
          break;
        }
      }
    }
  }
}

// Depth-first Search for looking up all possible trips
void findRoundTrip (TTRIP** list, const char* startCity, const int costMax, 
                    const char** visited, const int visitedCnt,
                    const char* currentCity, int costTotal, int cntCity, 
                    char* expr) {
  // Base case
  // If the next city is the same as startCity && cntCity > 1 && costTotal < costMax => return
  if (strcmp(startCity, currentCity) == 0 && cntCity >= MIN_TRIP) {
    // Append to list of trips
    TTRIP* tmp = (TTRIP*) malloc (sizeof(*tmp));
    tmp->m_Cost  = costTotal;
    tmp->m_Cities = cntCity;
    tmp->m_Desc = strdup(expr);
    tmp->m_Next = nullptr;
    sortedInsert(list, tmp); // inserted based on value of cost
    return;
  }
  // Recursive case
  for (int i = 0; i < routeArr.size; i++) {
    // If the next path (start from currentCity) is found
    if (strcmp(currentCity, routeArr.routes[i].from) == 0) {
      // Check if the next dest is alr visited and within budget 
      // OR its the startCity (then we can also visit)
      int nextCost = costTotal + routeArr.routes[i].cost;
      if ((!isVisited(visited, visitedCnt, routeArr.routes[i].to)
          || strcmp(startCity, routeArr.routes[i].to) == 0) && nextCost <= costMax) {
        // Update the new des to visited
        visited[visitedCnt] = routeArr.routes[i].to;
        size_t exprLen = strlen(expr); // Record the last string before changing it
        // Update the new string (append)
        buildExpr(expr, routeArr.routes[i].to);
        findRoundTrip(list, startCity, costMax, 
                      visited, visitedCnt+1, routeArr.routes[i].to, 
                      nextCost, cntCity+1, expr);
        expr[exprLen] = '\0'; // Backtrack the string
      }
    }
  }
}
// Initialize the route array
void mallocR () {
  TRoute* tmp = (TRoute*) malloc (sizeof(TRoute));
  if (tmp == nullptr) {
    return;
  }
  routeArr.routes = tmp;
  routeArr.size = 0;
  routeArr.memory = 1;
}
// Realloc route array
void reallocR () {
  if (routeArr.size + 1 >= routeArr.memory) {
    int newCap = routeArr.memory*2 + 1;
    TRoute* tmp = (TRoute*) realloc(routeArr.routes, newCap * sizeof(TRoute));
    if (tmp == nullptr) {
      return;
    }
    routeArr.routes = tmp;
    routeArr.memory = newCap;
  }
}
// Add a new route to array
void addR (TRoute newRoute) {
  routeArr.routes[routeArr.size++] = newRoute;
}
// Free route array
void freeR () {
  free(routeArr.routes);
}

// Main function
TTRIP * findTrips ( const char data[], const char from[], int costMax )
{
  // Open the string as a file
  FILE* f = fmemopen ((void*) data, strlen(data), "r");
  if (!f) return nullptr;

  // Buffer for a "route" - "Cost: CityA -> CityB"
  int cost;
  char cityA[CITY_NAME_MAX + 1]; // +1 for null-term 
  char cityB[CITY_NAME_MAX + 1];

  // Initialize the array to hold all "routes"
  mallocR();

  // Read data, one string at a time
  while(fscanf(f, " %d: %100s -> %100s", &cost, cityA, cityB) == 3) { //Limit string read to 100 (safety)
    // Save each "route" into an array
    TRoute tmp;
    tmp.cost = cost;
    strcpy(tmp.from, cityA);
    strcpy(tmp.to, cityB);
    reallocR();
    addR(tmp);
  }

  // Check 1 - Data insertion into R
  //checkR();

  // Initialize the TTrip head
  TTrip* list = nullptr;

  // Buffer for an "expr" - "Cost: City1 -> City 2 -> ..."
  char expr [100000]; // To be dynamically allocated for bonus test
  
  // Add in the start city
  expr[0] = '\0';
  strcat(expr, from);

  // Array of visited cities
  const char* visited[1000]; // To be dynamically allocated for bonus test
  int visitedCnt = 0;

  // Add in the start city
  visited[visitedCnt++] = from;

  // DFS through the array until it reach back to the same city 
  findRoundTrip(&list, from, costMax, visited, visitedCnt, from, 0, 0, expr);

  // Check 2 - DFS 
  printList(list);

  // Close file and other end-of-function operations
  fclose(f);
  freeR();
  
  // Default return
  return list;
}

#ifndef __PROGTEST__
int main ()
{
  const char * data0 = R"(
100: Prague -> London
80: Prague -> Paris
90: Paris -> London
75: London -> Madrid
95: Madrid -> Prague
1000: London -> Prague
50: Berlin -> Prague
80: Madrid -> Berlin
90: Rome -> Prague
100: Wien -> Rome
90: Prague -> Lisabon
80: Lisabon -> Dublin
)";
  const char * data1 = R"(
100: Prague -> London
107: London -> Prague
80: Prague -> Paris
78: Paris -> Prague
38: Paris -> London
43: London -> Paris
69: Prague -> Wien
89: London -> Wien
73: Paris -> Wien
63: Wien -> Prague
82: Wien -> London
77: Wien -> Paris
163: Zagreb -> Tallinn
282: Tallinn -> Lisboa
377: Lisboa -> Zagreb
)";
  TTRIP * t;
  t = findTrips ( data0, "Prague", 300 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "Prague", 700 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "Prague", 1100 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1100 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "Prague", 5000 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Madrid -> Berlin -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1100 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1170 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data0, "London", 2000 );
  assert ( t );
  assert ( t -> m_Cost == 270 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "London -> Madrid -> Prague -> London" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 305 );
  assert ( t -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "London -> Madrid -> Berlin -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 340 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "London -> Madrid -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 375 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 5 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Madrid -> Berlin -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1100 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 1170 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Prague", 270 );
  assert ( t );
  assert ( t -> m_Cost == 132 );
  assert ( t -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Desc, "Prague -> Wien -> Prague" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 158 );
  assert ( t -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Prague -> Paris -> Prague" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 207 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 216 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> Wien -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 221 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Paris -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 224 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Wien -> Paris -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 225 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 252 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> London -> Wien -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 258 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Wien -> London -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 270 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Prague -> Paris -> London -> Wien -> Prague" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Wien", 270 );
  assert ( t );
  assert ( t -> m_Cost == 132 );
  assert ( t -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Desc, "Wien -> Prague -> Wien" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 150 );
  assert ( t -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "Wien -> Paris -> Wien" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 171 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "Wien -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 198 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> London -> Paris -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 204 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Paris -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 216 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Prague -> Paris -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 224 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Paris -> Prague -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 252 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Prague -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 258 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> London -> Prague -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 270 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "Wien -> Prague -> Paris -> London -> Wien" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "London", 400 );
  assert ( t );
  assert ( t -> m_Cost == 81 );
  assert ( t -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Desc, "London -> Paris -> London" ) );
  assert ( t -> m_Next );
  assert ( t -> m_Next -> m_Cost == 171 );
  assert ( t -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Desc, "London -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Cost == 198 );
  assert ( t -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cost == 204 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 207 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 2 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 221 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 225 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 252 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 258 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 270 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Prague -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 272 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Prague -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 279 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Paris -> Wien -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 291 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Wien -> Paris -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 342 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Prague -> Paris -> Wien -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cost == 344 );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Cities == 4 );
  assert ( ! strcmp ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Desc, "London -> Wien -> Paris -> Prague -> London" ) );
  assert ( t -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Lisboa", 1000 );
  assert ( t );
  assert ( t -> m_Cost == 822 );
  assert ( t -> m_Cities == 3 );
  assert ( ! strcmp ( t -> m_Desc, "Lisboa -> Zagreb -> Tallinn -> Lisboa" ) );
  assert ( t -> m_Next == nullptr );
  freeTripList ( t );
  t = findTrips ( data1, "Oslo", 1000 );
  assert ( t == nullptr );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
