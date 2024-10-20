@echo off

SET PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64;%PATH%
REM SET CMAKE_MODULE_PATH=C:\Qt\Tools\CMake_64\share\cmake-3.29\Modules
SET QT6_DIR=C:\Qt\6.8.0\msvc2022_64\lib\cmake\Qt6\

mkdir build
cd build
cmake.exe ../..
REM MSBuild.exe libgit2.sln /property:Configuration=Release
