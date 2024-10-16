/*
  nodesadmin.cpp
  
  Author: Paul Hamilton (paul@visualops.com)
  Date: 1-Jul-2024
    
  Admin tool for Nodes using FLTK.
  
  Licensed under [version 3 of the GNU General Public License] contained in LICENSE.
 
  https://github.com/visualopsholdings/nodes
*/

#include "json.hpp"

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/fl_message.H>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <boost/program_options.hpp> 
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/json.hpp>
#include <zmq.hpp>

namespace po = boost::program_options;

using namespace std;
using json = boost::json::value;

class MainWindow : public Fl_Window {
  public:
    MainWindow(const string &reqConn);
    
    void send(const json &json);
    json receive();
    optional<string> getString(json &reply, const string &name);
    optional<boost::json::array> getArray(json &reply, const string &name);
    optional<string> getDate(json &reply, const string &name);
    
  private:
    Fl_Button users {20, 20, 100, 30, "Users"};
    
    shared_ptr<Fl_Window> _usersw;
    shared_ptr<Fl_Window> _streamsw;
    zmq::context_t _context;
    zmq::socket_t _req;
};

class Users_Table : public Fl_Table {
  public:
    Users_Table(MainWindow *main, int x, int y, int width, int height) : Fl_Table {x, y, width, height} {

      main->send({ { "type", "users" } });
      json j = main->receive();
      BOOST_LOG_TRIVIAL(trace) << j;
      auto type = main->getString(j, "type");
      if (!type) {
        fl_message_title("Alert");
        fl_alert("Missing type");
        return;
      }
      if (type == "err") {
        auto msg = main->getString(j, "msg");
        fl_message_title("Alert");
        fl_alert("%s", msg ? msg.value().c_str() : "unknown");
        return;
      }
      auto users = main->getArray(j, "users");
      if (!users) {
        fl_message_title("Alert");
        fl_alert("Missing users");
        return;
      }
      for (auto i: users.value()) {
        auto id = main->getString(i, "id");
        auto modifyDate = main->getDate(i, "modifyDate");
        auto name = main->getString(i, "name");
        std::vector<std::string> row;
        row.push_back(id ? id.value() : "?");
        row.push_back(modifyDate ? modifyDate.value() : "?");
        row.push_back(name ? name.value() : "?");
        _cells.push_back(row);
      }
      
      rows(static_cast<int>(_cells.size()));
      cols(static_cast<int>(_cells[0].size()));
      col_header(true);
      int w = ((width-2) / 3);
      col_width(0, w);
      col_width(1, w);
      col_width(2, w);
      col_resize(true);
    }

  private:
    static void draw_column_header(const std::string& value, int x, int y, int width, int height) noexcept {
      fl_push_clip(x, y, width, height);
      fl_draw_box(FL_THIN_UP_BOX, x, y, width, height, FL_BACKGROUND_COLOR);
      fl_color(FL_FOREGROUND_COLOR);
      fl_font(FL_HELVETICA_BOLD, FL_NORMAL_SIZE);
      fl_draw(value.c_str(), x, y, width, height, FL_ALIGN_CENTER);
      fl_pop_clip();
    }

    static void draw_cell(const std::string& value, int x, int y, int width, int height) noexcept {
      fl_push_clip(x, y, width, height);
      fl_color(FL_BACKGROUND2_COLOR);
      fl_rectf(x, y, width, height);
      fl_color(FL_FOREGROUND_COLOR);
      fl_font(FL_HELVETICA, FL_NORMAL_SIZE);
      fl_draw(value.c_str(), x + 2, y, width, height, FL_ALIGN_LEFT);
      fl_color(FL_BACKGROUND_COLOR);
      fl_rect(x, y, width, height);
      fl_pop_clip();
    }

    void draw_cell(TableContext context, int row = 0, int column = 0, int x = 0, int y = 0, int width = 0, int height = 0) noexcept override {
      Fl_Table::draw_cell(context, row, column, x, y, width, height);
      switch (context) {
        case CONTEXT_COL_HEADER: draw_column_header(_headers[column], x, y, width, height);  return;
        case CONTEXT_CELL: draw_cell(_cells[row][column], x, y, width, height); return;
        default: return;
      }
    }
    std::vector<std::string> _headers {"Id", "ModifyDate", "Name"};
    std::vector<std::vector<std::string> > _cells;
};

