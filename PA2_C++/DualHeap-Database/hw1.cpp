#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream> 
#include <iomanip> 
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
#endif /* __PROGTEST__ */

// Struct to bundle taxID, name, and address
struct Company {
    std::string taxID;
    std::pair <std::string, std::string> name_a_addr;
    unsigned int income;
    std::pair<std::string, std::string> key;
};

// Main class implementation
class CVATRegister
{
  public:
                  CVATRegister   () {data_byID.reserve(100); data_byNA.reserve(100);} // Reserve 100 spaces
                  ~CVATRegister  () {}
    bool          newCompany     ( const std::string    & name,
                                   const std::string    & addr,
                                   const std::string    & taxID );
    bool          cancelCompany  ( const std::string    & name,
                                   const std::string    & addr );
    bool          cancelCompany  ( const std::string    & taxID );
    bool          invoice        ( const std::string    & taxID,
                                   unsigned int           amount );
    bool          invoice        ( const std::string    & name,
                                   const std::string    & addr,
                                   unsigned int           amount );
    bool          auditCompany   ( const std::string    & name,
                                   const std::string    & addr,
                                   unsigned int         & sumIncome ) const;
    bool          auditCompany   ( const std::string    & taxID,
                                   unsigned int         & sumIncome ) const;
    bool          firstCompany   ( std::string          & name,
                                   std::string          & addr ) const;
    bool          nextCompany    ( std::string          & name,
                                   std::string          & addr ) const;
    unsigned int  medianInvoice  () const;
  private:
    // Attribute
    std::vector<Company> data_byID; // Databases
    std::vector<Company> data_byNA;
    std::vector <unsigned int> lower_income;
    std::vector <unsigned int> higher_income;
    // Method
    void dual_heap (unsigned int amount); // Push the data into a dual heap for median income finding
    std::string lower (std::string str) const; // Lowercase a string
    auto position_byID (const std::string & taxID); // Read position by taxID
    auto position_byID (const std::string & taxID) const; // by taxID (read-only)
    auto position_byNA (const std::string & name, const std::string & addr); // by name a addr 
    auto position_byNA (const std::string & name, const std::string & addr) const; // by n&a (read-only)
};

// Dual-heaping
void CVATRegister::dual_heap (unsigned int amount) {
  // Push in lower (largest front - std heap)
  if (lower_income.empty() || amount <= lower_income.front()) {
    lower_income.push_back(amount);
    std::push_heap(lower_income.begin(), lower_income.end());
  }
  // Push in higher (lowest front)
  else {
    higher_income.push_back(amount);
    std::push_heap(higher_income.begin(), higher_income.end(), std::greater<unsigned int>());
  }

  // Balance
  if (lower_income.size() > higher_income.size() + 1) {
    std::pop_heap(lower_income.begin(), lower_income.end());
    higher_income.push_back(lower_income.back());
    std::push_heap(higher_income.begin(), higher_income.end(), std::greater<unsigned int>());
    lower_income.pop_back();
  }
  else if (higher_income.size() > lower_income.size() + 1) {
    std::pop_heap(higher_income.begin(), higher_income.end(), std::greater<unsigned int>());
    lower_income.push_back(higher_income.back());
    std::push_heap(lower_income.begin(), lower_income.end());
    higher_income.pop_back();
  }
}

// Lowercase a string
std::string CVATRegister::lower (std::string str) const {
  std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){return std::tolower(c);});
  return str;
}

// Return iterator position to insert/cancel company (Overloaded)
auto CVATRegister::position_byID (const std::string & taxID) {
  return std::lower_bound(data_byID.begin(), data_byID.end(), taxID, 
  [](const Company & c, const std::string & ID){return c.taxID < ID;});
}
auto CVATRegister::position_byID (const std::string & taxID) const {
  return std::lower_bound(data_byID.begin(), data_byID.end(), taxID, 
  [](const Company & c, const std::string & ID){return c.taxID < ID;});
}
auto CVATRegister::position_byNA (const std::string & name, const std::string & addr) {
  auto name_ = lower(name);
  auto addr_ = lower(addr);
  return std::lower_bound(data_byNA.begin(), data_byNA.end(), std::make_pair(name_, addr_), 
  [](const Company & c, const std::pair<std::string, std::string> & NA){
    if (c.key.first != NA.first) return c.key.first < NA.first;
    else return c.key.second < NA.second;
  });
}
auto CVATRegister::position_byNA (const std::string & name, const std::string & addr) const {
  auto name_ = lower(name);
  auto addr_ = lower(addr);
  return std::lower_bound(data_byNA.begin(), data_byNA.end(), std::make_pair(name_, addr_), 
  [](const Company & c, const std::pair<std::string, std::string> & NA){
    if (c.key.first != NA.first) return c.key.first < NA.first;
    else return c.key.second < NA.second;
  });
}

