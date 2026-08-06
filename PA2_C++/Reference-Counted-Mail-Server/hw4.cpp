#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#include <compare>
#include <iterator>
#endif /* __PROGTEST */

#ifndef __PROGTEST__
class CTimeStamp
{
  public:
                   CTimeStamp                              ( int               year,
                                                             int               month,
                                                             int               day,
                                                             int               hour,
                                                             int               minute,
                                                             int               sec ) :
                                                             y(year), mon(month), d(day),
                                                             h(hour), min(minute), s (sec) {}
    std::strong_ordering operator <=>                      ( const CTimeStamp & x ) const {
      if (auto cmp = y   <=> x.y;   cmp != 0) return cmp;
      if (auto cmp = mon <=> x.mon; cmp != 0) return cmp;
      if (auto cmp = d   <=> x.d;   cmp != 0) return cmp;
      if (auto cmp = h   <=> x.h;   cmp != 0) return cmp;
      if (auto cmp = min <=> x.min; cmp != 0) return cmp;
      return s <=> x.s;
    }
    bool                 operator ==                       ( const CTimeStamp & x ) const = default;
    friend std::ostream & operator <<                      ( std::ostream     & os,
                                                             const CTimeStamp & x ) {
      os << std::setfill(' ') << x.y << '-';
      os << std::setfill('0') << std::setw(2) << x.mon << '-';
      os << std::setw(2) << x.d << ' ';
      os << std::setw(2) << x.h << ':';
      os << std::setw(2) << x.min << ':';
      os << std::setw(2) << x.s << std::setfill(' ');
      return os;
    }
  private: 
      int y, mon, d, h, min, s;
};
//=================================================================================================
class CMailBody
{
  public:
                   CMailBody                               ( int               size,
                                                             const char        data[] ) : 
                                                             m_Size(size), m_Data (size ? new char [size] : nullptr) 
                                                             {for (int i = 0; i < size; i++) m_Data[i] = data[i];}
     // copy cons/op=/destructor is correctly implemented in the testing environment
                   ~CMailBody() {delete[] m_Data; m_Data = nullptr; m_Size = 0;}
                   CMailBody(const CMailBody & body) {
                    reset();
                    m_copy(body);
                   }
                   CMailBody& operator = (CMailBody body) {
                    std::swap (m_Size, body.m_Size);
                    std::swap (m_Data, body.m_Data);
                    return *this;
                   }
    friend std::ostream & operator <<                      ( std::ostream    & os,
                                                             const CMailBody & x )
    {
      return os << "mail body: " << x . m_Size << " B";
    }
  private:
    int            m_Size;
    char         * m_Data;
    void m_copy (const CMailBody& body) {
      m_Size = body.m_Size;
      m_Data = m_Size ? new char [m_Size] : nullptr;
      for (int i = 0; i < m_Size; i++) m_Data[i] = body.m_Data[i];
    }
    void reset () {
      m_Size = 0;
      m_Data = nullptr;
    }
};
//=================================================================================================
class CAttach
{
  public:
                   CAttach                                 ( int               x )
      : m_X (x)
    {
    }
    void           addRef                                  ()
    { 
      m_RefCnt ++; 
    }
    void           release                                 ()
    { 
      if ( !--m_RefCnt ) 
        delete this; 
    }
  private:
    int            m_X;
    int            m_RefCnt = 1;
                   CAttach                                 ( const CAttach   & x );
    CAttach      & operator =                              ( const CAttach   & x );
                   ~CAttach                                () = default;
    friend std::ostream & operator <<                      ( std::ostream    & os,
                                                             const CAttach   & x )
    {
      return os << "attachment: " << x . m_X << " B";
    }
};
//=================================================================================================
#endif /* __PROGTEST__, DO NOT remove */


