cmake_minimum_required(VERSION 3.5)

project(cc)

set(CMAKE_C_STANDARD 90)

set(TESTARRAY_FILES
        src/config.h
        src/pmalloc.h
        src/pmalloc.c
        src/array.h
        src/array.c
        src/unittest.h
        src/testarray.c)

set(TESTCSTRING_FILES
        src/config.h
        src/pmalloc.c
        src/pmalloc.c
        src/cstring.c
        src/cstring.c
        src/unittest.h
        src/testcstring.c)

set(TESTDICT_FILES
        src/config.h
        src/pmalloc.h
        src/pmalloc.c
        src/cstring.h
        src/cstring.c
        src/hash.h
        src/siphash.c
        src/dict.h
        src/dict.c
        src/unittest.h
        src/testdict.c)

set(TESTCSPOOL_FILES
        src/config.h
        src/pmalloc.h
        src/pmalloc.c
        src/cstring.h
        src/cstring.c
        src/hash.h
        src/siphash.c
        src/dict.h
        src/dict.c
        src/cspool.h
        src/cspool.c
        src/unittest.h
        src/testcspool.c)

set(TESTSET_FILES
        src/config.h
        src/pmalloc.h
        src/pmalloc.c
        src/cstring.h
        src/cstring.c
        src/dict.h
        src/dict.c
        src/hash.h
        src/siphash.c
        src/set.h
        src/set.c
        src/unittest.h
        src/testset.c)

set(TESTMAP_FILES
        src/config.h
        src/pmalloc.h
        src/pmalloc.c
        src/cstring.h
        src/cstring.c
        src/dict.h
        src/dict.c
        src/hash.h
        src/siphash.c
        src/map.h
        src/map.c
        src/unittest.h
        src/testmap.c)

set(TESTDIAGNOSTOR_FILES
        src/config.h
        src/color.h
        src/pmalloc.h
        src/pmalloc.c
        src/cstring.h
        src/cstring.c
        src/hash.h
        src/siphash.c
        src/dict.h
        src/dict.c
        src/set.h
        src/set.c
        src/encoding.h
        src/encoding.c
        src/token.h
        src/token.c
        src/option.h
        src/option.c
        src/diagnostor.h
        src/diagnostor.c
        src/unittest.h
        src/testdiagnostor.c)

set(TESTREADER_FILES
        src/config.h
        src/color.h
        src/pmalloc.h
        src/pmalloc.c
        src/cstring.h
        src/cstring.c
        src/cspool.h
        src/cspool.c
        src/array.h
        src/array.c
        src/hash.h
        src/siphash.c
        src/dict.h
        src/dict.c
        src/set.h
        src/set.c
        src/encoding.h
        src/encoding.c
        src/token.h
        src/token.c
        src/option.h
        src/option.c
        src/diagnostor.h
        src/diagnostor.c
        src/reader.h
        src/reader.c
        src/utils.h
        src/unittest.h
        src/testreader.c)

set(TESTLEXER_FILES
        src/config.h
        src/color.h
        src/pmalloc.h
        src/pmalloc.c
        src/cstring.h
        src/cstring.c
        src/cspool.h
        src/cspool.c
        src/array.h
        src/array.c
        src/hash.h
        src/siphash.c
        src/dict.h
        src/dict.c
        src/set.h
        src/set.c
        src/encoding.h
        src/encoding.c
        src/token.h
        src/token.c
        src/option.h
        src/option.c
        src/diagnostor.h
        src/diagnostor.c
        src/reader.h
        src/reader.c
        src/lexer.h
        src/lexer.c
        src/utils.h
        src/unittest.h
        src/testlexer.c)


add_executable(testarray ${TESTARRAY_FILES})
add_executable(testcstring ${TESTCSTRING_FILES})
add_executable(testdict ${TESTDICT_FILES})
add_executable(testcspool ${TESTCSPOOL_FILES})
add_executable(testset ${TESTSET_FILES})
add_executable(testmap ${TESTMAP_FILES})
add_executable(testdiagnostor ${TESTDIAGNOSTOR_FILES})
add_executable(testreader ${TESTREADER_FILES})
add_executable(testlexer ${TESTLEXER_FILES})
