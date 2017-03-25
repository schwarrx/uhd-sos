cmake_minimum_required (VERSION 3.2)
project(uhd-sos)

SET(BUILD_SHARED_LIBS ON)

option(debug "Build with debugging symbols." ON)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

if (debug)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
endif ()


add_subdirectory(dfm-kernel) 
