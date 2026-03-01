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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H

#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "libarkfile/proto_data_accessor-inl.h"

namespace ark::ets {

static constexpr const char *GETTER_PREFIX = "%%get-";
static constexpr const char *SETTER_PREFIX = "%%set-";

ALWAYS_INLINE inline bool EtsReferenceNullish(EtsCoroutine *coro, EtsObject *ref)
{
    ASSERT(coro != nullptr);
    return ref == nullptr || ref == EtsObject::FromCoreType(coro->GetNullValue());
}

ALWAYS_INLINE inline bool IsValueNaN(EtsObject *ref)
{
    if (UNLIKELY(ref == nullptr || !ref->GetClass()->IsBoxed())) {
        return false;
    }
    if (ref->GetClass()->IsBoxedDouble()) {
        return std::isnan(static_cast<double>(EtsBoxPrimitive<EtsDouble>::Unbox(ref)));
    }
    if (ref->GetClass()->IsBoxedFloat()) {
        return std::isnan(static_cast<float>(EtsBoxPrimitive<EtsFloat>::Unbox(ref)));
    }
    return false;
}

template <bool IS_STRICT>
ALWAYS_INLINE inline bool EtsReferenceEquals(EtsCoroutine *coro, EtsObject *ref1, EtsObject *ref2)
{
    if (UNLIKELY(ref1 == ref2)) {
        return !IsValueNaN(ref1);
    }

    if (EtsReferenceNullish(coro, ref1) || EtsReferenceNullish(coro, ref2)) {
        if constexpr (IS_STRICT) {
            return false;
        } else {
            return EtsReferenceNullish(coro, ref1) && EtsReferenceNullish(coro, ref2);
        }
    }

    ASSERT(ref1 != nullptr);
    ASSERT(ref2 != nullptr);
    if (LIKELY(!(ref1->GetClass()->IsValueTyped() && ref2->GetClass()->IsValueTyped()))) {
        return false;
    }
    return EtsValueTypedEquals(coro, ref1, ref2);
}

ALWAYS_INLINE inline EtsString *EtsReferenceTypeof(EtsCoroutine *coro, EtsObject *ref)
{
    return EtsGetTypeof(coro, ref);
}

ALWAYS_INLINE inline bool EtsIstrue(EtsCoroutine *coro, EtsObject *obj)
{
    return EtsGetIstrue(coro, obj);
}

// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
inline EtsClass *GetMethodOwnerClassInFrames(EtsCoroutine *coro, uint32_t depth)
{
    auto stack = StackWalker::Create(coro);
    for (uint32_t i = 0; i < depth && stack.HasFrame(); ++i) {
        stack.NextFrame();
    }
    if (UNLIKELY(!stack.HasFrame())) {
        return nullptr;
    }
    auto method = stack.GetMethod();
    if (UNLIKELY(method == nullptr)) {
        return nullptr;
    }
    return EtsClass::FromRuntimeClass(method->GetClass());
}

ALWAYS_INLINE inline void IsValidByType(EtsCoroutine *coro, ark::Class *argClass, Field *metaField, bool isGetter)
{
    auto sourceClass = isGetter ? argClass : metaField->ResolveTypeClass();
    auto targetClass = isGetter ? metaField->ResolveTypeClass() : argClass;
    if (UNLIKELY(!targetClass->IsAssignableFrom(sourceClass))) {
        auto errorMsg = targetClass->GetName() + " cannot be cast to " + sourceClass->GetName();
        ThrowEtsException(coro,
                          utf::Mutf8AsCString(Runtime::GetCurrent()
                                                  ->GetLanguageContext(panda_file::SourceLang::ETS)
                                                  .GetClassCastExceptionClassDescriptor()),
                          errorMsg);
    }
}

ALWAYS_INLINE inline void IsValidAccessorByName(EtsCoroutine *coro, Field *metaField, Method *resolved, bool isGetter)
{
    auto pf = resolved->GetPandaFile();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, resolved->GetFileId()));
    auto argClass = classLinker->GetClass(*pf, pda.GetReferenceType(0), resolved->GetClass()->GetLoadContext());
    IsValidByType(coro, argClass, metaField, isGetter);
}

