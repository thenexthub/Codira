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

#ifndef INSPECTOR_UTILS_H
#define INSPECTOR_UTILS_H
#include <cstring>
#include <securec.h>
#include <sstream>
#include <string>

#include "jsvm_dfx.h"
#include "jsvm_util.h"
#include "uv.h"
#include "v8-inspector.h"

#ifdef __GNUC__
#define MUST_USE_RESULT __attribute__((warn_unused_result))
#else
#define MUST_USE_RESULT
#endif

namespace jsvm {
namespace inspector {
template<typename Inner, typename Outer>
class ContainerOfHelper {
public:
    // The helper is for doing safe downcasts from base types to derived types.
    inline ContainerOfHelper(Inner Outer::* field, Inner* pointer);
    template<typename TypeName>
    inline operator TypeName*() const;

private:
    Outer* const pointer;
};

inline char ToLower(char c)
{
    return std::tolower(c, std::locale::classic());
}

inline bool StringEqualNoCase(const char* a, const char* b)
{
    while (ToLower(*a) == ToLower(*b++)) {
        if (*a++ == '\0') {
            return true;
        }
    }
    return false;
}

inline bool StringEqualNoCaseN(const char* a, const char* b, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        if (ToLower(a[i]) != ToLower(b[i])) {
            return false;
        }
        if (a[i] == '\0') {
            return true;
        }
    }
    return true;
}

// Use this when a variable or parameter is unused in order to explicitly
// silence a compiler warning about that.
template<typename T>
inline void USE(T&&)
{}

template<typename T, void (*function)(T*)>
struct FunctionDeleter {
    void operator()(T* pointer) const
    {
        function(pointer);
    }
    typedef std::unique_ptr<T, FunctionDeleter> Pointer;
};

template<typename T, void (*function)(T*)>
using DeleteFnPtr = typename FunctionDeleter<T, function>::Pointer;

template<typename Inner, typename Outer>
constexpr uintptr_t OffsetOf(Inner Outer::* field)
{
    return reinterpret_cast<uintptr_t>(&(static_cast<Outer*>(nullptr)->*field));
}

template<typename Inner, typename Outer>
ContainerOfHelper<Inner, Outer>::ContainerOfHelper(Inner Outer::* field, Inner* pointer)
    : pointer(reinterpret_cast<Outer*>(reinterpret_cast<uintptr_t>(pointer) - OffsetOf(field)))
{}

template<typename Inner, typename Outer>
template<typename TypeName>
ContainerOfHelper<Inner, Outer>::operator TypeName*() const
{
    return static_cast<TypeName*>(pointer);
}

// Calculate the address of the outer (i.e. embedding) struct from
// the interior pointer to a data member.
template<typename Inner, typename Outer>
constexpr ContainerOfHelper<Inner, Outer> ContainerOf(Inner Outer::* field, Inner* pointer)
{
    return ContainerOfHelper<Inner, Outer>(field, pointer);
}

template<typename T, size_t kStackStorageSize = 1024>
class MaybeStackBuffer {
public:
    const T* Out() const
    {
        return buf;
    }

    T* Out()
    {
        return buf;
    }

    // operator* for compatibility with `v8::String::(Utf8)Value`
    T* operator*()
    {
        return buf;
    }

    const T* operator*() const
    {
        return buf;
    }

    T& operator[](size_t index)
    {
        CHECK_LT(index, GetLength());
        return buf[index];
    }

    const T& operator[](size_t index) const
    {
        CHECK_LT(index, GetLength());
        return buf[index];
    }

    size_t GetLength() const
    {
        return length;
    }

    // Current maximum capacity of the buffer with which SetLength() can be used
    // without first calling AllocateSufficientStorage().
    size_t GetCapacity() const
    {
        return capacity;
    }

    void AllocateSufficientStorage(size_t storage);

    void SetLength(size_t lengthParam)
    {
        // GetCapacity() returns how much memory is actually available.
        CHECK_LE(lengthParam, GetCapacity());
        length = lengthParam;
    }

    void SetLengthAndZeroTerminate(size_t len)
    {
        // GetCapacity() returns how much memory is actually available.
        CHECK_LE(len + 1, GetCapacity());
        SetLength(len);

        // T() is 0 for integer types, nullptr for pointers, etc.
        buf[len] = T();
    }

    void Invalidate()
    {
        CHECK(!IsAllocated());
        capacity = 0;
        length = 0;
        buf = nullptr;
    }

    // If the buffer is stored in the heap rather than on the stack.
    bool IsAllocated() const
    {
        return !IsInvalidated() && buf != bufSt;
    }