// Validate a new company and insert to database
bool CVATRegister::newCompany ( const std::string    & name,
                                const std::string    & addr,
                                const std::string    & taxID ) {
  // Check taxID
  auto pos_ID = position_byID(taxID);
  if ((pos_ID != data_byID.end() && pos_ID->taxID == taxID)) return false;

  // Check name a addr
  auto name_ = lower(name); auto addr_ = lower(addr);
  auto pos_NA = position_byNA (name, addr);
  if (pos_NA != data_byNA.end() && pos_NA->key.first == name_
    && pos_NA->key.second == addr_) return false;

  // Insert into main databases
  Company tmp = {taxID, {name, addr}, 0, {name_, addr_}};
  data_byID.insert (pos_ID, tmp);
  data_byNA.insert (pos_NA, tmp);
  return true;
}

// Remove a company with name and addr
bool CVATRegister::cancelCompany  ( const std::string    & name,
                                    const std::string    & addr ) {
  // Lookup w n&a
  auto pos_NA = position_byNA (name, addr);
  if (pos_NA == data_byNA.end() || pos_NA->key.first != lower(name)
    || pos_NA->key.second != lower(addr)) return false;
  
  // Erase
  auto pos_ID = position_byID(pos_NA->taxID);
  data_byID.erase(pos_ID);
  data_byNA.erase(pos_NA);
  return true;
}

// Remove a company with taxID
bool CVATRegister::cancelCompany  ( const std::string    & taxID ) {

  // Lookup w taxID
  auto pos_ID = position_byID(taxID);
  if (pos_ID == data_byID.end() || pos_ID->taxID != taxID) return false;
  auto pos_NA = position_byNA(pos_ID->name_a_addr.first, pos_ID->name_a_addr.second);

  // Erase
  data_byNA.erase(pos_NA);
  data_byID.erase(pos_ID);
  return true;
}

// Add income to a company thru taxid
bool CVATRegister::invoice        ( const std::string    & taxID,
                                unsigned int           amount ) {
  // Look
  auto pos_ID = position_byID(taxID);
  if (pos_ID == data_byID.end() || pos_ID->taxID != taxID) return false;
  auto pos_NA = position_byNA(pos_ID->name_a_addr.first, pos_ID->name_a_addr.second);

  // Add to main database
  pos_ID->income += amount;
  pos_NA->income += amount;

  // Add to income database
  dual_heap (amount);
  return true;
}

// Add income thru n&a
bool CVATRegister::invoice        ( const std::string    & name,
                                const std::string    & addr,
                                unsigned int           amount ) {
  // Look
  auto pos_NA = position_byNA(name, addr);
  if (pos_NA == data_byNA.end() || pos_NA->key.first != lower(name) || pos_NA->key.second != lower(addr)) return false;
  auto pos_ID = position_byID(pos_NA->taxID);

  // Add to main database
  pos_NA->income += amount;
  pos_ID->income += amount;

  // Add to median_data database
  dual_heap (amount);
  return true;
}

// Lookup income thru n&a
bool CVATRegister::auditCompany   ( const std::string    & name,
                                const std::string    & addr,
                                unsigned int         & sumIncome ) const {
  // Look
  auto pos_NA = position_byNA (name, addr);
  if (pos_NA == data_byNA.end() || pos_NA->key.first != lower(name) || pos_NA->key.second != lower(addr)) return false;

  // Output
  sumIncome = pos_NA->income;
  return true;
}

// Lookup income thru taxid
bool CVATRegister::auditCompany   ( const std::string    & taxID,
                                unsigned int         & sumIncome ) const {
  // Look
  auto pos_ID = position_byID(taxID);
  if (pos_ID == data_byID.end() || pos_ID->taxID != taxID) return false;

  // Output
  sumIncome = pos_ID->income;
  return true;
}

// Find the first company lexicographically name then addr
bool CVATRegister::firstCompany   ( std::string          & name,
                                std::string          & addr ) const {
  // If empty, false
  if (data_byNA.empty()) return false;

  // If not, get first element
  name = data_byNA[0].name_a_addr.first;
  addr = data_byNA[0].name_a_addr.second;
  return true; 
}

// Find the next company lexicographically name then addr
bool CVATRegister::nextCompany    ( std::string          & name,
                                std::string          & addr ) const {
  // If empty, false
  if (data_byNA.empty()) return false;

  // Lookup w n&a
  auto pos_NA = position_byNA(name, addr);

  // If the end or not matching, false
  if (pos_NA == data_byNA.end() || pos_NA->key.first != lower(name) || pos_NA->key.second != lower(addr)) return false;

  // If found, see if the next one exist
  if ((pos_NA+1) != data_byNA.end()) {
    name = (pos_NA+1)->name_a_addr.first;
    addr = (pos_NA+1)->name_a_addr.second;
    return true;
  }
  // If not, false
  return false;
}

