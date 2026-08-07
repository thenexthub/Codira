@echo off
REM Build the Codira compiler (codira.exe) from source on Windows.
REM
REM Requires: a Rust toolchain (cargo/rustc) and an LLVM 22.x dev package
REM (headers + static libs for the target backends you need, e.g. X86/AArch64).
REM
REM Configure via environment variables before running, or let this script
REM fall back to the defaults used to build this project:
REM   LLVM_SYS_220_PREFIX         Path to the LLVM 22 install (headers/lib/bin).
REM                               Default: C:\llvm22\built
REM   CODIRA_LLD_COFF             Path to a standalone lld-link.exe.
REM                               Default: C:\Program Files\LLVM\bin\lld-link.exe
REM   BUILD_PROFILE               "release" (default) or "dev".

setlocal enabledelayedexpansion

if "%LLVM_SYS_220_PREFIX%"=="" set "LLVM_SYS_220_PREFIX=C:\llvm22\built"
if not exist "%LLVM_SYS_220_PREFIX%\bin\llvm-config.exe" (
    echo ERROR: no LLVM 22 install found at "%LLVM_SYS_220_PREFIX%".
    echo Set LLVM_SYS_220_PREFIX to a directory containing an LLVM 22.x dev package.
    exit /b 1
)

if "%CODIRA_LLD_COFF%"=="" (
    where lld-link.exe >nul 2>nul
    if errorlevel 1 (
        if exist "C:\Program Files\LLVM\bin\lld-link.exe" (
            set "CODIRA_LLD_COFF=C:\Program Files\LLVM\bin\lld-link.exe"
        ) else (
            echo WARNING: lld-link.exe not found on PATH and CODIRA_LLD_COFF is unset.
            echo Linking codira-compiled modules will fail until one is available.
        )
    )
)

if "%BUILD_PROFILE%"=="" set "BUILD_PROFILE=release"

echo Building codira with LLVM_SYS_220_PREFIX=%LLVM_SYS_220_PREFIX%
if not "%CODIRA_LLD_COFF%"=="" echo Using CODIRA_LLD_COFF=%CODIRA_LLD_COFF%

if "%BUILD_PROFILE%"=="release" (
    cargo build --release -p codira
    set "OUT_DIR=release"
) else (
    cargo build -p codira
    set "OUT_DIR=debug"
)
if errorlevel 1 (
    echo === BUILD FAILED ===
    exit /b 1
)

echo === DONE ===
echo Binary at: target\%OUT_DIR%\codira.exe
endlocal