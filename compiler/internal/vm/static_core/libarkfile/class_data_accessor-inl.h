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

#ifndef LIBPANDAFILE_CLASS_DATA_ACCESSOR_INL_H_
#define LIBPANDAFILE_CLASS_DATA_ACCESSOR_INL_H_

#include "libarkfile/class_data_accessor.h"
#include "libarkfile/field_data_accessor-inl.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/annotation_data_accessor.h"

#include "libarkfile/helpers.h"

namespace ark::panda_file {

inline void ClassDataAccessor::SkipSourceLang()
{
    GetSourceLang();
}

inline void ClassDataAccessor::SkipRuntimeAnnotations()
{
    EnumerateRuntimeAnnotations([](File::EntityId /* unused */) {});
}

inline void ClassDataAccessor::SkipAnnotations()
{
    EnumerateAnnotations([](File::EntityId /* unused */) {});
}

inline void ClassDataAccessor::SkipSourceFile()
{
    GetSourceFileId();
}

inline void ClassDataAccessor::SkipFields()
{
    EnumerateFields([](const FieldDataAccessor & /* unused */) {});
}

inline void ClassDataAccessor::SkipMethods()
{
    EnumerateMethods([](const MethodDataAccessor & /* unused */) {});
}

inline void ClassDataAccessor::SkipRuntimeTypeAnnotations()
{
    EnumerateRuntimeTypeAnnotations([](File::EntityId /* unused */) {});
}

inline void ClassDataAccessor::SkipTypeAnnotations()
{
    EnumerateTypeAnnotations([](File::EntityId /* unused */) {});
}

template <class Callback>
inline void ClassDataAccessor::EnumerateInterfaces(const Callback &cb)
{
    auto sp = ifacesOffsetsSp_;

    for (size_t i = 0; i < numIfaces_; i++) {
        auto index = helpers::Read<IDX_SIZE>(&sp);
        cb(pandaFile_.ResolveClassIndex(classId_, index));
    }
}

inline File::EntityId ClassDataAccessor::GetInterfaceId(size_t idx) const
{
    ASSERT(idx < numIfaces_);
    auto sp = ifacesOffsetsSp_.SubSpan(idx * IDX_SIZE);
    auto index = helpers::Read<IDX_SIZE>(&sp);
    return pandaFile_.ResolveClassIndex(classId_, index);
}

inline std::optional<SourceLang> ClassDataAccessor::GetSourceLang()
{
    return helpers::GetOptionalTaggedValue<SourceLang>(sourceLangSp_, ClassTag::SOURCE_LANG, &runtimeAnnotationsSp_);
}

template <class Callback>
inline void ClassDataAccessor::EnumerateRuntimeAnnotations(const Callback &cb)
{
    if (runtimeAnnotationsSp_.data() == nullptr) {
        SkipSourceLang();
    }

    helpers::EnumerateTaggedValues<File::EntityId, ClassTag, Callback>(
        runtimeAnnotationsSp_, ClassTag::RUNTIME_ANNOTATION, cb, &annotationsSp_);
}

template <class Callback>
inline void ClassDataAccessor::EnumerateAnnotations(const Callback &cb)
{
    if (annotationsSp_.data() == nullptr) {
        SkipRuntimeAnnotations();
    }

    helpers::EnumerateTaggedValues<File::EntityId, ClassTag, Callback>(annotationsSp_, ClassTag::ANNOTATION, cb,
                                                                       &runtimeTypeAnnotationSp_);
}

template <class Callback>
inline bool ClassDataAccessor::EnumerateRuntimeAnnotationsWithEarlyStop(const Callback &cb)
{
    if (runtimeAnnotationsSp_.data() == nullptr) {
        SkipSourceLang();
    }

    return helpers::EnumerateTaggedValuesWithEarlyStop<File::EntityId, ClassTag, Callback>(
        runtimeAnnotationsSp_, ClassTag::RUNTIME_ANNOTATION, cb);
}

template <class Callback>
inline bool ClassDataAccessor::EnumerateAnnotationsWithEarlyStop(const Callback &cb)
{
    if (annotationsSp_.data() == nullptr) {
        SkipRuntimeAnnotations();
    }

    return helpers::EnumerateTaggedValuesWithEarlyStop<File::EntityId, ClassTag, Callback>(annotationsSp_,
                                                                                           ClassTag::ANNOTATION, cb);
}

template <class Callback>
auto ClassDataAccessor::EnumerateAnnotation(const char *name, const Callback &cb)
{
    std::optional<std::invoke_result_t<const Callback &, panda_file::AnnotationDataAccessor &>> result {};
    EnumerateAnnotationsWithEarlyStop([&cb, &pf = pandaFile_, name, &result](panda_file::File::EntityId annotationId) {
        panda_file::AnnotationDataAccessor ada(pf, annotationId);
        auto *annotationName = pf.GetStringData(ada.GetClassId()).data;
        if (utf::IsEqual(utf::CStringAsMutf8(name), annotationName)) {
            result = cb(ada);
            return true;
        }
        return false;
    });
    return result;
}

inline std::optional<File::EntityId> ClassDataAccessor::GetSourceFileId()
{
    if (sourceFileSp_.data() == nullptr) {
        SkipTypeAnnotations();
    }

    auto v = helpers::GetOptionalTaggedValue<File::EntityId>(sourceFileSp_, ClassTag::SOURCE_FILE, &fieldsSp_);

    fieldsSp_ = fieldsSp_.SubSpan(TAG_SIZE);  // NOTHING tag

    return v;
}

template <class Callback, class Accessor>
inline void ClassDataAccessor::EnumerateClassElements(const File &pf, Span<const uint8_t> sp, size_t elemNum,
                                                      const Callback &cb, Span<const uint8_t> *next)
{
    for (size_t i = 0; i < elemNum; i++) {
        File::EntityId id = pf.GetIdFromPointer(sp.data());
        Accessor dataAccessor(pf, id);
        cb(dataAccessor);
        sp = sp.SubSpan(dataAccessor.GetSize());
    }

    *next = sp;
}

template <class Callback>
inline void ClassDataAccessor::EnumerateFields(const Callback &cb)
{
    if (fieldsSp_.data() == nullptr) {
        SkipSourceFile();
    }

    EnumerateClassElements<Callback, FieldDataAccessor>(pandaFile_, fieldsSp_, numFields_, cb, &methodsSp_);
}

template <class Callback>
inline void ClassDataAccessor::EnumerateMethods(const Callback &cb)
{
    if (methodsSp_.data() == nullptr) {
        SkipFields();
    }

    Span<const uint8_t> sp {nullptr, nullptr};

    EnumerateClassElements<Callback, MethodDataAccessor>(pandaFile_, methodsSp_, numMethods_, cb, &sp);

    size_ = pandaFile_.GetIdFromPointer(sp.data()).GetOffset() - classId_.GetOffset();
}

template <class Callback>
inline void ClassDataAccessor::EnumerateRuntimeTypeAnnotations(const Callback &cb)
{
    if (runtimeTypeAnnotationSp_.data() == nullptr) {
        SkipAnnotations();
    }

    helpers::EnumerateTaggedValues<File::EntityId, ClassTag, Callback>(
        runtimeTypeAnnotationSp_, ClassTag::RUNTIME_TYPE_ANNOTATION, cb, &typeAnnotationSp_);
}

template <class Callback>
inline void ClassDataAccessor::EnumerateTypeAnnotations(const Callback &cb)
{
    if (typeAnnotationSp_.data() == nullptr) {
        SkipRuntimeTypeAnnotations();
    }

    helpers::EnumerateTaggedValues<File::EntityId, ClassTag, Callback>(typeAnnotationSp_, ClassTag::TYPE_ANNOTATION, cb,
                                                                       &sourceFileSp_);
}

inline uint32_t ClassDataAccessor::GetAnnotationsNumber()
{
    size_t n = 0;
    EnumerateAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

inline uint32_t ClassDataAccessor::GetRuntimeAnnotationsNumber()
{
    size_t n = 0;
    EnumerateRuntimeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_CLASS_DATA_ACCESSOR_INL_H_
