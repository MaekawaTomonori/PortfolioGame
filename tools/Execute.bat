@echo off
chcp 65001 >nul
echo Execute.bat - Building and running GameTemplate
echo.

REM Step 1: Run AssetsCopy.bat
echo [1/3] Running AssetsCopy.bat to copy assets...
call "%~dp0AssetsCopy.bat"
if %errorlevel% neq 0 (
    echo Error: AssetsCopy.bat failed
    pause
    exit /b 1
)

REM Step 2: Build GameTemplate in Debug mode
echo.
echo [2/3] Building GameTemplate in Debug mode...
cd /d "%~dp0.."
msbuild GameTemplate.sln /p:Configuration=Debug /p:Platform=x64 /m
if %errorlevel% neq 0 (
    echo Error: Build failed
    pause
    exit /b 1
)

REM Step 3: Launch the generated exe
echo.
echo [3/3] Launching Game.exe...
if exist ".\out\x64\Debug\Game\Game.exe" (
    start "" ".\out\x64\Debug\Game\Game.exe"
    echo Game launched successfully!
) else (
    echo Error: Game.exe not found at .\out\x64\Debug\Game\Game.exe
    pause
    exit /b 1
)

pause