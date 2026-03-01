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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_METHOD_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_METHOD_H_

#include <string_view>
#include "libarkbase/macros.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/file_items.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"
#include "runtime/include/thread.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/types/ets_value.h"

namespace ark {
class Value;
}  // namespace ark

namespace ark::ets {
namespace ani {
class ScopedManagedCodeFix;
}  // namespace ani

class PandaEtsEnv;

class EtsMethod {
public:
    PANDA_PUBLIC_API static EtsMethod *FromTypeDescriptor(const PandaString &td, EtsRuntimeLinker *contextLinker);

    PANDA_PUBLIC_API static bool IsMethod(const PandaString &td);

    PANDA_PUBLIC_API ani_status Invoke(ani::ScopedManagedCodeFix &s, Value *args, EtsValue *result);

    uint32_t GetParametersNum() const
    {
        if (IsStatic()) {
            return GetNumArgs();
        }
        return GetNumArgs() - 1;
    }

    uint32_t GetNumArgs() const
    {
        return GetPandaMethod()->GetNumArgs();
    }

    PANDA_PUBLIC_API uint32_t GetNumArgSlots() const;

    uint32_t GetNumMandatoryArgs();

    std::optional<uint32_t> TryGetMinArgCount();

    EtsType GetArgType(size_t idx) const
    {
        EtsType etsType = ConvertPandaTypeToEtsType(GetPandaMethod()->GetArgType(idx));
        if (etsType == EtsType::VOID) {
            LOG(FATAL, RUNTIME) << "VOID parameter";
        }
        return etsType;
    }

    EtsType GetReturnValueType() const
    {
        panda_file::Type pandaType = GetPandaMethod()->GetReturnType();
        return ConvertPandaTypeToEtsType(pandaType);
    }

    EtsType GetEffectiveArgType(size_t idx) const
    {
        EtsType etsType = ConvertPandaTypeToEtsType(GetPandaMethod()->GetEffectiveArgType(idx));
        if (etsType == EtsType::VOID) {
            LOG(FATAL, RUNTIME) << "VOID parameter";
        }
        return etsType;
    }

    EtsType GetEffectiveReturnValueType() const
    {
        panda_file::Type pandaType = GetPandaMethod()->GetEffectiveReturnType();
        return ConvertPandaTypeToEtsType(pandaType);
    }

    const char *GetRefArgType(size_t idx) const
    {
        return utf::Mutf8AsCString(GetPandaMethod()->GetRefArgType(idx).data);
    }

    const char *GetName() const
    {
        return utf::Mutf8AsCString(GetPandaMethod()->GetName().data);
    }

    PandaString GetFullName(bool withSignature = false) const
    {
        return GetPandaMethod()->GetFullName(withSignature);
    }

    EtsString *GetNameString()
    {
        auto nameData = GetPandaMethod()->GetName();
        return EtsString::Resolve(nameData.data, nameData.utf16Length);
    }

    bool IsStatic() const
    {
        return GetPandaMethod()->IsStatic();
    }

    bool IsSynthetic() const
    {
        return GetPandaMethod()->IsSynthetic();
    }

    PANDA_PUBLIC_API bool IsEqualParametersType(EtsArray *params) const;

    EtsClass *GetClass() const
    {
        return EtsClass::FromRuntimeClass(GetPandaMethod()->GetClass());
    }

    PANDA_PUBLIC_API EtsClass *ResolveArgType(uint32_t idx);

    EtsClass *ResolveReturnType();

    size_t GetVTableID() const
    {
        return GetPandaMethod()->GetVTableIndex();
    }

    bool IsPublic() const
    {
        return GetPandaMethod()->IsPublic();
    }

    bool IsProtected() const
    {
        return GetPandaMethod()->IsProtected();
    }

    bool IsPrivate() const
    {
        return GetPandaMethod()->IsPrivate();
    }

    bool IsNative() const
    {
        return GetPandaMethod()->IsNative();
    }

    bool IsProxy() const
    {
        return GetPandaMethod()->IsProxy();
    }

    void SetProxyPointer(EtsMethod *proxyPointer)
    {
        ASSERT(IsProxy());
        // Proxy method is never native, so we have free field.
        return GetPandaMethod()->SetNativePointer(reinterpret_cast<void *>(proxyPointer));
    }