class Users_Window : public Fl_Window {
  public:
    Users_Window(MainWindow *main) : Fl_Window {250, 150, WIDTH+20, 400, "Users"}, table {main, 10, 10, WIDTH, 380} {
    }
    
    enum {
      WIDTH = 600
    };
    
  private:
    Users_Table table;
};

MainWindow::MainWindow(const string &reqConn) : 
  Fl_Window {200, 100, 140, 70, "Nodes Admin"}, 
    _context(1), _req(_context, ZMQ_REQ) {

  _req.connect(reqConn);

  users.align(FL_ALIGN_INSIDE | FL_ALIGN_CLIP | FL_ALIGN_WRAP);
  users.callback([](Fl_Widget* sender, void* window) {
    auto self = reinterpret_cast<MainWindow*>(window);
    if (self->_usersw.get()) {
      self->_usersw->show();
      return;
    }
    self->_usersw.reset(new Users_Window {self});
    self->_usersw->end();
    self->_usersw->show();
  }, this);

}

void MainWindow::send(const json &json) {

  stringstream ss;
  ss << json;
  string m = ss.str();
  zmq::message_t msg(m.length());
  memcpy(msg.data(), m.c_str(), m.length());
  _req.send(msg);

}

json MainWindow::receive() {

  zmq::message_t reply;
  _req.recv(&reply);
  string r((const char *)reply.data(), reply.size());
  return boost::json::parse(r);

}

optional<string> MainWindow::getString(json &reply, const string &name) {

  if (!reply.is_object()) {
    BOOST_LOG_TRIVIAL(error) << "json is not object";
    return {};
  }
  if (!reply.as_object().if_contains(name)) {
    BOOST_LOG_TRIVIAL(error) << "json missing " << name;
    return {};
  }
  return reply.at(name).as_string().c_str();
  
}

optional<string> MainWindow::getDate(json &reply, const string &name) {

  if (!reply.is_object()) {
    BOOST_LOG_TRIVIAL(error) << "json is not object";
    return {};
  }
  if (!reply.as_object().if_contains(name)) {
    BOOST_LOG_TRIVIAL(error) << "json missing " << name;
    return {};
  }
  auto date = reply.at(name);
  if (!date.is_object()) {
    BOOST_LOG_TRIVIAL(error) << "date is not object";
    return {};
  }
  
  return Json::toISODate(date);
  
}

optional<boost::json::array> MainWindow::getArray(json &reply, const string &name) {

  if (!reply.is_object()) {
    BOOST_LOG_TRIVIAL(error) << "json is not object";
    return {};
  }
  if (!reply.as_object().if_contains(name)) {
    BOOST_LOG_TRIVIAL(error) << "json missing " << name;
    return {};
  }
  auto obj = reply.at(name);
  if (!obj.is_array()) {
    BOOST_LOG_TRIVIAL(error) << "obj is not array";
    return {};
  }
  return obj.as_array();
  
}

int main(int argc, char *argv[]) {

  int reqPort;
  string logLevel;
  
  po::options_description desc("Allowed options");
  desc.add_options()
    ("reqPort", po::value<int>(&reqPort)->default_value(3013), "ZMQ Req port.")
    ("logLevel", po::value<string>(&logLevel)->default_value("info"), "Logging level [trace, debug, warn, info].")
    ("help", "produce help message")
    ;
  po::positional_options_description p;

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);
  po::notify(vm);   

  if (logLevel == "trace") {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
  }
  else if (logLevel == "debug") {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::debug);
  }
  else if (logLevel == "warn") {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::warning);
  }
  else {
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
  }

  boost::log::formatter logFmt =
         boost::log::expressions::format("%1%\tzcadmin [%2%]\t%3%")
        %  boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") 
        %  boost::log::expressions::attr< boost::log::trivial::severity_level>("Severity")
        %  boost::log::expressions::smessage;
  boost::log::add_common_attributes();
  boost::log::add_console_log(clog)->set_formatter(logFmt);

  if (vm.count("help")) {
    cout << desc << endl;
    return 1;
  }
  
  auto window = MainWindow {"tcp://127.0.0.1:" + to_string(reqPort)};
  window.show();
  return Fl::run();
  
}
