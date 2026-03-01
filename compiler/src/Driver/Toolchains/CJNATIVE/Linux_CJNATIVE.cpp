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
 * This file implements the Linux_CODENATIVE ToolChain base class.
 */

#include "Toolchains/CODENATIVE/Linux_CODENATIVE.h"

#include "Codira/Driver/ToolOptions.h"
#include "Codira/Driver/Utils.h"
#include "Codira/Utils/FileUtil.h"

using namespace Codira;
using namespace Codira::Triple;

void Linux_CODENATIVE::AddSystemLibraryPaths()
{
    if (driverOptions.IsCrossCompiling() && driverOptions.customizedSysroot) {
        // user-specified sysroot is only considered in cross-compilation
        AddLibraryPaths(ComputeLibPaths());
    }
    Gnu::AddSystemLibraryPaths();
}

std::pair<std::string, std::string> Linux_CODENATIVE::GetGccCrtFilePair() const
{
    switch (driverOptions.outputMode) {
        case GlobalOptions::OutputMode::EXECUTABLE:
        case GlobalOptions::OutputMode::SHARED_LIB:
            return gccSharedCrtFilePair;
            // WARNING: OutputMode::STATIC_LIB has no relation with gcc static crt files.
            // OutputMode::STATIC_LIB indicates that we should produce an archive(.a) file, which is similar to '-c'
            // Static crt files are for 'ld -static' but it is not supported currently.
        case GlobalOptions::OutputMode::STATIC_LIB:
        default:
            break;
    }
    return {};
}

std::string Linux_CODENATIVE::GetCodeldScript(const std::unique_ptr<Tool>& tool) const
{
    std::string codeldScript = "codeld.lds";
    switch (driverOptions.outputMode) {
        case GlobalOptions::OutputMode::SHARED_LIB:
            tool->AppendArg("-shared");
            codeldScript = "codeld.shared.lds";
            [[fallthrough]];
        case GlobalOptions::OutputMode::EXECUTABLE:
            tool->AppendArg("-dynamic-linker");
            tool->AppendArg(GetDynamicLinkerPath(driverOptions.target));
            break;
        case GlobalOptions::OutputMode::STATIC_LIB:
        default:
            break;
    }
    return codeldScript;
}

void Linux_CODENATIVE::GenerateLinkingTool(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath,
    const std::pair<std::string, std::string>& gccCrtFilePair)
{
    auto tool = std::make_unique<Tool>(ldPath, ToolType::BACKEND, driverOptions.environment.allVariables);
    std::string outputFile = GetOutputFileInfo(objFiles).filePath;
    tool->AppendArg("-o", outputFile);
    if (driverOptions.IsLTOEnabled()) {
        tool->SetLdLibraryPath(FileUtil::JoinPath(FileUtil::GetDirPath(ldPath), "../lib"));
        GenerateLinkOptionsForLTO(*tool.get());
    } else if (driverOptions.EnableHwAsan()) {
        // same args as lto except GenerateLinkOptionsForLTO
        tool->SetLdLibraryPath(FileUtil::JoinPath(FileUtil::GetDirPath(ldPath), "../lib"));
        tool->AppendArg("-z", "notext");
    } else {
        tool->AppendArg("-z", "noexecstack");
    }

    tool->AppendArgIf(driverOptions.stripSymbolTable, "-s");
    tool->AppendArg("-m", GetEmulation());

    // Hot reload relies on .gnu.hash section.
    tool->AppendArg("--hash-style=both");
    tool->AppendArgIf(driverOptions.enableGcSections, "-gc-sections");

    std::string codeldScript = GetCodeldScript(tool);
    // Link order: crt1 -> crti -> crtbegin -> other input files -> crtend -> crtn.
    if (driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE) {
        tool->AppendArg("-pie");
        auto maybeCrt1 = FileUtil::FindFileByName("Scrt1.o", GetCRuntimeLibraryPath());
        // If we found Scrt1.o, we use the absolute path of system Scrt1.o, otherwise we let it simply be Scrt1.o,
        // we don't expect such cases though. Same as crti.o and crtend.o.
        tool->AppendArg(maybeCrt1 ? maybeCrt1.value() : "Scrt1.o");
    }
    auto maybeCrti = FileUtil::FindFileByName("crti.o", GetCRuntimeLibraryPath());
    tool->AppendArg(maybeCrti ? maybeCrti.value() : "crti.o");
    tool->AppendArg(GetGccLibFile(gccCrtFilePair.first, gccLibPath));
    // Add crtfastmath.o if fast math is enabled and crtfastmath.o is found.
    if (driverOptions.fastMathMode) {
        auto crtfastmathFilepath = GetGccLibFile("crtfastmath.o", gccLibPath);
        tool->AppendArgIf(FileUtil::FileExist(crtfastmathFilepath), crtfastmathFilepath);
    }
    HandleLLVMLinkOptions(objFiles, gccLibPath, *tool, codeldScript);
    GenerateRuntimePath(*tool);
    tool->AppendArg(GetGccLibFile(gccCrtFilePair.second, gccLibPath));
    auto maybeCrtn = FileUtil::FindFileByName("crtn.o", GetCRuntimeLibraryPath());
    tool->AppendArg(maybeCrtn ? maybeCrtn.value() : "crtn.o");
    backendCmds.emplace_back(MakeSingleToolBatch({std::move(tool)}));
}

