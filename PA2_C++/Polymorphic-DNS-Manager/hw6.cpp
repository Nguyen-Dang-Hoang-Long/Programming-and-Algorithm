#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#include <compare>
#include "ipaddress.h"
using namespace std::literals;
#endif /* __PROGTEST__ */

class CRec {
  protected:
    std::string r_name;
  public:
    CRec() = default;
    virtual ~CRec() = default;
    CRec(const std::string& n) : r_name(n) {}
    CRec(const CRec& r) : r_name(r.name()) {}
    std::string name() const {return r_name;}
    virtual std::string type() const = 0;
    virtual void print (std::ostream& os) const {
      os << name() << " " << type() << " ";
    };
    virtual CRec * clone() const = 0;
    virtual bool operator == (const CRec&) const = 0;
};

class CRecA : public CRec
{
  CIPv4 addr;
  public:
    // constructor
    CRecA(const std::string& n, const CIPv4& IP) : CRec(n) { std::stringstream str; str << IP; str >> addr;}
    // copy
    CRecA(const CRecA& r) : CRecA(r.name(), r.addr) {}
    // name ()
    // type ()
    std::string type() const override { return "A"; }
    // operator <<
    void print(std::ostream& os) const override {
      CRec::print(os);
      os << addr;
    }
    // clone
    CRecA * clone () const override {return new CRecA(*this);}
    // operator ==
    bool operator == (const CRec& r) const override {
      const CRecA * cast = dynamic_cast<const CRecA*>(&r);
      if (!cast) return false;
      else return name() == cast->name() && addr == cast->addr;
    }
};

class CRecAAAA : public CRec
{
  CIPv6 addr;
  public:
    // constructor
    CRecAAAA(const std::string& n, const CIPv6& IP) : CRec(n) {std::stringstream str; str << IP; str >> addr;}
    // copy
    CRecAAAA(const CRecAAAA& r) : CRecAAAA(r.name(), r.addr) {}
    // name ()
    // type ()
    std::string type() const override {return "AAAA";}
    // operator <<
    void print(std::ostream& os) const override {
      CRec::print(os);
      os << addr;
    }
    // clone
    CRecAAAA * clone () const override {return new CRecAAAA(*this);}
    // operator ==
    bool operator == (const CRec& r) const override {
      const CRecAAAA * cast = dynamic_cast<const CRecAAAA *>(&r);
      if (!cast) return false;
      else return name() == cast->name() && addr == cast->addr;
    }
};

class CRecMX : public CRec
{
  std::string server;
  int priority;
  public:
    // constructor
    CRecMX(const std::string& n, const std::string& s, const int p) : CRec(n), server(s), priority(p) {}
    // copy
    CRecMX (const CRecMX& r) : CRecMX(r.name(), r.server, r.priority) {}
    // name ()
    // type ()
    std::string type() const override {return "MX";}
    // operator <<
    void print(std::ostream& os) const override {
      CRec::print(os);
      os << priority << " " << server;
    }
    // clone
    CRecMX * clone() const override {return new CRecMX(*this);}
    // operator ==
    bool operator == (const CRec& r) const override {
      const CRecMX * cast = dynamic_cast<const CRecMX*>(&r);
      if (!cast) return false;
      else return name() == cast->name()
      && server == cast->server
      && priority == cast->priority;
    }
};

class CRecCNAME : public CRec
{
  std::string target;
  public:
    // constructor
    CRecCNAME (const std::string& a, const std::string& n) : CRec(a), target(n) {}
    // copy
    CRecCNAME (const CRecCNAME& cname) : CRecCNAME (cname.r_name, cname.target) {}
    // name ()
    // type ()
    std::string type() const override {return "CNAME";}
    // operator <<
    void print(std::ostream& os) const override {
      CRec::print(os);
      os << target;
    }
    CRecCNAME * clone() const override { return new CRecCNAME(*this);}
    bool operator == (const CRec& r) const override {      
      const CRecCNAME * cast = dynamic_cast<const CRecCNAME *>(&r);
      if (!cast) return false;
      else return name() == cast->name() && target == cast->target;
    }
};

