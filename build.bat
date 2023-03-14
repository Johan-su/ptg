@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-gnu-zero-variadic-macro-arguments -Wno-c99-extensions
set FLAGS=-O0 -D _DEBUG -D _CRT_SECURE_NO_WARNINGS  -g -gcodeview %WARNINGS%


if not exist build mkdir build

cd ./build

%CLANG% ../src/ptg.cpp %FLAGS% -o ptg.exe

if not exist tests mkdir tests
cd ./tests

%CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf1.cpp %FLAGS% -Wno-unused-function -o bnf1.exe
%CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf2.cpp %FLAGS% -Wno-unused-function -o bnf2.exe
%CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf3.cpp %FLAGS% -Wno-unused-function -o bnf3.exe
%CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf4.cpp %FLAGS% -Wno-unused-function -o bnf4.exe
%CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf5.cpp %FLAGS% -Wno-unused-function -o bnf5.exe
%CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf6.cpp %FLAGS% -Wno-unused-function -o bnf6.exe
@REM %CLANG% -D PTG_LIB ../../src/ptg.cpp ../../tests/bnf7.cpp %FLAGS% -Wno-unused-function -o bnf7.exe

bnf1.exe
bnf2.exe
bnf3.exe
bnf4.exe
bnf5.exe
bnf6.exe
@REM bnf7.exe


cd ..
cd ..