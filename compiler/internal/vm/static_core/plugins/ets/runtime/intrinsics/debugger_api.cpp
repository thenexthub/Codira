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

#include "include/tooling/debug_interface.h"
#include "include/tooling/vreg_value.h"
#include "intrinsics.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/tooling/helpers.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets::intrinsics {

static void SetNotFoundException(EtsLong regNumber, EtsCoroutine *coroutine, std::string_view typeName)
{
    auto errorMsg =
        "No local variable found at vreg #" + std::to_string(regNumber) + " and type " + std::string(typeName);
    ark::ets::ThrowEtsException(coroutine, panda_file_items::class_descriptors::LINKER_UNRESOLVED_FIELD_ERROR.data(),
                                errorMsg);
}

static void SetRuntimeException(EtsLong regNumber, EtsCoroutine *coroutine, std::string_view typeName)
{
    auto errorMsg =
        "Failed to access variable at vreg #" + std::to_string(regNumber) + " and type " + std::string(typeName);
    ark::ets::ThrowEtsException(coroutine, panda_file_items::class_descriptors::ERROR, errorMsg);
}

template <typename T>
static T DebuggerAPIGetLocal(EtsCoroutine *coroutine, EtsLong regNumber)
{
    static constexpr uint32_t PREVIOUS_FRAME_DEPTH = 1;

    ASSERT(coroutine);

    if (regNumber < 0) {
        SetNotFoundException(regNumber, coroutine, ark::ets::tooling::EtsTypeName<T>::NAME);
    }

    ark::tooling::VRegValue vregValue;
    auto &debugger = Runtime::GetCurrent()->StartDebugSession()->GetDebugger();
    auto err = debugger.GetVariable(ark::ets::tooling::CoroutineToPtThread(coroutine), PREVIOUS_FRAME_DEPTH, regNumber,
                                    &vregValue);
    if (err) {
        LOG(ERROR, DEBUGGER) << "Failed to get local variable: " << err.value().GetMessage();
        SetRuntimeException(regNumber, coroutine, ark::ets::tooling::EtsTypeName<T>::NAME);
        return static_cast<T>(0);
    }
    return ark::ets::tooling::VRegValueToEtsValue<T>(vregValue);
}

EtsBoolean DebuggerAPIGetLocalBoolean(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsBoolean>(coroutine, regNumber);
}

EtsByte DebuggerAPIGetLocalByte(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsByte>(coroutine, regNumber);
}

EtsShort DebuggerAPIGetLocalShort(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsShort>(coroutine, regNumber);
}

EtsChar DebuggerAPIGetLocalChar(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsChar>(coroutine, regNumber);
}

EtsInt DebuggerAPIGetLocalInt(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsInt>(coroutine, regNumber);
}

EtsFloat DebuggerAPIGetLocalFloat(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsFloat>(coroutine, regNumber);
}

EtsDouble DebuggerAPIGetLocalDouble(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsDouble>(coroutine, regNumber);
}

EtsLong DebuggerAPIGetLocalLong(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    return DebuggerAPIGetLocal<EtsLong>(coroutine, regNumber);
}

EtsObject *DebuggerAPIGetLocalObject(EtsLong regNumber)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);

    auto *obj = DebuggerAPIGetLocal<ObjectHeader *>(coroutine, regNumber);
    obj = (obj == nullptr) ? coroutine->GetNullValue() : obj;
    VMHandle<ObjectHeader> objHandle(coroutine, obj);
    return EtsObject::FromCoreType(objHandle.GetPtr());
}

template <typename T>
static void DebuggerAPISetLocal(EtsCoroutine *coroutine, EtsLong regNumber, T value)
{
    static constexpr uint32_t PREVIOUS_FRAME_DEPTH = 1;

    ASSERT(coroutine);

    if (regNumber < 0) {
        SetNotFoundException(regNumber, coroutine, ark::ets::tooling::EtsTypeName<T>::NAME);
        return;
    }

    auto vregValue = ark::ets::tooling::EtsValueToVRegValue<T>(value);
    auto &debugger = Runtime::GetCurrent()->StartDebugSession()->GetDebugger();
    auto err = debugger.SetVariable(ark::ets::tooling::CoroutineToPtThread(coroutine), PREVIOUS_FRAME_DEPTH, regNumber,
                                    vregValue);
    if (err) {
        LOG(ERROR, DEBUGGER) << "Failed to set local variable: " << err.value().GetMessage();
        SetRuntimeException(regNumber, coroutine, ark::ets::tooling::EtsTypeName<T>::NAME);
    }
}

void DebuggerAPISetLocalBoolean(EtsLong regNumber, EtsBoolean value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsBoolean>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalByte(EtsLong regNumber, EtsByte value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsByte>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalShort(EtsLong regNumber, EtsShort value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsShort>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalChar(EtsLong regNumber, EtsChar value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsChar>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalInt(EtsLong regNumber, EtsInt value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsInt>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalFloat(EtsLong regNumber, EtsFloat value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsFloat>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalDouble(EtsLong regNumber, EtsDouble value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsDouble>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalLong(EtsLong regNumber, EtsLong value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    DebuggerAPISetLocal<EtsLong>(coroutine, regNumber, value);
}

void DebuggerAPISetLocalObject(EtsLong regNumber, EtsObject *value)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, value->GetCoreType());

    DebuggerAPISetLocal<ObjectHeader *>(coroutine, regNumber, objHandle.GetPtr()->GetCoreType());
}

}  // namespace ark::ets::intrinsics
