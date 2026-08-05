#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

typedef struct TDate
{
  unsigned m_Year;
  unsigned short m_Month;
  unsigned short m_Day;
} TDATE;
TDATE makeDate ( unsigned       y,
                 unsigned short m,
                 unsigned short d )
{
  TDATE res = { y, m, d };
  return res;
}
bool  equalDate ( TDATE a,
                  TDATE b )
{
  return a . m_Year == b . m_Year
         && a . m_Month == b . m_Month
         && a . m_Day == b . m_Day;
}
#endif /* __PROGTEST__ */

// Return true if first date is greater than second date, false if not
bool greaterDate (TDATE a,
                  TDATE b )
{
  return ((a . m_Year >  b . m_Year)
      || ((a . m_Year >= b . m_Year) && ((a . m_Month > b . m_Month)))
      || ((a . m_Year >= b . m_Year) && (a . m_Month >= b. m_Month) && (a . m_Day >= b . m_Day)));
}

// Return true (1) if leapyear, false (0) if not
bool leapYear  (TDATE date)
{
    //printf("Year: %u\n", date . m_Year);  
  // If year % 4000, not leapyear
    if (date . m_Year % 4000 == 0) {
     // printf("Divisible 4000\n");
      return false;
    }

    // If year % 400, leapyear
    if (date . m_Year % 400 == 0){
     // printf("Divisible 400\n");
      return true;
    }

    // If year % 100, not leapyear
    if (date . m_Year % 100 == 0){
      //printf("Divisible 100\n");
      return false;
    }

    // If year % 4, leapyear
    if (date . m_Year % 4 == 0){
     // printf("Divisible 4\n");
      return true;
    }

    // Otherwise, not leapyear
   // printf("None of the above\n");
    return false;
}