void Linux_CODENATIVE::GenerateLinkOptions(Tool& tool)
{
    if (driverOptions.linkStatic) {
        if (driverOptions.EnableSanitizer()) {
            auto codiraLibPath = FileUtil::JoinPath(FileUtil::JoinPath(
                FileUtil::JoinPath(driver.codiraHome, "lib"),
                driverOptions.GetCodiraLibTargetPathName()),
                driverOptions.SanitizerTypeToShortString());
            tool.AppendArg(FileUtil::JoinPath(codiraLibPath, "libcodira-runtime.a"));
            tool.AppendArg(FileUtil::JoinPath(codiraLibPath, "libcodira-thread.a"));
            tool.AppendArg(FileUtil::JoinPath(codiraLibPath, "libboundscheck-static.a"));
        } else {
            tool.AppendArg("-l:libcodira-runtime.a");
            tool.AppendArg("-l:libcodira-thread.a");
            tool.AppendArg("-l:libboundscheck-static.a");
        }
        tool.AppendArg(LINUX_STATIC_LINK_OPTIONS);
    } else {
        tool.AppendArg("-l:libcodira-runtime.so");
        tool.AppendArg(LINUX_CODENATIVE_LINK_OPTIONS);
    }
    // `__gnu_h2f_ieee` `__gnu_f2h_ieee` two symbols are needed to float16 on non-arm64 platform.
    // Because std-core package is always imported, we put it here.
    if (driverOptions.target.arch != Triple::ArchType::AARCH64) {
        tool.AppendArg("-lclang_rt-builtins");
    } else {
        if (driverOptions.target.os == Triple::OSType::LINUX && driverOptions.target.env == Triple::Environment::GNU) {
            tool.AppendArg("-lgcc");
        }
    }
}

void Linux_CODENATIVE::GenerateLinkOptionsForLTO(Tool& tool) const
{
    using namespace ToolOptions;

    // Set LTO lld options
    SetFuncType setOptionHandler = [&tool](const std::string& option) { tool.AppendArg(option); };
    std::vector<ToolOptionType> setOptionsPass = {
        LLD::SetLTOOptimizationLevelOptions, // Comment ensure vector members are arranged vertically.
        LLD::SetLTOOptions,                  //
    };
    SetOptions(setOptionHandler, driverOptions, setOptionsPass);

    // Set opt passes
    std::string passesCollector = "--lto-newpm-passes=";
    std::vector<std::string> passItems;
    {
        using namespace ToolOptions;
        // remove the initial hyphen in the options.
        SetFuncType handler = [&passItems](const std::string& option) { passItems.emplace_back(option.substr(1)); };
        SetOptions(handler, driverOptions, OPT::SetNewPassManagerOptions);
    }
    for (auto& it : passItems) {
        passesCollector += it;
        if (&it != &passItems.back()) {
            passesCollector += ",";
        }
    }
    tool.AppendArg(passesCollector);

    // use set to avoid duplicate options
    std::unordered_set<std::string> optionSet = {};
    // set composite option
    SetFuncType setCompositeOption = [&tool, &optionSet](const std::string& option) {
        if (optionSet.find(option) == optionSet.end()) {
            optionSet.emplace(option);
            tool.AppendArg("--mllvm");
            tool.AppendArg(option);
        }
    };
    std::vector<ToolOptionType> setCompositeOptionsPass = {
        OPT::SetOptions,                // Comment ensure vector members are arranged vertically.
        OPT::SetCodeObfuscationOptions, //
        OPT::SetTransparentOptions,     // The transparent OPT options must after other OPT options.
        LLD::SetPgoOptions,             //
        LLC::SetOptions,                //
        LLC::SetTransparentOptions,     // The transparent LLC options must after other LLC options.
    };
    SetOptions(setCompositeOption, driverOptions, setCompositeOptionsPass);
}
