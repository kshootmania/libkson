@echo off
setlocal enabledelayedexpansion

set VERSION=0.1.0
set OUTPUT_FILE=%~1

REM Try to get git hash
set GIT_HASH=unknown
where git >nul 2>&1
if !errorlevel! equ 0 (
    for /f "delims=" %%i in ('git rev-parse --short HEAD 2^>nul') do set GIT_HASH=%%i
)

REM Set version full
if "!GIT_HASH!"=="unknown" (
    set VERSION_FULL=!VERSION!
) else (
    set VERSION_FULL=!VERSION!+!GIT_HASH!
)

REM Generate header file
(
echo #pragma once
echo.
echo constexpr const char* kKsh2KsonAppName = "ksh2kson";
echo constexpr const char* kKsh2KsonVersion = "!VERSION!";
echo constexpr const char* kKsh2KsonVersionFull = "!VERSION_FULL!";
echo constexpr const char* kKsh2KsonGitHash = "!GIT_HASH!";
) > "!OUTPUT_FILE!"

exit /b 0
