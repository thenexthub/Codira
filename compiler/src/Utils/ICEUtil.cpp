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
 * This file implements ICE related variables and functions.
 */

#include "Codira/Utils/ICEUtil.h"

#include "Codira/Basic/Version.h"
#include "Codira/Driver/TempFileManager.h"
#include "Codira/Frontend/CompilerInstance.h"

namespace {
using namespace Codira;

std::atomic<bool> g_writeOnceICEMessag(false);
} // namespace
namespace Codira {
namespace ICE {

int64_t TriggerPointSetter::triggerPoint = static_cast<int64_t>(Codira::CompileStage::COMPILE_STAGE_NUMBER);
int64_t TriggerPointSetter::interpreterTP = static_cast<int64_t>(Codira::CompileStage::COMPILE_STAGE_NUMBER) + 1;
int64_t TriggerPointSetter::writeCahedTP = static_cast<int64_t>(Codira::CompileStage::COMPILE_STAGE_NUMBER) + 2;

void TriggerPointSetter::SetICETriggerPoint(CompileStage cs)
{
    if (TriggerPointSetter::triggerPoint == Codira::ICE::LSP_TP) {
        return;
    }
    if (cs >= CompileStage::COMPILE_STAGE_NUMBER) {
        TriggerPointSetter::triggerPoint = static_cast<int64_t>(CompileStage::COMPILE_STAGE_NUMBER);
    } else {
        TriggerPointSetter::triggerPoint = static_cast<int64_t>(cs);
    }
}

void TriggerPointSetter::SetICETriggerPoint(int64_t tp)
{
    if (tp == Codira::ICE::LSP_TP) {
        TriggerPointSetter::triggerPoint = tp;
        return;
    }
    if (tp == FRONTEND_TP) {
        TriggerPointSetter::triggerPoint = static_cast<int64_t>(CompileStage::COMPILE_STAGE_NUMBER);
    } else {
        TriggerPointSetter::triggerPoint = tp;
    }
}

int64_t GetTriggerPoint()
{
    return TriggerPointSetter::triggerPoint;
}

bool CanWriteOnceICEMessage()
{
    return !g_writeOnceICEMessag.exchange(true);
}

void PrintVersionFromError()
{
    std::cerr << CODIRA_COMPILER_VERSION << std::endl;
}

void RemoveTempFile()
{
    Codira::TempFileManager::Instance().DeleteTempFiles();
}

} // namespace ICE

} // namespace Codira
