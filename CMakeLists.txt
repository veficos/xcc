cmake_minimum_required(VERSION 3.5)

project(cc)

set(CMAKE_C_STANDARD 90)

set(TESTARRAY_FILES
        src/pmalloc.c
        src/array.c
        src/testarray.c)

set(TESTCSTRING_FILES
        src/pmalloc.c
        src/cstring.c
        src/testcstring.c)

add_executable(testarray ${TESTARRAY_FILES})
add_executable(testcstring ${TESTCSTRING_FILES})
