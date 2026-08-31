@echo off
setlocal

SET ZLIB_VERSION=1.3.2
SET LIBGIT2_VERSION=1.9.0

if not defined DevEnvDir (
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
    )
)

:: Clean previous build artifacts
if exist zlib rmdir /s /q zlib
if exist build-zlib rmdir /s /q build-zlib
if exist libgit2 rmdir /s /q libgit2
if exist build-libgit2 rmdir /s /q build-libgit2
if exist install rmdir /s /q install

:: --- Build ZLIB ---
curl -L --output zlib.zip https://github.com/madler/zlib/archive/refs/tags/v%ZLIB_VERSION%.zip
tar -xf zlib.zip
del zlib.zip
ren zlib-%ZLIB_VERSION% zlib

cmake -S zlib -B build-zlib -DCMAKE_INSTALL_PREFIX=install -A x64
cmake --build build-zlib --config Release
cmake --install build-zlib --config Release

:: --- Build libgit2 ---
curl -L --output libgit2.zip https://github.com/libgit2/libgit2/archive/refs/tags/v%LIBGIT2_VERSION%.zip
tar -xf libgit2.zip
del libgit2.zip
ren libgit2-%LIBGIT2_VERSION% libgit2

cmake -S libgit2 -B build-libgit2 -DBUILD_TESTS=OFF -DBUILD_CLI=OFF -DLIBGIT2_FILENAME=git2 -DCMAKE_INSTALL_PREFIX=install -A x64
cmake --build build-libgit2 --config Release
cmake --install build-libgit2 --config Release

:: --- Copy to project include, bin & lib dirs ---
if not exist "..\bin" mkdir ..\bin
if not exist "..\lib" mkdir ..\lib
if not exist "..\include" mkdir ..\include

xcopy /y /e install\include ..\include
if exist install\bin copy /y install\bin\*.dll ..\bin\
if exist install\lib copy /y install\lib\*.lib ..\bin\
if exist install\lib copy /y install\lib\*.lib ..\lib\

:: --- Cleanup ---
rmdir /s /q zlib
rmdir /s /q build-zlib
rmdir /s /q libgit2
rmdir /s /q build-libgit2
rmdir /s /q install

endlocal
