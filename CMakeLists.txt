cmake_minimum_required(VERSION 3.17)
project(wy_cuda)

set(CMAKE_CXX_STANDARD 14)

add_executable(wy_cuda main.cpp Lump.h master.h)