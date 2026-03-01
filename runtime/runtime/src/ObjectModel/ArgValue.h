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


#ifndef MRT_ARGVALUE_H
#define MRT_ARGVALUE_H

namespace MapleRuntime {
#if defined(__linux__) && defined(__x86_64__)
constexpr U32 kXregSize = 6;
constexpr U32 kDregSize = 8;
#elif defined(__aarch64__)
constexpr U32 kXregSize = 8;
constexpr U32 kDregSize = 8;
#elif defined(__arm__)
constexpr U32 kXregSize = 4;
constexpr U32 kDregSize = 8;
#elif defined(_WIN64)
constexpr U32 kXregSize = 4;
constexpr U32 kDregSize = 4;
#elif defined(__APPLE__) && defined(__x86_64__)
constexpr U32 kXregSize = 6;
constexpr U32 kDregSize = 8;
#endif
constexpr U32 kXregAndDregSize = kXregSize + kDregSize;
constexpr U32 kRegArgsSize = kXregAndDregSize;
constexpr U32 kIncSize = 8;

using Value = union Values {
    U1 b;
    I8 a;
    I16 s;
    I32 i;
    I64 l;
    Size q;

    I8 u;
    U8 h;
    U16 t;
    U32 c;
    U64 m;
    USize r;

    F16 dh;
    F32 f;
    F64 d;
    BaseObject* ref;
};
class ArgValue {
public:
    ArgValue()
    {
        fregIdx = kXregSize;
        stackIdx = kXregAndDregSize;
        valuesSize = kRegArgsSize;
        gregIdx = 0;
        values = regArgValues;
        for (U32 idx = 0; idx < kRegArgsSize; ++idx) {
            regArgValues[idx].m = 0UL;
        }
    }
    ~ArgValue()
    {
        if (valuesSize > kRegArgsSize) {
            delete []values;
        }
    }

    Value* GetData()
    {
        return &values[0];
    }

    void AddReference(BaseObject* ref)
    {
        if (gregIdx < kXregSize) {
#if defined(_WIN64)
            values[gregIdx++].ref = ref;
            fregIdx++;
#else
            values[gregIdx++].ref = ref;
#endif
        } else {
            Resize(stackIdx + 1);
            values[stackIdx++].ref = ref;
        }
    }

    void AddInt64(I64 value)
    {
        if (gregIdx < kXregSize) {
#if defined(_WIN64)
            values[gregIdx++].l = value;
            fregIdx++;
#else
            values[gregIdx++].l = value;
#endif
        } else {
            Resize(stackIdx + 1);
            values[stackIdx++].l = value;
        }
    }

    void AddFloat64(F64 value)
    {
        if (fregIdx < kRegArgsSize) {
#if defined(_WIN64)
            values[fregIdx++].d = value;
            gregIdx++;
#else
            values[fregIdx++].d = value;
#endif
        } else {
            Resize(stackIdx + 1);
            values[stackIdx++].d = value;
        }
    }
    
    U32 GetGregIdx()
    {
        return gregIdx;
    }
    U32 GetFregIdx()
    {
        return gregIdx - kXregSize;
    }
    U32 GetStackSize()
    {
        // 8: means the size of each type in stack
#ifdef __arm__
        return (stackIdx - kRegArgsSize) * 4;
#else
        return (stackIdx - kRegArgsSize) * 8;
#endif
    }
    U32 GetStackIdx()
    {
        return stackIdx;
    }
private:
    void Resize(uint32_t newSize)
    {
        if (newSize < valuesSize) {
            return;
        }

        Value *oldValues = values;
        uint32_t oldValuesSize = valuesSize;
        valuesSize = newSize + kIncSize;
        values = new (std::nothrow) Value[valuesSize];
        CHECK_DETAIL(values != nullptr, "new Value alloc values fail");
        std::copy(oldValues, oldValues + oldValuesSize, values);
        if (oldValues != regArgValues) {
            delete []oldValues; // free dynamic array.
        }
    }
    U32 valuesSize;
    Value regArgValues[kRegArgsSize];
    Value *values;
    U32 gregIdx;
    U32 fregIdx;
    U32 stackIdx;
};
} // namespace MapleRuntime
#endif // MRT_ARGVALUE_H
