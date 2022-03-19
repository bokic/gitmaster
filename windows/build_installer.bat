@echo off

for /f "delims=" %%i in ('git rev-parse --short HEAD') do set git_hash=%%i
for /f "delims=" %%i in ('git describe --tags --dirty') do set git_describe=%%i

>installer.ini (
	echo [installer]
	echo target_name = "gitmaster_%git_describe%"
	echo description = "Git GUI client(git hash #%git_hash%)"
)

IF EXIST "C:\Qt\6.2.3" (set QT_PATH=C:\Qt\6.2.3) ELSE (set QT_PATH=C:\Qt\6.2.2)
set OPENSSL_PATH=C:\Qt\Tools\OpenSSL\Win_x64
set QT_KIT=mingw_64

"C:\Program Files (x86)\Inno Setup 5\iscc" "gitmaster.iss"

del installer.ini
