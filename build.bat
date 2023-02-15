@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types
set FLAGS=-O0 -D _DEBUG -g -gcodeview %WARNINGS%


if not exist build mkdir build



cd ./build

%CLANG% ../src/main.cpp %LIBS% %FLAGS% -o ptg.exe

cd ..