// Median income of the whole database
unsigned int  CVATRegister::medianInvoice  () const {
  // If empty, 0
  if (lower_income.empty() && higher_income.empty()) return 0;

  // Return middle element (or middle + 1 if even)
  unsigned res = 0;
  if (lower_income.size() > higher_income.size()) {
    res = lower_income.front();
  }
  else {
    res = higher_income.front();
  }
  return res;
}

#ifndef __PROGTEST__
int               main           ()
{
  std::string name, addr;
  unsigned int sumIncome;

  CVATRegister b1;
  assert ( b1 . newCompany ( "ACME", "Thakurova", "666/666" ) );
  assert ( b1 . newCompany ( "ACME", "Kolejni", "666/666/666" ) );
  assert ( b1 . newCompany ( "Dummy", "Thakurova", "123456" ) );
  assert ( b1 . invoice ( "666/666", 2000 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "666/666/666", 3000 ) );
  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "123456", 4000 ) );
  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "aCmE", "Kolejni", 5000 ) );
  assert ( b1 . medianInvoice () == 4000 );
  assert ( b1 . auditCompany ( "ACME", "Kolejni", sumIncome ) && sumIncome == 8000 );
  assert ( b1 . auditCompany ( "123456", sumIncome ) && sumIncome == 4000 );
  assert ( b1 . firstCompany ( name, addr ) && name == "ACME" && addr == "Kolejni" );
  assert ( b1 . nextCompany ( name, addr ) && name == "ACME" && addr == "Thakurova" );
  assert ( b1 . nextCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
  assert ( ! b1 . nextCompany ( name, addr ) );
  assert ( b1 . cancelCompany ( "ACME", "KoLeJnI" ) );
  assert ( b1 . medianInvoice () == 4000 );
  assert ( b1 . cancelCompany ( "666/666" ) );
  assert ( b1 . medianInvoice () == 4000 );
  assert ( b1 . invoice ( "123456", 100 ) );
  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "123456", 300 ) );
  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "123456", 200 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "123456", 230 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "123456", 830 ) );
  assert ( b1 . medianInvoice () == 830 );
  assert ( b1 . invoice ( "123456", 1830 ) );
  assert ( b1 . medianInvoice () == 1830 );
  assert ( b1 . invoice ( "123456", 2830 ) );
  assert ( b1 . medianInvoice () == 1830 );
  assert ( b1 . invoice ( "123456", 2830 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "123456", 3200 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . firstCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
  assert ( ! b1 . nextCompany ( name, addr ) );
  assert ( b1 . cancelCompany ( "123456" ) );
  assert ( ! b1 . firstCompany ( name, addr ) );

  CVATRegister b2;
  assert ( b2 . newCompany ( "ACME", "Kolejni", "abcdef" ) );
  assert ( b2 . newCompany ( "Dummy", "Kolejni", "123456" ) );
  assert ( ! b2 . newCompany ( "AcMe", "kOlEjNi", "1234" ) );
  assert ( b2 . newCompany ( "Dummy", "Thakurova", "ABCDEF" ) );
  assert ( b2 . medianInvoice () == 0 );
  assert ( b2 . invoice ( "ABCDEF", 1000 ) );
  assert ( b2 . medianInvoice () == 1000 );
  assert ( b2 . invoice ( "abcdef", 2000 ) );
  assert ( b2 . medianInvoice () == 2000 );
  assert ( b2 . invoice ( "aCMe", "kOlEjNi", 3000 ) );
  assert ( b2 . medianInvoice () == 2000 );
  assert ( ! b2 . invoice ( "1234567", 100 ) );
  assert ( ! b2 . invoice ( "ACE", "Kolejni", 100 ) );
  assert ( ! b2 . invoice ( "ACME", "Thakurova", 100 ) );
  assert ( ! b2 . auditCompany ( "1234567", sumIncome ) );
  assert ( ! b2 . auditCompany ( "ACE", "Kolejni", sumIncome ) );
  assert ( ! b2 . auditCompany ( "ACME", "Thakurova", sumIncome ) );
  assert ( ! b2 . cancelCompany ( "1234567" ) );
  assert ( ! b2 . cancelCompany ( "ACE", "Kolejni" ) );
  assert ( ! b2 . cancelCompany ( "ACME", "Thakurova" ) );
  assert ( b2 . cancelCompany ( "abcdef" ) );
  assert ( b2 . medianInvoice () == 2000 );
  assert ( ! b2 . cancelCompany ( "abcdef" ) );
  assert ( b2 . newCompany ( "ACME", "Kolejni", "abcdef" ) );
  assert ( b2 . cancelCompany ( "ACME", "Kolejni" ) );
  assert ( ! b2 . cancelCompany ( "ACME", "Kolejni" ) );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */