cmake_minimum_required(VERSION 3.5)

project(occ)

set(CMAKE_C_FLAGS -g)
set(CMAKE_C_STANDARD 11)

set(TESTCSTRING src/diag.c src/pmalloc.c src/cstring.c src/testcstring.c)
set(TESTARRAY src/diag.c src/pmalloc.c src/array.c src/testarray.c)
set(TESTREADER src/diag.c src/pmalloc.c src/cstring.c src/reader.c src/testreader.c)

add_executable(testcstring ${TESTCSTRING})
add_executable(testarray ${TESTARRAY})
add_executable(testreader ${TESTREADER})
