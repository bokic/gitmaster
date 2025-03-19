@echo off

SET PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64;%PATH%

git clone -bv1.8.2 --depth 1 https://github.com/libgit2/libgit2
cd libgit2
mkdir build
cd build
cmake.exe -DBUILD_TESTS=OFF ..
MSBuild.exe libgit2.sln /property:Configuration=Release
cd ..\..
mkdir ..\include
xcopy /y /e libgit2\include ..\include
xcopy /y /e libgit2\build\Release\git2.dll ..\bin
xcopy /y /e libgit2\build\Release\git2.lib ..\bin
