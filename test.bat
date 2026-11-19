@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
gcc -std=c11 -O2 -Wall -Wextra -Wno-misleading-indentation src/game.c tests/simulation.c -o build/simulation.exe || exit /b 1
build\simulation.exe || exit /b 1
gcc -std=c11 -O2 -Wall -Wextra src/game.c tests/motion.c -o build/motion-checks.exe || exit /b 1
build\motion-checks.exe || exit /b 1
gcc -std=c11 -O2 -Wall -Wextra src/game.c tests/feedback.c -o build/feedback-checks.exe || exit /b 1
build\feedback-checks.exe || exit /b 1
gcc -std=c11 -O2 -Wall -Wextra src/game.c tests/observers.c -o build/observer-checks.exe || exit /b 1
build\observer-checks.exe || exit /b 1
call build.bat || exit /b 1
gcc -std=c11 -O2 -Wall -Wextra -DUNICODE -D_UNICODE -DRIFT_CAPTURE tests/views.c src/ui.c src/presentation.c src/game.c src/draw.c src/living.c src/domain.c src/animation.c src/audio.c build/resources.o -o build/view-checks.exe -ld2d1 -ldwrite -lwindowscodecs -luuid -lole32 -lwinmm -lgdi32 -luser32 || exit /b 1
build\view-checks.exe || exit /b 1
start "" /wait Riftward.exe --audio-check
if errorlevel 1 exit /b 1
echo Embedded stereo score, voice bank and mixer checks passed.
start "" /wait Riftward.exe --gallery build\gallery
if errorlevel 1 exit /b 1
echo Direct2D gallery and UI checks passed.
