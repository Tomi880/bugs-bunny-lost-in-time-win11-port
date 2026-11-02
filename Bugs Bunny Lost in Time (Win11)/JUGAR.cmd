@echo off
setlocal
rem El juego exige esta clave (la escribe el instalador original).
set "GAME=%~dp0"
set "GAME=%GAME:~0,-1%"
reg add "HKCU\Software\Infogrames\Bugs Bunny Lost In Time" /v "installation path" /t REG_SZ /d "%GAME%" /f >nul
cd /d "%~dp0BIN"
start "" "%~dp0BIN\bugs.exe" /opengl /win /b0 /x800 /y600
