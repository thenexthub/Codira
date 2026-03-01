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

#include "Format/OptionContext.h"

using namespace Codira::Format;

void OptionContext::SetFmtFilePath(const std::string& fmtFilePath)
{
    this->m_fmtFilePath = fmtFilePath;
}

void OptionContext::SetFileOutputPath(const std::string& fileOutputPath)
{
    this->m_fileOutputPath = fileOutputPath;
}

void OptionContext::SetFmtDirPath(const std::string& fmtDirPath)
{
    this->m_fmtDirPath = fmtDirPath;
}

void OptionContext::SetDirOutputPath(const std::string& dirOutputPath)
{
    this->m_dirOutputPath = dirOutputPath;
}

void OptionContext::SetConfigFilePath(const std::string& configFilePath)
{
    this->m_configFilePath = configFilePath;
}

void OptionContext::SetCodiraHome(const std::string& cangjieHome)
{
    this->m_cangjieHome = cangjieHome;
}

void OptionContext::SetConfigOptions(const FormattingOptions& configOptions)
{
    this->m_configOptions = configOptions;
}

const std::string& OptionContext::GetFmtFilePath() const
{
    return m_fmtFilePath;
}

const std::string& OptionContext::GetFileOutputPath() const
{
    return m_fileOutputPath;
}

const std::string& OptionContext::GetFmtDirPath() const
{
    return m_fmtDirPath;
}

const std::string& OptionContext::GetDirOutputPath() const
{
    return m_dirOutputPath;
}

const std::string& OptionContext::GetConfigFilePath() const
{
    return m_configFilePath;
}

const std::string& OptionContext::GetCodiraHome() const
{
    return m_cangjieHome;
}

FormattingOptions OptionContext::GetConfigOptions() const noexcept
{
    return m_configOptions;
}

