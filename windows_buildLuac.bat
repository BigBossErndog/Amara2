@echo off
setlocal enabledelayedexpansion

set "INPUT_DIR=.\lua"
set "OUTPUT_DIR=.\build\lua"

REM Ensure output directory exists
if not exist "!OUTPUT_DIR!" (
    mkdir "!OUTPUT_DIR!"
)

REM Recursively compile all Lua files, preserving directory structure
for /r "%INPUT_DIR%" %%F in (*.lua) do (
    set "input=%%F"
    set "relativePath=%%F"
    set "relativePath=!relativePath:%INPUT_DIR%=!"
    set "output=!OUTPUT_DIR!!relativePath:.lua=.luac!"

    REM Get the folder part of the output file and ensure it exists
    for %%D in ("!output!") do set "outputFolder=%%~dpD"
    if not exist "!outputFolder!" mkdir "!outputFolder!"

    echo Compiling !input! to !output!
    luac -o "!output!" "!input!"

    if errorlevel 1 (
        echo Failed to compile !input!
    ) else (
        echo Successfully compiled !input!
    )
)

endlocal
pause