    EtsMethod *GetProxyPointer() const
    {
        ASSERT(IsProxy());
        return reinterpret_cast<EtsMethod *>(GetPandaMethod()->GetNativePointer());
    }

    EtsMethod *GetInterfaceMethodIfProxy()
    {
        if (UNLIKELY(IsProxy())) {
            return GetProxyPointer();
        }
        return this;
    }

    bool HasRestParam() const
    {
        return (this->GetAccessFlags() & ACC_VARARGS) != 0;
    }

    bool IsFinal() const
    {
        return GetPandaMethod()->IsFinal();
    }

    bool IsFastNative() const
    {
        return (GetAccessFlags() & ACC_FAST_NATIVE) != 0;
    }

    bool IsCriticalNative() const
    {
        return (GetAccessFlags() & ACC_CRITICAL_NATIVE) != 0;
    }

    bool IsBoundNativeFunction() const
    {
        ASSERT(!IsIntrinsic());
        return GetPandaMethod()->GetNativePointer() != nullptr;
    }

    bool IsIntrinsic() const
    {
        return GetPandaMethod()->IsIntrinsic();
    }

    bool IsConstructor() const
    {
        return GetPandaMethod()->IsConstructor();
    }

    bool IsInstanceConstructor() const
    {
        return GetPandaMethod()->IsInstanceConstructor();
    }

    bool IsAbstract() const
    {
        return GetPandaMethod()->IsAbstract();
    }

    bool IsDeclaredIn(const EtsClass *klass) const
    {
        return GetClass() == klass;
    }

    bool IsGetter()
    {
        auto name = GetNameString();
        return name->GetMutf8().rfind(GETTER_BEGIN, 0) == 0;
    }

    bool IsSetter()
    {
        auto name = GetNameString();
        return name->GetMutf8().rfind(SETTER_BEGIN, 0) == 0;
    }

    void RegisterNative(const void *ptr)
    {
        ASSERT(!IsIntrinsic());
        ASSERT(IsNative());
        ASSERT(!IsBoundNativeFunction());

        Method *m = GetPandaMethod();

        m->SetNativePointer(const_cast<void *>(ptr));
    }

    uint32_t GetAccessFlags() const
    {
        return GetPandaMethod()->GetAccessFlags();
    }

    AccessLevel GetAccessLevel()
    {
        if (GetPandaMethod()->IsPublic()) {
            return AccessLevel::PUBLIC;
        }
        if (GetPandaMethod()->IsProtected()) {
            return AccessLevel::PROTECTED;
        }
        if (GetPandaMethod()->IsPrivate()) {
            return AccessLevel::PRIVATE;
        }
        return AccessLevel::DEFAULT;
    }

    uint32_t GetMethodId() const
    {
        return GetPandaMethod()->GetFileId().GetOffset();
    }

    int32_t GetLineNumFromBytecodeOffset(uint32_t bcOffset) const
    {
        if (IsNative()) {
            return -2;  // -2
        }

        if (IsAbstract()) {
            return -1;
        }

        return GetPandaMethod()->GetLineNumFromBytecodeOffset(bcOffset);
    }

    panda_file::File::StringData GetClassSourceFile() const
    {
        return GetPandaMethod()->GetClassSourceFile();
    }

    static const EtsMethod *FromRuntimeMethod(const Method *method)
    {
        return reinterpret_cast<const EtsMethod *>(method);
    }

    static EtsMethod *FromRuntimeMethod(Method *method)
    {
        return reinterpret_cast<EtsMethod *>(method);
    }

    static Method *ToRuntimeMethod(EtsMethod *etsMethod)
    {
        return reinterpret_cast<Method *>(etsMethod);
    }

    EtsMethod() = delete;
    ~EtsMethod() = delete;

    const Method *GetPandaMethod() const
    {
        return reinterpret_cast<const Method *>(this);
    }

    Method *GetPandaMethod()
    {
        return reinterpret_cast<Method *>(this);
    }

    std::string_view GetReturnTypeDescriptor()
    {
        return GetPandaMethod()->GetProto().GetReturnTypeDescriptor();
    }

    PANDA_PUBLIC_API PandaString GetMethodSignature(bool includeReturnType = true) const;

    PANDA_PUBLIC_API PandaString GetDescriptor() const;

    NO_COPY_SEMANTIC(EtsMethod);
    NO_MOVE_SEMANTIC(EtsMethod);
};

}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_METHOD_H_
