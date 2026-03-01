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
 * This file declares the ToolChain class.
 */

#ifndef CODIRA_DRIVER_TOOLCHAIN_H
#define CODIRA_DRIVER_TOOLCHAIN_H

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
#include <unordered_map>
#endif

#include "Codira/Driver/Driver.h"
#include "Codira/Driver/DriverOptions.h"
#include "Codira/Driver/Tool.h"
#include "Codira/Option/Option.h"

namespace Codira {
class ToolChain {
public:
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    /**
     * @brief The constructor of class ToolChain.
     *
     * @param driver It is the object that triggers the compiler's compilation process.
     * @param driverOptions The command line arguments input by the user.
     * @param backendCmds The generated backend commands.
     * @return ToolChain The instance of ToolChain.
     */
    ToolChain(const Codira::Driver& driver, const DriverOptions& driverOptions, std::vector<ToolBatch>& backendCmds)
        : driver(driver), driverOptions(driverOptions), backendCmds(backendCmds) {};
#endif

    /**
     * @brief The destructor of class ToolChain.
     */
    virtual ~ToolChain() {};

    /**
     * @brief Initialize library paths.
     */
    virtual void InitializeLibraryPaths() {};

    /**
     * @brief Prepare dependency path.
     */
    virtual bool PrepareDependencyPath() = 0;

    /**
     * @brief Process generation.
     *
     * @param input The generated object files.
     * @return bool Return true If success.
     */
    virtual bool ProcessGeneration(std::vector<TempFileInfo>& input) = 0;

protected:
    const Codira::Driver& driver;
    const DriverOptions& driverOptions;
    std::vector<ToolBatch>& backendCmds;

    virtual std::string GetSharedLibraryExtension() const
    {
        return ".so";
    }

    void CheckAndAddPathTo(std::vector<std::string>& results, const std::string& path, std::string base = "") const
    {
        auto fullPath = FileUtil::JoinPath(base, path);
        if (FileUtil::FileExist(fullPath)) {
            results.emplace_back(fullPath);
        }
    }

    // @brief C Runtime library paths are system paths & some default paths under which we search for crt1.o, crti.o,
    // etc. Some default paths may not exist in some system, we add paths that exist only.
    void AddCRuntimeLibraryPath(const std::string& path)
    {
        CheckAndAddPathTo(cRuntimeLibraryPaths, path);
    }

    const std::vector<std::string>& GetCRuntimeLibraryPath() const
    {
        return cRuntimeLibraryPaths;
    }

    // Library paths are from LIBRARY_PATH variable. A user may give a directory
    // that doesn't exist at all! Comparing to AddCRuntimeLibraryPath(...), we treat user given paths
    // as existing paths instead of filtering them silently. The user could find such problems easier
    // if all user-given paths are shown in the final command.
    void AddLibraryPath(const std::string& path)
    {
        if (!path.empty()) {
            libraryPaths.emplace_back(path);
        }
    }

    void AddLibraryPaths(const std::vector<std::string>& paths)
    {
        libraryPaths.insert(libraryPaths.end(), paths.begin(), paths.end());
    }

    std::vector<std::string> GetLibraryPaths()
    {
        return libraryPaths;
    }

    std::string GetDynamicLinkerPath(const Triple::Info& tripleInfo) const
    {
        switch (tripleInfo.os) {
            case Triple::OSType::LINUX:
                if (tripleInfo.env == Triple::Environment::OHOS && tripleInfo.arch == Triple::ArchType::AARCH64) {
                    return "/lib/ld-musl-aarch64.so.1";
                }
                if (tripleInfo.env == Triple::Environment::OHOS && tripleInfo.arch == Triple::ArchType::ARM32) {
                    return "/lib/ld-musl-arm.so.1";
                }
                if (tripleInfo.env == Triple::Environment::OHOS && tripleInfo.arch == Triple::ArchType::X86_64) {
                    return "/lib/ld-musl-x86_64.so.1";
                }
                if (tripleInfo.env == Triple::Environment::ANDROID) {
                    return "/system/bin/linker64";
                }
                if (tripleInfo.arch == Triple::ArchType::X86_64) {
                    return "/lib64/ld-linux-x86-64.so.2";
                }
                if (tripleInfo.arch == Triple::ArchType::AARCH64) {
                    return "/lib/ld-linux-aarch64.so.1";
                }
                break;
            default:
                break;
        }
        return "";
    }