// Return maxday of a month, leapyear adjusted
// NO ERROR
int maxDay (TDATE date)
{
    int day [12] = {31, (28 + leapYear (date)), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int days = 31;
    for (int i = 0; i < 12; i++)
    {
        if (date . m_Month == i + 1)
            days = day[i];
    }
    return days;
}

// Return true if valid date, false if not
bool validate (TDATE date)
{
  return (date . m_Day >= 1 && date . m_Day <= maxDay (date) 
  && date . m_Month >= 1 && date . m_Month <= 12 && date . m_Year >= 1900); 
}

// Increase the month of a date
void increMonth (TDATE *date)
{
  if ((*date) . m_Month < 12) {
    ((*date) . m_Month)++;
  }
  else {
    ((*date) . m_Year)++;
    ((*date) . m_Month) = 1;
  }
}

// Decrease the month of a date
void decreMonth (TDATE *date)
{
  if ((*date) . m_Month > 1) {
    ((*date) . m_Month)--;
  }
  else {
    ((*date) . m_Year)--;
    ((*date) . m_Month) = 12;
  }
}

// Identify if its Friday
bool Friday (TDATE date)
{
    int q = date . m_Day;
    int m = date . m_Month;
    int year = date . m_Year;
    if (date . m_Month == 1 || date . m_Month == 2)
    {
        m = date . m_Month + 12;
        year--;
    }
    int K = year % 100;
    int J = year / 100;
    int h = (q + ((13*(m+1))/5) + K + (K/4) + (J/4) - (2*J)) % 7;
    if (h<0)
      h+=7;
    if (h == 6)
      return true;
    return false;
}

// Count Friday the 13th
bool countFriday13 ( TDATE from, TDATE to, long long int * cnt )
{
  // Reset cnt
  *cnt = 0;
  
  // Check invalid dates of "to" and "from"
  if (greaterDate (to, from) == false 
    || validate(from) == false || validate (to) == false) {
  //  printf("countFriday13 failed input.\n");
    return false;
    }
  // Buffer date. Start from "from" date.
  TDATE current = makeDate (from . m_Year, from . m_Month, from . m_Day);
  TDATE next = makeDate (to . m_Year, to . m_Month, to . m_Day);

  // Check interval
  long long int interval = next . m_Year - current . m_Year;
  if (interval >= 400)
  {
    *cnt = (interval / 400) * 688;
    next . m_Year = interval % 400;
  }
 
  // Move to the 13th day
  if (current . m_Day > 13)
    current . m_Month++;
  current . m_Day = 13;
 
  // Iterate through the 13th of each month
  while (greaterDate(next, current) == true) {
    if (Friday(current) == true) {
      (*cnt)++; 
    }
    increMonth (&current);
  }

  // Return true once successful
 // printf("countFriday13 success.\n");
  //printf("cnt: %lld\n", *cnt);
 // printf("from: %u %u %u. to: %u %u %u. current: %u %u %u. next: %u %u %u.\n", 
  //  from . m_Day, from . m_Month, from . m_Year,
  ///  to . m_Day, to . m_Month, to . m_Year, 
   // current . m_Day, current . m_Month, current . m_Year,
  //  next . m_Day, next . m_Month, next . m_Year);
  return true;
}

// From a TDATE pointer, store the TDATE of the prev Friday13 to the same pointer, then return true. 
// Return false if false input/output
bool prevFriday13 ( TDATE * date )
{
  // Validate input date
  if (validate(*date) == false) {
  //  printf("prevFriday13 failed input. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);
    return false;
  }

 // printf("prevFriday13 success input. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);

  // Revert to the prev 13th of month
  if (date -> m_Day <= 13) {
    if (date -> m_Month == 1) {
      date -> m_Year--;
      date -> m_Month = 12;
    }
    else {
      date -> m_Month--;
    }
  }
  date -> m_Day = 13;

  // Return and the store value of the first Friday the 13th
  while (Friday(*date) != true) {
    //printf("d: %u m: %u y: %u\n", date->m_Day, date->m_Month, date->m_Year);
    decreMonth(date);
  }
    
  // Validate output date
  if (validate(*date) == false) {
  //  printf("prevFriday13 failed output. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);
    return false;
  }

  // Otherwise, return true
 // printf("prevFriday13 Success. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);
  return true;
}

// From a TDATE pointer, store the TDATE of the next Friday13 to the same pointer, then return true. 
// Return false if false input/output
bool nextFriday13 ( TDATE * date )
{
  // Validate input date
  if (validate(*date) == false) {
   // printf("nextFriday13 failed input. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);
    return false;
  }

 // printf("nextFriday13 success input. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);

  // Revert to the next 13th of month
  if (date -> m_Day >= 13) {
    if (date -> m_Month == 12) {
      date -> m_Year++;
      date -> m_Month = 1;
    }
    else {
      date -> m_Month++;
    }
  }
  date -> m_Day = 13;

  // Return and the store value of the first Friday the 13th
  while (Friday(*date) != true) {
    increMonth(date);
  }
  // Validate output date
  if (validate(*date) == false) {
   // printf("nextFriday13 failed output. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);
    return false;
  }
    
  // Otherwise, return true
  //printf("nextFriday13 Success. d: %u, m: %u, y: %u\n", (*date) . m_Day, (*date) . m_Month, (*date) . m_Year);
  return true;
}

#ifndef __PROGTEST__
int main ()
{
  long long int cnt;
  TDATE x;
 // TDATE x1; //my test
  //TDATE x2, y; // my test
 // printf("* * * * * * * * \nStart of my test.\n"); // my test
  //long long int A = 2944949; // my test
  //x1 = makeDate ( 4400, 2, 12); // my test
  //if (leapYear(x1))
 // {
 //   printf("Leap Year.\n");
 //   printf("Feb Day: %d\n", maxDay(x1));
 // }
 // else {
 //   printf("Not leap year.\n");
 //   printf("Feb Day: %d\n", maxDay(x1));
 // }
  //x2 = makeDate (2020,3, 13); // my test
 // y = makeDate (2026, 11, 14); // my test
 // assert ( countFriday13 ( x1, y, &cnt )); // my test
 // printf("Cnt: %lld\n", cnt); // my test
 // assert ( prevFriday13 (&x1)); // my test
 // assert ( nextFriday13 (&x2)); // my test
   //assert ( prevFriday13 ( &x ) // my test
   //         && equalDate ( x, makeDate ( 1914, 11, 13 ) ) ); // my test
   //printf("End of my test.\n* * * * * * * * \n"); // my test
   assert ( countFriday13 ( makeDate ( 1900,  1,  1 ), makeDate ( 2025,  5,  1 ), &cnt )
            && cnt == 215LL );
   assert ( countFriday13 ( makeDate ( 1900,  1,  1 ), makeDate ( 2025,  6,  1 ), &cnt )
            && cnt == 215LL );
   assert ( countFriday13 ( makeDate ( 1900,  1,  1 ), makeDate ( 2025,  5, 13 ), &cnt )
            && cnt == 215LL );
   assert ( countFriday13 ( makeDate ( 1900,  1,  1 ), makeDate ( 2025,  6, 13 ), &cnt )
            && cnt == 216LL );
   assert ( countFriday13 ( makeDate ( 1904,  1,  1 ), makeDate ( 2025,  5,  1 ), &cnt )
            && cnt == 207LL );
   assert ( countFriday13 ( makeDate ( 1904,  1,  1 ), makeDate ( 2025,  6,  1 ), &cnt )
            && cnt == 207LL );
   assert ( countFriday13 ( makeDate ( 1904,  1,  1 ), makeDate ( 2025,  5, 13 ), &cnt )
            && cnt == 207LL );
   assert ( countFriday13 ( makeDate ( 1904,  1,  1 ), makeDate ( 2025,  6, 13 ), &cnt )
            && cnt == 208LL );
   assert ( countFriday13 ( makeDate ( 1905,  2, 13 ), makeDate ( 2025,  5,  1 ), &cnt )
            && cnt == 205LL );
   assert ( countFriday13 ( makeDate ( 1905,  2, 13 ), makeDate ( 2025,  6,  1 ), &cnt )
            && cnt == 205LL );
   assert ( countFriday13 ( makeDate ( 1905,  2, 13 ), makeDate ( 2025,  5, 13 ), &cnt )
            && cnt == 205LL );
   assert ( countFriday13 ( makeDate ( 1905,  2, 13 ), makeDate ( 2025,  6, 13 ), &cnt )
            && cnt == 206LL );
   assert ( countFriday13 ( makeDate ( 1905,  1, 13 ), makeDate ( 2025,  5,  1 ), &cnt )
            && cnt == 206LL );
   assert ( countFriday13 ( makeDate ( 1905,  1, 13 ), makeDate ( 2025,  6,  1 ), &cnt )
            && cnt == 206LL );
   assert ( countFriday13 ( makeDate ( 1905,  1, 13 ), makeDate ( 2025,  5, 13 ), &cnt )
            && cnt == 206LL );
   assert ( countFriday13 ( makeDate ( 1905,  1, 13 ), makeDate ( 2025,  6, 13 ), &cnt )
            && cnt == 207LL );
   assert ( countFriday13 ( makeDate ( 2025,  5, 13 ), makeDate ( 2025,  5, 13 ), &cnt )
            && cnt == 0LL );
   assert ( countFriday13 ( makeDate ( 2025,  6, 13 ), makeDate ( 2025,  6, 13 ), &cnt )
            && cnt == 1LL );
   assert ( ! countFriday13 ( makeDate ( 2025, 11,  1 ), makeDate ( 2025, 10,  1 ), &cnt ) );
   assert ( ! countFriday13 ( makeDate ( 2025, 10, 32 ), makeDate ( 2025, 11, 10 ), &cnt ) );
   assert ( ! countFriday13 ( makeDate ( 2090,  2, 29 ), makeDate ( 2090,  2, 29 ), &cnt ) );
   assert ( countFriday13 ( makeDate ( 2096,  2, 29 ), makeDate ( 2096,  2, 29 ), &cnt )
            && cnt == 0LL );
   assert ( ! countFriday13 ( makeDate ( 2100,  2, 29 ), makeDate ( 2100,  2, 29 ), &cnt ) );
   assert ( countFriday13 ( makeDate ( 2000,  2, 29 ), makeDate ( 2000,  2, 29 ), &cnt )
            && cnt == 0LL );
   x = makeDate ( 2025, 6, 12 );
   assert ( prevFriday13 ( &x )
            && equalDate ( x, makeDate ( 2024, 12, 13 ) ) );
   x = makeDate ( 2025, 6, 12 );
   assert ( nextFriday13 ( &x )
            && equalDate ( x, makeDate ( 2025, 6, 13 ) ) );
   x = makeDate ( 2025, 6, 13 );
   assert ( prevFriday13 ( &x )
            && equalDate ( x, makeDate ( 2024, 12, 13 ) ) );
   x = makeDate ( 2025, 6, 13 );
   assert ( nextFriday13 ( &x )
            && equalDate ( x, makeDate ( 2026, 2, 13 ) ) );
   x = makeDate ( 2025, 6, 14 );
   assert ( prevFriday13 ( &x )
            && equalDate ( x, makeDate ( 2025, 6, 13 ) ) );
   x = makeDate ( 2025, 6, 14 );
   assert ( nextFriday13 ( &x )
            && equalDate ( x, makeDate ( 2026, 2, 13 ) ) );
   x = makeDate ( 2025, 2, 29 );
   assert ( ! prevFriday13 ( &x ) );
   x = makeDate ( 2025, 2, 29 );
   assert ( ! nextFriday13 ( &x ) );
   x = makeDate ( 1900, 3, 18 );
   assert ( ! prevFriday13 ( &x ) );
   x = makeDate ( 1900, 3, 18 );
   assert ( nextFriday13 ( &x )
            && equalDate ( x, makeDate ( 1900, 4, 13 ) ) );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
