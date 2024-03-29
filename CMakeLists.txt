cmake_minimum_required(VERSION 3.0.0)
project(simple_logger)

set(CMAKE_CXX_STANDARD 20)

set(SRC 
    ${PROJECT_SOURCE_DIR}/src/DateTime.cpp
    ${PROJECT_SOURCE_DIR}/src/Formatter.cpp
    ${PROJECT_SOURCE_DIR}/src/Logger.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

link_directories(${PROJECT_SOURCE_DIR})

add_library(simple_logger STATIC ${SRC})

if(WIN32)
    MESSAGE(STATUS "Current OS is windows system")
    target_link_libraries(simple_logger)
elseif(APPLE)
    MESSAGE(STATUS "Current OS is Apple system.")
    target_link_libraries(simple_logger -lstdc++ -lpthread)
elseif(UNIX)
    MESSAGE(STATUS "Current OS is UNIX-like(including linux) system.")
    target_link_libraries(simple_logger -lstdc++ -lpthread)
endif()

