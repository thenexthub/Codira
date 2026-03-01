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
 * This file declares the MachO class.
 */

#ifndef CODIRA_DRIVER_TOOLCHAIN_MACHO_H
#define CODIRA_DRIVER_TOOLCHAIN_MACHO_H

#include "Codira/Driver/Backend/Backend.h"
#include "Codira/Driver/Driver.h"

namespace Codira {
class MachO : public ToolChain {
public:
    MachO(const Codira::Driver& driver, const DriverOptions& driverOptions, std::vector<ToolBatch>& backendCmds)
        : ToolChain(driver, driverOptions, backendCmds) {};

    ~MachO() override = default;

    bool ProcessGeneration(std::vector<TempFileInfo>& objFiles) override;

    bool PrepareDependencyPath() override;

    void InitializeLibraryPaths() override;

protected:
    std::string ldPath;
    std::string arPath;
    std::string dsymutilPath;
    std::string stripPath;
    std::string GetSharedLibraryExtension() const override
    {
        return ".dylib";
    }

    // Get the target architecture string. It is passed to the linker.
    virtual std::string GetTargetArchString() const;

    virtual void GenerateArchiveTool(const std::vector<TempFileInfo>& objFiles);
    void HandleLLVMLinkOptions(const std::vector<TempFileInfo>& objFiles, Tool& tool);
    virtual void HandleLibrarySearchPaths(Tool& tool, const std::string& codiraLibPath);

    virtual void AddCRuntimeLibraryPaths();
    // Gather library paths from LIBRARY_PATH and compiler guesses.
    virtual void AddSystemLibraryPaths();
    virtual void GenerateLinkOptions([[maybe_unused]] Tool& tool) {};
    virtual TempFileInfo GenerateLinkingTool([[maybe_unused]] const std::vector<TempFileInfo>& objFiles,
        [[maybe_unused]] const std::string& darwinSDKVersion)
    {
        return TempFileInfo{};
    };
    virtual TempFileInfo GenerateLinking(const std::vector<TempFileInfo>& objFiles);
    virtual void GenerateDebugSymbolFile(const TempFileInfo& binaryFile);
    void GenerateStripSymbolFile(const TempFileInfo& binaryFile);
    // Generate the static link options of built-in libraries except 'std-ast'.
    // The 'std-ast' library is dynamically linked by default.
    void GenerateLinkOptionsOfBuiltinLibsForStaticLink(Tool& tool) const override;
    // Generate the dynamic link options of built-in libraries.
    void GenerateLinkOptionsOfBuiltinLibsForDyLink(Tool& tool) const override;
};
} // namespace Codira
#endif // CODIRA_DRIVER_TOOLCHAIN_MACHO_H
