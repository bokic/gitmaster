@echo off

rmdir /s /q build

cmake.exe -S .. -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake.exe --build build --config Release --verbose

rmdir /s /q build
