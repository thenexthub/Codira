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
 * This file declares the Linux_CODENATIVE class.
 */

#ifndef CODIRA_DRIVER_TOOLCHAIN_Darwin_CODENATIVE_H
#define CODIRA_DRIVER_TOOLCHAIN_Darwin_CODENATIVE_H

#include "Codira/Driver/Backend/Backend.h"
#include "Codira/Driver/Driver.h"
#include "Toolchains/MachO.h"
#include "Codira/Driver/Toolchains/ToolChain.h"

namespace Codira {
class Darwin_CODENATIVE : public MachO {
public:
    Darwin_CODENATIVE(const Codira::Driver& driver, const DriverOptions& driverOptions,
        std::vector<ToolBatch>& backendCmds)
        : MachO(driver, driverOptions, backendCmds) {};
    ~Darwin_CODENATIVE() override = default;

protected:
    const std::vector<std::string> DARWIN_CODENATIVE_LINK_OPTIONS = {
#define CODENATIVE_STD_OPTIONS(OPTION) (OPTION),
#include "Toolchains/BackendOptions.inc"
#undef CODENATIVE_STD_OPTIONS
#define CODENATIVE_DARWIN_BASIC_OPTIONS(OPTION) (OPTION),
#include "Toolchains/BackendOptions.inc"
#undef CODENATIVE_DARWIN_BASIC_OPTIONS
    };
    // Gather library paths from LIBRARY_PATH and compiler guesses.
    void AddSystemLibraryPaths() override;
    TempFileInfo GenerateLinkingTool(
        const std::vector<TempFileInfo>& objFiles, const std::string& darwinSDKVersion) override;
    void GenerateLinkOptions(Tool& tool) override;
};
} // namespace Codira
#endif // CODIRA_DRIVER_TOOLCHAIN_Darwin_CODENATIVE_H
