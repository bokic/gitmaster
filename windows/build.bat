@echo off

SET QT_DIR=C:\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6
SET Qt6_DIR=C:\Qt\6.10.1\msvc2022_64\lib\cmake\Qt6

rmdir /s /q build

cmake.exe -S .. -B build
cmake --build build --config Release

rmdir /s /q build
