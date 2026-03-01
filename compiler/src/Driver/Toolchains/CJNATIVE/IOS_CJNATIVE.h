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
 * This file declares the IOS_CODENATIVE class.
 */

#ifndef CODIRA_DRIVER_TOOLCHAIN_IOS_CODENATIVE_H
#define CODIRA_DRIVER_TOOLCHAIN_IOS_CODENATIVE_H

#include "Toolchains/CODENATIVE/Darwin_CODENATIVE.h"
#include "Codira/Driver/Backend/Backend.h"
#include "Codira/Driver/Driver.h"
#include "Codira/Driver/Toolchains/ToolChain.h"

using namespace Codira;

class IOS_CODENATIVE : public Darwin_CODENATIVE {
public:
    IOS_CODENATIVE(const Codira::Driver& driver, const DriverOptions& driverOptions,
        std::vector<ToolBatch>& backendCmds)
        : Darwin_CODENATIVE(driver, driverOptions, backendCmds) {};
    ~IOS_CODENATIVE() override {};

protected:
    const std::vector<std::string> IOS_CODENATIVE_LINK_OPTIONS = {
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

#endif // CODIRA_DRIVER_TOOLCHAIN_IOS_CODENATIVE_H
