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

#include <chrono>
#include <cmath>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/time.h"
#include "runtime/compiler.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/thread_status.h"
#include "runtime/interpreter/frame.h"
#include "runtime/coroutines/coroutine_manager.h"
#include "libarkbase/utils/math_helpers.h"
#include "intrinsics.h"

namespace ark::intrinsics {

uint8_t IsInfF64(double v)
{
    return static_cast<uint8_t>(std::isinf(v));
}

uint8_t IsInfF32(float v)
{
    return static_cast<uint8_t>(std::isinf(v));
}

int32_t AbsI32(int32_t v)
{
    return std::abs(v);
}

int64_t AbsI64(int64_t v)
{
    return std::abs(v);
}

float AbsF32(float v)
{
    return std::abs(v);
}

double AbsF64(double v)
{
    return std::abs(v);
}

float SinF32(float v)
{
    return std::sin(v);
}

double SinF64(double v)
{
    return std::sin(v);
}

float CosF32(float v)
{
    return std::cos(v);
}

double CosF64(double v)
{
    return std::cos(v);
}

float PowF32(float base, float exp)
{
    return std::pow(base, exp);
}

double PowF64(double base, double exp)
{
    return std::pow(base, exp);
}

float SqrtF32(float v)
{
    return std::sqrt(v);
}

double SqrtF64(double v)
{
    return std::sqrt(v);
}

int32_t MinI32(int32_t a, int32_t b)
{
    return std::min(a, b);
}

int64_t MinI64(int64_t a, int64_t b)
{
    return std::min(a, b);
}

float MinF32(float a, float b)
{
    return ark::helpers::math::Min(a, b);
}

double MinF64(double a, double b)
{
    return ark::helpers::math::Min(a, b);
}

int32_t MaxI32(int32_t a, int32_t b)
{
    return std::max(a, b);
}

int64_t MaxI64(int64_t a, int64_t b)
{
    return std::max(a, b);
}

float MaxF32(float a, float b)
{
    return ark::helpers::math::Max(a, b);
}

double MaxF64(double a, double b)
{
    return ark::helpers::math::Max(a, b);
}

template <bool IS_ERR, class T>
void Print(T v)
{
    if (IS_ERR) {
        std::cerr << v;
    } else {
        std::cout << v;
    }
}

template <bool IS_ERR>
void PrintStringInternal(coretypes::String *v)
{
    static auto &outstream = IS_ERR ? std::cerr : std::cout;
    size_t len = v->GetUtf8Length();
    PandaVector<uint8_t> out(len);
    v->CopyDataRegionUtf8(out.data(), 0, len, len);
    outstream << std::string_view(reinterpret_cast<const char *>(out.data()), out.size());
}

void PrintString(coretypes::String *v)
{
    PrintStringInternal<false>(v);
}

void PrintF32(float v)
{
    Print<false>(v);
}

void PrintF64(double v)
{
    Print<false>(v);
}

void PrintI32(int32_t v)
{
    Print<false>(v);
}

void PrintU32(uint32_t v)
{
    Print<false>(v);
}

void PrintI64(int64_t v)
{
    Print<false>(v);
}

void PrintU64(uint64_t v)
{
    Print<false>(v);
}

int64_t NanoTime()
{
    return time::GetCurrentTimeInNanos();
}

void Assert(uint8_t cond)
{
    if (cond == 0) {
        Runtime::Abort();
    }
}

void UnknownIntrinsic()
{
    std::cerr << "UnknownIntrinsic\n";
    Runtime::Abort();
}

void AssertPrint(uint8_t cond, coretypes::String *s)
{
    if (cond == 0) {
        PrintStringInternal<true>(s);
        Runtime::Abort();
    }
}

void CheckTag(int64_t reg, int64_t expected)
{
    constexpr int ACC_NUM = std::numeric_limits<uint16_t>::max() + 1;
    int64_t tag = 0;
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    StaticFrameHandler handler(thread->GetCurrentFrame());
    if (reg == ACC_NUM) {
        auto vreg = handler.GetAcc();
        tag = vreg.GetTag();
    } else {
        auto vreg = handler.GetVReg(reg);
        tag = vreg.GetTag();
    }
    if (tag != expected) {
        std::cerr << "Error: "
                  << "Tag = " << tag << std::endl;
        std::cerr << "Expected = " << expected << std::endl;
        ark::PrintStack(std::cerr);
        Runtime::Abort();
    }
}

#ifndef PANDA_PRODUCT_BUILD
uint8_t CompileMethod(coretypes::String *fullMethodName)
{
    return ark::CompileMethodImpl(fullMethodName, panda_file::SourceLang::PANDA_ASSEMBLY);
}

double CalculateDouble(uint32_t n, double s)
{
    double result = 0.0;
    for (uint32_t i = 0U; i < n; ++i) {
        result += i * s;
    }
    return result;
}

float CalculateFloat(uint32_t n, float s)
{
    float result = 0.0F;
    for (uint32_t i = 0U; i < n; ++i) {
        result += i * s;
    }
    return result;
}
#endif  // PANDA_PRODUCT_BUILD

// NOTE(kbaladurin) : Convert methods should be implemented in managed library

int32_t ConvertStringToI32(coretypes::String *s)
{
    return static_cast<int32_t>(PandaStringToLL(ConvertToString(s)));
}

uint32_t ConvertStringToU32(coretypes::String *s)
{
    return static_cast<uint32_t>(PandaStringToULL(ConvertToString(s)));
}

int64_t ConvertStringToI64(coretypes::String *s)
{
    return static_cast<int64_t>(PandaStringToLL(ConvertToString(s)));
}

uint64_t ConvertStringToU64(coretypes::String *s)
{
    return static_cast<uint64_t>(PandaStringToULL(ConvertToString(s)));
}

float ConvertStringToF32(coretypes::String *s)
{
    return PandaStringToF(ConvertToString(s));
}

double ConvertStringToF64(coretypes::String *s)
{
    return PandaStringToD(ConvertToString(s));
}

void SystemExit(int32_t status)
{
    Runtime::Halt(status);
}

void SystemScheduleCoroutine()
{
    auto *coro = Coroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *cm = static_cast<CoroutineManager *>(coro->GetVM()->GetThreadManager());
    cm->Schedule();
}

int32_t SystemCoroutineGetWorkerId()
{
    // return os::thread::GetCurrentThreadId();
    return Coroutine::GetCurrent()->GetWorker()->GetId();
}

ObjectHeader *ObjectCreateNonMovable(coretypes::Class *cls)
{
    ASSERT(cls != nullptr);
    return ObjectHeader::CreateNonMovable(cls->GetRuntimeClass());
}

void ObjectMonitorEnter(ObjectHeader *header)
{
    if (header == nullptr) {
        ark::ThrowNullPointerException();
        return;
    }
    auto res = Monitor::MonitorEnter(header);
    // Expected results: OK, ILLEGAL
    ASSERT(res != Monitor::State::INTERRUPTED);
    if (UNLIKELY(res != Monitor::State::OK)) {
        // This should never happen
        // Object 'header' may be moved. It is no sence to wrap it into a handle just to print before exit
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        LOG(FATAL, RUNTIME) << "MonitorEnter for " << header << " returned Illegal state!";
    }
}

void ObjectMonitorExit(ObjectHeader *header)
{
    if (header == nullptr) {
        ark::ThrowNullPointerException();
        return;
    }
    auto res = Monitor::MonitorExit(header);
    // Expected results: OK, ILLEGAL
    ASSERT(res != Monitor::State::INTERRUPTED);
    if (res == Monitor::State::ILLEGAL) {
        PandaStringStream ss;
        ss << "MonitorExit for object " << std::hex << header << " returned Illegal state";
        ark::ThrowIllegalMonitorStateException(ss.str());
    }
}

void ObjectWait(ObjectHeader *header)
{
    Monitor::State state = Monitor::Wait(header, ThreadStatus::IS_WAITING, 0, 0);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
}

void ObjectTimedWait(ObjectHeader *header, uint64_t timeout)
{
    Monitor::State state = Monitor::Wait(header, ThreadStatus::IS_TIMED_WAITING, timeout, 0);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
}

void ObjectTimedWaitNanos(ObjectHeader *header, uint64_t timeout, uint64_t nanos)
{
    Monitor::State state = Monitor::Wait(header, ThreadStatus::IS_TIMED_WAITING, timeout, nanos);
    LOG_IF(state == Monitor::State::ILLEGAL, FATAL, RUNTIME) << "Monitor::Wait() failed";
}

void ObjectNotify(ObjectHeader *header)
{
    Monitor::State state = Monitor::Notify(header);
    LOG_IF(state != Monitor::State::OK, FATAL, RUNTIME) << "Monitor::Notify() failed";
}

void ObjectNotifyAll(ObjectHeader *header)
{
    Monitor::State state = Monitor::NotifyAll(header);
    LOG_IF(state != Monitor::State::OK, FATAL, RUNTIME) << "Monitor::NotifyAll() failed";
}

void Memset8(ObjectHeader *array, uint8_t value, uint32_t initialIndex, uint32_t maxIndex)
{
    auto data = reinterpret_cast<uint8_t *>(ark::coretypes::Array::Cast(array)->GetData());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::fill(data + initialIndex, data + maxIndex, value);
}

void Memset16(ObjectHeader *array, uint16_t value, uint32_t initialIndex, uint32_t maxIndex)
{
    auto data = reinterpret_cast<uint16_t *>(ark::coretypes::Array::Cast(array)->GetData());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::fill(data + initialIndex, data + maxIndex, value);
}

void Memset32(ObjectHeader *array, uint32_t value, uint32_t initialIndex, uint32_t maxIndex)
{
    auto data = reinterpret_cast<uint32_t *>(ark::coretypes::Array::Cast(array)->GetData());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::fill(data + initialIndex, data + maxIndex, value);
}

void Memset64(ObjectHeader *array, uint64_t value, uint32_t initialIndex, uint32_t maxIndex)
{
    auto data = reinterpret_cast<uint64_t *>(ark::coretypes::Array::Cast(array)->GetData());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::fill(data + initialIndex, data + maxIndex, value);
}
void Memsetf32(ObjectHeader *array, float value, uint32_t initialIndex, uint32_t maxIndex)
{
    auto data = reinterpret_cast<float *>(ark::coretypes::Array::Cast(array)->GetData());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::fill(data + initialIndex, data + maxIndex, value);
}

void Memsetf64(ObjectHeader *array, double value, uint32_t initialIndex, uint32_t maxIndex)
{
    auto data = reinterpret_cast<double *>(ark::coretypes::Array::Cast(array)->GetData());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::fill(data + initialIndex, data + maxIndex, value);
}
}  // namespace ark::intrinsics

#include "core_any_intrinsics.inc"

#include "runtime/entrypoints/entrypoints.h"

#include <intrinsics_gen.h>
