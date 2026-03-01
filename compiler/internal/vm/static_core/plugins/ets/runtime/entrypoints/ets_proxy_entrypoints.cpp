/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "runtime/arch/helpers.h"
#include "libarkfile/include/type.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

// Explicitly instantiation for arm.
template class ark::ets::EtsBoxPrimitive<double>;
template class ark::ets::EtsBoxPrimitive<long>;
template class ark::ets::EtsBoxPrimitive<unsigned short>;
template class ark::ets::EtsBoxPrimitive<unsigned char>;
template class ark::ets::EtsBoxPrimitive<float>;
template class ark::ets::EtsBoxPrimitive<short>;
template class ark::ets::EtsBoxPrimitive<signed char>;
template class ark::ets::EtsBoxPrimitive<int>;

namespace ark::ets::entrypoints {

extern "C" void EtsProxyEntryPoint();

const void *GetEtsProxyEntryPoint()
{
    return reinterpret_cast<const void *>(EtsProxyEntryPoint);
}

#if defined(__clang__)
#pragma clang diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

class BoxerBase {
public:
    virtual std::optional<EtsHandle<EtsObject>> BoxIfNeededAndCreateHandle() = 0;
    virtual ~BoxerBase() = default;
};

template <typename T>
class Boxer final : public BoxerBase {
public:
    explicit Boxer(T other, EtsCoroutine *coroutine) : coroutine_(coroutine)
    {
        static_assert(!std::is_same_v<std::remove_cv_t<std::remove_reference_t<T>>, ObjectHeader **>);
        value_ = other;
    }

    std::optional<EtsHandle<EtsObject>> BoxIfNeededAndCreateHandle() override
    {
        auto *boxPrimitive = EtsBoxPrimitive<T>::Create(coroutine_, value_);
        if (UNLIKELY(boxPrimitive == nullptr)) {
            ASSERT(coroutine_->HasPendingException());
            return std::nullopt;
        }
        EtsHandle<EtsObject> handle(coroutine_, boxPrimitive->AsObject());
        return handle;
    }

private:
    EtsCoroutine *coroutine_ {nullptr};

    T value_;
};

template <>
class Boxer<ObjectHeader **> final : public BoxerBase {
public:
    explicit Boxer(ObjectHeader **other, EtsCoroutine *coroutine)
    {
        value_ = EtsHandle<EtsObject>(coroutine, EtsObject::FromCoreType(*other));
    }

    std::optional<EtsHandle<EtsObject>> BoxIfNeededAndCreateHandle() override
    {
        return std::move(value_);
    }

private:
    EtsHandle<EtsObject> value_;
};

class BoxValueWriter final {
public:
    explicit BoxValueWriter(PandaVector<PandaUniquePtr<BoxerBase>> *values, EtsCoroutine *coroutine)
        : coroutine_(coroutine), allocator_(Runtime::GetCurrent()->GetInternalAllocator()), values_(values)
    {
    }
    ~BoxValueWriter() = default;

    template <class T>
    ALWAYS_INLINE void Write(T v)
    {
        PandaUniquePtr<BoxerBase> obj(allocator_->New<Boxer<T>>(v, coroutine_));
        values_->push_back(std::move(obj));
    }

