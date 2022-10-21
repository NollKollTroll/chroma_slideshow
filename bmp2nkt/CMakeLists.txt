cmake_minimum_required(VERSION 2.6)
project(bmp2nkt)

add_executable(bmp2nkt main.cpp libbmp-master/CPP/libbmp.cpp)

install(TARGETS bmp2nkt RUNTIME DESTINATION bin)
