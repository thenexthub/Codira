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

#include "debug_step_flags.h"

DebugStepFlags& DebugStepFlags::Get()
{
    static DebugStepFlags instance;
    return instance;
}

// 设置函数实现
void DebugStepFlags::SetDyn2StatInto(bool value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    dyn2statInto_ = value;
}

void DebugStepFlags::SetDyn2StatOut(bool value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    dyn2statOut_ = value;
}

void DebugStepFlags::SetDyn2StatOver(bool value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    dyn2statOver_ = value;
}

void DebugStepFlags::SetStat2DynInto(bool value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    stat2dynInto_ = value;
}

void DebugStepFlags::SetStat2DynOut(bool value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    stat2dynOut_ = value;
}

void DebugStepFlags::SetStat2DynOver(bool value)
{
    std::lock_guard<std::mutex> lock(mutex_);
    stat2dynOver_ = value;
}

bool DebugStepFlags::GetDyn2StatInto()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return dyn2statInto_;
}

bool DebugStepFlags::GetDyn2StatOut()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return dyn2statOut_;
}

bool DebugStepFlags::GetDyn2StatOver()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return dyn2statOver_;
}

bool DebugStepFlags::GetStat2DynInto()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return stat2dynInto_;
}

bool DebugStepFlags::GetStat2DynOut()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return stat2dynOut_;
}

bool DebugStepFlags::GetStat2DynOver()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return stat2dynOver_;
}