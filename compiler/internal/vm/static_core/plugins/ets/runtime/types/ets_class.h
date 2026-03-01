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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_CLASS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_CLASS_H_

#include <atomic>
#include <cstdint>
#include "include/mem/panda_containers.h"
#include "include/mem/panda_string.h"
#include "include/object_header.h"
#include "include/runtime.h"
#include "libarkbase/mem/object_pointer.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/class_linker.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/types/ets_method_signature.h"
#include "libarkbase/utils/utf.h"

namespace ark::ets {

enum class AccessLevel { PUBLIC, PROTECTED, DEFAULT, PRIVATE };

class EtsMethod;
class EtsObject;
class EtsString;
class EtsArray;
class EtsPromise;
class EtsJob;
class EtsErrorOptions;
class EtsTypeAPIField;
class EtsTypeAPIMethod;
class EtsTypeAPIParameter;
class EtsReflectMethod;
class EtsReflectField;
class EtsRuntimeLinker;

enum class EtsType;

namespace test {
class EtsClassTest;
}  // namespace test

class EtsClass {
public:
    // We shouldn't init header_ here - because it has been memset(0) in object allocation,
    // otherwise it may cause data race while visiting object's class concurrently in gc.
    void InitClass(const uint8_t *descriptor, uint32_t vtableSize, uint32_t imtSize, uint32_t klassSize)
    {
        new (&klass_) ark::Class(descriptor, panda_file::SourceLang::ETS, vtableSize, imtSize, klassSize);
    }

    const char *GetDescriptor() const
    {
        return utf::Mutf8AsCString(GetRuntimeClass()->GetDescriptor());
    }

    PANDA_PUBLIC_API EtsClass *GetBase();

    uint32_t GetFieldsNumber();

    uint32_t GetOwnFieldsNumber();  // Without inherited fields

    uint32_t GetStaticFieldsNumber()
    {
        return GetRuntimeClass()->GetStaticFields().size();
    }

    uint32_t GetInstanceFieldsNumber()
    {
        return GetRuntimeClass()->GetInstanceFields().size();
    }

    uint32_t GetFieldIndexByName(const char *name);

    PANDA_PUBLIC_API PandaVector<EtsField *> GetFields();

    EtsField *GetFieldByIndex(uint32_t i);

    EtsField *GetFieldIDByOffset(uint32_t fieldOffset);
    PANDA_PUBLIC_API EtsField *GetFieldIDByName(const char *name, const char *sig = nullptr);

    EtsField *GetOwnFieldByIndex(uint32_t i);
    EtsField *GetDeclaredFieldIDByName(std::string_view name);

