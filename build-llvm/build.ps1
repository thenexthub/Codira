# Builds a self-contained LLVM 22.1 + MLIR + LLD installation that Codira's
# codegen links against (via LLVM_SYS_221_PREFIX / llvm-sys 221).
#
# Mirror of the old C:\llvm14\build_llvm.bat recipe, versioned up to LLVM 22.1
# and extended to also build MLIR (for codira_mlir) and LLD (bundled lld-link /
# ld.lld so codira_linker does not need an external LLD install).
#
# Usage:
#   pwsh build-llvm/build.ps1 -InstallDir C:\llvm22\built [-Jobs 23] [-Skip]
#
# Produces: $InstallDir\bin\llvm-config.exe, static libs, headers, tools,
# and the MLIR/LLD binaries. Export LLVM_SYS_221_PREFIX=$InstallDir before
# building Codira (scripts/install-llvm.sh / program.bat / program.sh do this).

param(
    [string]$InstallDir = "C:\llvm22\built",
    [string]$Workspace = "C:\llvm22",
    [int]$Jobs = 23,
    [string]$Tag = "llvmorg-22.1.8",
    [string]$Ref = "22.1.8",
    [switch]$Skip
)

$ErrorActionPreference = "Stop"
$log = "$Workspace\build.log"

function Write-Log($msg) {
    $line = "[$(Get-Date -Format o)] $msg"
    Write-Host $line
    Add-Content -Path $log -Value $line
}

function Find-VsBuildTools {
    $roots = @(
        "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools",
        "C:\Program Files\Microsoft Visual Studio\18\BuildTools",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
        "C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
    )
    foreach ($r in $roots) {
        $vcvars = "$r\VC\Auxiliary\Build\vcvarsall.bat"
        if (Test-Path $vcvars) { return $r, $vcvars }
    }
    throw "Visual Studio Build Tools not found (checked $($roots -join ', '))"
}

New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
New-Item -ItemType Directory -Force -Path $Workspace | Out-Null

$vsRoot, $vcvars = Find-VsBuildTools
$cmakeBin = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
$ninjaBin = "$vsRoot\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"
if (-not (Test-Path "$cmakeBin\cmake.exe")) { throw "cmake.exe not found under $cmakeBin" }
if (-not (Test-Path "$ninjaBin\ninja.exe")) { throw "ninja.exe not found under $ninjaBin" }

$srcDir = "$Workspace\src\llvm-$Ref"
$buildDir = "$Workspace\build"
$archive = "$Workspace\llvm-$Ref.tar.gz"

Write-Log "LLVM bundle for $Tag"
Write-Log "InstallDir=$InstallDir Workspace=$Workspace Jobs=$Jobs"
Write-Log "VS BuildTools: $vsRoot"

$envSetup = "call `"$vcvars`" x64"

if (-not $Skip) {
    if (-not (Test-Path "$srcDir\llvm\CMakeLists.txt")) {
        Write-Log "Fetching llvm-project $Tag ..."
        if (-not (Test-Path $archive)) {
            Invoke-WebRequest -Uri "https://codeload.github.com/llvm/llvm-project/tar.gz/refs/tags/$Tag" -OutFile $archive
        }
        Write-Log "Extracting llvm-project ..."
        New-Item -ItemType Directory -Force -Path "$Workspace\src" | Out-Null
        tar -xf $archive -C "$Workspace\src"
        if (-not (Test-Path "$srcDir\llvm\CMakeLists.txt")) {
            throw "expected llvm-project checkout at $srcDir (extraction layout changed?)"
        }
    } else {
        Write-Log "llvm-project already present at $srcDir"
    }

    if (-not (Test-Path "$buildDir\CMakeCache.txt")) {
        Write-Log "Configuring with Ninja (LLVM, X86+AArch64, MLIR, LLD) ..."
        $cfg = "cmake -G Ninja -S `"$srcDir\llvm`" -B `"$buildDir`" -DCMAKE_BUILD_TYPE=Release -D`"LLVM_ENABLE_PROJECTS=mlir;lld`" -D`"LLVM_TARGETS_TO_BUILD=X86;AArch64`" -DLLVM_USE_CRT_RELEASE=MD -DCMAKE_INSTALL_PREFIX=`"$InstallDir`" -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_DOCS=OFF -DLLVM_ENABLE_ZLIB=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_LIBXML2=OFF -DLLVM_ENABLE_ASSERTIONS=OFF -DLLVM_APPEND_VC_REV=OFF -DLLVM_BUILD_TOOLS=ON -DLLVM_ENABLE_DIA_SDK=OFF"
        cmd /c "$envSetup && set `"PATH=$cmakeBin;$ninjaBin;%PATH%`" && $cfg"
        if ($LASTEXITCODE -ne 0) { throw "cmake configure failed (see $log)" }
        Write-Log "Configured OK"
    } else {
        Write-Log "Build dir already configured; reusing $buildDir"
    }

    Write-Log "Building + installing (this is the long part) ..."
    cmd /c "$envSetup && set `"PATH=$cmakeBin;$ninjaBin;%PATH%`" && cmake --build `"$buildDir`" --target install -- -j $Jobs"
    if ($LASTEXITCODE -ne 0) { throw "LLVM build/install failed (see $log)" }
} else {
    Write-Log "Skip switch set; using existing $InstallDir"
}

if (-not (Test-Path "$InstallDir\bin\llvm-config.exe")) { throw "no llvm-config.exe in $InstallDir" }
if (-not (Test-Path "$InstallDir\bin\lld-link.exe")) { throw "no lld-link.exe in $InstallDir (did MLIR/LLD project not install?)" }

Write-Log "DONE. Bundle ready at $InstallDir"
Write-Log "For Codira set: LLVM_SYS_221_PREFIX=$InstallDir CODIRA_LLD_COFF=$InstallDir\bin\lld-link.exe"