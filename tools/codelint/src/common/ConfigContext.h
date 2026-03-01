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

#ifndef CODIRACODECHECK_CONFIGCONTEXT_H
#define CODIRACODECHECK_CONFIGCONTEXT_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "ExcludeRule.h"

namespace Codira::CodeCheck {
static const std::string DEFAULT_EXCLUSION_FILE = "codelint_file_exclude.cfg";
static const std::string DEFAULT_EXCLUSION_FORMAT = ".cfg";

#ifdef _WIN32
const std::string PATH_SEPARATOR = "\\";
#else
const std::string PATH_SEPARATOR = "/";
#endif

class ConfigContext {
public:
    const std::vector<std::string>& GetSrcFileDir() const;
    const std::string& GetConfigFileDir() const;
    const std::string& GetModulesDir() const;
    const std::vector<ExcludeRule>& GetExcludeList() const;
    const std::string& GetReportFile() const;
    const std::string& GetReportFormat() const;
    const std::string& GetCodiraHome() const;
    const std::string GetCodiraHomePath() const;
    std::string GetCodelintConfigPath();
    const std::vector<std::string>& GetSrcFileList() const;

    void SetSrcFileList(const std::vector<std::string>& srcFileList);
    void SetSrcFileDir(size_t index, std::string srcFile);
    void AddSrcFileDir(const std::string srcFileDirInput);
    void SetConfigFileDir(const std::string configFileDirInput);
    void SetModulesDir(const std::string moduleFileDirInput);
    void AddExcludeList(const std::string excludeFile);
    void SetReportFile(const std::string reportFileInput);
    bool SetReportFormat(const std::string reportFormatInput);
    void SetCodiraHome(const std::string cangjieHomePath);
    static ConfigContext &GetInstance()
    {
        static ConfigContext instance;
        return instance;
    }
    void FilterCompileList(const std::vector<std::string> &fileList);
    void CheckDefaultExclusion();
    const std::string& GetCodeoPath() const;
    void SetCodeoPath(const std::string codeoPathStr);

private:
    ConfigContext() {}
    ~ConfigContext() = default;
    ConfigContext(const ConfigContext&);
    ConfigContext& operator=(const ConfigContext&);
    void AddExcludeRule(const std::string &excludeDir, const std::string &exclude);

#ifdef _WIN32
    const std::string configPath = "\\tools\\";
#else
    const std::string configPath = "/tools/";
#endif
    std::vector<std::string> srcFileDir;
    std::string configFileDir;
    std::string modulesDir;
    std::vector<ExcludeRule> excludeList;
    std::string reportFile;
    std::string reportFormat = "json";
    std::vector<std::string> srcFileList;
    std::string cangjieHome;
    bool defaultExclusion = true;
    std::string codeoPath;
};
}
#endif // CODIRACODECHECK_CONFIGCONTEXT_H
