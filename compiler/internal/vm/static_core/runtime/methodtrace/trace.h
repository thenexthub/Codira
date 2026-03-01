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

#ifndef PANDA_RUNTIME_DPROFILER_TRACE_H_
#define PANDA_RUNTIME_DPROFILER_TRACE_H_

#include "libarkbase/macros.h"
#include "include/mem/panda_containers.h"
#include "include/mem/panda_string.h"
#include "include/mem/panda_smart_pointers.h"
#include "include/runtime.h"
#include "include/runtime_notification.h"
#include "runtime/include/method.h"
#include "runtime/include/language_context.h"
#include "libarkbase/os/mutex.h"
#include <memory>
#include <string>
#include <unordered_set>

namespace ark {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
extern os::memory::Mutex g_traceLock;

enum EventFlag {
    TRACE_METHOD_ENTER = 0x00,
    TRACE_METHOD_EXIT = 0x01,
    TRACE_METHOD_UNWIND = 0x02,
};
class Trace : public RuntimeListener {
public:
    static constexpr size_t ENCODE_EVENT_BITS = 2;
    static void WriteDataByte(uint8_t *data, uint64_t value, uint8_t size)
    {
        for (uint8_t i = 0; i < size; i++) {
            *data = static_cast<uint8_t>(value);
            value = value >> WRITE_LENGTH;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            data++;
        }
    }

    static uint64_t GetDataFromBuffer(const uint8_t *buffer, size_t num)
    {
        uint64_t data = 0;
        for (size_t i = 0; i < num; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic, readability-magic-numbers)
            data |= static_cast<uint64_t>(buffer[i]) << (i * 8U);
        }
        return data;
    }

    static void StartTracing(const char *traceFilename, size_t bufferSize);

    static void TriggerTracing();

    void MethodEntry(ManagedThread *thread, Method *method) override;
    void MethodExit(ManagedThread *thread, Method *method) override;

    void SaveTracingData();

    static void StopTracing();

    static bool isTracing_;

    ~Trace() override;

    uint64_t GetAverageTime();

protected:
    explicit Trace(PandaUniquePtr<ark::os::unix::file::File> traceFile, size_t bufferSize);
    uint32_t EncodeMethodToId(Method *method);
    virtual PandaString GetThreadName(ManagedThread *thread) = 0;
    virtual PandaString GetMethodDetailInfo(Method *method) = 0;

private:
    static constexpr size_t TRACE_HEADER_REAL_LENGTH = 18U;

    static const char TRACE_STAR_CHAR = '*';
    static const uint16_t TRACE_HEADER_LENGTH = 32;
    static const uint32_t MAGIC_VALUE = 0x574f4c53;
    static const uint16_t TRACE_VERSION = 3;
    static const uint16_t TRACE_ITEM_SIZE = 14;

    static const uint32_t FILE_SIZE = 8 * 1024 * 1024;

    // used to define the number we need to right shift
    static const uint8_t WRITE_LENGTH = 8;

    // used to define the number of this item we have writed  just now
    const uint8_t numberOf2Bytes_ = 2;
    const uint8_t numberOf4Bytes_ = 4;
    const uint8_t numberOf8Bytes_ = 8;

    const uint32_t loopNumber_ = 10000;
    const uint32_t divideNumber_ = 10;

    PandaUniquePtr<RuntimeListener> listener_;

    os::memory::Mutex methodsLock_;
    // all methods are encoded to id, and put method、id into this map
    PandaMap<Method *, uint32_t> methodIdPandamap_ GUARDED_BY(methodsLock_);
    PandaVector<Method *> methodsCalledVector_ GUARDED_BY(methodsLock_);

    os::memory::Mutex threadInfoLock_;
    PandaSet<PandaString> threadInfoSet_ GUARDED_BY(threadInfoLock_);

    uint32_t EncodeMethodAndEventToId(Method *method, EventFlag flag);
    Method *DecodeIdToMethod(uint32_t id);

    void GetCalledMethods(size_t endOffset, PandaSet<Method *> *calledMethods);

    void GetTimes(uint32_t *threadTime, uint32_t *realTime);

    void WriteInfoToBuf(const ManagedThread *thread, Method *method, EventFlag event, uint32_t threadTime,
                        uint32_t realTime);

    void RecordThreadsInfo(PandaOStringStream *os);
    void RecordMethodsInfo(PandaOStringStream *os, const PandaSet<Method *> &calledMethods);

    void GetUniqueMethods(size_t bufferLength, PandaSet<Method *> *calledMethodsSet);

    static Trace *volatile singletonTrace_ GUARDED_BY(g_traceLock);

    PandaUniquePtr<ark::os::unix::file::File> traceFile_;
    const size_t bufferSize_;

    PandaUniquePtr<uint8_t[]> buffer_;  // NOLINT(modernize-avoid-c-arrays)

    const uint64_t traceStartTime_;

    const uint64_t baseCpuTime_;

    std::atomic<uint32_t> bufferOffset_;

    bool overbrim_ {false};

    static LanguageContext ctx_;

    NO_COPY_SEMANTIC(Trace);
    NO_MOVE_SEMANTIC(Trace);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_DPROFILER_TRACE_H_