    // If Invalidate() has been called.
    bool IsInvalidated() const
    {
        return buf == nullptr;
    }

    // Release ownership of the malloc'd buffer.
    // Note: This does not free the buffer.
    void Release()
    {
        CHECK(IsAllocated());
        buf = bufSt;
        length = 0;
        capacity = jsvm::ArraySize(bufSt);
    }

    MaybeStackBuffer() : length(0), capacity(jsvm::ArraySize(bufSt)), buf(bufSt)
    {
        // Default to a zero-length, null-terminated buffer.
        buf[0] = T();
    }

    explicit MaybeStackBuffer(size_t storage) : MaybeStackBuffer()
    {
        AllocateSufficientStorage(storage);
    }

    ~MaybeStackBuffer()
    {
        if (IsAllocated()) {
            free(buf);
        }
    }

    inline std::basic_string<T> ToString() const
    {
        return { Out(), GetLength() };
    }
    inline std::basic_string_view<T> ToStringView() const
    {
        return { Out(), GetLength() };
    }

private:
    size_t length;
    // capacity of the malloc'ed buf
    size_t capacity;
    T* buf;
    T bufSt[kStackStorageSize];
};

class TwoByteValue : public MaybeStackBuffer<uint16_t> {
public:
    explicit TwoByteValue(v8::Isolate* isolate, v8::Local<v8::Value> value);
};

template<typename T>
T* Realloc(T* pointer, size_t n, size_t oldN)
{
    CHECK(n > 0);
    size_t newSize = sizeof(T) * n;
    T* newPtr = static_cast<T*>(malloc(newSize));

    CHECK_NOT_NULL(newPtr);

    if (!pointer) {
        return newPtr;
    }

    size_t oldSize = sizeof(T) * oldN;
    CHECK(newSize > oldSize);
    errno_t ret = memcpy_s(newPtr, newSize, pointer, oldSize);
    CHECK(ret == EOK);
    free(pointer);

    return newPtr;
}

template<typename T, size_t kStackStorageSize>
void MaybeStackBuffer<T, kStackStorageSize>::AllocateSufficientStorage(size_t storage)
{
    CHECK(!IsInvalidated());
    if (storage > GetCapacity()) {
        bool wasAllocated = IsAllocated();
        T* allocatedPtr = wasAllocated ? buf : nullptr;
        buf = Realloc(allocatedPtr, storage, length);
        capacity = storage;
        if (!wasAllocated && length > 0) {
            int ret = memcpy_s(buf, length * sizeof(buf[0]), bufSt, length * sizeof(buf[0]));
            CHECK(ret == EOK);
        }
    }

    length = storage;
}

// Convertion between v8_inspector::StringView and std::string
std::string StringViewToUtf8(v8_inspector::StringView view);
std::unique_ptr<v8_inspector::StringBuffer> Utf8ToStringView(const std::string_view message);

constexpr size_t TO_TRANSFORM_CHAR_NUM = 3;
constexpr size_t TRANSFORMED_CHAR_NUM = 4;

enum ByteOffset : uint8_t {
    BYTE_0 = 0,
    BYTE_1 = 1,
    BYTE_2 = 2,
    BYTE_3 = 3,
    BYTE_4 = 4,
    BYTE_5 = 5,
    BYTE_6 = 6,
    BYTE_7 = 7,
    BIT_0 = 0,
    BIT_1 = 1,
    BIT_2 = 2,
    BIT_3 = 3,
    BIT_4 = 4,
    BIT_5 = 5,
    BIT_6 = 6,
    BIT_7 = 7,
    BIT_8 = 8,
};

// Encode base64
inline constexpr size_t Base64EncodeSize(size_t size)
{
    return ((size + TO_TRANSFORM_CHAR_NUM - 1) / TO_TRANSFORM_CHAR_NUM * TRANSFORMED_CHAR_NUM);
}

// Be careful: If dlen is less than expected encode size, it will crash.
size_t Base64Encode(const char* inputString, size_t slen, char* outputBuffer, size_t dlen);

std::string GetHumanReadableProcessName();

// Either succeeds with exactly |length| bytes of cryptographically
// strong pseudo-random data, or fails. This function may block.
// Don't assume anything about the contents of |buffer| on error.
// As a special case, |length == 0| can be used to check if the CSPRNG
// is properly seeded without consuming entropy.
MUST_USE_RESULT bool CSPRNG(void* buffer, size_t length);

void CheckedUvLoopClose(uv_loop_t* loop);
} // namespace inspector
} // namespace jsvm
#endif