ALWAYS_INLINE inline void IsValidFieldByName(EtsCoroutine *coro, Field *metaField, Field *resolved, bool isGetter)
{
    auto pf = resolved->GetPandaFile();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto argClass = classLinker->GetClass(*pf, panda_file::FieldDataAccessor::GetTypeId(*pf, resolved->GetFileId()),
                                          resolved->GetClass()->GetLoadContext());
    IsValidByType(coro, argClass, metaField, isGetter);
}

template <bool IS_GETTER>
inline void LookUpException(ark::Class *klass, Field *rawField)
{
    auto type = IS_GETTER ? "getter" : "setter";
    auto errorMsg = "Class " + ark::ConvertToString(klass->GetName()) + " does not have field and " +
                    ark::ConvertToString(type) + " with name " + utf::Mutf8AsCString(rawField->GetName().data);
    ThrowEtsException(EtsCoroutine::GetCurrent(),
                      panda_file_items::class_descriptors::LINKER_UNRESOLVED_FIELD_ERROR.data(), errorMsg);
}

inline void LookUpException(ark::Class *klass, ark::Method *rawMethod)
{
    auto rawMethodName = (rawMethod == nullptr) ? "null" : utf::Mutf8AsCString(rawMethod->GetName().data);
    auto errorMsg =
        "Class " + ark::ConvertToString(klass->GetName()) + " does not have method with name " + rawMethodName;
    ThrowEtsException(EtsCoroutine::GetCurrent(),
                      panda_file_items::class_descriptors::LINKER_UNRESOLVED_METHOD_ERROR.data(), errorMsg);
}

template <bool IS_GETTER>
ALWAYS_INLINE Field *GetFieldByName(InterpreterCache::Entry *entry, ark::Method *method, Field *rawField,
                                    const uint8_t *address, ark::Class *klass)
{
    auto *res = entry->item;
    auto resUint = reinterpret_cast<uint64_t>(res);
    auto fieldPtr = reinterpret_cast<Field *>(resUint & ~METHOD_FLAG_MASK);
    bool cacheExists = res != nullptr && ((resUint & METHOD_FLAG_MASK) == 1);
    Class *current = klass;
    Field *field = nullptr;
    while (current != nullptr) {
        if (cacheExists && (fieldPtr->GetClass() == current)) {
            return fieldPtr;
        }
        ASSERT(rawField != nullptr);
        field = LookupFieldByName(rawField->GetName(), current);
        if (field == nullptr) {
            current = current->GetBase();
            continue;
        }
        if (field->GetTypeId() == panda_file::Type::TypeId::REFERENCE) {
            IsValidFieldByName(EtsCoroutine::GetCurrent(), rawField, field, IS_GETTER);
        }
        *entry = {address, method, static_cast<void *>(field)};
        return field;
    }
    return field;
}

template <panda_file::Type::TypeId FIELD_TYPE, bool IS_GETTER>
ALWAYS_INLINE inline ark::Method *GetAccessorByName(InterpreterCache::Entry *entry, ark::Method *method,
                                                    Field *rawField, const uint8_t *address, ark::Class *klass)
{
    auto *res = entry->item;
    auto resUint = reinterpret_cast<uint64_t>(res);
    bool cacheExists = res != nullptr && ((resUint & METHOD_FLAG_MASK) == 1) && address == entry->pc;
    auto methodPtr = reinterpret_cast<Method *>(resUint & ~METHOD_FLAG_MASK);
    ark::Method *callee = nullptr;
    Class *current = klass;
    while (current != nullptr) {
        if (cacheExists && (methodPtr->GetClass() == klass)) {
            return methodPtr;
        }
        if constexpr (IS_GETTER) {
            callee = LookupGetterByName<FIELD_TYPE>(rawField->GetName(), current);
        } else {
            callee = LookupSetterByName<FIELD_TYPE>(rawField->GetName(), current);
        }
        if (callee == nullptr) {
            current = current->GetBase();
            continue;
        }
        if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
            IsValidAccessorByName(EtsCoroutine::GetCurrent(), rawField, callee, IS_GETTER);
        }
        auto mUint = reinterpret_cast<uint64_t>(callee);
        *entry = {address, method, reinterpret_cast<Method *>(mUint | METHOD_FLAG_MASK)};
        return callee;
    }
    return callee;
}

