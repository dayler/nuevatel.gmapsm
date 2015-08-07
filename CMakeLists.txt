cmake_minimum_required(VERSION 3.2)
project(trunk)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    base/appconn/appclient.hpp
    base/appconn/appconn.hpp
    base/appconn/baseconn.hpp
    base/appconn/basicie.hpp
    base/appconn/compositeie.hpp
    base/appconn/ie.hpp
    base/appconn/message.hpp
    base/cqueue.hpp
    base/exception.hpp
    base/executor.hpp
    base/logger.hpp
    base/properties.hpp
    base/socket.hpp
    base/thread.hpp
    base/timer.hpp
    gmap/gblocksm.hpp
    gmap/gmapconn.hpp
    gmap/gmapsm.hpp
    main.cpp)

include_directories("/Users/asalazar/dev/cpp/ulcm/include")
add_executable(trunk ${SOURCE_FILES})