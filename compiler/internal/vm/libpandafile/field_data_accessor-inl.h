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

#ifndef LIBPANDAFILE_FIELD_DATA_ACCESSOR_INL_H
#define LIBPANDAFILE_FIELD_DATA_ACCESSOR_INL_H

#include "field_data_accessor.h"
#include "helpers.h"

#include "utils/bit_utils.h"

#include <type_traits>

namespace panda::panda_file {

// static
inline File::EntityId FieldDataAccessor::GetTypeId(const File &panda_file, File::EntityId field_id)
{
    auto sp = panda_file.GetSpanFromId(field_id).SubSpan(IDX_SIZE);  // skip class_idx
    auto type_idx = helpers::Read<panda_file::IDX_SIZE>(&sp);
    return panda_file.ResolveClassIndex(field_id, type_idx);
}

// static
inline File::EntityId FieldDataAccessor::GetNameId(const File &panda_file, File::EntityId field_id)
{
    auto sp = panda_file.GetSpanFromId(field_id).SubSpan(IDX_SIZE * 2);  // skip class_idx, type_idx
    return File::EntityId(helpers::Read<panda_file::ID_SIZE>(&sp));
}

template <class T>
inline std::optional<T> FieldDataAccessor::GetValue()
{
    if (is_external_) {
        // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
        // which fails Release builds for GCC 8 and 9.
        std::optional<T> novalue = {};
        return novalue;
    }

    auto v = GetValueInternal();

    if (!v.has_value()) {
        // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
        // which fails Release builds for GCC 8 and 9.
        std::optional<T> novalue = {};
        return novalue;
    }

    FieldValue field_value = *v;

    // Disable checks due to clang-tidy bug https://bugs.llvm.org/show_bug.cgi?id=32203
    // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
    if constexpr (std::is_integral_v<T>) {
        // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
        if constexpr (sizeof(T) <= sizeof(uint32_t)) {
            return static_cast<T>(std::get<uint32_t>(field_value));
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            return static_cast<T>(std::get<uint64_t>(field_value));
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
        if constexpr (sizeof(T) <= sizeof(uint32_t)) {
            return bit_cast<T, uint32_t>(std::get<uint32_t>(field_value));
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            return bit_cast<T, uint64_t>(std::get<uint64_t>(field_value));
        }
    }
}

template <>
inline std::optional<File::EntityId> FieldDataAccessor::GetValue()
{
    if (is_external_) {
        return {};
    }

    auto v = GetValueInternal();

    if (!v.has_value()) {
        return {};
    }

    FieldValue field_value = *v;

    return File::EntityId(std::get<uint32_t>(field_value));
}

inline void FieldDataAccessor::SkipValue()
{
    GetValueInternal();
}

inline void FieldDataAccessor::SkipRuntimeAnnotations()
{
    EnumerateRuntimeAnnotations([](File::EntityId /* unused */) {});
}

inline void FieldDataAccessor::SkipAnnotations()
{
    EnumerateAnnotations([](File::EntityId /* unused */) {});
}

inline void FieldDataAccessor::SkipRuntimeTypeAnnotations()
{
    EnumerateRuntimeTypeAnnotations([](File::EntityId /* unused */) {});
}

inline void FieldDataAccessor::SkipTypeAnnotations()
{
    EnumerateTypeAnnotations([](File::EntityId /* unused */) {});
}

template <class Callback>
inline void FieldDataAccessor::EnumerateRuntimeAnnotations(const Callback &cb)
{
    if (is_external_) {
        return;
    }

    if (runtime_annotations_sp_.data() == nullptr) {
        SkipValue();
    }

    helpers::EnumerateTaggedValues<File::EntityId, FieldTag, Callback>(
        runtime_annotations_sp_, FieldTag::RUNTIME_ANNOTATION, cb, &annotations_sp_);
}

template <class Callback>
inline void FieldDataAccessor::EnumerateAnnotations(const Callback &cb)
{
    if (is_external_) {
        return;
    }

    if (annotations_sp_.data() == nullptr) {
        SkipRuntimeAnnotations();
    }

    helpers::EnumerateTaggedValues<File::EntityId, FieldTag, Callback>(annotations_sp_, FieldTag::ANNOTATION, cb,
                                                                       &runtime_type_annotations_sp_);
}

template <class Callback>
inline void FieldDataAccessor::EnumerateRuntimeTypeAnnotations(const Callback &cb)
{
    if (is_external_) {
        return;
    }

    if (runtime_type_annotations_sp_.data() == nullptr) {
        SkipAnnotations();
    }

    helpers::EnumerateTaggedValues<File::EntityId, FieldTag, Callback>(runtime_type_annotations_sp_,
                                                                       FieldTag::ANNOTATION, cb, &type_annotations_sp_);
}

template <class Callback>
inline void FieldDataAccessor::EnumerateTypeAnnotations(const Callback &cb)
{
    if (is_external_) {
        return;
    }

    if (type_annotations_sp_.data() == nullptr) {
        SkipRuntimeTypeAnnotations();
    }

    Span<const uint8_t> sp {nullptr, nullptr};
    helpers::EnumerateTaggedValues<File::EntityId, FieldTag, Callback>(type_annotations_sp_, FieldTag::ANNOTATION, cb,
                                                                       &sp);

    size_ = panda_file_.GetIdFromPointer(sp.data()).GetOffset() - field_id_.GetOffset() + 1;  // + 1 for NOTHING tag
}

template <class Callback>
inline bool FieldDataAccessor::EnumerateRuntimeAnnotationsWithEarlyStop(const Callback &cb)
{
    if (is_external_) {
        return false;
    }

    if (runtime_annotations_sp_.data() == nullptr) {
        SkipValue();
    }

    return helpers::EnumerateTaggedValuesWithEarlyStop<File::EntityId, FieldTag, Callback>(
        runtime_annotations_sp_, FieldTag::RUNTIME_ANNOTATION, cb);
}

template <class Callback>
inline bool FieldDataAccessor::EnumerateAnnotationsWithEarlyStop(const Callback &cb)
{
    if (is_external_) {
        return false;
    }

    if (annotations_sp_.data() == nullptr) {
        SkipRuntimeAnnotations();
    }

    return helpers::EnumerateTaggedValuesWithEarlyStop<File::EntityId, FieldTag, Callback>(annotations_sp_,
                                                                                           FieldTag::ANNOTATION, cb);
}

inline uint32_t FieldDataAccessor::GetAnnotationsNumber()
{
    size_t n = 0;
    EnumerateRuntimeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

inline uint32_t FieldDataAccessor::GetRuntimeAnnotationsNumber()
{
    size_t n = 0;
    EnumerateRuntimeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

inline uint32_t FieldDataAccessor::GetRuntimeTypeAnnotationsNumber()
{
    size_t n = 0;
    EnumerateRuntimeTypeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

inline uint32_t FieldDataAccessor::GetTypeAnnotationsNumber()
{
    size_t n = 0;
    EnumerateTypeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_FIELD_DATA_ACCESSOR_INL_H