class CMail
{
  public:
                        CMail                              ( const CTimeStamp & timeStamp,
                                                             const std::string& from,
                                                             const CMailBody  & body,
                                                             CAttach          * attach = nullptr) :
                                                             time_(timeStamp), from_(from), 
                                                             body_ (body), attach_ (attach) {if (attach_) attach_->addRef();}
                        ~CMail() {if (attach_) attach_->release();}
                        CMail (const CMail& mail) : time_(mail.time_), from_(mail.from_), 
                                                    body_(mail.body_), attach_(mail.attach_)
                                                    {if (attach_) attach_->addRef(); }
                        CMail& operator = (const CMail& mail) {
                          if (this == &mail) return *this;

                          time_ = mail.time_;
                          from_ = mail.from_;
                          body_ = mail.body_;

                          if (attach_) attach_->release();
                          attach_ = mail.attach_;
                          if (attach_) attach_->addRef();

                          return *this;
                        }
    const std::string   & from                             () const {return from_;}
    const CMailBody     & body                             () const {return body_;}
    const CTimeStamp    & timeStamp                        () const {return time_;}
    CAttach             * attachment                       () const {if (attach_) attach_->addRef(); return attach_;}
    friend std::ostream & operator <<                      ( std::ostream     & os,
                                                             const CMail      & x ) {
      os << x.time_ << ' ';
      os << x.from_ << ' ';
      os << x.body_;
      if (x.attach_) os << " + " << *x.attach_; 
      return os;
    }
  private:
    // todo
    CTimeStamp time_;
    std::string from_;
    CMailBody body_;
    CAttach * attach_;
};
//=================================================================================================
class CMailBox
{
  public:
                          CMailBox                         () {folders.insert(std::make_pair("inbox", std::list<CMail>()));};
    bool                  delivery                         ( const CMail      & mail ) {
      // Find the element strictly > mail => even if equal, the latest mail end at the latest
      auto& inbox = folders["inbox"];
      auto it = std::upper_bound (inbox.begin(), inbox.end(), mail, 
                                  [](const CMail& a, const CMail& b) {return a.timeStamp() < b.timeStamp();});

      // Insert the mail to folders
      inbox.insert(it, mail);

      // Insert to address index for fast lookup
      addresses.insert(std::make_pair(mail.timeStamp(), mail.from()));
      return true;
    }
    bool                  newFolder                        ( const std::string& folderName ) {
      return folders.insert(std::make_pair(folderName, std::list<CMail>())).second;
    }
    bool                  moveMail                         ( const std::string& fromFolder,
                                                             const std::string& toFolder ) {
      // If 2 folders are the same
      if (fromFolder == toFolder) return true;

      // Get iterators from each folder
      auto it_fr = folders.find(fromFolder);
      auto it_to = folders.find(toFolder);
      
      // Check exist
      if (it_fr == folders.end() || it_to == folders.end()) return false;
      
      // O(1) shallow copy
      it_to->second.merge(it_fr->second, [] (const CMail& a, const CMail& b){return a.timeStamp() < b.timeStamp();});
      return true;
    }
    std::list<CMail>      listMail                         ( const std::string& folderName,
                                                             const CTimeStamp & from,
                                                             const CTimeStamp & to ) const {
      // If folder not exist, return empty list
      auto it = folders.find(folderName);
      if (it == folders.end()) return {};      

      // If yes, iterate thru list and add to new list
      const auto& m_list = it->second;
      std::list<CMail> res;
      for (auto it = std::lower_bound(m_list.begin(), m_list.end(), from, 
                                      [](const CMail& mail, const CTimeStamp& time) 
                                      {return mail.timeStamp() < time;}); it != m_list.end(); it++) {
        if (it->timeStamp() > to ) break;
        res.push_back(*it);
      }
      return res;
    }
    std::set<std::string> listAddr                         ( const CTimeStamp & from,
                                                             const CTimeStamp & to ) const {
      // Iterators on multimap
      auto begin = addresses.lower_bound(from);
      auto end = addresses.upper_bound(to);

      // Copy within interval & return
      std::set<std::string> a_list;
      for (auto it = begin; it != end; it++) a_list.insert(it->second);
      return a_list;
    }
  private:
    // todo
    std::map <std::string, std::list<CMail>> folders;
    std::multimap <CTimeStamp, std::string> addresses;

};
//=================================================================================================
#ifndef __PROGTEST__
static std::string showMail ( const std::list<CMail> & l )
{
  std::ostringstream oss;
  for ( const auto & x : l )
    oss << x << std::endl;
  return oss . str ();
}
static std::string showUsers ( const std::set<std::string> & s )
{
  std::ostringstream oss;
  for ( const auto & x : s )
    oss << x << std::endl;
  return oss . str ();
}
int main ()
{
  CAttach * att;
  std::ostringstream oss;

  att = new CAttach ( 100 );
  CMail testMail ( CTimeStamp ( 2026, 1, 2, 12, 5, 0 ), "test@domain.cz", CMailBody ( 10, "test, test" ), att );
  att -> release ();
  assert ( testMail . timeStamp () == CTimeStamp ( 2026, 1, 2, 12, 5, 0 ) );
  assert ( testMail . from () == "test@domain.cz" );
  att = testMail . attachment ();
  oss << *att;
  att -> release ();
  assert ( oss . str () == "attachment: 100 B" );
  assert ( showMail ( { testMail } ) == "2026-01-02 12:05:00 test@domain.cz mail body: 10 B + attachment: 100 B\n" );

  CMailBox m0;
  assert ( m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 24, 13 ), "user1@fit.cvut.cz", CMailBody ( 14, "mail content 1" ), nullptr ) ) );
  assert ( m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 26, 23 ), "user2@fit.cvut.cz", CMailBody ( 22, "some different content" ), nullptr ) ) );
  att = new CAttach ( 200 );
  assert ( m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 11, 23, 43 ), "boss1@fit.cvut.cz", CMailBody ( 14, "urgent message" ), att ) ) );
  assert ( m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 18, 52, 27 ), "user1@fit.cvut.cz", CMailBody ( 14, "mail content 2" ), att ) ) );
  att -> release ();
  att = new CAttach ( 97 );
  assert ( m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 16, 12, 48 ), "boss1@fit.cvut.cz", CMailBody ( 24, "even more urgent message" ), att ) ) );
  att -> release ();
  assert ( showMail ( m0 . listMail ( "inbox",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n"
                        "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
                        "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n"
                        "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n" );
  assert ( showMail ( m0 . listMail ( "inbox",
                      CTimeStamp ( 2024, 3, 31, 15, 26, 23 ),
                      CTimeStamp ( 2024, 3, 31, 16, 12, 48 ) ) ) == "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
                        "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n" );
  assert ( showUsers ( m0 . listAddr ( CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                       CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "boss1@fit.cvut.cz\n"
                        "user1@fit.cvut.cz\n"
                        "user2@fit.cvut.cz\n" );
  assert ( showUsers ( m0 . listAddr ( CTimeStamp ( 2024, 3, 31, 15, 26, 23 ),
                       CTimeStamp ( 2024, 3, 31, 16, 12, 48 ) ) ) == "boss1@fit.cvut.cz\n"
                        "user2@fit.cvut.cz\n" );

  CMailBox m1;
  assert ( m1 . newFolder ( "work" ) );
  assert ( m1 . newFolder ( "spam" ) );
  assert ( !m1 . newFolder ( "spam" ) );
  assert ( m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 24, 13 ), "user1@fit.cvut.cz", CMailBody ( 14, "mail content 1" ), nullptr ) ) );
  att = new CAttach ( 500 );
  assert ( m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 26, 23 ), "user2@fit.cvut.cz", CMailBody ( 22, "some different content" ), att ) ) );
  att -> release ();
  assert ( m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 11, 23, 43 ), "boss1@fit.cvut.cz", CMailBody ( 14, "urgent message" ), nullptr ) ) );
  att = new CAttach ( 468 );
  assert ( m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 18, 52, 27 ), "user1@fit.cvut.cz", CMailBody ( 14, "mail content 2" ), att ) ) );
  att -> release ();
  assert ( m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 16, 12, 48 ), "boss1@fit.cvut.cz", CMailBody ( 24, "even more urgent message" ), nullptr ) ) );
  assert ( showMail ( m1 . listMail ( "inbox",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
                        "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
                        "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n" );
  assert ( showMail ( m1 . listMail ( "work",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "" );
  assert ( m1 . moveMail ( "inbox", "work" ) );
  assert ( showMail ( m1 . listMail ( "inbox",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "" );
  assert ( showMail ( m1 . listMail ( "work",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
                        "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
                        "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n" );
  assert ( m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 19, 24, 13 ), "user2@fit.cvut.cz", CMailBody ( 14, "mail content 4" ), nullptr ) ) );
  att = new CAttach ( 234 );
  assert ( m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 13, 26, 23 ), "user3@fit.cvut.cz", CMailBody ( 9, "complains" ), att ) ) );
  att -> release ();
  assert ( showMail ( m1 . listMail ( "inbox",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
                        "2024-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n" );
  assert ( showMail ( m1 . listMail ( "work",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
                        "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
                        "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n" );
  assert ( m1 . moveMail ( "inbox", "work" ) );
  assert ( showMail ( m1 . listMail ( "inbox",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "" );
  assert ( showMail ( m1 . listMail ( "work",
                      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
                      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
                        "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
                        "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
                        "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
                        "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n"
                        "2024-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n" );

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
