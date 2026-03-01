@REM Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
@REM This source file is part of the Codira project, licensed under Apache-2.0
@REM with Runtime Library Exception.
@REM
@REM See https://codira-lang.cn/pages/LICENSE for license information.

@REM This script needs to be placed in the root directory of installation of Codira compiler and libraries.

@echo off
REM Set CODIRA_HOME to the path of this batch script.
set "CODIRA_HOME=%~dp0"

REM Windows searches for both binaries and libs in %Path%
set "PATH=%CODIRA_HOME%runtime\lib\windows_x86_64_codenative;%CODIRA_HOME%bin;%CODIRA_HOME%tools\bin;%CODIRA_HOME%tools\lib;%PATH%;%USERPROFILE%\.codepm\bin"
