@REM Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
@REM This source file is part of the Codira project, licensed under Apache-2.0
@REM with Runtime Library Exception.
@REM
@REM See https://cangjie-lang.cn/pages/LICENSE for license information.

@REM Windows build script
@echo off

set platform=windows_x86_64
set CURRENT_PATH="%CD%"
set PROJECT_PATH="%~dp0\..\"
set CODETHREAD_PATH="%~dp0\..\src\CODEThread"
set TESTCODE_PATH="%~dp0\..\test_tools\tests\codethread_test"
set BUILD_PATH=%PROJECT_PATH%\build\codethread_build

@REM 检查路径是否包含空格
if %PROJECT_PATH% neq %PROJECT_PATH: =% (
    echo The path cannot contain spaces.
    exit /b 1
)

if "%1" == "-s" (
    cd %TESTCODE_PATH%\codethread_sdv\src
    call build_test_windows.bat %*
) else if "%1" == "-h" (
    cd %TESTCODE_PATH%\codethread_sdv\src
    call build_test_windows.bat %*
) else if "%1" == "set_env" (
    call %PROJECT_PATH%\scripts\environment_variables_setting.bat
    echo set environment success
) else if "%1" == "clean" (
    call %PROJECT_PATH%\build\scripts\clean_history.bat
) else if "%1" == "-p" (
    if exist %BUILD_PATH% (
        rd  /S /Q %BUILD_PATH%
    ) 
    if not exist %BUILD_PATH% (
        md %BUILD_PATH%
    )

    if exist %PROJECT_PATH%\output (
        rd  /S /Q %PROJECT_PATH%\output\
    ) 
    if not exist %PROJECT_PATH%\output (
        md %PROJECT_PATH%\output
    )

    cd %BUILD_PATH%\

    if "%2" == "windows_x86_64" (
        cmake -DTARGET="%2" -DCMAKE_BUILD_TYPE="%3" -DLIBTYPE="%4" -DBUILDING_STAGE="%5" %6 -DCMAKE_C_COMPILER_TARGET=x86_64-windows-gnu -DCMAKE_CXX_COMPILER_TARGET=x86_64-windows-gnu %CODETHREAD_PATH% -G "MinGW Makefiles"
    ) else (
        cmake -DTARGET="%2" -DCMAKE_BUILD_TYPE="%3" -DLIBTYPE="%4" -DBUILDING_STAGE="%5" %6 %CODETHREAD_PATH% -G "MinGW Makefiles"
    )

    mingw32-make
)

cd %CURRENT_PATH%
exit /b 0

@echo on
