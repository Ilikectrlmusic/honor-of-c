@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
gcc -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation src/game.c tests/simulation.c -o build/simulation.exe || exit /b 1
build\simulation.exe || exit /b 1
call build.bat || exit /b 1
start "" /wait Riftward.exe --gallery build\gallery
if errorlevel 1 exit /b 1
echo Direct2D gallery and UI checks passed.
