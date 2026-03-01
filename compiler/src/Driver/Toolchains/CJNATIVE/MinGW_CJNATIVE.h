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
 * This file declares the MinGW_CODENATIVE class.
 */

#ifndef CODIRA_DRIVER_TOOLCHAIN_MINGW_CODENATIVE_H
#define CODIRA_DRIVER_TOOLCHAIN_MINGW_CODENATIVE_H

#include "Codira/Driver/Backend/Backend.h"
#include "Codira/Driver/Driver.h"
#include "Toolchains/Gnu.h"
#include "Codira/Driver/Toolchains/ToolChain.h"

namespace Codira {
class MinGW_CODENATIVE : public Gnu {
public:
    MinGW_CODENATIVE(const Codira::Driver& driver, const DriverOptions& driverOptions,
        std::vector<ToolBatch>& backendCmds)
        : Gnu(driver, driverOptions, backendCmds)
    {
        mingwLibPath = FileUtil::JoinPath(driver.codiraHome, "third_party/mingw/lib/");
    };
    ~MinGW_CODENATIVE() override = default;

protected:
    std::string GetSharedLibraryExtension() const override
    {
        return ".dll";
    }

    void InitializeLibraryPaths() override;

    void InitializeMinGWSysroot();

    void AddCRuntimeLibraryPaths() override;
    bool PrepareDependencyPath() override;

    // Gather library paths from LIBRARY_PATH and compiler guesses.
    void AddSystemLibraryPaths() override;

    std::vector<std::string> ComputeLibPaths() const override;

    std::string GenerateGCCLibPath(const std::pair<std::string, std::string>& gccCrtFilePair) const override;

    void GenerateArchiveTool(const std::vector<TempFileInfo>& objFiles) override;

    void HandleLibrarySearchPaths(Tool& tool, const std::string& codiraLibPath) override;

    std::pair<std::string, std::string> GetGccCrtFilePair() const override
    {
        return gccExecCrtFilePair;
    }

    std::string GetCodeldScript(const std::unique_ptr<Tool>& tool) const;
    void GenerateLinkingTool(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath,
        const std::pair<std::string, std::string>& gccCrtFilePair) override;

    void GenerateLinkOptions(Tool& tool) override;

    std::string FindCodiraMinGWToolPath(const std::string toolName) const;

private:
    const std::vector<std::string> MINGW_CODENATIVE_LINK_OPTIONS = {
#define CODENATIVE_WINDOWS_BASIC_OPTIONS(OPTION) (OPTION),
#include "Toolchains/BackendOptions.inc"
#undef CODENATIVE_WINDOWS_BASIC_OPTIONS
    };
    std::string sysroot;
    std::string mingwLibPath;
};
} // namespace Codira

#endif // CODIRA_DRIVER_TOOLCHAIN_MINGW_CODENATIVE_H
