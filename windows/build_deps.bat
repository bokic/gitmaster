@echo off

SET LIBGIT2_VERSION=1.9.7

rmdir /s /q libgit2
rmdir /s /q build
rmdir /s /q install

curl -L --output libgit2.zip https://github.com/libgit2/libgit2/archive/refs/tags/v%LIBGIT2_VERSION%.zip
tar -xf libgit2.zip
del libgit2.zip
ren libgit2-%LIBGIT2_VERSION% libgit2

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake.exe -S libgit2 -B build -DBUILD_TESTS=OFF -DBUILD_CLI=OFF -DLIBGIT2_FILENAME=git2 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -G Ninja
cmake.exe --build build --config Release --verbose
cmake.exe --install build --config Release

rmdir /s /q ..\include
mkdir ..\bin
mkdir ..\include

xcopy /y /e install\include ..\include
xcopy /y /e install\bin\git2.dll ..\bin
xcopy /y /e install\lib\git2.lib ..\bin

rmdir /s /q install
rmdir /s /q libgit2
rmdir /s /q build
