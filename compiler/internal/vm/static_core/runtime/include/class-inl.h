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
#ifndef PANDA_RUNTIME_CLASS_INL_H_
#define PANDA_RUNTIME_CLASS_INL_H_

#include "runtime/include/class.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/field.h"
#include "runtime/include/object_header.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/object_accessor-inl.h"

namespace ark {

template <typename Item>
struct NameComp {
    // NOLINTNEXTLINE(readability-identifier-naming)
    bool equal(const Item &m, const panda_file::File::StringData &name) const
    {
        return m.GetName() == name;
    }
    bool operator()(const Method &m, const panda_file::File::StringData &name) const
    {
        return m.GetName() < name;
    }
};

template <typename Item>
struct EntityIdComp {
    // NOLINTNEXTLINE(readability-identifier-naming)
    bool equal(const Item &m, panda_file::File::EntityId id) const
    {
        return m.GetFileId().GetOffset() == id.GetOffset();
    }
    bool operator()(const Item &m, panda_file::File::EntityId id) const
    {
        return m.GetFileId().GetOffset() < id.GetOffset();
    }
};

using MethodNameComp = NameComp<Method>;
using MethodIdComp = EntityIdComp<Method>;

template <typename Item>
ALWAYS_INLINE inline bool PredComp([[maybe_unused]] const Item &item)
{
    return true;
}

template <typename Item, typename FirstPred, typename... Pred>
ALWAYS_INLINE inline bool PredComp(const Item &item, const FirstPred &pred, const Pred &...preds)
{
    return pred(item) && PredComp(item, preds...);
}

template <typename KeyComp, typename Key, typename Item, typename... Pred>
ALWAYS_INLINE inline Item *BinSearch(Span<Item> items, Key key, const Pred &...preds)
{
    auto it = std::lower_bound(items.begin(), items.end(), key, KeyComp());
    while (it != items.end()) {
        auto &item = *it;
        if (!KeyComp().equal(item, key)) {
            break;
        }
        if (PredComp(item, preds...)) {
            return &item;
        }
        ++it;
    }
    return nullptr;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline uint32_t Class::GetTypeSize(panda_file::Type type)
{
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::I8:
        case panda_file::Type::TypeId::U8:
            return sizeof(uint8_t);
        case panda_file::Type::TypeId::I16:
        case panda_file::Type::TypeId::U16:
            return sizeof(uint16_t);
        case panda_file::Type::TypeId::I32:
        case panda_file::Type::TypeId::U32:
        case panda_file::Type::TypeId::F32:
            return sizeof(uint32_t);
        case panda_file::Type::TypeId::I64:
        case panda_file::Type::TypeId::U64:
        case panda_file::Type::TypeId::F64:
            return sizeof(uint64_t);
        case panda_file::Type::TypeId::TAGGED:
            return coretypes::TaggedValue::TaggedTypeSize();
        case panda_file::Type::TypeId::REFERENCE:
            return ClassHelper::OBJECT_POINTER_SIZE;
        default:
            UNREACHABLE();
    }
}

inline uint32_t Class::GetComponentSize() const
{
    if (componentType_ == nullptr) {
        return 0;
    }

    return GetTypeSize(componentType_->GetType());
}

inline bool Class::IsClassClass() const
{
    return GetManagedObject()->ClassAddr<Class>() == this;
}

inline bool Class::IsSubClassOf(const Class *klass) const
{
    const Class *current = this;

    do {
        if (current == klass) {
            return true;
        }

        current = current->GetBase();
    } while (current != nullptr);

    return false;
}

inline static bool RefIsAssignableToImpl(const Class *sub, const Class *super, uint32_t depth);

inline static bool UnionIsAssignableToRef(const Class *subUnion, const Class *super, uint32_t depth)
{
    for (auto *consClass : subUnion->GetConstituentTypes()) {
        bool isAssignableTo = RefIsAssignableToImpl(consClass, super, depth);
        if (!isAssignableTo) {
            return false;
        }
    }
    return true;
}

inline static bool IsAssignableToUnion(const Class *sub, const Class *superUnion, uint32_t depth)
{
    for (auto *consClass : superUnion->GetConstituentTypes()) {
        bool isAssignableTo = RefIsAssignableToImpl(sub, consClass, depth);
        if (isAssignableTo) {
            return true;
        }
    }
    return false;
}

// CC-OFFNXT(G.FUD.06, huge_method) perf critical, solid logic
inline static bool RefIsAssignableToImpl(const Class *sub, const Class *super, uint32_t depth)
{
    if (UNLIKELY(depth-- == 0)) {
        LOG(ERROR, RUNTIME) << "Max class assignability test depth reached";
        return false;
    }

    if (sub == super) {
        return true;
    }
    if (sub->IsUnionClass()) {
        return UnionIsAssignableToRef(sub, super, depth);
    }
    if (super->IsUnionClass()) {
        return IsAssignableToUnion(sub, super, depth);
    }
    if (super->IsObjectClass()) {
        return !sub->IsPrimitive();
    }
    if (super->IsInterface()) {
        return sub->Implements(super);
    }
    if (sub->IsArrayClass()) {
        if (!super->IsArrayClass()) {
            return false;
        }
        return RefIsAssignableToImpl(sub->GetComponentType(), super->GetComponentType(), depth);
    }

    return sub->IsSubClassOf(super);
}

inline static bool RefIsAssignableTo(const Class *sub, const Class *super)
{
    constexpr uint32_t ASSIGNABILITY_MAX_DEPTH = 256U;
    return RefIsAssignableToImpl(sub, super, ASSIGNABILITY_MAX_DEPTH);
}

// CC-OFFNXT(G.FUD.06, huge_method) perf critical, solid logic
inline static bool RefIsAssignableToNoSuper(const Class *sub, const Class *super)
{
    if (sub->IsUnionClass() || super->IsUnionClass()) {
        return RefIsAssignableTo(sub, super);
    }
    if (sub == super) {
        return true;
    }
    if (super->IsObjectClass()) {
        return !sub->IsPrimitive();
    }
    if (super->IsInterface()) {
        return sub->Implements(super);
    }
    if (sub->IsArrayClass()) {
        if (super->IsArrayClass()) {
            return RefIsAssignableTo(sub->GetComponentType(), super->GetComponentType());
        }
    }
    return false;
}

inline bool Class::IsAssignableFrom(const Class *klass) const
{
    return RefIsAssignableTo(klass, this);
}

inline bool Class::IsAssignableFromRefNoSuper(const Class *klass) const
{
    return RefIsAssignableToNoSuper(klass, this);
}

inline bool Class::Implements(const Class *klass) const
{
    for (const auto &elem : itable_.Get()) {
        if (elem.GetInterface() == klass) {
            return true;
        }
    }

    return false;
}

template <Class::FindFilter FILTER>
inline Span<Field> Class::GetFields() const
{
    switch (FILTER) {
        case FindFilter::STATIC:
            return GetStaticFields();
        case FindFilter::INSTANCE:
            return GetInstanceFields();
        case FindFilter::ALL:
            return GetFields();
        default:
            UNREACHABLE();
    }
}

template <Class::FindFilter FILTER, class Pred>
inline Field *Class::FindDeclaredField(Pred pred) const
{
    auto fields = GetFields<FILTER>();
    auto it = std::find_if(fields.begin(), fields.end(), pred);
    if (it != fields.end()) {
        return &*it;
    }
    return nullptr;
}

ALWAYS_INLINE inline Field *BinarySearchField(Span<Field> fields, panda_file::File::EntityId id)
{
    auto comp = [](const Field &field, panda_file::File::EntityId fieldId) { return field.GetFileId() < fieldId; };
    auto it = std::lower_bound(fields.begin(), fields.end(), id, comp);
    if (it != fields.end() && (*it).GetFileId() == id) {
        return &*it;
    }
    return nullptr;
}

template <Class::FindFilter FILTER>
// CC-OFFNXT(G.FUD.06) solid logic
inline Field *Class::FindDeclaredField(panda_file::File::EntityId id) const
{
    if (FILTER == FindFilter::ALL) {
        auto staticFields = GetStaticFields();
        auto *staticField = BinarySearchField(staticFields, id);
        if (staticField != nullptr) {
            return staticField;
        }
        auto instanceFields = GetInstanceFields();
        auto *instanceField = BinarySearchField(instanceFields, id);
        if (instanceField != nullptr) {
            return instanceField;
        }
    } else {
        auto fields = GetFields<FILTER>();
        auto *field = BinarySearchField(fields, id);
        if (field != nullptr) {
            return field;
        }
    }

    return nullptr;
}

template <Class::FindFilter FILTER, class Pred>
inline Field *Class::FindFieldInInterfaces(const Class *kls, Pred pred) const
{
    while (kls != nullptr) {
        for (auto *iface : kls->GetInterfaces()) {
            auto *field = iface->FindField<FILTER>(pred);
            if (field != nullptr) {
                return field;
            }
        }

        kls = kls->GetBase();
    }

    return nullptr;
}

template <Class::FindFilter FILTER, class Pred>
// CC-OFFNXT(G.FUD.06) solid logic
inline Field *Class::FindField(Pred pred) const
{
    auto *cls = this;
    while (cls != nullptr) {
        auto *field = cls->FindDeclaredField<FILTER>(pred);
        if (field != nullptr) {
            return field;
        }

        cls = cls->GetBase();
    }

    if (FILTER == FindFilter::STATIC || FILTER == FindFilter::ALL) {
        return FindFieldInInterfaces<FILTER>(this, pred);
    }

    return nullptr;
}

template <Class::FindFilter FILTER>
// CC-OFFNXT(G.FUD.06) big swtich case
inline Span<Method> Class::GetMethods() const
{
    switch (FILTER) {
        case FindFilter::STATIC:
            return GetStaticMethods();
        case FindFilter::INSTANCE:
            return GetVirtualMethods();
        case FindFilter::ALL:
            return GetMethods();
        case FindFilter::COPIED:
            return GetCopiedMethods();
        default:
            UNREACHABLE();
    }
}

template <Class::FindFilter FILTER, typename KeyComp, typename Key, typename... Pred>
// CC-OFFNXT(G.FUD.06) solid logic
inline Method *Class::FindDirectMethod(Key key, const Pred &...preds) const
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (FILTER == FindFilter::ALL || FILTER == FindFilter::STATIC) {
        auto methods = GetMethods<FindFilter::STATIC>();
        auto *method = BinSearch<KeyComp>(methods, key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (FILTER == FindFilter::ALL || FILTER == FindFilter::INSTANCE) {
        auto methods = GetMethods<FindFilter::INSTANCE>();
        auto *method = BinSearch<KeyComp>(methods, key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    // Copied methods come from implemented interfaces default methods and unsorted,
    // they can't be sorted by both id and name, so just visit method one by one now
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (FILTER == FindFilter::COPIED) {
        auto methods = GetMethods<FindFilter::COPIED>();
        for (auto &method : methods) {
            if (KeyComp().equal(method, key) && PredComp(method, preds...)) {
                return &method;
            }
        }
    }

    return nullptr;
}

template <Class::FindFilter FILTER, typename KeyComp, typename Key, typename... Pred>
// CC-OFFNXT(G.FUD.06) solid logic
inline Method *Class::FindClassMethod(Key key, const Pred &...preds) const
{
    auto *cls = this;
    while (cls != nullptr) {
        auto *method = cls->FindDirectMethod<FILTER, KeyComp>(key, preds...);
        if (method != nullptr) {
            return method;
        }
        cls = cls->GetBase();
    }

    if (FILTER == FindFilter::ALL || FILTER == FindFilter::INSTANCE) {
        return FindClassMethod<FindFilter::COPIED, KeyComp>(key, preds...);
    }

    return nullptr;
}

template <Class::FindFilter FILTER, typename KeyComp, typename Key, typename... Pred>
// CC-OFFNXT(G.FUD.06) solid logic
inline Method *Class::FindInterfaceMethod(Key key, const Pred &...preds) const
{
    static_assert(FILTER != FindFilter::COPIED, "interfaces don't have copied methods");

    if (LIKELY(IsInterface())) {
        auto *method = FindDirectMethod<FILTER, KeyComp>(key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    if (FILTER == FindFilter::STATIC) {
        return nullptr;
    }

    for (const auto &entry : itable_.Get()) {
        auto *iface = entry.GetInterface();
        auto *method = iface->FindDirectMethod<FindFilter::INSTANCE, KeyComp>(key, preds...);
        if (method != nullptr) {
            return method;
        }
    }

    if (LIKELY(IsInterface())) {
        return GetBase()->FindDirectMethod<FindFilter::INSTANCE, KeyComp>(
            key, [&](const Method &method) { return method.IsPublic() && PredComp(method, preds...); });
    }

    return nullptr;
}

inline Method *Class::GetVirtualInterfaceMethod(panda_file::File::EntityId id) const
{
    return FindInterfaceMethod<FindFilter::INSTANCE, MethodIdComp>(id);
}

inline Method *Class::GetStaticInterfaceMethod(panda_file::File::EntityId id) const
{
    return FindInterfaceMethod<FindFilter::STATIC, MethodIdComp>(id);
}

template <class Pred>
inline Field *Class::FindInstanceField(Pred pred) const
{
    return FindField<FindFilter::INSTANCE>(pred);
}

inline Field *Class::FindInstanceFieldById(panda_file::File::EntityId id) const
{
    return FindField<FindFilter::INSTANCE>(id);
}

template <class Pred>
inline Field *Class::FindStaticField(Pred pred) const
{
    return FindField<FindFilter::STATIC>(pred);
}

inline Field *Class::FindStaticFieldById(panda_file::File::EntityId id) const
{
    return FindField<FindFilter::STATIC>(id);
}

template <class Pred>
inline Field *Class::FindField(Pred pred) const
{
    return FindField<FindFilter::ALL>(pred);
}

template <class Pred>
inline Field *Class::FindDeclaredField(Pred pred) const
{
    return FindDeclaredField<FindFilter::ALL>(pred);
}

inline Field *Class::GetInstanceFieldByName(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindInstanceField([sd](const Field &field) { return field.GetName() == sd; });
}

inline Field *Class::GetStaticFieldByName(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindStaticField([sd](const Field &field) { return field.GetName() == sd; });
}

inline size_t Class::GetStaticFieldsOffset() const
{
    return ComputeClassSize(vtableSize_, imtSize_, 0, 0, 0, 0, 0, 0);
}

inline Field *Class::GetDeclaredFieldByName(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindDeclaredField([sd](const Field &field) { return field.GetName() == sd; });
}

inline Field *Class::GetDeclaredInstanceFieldByName(std::string_view name) const
{
    return FindDeclaredField<FindFilter::INSTANCE>([name](const Field &field) {
        auto fieldNameData = field.GetName().data;
        std::string_view fieldName(utf::Mutf8AsCString(fieldNameData));
        return fieldName == name;
    });
}

inline Field *Class::GetDeclaredInstanceFieldByName(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindDeclaredField<FindFilter::INSTANCE>([sd](const Field &field) { return field.GetName() == sd; });
}

inline Field *Class::GetDeclaredInstanceFieldByName(const uint8_t *mutf8Name, uint32_t mutf16Length) const
{
    panda_file::File::StringData sd = {mutf16Length, mutf8Name};
    return FindDeclaredField<FindFilter::INSTANCE>([sd](const Field &field) { return field.GetName() == sd; });
}

inline Field *Class::GetDeclaredStaticFieldByName(std::string_view name) const
{
    return FindDeclaredField<FindFilter::STATIC>([name](const Field &field) {
        auto fieldNameData = field.GetName().data;
        std::string_view fieldName(utf::Mutf8AsCString(fieldNameData));
        return fieldName == name;
    });
}

inline Field *Class::GetDeclaredStaticFieldByName(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindDeclaredField<FindFilter::STATIC>([sd](const Field &field) { return field.GetName() == sd; });
}

inline Field *Class::GetDeclaredStaticFieldByName(const uint8_t *mutf8Name, uint32_t mutf16Length) const
{
    panda_file::File::StringData sd = {mutf16Length, mutf8Name};
    return FindDeclaredField<FindFilter::STATIC>([sd](const Field &field) { return field.GetName() == sd; });
}

inline Method *Class::GetVirtualClassMethod(panda_file::File::EntityId id) const
{
    return FindClassMethod<FindFilter::INSTANCE, MethodIdComp>(id);
}

inline Method *Class::GetStaticClassMethod(panda_file::File::EntityId id) const
{
    return FindClassMethod<FindFilter::STATIC, MethodIdComp>(id);
}

inline Method *Class::GetDirectMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindDirectMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetClassMethod(const panda_file::File::StringData &sd, const Method::Proto &proto) const
{
    return FindClassMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetStaticClassMethodByName(const panda_file::File::StringData &sd,
                                                 const Method::Proto &proto) const
{
    return FindClassMethod<FindFilter::STATIC, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetVirtualClassMethodByName(const panda_file::File::StringData &sd,
                                                  const Method::Proto &proto) const
{
    return FindClassMethod<FindFilter::INSTANCE, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetInterfaceMethod(const panda_file::File::StringData &sd, const Method::Proto &proto) const
{
    return FindInterfaceMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetStaticInterfaceMethodByName(const panda_file::File::StringData &sd,
                                                     const Method::Proto &proto) const
{
    return FindInterfaceMethod<FindFilter::STATIC, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetVirtualInterfaceMethodByName(const panda_file::File::StringData &sd,
                                                      const Method::Proto &proto) const
{
    return FindInterfaceMethod<FindFilter::INSTANCE, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetDirectMethod(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindDirectMethod<FindFilter::ALL, MethodNameComp>(sd);
}

inline Method *Class::GetClassMethod(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindClassMethod<FindFilter::ALL, MethodNameComp>(sd);
}

inline Method *Class::GetStaticClassMethod(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindClassMethod<FindFilter::STATIC, MethodNameComp>(sd);
}

inline Method *Class::GetVirtualClassMethod(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindClassMethod<FindFilter::INSTANCE, MethodNameComp>(sd);
}

inline Method *Class::GetClassMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindClassMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetStaticClassMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindClassMethod<FindFilter::STATIC, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetVirtualClassMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindClassMethod<FindFilter::INSTANCE, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetInterfaceMethod(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindInterfaceMethod<FindFilter::ALL, MethodNameComp>(sd);
}

inline Method *Class::GetStaticInterfaceMethod(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindInterfaceMethod<FindFilter::STATIC, MethodNameComp>(sd);
}

inline Method *Class::GetVirtualInterfaceMethod(const uint8_t *mutf8Name) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindInterfaceMethod<FindFilter::INSTANCE, MethodNameComp>(sd);
}

inline Method *Class::GetInterfaceMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindInterfaceMethod<FindFilter::ALL, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetStaticInterfaceMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindInterfaceMethod<FindFilter::STATIC, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

inline Method *Class::GetVirtualInterfaceMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const
{
    panda_file::File::StringData sd = {static_cast<uint32_t>(ark::utf::MUtf8ToUtf16Size(mutf8Name)), mutf8Name};
    return FindInterfaceMethod<FindFilter::INSTANCE, MethodNameComp>(
        sd, [&proto](const Method &method) { return method.GetProtoId() == proto; });
}

// CC-OFFNXT(G.FUD.06) perf critical
inline Method *Class::ResolveVirtualMethod(const Method *method) const
{
    Method *resolved = nullptr;

    ASSERT(!IsInterface());

    if (method->GetClass()->IsInterface() && !method->IsDefaultInterfaceMethod()) {
        // find method in imtable
        auto imtableSize = GetIMTSize();
        if (LIKELY(imtableSize != 0)) {
            auto imtable = GetIMT();
            auto methodId = GetIMTableIndex(method->GetFileId().GetOffset());
            resolved = imtable[methodId];
            if (resolved != nullptr) {
                return resolved;
            }
        }

        // find method in itable
        auto *iface = method->GetClass();
        auto itable = GetITable();
        for (size_t i = 0; i < itable.Size(); i++) {
            auto &entry = itable[i];
            if (entry.GetInterface() != iface) {
                continue;
            }

            resolved = entry.GetMethods()[method->GetVTableIndex()];
            break;
        }
    } else {
        // find method in vtable
        auto vtable = GetVTable();
        ASSERT_PRINT(method->GetVTableIndex() < vtable.size(),
                     "cls = " << this << ", managed object = " << GetManagedObject());
        resolved = vtable[method->GetVTableIndex()];
    }

    return resolved;
}

constexpr size_t Class::ComputeClassSize(size_t vtableSize, size_t imtSize, size_t num8bitSfields,
                                         size_t num16bitSfields, size_t num32bitSfields, size_t num64bitSfields,
                                         size_t numRefSfields, size_t numTaggedSfields)
{
    size_t size = sizeof(Class);
    size = AlignUp(size, ClassHelper::OBJECT_POINTER_SIZE);
    size += vtableSize * ClassHelper::POINTER_SIZE;
    size += imtSize * ClassHelper::POINTER_SIZE;
    size += numRefSfields * ClassHelper::OBJECT_POINTER_SIZE;

    constexpr size_t SIZE_64 = sizeof(uint64_t);
    constexpr size_t SIZE_32 = sizeof(uint32_t);
    constexpr size_t SIZE_16 = sizeof(uint16_t);
    constexpr size_t SIZE_8 = sizeof(uint8_t);

    // Try to fill alignment gaps with fields that have smaller size from largest to smallests
    static_assert(coretypes::TaggedValue::TaggedTypeSize() == SIZE_64,
                  "Please fix alignment of the fields of type \"TaggedValue\"");
    if (!IsAligned<SIZE_64>(size) && (num64bitSfields > 0 || numTaggedSfields > 0)) {
        size_t padding = AlignUp(size, SIZE_64) - size;
        size += padding;

        Pad(SIZE_32, &padding, &num32bitSfields);
        Pad(SIZE_16, &padding, &num16bitSfields);
        Pad(SIZE_8, &padding, &num8bitSfields);
    }

    if (!IsAligned<SIZE_32>(size) && num32bitSfields > 0) {
        size_t padding = AlignUp(size, SIZE_32) - size;
        size += padding;

        Pad(SIZE_16, &padding, &num16bitSfields);
        Pad(SIZE_8, &padding, &num8bitSfields);
    }

    if (!IsAligned<SIZE_16>(size) && num16bitSfields > 0) {
        size_t padding = AlignUp(size, SIZE_16) - size;
        size += padding;

        Pad(SIZE_8, &padding, &num8bitSfields);
    }

    size += num64bitSfields * SIZE_64 + num32bitSfields * SIZE_32 + num16bitSfields * SIZE_16 +
            num8bitSfields * SIZE_8 + numTaggedSfields * coretypes::TaggedValue::TaggedTypeSize();

    return size;
}

constexpr void Class::Pad(size_t size, size_t *padding, size_t *n)
{
    while (*padding >= size && *n > 0) {
        *padding -= size;
        *n -= 1;
    }
}

constexpr size_t Class::GetVTableOffset()
{
    return ComputeClassSize(0, 0, 0, 0, 0, 0, 0, 0);
}

inline Span<Method *> Class::GetVTable()
{
    return GetClassSpan().SubSpan<Method *>(GetVTableOffset(), vtableSize_);
}

inline Span<Method *const> Class::GetVTable() const
{
    return GetClassSpan().SubSpan<Method *const>(GetVTableOffset(), vtableSize_);
}

inline size_t Class::GetIMTOffset() const
{
    return GetVTableOffset() + vtableSize_ * sizeof(uintptr_t);
}

// IS_VOLATILE = false
template <class T, bool IS_VOLATILE>
inline T Class::GetFieldPrimitive(size_t offset) const
{
    ASSERT_DO(IsInitializing() || IsInitialized(), LOG(ERROR, RUNTIME) << "class state: " << state_);
    return ObjectAccessor::GetPrimitive<T, IS_VOLATILE>(this, offset);
}

// IS_VOLATILE = false
template <class T, bool IS_VOLATILE>
inline void Class::SetFieldPrimitive(size_t offset, T value)
{
    ObjectAccessor::SetPrimitive<T, IS_VOLATILE>(this, offset, value);
}

// IS_VOLATILE = false , bool NEED_READ_BARRIER = true
template <bool IS_VOLATILE, bool NEED_READ_BARRIER>
inline ObjectHeader *Class::GetFieldObject(size_t offset) const
{
    // NOTE(alovkov): GC can skip classes which are IsErroneous #6458
    // GC can't easily check state & get fields because state can be changed concurrently and checking on every field
    // is too expensive and it should be atomic {check state, get field}
    ASSERT_DO(IsInitializing() || IsInitialized() || IsErroneous(), LOG(ERROR, RUNTIME) << "class state: " << state_);
    return ObjectAccessor::GetObject<IS_VOLATILE, NEED_READ_BARRIER>(this, offset);
}

// IS_VOLATILE = false , bool NEED_WRITE_BARRIER = true
template <bool IS_VOLATILE, bool NEED_WRITE_BARRIER>
inline void Class::SetFieldObject(size_t offset, ObjectHeader *value)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto newOffset = offset + (ToUintPtr(this) - ToUintPtr(object));
    ObjectAccessor::SetObject<IS_VOLATILE, NEED_WRITE_BARRIER>(object, newOffset, value);
}

template <class T>
inline T Class::GetFieldPrimitive(const Field &field) const
{
    return ObjectAccessor::GetFieldPrimitive<T>(this, field);
}

template <class T>
inline void Class::SetFieldPrimitive(const Field &field, T value)
{
    ObjectAccessor::SetFieldPrimitive(this, field, value);
}

// NEED_READ_BARRIER = true
template <bool NEED_READ_BARRIER>
inline ObjectHeader *Class::GetFieldObject(const Field &field) const
{
    return ObjectAccessor::GetFieldObject<NEED_READ_BARRIER>(this, field);
}

// NEED_WRITE_BARRIER = true
template <bool NEED_WRITE_BARRIER>
inline void Class::SetFieldObject(const Field &field, ObjectHeader *value)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto offset = field.GetOffset() + (ToUintPtr(this) - ToUintPtr(object));
    if (UNLIKELY(field.IsVolatile())) {
        ObjectAccessor::SetObject<true, NEED_WRITE_BARRIER>(object, offset, value);
    } else {
        ObjectAccessor::SetObject<false, NEED_WRITE_BARRIER>(object, offset, value);
    }
}

// NEED_READ_BARRIER = true
template <bool NEED_READ_BARRIER>
inline ObjectHeader *Class::GetFieldObject(ManagedThread *thread, const Field &field) const
{
    return ObjectAccessor::GetFieldObject<NEED_READ_BARRIER>(thread, this, field);
}

// NEED_WRITE_BARRIER = true
template <bool NEED_WRITE_BARRIER>
inline void Class::SetFieldObject(ManagedThread *thread, const Field &field, ObjectHeader *value)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto offset = field.GetOffset() + (ToUintPtr(this) - ToUintPtr(object));
    if (UNLIKELY(field.IsVolatile())) {
        ObjectAccessor::SetObject<true, NEED_WRITE_BARRIER>(thread, object, offset, value);
    } else {
        ObjectAccessor::SetObject<false, NEED_WRITE_BARRIER>(thread, object, offset, value);
    }
}

template <class T>
inline T Class::GetFieldPrimitive(size_t offset, std::memory_order memoryOrder) const
{
    return ObjectAccessor::GetFieldPrimitive<T>(this, offset, memoryOrder);
}

template <class T>
inline void Class::SetFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    ObjectAccessor::SetFieldPrimitive(this, offset, value, memoryOrder);
}

// NEED_READ_BARRIER = true
template <bool NEED_READ_BARRIER>
inline ObjectHeader *Class::GetFieldObject(size_t offset, std::memory_order memoryOrder) const
{
    return ObjectAccessor::GetFieldObject<NEED_READ_BARRIER>(this, offset, memoryOrder);
}

// NEED_WRITE_BARRIER = true
template <bool NEED_WRITE_BARRIER>
inline void Class::SetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto newOffset = offset + (ToUintPtr(this) - ToUintPtr(object));
    ObjectAccessor::SetFieldObject<NEED_WRITE_BARRIER>(object, newOffset, value, memoryOrder);
}

template <typename T>
inline bool Class::CompareAndSetFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder,
                                               bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, offset, oldValue, newValue, memoryOrder, strong).first;
}

// NEED_WRITE_BARRIER = true
template <bool NEED_WRITE_BARRIER>
inline bool Class::CompareAndSetFieldObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                            std::memory_order memoryOrder, bool strong)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto newOffset = offset + (ToUintPtr(this) - ToUintPtr(object));
    return ObjectAccessor::CompareAndSetFieldObject<NEED_WRITE_BARRIER>(object, newOffset, oldValue, newValue,
                                                                        memoryOrder, strong)
        .first;
}

template <typename T>
inline T Class::CompareAndExchangeFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder,
                                                 bool strong)
{
    return ObjectAccessor::CompareAndSetFieldPrimitive(this, offset, oldValue, newValue, memoryOrder, strong).second;
}

// NEED_WRITE_BARRIER = true
template <bool NEED_WRITE_BARRIER>
inline ObjectHeader *Class::CompareAndExchangeFieldObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                                          std::memory_order memoryOrder, bool strong)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto newOffset = offset + (ToUintPtr(this) - ToUintPtr(object));
    return ObjectAccessor::CompareAndSetFieldObject<NEED_WRITE_BARRIER>(object, newOffset, oldValue, newValue,
                                                                        memoryOrder, strong)
        .second;
}

