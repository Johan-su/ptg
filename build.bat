@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-gnu-zero-variadic-macro-arguments -Wno-c99-extensions
set FLAGS=-O0 -D _DEBUG -D _CRT_SECURE_NO_WARNINGS  -g -gcodeview -std=c++11 %WARNINGS%


if not exist build mkdir build
cd ./build


%CLANG% ../src/ptg.cpp ../src/ptg_cmd.cpp %FLAGS% -o ptg.exe

if not exist tests mkdir tests
cd ./tests

@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf1.cpp %FLAGS% -Wno-unused-function -o bnf1.exe && bnf1.exe
@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf2.cpp %FLAGS% -Wno-unused-function -o bnf2.exe && bnf2.exe 
@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf3.cpp %FLAGS% -Wno-unused-function -o bnf3.exe && bnf3.exe
@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf4.cpp %FLAGS% -Wno-unused-function -o bnf4.exe && bnf4.exe
@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf5.cpp %FLAGS% -Wno-unused-function -o bnf5.exe && bnf5.exe
@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf6.cpp %FLAGS% -Wno-unused-function -o bnf6.exe && bnf6.exe
@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf7.cpp %FLAGS% -Wno-unused-function -o bnf7.exe && bnf7.exe


cd ..
cd ..