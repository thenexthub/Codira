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
 * This file declares the Android_CODENATIVE ToolChain class.
 */

#ifndef CODIRA_DRIVER_TOOLCHAIN_ANDROID_CODENATIVE_H
#define CODIRA_DRIVER_TOOLCHAIN_ANDROID_CODENATIVE_H

#include <string>

#include "Toolchains/CODENATIVE/Linux_CODENATIVE.h"
#include "Codira/Driver/Backend/Backend.h"
#include "Codira/Driver/Driver.h"
#include "Codira/Driver/Toolchains/ToolChain.h"

namespace Codira {
class Android_CODENATIVE : public Linux_CODENATIVE {
public:
    Android_CODENATIVE(
        const Codira::Driver& driver, const DriverOptions& driverOptions, std::vector<ToolBatch>& backendCmds)
        : Linux_CODENATIVE(driver, driverOptions, backendCmds) {};
    ~Android_CODENATIVE() override {};

protected:
    void InitializeLibraryPaths() override;
    void AddCRuntimeLibraryPaths() override;
    bool PrepareDependencyPath() override;

    bool GenerateLinking(const std::vector<TempFileInfo>& objFiles) override;
    void GenerateLinkingTool(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath,
        const std::pair<std::string, std::string>& gccCrtFilePair) override;

    void GenerateLinkOptions(Tool& tool) override;

    void HandleSanitizerDependencies(Tool& tool) override;

private:
    std::string sysroot;
};
} // namespace Codira
#endif // CODIRA_DRIVER_TOOLCHAIN_ANDROID_CODENATIVE_H
