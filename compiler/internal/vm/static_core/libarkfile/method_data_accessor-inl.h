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

#ifndef LIBPANDAFILE_METHOD_DATA_ACCESSOR_INL_H_
#define LIBPANDAFILE_METHOD_DATA_ACCESSOR_INL_H_

#include <cstdint>
#include "libarkfile/helpers.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/proto_data_accessor.h"

namespace ark::panda_file {

// static
inline File::EntityId MethodDataAccessor::GetNameId(const File &pandaFile, File::EntityId methodId)
{
    constexpr size_t SKIP_NUM = 2;  // skip class_idx and proto_idx
    auto sp = pandaFile.GetSpanFromId(methodId).SubSpan(IDX_SIZE * SKIP_NUM);
    return File::EntityId(helpers::Read<ID_SIZE>(&sp));
}

// static
inline panda_file::File::StringData MethodDataAccessor::GetName(const File &pandaFile, File::EntityId methodId)
{
    return pandaFile.GetStringData(GetNameId(pandaFile, methodId));
}

// static
inline File::EntityId MethodDataAccessor::GetProtoId(const File &pandaFile, File::EntityId methodId)
{
    constexpr size_t SKIP_NUM = 1;  // skip class_idx
    auto sp = pandaFile.GetSpanFromId(methodId).SubSpan(IDX_SIZE * SKIP_NUM);
    auto protoIdx = helpers::Read<IDX_SIZE>(&sp);
    return File::EntityId(pandaFile.ResolveProtoIndex(methodId, protoIdx).GetOffset());
}

// static
inline File::EntityId MethodDataAccessor::GetClassId(const File &pandaFile, File::EntityId methodId)
{
    auto sp = pandaFile.GetSpanFromId(methodId);
    auto classIdx = helpers::Read<IDX_SIZE>(&sp);
    return File::EntityId(pandaFile.ResolveClassIndex(methodId, classIdx).GetOffset());
}

inline panda_file::File::StringData MethodDataAccessor::GetName() const
{
    return pandaFile_.GetStringData(GetNameId());
}

inline void MethodDataAccessor::SkipCode()
{
    GetCodeId();
}

inline void MethodDataAccessor::SkipSourceLang()
{
    GetSourceLang();
}

inline void MethodDataAccessor::SkipRuntimeAnnotations()
{
    EnumerateRuntimeAnnotations([](File::EntityId /* unused */) {});
}

inline void MethodDataAccessor::SkipRuntimeParamAnnotation()
{
    GetRuntimeParamAnnotationId();
}

inline void MethodDataAccessor::SkipDebugInfo()
{
    GetDebugInfoId();
}

inline void MethodDataAccessor::SkipAnnotations()
{
    EnumerateAnnotations([](File::EntityId /* unused */) {});
}

inline void MethodDataAccessor::SkipParamAnnotation()
{
    GetParamAnnotationId();
}

inline void MethodDataAccessor::SkipTypeAnnotation()
{
    EnumerateTypeAnnotations([](File::EntityId /* unused */) {});
}

inline void MethodDataAccessor::SkipRuntimeTypeAnnotation()
{
    EnumerateRuntimeTypeAnnotations([](File::EntityId /* unused */) {});
}

inline std::optional<File::EntityId> MethodDataAccessor::GetCodeId()
{
    if (isExternal_) {
        // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
        // which fails Release builds for GCC 8 and 9.
        std::optional<File::EntityId> novalue;
        return novalue;
    }

    return helpers::GetOptionalTaggedValue<File::EntityId>(taggedValuesSp_, MethodTag::CODE, &sourceLangSp_);
}

inline std::optional<SourceLang> MethodDataAccessor::GetSourceLang()
{
    if (isExternal_) {
        // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
        // which fails Release builds for GCC 8 and 9.
        std::optional<SourceLang> novalue;
        return novalue;
    }

    if (sourceLangSp_.data() == nullptr) {
        SkipCode();
    }

    return helpers::GetOptionalTaggedValue<SourceLang>(sourceLangSp_, MethodTag::SOURCE_LANG, &runtimeAnnotationsSp_);
}

template <class Callback>
inline void MethodDataAccessor::EnumerateRuntimeAnnotations(Callback cb)
{
    if (isExternal_) {
        return;
    }

    if (runtimeAnnotationsSp_.data() == nullptr) {
        SkipSourceLang();
    }

    helpers::EnumerateTaggedValues<File::EntityId, MethodTag, Callback>(
        runtimeAnnotationsSp_, MethodTag::RUNTIME_ANNOTATION, cb, &runtimeParamAnnotationSp_);
}

inline std::optional<File::EntityId> MethodDataAccessor::GetRuntimeParamAnnotationId()
{
    if (isExternal_) {
        return {};
    }

    if (runtimeParamAnnotationSp_.data() == nullptr) {
        SkipRuntimeAnnotations();
    }

    return helpers::GetOptionalTaggedValue<File::EntityId>(runtimeParamAnnotationSp_,
                                                           MethodTag::RUNTIME_PARAM_ANNOTATION, &debugSp_);
}

inline std::optional<File::EntityId> MethodDataAccessor::GetDebugInfoId()
{
    if (isExternal_) {
        return {};
    }

    if (debugSp_.data() == nullptr) {
        SkipRuntimeParamAnnotation();
    }

    return helpers::GetOptionalTaggedValue<File::EntityId>(debugSp_, MethodTag::DEBUG_INFO, &annotationsSp_);
}

template <class Callback>
inline void MethodDataAccessor::EnumerateAnnotations(Callback cb)
{
    if (isExternal_) {
        return;
    }

    if (annotationsSp_.data() == nullptr) {
        SkipDebugInfo();
    }

    helpers::EnumerateTaggedValues<File::EntityId, MethodTag, Callback>(annotationsSp_, MethodTag::ANNOTATION, cb,
                                                                        &paramAnnotationSp_);
}

template <class Callback>
inline bool MethodDataAccessor::EnumerateRuntimeAnnotationsWithEarlyStop(Callback cb)
{
    if (isExternal_) {
        return false;
    }

    if (runtimeAnnotationsSp_.data() == nullptr) {
        SkipSourceLang();
    }

    return helpers::EnumerateTaggedValuesWithEarlyStop<File::EntityId, MethodTag, Callback>(
        runtimeAnnotationsSp_, MethodTag::RUNTIME_ANNOTATION, cb);
}

template <class Callback>
inline bool MethodDataAccessor::EnumerateAnnotationsWithEarlyStop(Callback cb)
{
    if (isExternal_) {
        return false;
    }

    if (annotationsSp_.data() == nullptr) {
        SkipDebugInfo();
    }

    return helpers::EnumerateTaggedValuesWithEarlyStop<File::EntityId, MethodTag, Callback>(annotationsSp_,
                                                                                            MethodTag::ANNOTATION, cb);
}

template <class Callback>
inline void MethodDataAccessor::EnumerateTypeAnnotations(Callback cb)
{
    if (isExternal_) {
        return;
    }

    if (typeAnnotationSp_.data() == nullptr) {
        SkipParamAnnotation();
    }

    helpers::EnumerateTaggedValues<File::EntityId, MethodTag, Callback>(typeAnnotationSp_, MethodTag::TYPE_ANNOTATION,
                                                                        cb, &runtimeTypeAnnotationSp_);
}

template <class Callback>
inline void MethodDataAccessor::EnumerateRuntimeTypeAnnotations(Callback cb)
{
    if (isExternal_) {
        return;
    }

    if (runtimeTypeAnnotationSp_.data() == nullptr) {
        SkipTypeAnnotation();
    }

    helpers::EnumerateTaggedValues<File::EntityId, MethodTag, Callback>(
        runtimeTypeAnnotationSp_, MethodTag::RUNTIME_TYPE_ANNOTATION, cb, &profileInfoSp_);
}

inline std::optional<size_t> MethodDataAccessor::GetProfileSize()
{
    if (isExternal_) {
        // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
        // which fails Release builds for GCC 8 and 9.
        return std::nullopt;
    }

    if (profileInfoSp_.data() == nullptr) {
        SkipRuntimeTypeAnnotation();
    }
    Span<const uint8_t> sp {nullptr, nullptr};
    auto v = helpers::GetOptionalTaggedValue<uint16_t>(profileInfoSp_, MethodTag::PROFILE_INFO, &sp);
    size_ = pandaFile_.GetIdFromPointer(sp.data()).GetOffset() - methodId_.GetOffset() + 1;  // + 1 for NOTHING tag
    return v;
}

inline std::optional<File::EntityId> MethodDataAccessor::GetParamAnnotationId()
{
    if (isExternal_) {
        // NB! This is a workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
        // which fails Release builds for GCC 8 and 9.
        std::optional<File::EntityId> novalue;
        return novalue;
    }

    if (paramAnnotationSp_.data() == nullptr) {
        SkipAnnotations();
    }

    auto v = helpers::GetOptionalTaggedValue<File::EntityId>(paramAnnotationSp_, MethodTag::PARAM_ANNOTATION,
                                                             &typeAnnotationSp_);

    return v;
}

inline uint32_t MethodDataAccessor::GetAnnotationsNumber()
{
    size_t n = 0;
    EnumerateRuntimeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

inline uint32_t MethodDataAccessor::GetRuntimeAnnotationsNumber()
{
    size_t n = 0;
    EnumerateRuntimeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

inline uint32_t MethodDataAccessor::GetTypeAnnotationsNumber()
{
    size_t n = 0;
    EnumerateTypeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

inline uint32_t MethodDataAccessor::GetRuntimeTypeAnnotationsNumber()
{
    size_t n = 0;
    EnumerateRuntimeTypeAnnotations([&n](File::EntityId /* unused */) { n++; });
    return n;
}

template <typename Callback>
void MethodDataAccessor::EnumerateTypesInProto(Callback cb, bool skipThis)
{
    size_t refIdx = 0;
    panda_file::ProtoDataAccessor pda(GetPandaFile(), GetProtoId());

    auto type = pda.GetReturnType();
    panda_file::File::EntityId classId;

    if (!type.IsPrimitive()) {
        classId = pda.GetReferenceType(refIdx++);
    }

    cb(type, classId);

    if (!IsStatic() && !skipThis) {
        // first arg type is method class
        cb(panda_file::Type {panda_file::Type::TypeId::REFERENCE}, GetClassId());
    }

    for (uint32_t idx = 0; idx < pda.GetNumArgs(); ++idx) {
        auto argType = pda.GetArgType(idx);
        panda_file::File::EntityId klassId;
        if (!argType.IsPrimitive()) {
            klassId = pda.GetReferenceType(refIdx++);
        }
        cb(argType, klassId);
    }
}

inline Type MethodDataAccessor::GetReturnType() const
{
    panda_file::ProtoDataAccessor pda(GetPandaFile(), GetProtoId());
    return pda.GetReturnType();
}

template <std::size_t SIZE>
void MethodDataAccessor::NumAnnonationProcess(uint32_t &result, File::EntityId &annotationId,
                                              std::array<const char *, SIZE> elemNameTable, uint32_t fieldId)
{
    AnnotationDataAccessor ada(pandaFile_, annotationId);
    auto *annotationName = reinterpret_cast<const char *>(pandaFile_.GetStringData(ada.GetClassId()).data);
    if (::strcmp("L_ESAnnotation;", annotationName) != 0) {
        return;
    }
    uint32_t elemCount = ada.GetCount();
    for (uint32_t i = 0; i < elemCount; i++) {
        AnnotationDataAccessor::Elem adae = ada.GetElement(i);
        auto *elemName = reinterpret_cast<const char *>(pandaFile_.GetStringData(adae.GetNameId()).data);
        if (::strcmp(elemNameTable[fieldId], elemName) == 0) {
            result = adae.GetScalarValue().GetValue();
        }
    }
}

inline uint32_t MethodDataAccessor::GetNumericalAnnotation(uint32_t fieldId)
{
    static constexpr uint32_t NUM_ELEMENT = 3;
    static std::array<const char *, NUM_ELEMENT> elemNameTable = {"icSize", "parameterLength", "funcName"};
    uint32_t result = 0;
    EnumerateAnnotations(
        [&](File::EntityId annotationId) { NumAnnonationProcess(result, annotationId, elemNameTable, fieldId); });
    return result;
}
inline std::string MethodDataAccessor::GetClassName() const
{
    return panda_file::ClassDataAccessor::DemangledName(pandaFile_.GetStringData(GetClassId()));
}

inline std::string MethodDataAccessor::GetFullName() const
{
    uint32_t strOffset = const_cast<MethodDataAccessor *>(this)->GetNumericalAnnotation(AnnotationField::FUNCTION_NAME);
    if (strOffset != 0) {
        auto mname = utf::Mutf8AsCString(pandaFile_.GetStringData(panda_file::File::EntityId(strOffset)).data);
        return GetClassName() + "::" + mname;
    }
    return utf::Mutf8AsCString(GetName().data);
}

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_METHOD_DATA_ACCESSOR_INL_H_