    PANDA_PUBLIC_API EtsField *GetStaticFieldIDByName(const char *name, const char *sig = nullptr);
    EtsField *GetStaticFieldIDByOffset(uint32_t fieldOffset);

    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const char *name);
    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const uint8_t *name, const char *signature);
    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const char *name, const char *signature);

    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(bool isStatic, const char *name, const char *signature) const;
    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(bool isStatic, const char *name,
                                                const EtsMethodSignature &methodSignature) const;
    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(bool isStatic, const char *name, bool *outIsUnique) const;

    PANDA_PUBLIC_API EtsMethod *GetStaticMethod(const char *name, const char *signature) const
    {
        if (signature == nullptr) {
            return GetMethodInternal<FindFilter::STATIC>(name);
        }
        return GetMethodInternal<FindFilter::STATIC>(name, signature);
    }

    PANDA_PUBLIC_API EtsMethod *GetStaticMethod(const char *name,
                                                std::optional<EtsMethodSignature> &methodSignature) const
    {
        if (!methodSignature.has_value()) {
            return GetMethodInternal<FindFilter::STATIC>(name);
        }
        return GetMethodInternal<FindFilter::STATIC>(name, methodSignature.value());
    }

    PANDA_PUBLIC_API EtsMethod *GetInstanceMethod(const char *name, const char *signature) const
    {
        if (signature == nullptr) {
            return GetMethodInternal<FindFilter::INSTANCE>(name);
        }
        return GetMethodInternal<FindFilter::INSTANCE>(name, signature);
    }

    PANDA_PUBLIC_API EtsMethod *GetInstanceMethod(const char *name,
                                                  std::optional<EtsMethodSignature> &methodSignature) const
    {
        if (!methodSignature.has_value()) {
            return GetMethodInternal<FindFilter::INSTANCE>(name);
        }
        return GetMethodInternal<FindFilter::INSTANCE>(name, methodSignature.value());
    }

    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const PandaString &name, const PandaString &signature)
    {
        return GetDirectMethod(name.c_str(), signature.c_str());
    }

    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const char *name, const Method::Proto &proto) const;

    EtsMethod *GetMethodByIndex(uint32_t i);

    uint32_t GetMethodsNum();

    PandaVector<EtsMethod *> GetMethods();

    PANDA_PUBLIC_API PandaVector<EtsMethod *> GetConstructors();

    template <class T>
    T GetStaticFieldPrimitive(EtsField *field)
    {
        return GetRuntimeClass()->GetFieldPrimitive<T>(*field->GetRuntimeField());
    }

    template <class T>
    T GetStaticFieldPrimitive(int32_t fieldOffset, bool isVolatile)
    {
        if (isVolatile) {
            return GetRuntimeClass()->GetFieldPrimitive<T, true>(fieldOffset);
        }
        return GetRuntimeClass()->GetFieldPrimitive<T, false>(fieldOffset);
    }

    template <class T>
    void SetStaticFieldPrimitive(EtsField *field, T value)
    {
        GetRuntimeClass()->SetFieldPrimitive<T>(*field->GetRuntimeField(), value);
    }

    template <class T>
    void SetStaticFieldPrimitive(int32_t fieldOffset, bool isVolatile, T value)
    {
        if (isVolatile) {
            GetRuntimeClass()->SetFieldPrimitive<T, true>(fieldOffset, value);
        }
        GetRuntimeClass()->SetFieldPrimitive<T, false>(fieldOffset, value);
    }

    PANDA_PUBLIC_API EtsObject *GetStaticFieldObject(EtsField *field);
    EtsObject *GetStaticFieldObject(int32_t fieldOffset, bool isVolatile);

    void SetStaticFieldObject(EtsField *field, EtsObject *value);
    void SetStaticFieldObject(int32_t fieldOffset, bool isVolatile, EtsObject *value);

    EtsObject *CreateInstance();

    bool IsEtsObject()
    {
        return GetRuntimeClass()->IsObjectClass();
    }

    bool IsPrimitive() const
    {
        return GetRuntimeClass()->IsPrimitive();
    }

    bool IsVoid() const
    {
        return GetRuntimeClass()->IsVoid();
    }

    bool IsAbstract()
    {
        return GetRuntimeClass()->IsAbstract();
    }

    bool IsPublic() const
    {
        return GetRuntimeClass()->IsPublic();
    }

    bool IsFinal() const
    {
        return GetRuntimeClass()->IsFinal();
    }

    bool IsExtensible() const
    {
        return GetRuntimeClass()->IsExtensible();
    }

    bool IsAnnotation() const
    {
        return GetRuntimeClass()->IsAnnotation();
    }

    bool IsEnum() const
    {
        return GetRuntimeClass()->IsEnum();
    }

    bool IsStringClass() const
    {
        return GetRuntimeClass()->IsStringClass();
    }

    bool IsClassClass() const
    {
        return GetRuntimeClass()->IsClassClass();
    }

    bool IsArrayClass() const
    {
        return GetRuntimeClass()->IsArrayClass();
    }

    bool IsUnionClass() const
    {
        return GetRuntimeClass()->IsUnionClass();
    }

    bool IsInterface() const
    {
        return GetRuntimeClass()->IsInterface();
    }

    bool IsClass() const
    {
        return GetRuntimeClass()->IsClass();
    }

    static bool IsInSamePackage(std::string_view className1, std::string_view className2);

    bool IsInSamePackage(EtsClass *that);

    uint32_t GetComponentSize() const
    {
        return GetRuntimeClass()->GetComponentSize();
    }

    panda_file::Type GetType() const
    {
        return GetRuntimeClass()->GetType();
    }

    bool IsSubClass(EtsClass *cls)
    {
        return GetRuntimeClass()->IsSubClassOf(cls->GetRuntimeClass());
    }

    bool IsAssignableFrom(EtsClass *clazz) const
    {
        return GetRuntimeClass()->IsAssignableFrom(clazz->GetRuntimeClass());
    }

    bool IsGenerated() const
    {
        return IsArrayClass() || IsPrimitive();
    }

    bool CanAccess(EtsClass *that)
    {
        return that->IsPublic() || IsInSamePackage(that);
    }

    EtsMethod *ResolveVirtualMethod(const EtsMethod *method) const;

    EtsClass *ResolvePublicClass();

    PANDA_PUBLIC_API EtsClass *ResolveStringClass();

    template <class Callback>
    void EnumerateDirectMethods(const Callback &callback) const
    {
        for (auto &method : GetRuntimeClass()->GetMethods()) {
            bool finished = callback(reinterpret_cast<EtsMethod *>(&method));
            if (finished) {
                break;
            }
        }
    }

    template <class Callback>
    void EnumerateVtable(Callback cb)
    {
        auto vtable = GetRuntimeClass()->GetVTable();
        for (auto *method : vtable) {
            bool res = cb(method);
            if (res) {
                break;
            }
        }
    }

    template <class Callback>
    void EnumerateDirectInterfaces(const Callback &callback) const
    {
        for (Class *runtimeInterface : GetRuntimeClass()->GetInterfaces()) {
            EtsClass *interface = EtsClass::FromRuntimeClass(runtimeInterface);
            bool finished = callback(interface);
            if (finished) {
                break;
            }
        }
    }

    void GetInterfaces(PandaUnorderedSet<EtsClass *> &ifaces, EtsClass *iface);

    template <class Callback>
    void EnumerateInterfaces(const Callback &callback)
    {
        PandaUnorderedSet<EtsClass *> ifaces;
        GetInterfaces(ifaces, this);
        for (auto iface : ifaces) {
            bool finished = callback(iface);
            if (finished) {
                break;
            }
        }
    }

    template <class Callback>
    void EnumerateConstituentClasses(const Callback &callback)
    {
        for (Class *runtimeClasses : GetRuntimeClass()->GetConstituentTypes()) {
            EtsClass *klass = EtsClass::FromRuntimeClass(runtimeClasses);
            bool finished = callback(klass);
            if (finished) {
                break;
            }
        }
    }

    template <class Callback>
    void EnumerateBaseClasses(const Callback &callback)
    {
        PandaVector<EtsClass *> inherChain;
        auto curClass = this;
        while (curClass != nullptr) {
            inherChain.push_back(curClass);
            curClass = curClass->GetBase();
        }
        for (auto i = inherChain.rbegin(); i != inherChain.rend(); i++) {
            bool finished = callback(*i);
            if (finished) {
                break;
            }
        }
    }

    ClassLinkerContext *GetLoadContext() const
    {
        return GetRuntimeClass()->GetLoadContext();
    }

    Class *GetRuntimeClass()
    {
        return &klass_;
    }

    const Class *GetRuntimeClass() const
    {
        return &klass_;
    }

    EtsLong GetTypeMetaData() const
    {
        return typeMetaData_;
    }

    void SetTypeMetaData(EtsLong metaData)
    {
        typeMetaData_ = metaData;
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    bool IsInitialized() const
    {
        return GetRuntimeClass()->IsInitialized();
    }

    EtsType GetEtsType()
    {
        return ConvertPandaTypeToEtsType(GetRuntimeClass()->GetType());
    }

    static size_t GetSize(uint32_t klassSize)
    {
        return GetRuntimeClassOffset() + klassSize;
    }

    static constexpr size_t GetRuntimeClassOffset()
    {
        return MEMBER_OFFSET(EtsClass, klass_);
    }

    static constexpr size_t GetTypeMetaDataOffset()
    {
        return MEMBER_OFFSET(EtsClass, typeMetaData_);
    }

    static constexpr size_t GetHeaderOffset()
    {
        return MEMBER_OFFSET(EtsClass, header_);
    }

    static EtsClass *FromRuntimeClass(const Class *cls)
    {
        ASSERT(cls != nullptr);
        return reinterpret_cast<EtsClass *>(reinterpret_cast<uintptr_t>(cls) - GetRuntimeClassOffset());
    }

    static EtsClass *FromClassObject(ObjectHeader *obj)
    {
        ASSERT(obj->ClassAddr<Class>()->IsClassClass());
        return reinterpret_cast<EtsClass *>(obj);
    }

    static EtsClass *FromEtsClassObject(EtsObject *obj)
    {
        return FromClassObject(reinterpret_cast<ObjectHeader *>(obj));
    }

    void Initialize(EtsClass *superClass, uint16_t accessFlags, bool isPrimitiveType,
                    ClassLinkerErrorHandler *errorHandler);

    void SetComponentType(EtsClass *componentType);

    EtsClass *GetComponentType() const;

    void SetName(EtsString *name);

    bool CompareAndSetName(EtsString *oldName, EtsString *newName);

    EtsString *GetName();
    static EtsString *CreateEtsClassName([[maybe_unused]] const char *descriptor);

    void SetSuperClass(EtsClass *superClass)
    {
        auto obj = reinterpret_cast<ObjectHeader *>(superClass);
        GetObjectHeader()->SetFieldObject(GetSuperClassOffset(), obj);
    }

    EtsClass *GetSuperClass() const
    {
        return reinterpret_cast<EtsClass *>(GetObjectHeader()->GetFieldObject(GetSuperClassOffset()));
    }

    void SetFlags(uint32_t flags)
    {
        GetObjectHeader()->SetFieldPrimitive(GetFlagsOffset(), flags);
    }

    uint32_t GetFlags() const
    {
        return GetObjectHeader()->GetFieldPrimitive<uint32_t>(GetFlagsOffset());
    }

    enum class BoxedType { BOOLEAN, BYTE, CHAR, SHORT, INT, LONG, FLOAT, DOUBLE };

    void SetWeakReference();
    void SetFinalizeReference();
    void SetValueTyped();
    void SetNullValue();
    void SetBoxedKind(BoxedType boxedKind);
    void SetFunction();
    void SetEtsEnum();
    void SetBigInt();

    [[nodiscard]] bool IsWeakReference() const
    {
        return (flags_ & IS_WEAK_REFERENCE) != 0;
    }

    [[nodiscard]] bool IsFinalizerReference() const
    {
        return (flags_ & IS_FINALIZE_REFERENCE) != 0;
    }

    /// True if class inherited from Reference, false otherwise
    [[nodiscard]] bool IsReference() const
    {
        return (flags_ & IS_REFERENCE) != 0;
    }

    [[nodiscard]] bool IsValueTyped() const
    {
        return (GetFlags() & IS_VALUE_TYPED) != 0;
    }

    [[nodiscard]] bool IsNullValue() const
    {
        return (GetFlags() & IS_NULLVALUE) != 0;
    }

    [[nodiscard]] bool IsBoxed() const
    {
        return (GetFlags() & IS_BOXED) != 0;
    }

    [[nodiscard]] BoxedType GetBoxedType() const
    {
        return static_cast<BoxedType>(BoxedTypeField::Get(GetFlags()));
    }

    [[nodiscard]] bool IsBoxedDouble() const
    {
        return GetBoxedType() == BoxedType::DOUBLE;
    }

    [[nodiscard]] bool IsBoxedFloat() const
    {
        return GetBoxedType() == BoxedType::FLOAT;
    }

    [[nodiscard]] bool IsBoxedInt() const
    {
        return GetBoxedType() == BoxedType::INT;
    }

    [[nodiscard]] bool IsFunction() const
    {
        return (GetFlags() & IS_FUNCTION) != 0;
    }

    [[nodiscard]] bool IsFunctionReference() const
    {
        return (GetFlags() & IS_FUNCTION_REFERENCE) != 0;
    }

    [[nodiscard]] bool IsEtsEnum() const
    {
        return (GetFlags() & IS_ETS_ENUM) != 0;
    }

    [[nodiscard]] bool IsBigInt() const
    {
        return (GetFlags() & IS_BIGINT) != 0;
    }

    [[nodiscard]] bool IsModule() const
    {
        return (GetFlags() & IS_MODULE) != 0;
    }

    EtsClass() = delete;
    ~EtsClass() = delete;
    NO_COPY_SEMANTIC(EtsClass);
    NO_MOVE_SEMANTIC(EtsClass);

    static constexpr size_t GetNameOffset()
    {
        return MEMBER_OFFSET(EtsClass, name_);
    }

    static constexpr size_t GetSuperClassOffset()
    {
        return MEMBER_OFFSET(EtsClass, superClass_);
    }

    static constexpr size_t GetFlagsOffset()
    {
        return MEMBER_OFFSET(EtsClass, flags_);
    }

    static constexpr size_t GCRefFieldsOffset()
    {
        return GetHeaderOffset() + sizeof(header_);
    }

    static constexpr size_t GCRefFieldsNum()
    {
        return (GetFlagsOffset() - GCRefFieldsOffset()) / sizeof(ObjectPointerType);
    }

private:
    enum class FindFilter { STATIC, INSTANCE, ALL };

    template <FindFilter FILTER>
    EtsMethod *GetMethodInternal(const char *name) const
    {
        const auto *coreName = utf::CStringAsMutf8(name);

        Method *coreMethod = nullptr;
        auto *runtimeClass = GetRuntimeClass();
        if (IsInterface()) {
            if constexpr (FILTER == FindFilter::STATIC) {
                coreMethod = runtimeClass->GetStaticInterfaceMethod(coreName);
            } else {
                static_assert(FILTER == FindFilter::INSTANCE);
                coreMethod = runtimeClass->GetVirtualInterfaceMethod(coreName);
            }
        } else {
            if constexpr (FILTER == FindFilter::STATIC) {
                coreMethod = runtimeClass->GetStaticClassMethod(coreName);
            } else {
                static_assert(FILTER == FindFilter::INSTANCE);
                coreMethod = runtimeClass->GetVirtualClassMethod(coreName);
            }
        }
        return reinterpret_cast<EtsMethod *>(coreMethod);
    }

    template <FindFilter FILTER>
    EtsMethod *GetMethodInternal(const char *name, const char *signature) const
    {
        EtsMethodSignature methodSignature(signature);
        if (!methodSignature.IsValid()) {
            LOG(ERROR, ETS_NAPI) << "Wrong method signature:" << signature;
            return nullptr;
        }

        return this->GetMethodInternal<FILTER>(name, methodSignature);
    }

    template <FindFilter FILTER>
    EtsMethod *GetMethodInternal(const char *name, const EtsMethodSignature &methodSignature) const
    {
        const auto *coreName = utf::CStringAsMutf8(name);

        Method *coreMethod = nullptr;
        auto *runtimeClass = GetRuntimeClass();
        if (IsInterface()) {
            if constexpr (FILTER == FindFilter::STATIC) {
                coreMethod = runtimeClass->GetStaticInterfaceMethod(coreName, methodSignature.GetProto());
            } else {
                static_assert(FILTER == FindFilter::INSTANCE);
                coreMethod = runtimeClass->GetVirtualInterfaceMethod(coreName, methodSignature.GetProto());
            }
        } else {
            if constexpr (FILTER == FindFilter::STATIC) {
                coreMethod = runtimeClass->GetStaticClassMethod(coreName, methodSignature.GetProto());
            } else {
                static_assert(FILTER == FindFilter::INSTANCE);
                coreMethod = runtimeClass->GetVirtualClassMethod(coreName, methodSignature.GetProto());
            }
        }
        return reinterpret_cast<EtsMethod *>(coreMethod);
    }

    void SetLinker(ObjectHeader *linker);

    ObjectHeader *GetObjectHeader()
    {
        return &header_;
    }

    const ObjectHeader *GetObjectHeader() const
    {
        return &header_;
    }

    constexpr static uint32_t ETS_ACC_PRIMITIVE = 1U << 16U;
    /// Class is a WeakReference or successor of this class
    constexpr static uint32_t IS_WEAK_REFERENCE = 1U << 17U;
    /// Class is a FinalizerReference or successor of this class
    constexpr static uint32_t IS_FINALIZE_REFERENCE = 1U << 18U;
    constexpr static uint32_t IS_REFERENCE = IS_WEAK_REFERENCE | IS_FINALIZE_REFERENCE;

public:
    // Class is a value-semantic type
    constexpr static uint32_t IS_VALUE_TYPED = 1U << 19U;

    // Class is an internal "nullvalue" class
    constexpr static uint32_t IS_NULLVALUE = 1U << 20U;
    // Class is a boxed type
    constexpr static uint32_t IS_BOXED = 1U << 21U;
    // Class is Function
    constexpr static uint32_t IS_FUNCTION = 1U << 22U;
    // Class is BigInt
    constexpr static uint32_t IS_BIGINT = 1U << 23U;
    // Class is Module
    constexpr static uint32_t IS_MODULE = 1U << 24U;
    // Class is enum
    constexpr static uint32_t IS_ETS_ENUM = 1U << 25U;
    // Class is Function Reference
    constexpr static uint32_t IS_FUNCTION_REFERENCE = 1U << 26U;
    // To get information about boxed type.
    constexpr static uint32_t BOXED_TYPE_FIELD_START = 27;
    constexpr static uint32_t BOXED_TYPE_FIELD_SIZE = 3;

private:
    using BoxedTypeField = BitField<uint32_t, BOXED_TYPE_FIELD_START, BOXED_TYPE_FIELD_SIZE>;
    ark::ObjectHeader header_;  // EtsObject

    // ets.Class fields BEGIN
    FIELD_UNUSED ObjectPointer<EtsString> name_;       // String
    FIELD_UNUSED ObjectPointer<EtsClass> superClass_;  // Class<? super T>
    FIELD_UNUSED ObjectPointer<EtsRuntimeLinker> linker_;
#if defined(PANDA_32_BIT_MANAGED_POINTER)
    FIELD_UNUSED uint32_t flags_;
    FIELD_UNUSED EtsLong typeMetaData_;
#else
    FIELD_UNUSED EtsLong typeMetaData_;
    FIELD_UNUSED uint32_t flags_;
#endif
    // NOTE(zhushihao8, #20712) remove this field after removal of `GetMethodsNum`
    std::atomic_uint32_t methodsNum_ {0};
    // ets.Class fields END

    ark::Class klass_;

    friend class test::EtsClassTest;
};

// Object header field must be first
static_assert(EtsClass::GetHeaderOffset() == 0);

// Klass field has variable size so it must be last
static_assert(EtsClass::GetRuntimeClassOffset() + sizeof(ark::Class) == sizeof(EtsClass));

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_CLASS_H_
