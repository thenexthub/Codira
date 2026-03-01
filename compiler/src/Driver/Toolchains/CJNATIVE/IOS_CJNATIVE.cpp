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
 * This file implements the IOS_CODENATIVE ToolChain base class.
 */

#include "Toolchains/CODENATIVE/IOS_CODENATIVE.h"

#include "Codira/Driver/TempFileManager.h"
#include "Codira/Driver/ToolOptions.h"
#include "Codira/Driver/Utils.h"
#include "Codira/Utils/FileUtil.h"

using namespace Codira;
using namespace Codira::Triple;

void IOS_CODENATIVE::AddSystemLibraryPaths()
{
    if (driverOptions.IsCrossCompiling() && driverOptions.customizedSysroot) {
        // user-specified sysroot is only considered in cross-compilation
        AddLibraryPaths(ComputeLibPaths());
    }
    MachO::AddSystemLibraryPaths();
}

TempFileInfo IOS_CODENATIVE::GenerateLinkingTool(
    const std::vector<TempFileInfo>& objFiles, const std::string& darwinSDKVersion)
{
    auto tool = std::make_unique<Tool>(ldPath, ToolType::BACKEND, driverOptions.environment.allVariables);
    auto outputFileInfo = TempFileInfo{};
    if (driverOptions.stripSymbolTable) {
        TempFileKind kind = driverOptions.outputMode == GlobalOptions::OutputMode::SHARED_LIB
            ? TempFileKind::T_DYLIB_MAC
            : TempFileKind::T_EXE_MAC;
        outputFileInfo = TempFileManager::Instance().CreateNewFileInfo(objFiles[0], kind);
    } else {
        outputFileInfo = GetOutputFileInfo(objFiles);
    }
    std::string outputFile = outputFileInfo.filePath;
    tool->AppendArg("-o", outputFile);
    tool->AppendArgIf(driverOptions.outputMode == GlobalOptions::OutputMode::SHARED_LIB, "-dylib");
    tool->AppendArg("-arch", GetTargetArchString());

    tool->AppendArg("-platform_version");
    if (driverOptions.target.env == Triple::Environment::SIMULATOR) {
        tool->AppendArg("ios-simulator");
        tool->AppendArg("17.5.0");
    } else {
        tool->AppendArg("ios");
        tool->AppendArg("17.5.0");
    }
    tool->AppendArg(darwinSDKVersion);

    tool->AppendArg("-syslibroot");
    tool->AppendArg(driverOptions.sysroot.empty() ? "/" : driverOptions.sysroot);

    if (driverOptions.outputMode == GlobalOptions::OutputMode::EXECUTABLE) {
        tool->AppendArg("-pie");
    }
    HandleLLVMLinkOptions(objFiles, *tool);
    GenerateRuntimePath(*tool);
    backendCmds.emplace_back(MakeSingleToolBatch({std::move(tool)}));
    return outputFileInfo;
}

void IOS_CODENATIVE::GenerateLinkOptions(Tool& tool)
{
    for (auto& option : IOS_CODENATIVE_LINK_OPTIONS) {
        tool.AppendArg(option);
    }
    auto codiraLibPath =
        FileUtil::JoinPath(FileUtil::JoinPath(driver.codiraHome, "lib"),
                           driverOptions.GetCodiraLibTargetPathName());
    if (driverOptions.target.env == Triple::Environment::SIMULATOR) {
        tool.AppendArg(FileUtil::JoinPath(codiraLibPath, "libclang_rt.iossim.a"));
    } else {
        tool.AppendArg(FileUtil::JoinPath(codiraLibPath, "libclang_rt.ios.a"));
    }
}