template <typename T>
inline T Class::GetAndSetFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndSetFieldPrimitive(this, offset, value, memoryOrder);
}

// NEED_WRITE_BARRIER = true
template <bool NEED_WRITE_BARRIER>
inline ObjectHeader *Class::GetAndSetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder)
{
    auto object = GetManagedObject();
    ASSERT(ToUintPtr(object) < ToUintPtr(this) && ToUintPtr(this) < ToUintPtr(object) + object->ObjectSize());
    auto newOffset = offset + (ToUintPtr(this) - ToUintPtr(object));
    return ObjectAccessor::GetAndSetFieldObject<NEED_WRITE_BARRIER>(object, newOffset, value, memoryOrder);
}

template <typename T>
inline T Class::GetAndAddFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndAddFieldPrimitive(this, offset, value, memoryOrder);
}

template <typename T>
inline T Class::GetAndBitwiseOrFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndBitwiseOrFieldPrimitive(this, offset, value, memoryOrder);
}

template <typename T>
inline T Class::GetAndBitwiseAndFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndBitwiseAndFieldPrimitive(this, offset, value, memoryOrder);
}

template <typename T>
inline T Class::GetAndBitwiseXorFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder)
{
    return ObjectAccessor::GetAndBitwiseXorFieldPrimitive(this, offset, value, memoryOrder);
}

}  // namespace ark

#endif  // PANDA_RUNTIME_CLASS_INL_H_
