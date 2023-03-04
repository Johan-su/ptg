@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types
set FLAGS=-O0 -D _DEBUG -D _CRT_SECURE_NO_WARNINGS  -g -gcodeview %WARNINGS%


if not exist build mkdir build

cd ./build

%CLANG% ../src/main.cpp %FLAGS% -o ptg.exe

if not exist examples mkdir examples
cd ./examples

%CLANG% ../../examples/bnf1.cpp %FLAGS% -o bnf1.exe
%CLANG% ../../examples/bnf2.cpp %FLAGS% -o bnf2.exe
%CLANG% ../../examples/bnf3.cpp %FLAGS% -o bnf3.exe
%CLANG% ../../examples/bnf4.cpp %FLAGS% -o bnf4.exe
%CLANG% ../../examples/bnf5.cpp %FLAGS% -o bnf5.exe
%CLANG% ../../examples/bnf6.cpp %FLAGS% -o bnf6.exe

bnf1.exe
bnf2.exe
bnf3.exe
bnf4.exe
bnf5.exe
bnf6.exe


cd ..
cd ..