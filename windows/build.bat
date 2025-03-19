@echo off

SET PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64;%PATH%
SET QT_DIR=C:\Qt\6.8.2\msvc2022_64\lib\cmake\Qt6
SET Qt6_DIR=C:\Qt\6.8.2\msvc2022_64\lib\cmake\Qt6

mkdir build
cd build
cmake.exe ../..
MSBuild.exe gitmaster.sln /property:Configuration=Release
