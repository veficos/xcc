cmake_minimum_required(VERSION 3.5)

project(occ)

set(CMAKE_C_FLAGS -g -ansi -Wall -Wno-unused-function)
# set(CMAKE_C_STANDARD)

set(TESTREADER src/array.c		\
			   src/diag.c		\
			   src/encoding.c	\
			   src/token.c		\
			   src/cstring.c	\
			   src/pmalloc.c	\
			   src/reader.c		\
			   src/vpfmt.c		\
			   src/testreader.c)

add_executable(testreader ${TESTREADER})