class CRecSPF : public CRec
{
  std::vector<std::string> mails;
  public:
    // constructor
    CRecSPF (const std::string& n) : CRec(n) {}
    // name ()
    // type ()
    std::string type() const override {return "SPF";}
    // add ()
    CRecSPF& add (const std::string& m) {
      mails.push_back(m);
      return *this;
    }
    // operator <<
    void print (std::ostream& os) const override {      
      CRec::print(os);
      size_t i = 0;
      for (const auto& m : mails) {
        os << m;
        if (i++ != mails.size() - 1) os << ", ";
      }
    }
    CRecSPF * clone () const override {return new CRecSPF(*this);}
    bool operator == (const CRec& r) const override {
      const CRecSPF * cast = dynamic_cast<const CRecSPF *>(&r);
      if (!cast) return false;
      else return name() == cast->name() && mails == cast->mails;
    }
};

class CZone;

class CResult {
  std::vector <CRec*> res;
  public:
    void push (CRec* r) {res.push_back(r);}
    size_t size() const {return res.size();}
    CRec& operator[] (size_t i) {
      if (i >= size()) throw std::out_of_range("Out of range");
      return *res[i];
    }
    friend std::ostream& operator << (std::ostream& os, const CResult& result); 
};

class CZone : public CRec
{
  // struct to hold each CRec
  struct CBlock {
    CRec * rec;
    CBlock * next;
    CBlock(CRec * r, CBlock * b = nullptr) : rec(r), next(b) {}
    ~CBlock () {delete rec;}
  };

  // linked list of struct
  CBlock * head = nullptr;
  CBlock * tail = nullptr;
  size_t cap = 0;

  // index for fast search
  std::unordered_map<std::string, std::vector<CRec*>> index;