    NO_COPY_SEMANTIC(BoxValueWriter);
    NO_MOVE_SEMANTIC(BoxValueWriter);

private:
    EtsCoroutine *coroutine_ {nullptr};
    mem::InternalAllocatorPtr allocator_;
    PandaVector<PandaUniquePtr<BoxerBase>> *values_ {nullptr};
};

static bool UnboxValue(EtsObject *boxedValue, EtsType type, Value *unboxedValue, EtsCoroutine *coroutine)
{
    ASSERT(unboxedValue != nullptr);
    ASSERT(coroutine != nullptr);

    auto *platformTypes = PlatformTypes(coroutine);

// CC-OFFNXT(G.PRE.02-CPP) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPE_CASE(boxedObj, T, resultPtr, boxedClass)                              \
    {                                                                              \
        /* CC-OFFNXT(G.PRE.02) codegen */                                          \
        if (LIKELY(boxedObj != nullptr && (boxedObj->GetClass() == boxedClass))) { \
            /* CC-OFFNXT(G.PRE.02) codegen */                                      \
            *resultPtr = Value(EtsBoxPrimitive<T>::Unbox(boxedObj));               \
            return true; /* CC-OFF(G.PRE.05) codegen */                            \
        }                                                                          \
        break; /* CC-OFF(G.PRE.05) codegen */                                      \
    }

    switch (type) {
        case EtsType::BOOLEAN:
            TYPE_CASE(boxedValue, EtsBoolean, unboxedValue, platformTypes->coreBoolean)
        case EtsType::BYTE:
            TYPE_CASE(boxedValue, EtsByte, unboxedValue, platformTypes->coreByte)
        case EtsType::CHAR:
            TYPE_CASE(boxedValue, EtsChar, unboxedValue, platformTypes->coreChar)
        case EtsType::SHORT:
            TYPE_CASE(boxedValue, EtsShort, unboxedValue, platformTypes->coreShort)
        case EtsType::INT:
            TYPE_CASE(boxedValue, EtsInt, unboxedValue, platformTypes->coreInt)
        case EtsType::LONG:
            TYPE_CASE(boxedValue, EtsLong, unboxedValue, platformTypes->coreLong)
        case EtsType::FLOAT:
            TYPE_CASE(boxedValue, EtsFloat, unboxedValue, platformTypes->coreFloat)
        case EtsType::DOUBLE:
            TYPE_CASE(boxedValue, EtsDouble, unboxedValue, platformTypes->coreDouble)
        case EtsType::OBJECT: {
            if (boxedValue == nullptr) {
                *unboxedValue = Value(nullptr);
            } else {
                *unboxedValue = Value(boxedValue->GetCoreType());
            }
            return true;
        }
        case EtsType::UNKNOWN:
            break;
        default:
            LOG(FATAL, ETS) << "Unexpected argument type";
    }
    return false;
}

static int64_t UnboxResult(EtsCoroutine *coroutine, EtsMethod *ifaceMethod, Value value)
{
    if (UNLIKELY(coroutine->HasPendingException())) {
        return 0;
    }

    auto *boxedResult = value.GetAs<ObjectHeader *>();
    auto effectiveReturnType = ifaceMethod->GetEffectiveReturnValueType();
    if (effectiveReturnType == EtsType::VOID) {
        return 0;
    }

    Value unboxedResult;
    if (!UnboxValue(EtsObject::FromCoreType(boxedResult), effectiveReturnType, &unboxedResult, coroutine)) {
        PandaOStringStream msg;
        msg << "result has type " << EtsTypeToString(effectiveReturnType) << ", but got "
            << EtsObject::FromCoreType(boxedResult)->GetClass()->GetName()->GetMutf8();
        ark::ets::ThrowEtsException(coroutine, panda_file_items::class_descriptors::ILLEGAL_ARGUMENT_ERROR,
                                    msg.str().c_str());
    }
    return unboxedResult.GetAsLong();
}

Value PrepareArgumentsAndInvoke(const EtsHandle<EtsObject> &thisH, EtsMethod *ifaceMethod,
                                const PandaVector<EtsHandle<EtsObject>> &handledArgs, EtsCoroutine *coroutine)
{
    auto *platformTypes = PlatformTypes(coroutine);

    EtsHandle<EtsReflectMethod> reflectMethodH(coroutine,
                                               EtsReflectMethod::CreateFromEtsMethod(coroutine, ifaceMethod));
    if (UNLIKELY(reflectMethodH.GetPtr() == nullptr)) {
        ASSERT(coroutine->HasPendingException());
        return Value(0U);
    }

    PandaVector<Value> invokeArgs;
    invokeArgs.reserve(3U);

    EtsMethod *methodToInvoke = nullptr;

    if (ifaceMethod->IsSetter()) {
        ASSERT(handledArgs.size() == 1U);
        invokeArgs.push_back(Value(thisH->GetCoreType()));
        invokeArgs.push_back(Value(reflectMethodH->AsObject()->GetCoreType()));
        // nullptr in case of arg = `undefined`.
        Value arg = (handledArgs[0].GetPtr() == nullptr) ? Value(nullptr) : Value(handledArgs[0]->GetCoreType());
        invokeArgs.push_back(std::move(arg));
        methodToInvoke = platformTypes->coreReflectProxyInvokeSet;
    } else if (ifaceMethod->IsGetter()) {
        ASSERT(handledArgs.empty());
        invokeArgs.push_back(Value(thisH->GetCoreType()));
        invokeArgs.push_back(Value(reflectMethodH->AsObject()->GetCoreType()));
        methodToInvoke = platformTypes->coreReflectProxyInvokeGet;
    } else {
        auto *argsArrayRaw = EtsObjectArray::Create(
            coroutine->GetPandaVM()->GetClassLinker()->GetClassRoot(EtsClassRoot::OBJECT), handledArgs.size());
        if (UNLIKELY(argsArrayRaw == nullptr)) {
            ASSERT(coroutine->HasPendingException());
            return Value(0U);
        }

        for (size_t idx = 0; idx < handledArgs.size(); ++idx) {
            argsArrayRaw->Set(idx, handledArgs[idx].GetPtr());
        }
        invokeArgs.push_back(Value(thisH->GetCoreType()));
        invokeArgs.push_back(Value(reflectMethodH->AsObject()->GetCoreType()));
        invokeArgs.push_back(Value(argsArrayRaw->AsObject()->GetCoreType()));
        methodToInvoke = platformTypes->coreReflectProxyInvoke;
    }

    return methodToInvoke->GetPandaMethod()->Invoke(coroutine, invokeArgs.data(), true);
}

/*
    In case of i2c the following flow take place:
    interpreter --> HandleCall --> i2c bridge --> call entrypoint(proxy bridge) --> call EtsProxyMethodInvoke

    When some method genereted by proxy called with N arguments, then runtime calls i2c bridge
    and some of arguments saved in caller-saved registers and some on the stack according to the target calling
    convention. Then i2c bridge calls proxy bridge (that was set as entrypoint to the Method *).
    Proxy bridge pushes on the stack all incoming argument registers with purpose of passing
    it as array `args` to the EtsProxyMethodInvoke.
    `inStackArgs` is array of arguments from the stack that was formed by i2c bridge.

    So, proxy bridge does 2 main things:
    1) maps signature of given managed interface function to the signature of `EtsProxyMethodInvoke`.
    2) forms arguments to have opportunity of passing N arguments as one array argument.

    // CC-OFFNXT(G.CMT.04-CPP) false positive
0x0000                               AMD64 STACK:

    |            |                top of the stack                  |
  a |            |                                                  |
  d |            ---------------------------------------------------- <-- call of EtsProxyMethodInvoke(%rdi, %rsi, %rdx)
  d |            | Arguments that pushed in registers by i2c (%rsi) |
  r |            |               GPR arguments                      |
  e |            |               FPR arguments                      |
  s |            ---------------------------------------------------- <-- call of proxy bridge
  s |            |               return address                     |
  e |            ----------------------------------------------------
  s |            |               in stack arguments (%rdx)          |
    V            ----------------------------------------------------
                 |               Method* (%rdi)                     |
                 ----------------------------------------------------
                 | ................................................ |
                                                                      <-- call of i2c bridge by HandleCall
0xFFFF
*/
extern "C" int64_t EtsProxyMethodInvoke(Method *method, uint8_t *args, uint8_t *inStackArgs)
{
    ASSERT(method != nullptr);
    ASSERT(args != nullptr);
    ASSERT(inStackArgs != nullptr);

    ASSERT_MANAGED_CODE();

    // Proxy instance override some interface method, handler.invoke should accept interface method.
    auto *ifaceMethod = EtsMethod::FromRuntimeMethod(method)->GetInterfaceMethodIfProxy();
    ASSERT(ifaceMethod != nullptr);

    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);

