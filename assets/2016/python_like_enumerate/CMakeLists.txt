cmake_minimum_required (VERSION 2.6)
project (python_like)
set(CMAKE_CXX_COMPILER clang++)
set(MY_CXX_FLAGS "-fno-elide-constructors")
#set(MY_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1z -g -O3 ${MY_CXX_FLAGS}")
add_executable(python_like python_like.cpp)

set(CMAKE_CTEST_COMMAND python_like)
add_test(test python_like)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND}
                  DEPENDS python_like)