  // copy for both copy constructor and assign operator =
  void generic_copy(const CZone& cz) {
      r_name = cz.r_name;
      cap = cz.cap;
      head = tail = nullptr;
      index.clear();
      if (empty()) return;

      CBlock * ptr = cz.head;
      head = new CBlock (ptr->rec->clone());
      index[head->rec->name()].push_back(head->rec);
      
      CBlock * track = head;
      ptr = ptr->next;
      while (ptr) {
        track->next = new CBlock(ptr->rec->clone());
        track = track->next;
        index[track->rec->name()].push_back(track->rec);
        ptr = ptr->next;
      }
      tail = track;
  }
  // exist ()
  bool exist (const CRec& rec) const {
    auto it = index.find(rec.name());
    if (it == index.end()) return false;
    if (rec.type() == "CNAME" || rec.type() == "ZONE") return true;
    for (const CRec* p : it->second) {
      if (p->type() == "CNAME" || p->type() == "ZONE") return true;
      else if (*p == rec) return true;
    }
    return false;
  }
  // empty ()
  bool empty() const {
    return !cap;
  }
  // remove an index
  void remove (const CRec& rec) {
    auto& vec = index[rec.name()];
    vec.erase(std::remove_if(vec.begin(), vec.end(),
      [&](const CRec* p){ return *p == rec; }), vec.end());
    if (vec.empty()) index.erase(rec.name());
  }
  // delete a whole zone
  void mass_delete () {
      for (CBlock * ptr = head; ptr;) {
        CBlock * tmp = ptr->next;
        delete ptr;
        ptr = tmp;
      }
  }
  void printRcs (std::ostream& os, const std::string& prefix) const {
    for (CBlock * it = head; it; it = it->next) {
      os << prefix << ((it == tail) ? "\\- " : "+- "); 
      it->rec->print(os);
      os << '\n';
      CZone * sub = dynamic_cast<CZone* >(it->rec);
      if (sub) {
        std::string new_prefix = prefix + (it == tail ? "   " : "|  ");
        sub->printRcs(os, new_prefix);
      }
    }
  }
  public:
    // constructor(s)
    CZone(const std::string& n) : CRec(n) {}
    // destructor (if needed)
    ~CZone() {
      mass_delete();
    }
    // operator = (if needed)
    CZone(const CZone& cz) : CRec(cz.name()) {
      generic_copy(cz);
    }
    CZone& operator = (const CZone& cz) {
      if (this == &cz) return *this;
      mass_delete();
      generic_copy(cz);
      return *this;
    }
    // add ()
    bool add(const CRec& rec) {
      if (exist(rec)) return false; // if exist, false
      CBlock * tmp = new CBlock(rec.clone()); 
      if(empty()) head = tmp; // if empty list, initiate head
      else tail->next = tmp; // if not, initiate tail->next
      tail = tmp; // update tail
      index[rec.name()].push_back(tmp->rec);  // O(1) index update
      cap++; // incre size
      return true;
    }
    // del ()
    bool del(const CRec& rec) {
      bool found = false;

      if (empty()) return found; // empty list, false

      CBlock * ptr = head; 
      if (*ptr->rec == rec) { // first node
        head = head->next;
        remove(*ptr->rec);
        delete ptr;
        found = true;
      } 
      else { // any other node
        for (; ptr->next ; ptr = ptr->next) {
          if (*ptr->next->rec == rec) {
            CBlock * tmp = ptr->next;
            ptr->next = ptr->next->next;
            if (!ptr->next) tail = ptr;
            remove(*tmp->rec);
            delete tmp;
            found = true;
            break;
          }
        }
      }

      if (!found) return found; // cant find

      cap--; // update size if found (and thus deleted)

      if (empty()) head = tail = nullptr; // null head and tail if empty

      return found; 
    }
    // search ()
    CResult search (const std::string& n) const {
      // Clear last '.'
      std::string remaining = n;
      if (!remaining.empty() && remaining.back() == '.') {
        remaining.pop_back(); 
      }
      // Index Search - No dot
      size_t dot = remaining.find_last_of(".");
      CResult res;
      if (dot == std::string::npos)  {
        auto it = index.find(remaining);
        if (it == index.end()) return res;
        for (CRec* r : it->second) res.push(r);
        return res;
      }
      // Zone Search - Dot
      std::string subzone = remaining.substr(dot + 1);
      remaining = remaining.substr(0, dot);
      auto it = index.find(subzone);
      if (it != index.end()) {
        for (CRec * r : it->second) {
          CZone * cast = dynamic_cast<CZone *>(r);
          if (cast) return cast->search(remaining);
        }
      }
      return res;
    }  
    // operator <<
    std::string type() const override {return "ZONE";}
    void print (std::ostream& os) const override {
      os << name();
    };
    CZone * clone() const override {return new CZone(*this);}
    bool operator == (const CRec& r) const override {
      const CZone * cast = dynamic_cast<const CZone *>(&r);
      if (!cast) return false;
      else return name() == cast->name(); 
    }
    friend std::ostream& operator<<(std::ostream& os, const CZone& z);
};

std::ostream& operator << (std::ostream& os, const CRec& rec) {
  rec.print(os);
  return os;
}
std::ostream& operator<<(std::ostream& os, const CZone& z) {
  z.print(os);
  os << '\n';
  z.printRcs(os, " ");
  return os;
}

std::ostream& operator << (std::ostream& os, const CResult& result) {
  for (CRec* r : result.res) {
    CZone * cast = dynamic_cast<CZone *>(r);
    if (cast) os << *cast;
    else {r->print(os); os << '\n';}
  }
  return os;
}


