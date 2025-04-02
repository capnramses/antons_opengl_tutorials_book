@echo off
REM Build File for MSVC (Visual Studio).

REM This clause is for the global "build_all" batch file, so that you don't need to set the VS edition path in every file first.
IF NOT "%~1"=="" (
  echo "Using supplied vcvars:" %1
  if not defined DevEnvDir ( call %1 )
  GOTO setpaths
)

REM Uncomment whichever one that you have installed:
REM See https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170#developer_command_file_locations
REM call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 13.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64
REM call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:setpaths

set LFLAGS=/DEBUG /MACHINE:X64
set INCLUDES=/I "..\third_party\glew-2.1.0\include" /I "..\third_party\glfw-3.4.bin.WIN64\include" /I "..\third_party\assimp\include"
set LIB_PATH_GLFW=/LIBPATH:"..\third_party\glfw-3.4.bin.WIN64\lib-vc2022"
set LIB_PATH_GLEW=/LIBPATH:"..\third_party\glew-2.1.0\lib\Release\x64"
set LIB_PATH_ASSIMP=/LIBPATH:"..\third_party\assimp\lib\"
set SYSTEM_LIBS="kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"
set LIBS=%LIB_PATH_GLFW% %LIB_PATH_GLEW% glew32.lib glfw3dll.lib OpenGL32.lib %SYSTEM_LIBS%
set DLL_PATH_GLEW="third_party\glew-2.1.0\bin\Release\x64\glew32.dll"
set DLL_PATH_GLFW="third_party\glfw-3.4.bin.WIN64\lib-vc2019\glfw3.dll"
set DLL_PATH_ASSIMP="third_party\assimp\bin\vs2022\assimp-vc143-mt.dll"
set SRC="*.c??"

@echo on

cl %CFLAGS% %SRC% %INCLUDES% /link %LFLAGS% %LIBS% /OUT:"vcam.exe" 

copy ..\%DLL_PATH_GLEW% .\
copy ..\%DLL_PATH_GLFW% .\
