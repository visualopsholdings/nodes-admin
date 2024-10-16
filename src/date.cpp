/*
  date.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 18-Sep-2024
    
  Licensed under [version 3 of the GNU General Public License] contained in LICENSE.
 
  https://github.com/visualopsholdings/nodes
*/

#include "date.hpp"

#include <sstream>
#include <ctime>
#include <iomanip>
#include <boost/log/trivial.hpp>
#include <cstdlib>

#define TIME_FORMAT "%Y-%m-%dT%H:%M:%S"
#define RUBY_TIME_FORMAT "%Y-%m-%d %H:%M:%S"

long Date::now() {

  auto now = chrono::system_clock::now();
  auto duration = now.time_since_epoch();
  auto milliseconds
        = chrono::duration_cast<chrono::milliseconds>(
              duration)
              .count();

  return milliseconds;

}

string Date::getFutureTime(long now, int hours) {

  time_t tnum = now / 1000;

  auto tp = chrono::system_clock::from_time_t(tnum);
  auto hrs = chrono::hours(hours);
  long t = chrono::system_clock::to_time_t(tp + hrs);
  
  t *= 1000;

  int ms = now - (tnum * 1000);
  t += ms;
  
  return Date::toISODate(t);

}

string Date::toISODate(long t) {

  time_t tnum = t / 1000;
  tm tm = *gmtime(&tnum);
  
//   BOOST_LOG_TRIVIAL(trace) << "tm_sec " << tm.tm_sec;
//   BOOST_LOG_TRIVIAL(trace) << "tm_min " << tm.tm_min;
//   BOOST_LOG_TRIVIAL(trace) << "tm_hour " << tm.tm_hour;
//   BOOST_LOG_TRIVIAL(trace) << "tm_mday " << tm.tm_mday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_mon " << tm.tm_mon;
//   BOOST_LOG_TRIVIAL(trace) << "tm_year " << tm.tm_year;
//   BOOST_LOG_TRIVIAL(trace) << "tm_wday " << tm.tm_wday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_yday " << tm.tm_yday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_isdst " << tm.tm_isdst;

  int ms = t - (tnum * 1000);

  stringstream ss;
  // 2024-07-01T06:54:39
  ss << put_time(&tm, TIME_FORMAT);
  ss << ".";
  ss << ms;
  ss << "+00:00";

  return ss.str();
  
}

long Date::fromISODate(const string &d) {

//  BOOST_LOG_TRIVIAL(trace) << d;

  if (d.rfind("T") == string::npos) {
    return fromRubyDate(d);
  }
  
  auto dot = d.rfind(".");
  if (dot == string::npos) {
    BOOST_LOG_TRIVIAL(error) << "no dot in " << d;
    return 0;
  }
  
  string start = d.substr(0, dot);
//  BOOST_LOG_TRIVIAL(trace) << start;
  
  string rem = d.substr(dot+1);
//  BOOST_LOG_TRIVIAL(trace) << rem;
  auto plus = rem.rfind("+");
  if (plus == string::npos) {
    BOOST_LOG_TRIVIAL(error) << "no plus in " << rem;
    return 0;
  }

  tm tm = {};
  istringstream ss(start);
  // 2024-07-01T06:54:39
  ss >> get_time(&tm, TIME_FORMAT);
   
//   BOOST_LOG_TRIVIAL(trace) << "tm_sec " << tm.tm_sec;
//   BOOST_LOG_TRIVIAL(trace) << "tm_min " << tm.tm_min;
//   BOOST_LOG_TRIVIAL(trace) << "tm_hour " << tm.tm_hour;
//   BOOST_LOG_TRIVIAL(trace) << "tm_mday " << tm.tm_mday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_mon " << tm.tm_mon;
//   BOOST_LOG_TRIVIAL(trace) << "tm_year " << tm.tm_year;
//   BOOST_LOG_TRIVIAL(trace) << "tm_wday " << tm.tm_wday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_yday " << tm.tm_yday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_isdst " << tm.tm_isdst;

  auto t = timegm(&tm);
//  BOOST_LOG_TRIVIAL(trace) << "t: " << t;
  
  rem = rem.substr(0, plus);
  long ms = atol(rem.substr(0, plus).c_str());
//  BOOST_LOG_TRIVIAL(trace) << "ms: " << ms;
  
  return (t * 1000) + ms;
  
}

long Date::fromRubyDate(const string &d) {

  auto spc = d.rfind(" ");
  if (spc == string::npos) {
    BOOST_LOG_TRIVIAL(error) << "no space in " << d;
    return 0;
  }
  
  string start = d.substr(0, spc);
//  BOOST_LOG_TRIVIAL(trace) << start;
  
  tm tm = {};
  istringstream ss(start);
  // 2024-07-01 06:54:39
  ss >> get_time(&tm, RUBY_TIME_FORMAT);
   
//   BOOST_LOG_TRIVIAL(trace) << "tm_sec " << tm.tm_sec;
//   BOOST_LOG_TRIVIAL(trace) << "tm_min " << tm.tm_min;
//   BOOST_LOG_TRIVIAL(trace) << "tm_hour " << tm.tm_hour;
//   BOOST_LOG_TRIVIAL(trace) << "tm_mday " << tm.tm_mday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_mon " << tm.tm_mon;
//   BOOST_LOG_TRIVIAL(trace) << "tm_year " << tm.tm_year;
//   BOOST_LOG_TRIVIAL(trace) << "tm_wday " << tm.tm_wday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_yday " << tm.tm_yday;
//   BOOST_LOG_TRIVIAL(trace) << "tm_isdst " << tm.tm_isdst;

  auto t = timegm(&tm);
//  BOOST_LOG_TRIVIAL(trace) << "t: " << t;
  
  string rem = d.substr(spc+1);
//  BOOST_LOG_TRIVIAL(trace) << rem;
  auto plus = rem.rfind("+");
  if (plus == string::npos) {
    BOOST_LOG_TRIVIAL(error) << "no plus in " << rem;
    return 0;
  }

  long hrs = atol(rem.substr(1).c_str()) / 100;
//  BOOST_LOG_TRIVIAL(trace) << hrs;
  long offs = hrs * 60 * 60;
  if (rem[0] == '-') {
    offs *= -1;
  }
//  BOOST_LOG_TRIVIAL(trace) << offs;

  return (t + offs) * 1000;
  
}

