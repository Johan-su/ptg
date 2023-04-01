@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-gnu-zero-variadic-macro-arguments -Wno-c99-extensions

set COMMON=-D _CRT_SECURE_NO_WARNINGS -std=c++11 %WARNINGS%


@REM set SANITIZE=-fsanitize=address -fsanitize=undefined
@REM set FLAGS=-O0 -D _DEBUG  -g -gcodeview %COMMON% %SANITIZE%
set FLAGS=-O2 %COMMON%


if not exist build mkdir build
cd ./build


if not exist tests mkdir tests
cd ./tests
cd ..
cd ..

@REM %CLANG% -shared src/ptg.cpp %FLAGS% -D BUILD_DLL -o build/ptg.dll
@REM %CLANG% src/ptg.cpp src/ptg_cmd.cpp %FLAGS% -o build/ptg.exe

%CLANG% src/ptg.cpp tests/bnf1.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf1.exe && "build/tests/bnf1.exe"
%CLANG% src/ptg.cpp tests/bnf2.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf2.exe && "build/tests/bnf2.exe" 
%CLANG% src/ptg.cpp tests/bnf3.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf3.exe && "build/tests/bnf3.exe"
%CLANG% src/ptg.cpp tests/bnf4.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf4.exe && "build/tests/bnf4.exe"
%CLANG% src/ptg.cpp tests/bnf5.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf5.exe && "build/tests/bnf5.exe"
%CLANG% src/ptg.cpp tests/bnf6.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf6.exe && "build/tests/bnf6.exe"
%CLANG% src/ptg.cpp tests/bnf7.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf7.exe && "build/tests/bnf7.exe"
%CLANG% src/ptg.cpp tests/bnf8.cpp %FLAGS% -Wno-unused-function -o build/tests/bnf8.exe && "build/tests/bnf8.exe"