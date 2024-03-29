cmake_minimum_required(VERSION 3.0.0)
project(simple_logger_example)

set(CMAKE_CXX_STANDARD 20)

set(SRC 
    ${PROJECT_SOURCE_DIR}/../src/DateTime.cpp
    ${PROJECT_SOURCE_DIR}/../src/Formatter.cpp
    ${PROJECT_SOURCE_DIR}/../src/Logger.cpp
    ${PROJECT_SOURCE_DIR}/Example.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/../include
)

link_directories(${PROJECT_SOURCE_DIR})

add_executable(simple_logger_example ${SRC})

if(WIN32)
    MESSAGE(STATUS "Current OS is windows system")
    target_link_libraries(simple_logger_example)
elseif(APPLE)
    MESSAGE(STATUS "Current OS is Apple system.")
    target_link_libraries(simple_logger_example -lstdc++ -lpthread)
elseif(UNIX)
    MESSAGE(STATUS "Current OS is UNIX-like(including linux) system.")
    target_link_libraries(simple_logger_example -lstdc++ -lpthread)
endif()

