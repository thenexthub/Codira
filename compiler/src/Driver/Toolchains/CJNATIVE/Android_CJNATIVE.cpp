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
 * This file implements the Android_CODENATIVE ToolChain base class.
 */

#include "Toolchains/CODENATIVE/Android_CODENATIVE.h"

#include <string>
#include <vector>

#include "Codira/Driver/Toolchains/GCCPathScanner.h"
#include "Codira/Utils/FileUtil.h"

using namespace Codira;
using namespace Codira::Triple;

void Android_CODENATIVE::InitializeLibraryPaths()
{
    auto deduceToolchainLibPath = [this](const std::string& toolchainPath) {
        auto toolChainRoot = FileUtil::GetDirPath(toolchainPath);
        auto clangrtLibRoot = FileUtil::JoinPath(toolChainRoot, "lib64/clang");
        if (!FileUtil::FileExist(clangrtLibRoot)) {
            clangrtLibRoot = FileUtil::JoinPath(toolChainRoot, "lib/clang");
        }
        std::vector<FileUtil::Directory> subDirs = FileUtil::GetDirectories(clangrtLibRoot);
        GCCVersion selectedClangVersion{0, 0, 0};
        std::string selectedClangPath{};
        for (const auto& dir : subDirs) {
            std::optional<GCCVersion> gccVersion = GCCPathScanner::StrToGCCVersion(dir.name);
            if (gccVersion && selectedClangVersion < *gccVersion) {
                selectedClangPath = dir.path;
                selectedClangVersion = *gccVersion;
            }
        }
        if (!selectedClangPath.empty()) {
            AddLibraryPath(FileUtil::JoinPath(selectedClangPath, "lib/linux/aarch64"));
            AddLibraryPath(FileUtil::JoinPath(selectedClangPath, "lib/linux"));
        }
    };
    // 1. Deduce libs path from toolchain.
    // If the toolchain path is not specified by -B/--toolchain, deduce libs path from the environment path.
    auto clangPath = FileUtil::FindProgramByName(driverOptions.target.GetEffectiveTripleString() + "-clang",
        driverOptions.toolChainPaths.empty() ? driverOptions.environment.paths : driverOptions.toolChainPaths);
    if (!clangPath.empty()) {
        auto clangDir = FileUtil::GetDirPath(clangPath);
        deduceToolchainLibPath(clangDir);
        sysroot = FileUtil::JoinPath(FileUtil::GetDirPath(clangDir), "sysroot");
    }
    if (driverOptions.customizedSysroot) {
        sysroot = driverOptions.sysroot;
    }
    // 2. Deduce libs path from sysroot.
    if (!sysroot.empty()) {
        std::string tripleDirectory = "usr/lib/" + driverOptions.target.ArchToString() + "-linux-android/";
        AddLibraryPath(FileUtil::JoinPath(sysroot, tripleDirectory + driverOptions.target.apiLevel));
        AddLibraryPath(FileUtil::JoinPath(sysroot, tripleDirectory));
        AddLibraryPath(sysroot);
    }
    AddCRuntimeLibraryPaths();
}

void Android_CODENATIVE::AddCRuntimeLibraryPaths()
{
    std::string tripleDirectory = "usr/lib/" + driverOptions.target.ArchToString() + "-linux-android/";
    AddCRuntimeLibraryPath(FileUtil::JoinPath(sysroot, tripleDirectory + driverOptions.target.apiLevel));
}

bool Android_CODENATIVE::PrepareDependencyPath()
{
    if ((objcopyPath = FindCodiraLLVMToolPath(g_toolList.at(ToolID::LLVM_OBJCOPY).name)).empty()) {
        return false;
    }
    if ((arPath = FindUserToolPath(g_toolList.at(ToolID::LLVM_AR).name)).empty()) {
        return false;
    }
    if ((ldPath = FindCodiraLLVMToolPath(g_toolList.at(ToolID::LLD).name)).empty()) {
        return false;
    }
    return true;
}

bool Android_CODENATIVE::GenerateLinking(const std::vector<TempFileInfo>& objFiles)
{
    // Different linking mode requires different gcc crt files
    GenerateLinkingTool(objFiles, "", {});
    return true;
}

void Android_CODENATIVE::GenerateLinkingTool(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath,
    const std::pair<std::string, std::string>& /* gccCrtFilePair */)
{
    auto tool = std::make_unique<Tool>(ldPath, ToolType::BACKEND, driverOptions.environment.allVariables);
    tool->SetLdLibraryPath(FileUtil::JoinPath(FileUtil::GetDirPath(ldPath.c_str()), "../lib"));
    std::string outputFile = GetOutputFileInfo(objFiles).filePath;
    tool->AppendArg("-o", outputFile);
    if (driverOptions.IsLTOEnabled()) {
        GenerateLinkOptionsForLTO(*tool.get());
        // The -z notext option is the default for ld, while the -z noexecstack option is the default for lld.
        // Therefore, ld needs to explicitly pass -z noexecstack, and lld needs to explicitly pass -z notext.
        tool->AppendArg("-z", "notext");
    } else {
        tool->AppendArg("-z", "noexecstack");
    }
    tool->AppendArgIf(driverOptions.stripSymbolTable, "-s");

    // Hot reload relies on .gnu.hash section.
    tool->AppendArg("--hash-style=both");
    tool->AppendArg("-EL");
    tool->AppendArg("-m", GetEmulation());

    std::string codeldScript = GetCodeldScript(tool);
    if (driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE) {
        tool->AppendArg("-pie");
    }
    std::string crtBeginName =
        driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE ? "crtbegin_dynamic.o" : "crtbegin_so.o";
    auto maybeCrtBegin = FileUtil::FindFileByName(crtBeginName, GetCRuntimeLibraryPath());

    tool->AppendArg(maybeCrtBegin.value_or(crtBeginName));
    HandleLLVMLinkOptions(objFiles, gccLibPath, *tool, codeldScript);
    std::string crtEndName =
        driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE ? "crtend_android.o" : "crtend_so.o";
    auto maybeCrtEnd = FileUtil::FindFileByName(crtEndName, GetCRuntimeLibraryPath());
    tool->AppendArg(maybeCrtEnd.value_or(crtEndName));
    backendCmds.emplace_back(MakeSingleToolBatch({std::move(tool)}));
}

void Android_CODENATIVE::GenerateLinkOptions(Tool& tool)
{
    tool.AppendArg("-lclang_rt.builtins-" + driverOptions.target.ArchToString() + "-android");
    tool.AppendArg("-l:libcodira-runtime.so");
    tool.AppendArg(LINUX_CODENATIVE_LINK_OPTIONS);
    tool.AppendArg("-lclang_rt.builtins-" + driverOptions.target.ArchToString() + "-android");
    tool.AppendArg("-ldl");
    tool.AppendArg("-z", "max-page-size=4096");
}

void Android_CODENATIVE::HandleSanitizerDependencies(Tool& tool)
{
    for (const auto& arg : {"-lpthread", "-lrt", "-lm", "-ldl", "-lresolv"}) {
        tool.AppendArg(arg);
    }
}
