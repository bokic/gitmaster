@echo off

for /f "delims=" %%i in ('git rev-parse --short HEAD') do set git_hash=%%i
for /f "delims=" %%i in ('git describe --tags --dirty') do set git_describe=%%i

>installer.ini (
	echo [installer]
	echo target_name = "gitmaster_%git_describe%"
	echo description = "Git GUI client(git hash #%git_hash%)"
)

"C:\Program Files (x86)\Inno Setup 5\iscc" "gitmaster.iss"

del installer.ini
