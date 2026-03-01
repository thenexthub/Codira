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

#ifndef PANDA_LIBPANDABASE_UTILS_SERIALIZER_SERIALIZER_H_
#define PANDA_LIBPANDABASE_UTILS_SERIALIZER_SERIALIZER_H_

#include <securec.h>

#include "libarkbase/utils/expected.h"
#include "for_each_tuple.h"
#include "tuple_to_struct.h"
#include "struct_to_tuple.h"
#include "libarkbase/concepts.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <cstring>
#include "libarkbase/utils/utils.h"

namespace ark::serializer {

inline uintptr_t ToUintPtr(const uint8_t *p)
{
    return reinterpret_cast<uintptr_t>(p);
}

inline const uint8_t *ToUint8tPtr(uintptr_t v)
{
    return reinterpret_cast<const uint8_t *>(v);
}

template <typename T>
// NOLINTNEXTLINE(google-runtime-references)
inline auto TypeToBuffer(const T &value, std::vector<uint8_t> &buffer)
    -> std::enable_if_t<std::is_pod_v<T>, Expected<size_t, const char *>>
{
    const auto *ptr = reinterpret_cast<const uint8_t *>(&value);
    std::copy(ptr, ToUint8tPtr(ToUintPtr(ptr) + sizeof(value)), std::back_inserter(buffer));
    return sizeof(value);
}

template <class VecT>
// NOLINTNEXTLINE(google-runtime-references)
inline auto TypeToBuffer(const VecT &vec, std::vector<uint8_t> &buffer)
    -> std::enable_if_t<is_vectorable_v<VecT> && std::is_pod_v<typename VecT::value_type>,
                        Expected<size_t, const char *>>
{
    using Type = typename VecT::value_type;
    // pack size
    uint32_t size = vec.size() * sizeof(Type);
    auto ret = TypeToBuffer(size, buffer);
    if (!ret) {
        return ret;
    }

    // pack data
    const auto *ptr = reinterpret_cast<const uint8_t *>(vec.data());
    const uint8_t *ptrEnd = ToUint8tPtr(ToUintPtr(ptr) + size);
    std::copy(ptr, ptrEnd, std::back_inserter(buffer));
    return ret.Value() + size;
}

template <class UnMap>
auto TypeToBuffer(const UnMap &map, std::vector<uint8_t> &buffer)
    -> std::enable_if_t<is_hash_mappable_v<UnMap>, Expected<size_t, const char *>>
{
    // pack size
    auto ret = TypeToBuffer(static_cast<uint32_t>(map.size()), buffer);
    if (!ret) {
        return ret;
    }

    for (const auto &[key, value] : map) {
        // pack key
        auto k = TypeToBuffer(key, buffer);
        if (!k) {
            return k;
        }
        ret.Value() += k.Value();

        // pack value
        auto v = TypeToBuffer(value, buffer);
        if (!v) {
            return v;
        }
        ret.Value() += v.Value();
    }

    return ret;
}

template <typename T>
Expected<size_t, const char *> BufferToType(const uint8_t *data, size_t size, T &out)
{
    static_assert(std::is_pod<T>::value, "Type is not supported");

    if (sizeof(out) > size) {
        return Unexpected("Cannot deserialize POD type, the buffer is too small.");
    }

    auto *ptr = reinterpret_cast<uint8_t *>(&out);
    MemcpyUnsafe(ptr, data, sizeof(out));
    return sizeof(out);
}

// NOLINTNEXTLINE(google-runtime-references)
inline Expected<size_t, const char *> BufferToTypeUnpackString(const uint8_t *data, size_t size, std::string &out,
                                                               uint32_t strSize, size_t value)
{
    if (size < strSize) {
        return Unexpected("Cannot deserialize string, the buffer is too small.");
    }

    out.resize(strSize);
    MemcpyUnsafe(out.data(), data, strSize);
    return value + strSize;
}

// NOLINTNEXTLINE(google-runtime-references)
inline Expected<size_t, const char *> BufferToType(const uint8_t *data, size_t size, std::string &out)
{
    // unpack size
    uint32_t strSize = 0;
    auto r = BufferToType(data, size, strSize);
    if (!r || strSize == 0) {
        return r;
    }
    ASSERT(r.Value() <= size);
    data = ToUint8tPtr(ToUintPtr(data) + r.Value());
    size -= r.Value();
    return BufferToTypeUnpackString(data, size, out, strSize, r.Value());
}

template <typename T>
Expected<size_t, const char *> BufferToType(const uint8_t *data, size_t size, std::vector<T> &vector)
{
    static_assert(std::is_pod<T>::value, "Type is not supported");

    // unpack size
    uint32_t vectorSize = 0;
    auto r = BufferToType(data, size, vectorSize);
    if (!r || vectorSize == 0) {
        return r;
    }
    ASSERT(r.Value() <= size);
    data = ToUint8tPtr(ToUintPtr(data) + r.Value());
    size -= r.Value();
    // unpack data
    if (size < vectorSize || (vectorSize % sizeof(T))) {
        return Unexpected("Cannot deserialize vector, the buffer is too small.");
    }

    vector.resize(vectorSize / sizeof(T));
    MemcpyUnsafe(vector.data(), data, vectorSize);

    return r.Value() + vectorSize;
}

template <typename K, typename V>
Expected<size_t, const char *> BufferToType(const uint8_t *data, size_t size, std::unordered_map<K, V> &out)
{
    size_t backupSize = size;
    uint32_t count = 0;
    auto r = BufferToType(data, size, count);
    if (!r) {
        return r;
    }
    ASSERT(r.Value() <= size);
    data = ToUint8tPtr(ToUintPtr(data) + r.Value());
    size -= r.Value();

    for (uint32_t i = 0; i < count; ++i) {
        K key {};
        auto v = serializer::BufferToType(data, size, key);
        if (!v) {
            return v;
        }
        ASSERT(v.Value() <= size);
        data = ToUint8tPtr(ToUintPtr(data) + v.Value());
        size -= v.Value();

        V value {};
        v = serializer::BufferToType(data, size, value);
        if (!v) {
            return v;
        }
        ASSERT(v.Value() <= size);
        data = ToUint8tPtr(ToUintPtr(data) + v.Value());
        size -= v.Value();

        auto ret = out.emplace(std::make_pair(std::move(key), std::move(value)));
        if (!ret.second) {
            return Unexpected("Cannot emplace KeyValue to map.");
        }
    }
    return backupSize - size;
}

namespace internal {

class Serializer {
public:
    // NOLINTNEXTLINE(google-runtime-references)
    explicit Serializer(std::vector<uint8_t> &buffer) : buffer_(buffer) {}

