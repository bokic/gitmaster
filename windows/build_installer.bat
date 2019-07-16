@echo off

for /f "delims=" %%i in ('git rev-parse --short HEAD') do set git_hash=%%i
for /f "delims=" %%i in ('git describe --tags --dirty') do set git_describe=%%i

>installer.ini (
	echo [installer]
	echo target_name = "gitmaster_%git_describe%"
	echo description = "Git GUI client(git hash #%git_hash%)"
)

IF EXIST "C:\Qt\5.13.0" (set QT_PATH=C:\Qt\5.13.0) else (set QT_PATH=C:\Qt\5.12)
IF EXIST "C:\Qt\Tools\OpenSSL\Win_x86\bin" (set OPENSSL_PATH=C:\Qt\Tools\OpenSSL\Win_x86) else (set OPENSSL_PATH=C:\OpenSSL-v111-Win32)
set QT_KIT=mingw73_32

"C:\Program Files (x86)\Inno Setup 5\iscc" "gitmaster.iss"

del installer.ini
