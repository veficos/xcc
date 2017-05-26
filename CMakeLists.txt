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

set(TESTSET_FILES
        src/pmalloc.c
        src/cstring.c
        src/dict.c
        src/siphash.c
        src/set.c
        src/testset.c)

set(TESTMAP_FILES
        src/pmalloc.c
        src/cstring.c
        src/dict.c
        src/siphash.c
        src/map.c
        src/testmap.c)

add_executable(testarray ${TESTARRAY_FILES})
add_executable(testcstring ${TESTCSTRING_FILES})
add_executable(testset ${TESTSET_FILES})
add_executable(testmap ${TESTMAP_FILES})