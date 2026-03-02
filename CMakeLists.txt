cmake_minimum_required(VERSION 3.5)
project(cpp-proxy)

set(CMAKE_CXX_STANDARD 17)

add_library(common
    src/common/Logger.cpp
    src/common/Utils.cpp
)

add_library(epoll_proxy
    src/epoll/EpollProxy.cpp
)

add_library(thread_proxy
    src/thread/ThreadProxy.cpp
)

target_link_libraries(epoll_proxy common)
target_link_libraries(thread_proxy common)

add_executable(epoll_proxy_bin src/main_epoll.cpp)
add_executable(thread_proxy_bin src/main_thread.cpp)

target_link_libraries(epoll_proxy_bin epoll_proxy common)
target_link_libraries(thread_proxy_bin thread_proxy common pthread)
