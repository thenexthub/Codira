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

#include "Codira/Utils/ProfileRecorder.h"

#include "UserBase.h"
#include "UserCodeInfo.h"
#include "UserMemoryUsage.h"
#include "UserTimer.h"

using namespace Codira;
using namespace Codira::Utils;

namespace {
inline bool operator&(ProfileRecorder::Type a, ProfileRecorder::Type b)
{
    return static_cast<bool>(static_cast<uint16_t>(a) & static_cast<uint16_t>(b));
}
} // namespace

ProfileRecorder::ProfileRecorder(
    const std::string& title, const std::string& subtitle, const std::string& desc)
    : title_(title), subtitle_(subtitle), desc_(desc)
{
    Start(title, subtitle, desc);
}

ProfileRecorder::~ProfileRecorder()
{
#ifndef CODIRA_ENABLE_GCOV
    try {
#endif
        Stop(title_, subtitle_, desc_);
#ifndef CODIRA_ENABLE_GCOV
    } catch (...) {
        // The try-catch block is placed here as the used functions above do not declare with noexcept.
        // We shall avoid any exception occurring in the destruction function.
    }
#endif
}

void ProfileRecorder::SetPackageName(const std::string& name)
{
    UserTimer::Instance().SetPackageName(name);
    UserMemoryUsage::Instance().SetPackageName(name);
    UserCodeInfo::Instance().SetPackageName(name);
}

void ProfileRecorder::SetOutputDir(const std::string& path)
{
    UserTimer::Instance().SetOutputDir(path);
    UserMemoryUsage::Instance().SetOutputDir(path);
    UserCodeInfo::Instance().SetOutputDir(path);
}

void ProfileRecorder::Start(
    const std::string& title, const std::string& subtitle, const std::string& desc)
{
    if (UserTimer::Instance().IsEnable()) {
        UserTimer::Instance().Start(title, subtitle, desc);
    }
    if (UserMemoryUsage::Instance().IsEnable()) {
        UserMemoryUsage::Instance().Start(title, subtitle, desc);
    }
}

void ProfileRecorder::Stop(
    const std::string& title, const std::string& subtitle, const std::string& desc)
{
    if (UserTimer::Instance().IsEnable()) {
        UserTimer::Instance().Stop(title, subtitle, desc);
    }
    if (UserMemoryUsage::Instance().IsEnable()) {
        UserMemoryUsage::Instance().Stop(title, subtitle, desc);
    }
}

void ProfileRecorder::RecordCodeInfo(const std::string& item, const std::function<int64_t(void)>& getData)
{
    if (UserTimer::Instance().IsEnable() || UserMemoryUsage::Instance().IsEnable()) {
        UserCodeInfo::Instance().RecordInfo(item, getData());
    }
}

void ProfileRecorder::RecordCodeInfo(const std::string& item, int64_t value)
{
    UserCodeInfo::Instance().RecordInfo(item, value);
}

void ProfileRecorder::Enable(bool en, const Type& type)
{
    if (type & ProfileRecorder::Type::TIMER) {
        UserTimer::Instance().Enable(en);
    }
    if (type & ProfileRecorder::Type::MEMORY) {
        UserMemoryUsage::Instance().Enable(en);
    }
    if (UserTimer::Instance().IsEnable() || UserMemoryUsage::Instance().IsEnable()) {
        UserCodeInfo::Instance().Enable(true);
    } else {
        UserCodeInfo::Instance().Enable(false);
    }
}

std::string ProfileRecorder::GetResult(const Type& type)
{
    std::string result;

    if ((type & ProfileRecorder::Type::TIMER) || (type & ProfileRecorder::Type::MEMORY)) {
        result += UserCodeInfo::Instance().GetResult();
    }
    if (type & ProfileRecorder::Type::TIMER) {
        result += UserTimer::Instance().GetResult();
    }
    if (type & ProfileRecorder::Type::MEMORY) {
        result += UserMemoryUsage::Instance().GetResult();
    }
    return result;
}