    template <typename T>
    void operator()(const T &value)
    {
        TypeToBuffer(value, buffer_);
    }

    virtual ~Serializer() = default;

    NO_COPY_SEMANTIC(Serializer);
    NO_MOVE_SEMANTIC(Serializer);

private:
    std::vector<uint8_t> &buffer_;
};

class Deserializer {
public:
    Deserializer(const uint8_t *data, size_t size) : data_(data), size_(size) {}

    bool IsError() const
    {
        return error_ != nullptr;
    }

    const char *GetError() const
    {
        return error_;
    }

    size_t GetEndPosition() const
    {
        return pos_;
    }

    template <typename T>
    void operator()(T &value)
    {
        if (error_ != nullptr) {
            return;
        }

        ASSERT(pos_ < size_);
        const uint8_t *ptr = ToUint8tPtr(ToUintPtr(data_) + pos_);
        auto ret = BufferToType(ptr, size_ - pos_, value);
        if (!ret) {
            error_ = ret.Error();
            return;
        }
        pos_ += ret.Value();
    }

    virtual ~Deserializer() = default;

    NO_COPY_SEMANTIC(Deserializer);
    NO_MOVE_SEMANTIC(Deserializer);

private:
    const uint8_t *data_;
    size_t size_;
    size_t pos_ = 0;
    const char *error_ = nullptr;
};

}  // namespace internal

template <size_t N, typename Struct>
bool StructToBuffer(Struct &&str, std::vector<uint8_t> &buffer)
{
    internal::ForEachTuple(internal::StructToTuple<N>(std::forward<Struct>(str)), internal::Serializer(buffer));
    return true;
}

template <size_t N, typename Struct>
Expected<size_t, const char *> RawBufferToStruct(const uint8_t *data, size_t size, Struct &out)
{
    using S = std::remove_reference_t<Struct>;
    using TupleType = decltype(internal::StructToTuple<N, S>({}));

    TupleType tuple;
    internal::Deserializer deserializer(data, size);
    internal::ForEachTuple(tuple, deserializer);
    if (deserializer.IsError()) {
        return Unexpected(deserializer.GetError());
    }

    out = std::move(internal::TupleToStruct<S>(tuple));
    return deserializer.GetEndPosition();
}

template <size_t N, typename Struct>
bool BufferToStruct(const uint8_t *data, size_t size, Struct &out)
{
    auto r = RawBufferToStruct<N>(data, size, out);
    if (!r) {
        return false;
    }
    return r.Value() == size;
}

template <size_t N, typename Struct>
bool BufferToStruct(const std::vector<uint8_t> &buffer, Struct &out)
{
    return BufferToStruct<N>(buffer.data(), buffer.size(), out);
}

}  // namespace ark::serializer

#endif  // PANDA_LIBPANDABASE_UTILS_SERIALIZER_SERIALIZER_H_
