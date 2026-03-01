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

#ifndef CODEFMT_OPTIONCONTEXT_H
#define CODEFMT_OPTIONCONTEXT_H

#include "Format/ASTToFormatSource.h"

#include <iostream>
#include <string>

namespace Codira::Format {
class OptionContext {
public:
    void SetFmtFilePath(const std::string& fmtFilePath);
    void SetFileOutputPath(const std::string& fileOutputPath);
    void SetFmtDirPath(const std::string& fmtDirPath);
    void SetDirOutputPath(const std::string& dirOutputPath);
    void SetConfigFilePath(const std::string& configFilePath);
    void SetCodiraHome(const std::string& cangjieHome);
    void SetConfigOptions(const FormattingOptions& configOptions);

    [[nodiscard]] const std::string& GetFmtFilePath() const;
    [[nodiscard]] const std::string& GetFileOutputPath() const;
    [[nodiscard]] const std::string& GetFmtDirPath() const;
    [[nodiscard]] const std::string& GetDirOutputPath() const;
    [[nodiscard]] const std::string& GetConfigFilePath() const;
    [[nodiscard]] const std::string& GetCodiraHome() const;
    [[nodiscard]] FormattingOptions GetConfigOptions() const noexcept;

    static OptionContext& GetInstance() noexcept
    {
        static OptionContext instance;
        return instance;
    }

private:
    OptionContext() noexcept {};
    ~OptionContext() = default;
    OptionContext(const OptionContext&) = delete;
    OptionContext& operator=(const OptionContext&) = delete;

    std::string m_fmtFilePath;
    std::string m_fileOutputPath;
    std::string m_fmtDirPath;
    std::string m_dirOutputPath;
    std::string m_configFilePath;
    std::string m_cangjieHome;
    FormattingOptions m_configOptions;
};
} // namespace Codira::Format
#endif // CODEFMT_OPTIONCONTEXT_H