    Span<uint8_t> gprArgs(args, arch::ExtArchTraits<RUNTIME_ARCH>::GP_ARG_NUM_BYTES);
    Span<uint8_t> fprArgs(gprArgs.end(), arch::ExtArchTraits<RUNTIME_ARCH>::FP_ARG_NUM_BYTES);
    arch::ArgReader<RUNTIME_ARCH> argReader(gprArgs, fprArgs, inStackArgs);

    argReader.Read<Method *>();  // Method * also pushed in GPR args, skip.

    auto *thisObj = EtsObject::FromCoreType(argReader.Read<ObjectHeader *>());
    [[maybe_unused]] EtsHandleScope scope(coroutine);

    EtsHandle<EtsObject> thisH(coroutine, thisObj);

    // Collect values from stack and create handles for reference types. It should be done before any allocation.
    PandaVector<PandaUniquePtr<BoxerBase>> values;
    BoxValueWriter writer(&values, coroutine);

    ARCH_COPY_METHOD_ARGS(ifaceMethod->GetPandaMethod(), argReader, writer);

    PandaVector<EtsHandle<EtsObject>> handledArgs;
    handledArgs.reserve(values.size());

    for (auto &entry : values) {
        auto obj = entry->BoxIfNeededAndCreateHandle();
        if (UNLIKELY(!obj.has_value())) {
            ASSERT(coroutine->HasPendingException());
            return 0;
        }
        handledArgs.push_back(std::move(*obj));
    }

    // After all handles are created we can do allocations.
    Value result = PrepareArgumentsAndInvoke(thisH, ifaceMethod, handledArgs, coroutine);
    // result is boxed Any value.
    return UnboxResult(coroutine, ifaceMethod, result);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

}  // namespace ark::ets::entrypoints
