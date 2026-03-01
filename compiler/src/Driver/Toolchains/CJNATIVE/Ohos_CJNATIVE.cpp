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
 * This file implements the Ohos_CODENATIVE ToolChain base class.
 */

#include "Toolchains/CODENATIVE/Ohos_CODENATIVE.h"

using namespace Codira;
using namespace Codira::Triple;

void Ohos_CODENATIVE::AddCRuntimeLibraryPaths()
{
    for (const auto& path : driverOptions.toolChainPaths) {
        AddCRuntimeLibraryPath(path);
    }
}

bool Ohos_CODENATIVE::PrepareDependencyPath()
{
    if ((objcopyPath = FindCodiraLLVMToolPath(g_toolList.at(ToolID::LLVM_OBJCOPY).name)).empty()) {
        return false;
    }
    if ((arPath = FindCodiraLLVMToolPath(g_toolList.at(ToolID::LLVM_AR).name)).empty()) {
        return false;
    }
    if ((ldPath = FindCodiraLLVMToolPath(g_toolList.at(ToolID::LLD).name)).empty()) {
        return false;
    }
    return true;
}

bool Ohos_CODENATIVE::GenerateLinking(const std::vector<TempFileInfo>& objFiles)
{
    // Different linking mode requires different gcc crt files
    GenerateLinkingTool(objFiles, "", {});
    return true;
}

void Ohos_CODENATIVE::GenerateLinkingTool(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath,
    const std::pair<std::string, std::string>& /* gccCrtFilePair */)
{
    auto tool = std::make_unique<Tool>(ldPath, ToolType::BACKEND, driverOptions.environment.allVariables);
    tool->SetLdLibraryPath(FileUtil::JoinPath(FileUtil::GetDirPath(ldPath.c_str()), "../lib"));
    std::string outputFile = GetOutputFileInfo(objFiles).filePath;
    tool->AppendArg("-o", outputFile);
    if (driverOptions.IsLTOEnabled()) {
        GenerateLinkOptionsForLTO(*tool.get());
    } else if (driverOptions.EnableHwAsan()) {
        // Same args as lto except GenerateLinkOptionsForLTO
        tool->AppendArg("-z", "notext");
    } else {
        tool->AppendArg("-z", "noexecstack");
    }
    tool->AppendArg("-z", "max-page-size=4096");
    tool->AppendArgIf(driverOptions.stripSymbolTable, "-s");

    // Hot reload relies on .gnu.hash section.
    tool->AppendArg("--hash-style=both");

    tool->AppendArg("-m", GetEmulation());

    std::string codeldScript = GetCodeldScript(tool);
    // Link order: Scrt1 -> crti -> crtbegin -> other input files -> crtend -> crtn.
    if (driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE) {
        tool->AppendArg("-pie");
        auto maybeCrt1 = FileUtil::FindFileByName("Scrt1.o", GetCRuntimeLibraryPath());
        // If we found Scrt1.o, we use the absolute path of system Scrt1.o, otherwise we let it simply be Scrt1.o,
        // we don't expect such cases though. Same as crti.o and crtend.o.
        tool->AppendArg(maybeCrt1.value_or("Scrt1.o"));
    }
    auto maybeCrti = FileUtil::FindFileByName("crti.o", GetCRuntimeLibraryPath());
    tool->AppendArg(maybeCrti.value_or("crti.o"));
    HandleLLVMLinkOptions(objFiles, gccLibPath, *tool, codeldScript);
    GenerateRuntimePath(*tool);
    auto maybeCrtn = FileUtil::FindFileByName("crtn.o", GetCRuntimeLibraryPath());
    tool->AppendArg(maybeCrtn.value_or("crtn.o"));
    backendCmds.emplace_back(MakeSingleToolBatch({std::move(tool)}));
}

void Ohos_CODENATIVE::GenerateLinkOptions(Tool& tool)
{
    tool.AppendArg("-l:libcodira-runtime.so");
    for (auto& option : LINUX_CODENATIVE_LINK_OPTIONS) {
        // No libgcc_s.so in hm toolchain
        if (option.compare("-l gcc_s") != 0) {
            tool.AppendArg(option);
        }
    }
    tool.AppendArg("-lclang_rt.builtins");
    // Remind runtime to remove dependency of unwind_s in hm
    tool.AppendArg("-lunwind");
}

void Ohos_CODENATIVE::HandleSanitizerDependencies(Tool& tool)
{
    for (const auto& arg : {"-lpthread", "-lrt", "-lm", "-ldl", "-lresolv"}) {
        tool.AppendArg(arg);
    }
    // The ohos has no gcc_s, unwind (has same function as gcc_s) is
    // always linked in GenerateLinkOptions
}
