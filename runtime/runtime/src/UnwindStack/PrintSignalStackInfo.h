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


#ifndef MRT_PRINT_SIGNALSTACKINFO_H
#define MRT_PRINT_SIGNALSTACKINFO_H

#include "Base/LogFile.h"
#include "PrintStackInfo.h"

namespace MapleRuntime {
class PrintSignalStackInfo : public PrintStackInfo {
public:
    explicit PrintSignalStackInfo(const UnwindContext* context = nullptr) : PrintStackInfo(context), stackIndex(0) {}

    ~PrintSignalStackInfo() override = default;
    void FillInStackTrace() override;
    void PrintStackTrace() const override;
    SigHandlerFrameinfo* GetSignalStack() { return signalStack; }
    uint16_t GetStackIndex() const { return stackIndex; }
    void SetStackIndex(uint8_t i) { stackIndex = i; }

private:
    constexpr static uint8_t MAX_SIGNAL_STACK_SIZE = 32;
    SigHandlerFrameinfo signalStack[MAX_SIGNAL_STACK_SIZE];
    uint16_t stackIndex;
};
} // namespace MapleRuntime
#endif // MRT_PRINT_SIGNALSTACKINFO_H
