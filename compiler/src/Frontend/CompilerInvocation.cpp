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
 * This file implements the CompilerInvocation.
 */

#include "Codira/Frontend/CompilerInvocation.h"
#include "Codira/Driver/Backend/CODENATIVEBackend.h"
#include "Codira/Driver/DriverOptions.h"
#include "Codira/Macro/InvokeUtil.h"

using namespace Codira;

bool CompilerInvocation::ParseArgs(const std::vector<std::string>& args)
{
    if (!optionTable->ParseArgs(args, *argList)) {
        return false;
    }

    frontendOptions.SetFrontendMode();
    if (!frontendOptions.ParseFromArgs(*argList)) {
        return false;
    }

    frontendOptions.SetCompilationCachedPath();
    globalOptions = frontendOptions;
    return true;
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
std::string CompilerInvocation::GetRuntimeLibPath(const std::string& relativePath)
{
    auto runtimeLib = "libcodira-runtime.so";
#ifdef _WIN64
    runtimeLib = "libcodira-runtime.dll";
#elif defined(__APPLE__)
    runtimeLib = "libcodira-runtime.dylib";
#endif
    std::string& exePath = globalOptions.executablePath;
    std::string hostPathName = globalOptions.GetCodiraLibHostPathName();
    auto basePath = FileUtil::JoinPath(FileUtil::GetDirPath(exePath), relativePath);
    auto runtimeLibPath =
        FileUtil::JoinPath(FileUtil::JoinPath(basePath, hostPathName), runtimeLib);
    return runtimeLibPath;
}
#endif
