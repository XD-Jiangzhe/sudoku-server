cmake_minimum_required(VERSION 3.5)

project(SudokuServer)

SET(CMAKE_CXX_FLAGS "-g -std=c++11")

find_library(muduo_base_cpp11 muduo_base_cpp11)
find_library(muduo_net muduo_net)

MESSAGE(STATUS ${muduo_base_cpp11})
MESSAGE(STATUS ${muduo_net})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)
MESSAGE(STATUS PROJECT_BINARY_DIR)

add_executable(SudokuServer sudoku_onethread.cc sudoku.cc)
target_link_libraries(SudokuServer muduo_base_cpp11 muduo_net pthread)


add_executable(SudokuServer_threadpool sudoku_threadpool.cc sudoku.cc)
target_link_libraries(SudokuServer_threadpool   muduo_net muduo_base_cpp11 pthread boost_system)


add_executable(use_mypool sudoku_mythread.cc sudoku.cc)
target_link_libraries(use_mypool   muduo_net muduo_base_cpp11 pthread boost_system)

add_executable(multi_io_multi_compute sudoku_mythread.cc sudoku.cc)
target_link_libraries(multi_io_multi_compute  muduo_net muduo_base_cpp11 pthread boost_system)