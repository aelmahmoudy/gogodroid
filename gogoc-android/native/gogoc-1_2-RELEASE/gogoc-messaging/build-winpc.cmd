@ECHO OFF
REM *************************************************************************
REM * $Id: build-winpc.cmd,v 1.1 2009/11/20 16:34:51 jasminko Exp $
REM *
REM * Batch file used to build the gogoCLIENT Messaging Subsystem.
REM *
REM * Prerequisites:
REM *  - Visual Studio 2005 (SP1)
REM *  - Windows Vista RTM SDK  (Integrated in VS2005SP1)
REM *  - DEVENV executable in %PATH%
REM *
REM * Usage:
REM *   build-gogoc [/Release /Debug] [/Win32 /x64]
REM *
REM *************************************************************************


REM Defaults:
SET CONFIGURATION=Release
SET PLATFORM=Win32
SET COMMAND=Build


REM Overrides:
:ParseArgs
IF /I "%~1" == "/?"             GOTO Usage
IF /I "%~1" == "-h"             GOTO Usage
IF /I "%~1" == "/Release"       SET CONFIGURATION=Release& SHIFT& GOTO ParseArgs
IF /I "%~1" == "/Debug"         SET CONFIGURATION=Debug& SHIFT& GOTO ParseArgs
IF /I "%~1" == "/Win32"         SET PLATFORM=Win32& SHIFT& GOTO ParseArgs
IF /I "%~1" == "/x64"           SET PLATFORM=x64& SHIFT& GOTO ParseArgs
IF /I "%~1" == "/Clean"         SET COMMAND=Clean& SHIFT& GOTO ParseArgs
IF    "%~1" EQU ""              GOTO Done_Args
ECHO Unknown command-line switch: %~1
GOTO Usage
:Done_Args


REM Build the target
ECHO Launching build of gogoc-messaging solution...
ECHO Configuration: COMMAND=%COMMAND% PLATFORM=%PLATFORM% CONFIGURATION=%CONFIGURATION%
ECHO.
DEVENV winbuild\winpc\gogoc-messaging.sln /%COMMAND% "%CONFIGURATION%|%PLATFORM%"
IF %ERRORLEVEL% == 0 GOTO build_ok
GOTO build_error


:Usage
ECHO.
ECHO Usage:
ECHO   %0 ^[^/Release^|^/Debug^] ^[^/Win32^|^/x64^] ^[^/Clean^]
ECHO.
ECHO   Defaults: ^/Release ^/Win32
ECHO.
GOTO the_end


:build_ok
ECHO.
ECHO Build of gogoc-messaging solution completed successfully.
GOTO the_end

:build_error
ECHO.
ECHO Build of gogoc-messaging solution FAILED!

:the_end

