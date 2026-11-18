@echo off
setlocal

SET ZLIB_VERSION=1.4
SET LIBGIT2_VERSION=1.9.7

SET "ZLIB_SRC=zlib-%ZLIB_VERSION%"
SET "LIBGIT2_SRC=libgit2-%LIBGIT2_VERSION%"

pushd "%~dp0" || (
    echo [ERROR] Cannot change directory to "%~dp0"
    exit /b 1
)
SET "INSTALL_DIR=%CD%\install"

call :check_tools
if errorlevel 1 goto :fail

call :setup_msvc
if errorlevel 1 goto :fail

:: Clean previous build artifacts
call :clean_artifacts

:: --- Build ZLIB ---
echo.
echo === Building zlib %ZLIB_VERSION% ===
curl -fL --retry 3 --output zlib.zip "https://github.com/madler/zlib/archive/refs/tags/v%ZLIB_VERSION%.zip"
if errorlevel 1 goto :fail

tar -xf zlib.zip
if errorlevel 1 goto :fail
del /f /q zlib.zip

if not exist "%ZLIB_SRC%\CMakeLists.txt" (
    echo [ERROR] zlib sources not found in "%ZLIB_SRC%".
    goto :fail
)

cmake -S "%ZLIB_SRC%" -B build-zlib -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" -DZLIB_BUILD_TESTING=OFF -A x64
if errorlevel 1 goto :fail
cmake --build build-zlib --config Release
if errorlevel 1 goto :fail
cmake --install build-zlib --config Release
if errorlevel 1 goto :fail

:: --- Build libgit2 ---
echo.
echo === Building libgit2 %LIBGIT2_VERSION% ===
curl -fL --retry 3 --output libgit2.zip "https://github.com/libgit2/libgit2/archive/refs/tags/v%LIBGIT2_VERSION%.zip"
if errorlevel 1 goto :fail

:: The source archive contains test fixtures with symlinks and non-portable names
:: that tar cannot create on Windows. They are only needed by libgit2's own test
:: suite, which we do not build, so skip the whole tests directory.
:: tar still reports those entries as errors, so the result is verified below
:: instead of relying on its exit code.
tar -xf libgit2.zip --exclude "*tests/*"
del /f /q libgit2.zip

if not exist "%LIBGIT2_SRC%\CMakeLists.txt" goto :fail_libgit2_sources
if not exist "%LIBGIT2_SRC%\src\CMakeLists.txt" goto :fail_libgit2_sources

cmake -S "%LIBGIT2_SRC%" -B build-libgit2 ^
    -DBUILD_TESTS=OFF ^
    -DBUILD_CLI=OFF ^
    -DBUILD_EXAMPLES=OFF ^
    -DUSE_BUNDLED_ZLIB=ON ^
    -DUSE_SSH=OFF ^
    -DLIBGIT2_FILENAME=git2 ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -A x64
if errorlevel 1 goto :fail
cmake --build build-libgit2 --config Release
if errorlevel 1 goto :fail
cmake --install build-libgit2 --config Release
if errorlevel 1 goto :fail

:: --- Copy to project include, bin & lib dirs ---
echo.
echo === Copying dependencies to project directories ===
if not exist "..\bin" mkdir "..\bin"
if not exist "..\lib" mkdir "..\lib"
if not exist "..\include" mkdir "..\include"

:: Remove artifacts of previous dependency versions. zlib changed its MSVC output
:: names in 1.4 (z/zs -> libz/libzs) and stale import libraries would otherwise be
:: picked up instead of the freshly built ones.
:: Import libraries never belong into bin, so drop anything left there by
:: earlier versions of this script.
if exist "..\bin\*.lib" del /f /q "..\bin\*.lib"
for %%f in (z.dll libz.dll z.lib zs.lib zd.lib zsd.lib libz.lib libzs.lib) do (
    if exist "..\bin\%%f" del /f /q "..\bin\%%f"
    if exist "..\lib\%%f" del /f /q "..\lib\%%f"
)

xcopy /y /e /i /q "%INSTALL_DIR%\include\*" "..\include\"
if errorlevel 1 goto :fail
copy /y "%INSTALL_DIR%\bin\*.dll" "..\bin\" >nul
if errorlevel 1 goto :fail
copy /y "%INSTALL_DIR%\lib\*.lib" "..\lib\" >nul
if errorlevel 1 goto :fail

:: --- Cleanup ---
call :clean_artifacts

popd
echo.
echo Dependencies built successfully.
exit /b 0

:fail_libgit2_sources
echo [ERROR] libgit2 sources were not extracted correctly into "%LIBGIT2_SRC%".
goto :fail

:fail
echo.
echo [ERROR] Dependency build failed.
popd
exit /b 1

:: ---------------------------------------------------------------
:: Subroutines
:: ---------------------------------------------------------------

:check_tools
for %%t in (cmake curl tar) do (
    where %%t >nul 2>&1
    if errorlevel 1 (
        echo [ERROR] Required tool not found in PATH: %%t
        exit /b 1
    )
)
exit /b 0

:setup_msvc
if defined VCINSTALLDIR exit /b 0

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :msvc_fallback

"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath > "%TEMP%\gitmaster_vsdir.txt" 2>nul
set "VSDIR="
set /p VSDIR=<"%TEMP%\gitmaster_vsdir.txt"
del /f /q "%TEMP%\gitmaster_vsdir.txt" >nul 2>&1
if not defined VSDIR goto :msvc_fallback
if not exist "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" goto :msvc_fallback

call "%VSDIR%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
exit /b 0

:msvc_fallback
for %%d in (
    "C:\Program Files\Microsoft Visual Studio\18\Enterprise"
    "C:\Program Files\Microsoft Visual Studio\18\Professional"
    "C:\Program Files\Microsoft Visual Studio\18\Community"
    "C:\Program Files\Microsoft Visual Studio\18\BuildTools"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional"
    "C:\Program Files\Microsoft Visual Studio\2022\Community"
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
    "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools"
) do (
    if exist "%%~d\VC\Auxiliary\Build\vcvarsall.bat" (
        call "%%~d\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
        exit /b 0
    )
)

echo [ERROR] Visual Studio with C++ build tools was not found.
echo         Install the "Desktop development with C++" workload.
exit /b 1

:clean_artifacts
if exist zlib rmdir /s /q zlib
if exist "%ZLIB_SRC%" rmdir /s /q "%ZLIB_SRC%"
if exist build-zlib rmdir /s /q build-zlib
if exist libgit2 rmdir /s /q libgit2
if exist "%LIBGIT2_SRC%" rmdir /s /q "%LIBGIT2_SRC%"
if exist build-libgit2 rmdir /s /q build-libgit2
if exist install rmdir /s /q install
del /f /q zlib.zip >nul 2>&1
del /f /q libgit2.zip >nul 2>&1
exit /b 0
