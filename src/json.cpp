/*
  json.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 2-Jul-2024
    
  Licensed under [version 3 of the GNU General Public License] contained in LICENSE.
 
  https://github.com/visualopsholdings/nodes
*/

#include "json.hpp"

#include "date.hpp"

#include <boost/log/trivial.hpp>

string Json::toISODate(json &date) {

  if (!date.is_object()) {
    return "not_object";
  }
  
  if (!date.as_object().if_contains("$date")) {
    return "bad_object";
  }
  
  return Date::toISODate(date.at("$date").as_int64());
  
}

json Json::getMember(const json &j, const string &name, bool silent) {

  if (!j.is_object()) {
    if (!silent) {
      BOOST_LOG_TRIVIAL(error) << "json is not object";
    }
    return {};
  }
  if (!j.as_object().if_contains(name)) {
    return {};
  }
  return j.at(name);
  
}

bool Json::has(const json &j, const string &name) {

  return j.is_object() && j.as_object().if_contains(name);

}

optional<string> Json::getString(const json &j, const string &name, bool silent) {

  auto obj = getMember(j, name, silent);
  if (!obj.is_string()) {
    if (!silent) {
      BOOST_LOG_TRIVIAL(error) << "obj is not string " << j << " " << name;
    }
    return {};
  }
  return boost::json::value_to<string>(obj);

}

optional<boost::json::array> Json::getArray(json &j, const string &name, bool silent) {

  auto obj = getMember(j, name, silent);
  if (!obj.is_array()) {
    if (!silent) {
      BOOST_LOG_TRIVIAL(error) << "obj is not array " << j << " " << name;
    }
    return {};
  }
  return obj.as_array();
  
}

optional<json> Json::getObject(json &j, const string &name, bool silent) {

  auto obj = getMember(j, name, silent);
  if (!obj.is_object()) {
    if (!silent) {
      BOOST_LOG_TRIVIAL(error) << "obj is not object " << j << " " << name;
    }
    return {};
  }
  return obj;
  
}

optional<bool> Json::getBool(const json &j, const string &name, bool silent) {

  auto obj = getMember(j, name, silent);
  if (!obj.is_bool()) {
    if (!silent) {
      BOOST_LOG_TRIVIAL(error) << "obj is not bool " << j << " " << name;
    }
    return {};
  }
  return boost::json::value_to<bool>(obj);

}

optional<long> Json::getNumber(const json &j, const string &name, bool silent) {

  auto obj = getMember(j, name, silent);
  if (!obj.is_int64()) {
    if (!silent) {
      BOOST_LOG_TRIVIAL(error) << "obj is not number " << j << " " << name;
    }
    return {};
  }
  return boost::json::value_to<long>(obj);

}



