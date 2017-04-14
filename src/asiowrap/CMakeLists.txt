cmake_minimum_required(VERSION 3.0)
project(asiowrapp C CXX)

set(CMAKE_CXX_STANDARD 11)

set(CXX_FLAGS -g -Wall -Werror -Wno-deprecated -Wcpp)

find_package(Boost REQUIRED)


set(SOURCE_FILES Main.cc 
    TcpClient.cc 
    TcpServer.cc
    TcpConn.cc
    EventLoop.cc
    EventLoopThread.cc
    EventLoopThreadPool.cc
)

add_executable(echoserver ${SOURCE_FILES})
target_link_libraries(echoserver
    pthread
    boost_system
)