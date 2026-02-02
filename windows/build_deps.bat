@echo off

SET LIBGIT2_VERSION=1.9.2

rmdir /s /q libgit2
rmdir /s /q build

curl -L --output libgit2.zip https://github.com/libgit2/libgit2/archive/refs/tags/v%LIBGIT2_VERSION%.zip
tar -xf libgit2.zip
del libgit2.zip
ren libgit2-%LIBGIT2_VERSION% libgit2

cmake.exe -S libgit2 -B build -DBUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake.exe --build build --target libgit2package --config Release  --verbose

rmdir /s /q ..\include
mkdir ..\bin
mkdir ..\include

xcopy /y /e libgit2\include ..\include
xcopy /y /e build\git2.dll ..\bin
xcopy /y /e build\git2.lib ..\bin

rmdir /s /q libgit2
rmdir /s /q build

