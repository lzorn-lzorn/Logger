cmake_minimum_required(VERSION 3.20)

project(LoggerTest)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(Logger)

add_executable(${CMAKE_PROJECT_NAME} ./main.cpp)


# 链接 Logger 库
target_link_libraries(LoggerTest PRIVATE Logger)
target_include_directories(LoggerTest PRIVATE Logger/include)