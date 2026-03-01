/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

/**
 * @file
 *
 * This file implements the MinGW_CODENATIVE ToolChain base class.
 */

#include "Toolchains/CODENATIVE/MinGW_CODENATIVE.h"

#include "Codira/Driver/TempFileManager.h"
#include "Codira/Driver/Utils.h"
#include "Codira/Utils/FileUtil.h"

using namespace Codira;
using namespace Codira::Triple;

void MinGW_CODENATIVE::InitializeLibraryPaths()
{
    // The paths from --toolchain/-B can be used to search system libs, such as Scrt1.o, crti.o, etc.
    for (auto& libPath : driverOptions.toolChainPaths) {
        AddCRuntimeLibraryPath(libPath);
    }
    InitializeMinGWSysroot();
    AddCRuntimeLibraryPaths();
    AddSystemLibraryPaths();
}

void MinGW_CODENATIVE::InitializeMinGWSysroot()
{
    if (driverOptions.customizedSysroot) {
        sysroot = driverOptions.sysroot;
    } else {
        auto gccPath = FileUtil::FindProgramByName("x86_64-w64-mingw32-gcc.exe", driverOptions.environment.paths);
        if (gccPath.empty()) {
            return;
        }
        sysroot = FileUtil::GetDirPath(FileUtil::GetDirPath(gccPath));
    }
}

void MinGW_CODENATIVE::AddCRuntimeLibraryPaths()
{
    // We search <mingw64-root>/lib path for gcc libraries.
    auto libPath = FileUtil::JoinPath(sysroot, "lib");
    AddCRuntimeLibraryPath(libPath);
    // Some libraries can be found in a compatible toolchain folder, we try to add such folder as well.
    auto directories = FileUtil::GetDirectories(libPath);
    for (const auto& dir : directories) {
        if (Triple::IsPossibleMatchingTripleName(driverOptions.target, dir.name)) {
            AddCRuntimeLibraryPath(dir.path);
        }
    }
}

void MinGW_CODENATIVE::AddSystemLibraryPaths()
{
    AddLibraryPaths(ComputeLibPaths());
    Gnu::AddSystemLibraryPaths();
}

std::vector<std::string> MinGW_CODENATIVE::ComputeLibPaths() const
{
    return {sysroot + "/lib", sysroot + "/x86_64-w64-mingw32/lib"};
}

std::string MinGW_CODENATIVE::GetCodeldScript(const std::unique_ptr<Tool>& tool) const
{
    std::string codeldScript = "codeld.lds";
    switch (driverOptions.outputMode) {
        case GlobalOptions::OutputMode::SHARED_LIB:
            tool->AppendArg("-shared");
            codeldScript = "codeld.shared.lds";
            [[fallthrough]];
        case GlobalOptions::OutputMode::STATIC_LIB:
        case GlobalOptions::OutputMode::EXECUTABLE:
        default:
            break;
    }
    return codeldScript;
}

std::string MinGW_CODENATIVE::GenerateGCCLibPath(
    [[maybe_unused]] const std::pair<std::string, std::string>& gccCrtFilePair) const
{
    return mingwLibPath;
}

void MinGW_CODENATIVE::GenerateArchiveTool(const std::vector<TempFileInfo>& objFiles)
{
    auto tool = std::make_unique<Tool>(arPath, ToolType::BACKEND, driverOptions.environment.allVariables);
    // llvm-ar in our package may be used for creating archive file, so LD_LIBRARY_PATH need to be set to llvm lib dir.
    if (driverOptions.IsCrossCompiling() && FileUtil::GetFileName(arPath).find("llvm-ar") != std::string::npos) {
        tool->SetLdLibraryPath(FileUtil::JoinPath(FileUtil::GetDirPath(arPath), "../lib"));
    }
    // c for no warn if the library had to be created
    // r for replacing existing or insert new file(s) into the archive
    // D for deterministic mode
    tool->AppendArg("crD");

    // When we reach here, we must be at the final phase of the compilation,
    // which means that is the final output.
    TempFileInfo fileInfo = TempFileManager::Instance().CreateNewFileInfo(objFiles[0], TempFileKind::O_STATICLIB);
    std::string outputFile = fileInfo.filePath;

    // If archive exists, ar attempts to insert given obj files into the archive.
    // We always try to remove the archive before creating a new one.
    (void)FileUtil::Remove(outputFile.c_str());
    tool->AppendArg(outputFile);

    // Note: We do not use tool->inputs here since it is always placed right after executable
    // the first arg of ar should be the option not input
    for (const auto& objFile : objFiles) {
        tool->AppendArg(objFile.filePath);
    }
    backendCmds.emplace_back(MakeSingleToolBatch({std::move(tool)}));
}

