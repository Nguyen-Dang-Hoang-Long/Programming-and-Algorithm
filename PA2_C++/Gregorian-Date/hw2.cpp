#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <compare>
#endif /* __PROGTEST__ */

//=================================================================================================
// date_format manipulator - a dummy implementation. Keep this code unless you implement your
// own working manipulator.
std::ios_base & ( * date_format ( const char * fmt ) ) ( std::ios_base & x )
{
  return [] ( std::ios_base & ios ) -> std::ios_base & { return ios; };
}
//=================================================================================================


// Date functions
// Leap year bool
bool leap_year (int y1) {
  if (y1 % 400 == 0) return true;
  if (y1 % 100 == 0) return false;
  if (y1 % 4 == 0) return true;
  return false;
}
// Max day of month
int max_day (int m1, int y1) {
  int days[] = {31, 28 + leap_year(y1), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  return days[m1-1];
}
// Check date validity
bool valid_date (int y1, int m1, int d1) {
  if (m1 < 1 || m1 > 12)  {
    return false;
  }
  if (d1 < 1 || d1 > max_day (m1, y1)) {
    return false;
  } 
  return true;
}

class InvalidDateException : public std::invalid_argument {
  public:
    InvalidDateException() : std::invalid_argument("Invalid date") {}
};

class CDate
{
  // Attributes
  private:
    int y, m, d;

  // Public
  public: 
    // Constructor
    CDate (int y1, int m1, int d1) : y(y1), m(m1), d(d1) {
      if (!valid_date(y,m,d)) {
        throw InvalidDateException();
      } 
    }

    // Destructor
    ~CDate() = default;

    // Copy 
    CDate (const CDate& date) : y(date.y), m(date.m), d (date.d) {}
    friend void swap (CDate& a, CDate& b);
    CDate& operator = (CDate date); 

    // Infix 
    CDate operator + (const int val);
    CDate operator - (const int val);

    // Diff 
    int operator - (const CDate& date) const;

    // Prefix
    CDate& operator ++ ();
    CDate& operator -- ();

    // Postfix
    CDate operator ++ (int);
    CDate operator -- (int);

    // Compare
    std::strong_ordering operator <=> (const CDate & date) const;
    bool operator == (const CDate& date) const;

    // Streams
    friend std::istream & operator >> (std::istream & is, CDate& date);
    friend std::ostream & operator << (std::ostream & os, const CDate& date);
};



void swap (CDate& a, CDate& b) {
  using std::swap;
  swap(a.d, b.d);
  swap (a.m, b.m);
  swap (a.y, b.y);
}
// Copy Assignment Operator
CDate& CDate::operator = (CDate date) {
  swap (*this, date);
  return *this;
}

// Common Infix
// Operator +
CDate CDate::operator + (int const val) {
  CDate copy = *this;
  if (val > 0) {
    int increase = val;
    while (increase > 0) {
      if (copy.d == max_day(copy.m, copy.y)) {
        if (copy.m == 12) {
          copy.y++;
          copy.m = 1;
        }
        else {
          copy.m++;
        }
        copy.d = 1;
      }
      else {
        copy.d++;
      }
      increase--;
    }
  }
  else if (val < 0) {
    int decrease = val;
    while (decrease < 0) {
      if (copy.d == 1) {
        if (copy.m == 1) {
          copy.y--;
          copy.m = 12;
        } 
        else {
          copy.m--;
        }
        copy.d = max_day(copy.m, copy.y);
      }
      else {
        copy.d--;
      }
      decrease++;
    }
  }
  return copy;
}
// Operator -
CDate CDate::operator - (const int val) {
  return *this + (-val);
}

// Difference between two CDates
int CDate::operator - (const CDate & date) const {
  if (*this == date) return 0;
  int cnt = 0;
  CDate future = (*this < date) ? date : *this;
  CDate current = (*this < date) ? *this : date; 
  while (current != future) {
    current++;
    cnt++;
  }
  return cnt;
}

// Prefix ++a, change then return
CDate& CDate::operator ++ () {
  (*this) = (*this + 1);
  return *this;
}
// Prefix --a
CDate& CDate::operator -- () {
  (*this) = (*this - 1);
  return *this;
}
// Postfix a++, return then change
CDate CDate::operator ++ (int) {
  CDate copy = *this;
  ++(*this);
  return copy;
}
// Postfix a--
CDate CDate::operator -- (int) {
  CDate copy = *this;
  --(*this);
  return copy;
}

// Compare inequalities
std::strong_ordering CDate::operator <=> (const CDate& date) const {
  if (auto cmp = y <=> date.y; cmp != 0) return cmp;
  if (auto cmp = m <=> date.m; cmp != 0) return cmp;
  return d <=> date.d;
}

// Compare equalities
bool CDate::operator == (const CDate& date) const {
  return this->d == date.d && this->m == date.m && this->y == date.y;
}

// In stream
std::istream& operator >> (std::istream& is, CDate& date) {
  char d1, d2;
  int y,m,d;
  if (!(is >> y >> d1 >> m >> d2 >> d)
    || d1 != '-' || d2 != '-' || !valid_date(y, m, d)) {
      is.setstate(std::ios::failbit);
  }
  else date = {y,m,d};
  return is;
}
// Out stream
std::ostream& operator << (std::ostream& os, const CDate& date) {
  std::ostringstream ss;
  ss << date.y << '-'
     << std::setfill('0') << std::setw(2) << date.m << '-' 
     << std::setfill('0') << std::setw(2) << date.d;
  return os << ss.str();
}

#ifndef __PROGTEST__
int main ()
{
  std::ostringstream oss;
  std::istringstream iss;

  CDate a ( 2000, 1, 2 );
  CDate b ( 2010, 2, 3 );
  CDate c ( 2004, 2, 10 );
  oss . str ("");
  oss << a;
  assert ( oss . str () == "2000-01-02" );
  oss . str ("");
  oss << b;
  assert ( oss . str () == "2010-02-03" );
  oss . str ("");
  oss << c;
  assert ( oss . str () == "2004-02-10" );
  a = a + 1500;
  oss . str ("");
  oss << a;
  assert ( oss . str () == "2004-02-10" );
  b = b - 2000;
  oss . str ("");
  oss << b;
  assert ( oss . str () == "2004-08-13" );
  assert ( b - a == 185 );
  assert ( ( b == a ) == false );
  assert ( ( b != a ) == true );
  assert ( ( b <= a ) == false );
  assert ( ( b < a ) == false );
  assert ( ( b >= a ) == true );
  assert ( ( b > a ) == true );
  assert ( ( c == a ) == true );
  assert ( ( c != a ) == false );
  assert ( ( c <= a ) == true );
  assert ( ( c < a ) == false );
  assert ( ( c >= a ) == true );
  assert ( ( c > a ) == false );
  a = ++c;
  oss . str ( "" );
  oss << a << " " << c;
  assert ( oss . str () == "2004-02-11 2004-02-11" );
  a = --c;
  oss . str ( "" );
  oss << a << " " << c;
  assert ( oss . str () == "2004-02-10 2004-02-10" );
  a = c++;
  oss . str ( "" );
  oss << a << " " << c;
  assert ( oss . str () == "2004-02-10 2004-02-11" );
  a = c--;
  oss . str ( "" );
  oss << a << " " << c;
  assert ( oss . str () == "2004-02-11 2004-02-10" );
  iss . clear ();
  iss . str ( "2015-09-03" );
  assert ( ( iss >> a ) );
  oss . str ("");
  oss << a;
  assert ( oss . str () == "2015-09-03" );
  a = a + 70;
  oss . str ("");
  oss << a;
  assert ( oss . str () == "2015-11-12" );
  oss . str ("");
  oss << std::setw ( 20 ) << a;
  assert ( oss . str () == "          2015-11-12" );

  CDate d ( 2000, 1, 1 );
  try
  {
    CDate e ( 2000, 32, 1 );
    assert ( "No exception thrown!" == nullptr );
  }
  catch ( ... )
  {
  }
  iss . clear ();
  iss . str ( "2000-12-33" );
  assert ( ! ( iss >> d ) );
  oss . str ("");
  oss << d;
  assert ( oss . str () == "2000-01-01" );
  iss . clear ();
  iss . str ( "2000-11-31" );
  assert ( ! ( iss >> d ) );
  oss . str ("");
  oss << d;
  assert ( oss . str () == "2000-01-01" );
  iss . clear ();
  iss . str ( "2000-02-29" );
  assert ( ( iss >> d ) );
  oss . str ("");
  oss << d;
  assert ( oss . str () == "2000-02-29" );
  iss . clear ();
  iss . str ( "2001-02-29" );
  assert ( ! ( iss >> d ) );
  oss . str ("");
  oss << d;
  assert ( oss . str () == "2000-02-29" );

  //-----------------------------------------------------------------------------
  // bonus test examples
  //-----------------------------------------------------------------------------
  CDate f ( 2000, 5, 12 );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2000-05-12" );
  oss . str ("");
  oss << date_format ( "%Y/%m/%d" ) << f;
  assert ( oss . str () == "2000/05/12" );
  oss . str ("");
  oss << date_format ( "%d.%m.%Y" ) << f;
  assert ( oss . str () == "12.05.2000" );
  oss . str ("");
  oss << date_format ( "%m/%d/%Y" ) << f;
  assert ( oss . str () == "05/12/2000" );
  oss . str ("");
  oss << date_format ( "%Y%m%d" ) << f;
  assert ( oss . str () == "20000512" );
  oss . str ("");
  oss << date_format ( "hello kitty" ) << f;
  assert ( oss . str () == "hello kitty" );
  oss . str ("");
  oss << date_format ( "%d%d%d%d%d%d%m%m%m%Y%Y%Y%%%%%%%%%%" ) << f;
  assert ( oss . str () == "121212121212050505200020002000%%%%%" );
  oss . str ("");
  oss << date_format ( "%Y-%m-%d" ) << f;
  assert ( oss . str () == "2000-05-12" );
  iss . clear ();
  iss . str ( "2001-01-1" );
  assert ( ! ( iss >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2000-05-12" );
  iss . clear ();
  iss . str ( "2001-1-01" );
  assert ( ! ( iss >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2000-05-12" );
  iss . clear ();
  iss . str ( "2001-001-01" );
  assert ( ! ( iss >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2000-05-12" );
  iss . clear ();
  iss . str ( "2001-01-02" );
  assert ( ( iss >> date_format ( "%Y-%m-%d" ) >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2001-01-02" );
  iss . clear ();
  iss . str ( "05.06.2003" );
  assert ( ( iss >> date_format ( "%d.%m.%Y" ) >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2003-06-05" );
  iss . clear ();
  iss . str ( "07/08/2004" );
  assert ( ( iss >> date_format ( "%m/%d/%Y" ) >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2004-07-08" );
  iss . clear ();
  iss . str ( "2002*03*04" );
  assert ( ( iss >> date_format ( "%Y*%m*%d" ) >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2002-03-04" );
  iss . clear ();
  iss . str ( "C++09format10PA22006rulez" );
  assert ( ( iss >> date_format ( "C++%mformat%dPA2%Yrulez" ) >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2006-09-10" );
  iss . clear ();
  iss . str ( "%12%13%2010%" );
  assert ( ( iss >> date_format ( "%%%m%%%d%%%Y%%" ) >> f ) );
  oss . str ("");
  oss << f;
  assert ( oss . str () == "2010-12-13" );

  CDate g ( 2000, 6, 8 );
  iss . clear ();
  iss . str ( "2001-11-33" );
  assert ( ! ( iss >> date_format ( "%Y-%m-%d" ) >> g ) );
  oss . str ("");
  oss << g;
  assert ( oss . str () == "2000-06-08" );
  iss . clear ();
  iss . str ( "29.02.2003" );
  assert ( ! ( iss >> date_format ( "%d.%m.%Y" ) >> g ) );
  oss . str ("");
  oss << g;
  assert ( oss . str () == "2000-06-08" );
  iss . clear ();
  iss . str ( "14/02/2004" );
  assert ( ! ( iss >> date_format ( "%m/%d/%Y" ) >> g ) );
  oss . str ("");
  oss << g;
  assert ( oss . str () == "2000-06-08" );
  iss . clear ();
  iss . str ( "2002-03" );
  assert ( ! ( iss >> date_format ( "%Y-%m" ) >> g ) );
  oss . str ("");
  oss << g;
  assert ( oss . str () == "2000-06-08" );
  iss . clear ();
  iss . str ( "hello kitty" );
  assert ( ! ( iss >> date_format ( "hello kitty" ) >> g ) );
  oss . str ("");
  oss << g;
  assert ( oss . str () == "2000-06-08" );
  iss . clear ();
  iss . str ( "2005-07-12-07" );
  assert ( ! ( iss >> date_format ( "%Y-%m-%d-%m" ) >> g ) );
  oss . str ("");
  oss << g;
  assert ( oss . str () == "2000-06-08" );
  iss . clear ();
  iss . str ( "20000101" );
  assert ( ( iss >> date_format ( "%Y%m%d" ) >> g ) );
  oss . str ("");
  oss << g;
  assert ( oss . str () == "2000-01-01" );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
