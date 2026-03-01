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

#ifndef GUARD_CONFIGURATION_CONFIGURATION_H
#define GUARD_CONFIGURATION_CONFIGURATION_H

#include "class_specification.h"

namespace ark::guard {
/**
 * @brief PrintSeedsOption
 * This structure corresponds to the obfuscation option '-printseeds', which outputs the classes and class members
 * matched by the keep option to the console or specified file.
 */
struct PrintSeedsOption {
    /* The default value is false, no output. When the value is true, it indicates printing */
    bool enable = false;
    /* The default value is empty, indicating output to the console. When enable is true and the value is valid, output
     * to this file */
    std::string filePath;
};

/**
 * @brief FileNameObfuscationOption
 * This structure corresponds to the obfuscation and reserved option '-keep-file-name', which represents a list of non
 * obfuscated file names.
 */
struct FileNameObfuscationOption {
    /* The default value is true, indicating that file name obfuscation is enabled */
    bool enable = true;
    /* Record the list of file names to be retained, excluding wildcard characters */
    std::vector<std::string> reservedFileNames;
    /* Record a list of file names to be retained, including wildcard characters */
    std::vector<std::string> universalReservedFileNames;
};

/**
 * @brief KeepPathOption
 * This structure corresponds to the reserved option '-keep path', indicating that the content in the file path is not
 * obfuscated.
 */
struct KeepPathOption {
    /* Record the list of file paths to be retained, without wildcard characters */
    std::vector<std::string> reservedPaths;
    /* Record a list of file paths to be retained, including wildcard characters */
    std::vector<std::string> universalReservedPaths;
};

/**
 * @brief KeepOptions
 * The file path and class_Specification of this structure will not be obfuscated.
 */
struct KeepOptions {
    /* Record the list of file paths to be retained */
    KeepPathOption pathOption;
    /*  Record the list of class_Specification to be retained */
    std::vector<ClassSpecification> classSpecifications;
};

/**
 * @brief ObfuscationRules
 * This structure records the content of obfuscation and reserved options.
 */
struct ObfuscationRules {
    /* The default value is false, indicating that obfuscation is enabled */
    bool disableObfuscation = false;
    /* Save the name cache to the specified file path filename, which contains the mapping before and after name
     * obfuscation */
    std::string printNameCache;
    /* Reuse the specified name to cache the file filename */
    std::string applyNameCache;
    /* The default value is false, indicating that the call to the log statement is not deleted */
    bool removeLog = false;
    /* Obfuscation options '-printseeds' */
    PrintSeedsOption printSeedsOption;
    /* File name obfuscation and reserved options */
    FileNameObfuscationOption fileNameOption;
    /*  Keep option reserved */
    KeepOptions keepOptions;
};

/**
 * @brief ObfuscationConfig
 * This structure records all obfuscated configurations. The fields that have been added with the required field
 * description are all mandatory fields, while the rest are optional fields.
 */
class Configuration {
public:
    /**
     * @brief Check required field has value
     * @return true: valid, false: invalid
     */
    [[nodiscard]] bool IsValid() const;

    /**
     * @brief Get abc file path
     * @return abc file path
     */
    [[nodiscard]] const std::string &GetAbcPath() const;

    /**
     * @brief Get obfuscation abc file path
     * @return obf abc file path
     */
    [[nodiscard]] const std::string &GetObfAbcPath() const;

    /**
     * @brief Get default name cache path
     * @return default name cache path
     */
    [[nodiscard]] const std::string &GetDefaultNameCachePath() const;

    /**
     * @brief Get print name cache path
     * @return print name cache path
     */
    [[nodiscard]] const std::string &GetPrintNameCache() const;

    /**
     * @brief Get apply name cache path
     * @return apply name cache path
     */
    [[nodiscard]] const std::string &GetApplyNameCache() const;

    /**
     * @brief Is disable obfuscation
     * @return true: disable obfuscation, false: enable obfuscation
     */
    [[nodiscard]] bool DisableObfuscation() const;

    /**
     * @brief Is enable remove_log
     * @return true: remove log, false: not remove log
     */
    [[nodiscard]] bool IsRemoveLogObfEnabled() const;

    /**
     * @brief Get print seeds option
     * @return PrintSeedsOption: print seeds option
     */
    [[nodiscard]] PrintSeedsOption GetPrintSeedsOption() const;

    /**
     * @brief Is enable file name obfuscation
     * @return true: enable file name obfuscation, false: not enable file name obfuscation
     */
    [[nodiscard]] bool IsFileNameObfEnabled() const;

    /**
     * @brief Is keep file name
     * @param str file name
     * @return true: keep, false: not keep
     */
    [[nodiscard]] bool IsKeepFileName(const std::string &str) const;

    /**
     * @brief Is keep path
     * @param str path
     * @param keptPath receive the result of keptPath
     * @return true: keep, false: not keep
     */
    [[nodiscard]] bool IsKeepPath(const std::string &str, std::string &keptPath) const;

    /**
     * @brief Get ClassSpecification object list
     * @return ClassSpecification object list
     */
    [[nodiscard]] std::vector<ClassSpecification> GetClassSpecifications() const;

    /**
     * @brief Check has config keep, file name or path
     * @return true: config, false: not config
     */
    [[nodiscard]] bool HasKeepConfiguration() const;

    /*
     * Required field, abc file path to be confused
     */
    std::string abcPath;

    /*
     * Required field, storage path of obfuscated abc file
     */
    std::string obfAbcPath;

    /*
     * Required field, default storage path for name cache. Before confusion, prioritize reading the configuration of
     * the '-applynamecache' option. When the configuration is invalid, the defaultNameCachePath will be read
     */
    std::string defaultNameCachePath;

    /*
     * Obfuscation rules
     */
    ObfuscationRules obfuscationRules{};
};
} // namespace ark::guard

#endif  // GUARD_CONFIGURATION_CONFIGURATION_H