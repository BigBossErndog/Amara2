@echo off
setlocal EnableExtensions

rem ============================================================
rem  Amara2 - MSVC build script (cl.exe / rc.exe / link.exe)
rem  Usage: build.bat [win | test | release | play | icon | dll | dirs | clean]
rem ============================================================

rem ---------- Project config ----------
set "ENTRY_FILES=amara2\main\main.cpp"
set "BUILD_NAME=Amara2"
set "BUILD_PATH=build"
set "EXE=%BUILD_PATH%\%BUILD_NAME%.exe"
set "OBJ_PATH=%BUILD_PATH%\obj"
set "EXE_OPTIONS=-context ../"

set "RESOURCES=resources"
set "SDL_PATH=%RESOURCES%\libs\SDL3-3.4.10"

set "ICON_SRC=assets/icons/icon.ico"
set "ICON_RC=assets\icons\icon.rc"
set "ICON_RES=assets\icons\icon.res"

rem Set to 1 if your entry point is plain main() instead of WinMain/SDL_main
@REM  set "USE_MAIN_ENTRY=1"

rem ---------- Defines ----------
set "DEFINES=/DAMARA_DEBUGGING /DAMARA_PLUGINS /DAMARA_ENGINE_TOOLS /DAMARA_OPENGL"

rem ---------- Include paths ----------
set "INCLUDES=/Iamara2 /Iplugins /Isrc /I%SDL_PATH%\include"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\json\include"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\lua"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\sol2"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\stb"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\glm"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\minimp3"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\portable-file-dialogs"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\tinyxml2"
set "INCLUDES=%INCLUDES% /I%RESOURCES%\libs\miniz-3.1.1"

rem ---------- Extra sources (C files compile as C automatically) ----------
set "EXTRA_SOURCES=%RESOURCES%\libs\miniz-3.1.1\miniz.c"

rem ---------- Compiler flags ----------
rem /w = no warnings (like -w), /MT = static CRT (like -static), /O2 = optimize
set "CL_FLAGS=/nologo /std:c++17 /EHsc /MT /O2 /w /bigobj /utf-8 /Zc:__cplusplus"

rem ---------- Linker flags ----------
set "SYSTEM_LIBS=opengl32.lib shell32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib version.lib advapi32.lib setupapi.lib"
set "LINK_FLAGS=/SUBSYSTEM:WINDOWS /NOIMPLIB /LIBPATH:%SDL_PATH%\lib\x64 SDL3.lib %SYSTEM_LIBS%"
if "%USE_MAIN_ENTRY%"=="1" set "LINK_FLAGS=%LINK_FLAGS% /ENTRY:mainCRTStartup"

rem ---------- Dispatch ----------
set "TARGET=%~1"
if "%TARGET%"=="" set "TARGET=release"

if /i "%TARGET%"=="win"     goto :target_win
if /i "%TARGET%"=="test"    goto :target_test
if /i "%TARGET%"=="release" goto :target_release
if /i "%TARGET%"=="play"    goto :target_play
if /i "%TARGET%"=="icon"    goto :target_icon
if /i "%TARGET%"=="dll"     goto :target_dll
if /i "%TARGET%"=="dirs"    goto :target_dirs
if /i "%TARGET%"=="clean"   goto :target_clean

echo Unknown target: %TARGET%
echo Usage: build.bat [win ^| test ^| release ^| play ^| icon ^| dll ^| dirs ^| clean]
exit /b 1

rem ============================================================
:target_win
call :setup_msvc || exit /b 1
call :build_icon || exit /b 1
call :compile    || exit /b 1
call :copy_dlls  || exit /b 1
exit /b 0

:target_test
call :setup_msvc || exit /b 1
call :build_icon || exit /b 1
call :compile    || exit /b 1
call :copy_dlls  || exit /b 1
call :copy_dirs  || exit /b 1
call :copy_engine || exit /b 1
exit /b 0