void MinGW_CODENATIVE::GenerateLinkingTool(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath,
    const std::pair<std::string, std::string>& gccCrtFilePair)
{
    auto tool = std::make_unique<Tool>(ldPath, ToolType::BACKEND, driverOptions.environment.allVariables);
    std::string outputFile = GetOutputFileInfo(objFiles).filePath;
    tool->AppendArg("-o", outputFile);

    tool->AppendArgIf(driverOptions.stripSymbolTable, "-s");

    // --nxcompat prevents executing codes from stack.
    tool->AppendArg("--nxcompat");
    // --dynamic-base makes that the base address of the program is randomly set
    // every time it is executed and loaded into memory.
    tool->AppendArg("--dynamicbase");
    // --high-entropy-va makes the executable image supports high-entropy 64-bit
    // address space layout randomization (ASLR), which means ASLR can use the entire 64-bit address space.
    tool->AppendArg("--high-entropy-va");

    tool->AppendArg("-m", GetEmulation());

    std::string codeldScript = GetCodeldScript(tool);
    tool->AppendArg("-Bdynamic");
    if (driverOptions.outputMode == GlobalOptions::OutputMode::SHARED_LIB) {
        tool->AppendArg("-e", "DllMainCRTStartup");
        tool->AppendArg("--export-all-symbols");
    }
    // Link order: crt2 -> crtbegin -> other input files -> crtend.
    if (driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE ||
        driverOptions.outputMode == GlobalOptions::OutputMode::SHARED_LIB) {
        auto crtObjName = driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE ? "crt2.o" : "dllcrt2.o";
        auto crtObjPath = mingwLibPath + crtObjName;
        // If we found crt2.o, we use the absolute path of system crt2.o, otherwise we let it simply be (dll)crt2.o,
        // we don't expect such cases though. Same as crtend.o.
        tool->AppendArg(crtObjPath);
    }
    tool->AppendArg(GetGccLibFile(gccCrtFilePair.first, gccLibPath));
    // Add crtfastmath.o if fast math is enabled and crtfastmath.o is found.
    if (driverOptions.fastMathMode) {
        auto crtfastmathFilepath = GetGccLibFile("crtfastmath.o", gccLibPath);
        tool->AppendArgIf(FileUtil::FileExist(crtfastmathFilepath), crtfastmathFilepath);
    }
    HandleLLVMLinkOptions(objFiles, gccLibPath, *tool, codeldScript);
    // extra ld options given by -ld-options
    tool->AppendArg(GetGccLibFile(gccCrtFilePair.second, gccLibPath));
    backendCmds.emplace_back(MakeSingleToolBatch({std::move(tool)}));
}

void MinGW_CODENATIVE::HandleLibrarySearchPaths(Tool& tool, const std::string& codiraLibPath)
{
    // Append -L (those specified in command) as search paths
    tool.AppendArg("-L" + mingwLibPath);
    Gnu::HandleLibrarySearchPaths(tool, codiraLibPath);
}

void MinGW_CODENATIVE::GenerateLinkOptions(Tool& tool)
{
    tool.AppendArg("-l:libcodira-runtime.dll");
    tool.AppendArg("-lclang_rt-builtins");
    tool.AppendArg(MINGW_CODENATIVE_LINK_OPTIONS);
}

bool MinGW_CODENATIVE::PrepareDependencyPath()
{
    if ((arPath = FindCodiraLLVMToolPath(g_toolList.at(ToolID::LLVM_AR).name)).empty()) {
        return false;
    }
    if ((ldPath = FindCodiraLLVMToolPath(g_toolList.at(ToolID::LLD).name)).empty()) {
        return false;
    }
    return true;
}

std::string MinGW_CODENATIVE::FindCodiraMinGWToolPath(const std::string toolName) const
{
    std::string toolPath = FindToolPath(
        toolName, std::vector<std::string>{FileUtil::JoinPath(driver.codiraHome, "third_party/mingw/bin/")});
    if (toolPath.empty()) {
        Errorf("not found `%s` in search paths. Your Codira installation might be broken.", toolName.c_str());
    }
    return toolPath;
}
