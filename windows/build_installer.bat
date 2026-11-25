@echo off

for /f "delims=" %%i in ('git rev-parse --short HEAD') do set git_hash=%%i
for /f "delims=" %%i in ('git describe --tags --dirty') do set git_describe=%%i

IF EXIST "C:\Qt\6.12.0" (set QT_PATH=C:\Qt\6.12.0) ELSE (set QT_PATH=C:\Qt\6.10.0)

set OPENSSL_PATH=C:\Qt\Tools\OpenSSLv3\Win_x64
set QT_KIT=msvc2022_64
set ARCH=x64

>installer.ini (
	echo [installer]
	echo target_name = "GitMaster_%git_describe%-%ARCH%"
	echo description = "Git GUI client(#%git_hash%)"
)

"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "gitmaster.iss"

del installer.ini
