@echo off

rmdir /s /q build

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake.exe -S .. -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake.exe --build build --config Release --verbose

rmdir /s /q build
