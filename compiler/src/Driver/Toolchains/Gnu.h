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
 * This file declares the Gnu class.
 */

#ifndef CODIRA_DRIVER_TOOLCHAIN_GNU_H
#define CODIRA_DRIVER_TOOLCHAIN_GNU_H

#include "Codira/Driver/Backend/Backend.h"
#include "Codira/Driver/Driver.h"
#include "Codira/Driver/Toolchains/ToolChain.h"

namespace Codira {

class Gnu : public ToolChain {
public:
    Gnu(const Codira::Driver& driver, const DriverOptions& driverOptions, std::vector<ToolBatch>& backendCmds)
        : ToolChain(driver, driverOptions, backendCmds) {};
    ~Gnu() override = default;

protected:
    std::string ldPath;
    std::string arPath;
    std::string objcopyPath;
    const std::pair<std::string, std::string> gccExecCrtFilePair = std::make_pair("crtbegin.o", "crtend.o");
    const std::pair<std::string, std::string> gccSharedCrtFilePair = std::make_pair("crtbeginS.o", "crtendS.o");

    std::string GetGccLibFile(const std::string& filename, const std::string& gccLibPath) const;
    // Get the executable formats of target system for the linker,
    // and the linker will emulate the target to do cross-compilation.
    virtual std::string GetEmulation() const;
    // Libc uses crtbegin.o/crtend.o to find the start of the constructors/destructors.
    virtual std::pair<std::string, std::string> GetGccCrtFilePair() const;

    bool PrepareDependencyPath() override;
    bool ProcessGeneration(std::vector<TempFileInfo>& objFiles) override;

    virtual std::string GenerateGCCLibPath(const std::pair<std::string, std::string>& gccCrtFilePair) const;

    virtual void GenerateArchiveTool(const std::vector<TempFileInfo>& objFiles);
    // utility method to find clang library, used to find asan and libfuzzer
    // clang library format: libclang_rt.<module name>[-<arch>].<suffix>
    std::optional<std::string> SearchClangLibrary(const std::string libName, const std::string libSuffix);
    void HandleSanitizer(Tool& tool, const std::string& codiraLibPath, const std::string& gccLibPath);
    virtual void HandleSanitizerDependencies(Tool& tool);
    void HandleLLVMLinkOptions(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath, Tool& tool,
        const std::string& codeldScript);
    virtual void HandleLibrarySearchPaths(Tool& tool, const std::string& codiraLibPath);

    void InitializeLibraryPaths() override;
    virtual void AddCRuntimeLibraryPaths();
    // Gather library paths from LIBRARY_PATH and compiler guesses.
    virtual void AddSystemLibraryPaths();
    virtual void GenerateLinkOptions(Tool& tool)
    {
        (void)tool;
    }
    virtual void GenerateLinkingTool(const std::vector<TempFileInfo>& objFiles, const std::string& gccLibPath,
        const std::pair<std::string, std::string>& gccCrtFilePair)
    {
        (void)objFiles;
        (void)gccLibPath;
        (void)gccCrtFilePair;
    }
    virtual bool GenerateLinking(const std::vector<TempFileInfo>& objFiles);

private:
    void HandleAsanDependencies(Tool& tool, const std::string& codiraLibPath, const std::string& gccLibPath);
    void HandleHwasanDependencies(Tool& tool, const std::string& codiraLibPath);
};
} // namespace Codira
#endif // CODIRA_DRIVER_TOOLCHAIN_GNU_H
