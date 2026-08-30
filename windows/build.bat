@echo off

rmdir /s /q build

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake.exe -S .. -B build -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/msvc2022_64 -A x64
cmake.exe --build build --config Release --verbose

rmdir /s /q build
