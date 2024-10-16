#   CMakeLists.txt
#
#   Author: Paul Hamilton (paul@visualops.com)
#   Date: 16-Oct-2024
#
#   Input file for Nodes admin driver for Visual Ops
#
#   Licensed under [version 3 of the GNU General Public License] contained in LICENSE.
#   
#   https://github.com/visualopsholdings/nodes

cmake_minimum_required(VERSION 3.5)

project (nodes)
  find_package(Boost 1.85.0 COMPONENTS program_options filesystem unit_test_framework chrono thread log_setup log json REQUIRED)
  find_package(cppzmq REQUIRED)
  find_package(mongocxx REQUIRED)
  find_package(bsoncxx REQUIRED)
  find_package(OpenSSL REQUIRED)
  add_definitions(-DBOOST_ALL_DYN_LINK) 
  enable_testing()

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

if (UNIX AND NOT APPLE)
  add_definitions(-funwind-tables) 
endif ()
if (APPLE)
  include_directories(/usr/local/include)
  LINK_DIRECTORIES(/opt/homebrew/Cellar/zeromq/4.3.5_1/lib)
endif ()

# fetch from other repos.

set(LIBS zmq)
set(BOOSTLIBS ${Boost_LOG_LIBRARY} ${Boost_JSON_LIBRARY})
set(CORE
  src/date.cpp
  src/json.cpp
)

include_directories(include)

# FLTK applications

if (APPLE)
  find_package(FLTK REQUIRED)
  add_executable(nodesadmin MACOSX_BUNDLE src/nodesadmin.cpp ${CORE})
  target_include_directories(nodesadmin PRIVATE ${FLTK_INCLUDE_DIR})
  # Workaround: fltk.framework does not exist, so we have to replace it by libfltk.dylib
  string(REPLACE "libfltk_images" "libfltk" FLTK_LIBRARY_NAME "${FLTK_IMAGES_LIBRARY_RELEASE}")
  string(REPLACE "/usr/local/fltk.framework" "${FLTK_LIBRARY_NAME}" FLTK_LIBRARIES "${FLTK_LIBRARIES}")
  target_link_libraries(nodesadmin ${FLTK_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${BOOSTLIBS} ${LIBS})
  set_target_properties(nodesadmin PROPERTIES FOLDER "Applications")
endif ()

