#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <deque>
#include <algorithm>
#include <memory>
#include <compare>
#include <stdexcept>
#include <optional>
using namespace std::literals;
#endif /* __PROGTEST__ */

struct Time {
  int y, mon, d, h, min;
  Time () : y(0), mon(1), d(1), h(0), min(0) {}
  Time(int y1, int mon1, int d1, int h1, int min1) : y(y1), mon(mon1), d(d1), h(h1), min(min1) {}
  Time (const Time& time) :  y(time.y), mon(time.mon), d(time.d), h(time.h), min(time.min) {}
  ~Time() = default;
  Time& operator = (const Time& time) {
    if (this == &time) return *this;
    y = time.y;
    mon = time.mon;
    d = time.d;
    h = time.h;
    min = time.min;
    return *this;
  }
  friend std::ostream& operator << (std::ostream& out, const Time& time) ;
};

std::ostream& operator << (std::ostream& out, const Time& time) {
  out << time.y << "-" << time.mon << "-" << time.d << " " << time.h << ":" << time.min << '\n';
  return out;
}

bool leap_year (int year) {
  int y = year;
  if (y % 400 == 0) return true;
  if (y % 100 == 0) return false;
  if (y % 4 == 0) return true;
  return false;
}

long long max_day (int year, int mon) {
  long long arr [12] = {31, 28 + leap_year(year), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return arr[mon-1];
}

bool valid_time (const Time& time) {
  return (time.mon >= 1 && time.mon <= 12) 
      && (time.d >= 1 && time.d <= static_cast<int> (max_day (time.y, time.mon))) 
      && (time.h >= 0 && time.h<= 23) 
      && (time.min >= 0 && time.min <= 59); 
}

long long time_to_min (const Time& time) {
  // Check valid time
  if (!valid_time(time)) {
    throw std::runtime_error("Invalid time.");
  }
  
  // Calculate days from previous years
  long long prev_years = time.y - 1;
  long long total_days = prev_years * 365 + prev_years / 4 - prev_years / 100 + prev_years / 400;
  
  // Calculate days from current year
  static const int total_days_bef_month[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  total_days += total_days_bef_month[time.mon - 1];
  if (time.mon > 2 && leap_year(time.y)) total_days ++; // +1 day for leap_year (past feb)
  total_days += time.d - 1;

  // Convert to min
  return time.min + time.h * 60 + total_days * 24 * 60; 
}

class CAuditFilter
{
  std::string zone;
  std::optional <long long> start, end;
  public:
    CAuditFilter(std::string zone_name) : zone(zone_name) {}
    CAuditFilter& notBefore (int y, int mon, int d, int h, int mi) {
      start = time_to_min(Time(y, mon, d, h, mi));
      return *this;
    }
    CAuditFilter& notAfter (int y, int mon, int d, int h, int mi) {
      end = time_to_min(Time(y, mon, d, h, mi));
      return *this;
    }
    std::string const zone_() const {return zone;}
    std::optional <long long> const start_() const {return start;}
    std::optional <long long> const end_() const {return end;}
};

class CVisitorLog
{
  public:
    CVisitorLog(std::map<std::string, std::map<std::string, int>> * dist) : log_dist(dist) {}
    void processEntered (std::map<std::string, std::pair<std::string, Time>>& entered) {
      //std::cout << "--------------------- Debug Entered Log -----------------\n";
      for (const auto& [name, data] : entered) {
        Trip trip;
        trip.zoneA = data.first;
        trip.enter = data.second;
        trip.is_inside = true;
        //std::cout << name << ": " << trip.zoneA << " at " << trip.enter << '\n';  
        addTrip(name, trip);
      }
      //std::cout << "---------------- Finish Debugging Entered Log --------------\n";
    }
    void updateLog (std::map<std::string, std::pair<std::string, Time>>& entered, 
                    const std::string& name, const std::string& zone, const Time& time) {
      // Update people who completed the trip
      auto it = entered.find(name);
      if (it == entered.end()) entered[name] = {zone, time}; // If not, entry
      else { // If yes, exit => Full trip => Log
        Trip trip;
        trip.zoneA = entered[name].first;
        trip.zoneB = zone;
        trip.enter = entered[name].second;
        trip.exit = time;
        addTrip(name, trip);
        entered.erase(it); // Remove from current entered list, since they have exited
      }
    }
    std::set<std::string> search (const CAuditFilter& filter) const
    { 
      // Set up
      std::set <std::string> suspects;
      std::string zone = filter.zone_();

      // Look through each person
      for (const auto& [name, trips] : logs) {

        // Look through each of their trips
        //std::cout << " ---------- Search Entered Debug ------------\n";
        for (const auto& trip : trips) {

          // Check if still inside & path exist
          int no_path = std::numeric_limits<int>::max();
          
          //if (trip.is_inside) std::cout << name << " " << trip.zoneA << "->" << zone << " "
          //                              << get_dist(trip.zoneA, zone) << " "
          //                              << *filter.end_() << time_to_min(trip.enter) + get_dist(trip.zoneA, zone) << " "
          //                              << " <= " << *filter.end_() << '\n';
          
          if (trip.is_inside == true 
            && get_dist(trip.zoneA, zone) != no_path 
            && (!filter.end_() || (filter.end_() && time_to_min(trip.enter) + get_dist(trip.zoneA, zone) <= filter.end_()))) {
            suspects.insert(name);
            continue;
          }

          // Check if a path exist
          int in = get_dist(trip.zoneA, zone);
          int out = get_dist(zone, trip.zoneB);
          if (in == no_path || out == no_path) continue;

          // Check if enough time to reach zone
          auto start = filter.start_();
          auto end = filter.end_();
          long long arrive = time_to_min(trip.enter) + in;
          long long depart = time_to_min(trip.exit) - out;
          if (arrive > depart) continue;

          // Check with constraint
          if (start && depart < start) continue;
          if (end && arrive > end) continue;
          
          // Add
          suspects.insert(name);
          break; 
        }
        //std::cout << " ---------- Finish Search Entered Debug ------------\n";
      }

      // Debug
      //searchDebug(suspects);

      // Return
      return suspects;
    }
    friend void processLogDebug(const CVisitorLog& visitorLog);
  private:
    struct Trip {
      std::string zoneA, zoneB;
      Time enter, exit;
      bool is_inside = false;
    };
    std::map <std::string, std::vector<Trip>> logs; // People names, and their logs (trips)
    std::map<std::string, std::map<std::string, int>> * log_dist; // Pointer to the distance map calculated by CMilBase
    void addTrip (const std::string& name, const Trip& trip) {
      logs[name].push_back(trip);
    }
    int get_dist (const std::string& zoneA, const std::string& zoneB) const {
      if (zoneA == zoneB) return 0;
      const auto& ref_dist = *log_dist;
      int error = std::numeric_limits<int>::max();

      auto it1 = ref_dist.find(zoneA);
      if (it1 == ref_dist.end()) return error;

      auto it2 = it1->second.find(zoneB);
      if (it2 == it1->second.end()) return error;

      return it2->second;
    }
    void searchDebug(std::set<std::string>& tmp) const {
      std::cout << " --------------- Start Debugging search ---------------- \n";
      std::cout << "All suspects\n";
      for (const auto& stuff : tmp) {
        std::cout << stuff << '\n';
      }
      std::cout << " ---------------- End of Debugging of search -------------\n";
    }
};

class CMilBase
{
  public:
    CMilBase         () { 
      clear_base();
    }
    void readBase    ( const std::string & baseFilename )
    {
      // Open file
      std::ifstream in(baseFilename);
      if (!in) throw std::runtime_error("Cannot open file: " + baseFilename);

      // Clear old file (if any), and set up default distances
      clear_base();
      
      // Read file and insert real zones
      int dummy_code = 0;
      std::string line;
      while (std::getline(in, line)) {
        if (line.empty()) throw std::runtime_error("Wrong format");
        std::stringstream ss(line);
        std::string zoneA, zoneB;
        int distance;
        char sp1, sp2;
        if (!(ss >> zoneA)
            ||!ss.get(sp1) || sp1 != ' ' || std::isspace(ss.peek())
            ||!(ss >> zoneB)
            ||!ss.get(sp2) || sp2 != ' ' || std::isspace(ss.peek())
            ||!(ss >> distance) || distance < 1 || distance > 9) throw std::runtime_error("Wrong format");
        zones.insert(zoneA);
        zones.insert(zoneB);
        dist[zoneA][zoneB] = distance;
        dist[zoneB][zoneA] = distance;
      
        // Insert dummy zones
        std::string current = zoneA;
        for (int i = 0; i < distance - 1; i++) {
          std::string dummy_zone = "dummy_" + std::to_string(dummy_code++);
          adj_zones[current].push_back(dummy_zone);
          adj_zones[dummy_zone].push_back(current); // Bidirectional connection
          current = dummy_zone;
        }
        adj_zones[current].push_back(zoneB);
        adj_zones[zoneB].push_back(current); // Bidirectional connection
      }
      
      // Set up default
      set_base();

      // Check if file is read correctly
      if (!in.eof() && in.fail()) throw std::runtime_error("File read error: " + baseFilename);

      // Run BFS to calculate distances between ALL real zones pairs
      for (const std::string& zone : zones) BFS_all_dists(zone);

      // Debug
      //readBaseDebug();
    }

    CVisitorLog processLog  ( const std::string & logFilename )
    {
      // Clear last log
      raw_log.clear();

      // Open the file in binary
      std::ifstream ifs (logFilename, std::ios::binary);
      if (!ifs) throw std::runtime_error("Log file error");

      // Ready an instance of CVisitorLog
      CVisitorLog result (&dist);

      // Ready a map of people and the zones they entered with time
      std::map<std::string, std::pair<std::string, Time>> entered;

      // While not empty
      char header[4];
      while (ifs.read(header, 4)) {
        // Header format
        if (ifs.gcount() < 4) throw std::runtime_error("Truncated header");

        // Parse accordingly to format
        std::string format(header, 4);
        if (format == "TEXT") {
          //std::cout << "Text\n";
          if (!parseText(ifs)) throw std::runtime_error("Wrong text format");;
        }
        else if (format == "IIII") {
          //std::cout << "Little Endian\n";
          if (!parseBinary(ifs, true)) throw std::runtime_error("Wrong little endian format");
        } // I = 0x49 => Little Endian
        else if (format == "MMMM") {
          //std::cout << "Big Endian\n";
          if (!parseBinary(ifs, false)) throw std::runtime_error("Wrong big endian format");
        } // M = 4x4D => Big Endian
        else throw std::runtime_error ("Log header error");
      }
      
      // Check if all bytes are read
      if (!ifs.eof() || ifs.gcount() != 0) 
        throw std::runtime_error("Unexpected trailing data in log file");

      // Sort and process all raw logs so far
      std::sort(raw_log.begin(), raw_log.end());
      for (const auto& tmp : raw_log) result.updateLog(entered, tmp.name, tmp.zone, tmp.time);

      // Process people who have entered and havent left
      result.processEntered(entered);

      // Debug
      //processLogDebug(result);

      // Return
      return result;
    }
  private:
    std::set<std::string> zones; // All real zones
    std::map<std::string, std::vector<std::string>> adj_zones; // Zones adj to current zone, including dummy zones
    std::map<std::string, std::map<std::string, int>> dist; // Distances between real zones
    struct RawLog {
      std::string name ,zone;
      Time time;
      bool operator < (const RawLog& raw) {
        return time_to_min(time) < time_to_min(raw.time);
      }      
    };
    std::vector<RawLog> raw_log;
    void readBaseDebug() {
      std::cout << " ---------------- Debugging readBase --------------- \n";
      std::cout << "All real zones:\n";
      for (const auto& zone : zones) std::cout << zone << std::endl;
      std::cout << "All adj zones:\n";
      for (const auto& [zone, nexts] : adj_zones) {
        for (const auto& adj_zone : nexts) {
          std::cout << zone << "->" << adj_zone << std::endl;
        }
      }
      std::cout << "All real distances:\n";
      for (const auto& [zone, reals] : dist) {
        for (const auto& d : reals) {
          std::cout << zone << "->" << d.first << ":" << d.second << std::endl;  
        }
      }
      std::cout << " --------------- End of Debugging of readBase ----------- \n";
    }
    void set_base () {
      for (const auto& src: zones) {
        for (const auto& adj : zones) {
          if (src == adj) dist[src][adj] = 0;
        }
      }
    }
    void clear_base () {
      zones.clear();
      adj_zones.clear();
      dist.clear();
      raw_log.clear();
    }
    bool parseText (std::ifstream& ifs) {
      // Extract zone and number of entries
      std::string zone; 
      unsigned int entries;
      char sp1, sp2, nl;
      if (!ifs.get(sp1) || sp1 != ' ' || std::isspace(ifs.peek())) return false;
      if (!(ifs >> zone)) return false;
      if (!ifs.get(sp2) || sp2 != ' ') return false;
      if (ifs.peek() == '-' || !(ifs >> entries)) return false;
      if (!ifs.get(nl) || nl != '\n') 
        return false;

      // Extract all logs
      //std::cout << "----------------------- Debugging parseText --------------\n";
      for (unsigned int i = 0; i < entries; i++) {
        // Extract lines
        std::string line;
        if (!std::getline(ifs, line) || line.empty()) return false;

        // Extract time
        std::stringstream ss(line);
        int y, mon, d, h, min;
        char c1, c2, c3;
        if (!(ss >> y >> c1 >> mon >> c2 >> d)
            || c1 != '-' || c2 != '-') return false;
        char sp;
        if (!ss.get(sp) || sp != ' ' || std::isspace(ss.peek())) return false;
        if (!(ss >> h >> c3 >> min) || c3 != ':') return false;
        Time time (y, mon, d, h, min);
        if (!valid_time(time)) return false;

        // Extract name
        std::string name;
        if (!ss.get(sp) || sp != ' ') 
            return false;
        std::getline(ss, name);
        if (name.empty()) return false;

        // Update CVisitorLog
        //parseTextDebug(name, zone, time);
        RawLog tmp;
        tmp.name = name;
        tmp.zone = zone;
        tmp.time = time;
        raw_log.push_back(tmp);
      }
      //std::cout << "----------------------- Complete debugging parseText --------------\n";
      return true;
    }
    uint32_t readBinary32 (std::ifstream& ifs, bool is_little_endian) {
      unsigned char b[4];
      if (!ifs.read(reinterpret_cast<char*> (b), 4)) throw std::runtime_error("Wrong file format");
      if (ifs.gcount() < 4) throw std::runtime_error("Unexpected EOF in binary data");
      if (is_little_endian) return static_cast<uint32_t>(b[3]) << 24 | static_cast<uint32_t>(b[2]) << 16 | static_cast<uint32_t>(b[1]) << 8 | static_cast<uint32_t>(b[0]);
      else return static_cast<uint32_t>( b[0]) << 24 | static_cast<uint32_t>(b[1]) << 16 | static_cast<uint32_t>(b[2]) << 8 | static_cast<uint32_t>(b[3]); // default big endian
    }
    uint16_t readBinary16(std::ifstream& ifs, bool is_little_endian) {
    unsigned char b[2];
      if (!ifs.read(reinterpret_cast<char*>(b), 2)) throw std::runtime_error("Binary read error");
      if (ifs.gcount() < 2) throw std::runtime_error("Unexpected EOF in binary data");
      if (is_little_endian) return static_cast<uint16_t>(b[1]) << 8 | static_cast<uint16_t>(b[0]);
      else return static_cast<uint16_t>(b[0]) << 8 | static_cast<uint16_t>(b[1]); // default big endian
    }
    Time readTime (uint32_t time) {
      int min = time & 0x3F;
      int h = (time >> 6) & 0x1F;
      int d = (time >> 11) & 0x1F;
      int mon = (time >> 16) & 0x0F;
      int y = (time >> 20) & 0xFFF;
      return Time(y,mon,d,h,min); 
    }
    bool parseBinary (std::ifstream& ifs, bool is_little_endian) {
      //std::cout << " ------------------------ Start debugging parseBinary ------------\n";
      // Get zone name length
      uint16_t zone_len = readBinary16(ifs, is_little_endian);
      if (zone_len > 1024 || zone_len == 0) return false;

    
      // Read zone name
      std::string zone(zone_len, '\0');
      if (zone_len > 0) { 
          if (!ifs.read(&zone[0], zone_len) || 
              static_cast<uint16_t>(ifs.gcount()) < zone_len)
              return false;
      }
      for (char c : zone) {
        if (std::isspace(c) || !std::isprint(static_cast<unsigned char>(c))) {
          return false;
        }
      }

      // Read num entries
      uint32_t entries = readBinary32(ifs, is_little_endian);
      if (entries > 1024) return false;

      // Read all logs
      for (uint32_t i = 0; i < entries; i++) {
        // Extract time
        uint32_t binary_time = readBinary32(ifs, is_little_endian);
        Time time = readTime(binary_time);
        if (!valid_time(time)) return false;

        // Extract name length
        uint16_t name_len = readBinary16(ifs, is_little_endian);

        // Extract name
        std::string name(name_len, '\0');
        if (name_len > 0) {
            if (!ifs.read(&name[0], name_len) || 
                static_cast<uint16_t>(ifs.gcount()) < name_len)
                return false;
        }
        for (char c : name) {
            if (!std::isprint((unsigned char)c) || c == '\n' || c == '\r') 
                return false;
        }

        // Update log
        //parseBinaryDebug(name, zone, time);
        RawLog tmp;
        tmp.name = name;
        tmp.zone = zone;
        tmp.time = time;
        raw_log.push_back(tmp);
      }
      //std::cout << " ------------------------ Finish debugging parseBinary ------------\n";
      return true;
    }
    void BFS_all_dists(const std::string& zone) {
      // Set up
      std::unordered_set <std::string> visited = {zone};
      std::deque<std::pair <std::string, int>> queue = {{zone, 0}};

      // Calculate distance to any adj real zones
      while (!queue.empty()) {
        // Look at the closest node in queue
        auto current = queue.front();
        queue.pop_front();

        // If node is real zone, collect the distance
        if (zones.contains(current.first)) dist[zone][current.first] = current.second;

        // Then, look for its adj nodes
        for (const std::string& adj : adj_zones.at(current.first)) {
          if (visited.count(adj)) continue; // If node visited, skip
          queue.push_back({adj, current.second + 1}); // If not, put in queue, make it distance +1 from current
          visited.insert(adj); // Then mark as visited
        }
      }
    }
    void parseTextDebug (const std::string& name, const std::string& zone, const Time& time) {
      std::cout << name << " in " << zone << " at " << time << '\n';
    }
    void parseBinaryDebug(const std::string& name, const std::string& zone, const Time& time) {
      std::cout << name << " in " << zone << " at " << time << '\n';
    }
};

void processLogDebug(const CVisitorLog& visitorLog) {
  std::cout << "------------------------------- Debugging Process Log ---------------------- \n";
  for (const auto& [name, trips] : visitorLog.logs) {
    for (const auto& trip : trips) {
      std::cout << name << ": " << "Enter " << trip.zoneA << "at " << trip.enter << " -> " << "Exit " << trip.zoneB << "at " << trip.exit <<'\n';  
    }
  }
  std::cout << "------------------------------- Finish Debugging Process Log ---------------------- \n";
}

#ifndef __PROGTEST__
void basicTests ( const CVisitorLog & log )
{
  assert ( log . search ( CAuditFilter ( "headquarters" ) )
           == ( std::set<std::string> { "Alice Cooper", "George Peterson", "Henry Montgomery", "Jane Bush", "John Smith", "Tim Cook", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notAfter ( 2026, 3, 10, 8, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 11, 12, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery", "Jane Bush", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 0 ) . notAfter ( 2026, 3, 10, 13, 0 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 5 ) . notAfter ( 2026, 3, 10, 9, 5 ) )
           == ( std::set<std::string> { "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 6 ) . notAfter ( 2026, 3, 10, 9, 6 ) )
           == ( std::set<std::string> { "Henry Montgomery", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 24 ) . notAfter ( 2026, 3, 10, 9, 24 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 25 ) . notAfter ( 2026, 3, 10, 9, 25 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2024, 2, 1, 0, 0 ) . notAfter ( 2024, 3, 31, 0, 0 ) )
           == ( std::set<std::string> { "Robert Smith", "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2025, 2, 1, 0, 0 ) . notAfter ( 2025, 3, 31, 0, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "privateParking" )  )
           == ( std::set<std::string> { "<classified>" } ) );
}

int main ()
{
  class CMilBase b;
  b . readBase ( "base.txt" );

  for ( const char * fn : std::initializer_list<const char *> { "in1.log", "in2.log", "in3.log"} )
    basicTests ( b . processLog ( fn ) );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */