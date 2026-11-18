@echo off
setlocal
cd /d "%~dp0"
where gcc >nul 2>nul || (echo GCC not found. Add the MSYS2 UCRT64 bin directory to PATH. & exit /b 1)
if not exist build mkdir build
windres -I . src/resources.rc -O coff -o build/resources.o || exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -DUNICODE -D_UNICODE -municode -mwindows -static -s src/main.c src/ui.c src/game.c src/draw.c src/audio.c build/resources.o -o Riftward.exe -ld2d1 -ldwrite -lwindowscodecs -luuid -lole32 -lwinmm -lgdi32 -luser32 -lshell32 -limm32 || exit /b 1
echo Built %CD%\Riftward.exe
