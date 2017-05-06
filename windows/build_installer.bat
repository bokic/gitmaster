@echo off

for /f "delims=" %%i in ('git rev-parse --short HEAD') do set git_hash=%%i

>installer.ini (
	echo [installer]
	echo description = "Git GUI client(git hash #%git_hash%)"
)

"C:\Program Files (x86)\Inno Setup 5\iscc" "gitmaster.iss"

del installer.ini