:target_release
if exist "%BUILD_PATH%" rmdir /s /q "%BUILD_PATH%"
md "%BUILD_PATH%"
call :setup_msvc || exit /b 1
call :build_icon || exit /b 1
call :compile    || exit /b 1
call :copy_dlls  || exit /b 1
call :copy_dirs  || exit /b 1
call :copy_engine || exit /b 1
if exist "%BUILD_PATH%\files\settings.json" del /f /q "%BUILD_PATH%\files\settings.json"
exit /b 0

:target_play
"%EXE%" %EXE_OPTIONS%
exit /b %errorlevel%

:target_icon
call :setup_msvc || exit /b 1
call :build_icon
exit /b %errorlevel%

:target_dll
call :copy_dlls
exit /b %errorlevel%

:target_dirs
call :copy_dirs
exit /b %errorlevel%

:target_clean
if exist "%BUILD_PATH%" rmdir /s /q "%BUILD_PATH%"
if exist "%ICON_RES%" del /f /q "%ICON_RES%"
if exist "%ICON_RC%" del /f /q "%ICON_RC%"
exit /b 0

rem ============================================================
rem  Subroutines
rem ============================================================

:setup_msvc
rem Already inside a Developer Command Prompt?
where cl.exe >nul 2>nul && where rc.exe >nul 2>nul && exit /b 0

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :no_msvc

set "VSINSTALL="
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL goto :no_msvc
if not exist "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" goto :no_msvc

echo Initializing MSVC environment from: %VSINSTALL%
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 goto :no_msvc
exit /b 0

:no_msvc
echo ERROR: Could not find Visual Studio Build Tools with the C++ x64 toolset.
echo Install "Desktop development with C++" or run this from the
echo "x64 Native Tools Command Prompt".
exit /b 1

:build_icon
echo Building icon resource...
echo 1 ICON "%ICON_SRC%"> "%ICON_RC%"
rc.exe /nologo /fo "%ICON_RES%" "%ICON_RC%"
if errorlevel 1 (
    echo ERROR: rc.exe failed.
    exit /b 1
)
exit /b 0

:compile
if not exist "%BUILD_PATH%" md "%BUILD_PATH%"
if not exist "%OBJ_PATH%" md "%OBJ_PATH%"
echo Compiling %BUILD_NAME%...
cl.exe %CL_FLAGS% %DEFINES% %INCLUDES% %ENTRY_FILES% %EXTRA_SOURCES% ^
    /Fo"%OBJ_PATH%\\" ^
    /Fe"%EXE%" ^
    /link %LINK_FLAGS% "%ICON_RES%"
set "CL_RESULT=%errorlevel%"

rem Objects are only needed during the build
if exist "%OBJ_PATH%" rmdir /s /q "%OBJ_PATH%"

if not "%CL_RESULT%"=="0" (
    echo ERROR: build failed.
    exit /b 1
)
echo Build succeeded: %EXE%
exit /b 0

:copy_dlls
if not exist "%BUILD_PATH%" md "%BUILD_PATH%"
xcopy /s /e /i /y "%RESOURCES%\dlls\win64\*.*" "%BUILD_PATH%\" >nul
exit /b 0

:copy_dirs
if not exist "%BUILD_PATH%" md "%BUILD_PATH%"
if not exist "%BUILD_PATH%\assets" md "%BUILD_PATH%\assets"
if not exist "%BUILD_PATH%\files" md "%BUILD_PATH%\files"
if not exist "%BUILD_PATH%\lua_scripts" md "%BUILD_PATH%\lua_scripts"
xcopy /s /e /i /y "assets\*.*" "%BUILD_PATH%\assets" >nul
xcopy /s /e /i /y "files\*.*" "%BUILD_PATH%\files" >nul
xcopy /s /e /i /y "lua_scripts\*.*" "%BUILD_PATH%\lua_scripts" >nul
exit /b 0

:copy_engine
if not exist "%BUILD_PATH%\amara2" md "%BUILD_PATH%\amara2"
xcopy /s /e /i /y "amara2\*.*" "%BUILD_PATH%\amara2" >nul
exit /b 0