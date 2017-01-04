cmake_minimum_required(VERSION 3.5)

project(occ)

set(CMAKE_C_STANDARD 11)

set(TESTCSTRING src/cstring.c src/testcstring.c)
set(TESTARRAY src/array.c src/testarray.c)

add_executable(testcstring ${TESTCSTRING})
add_executable(testarray ${TESTARRAY})