template <bool IS_GETTER>
ALWAYS_INLINE inline bool CheckAccessorNameMatch(Span<const uint8_t> name, Span<const uint8_t> method)
{
    std::string_view methodStr(utf::Mutf8AsCString(method.data()), method.size());
    std::string_view nameStr(utf::Mutf8AsCString(name.data()), name.size());
    std::string_view prefix = IS_GETTER ? GETTER_PREFIX : SETTER_PREFIX;
    if (methodStr.size() - nameStr.size() != prefix.size() || (methodStr.substr(0, prefix.size()) != prefix)) {
        return false;
    }
    return methodStr.substr(prefix.size()) == nameStr;
}

ALWAYS_INLINE inline Field *LookupFieldByName(panda_file::File::StringData name, const ark::Class *klass)
{
    for (auto &f : klass->GetInstanceFields()) {
        if (name == f.GetName()) {
            return &f;
        }
    }
    return nullptr;
}

template <panda_file::Type::TypeId FIELD_TYPE>
ALWAYS_INLINE inline ark::Method *LookupGetterByName(panda_file::File::StringData name, const ark::Class *klass)
{
    Span<const uint8_t> nameSpan((name.data), utf::Mutf8Size(name.data));
    for (auto &m : klass->GetMethods()) {
        Span<const uint8_t> methodSpan(m.GetName().data, utf::Mutf8Size(m.GetName().data));
        if (!CheckAccessorNameMatch<true>(nameSpan, methodSpan)) {
            continue;
        }
        auto retType = m.GetReturnType();
        if (retType.IsVoid()) {
            continue;
        }
        if (m.GetNumArgs() != 1) {
            continue;
        }
        if (!m.GetArgType(0).IsReference()) {
            continue;
        }
        if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
            if (retType.IsPrimitive()) {
                continue;
            }
            return &m;
        }

        if (retType.IsReference()) {
            continue;
        }
        if constexpr (panda_file::Type(FIELD_TYPE).GetBitWidth() == coretypes::INT64_BITS) {
            if (retType.GetBitWidth() != coretypes::INT64_BITS) {
                continue;
            }
        } else {
            if (retType.GetBitWidth() > coretypes::INT32_BITS) {
                continue;
            }
        }
        return &m;
    }
    return nullptr;
}

template <panda_file::Type::TypeId FIELD_TYPE>
ALWAYS_INLINE inline ark::Method *LookupSetterByName(panda_file::File::StringData name, const ark::Class *klass)
{
    Span<const uint8_t> nameSpan((name.data), utf::Mutf8Size(name.data));
    for (auto &m : klass->GetMethods()) {
        Span<const uint8_t> methodSpan(m.GetName().data, utf::Mutf8Size(m.GetName().data));
        if (!CheckAccessorNameMatch<false>(nameSpan, methodSpan)) {
            continue;
        }
        if (m.IsStatic() || !m.GetReturnType().IsVoid() || m.GetNumArgs() != 2U || !m.GetArgType(0).IsReference()) {
            continue;
        }
        if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
            if (m.GetArgType(1).IsPrimitive()) {
                continue;
            }
            return &m;
        }

        auto arg1 = m.GetArgType(1);
        if (arg1.IsReference()) {
            continue;
        }
        if constexpr (panda_file::Type(FIELD_TYPE).GetBitWidth() == coretypes::INT64_BITS) {
            if (arg1.GetBitWidth() != coretypes::INT64_BITS) {
                continue;
            }
        } else {
            if (arg1.GetBitWidth() > coretypes::INT32_BITS) {
                continue;
            }
        }
        return &m;
    }
    return nullptr;
}

ALWAYS_INLINE inline Method *GetMethodFromCache(ETSStubCacheInfo const &cache, ark::Class *klass)
{
    auto *res = cache.GetItem();
    auto resUint = reinterpret_cast<uint64_t>(res);
    bool cacheExists = (res != nullptr) && ((resUint & METHOD_FLAG_MASK) == 1);
    auto methodPtr = reinterpret_cast<Method *>(resUint & ~METHOD_FLAG_MASK);
    if (LIKELY(cacheExists && (methodPtr->GetClass()->IsAssignableFrom(klass)))) {
        auto methods = klass->GetVTable();
        return methods[methodPtr->GetVTableIndex()];
    }
    return nullptr;
}

// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
// CC-OFFNXT(G.FUD.06) perf critical
ALWAYS_INLINE inline bool MethodIsSupertypeOf(ClassLinker *linker, Method *superm, Method *subm)
{
    Method::ProtoId const &super = superm->GetProtoId();
    Method::ProtoId const &sub = subm->GetProtoId();

    auto subPDA = panda_file::ProtoDataAccessor(sub.GetPandaFile(), sub.GetEntityId());
    auto superPDA = panda_file::ProtoDataAccessor(super.GetPandaFile(), super.GetEntityId());
    if (superPDA.GetNumElements() != subPDA.GetNumElements()) {
        return false;
    }
    if (superPDA.GetReturnType() != subPDA.GetReturnType()) {
        return false;
    }

    uint32_t numArgs = subPDA.GetNumArgs();

    for (uint32_t i = 0, refIdx = 0; i < numArgs; ++i) {
        if (superPDA.GetArgType(i) != subPDA.GetArgType(i)) {
            return false;
        }
        if (superPDA.GetArgType(i).IsReference()) {
            auto subRef = linker->GetClass(*subm, subPDA.GetReferenceType(refIdx));
            if (UNLIKELY(subRef == nullptr)) {
                return false;
            }
            auto superRef = linker->GetClass(*superm, superPDA.GetReferenceType(refIdx));
            if (UNLIKELY(superRef == nullptr)) {
                return false;
            }
            if (!subRef->IsAssignableFrom(superRef)) {
                return false;
            }
            refIdx++;
        }
    }
    return true;
}

// CC-OFFNXT(C_RULE_ID_INLINE_FUNCTION_SIZE) Perf critical common runtime code stub
// CC-OFFNXT(G.FUD.06) perf critical
ALWAYS_INLINE inline Method *ResolveCompatibleVMethodInClass(EtsCoroutine *coro, const ark::Class *klass,
                                                             Method *lookupTarget)
{
    auto linker = Runtime::GetCurrent()->GetClassLinker();

    // CC-OFFNXT(C_RULE_ID_POINTER_DECLARE_FOLLOW_NAME) project code style
    // CC-OFFNXT(G.FMT.14-CPP,G.FMT.10-CPP) project code style
    auto lookupFromIndex = [coro, linker, klass, lookupTarget](size_t from) -> Method * {
        auto methods = klass->GetVTable();
        for (size_t idx = from; idx < methods.size(); ++idx) {
            auto vmethod = methods[idx];
            if (vmethod->IsPrivate()) {
                continue;
            }
            if (LIKELY(lookupTarget->GetName() != vmethod->GetName())) {
                continue;
            }
            if (MethodIsSupertypeOf(linker, vmethod, lookupTarget)) {
                ASSERT(vmethod->GetVTableIndex() == idx);
                return vmethod;
            }
            if (UNLIKELY(coro->HasPendingException())) {
                return nullptr;
            }
        }
        return nullptr;
    };

    size_t vtStart = 0;

    Method *matched = lookupFromIndex(vtStart);
    if (matched == nullptr) {
        // not found
        return nullptr;
    }
    if (lookupFromIndex(matched->GetVTableIndex() + 1) != nullptr) {
        // inconsistent
        return nullptr;
    }
    return matched;
}

ALWAYS_INLINE inline Method *ResolveCompatibleVMethod(EtsCoroutine *coro, ark::Class *klass, Method *lookupTarget)
{
    for (Class *t = klass; t != nullptr; t = t->GetBase()) {
        Method *resolved = ResolveCompatibleVMethodInClass(coro, t, lookupTarget);
        if (resolved != nullptr) {
            return resolved;
        }
    }
    return nullptr;
}

ALWAYS_INLINE inline Method *GetMethodByName(EtsCoroutine *coro, ETSStubCacheInfo const &cache, Method *rawMethod,
                                             ark::Class *klass)
{
    Method *callee = GetMethodFromCache(cache, klass);
    if (callee != nullptr) {
        return callee;
    }
    callee = ResolveCompatibleVMethod(coro, klass, rawMethod);
    if (callee != nullptr) {
        cache.UpdateItem(callee);
        return callee;
    }
    return callee;
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_STUBS_INL_H
