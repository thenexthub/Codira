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
 * This file declares the DriverOptions, which provides options for Driver.
 */

#ifndef CODIRA_DRIVER_DRIVEROPTIONS_H
#define CODIRA_DRIVER_DRIVEROPTIONS_H

#include <string>

#include "Codira/Option/Option.h"

namespace Codira {
class DriverOptions : public GlobalOptions {
public:
    /**
     * @brief The constructor of class DriverOptions.
     *
     * @return DriverOptions The instance of DriverOptions.
     */
    DriverOptions() = default;

    /**
     * @brief The destructor of class DriverOptions.
     */
    ~DriverOptions() override = default;

    std::string optArg;
    std::string llcArg;

    std::optional<std::string> targetCPU = std::nullopt;

    bool linkStatic = false;

    // Strip symbol table for DSO and executable or not.
    // Controlled by --strip-all/-s.
    bool stripSymbolTable = false;

    // User-provided a custom argument to linker.
    // Values is passed by --link-option.
    std::vector<std::string> linkOption;

    // User-provided custom arguments to linker.
    // Values is passed by --link-options.
    std::vector<std::string> linkOptions;

    // User-provided paths for linkers to search for libraries,
    // passed by --library-path/-L.
    std::vector<std::string> librarySearchPaths;

    // User-provided library names for linkers,
    // passed by --library/-l.
    std::vector<std::string> libraries;

    // User-provided paths for the driver to search for binaries and object files,
    // passed by --toolchain/-B.
    std::vector<std::string> toolChainPaths;

    // Sysroot is the root directory under which toolchain binaries, libraries and header files can be found.
    // The sysroot is "/" by default.
#ifdef _WIN32
    std::string sysroot = "C:/windows";
#else
    std::string sysroot = "/";
#endif

    bool customizedSysroot = false;

    bool useRuntimeRpath = false;

    // Whether codec write rpath with sanitizer version codira runtime path to binary
    bool sanitizerEnableRpath = false;

    bool incrementalCompileNoChange = false;

    // ---------- CODE OBFUSCATION OPTIONS ----------
    bool enableObfAll = false;

    /**
     * @brief Check whether it is obfuscation enabled.
     *
     * @return bool Return true If it is obfuscation enabled.
     */
    bool IsObfuscationEnabled() const override
    {
        return enableStringObfuscation || enableConstObfuscation || enableLayoutObfuscation ||
            enableCFflattenObfuscation || enableCFBogusObfuscation;
    }

    std::optional<bool> enableObfExportSyms;
    std::optional<bool> enableObfLineNumber;
    std::optional<bool> enableObfSourcePath;
    std::optional<bool> enableLayoutObfuscation;
    std::optional<bool> enableConstObfuscation;
    std::optional<bool> enableStringObfuscation;
    std::optional<bool> enableCFflattenObfuscation;
    std::optional<bool> enableCFBogusObfuscation;
    std::optional<std::string> layoutObfSymPrefix;
    std::optional<std::string> layoutObfInputSymMappingFiles = std::nullopt;
    std::optional<std::string> layoutObfOutputSymMappingFile = std::nullopt;
    std::optional<std::string> layoutObfUserMappingFile = std::nullopt;
    std::optional<std::string> obfuscationConfigFile = std::nullopt;

    static const int OBFUCATION_LEVEL_MIN = 1;
    static const int OBFUCATION_LEVEL_MAX = 9;

    /**
     * Valid range of obfuscation level is [1,9] (see OBFUCATION_LEVEL_MIN and OBFUCATION_LEVEL_MAX).
     * Higher obfuscation level represents that more obfuscation work will be done on the code, which
     * makes the product more difficult to be understood by reverse enginering. Higher obfuscation level
     * also causes larger code size and more runtime overhead. 5 is a balanced choice, for both source
     * code safety and cost.
     */
    int obfuscationLevel = 5;

    std::optional<int> obfuscationSeed = 0;

    /**
     * @brief Reprocess obfuse option.
     *
     * @return bool Return true.
     */
    bool ReprocessObfuseOption() override;

protected:
    virtual std::optional<bool> ParseOption(OptionArgInstance& arg) override;
    virtual bool PerformPostActions() override;

private:
    bool CheckObfuscationOptions() const;
    bool CheckTargetCPUOption();
    bool SetupSysroot();
    bool CheckRuntimeRPath() const;
    bool CheckSanitizerRPath() const;
    bool CheckStaticOption();
};
} // namespace Codira
#endif // CODIRA_DRIVER_DRIVEROPTIONS_H
