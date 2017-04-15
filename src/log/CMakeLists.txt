cmake_minimum_required(VERSION 3.0)
project(log C CXX)

set(CMAKE_CXX_STANDARD 11)

set(CXX_FLAGS -g -Wall -Werror -Wno-deprecated -Wcpp)



add_executable(test001 test001.cc easylogging++.cc)
target_link_libraries(test001 pthread)

