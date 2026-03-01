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

#include "UserBase.h"

#include <fstream>
#include <iostream>

#include "Codira/Utils/CheckUtils.h"
#include "Codira/Utils/FileUtil.h"

using namespace Codira;

std::string UserBase::GetResult() const
{
    if (!enable) {
        return "";
    }
    return GetJson();
}

void UserBase::OutputResult() const noexcept
{
    if (!enable) {
        return;
    }
#ifndef CODIRA_ENABLE_GCOV
    try {
#endif
        std::string result = GetResult();
        WriteJson(result, GetSuffix());
#ifndef CODIRA_ENABLE_GCOV
    } catch (...) {
        std::cerr << "Get an exception while running function 'OutputResult' !!!\n" << std::endl;
    }
#endif
}

void UserBase::WriteJson(const std::string& context, const std::string& suffix) const
{
    std::string name = packageName;
    size_t startPos = name.find('/');
    if (startPos != std::string::npos) {
        (void)name.replace(startPos, 1, "-");
    }
    std::string filename = name + suffix;
    std::ofstream out(FileUtil::JoinPath(outputDir, filename).c_str());
    if (!out) {
        return;
    }
    out.write(context.c_str(), static_cast<long>(context.size()));
    out.close();
}

void UserBase::Enable(bool en)
{
    enable = en;
}

bool UserBase::IsEnable() const
{
    return enable;
}

void UserBase::SetPackageName(const std::string& name)
{
    packageName = name;
}

void UserBase::SetOutputDir(const std::string& path)
{
    if (FileUtil::IsDir(path)) {
        outputDir = path;
    } else {
        outputDir = FileUtil::GetDirPath(path);
    }
}