#ifndef __PROGTEST__
int main ()
{
  std::ostringstream oss;

  CZone z0 ( "fit" );
  assert ( z0 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z0 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.158" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.159" ) ) ) == true );
  assert ( z0 . add ( CRecCNAME ( "pririz", "sto.fit.cvut.cz." ) ) == true );
  assert ( z0 . add ( CRecSPF ( "courses" ) . add ( "ip4:147.32.232.128/25" ) . add ( "ip4:147.32.232.64/26" ) ) == true );
  assert ( z0 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:1:2:3:4" ) ) ) == true );
  assert ( z0 . add ( CRecMX ( "courses", "relay.fit.cvut.cz.", 0 ) ) == true );
  assert ( z0 . add ( CRecMX ( "courses", "relay2.fit.cvut.cz.", 10 ) ) == true );
  oss . str ( "" );
  oss << z0;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- courses MX 0 relay.fit.cvut.cz.\n"
    " \\- courses MX 10 relay2.fit.cvut.cz.\n" );
  assert ( z0 . search ( "progtest" ) . size () == 3 );
  oss . str ( "" );
  oss << z0 . search ( "progtest" );
  assert ( oss . str () == 
    "progtest A 147.32.232.142\n"
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "progtest AAAA 2001:718:2:2902:1:2:3:4\n" );
  assert ( z0 . del ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.122" ) ) ) == true );
  oss . str ( "" );
  oss << z0;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- courses MX 0 relay.fit.cvut.cz.\n"
    " +- courses MX 10 relay2.fit.cvut.cz.\n"
    " \\- courses A 147.32.232.122\n" );
  assert ( z0 . search ( "courses" ) . size () == 6 );
  oss . str ( "" );
  oss << z0 . search ( "courses" );
  assert ( oss . str () == 
    "courses A 147.32.232.158\n"
    "courses A 147.32.232.159\n"
    "courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "courses MX 0 relay.fit.cvut.cz.\n"
    "courses MX 10 relay2.fit.cvut.cz.\n"
    "courses A 147.32.232.122\n" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 0 ];
  assert ( oss . str () == "courses A 147.32.232.158" );
  assert ( z0 . search ( "courses" ) [ 0 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 0 ] . type () == "A" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 1 ];
  assert ( oss . str () == "courses A 147.32.232.159" );
  assert ( z0 . search ( "courses" ) [ 1 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 1 ] . type () == "A" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 2 ];
  assert ( oss . str () == "courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26" );
  assert ( z0 . search ( "courses" ) [ 2 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 2 ] . type () == "SPF" );
  try
  {
    oss . str ( "" );
    oss << z0 . search ( "courses" ) [ 10 ];
    assert ( "No exception thrown!" == nullptr );
  }
  catch ( const std::out_of_range & e )
  {
  }
  catch ( ... )
  {
    assert ( "Invalid exception thrown!" == nullptr );
  }
  dynamic_cast<const CRecAAAA &> ( z0 . search ( "progtest" ) [ 1 ] );
  CZone z1 ( "fit2" );
  z1 . add ( z0 . search ( "progtest" ) [ 2 ] );
  z1 . add ( z0 . search ( "progtest" ) [ 0 ] );
  z1 . add ( z0 . search ( "progtest" ) [ 1 ] );
  z1 . add ( z0 . search ( "courses" ) [ 2 ] );
  oss . str ( "" );
  oss << z1;
  assert ( oss . str () == 
    "fit2\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n" );
  dynamic_cast<const CRecA &> ( z1 . search ( "progtest" ) [ 1 ] );

  CZone z10 ( "fit" );
  assert ( z10 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z10 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z10 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.144" ) ) ) == true );
  assert ( z10 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 10 ) ) == true );
  assert ( z10 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == false );
  assert ( z10 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.140" ) ) ) == false );
  assert ( z10 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z10 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == false );
  assert ( z10 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 20 ) ) == true );
  assert ( z10 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 10 ) ) == false );
  assert ( z10 . add ( CRecCNAME ( "pririz", "sto.fit.cvut.cz." ) ) == true );
  assert ( z10 . add ( CRecCNAME ( "pririz", "stojedna.fit.cvut.cz." ) ) == false );
  assert ( z10 . add ( CRecA ( "pririz", CIPv4 ( "147.32.232.111" ) ) ) == false );
  assert ( z10 . add ( CRecCNAME ( "progtest", "progtestbak.fit.cvut.cz." ) ) == false );
  assert ( z10 . add ( CZone ( "test" ) ) == true );
  assert ( z10 . add ( CZone ( "pririz" ) ) == false );
  assert ( z10 . add ( CRecA ( "test", CIPv4 ( "147.32.232.232" ) ) ) == false );
  oss . str ( "" );
  oss << z10;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- progtest A 147.32.232.144\n"
    " +- progtest MX 10 relay.fit.cvut.cz.\n"
    " +- progtest MX 20 relay.fit.cvut.cz.\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " \\- test\n" );
  assert ( z10 . search ( "progtest" ) . size () == 4 );
  oss . str ( "" );
  oss << z10 . search ( "progtest" );
  assert ( oss . str () == 
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "progtest A 147.32.232.144\n"
    "progtest MX 10 relay.fit.cvut.cz.\n"
    "progtest MX 20 relay.fit.cvut.cz.\n" );
  assert ( z10 . search ( "courses" ) . size () == 0 );
  oss . str ( "" );
  oss << z10 . search ( "courses" );
  assert ( oss . str () == "" );

  CZone z20 ( "<ROOT ZONE>" );
  CZone z21 ( "cz" );
  CZone z22 ( "cvut" );
  CZone z23 ( "fit" );
  assert ( z23 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z23 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z23 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.158" ) ) ) == true );
  assert ( z23 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z23 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.159" ) ) ) == true );
  assert ( z23 . add ( CRecCNAME ( "pririz", "sto.fit.cvut.cz." ) ) == true );
  assert ( z23 . add ( CRecSPF ( "courses" ) . add ( "ip4:147.32.232.128/25" ) . add ( "ip4:147.32.232.64/26" ) ) == true );
  CZone z24 ( "fel" );
  assert ( z24 . add ( CRecA ( "www", CIPv4 ( "147.32.80.2" ) ) ) == true );
  assert ( z24 . add ( CRecAAAA ( "www", CIPv6 ( "1:2:3:4:5:6:7:8" ) ) ) == true );
  assert ( z22 . add ( z23 ) == true );
  assert ( z22 . add ( z24 ) == true );
  assert ( z21 . add ( z22 ) == true );
  assert ( z20 . add ( z21 ) == true );
  assert ( z23 . add ( CRecA ( "www", CIPv4 ( "147.32.90.1" ) ) ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  oss . str ( "" );
  oss << z21;
  assert ( oss . str () == 
    "cz\n"
    " \\- cvut\n"
    "    +- fit\n"
    "    |  +- progtest A 147.32.232.142\n"
    "    |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "    |  +- courses A 147.32.232.158\n"
    "    |  +- courses A 147.32.232.160\n"
    "    |  +- courses A 147.32.232.159\n"
    "    |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "    |  \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "    \\- fel\n"
    "       +- www A 147.32.80.2\n"
    "       \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  oss . str ( "" );
  oss << z22;
  assert ( oss . str () == 
    "cvut\n"
    " +- fit\n"
    " |  +- progtest A 147.32.232.142\n"
    " |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |  +- courses A 147.32.232.158\n"
    " |  +- courses A 147.32.232.160\n"
    " |  +- courses A 147.32.232.159\n"
    " |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |  \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " \\- fel\n"
    "    +- www A 147.32.80.2\n"
    "    \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  oss . str ( "" );
  oss << z23;
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " \\- www A 147.32.90.1\n" );
  oss . str ( "" );
  oss << z24;
  assert ( oss . str () == 
    "fel\n"
    " +- www A 147.32.80.2\n"
    " \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( z20 . search ( "progtest.fit.cvut.cz" ) . size () == 2 );
  oss . str ( "" );
  oss << z20 . search ( "progtest.fit.cvut.cz" );
  assert ( oss . str () == 
    "progtest A 147.32.232.142\n"
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n" );
  assert ( z20 . search ( "fit.cvut.cz" ) . size () == 1 );
  oss . str ( "" );
  oss << z20 . search ( "fit.cvut.cz" );
  assert ( oss . str () == 
    "fit\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- pririz CNAME sto.fit.cvut.cz.\n"
    " \\- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz" ) [0] ) . add ( z20 . search ( "fel.cvut.cz" ) [0] ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  \\- fel\n"
    "       |     +- www A 147.32.80.2\n"
    "       |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz" ) [0] ) . add ( z20 . search ( "cz" ) [0] ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  +- fel\n"
    "       |  |  +- www A 147.32.80.2\n"
    "       |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |  \\- cz\n"
    "       |     \\- cvut\n"
    "       |        +- fit\n"
    "       |        |  +- progtest A 147.32.232.142\n"
    "       |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |  +- courses A 147.32.232.158\n"
    "       |        |  +- courses A 147.32.232.160\n"
    "       |        |  +- courses A 147.32.232.159\n"
    "       |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |  \\- fel\n"
    "       |        |     +- www A 147.32.80.2\n"
    "       |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        \\- fel\n"
    "       |           +- www A 147.32.80.2\n"
    "       |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz.fit.cvut.cz" ) [0] ) . add ( z20 . search ( "cz" ) [0] ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  +- fel\n"
    "       |  |  +- www A 147.32.80.2\n"
    "       |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |  \\- cz\n"
    "       |     \\- cvut\n"
    "       |        +- fit\n"
    "       |        |  +- progtest A 147.32.232.142\n"
    "       |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |  +- courses A 147.32.232.158\n"
    "       |        |  +- courses A 147.32.232.160\n"
    "       |        |  +- courses A 147.32.232.159\n"
    "       |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |  +- fel\n"
    "       |        |  |  +- www A 147.32.80.2\n"
    "       |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |  \\- cz\n"
    "       |        |     \\- cvut\n"
    "       |        |        +- fit\n"
    "       |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |  +- fel\n"
    "       |        |        |  |  +- www A 147.32.80.2\n"
    "       |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |  \\- cz\n"
    "       |        |        |     \\- cvut\n"
    "       |        |        |        +- fit\n"
    "       |        |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |        |  \\- fel\n"
    "       |        |        |        |     +- www A 147.32.80.2\n"
    "       |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |        \\- fel\n"
    "       |        |        |           +- www A 147.32.80.2\n"
    "       |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        \\- fel\n"
    "       |        |           +- www A 147.32.80.2\n"
    "       |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        \\- fel\n"
    "       |           +- www A 147.32.80.2\n"
    "       |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  assert ( dynamic_cast<CZone &> ( z20 . search ( "fit.cvut.cz.fit.cvut.cz" ) [0] ) . del ( CZone ( "fel" ) ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " \\- cz\n"
    "    \\- cvut\n"
    "       +- fit\n"
    "       |  +- progtest A 147.32.232.142\n"
    "       |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |  +- courses A 147.32.232.158\n"
    "       |  +- courses A 147.32.232.160\n"
    "       |  +- courses A 147.32.232.159\n"
    "       |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |  +- fel\n"
    "       |  |  +- www A 147.32.80.2\n"
    "       |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |  \\- cz\n"
    "       |     \\- cvut\n"
    "       |        +- fit\n"
    "       |        |  +- progtest A 147.32.232.142\n"
    "       |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |  +- courses A 147.32.232.158\n"
    "       |        |  +- courses A 147.32.232.160\n"
    "       |        |  +- courses A 147.32.232.159\n"
    "       |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |  \\- cz\n"
    "       |        |     \\- cvut\n"
    "       |        |        +- fit\n"
    "       |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |  +- fel\n"
    "       |        |        |  |  +- www A 147.32.80.2\n"
    "       |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |  \\- cz\n"
    "       |        |        |     \\- cvut\n"
    "       |        |        |        +- fit\n"
    "       |        |        |        |  +- progtest A 147.32.232.142\n"
    "       |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "       |        |        |        |  +- courses A 147.32.232.158\n"
    "       |        |        |        |  +- courses A 147.32.232.160\n"
    "       |        |        |        |  +- courses A 147.32.232.159\n"
    "       |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    "       |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    "       |        |        |        |  \\- fel\n"
    "       |        |        |        |     +- www A 147.32.80.2\n"
    "       |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        |        \\- fel\n"
    "       |        |        |           +- www A 147.32.80.2\n"
    "       |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        |        \\- fel\n"
    "       |        |           +- www A 147.32.80.2\n"
    "       |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       |        \\- fel\n"
    "       |           +- www A 147.32.80.2\n"
    "       |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    "       \\- fel\n"
    "          +- www A 147.32.80.2\n"
    "          \\- www AAAA 1:2:3:4:5:6:7:8\n" );
  CZone z25 ( z20 );
z22 = z20;
  assert ( z20 . add ( CZone ( "sk" ) ) == true );
  assert ( z25 . add ( CZone ( "au" ) ) == true );
  assert ( z22 . add ( CZone ( "de" ) ) == true );
  oss . str ( "" );
  oss << z20;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " +- cz\n"
    " |  \\- cvut\n"
    " |     +- fit\n"
    " |     |  +- progtest A 147.32.232.142\n"
    " |     |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |  +- courses A 147.32.232.158\n"
    " |     |  +- courses A 147.32.232.160\n"
    " |     |  +- courses A 147.32.232.159\n"
    " |     |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |  +- fel\n"
    " |     |  |  +- www A 147.32.80.2\n"
    " |     |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |  \\- cz\n"
    " |     |     \\- cvut\n"
    " |     |        +- fit\n"
    " |     |        |  +- progtest A 147.32.232.142\n"
    " |     |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |  +- courses A 147.32.232.158\n"
    " |     |        |  +- courses A 147.32.232.160\n"
    " |     |        |  +- courses A 147.32.232.159\n"
    " |     |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |  \\- cz\n"
    " |     |        |     \\- cvut\n"
    " |     |        |        +- fit\n"
    " |     |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |  +- fel\n"
    " |     |        |        |  |  +- www A 147.32.80.2\n"
    " |     |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |  \\- cz\n"
    " |     |        |        |     \\- cvut\n"
    " |     |        |        |        +- fit\n"
    " |     |        |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |        |  \\- fel\n"
    " |     |        |        |        |     +- www A 147.32.80.2\n"
    " |     |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |        \\- fel\n"
    " |     |        |        |           +- www A 147.32.80.2\n"
    " |     |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        \\- fel\n"
    " |     |        |           +- www A 147.32.80.2\n"
    " |     |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        \\- fel\n"
    " |     |           +- www A 147.32.80.2\n"
    " |     |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     \\- fel\n"
    " |        +- www A 147.32.80.2\n"
    " |        \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " \\- sk\n" );
  oss . str ( "" );
  oss << z22;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " +- cz\n"
    " |  \\- cvut\n"
    " |     +- fit\n"
    " |     |  +- progtest A 147.32.232.142\n"
    " |     |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |  +- courses A 147.32.232.158\n"
    " |     |  +- courses A 147.32.232.160\n"
    " |     |  +- courses A 147.32.232.159\n"
    " |     |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |  +- fel\n"
    " |     |  |  +- www A 147.32.80.2\n"
    " |     |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |  \\- cz\n"
    " |     |     \\- cvut\n"
    " |     |        +- fit\n"
    " |     |        |  +- progtest A 147.32.232.142\n"
    " |     |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |  +- courses A 147.32.232.158\n"
    " |     |        |  +- courses A 147.32.232.160\n"
    " |     |        |  +- courses A 147.32.232.159\n"
    " |     |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |  \\- cz\n"
    " |     |        |     \\- cvut\n"
    " |     |        |        +- fit\n"
    " |     |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |  +- fel\n"
    " |     |        |        |  |  +- www A 147.32.80.2\n"
    " |     |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |  \\- cz\n"
    " |     |        |        |     \\- cvut\n"
    " |     |        |        |        +- fit\n"
    " |     |        |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |        |  \\- fel\n"
    " |     |        |        |        |     +- www A 147.32.80.2\n"
    " |     |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |        \\- fel\n"
    " |     |        |        |           +- www A 147.32.80.2\n"
    " |     |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        \\- fel\n"
    " |     |        |           +- www A 147.32.80.2\n"
    " |     |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        \\- fel\n"
    " |     |           +- www A 147.32.80.2\n"
    " |     |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     \\- fel\n"
    " |        +- www A 147.32.80.2\n"
    " |        \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " \\- de\n" );
  oss . str ( "" );
  oss << z25;
  assert ( oss . str () == 
    "<ROOT ZONE>\n"
    " +- cz\n"
    " |  \\- cvut\n"
    " |     +- fit\n"
    " |     |  +- progtest A 147.32.232.142\n"
    " |     |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |  +- courses A 147.32.232.158\n"
    " |     |  +- courses A 147.32.232.160\n"
    " |     |  +- courses A 147.32.232.159\n"
    " |     |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |  +- fel\n"
    " |     |  |  +- www A 147.32.80.2\n"
    " |     |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |  \\- cz\n"
    " |     |     \\- cvut\n"
    " |     |        +- fit\n"
    " |     |        |  +- progtest A 147.32.232.142\n"
    " |     |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |  +- courses A 147.32.232.158\n"
    " |     |        |  +- courses A 147.32.232.160\n"
    " |     |        |  +- courses A 147.32.232.159\n"
    " |     |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |  \\- cz\n"
    " |     |        |     \\- cvut\n"
    " |     |        |        +- fit\n"
    " |     |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |  +- fel\n"
    " |     |        |        |  |  +- www A 147.32.80.2\n"
    " |     |        |        |  |  \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |  \\- cz\n"
    " |     |        |        |     \\- cvut\n"
    " |     |        |        |        +- fit\n"
    " |     |        |        |        |  +- progtest A 147.32.232.142\n"
    " |     |        |        |        |  +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " |     |        |        |        |  +- courses A 147.32.232.158\n"
    " |     |        |        |        |  +- courses A 147.32.232.160\n"
    " |     |        |        |        |  +- courses A 147.32.232.159\n"
    " |     |        |        |        |  +- pririz CNAME sto.fit.cvut.cz.\n"
    " |     |        |        |        |  +- courses SPF ip4:147.32.232.128/25, ip4:147.32.232.64/26\n"
    " |     |        |        |        |  \\- fel\n"
    " |     |        |        |        |     +- www A 147.32.80.2\n"
    " |     |        |        |        |     \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        |        \\- fel\n"
    " |     |        |        |           +- www A 147.32.80.2\n"
    " |     |        |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        |        \\- fel\n"
    " |     |        |           +- www A 147.32.80.2\n"
    " |     |        |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     |        \\- fel\n"
    " |     |           +- www A 147.32.80.2\n"
    " |     |           \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " |     \\- fel\n"
    " |        +- www A 147.32.80.2\n"
    " |        \\- www AAAA 1:2:3:4:5:6:7:8\n"
    " \\- au\n" );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