    // Generate the link options of built-in libraries.
    void GenerateLinkOptionsOfBuiltinLibs(Tool& tool) const;

    // Generate the static link options of built-in libraries except 'std-ast'.
    // The 'std-ast' library is dynamically linked by default.
    virtual void GenerateLinkOptionsOfBuiltinLibsForStaticLink(Tool& tool) const;
    // Generate the dynamic link options of built-in libraries.
    virtual void GenerateLinkOptionsOfBuiltinLibsForDyLink(Tool& tool) const;

    // Traverse built-in libraries.
    void ForEachBuiltinDependencies(const std::unordered_set<std::string>& builtinDependencies,
        const std::function<void(std::string)>& lambda) const
    {
        std::for_each(builtinDependencies.begin(), builtinDependencies.end(), lambda);
    }

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    const std::unordered_map<std::string, std::vector<std::string>> ALWAYS_DYNAMIC_LINK_STD_LIBRARIES = {
        {"libcodira-std-ast.a",
            {"libcodira-std-core.a", "libcodira-std-collection.a", "libcodira-std-sort.a",
                "libcodira-std-math.a"}}};
#endif
    // Check other dependencies of 'staticLib' and emplace them in 'otherLibs'.
    void CheckOtherDependeniesOfStaticLib(
        const std::string& libName, std::set<std::string>& dynamicLibraries, std::set<std::string>& otherLibs) const;

    void AppendObjectsFromCompiled(Tool& tool, const std::vector<TempFileInfo>& objFiles,
        std::vector<std::tuple<std::string, uint64_t>>& inputOrderTuples);
    void AppendObjectsFromInput(std::vector<std::tuple<std::string, uint64_t>>& inputOrderTuples);
    void AppendLibrariesFromInput(std::vector<std::tuple<std::string, uint64_t>>& inputOrderTuples);
    void AppendLinkOptionFromInput(std::vector<std::tuple<std::string, uint64_t>>& inputOrderTuples);
    void AppendLinkOptionsFromInput(std::vector<std::tuple<std::string, uint64_t>>& inputOrderTuples);
    void SortInputlibraryFileAndAppend(Tool& tool, const std::vector<TempFileInfo>& objFiles);
    TempFileInfo GetOutputFileInfo(const std::vector<TempFileInfo>& objFiles) const;
    // Get the right cruntime lib folder name by giving arch.
    std::string GetArchFolderName(const Triple::ArchType& arch) const;
    // Only available for ELF or MachO targets.
    void GenerateRuntimePath(Tool& tool);

    // Make some guesses about library paths based on target and sysroot.
    virtual std::vector<std::string> ComputeLibPaths() const;
    // Make some guesses about tool binary paths based on target and sysroot.
    virtual std::vector<std::string> ComputeBinPaths() const;

    std::string FindToolPath(const std::string toolName, const std::vector<std::string>& paths) const
    {
        return FileUtil::FindProgramByName(toolName, paths);
    };

    template <typename... Args>
    std::string FindToolPath(
        const std::string toolName, const std::vector<std::string>& paths, Args&&... morePaths) const
    {
        std::string toolPath = FindToolPath(toolName, paths);
        if (!toolPath.empty()) {
            return toolPath;
        }
        return FindToolPath(toolName, morePaths...);
    };

    std::string FindCodiraLLVMToolPath(const std::string& toolName) const;
    std::string FindUserToolPath(const std::string& toolName) const;

    virtual std::string GetClangRTProfileLibraryName() const
    {
        return "libclang_rt-profile.a";
    }

private:
    // cRuntimeLibraryPaths is used to search for C runtime object files, such as crt1.o, crti.o, crtn.o.
    std::vector<std::string> cRuntimeLibraryPaths;
    // libraryPaths are obtained from LIBRARY_PATH env, which will be used to search for library files (.so/.a).
    std::vector<std::string> libraryPaths;
};
} // namespace Codira
#endif // CODIRA_DRIVER_TOOLCHAIN_H
