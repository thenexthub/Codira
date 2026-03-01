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

#include "plugins/ets/runtime/ani/verify/verify_ani_checker.h"

#include "plugins/ets/runtime/ani/ani_converters.h"
#include "plugins/ets/runtime/ani/ani_interaction_api.h"
#include "plugins/ets/runtime/ani/verify/types/venv.h"
#include "plugins/ets/runtime/ani/verify/types/vvm.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

namespace ark::ets::ani::verify {

class ANIRefTypeChecker {
public:
    static bool IsClass(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        auto klass = s.ToInternalType(ref)->GetClass();
        return klass->IsClassClass();
    }

    static bool IsError(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *errorKlass = PlatformTypes()->escompatError;

        return errorKlass->IsAssignableFrom(klass);
    }

    static bool IsString(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (ManagedCodeAccessor::IsUndefined(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *stringKlass = PlatformTypes()->coreString;

        return stringKlass->IsAssignableFrom(klass);
    }
    static bool IsObject(ScopedManagedCodeFix &s, ani_ref ref)
    {
        if (s.IsNullishValue(ref)) {
            return false;
        }
        EtsClass *klass = s.ToInternalType(ref)->GetClass();
        EtsClass *objectKlass = PlatformTypes()->coreObject;

        return objectKlass->IsAssignableFrom(klass);
    }
};

class CallArgs {
public:
    explicit CallArgs(const EtsMethod *etsMethod, const ani_value *args)
        : method_(etsMethod->GetPandaMethod()), args_(args)
    {
    }

    size_t GetNrArgs() const
    {
        return method_->GetNumArgs();
    }

    template <typename Callback>
    void ForEachArgs(Callback callback) const
    {
        static_assert(std::is_invocable_r_v<bool, Callback, ani_value, panda_file::Type, size_t>,
                      "wrong callback signature");

        panda_file::ShortyIterator it(method_->GetShorty());
        panda_file::ShortyIterator end;
        ++it;  // skip the return value

        size_t i = 0;
        size_t refArgNumber = 0;
        if (!method_->IsStatic()) {
            ++refArgNumber;
        }
        for (; it != end; ++it) {
            panda_file::Type type = *it;
            if (!callback(args_[i++], type, refArgNumber)) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                return;
            }
            if (type.IsReference()) {
                ++refArgNumber;
            }
        }
    }

private:
    const Method *method_ {};
    const ani_value *args_ {};
};

// Сheckers must be consistent with the type hierarchy
// ani_ref
//    |
//    +-- (ani_undefined)
//    +-- (ani_null)
//    +-- ani_object
//          |
//          +-- ani_fn_object
//          +-- ani_enum_item
//          +-- ani_error
//          +-- ani_arraybuffer
//          +-- ani_string
//          +-- ani_tuple_value
//          +-- ani_array
//          +-- ani_fixedarray
//          |    |
//          |    +-- ani_fixedarray_boolean
//          |    +-- ani_fixedarray_char
//          |    +-- ani_fixedarray_byte
//          |    +-- ani_fixedarray_short
//          |    +-- ani_fixedarray_int
//          |    +-- ani_fixedarray_long
//          |    +-- ani_fixedarray_float
//          |    +-- ani_fixedarray_double
//          |    +-- ani_fixedarray_ref
//          +-- ani_type
//              |
//              +-- ani_class
//              +-- ani_enum
//              +-- ani_namespace
//              +-- (ani_module)
std::string_view ANIRefTypeToString(ScopedManagedCodeFix &s, ani_ref ref)
{
    if (ManagedCodeAccessor::IsUndefined(ref)) {
        return "undefined";
    }
    if (s.IsNull(ref)) {
        return "null";
    }
    if (ANIRefTypeChecker::IsObject(s, ref)) {
        if (ANIRefTypeChecker::IsError(s, ref)) {
            return "ani_error";
        }
        if (ANIRefTypeChecker::IsString(s, ref)) {
            return "ani_string";
        }
        if (ANIRefTypeChecker::IsClass(s, ref)) {
            return "ani_class";
        }
        return "ani_object";
    }
    return "ani_ref";
}

std::string_view ANIFuncTypeToString(impl::VMethod::ANIMethodType funcType)
{
    // clang-format off
    switch (funcType) {
        case impl::VMethod::ANIMethodType::FUNCTION:      return "ani_function";
        case impl::VMethod::ANIMethodType::METHOD:        return "ani_method";
        case impl::VMethod::ANIMethodType::STATIC_METHOD: return "ani_static_method";
    }
    // clang-format on
    UNREACHABLE();
}

std::string_view ANIFieldTypeToString(impl::VField::ANIFieldType fieldType)
{
    // clang-format off
    switch (fieldType) {
        case impl::VField::ANIFieldType::FIELD:        return "ani_field";
        case impl::VField::ANIFieldType::STATIC_FIELD: return "ani_static_field";
        case impl::VField::ANIFieldType::VARIABLE:     return "ani_variable";
    }
    // clang-format on
    UNREACHABLE();
}

template <class T>
void FormatPointer(PandaStringStream &ss, T *ptr)
{
    if (ptr == nullptr) {
        ss << "NULL";
    } else {
        ss << ptr;
    }
}

// CC-OFFNXT(G.FUD.05) solid logic
PandaString ANIArg::GetStringValue() const
{
    PandaStringStream ss;
    switch (type_) {
        case ValueType::ANI_SIZE:
            ss << GetValueSize();
            break;
        case ValueType::UINT32:
            ss << "0x" << std::setfill('0') << std::setw(2U * sizeof(uint32_t)) << std::hex << GetValueU32();
            break;
        case ValueType::METHOD_ARGS:
            if (action_ == Action::VERIFY_METHOD_A_ARGS) {
                const ani_value *args = GetValueMethodArgs()->vargs;
                FormatPointer(ss, args);
            } else if (action_ == Action::VERIFY_METHOD_V_ARGS) {
                // Do nothing
            } else {
                UNREACHABLE();
            }
            break;
        default:
            FormatPointer(ss, GetValueRawPointer());
            break;
    }
    return ss.str();
}

// CC-OFFNXT(G.FUD.05) solid logic
PandaString ANIArg::GetStringType() const
{
    // clang-format off
    switch (type_) {
        case ValueType::ANI_ENV:                          return "ani_env *";
        case ValueType::ANI_VM:                           return "ani_vm *";
        case ValueType::ANI_OPTIONS:                      return "ani_options *";
        case ValueType::ANI_SIZE:                         return "ani_size";
        case ValueType::ANI_REF:                          return "ani_ref";
        case ValueType::ANI_CLASS:                        return "ani_class";
        case ValueType::ANI_METHOD:                       return "ani_method";
        case ValueType::ANI_STATIC_METHOD:                return "ani_static_method";
        case ValueType::ANI_FUNCTION:                     return "ani_function";
        case ValueType::ANI_FIELD:                        return "ani_field";
        case ValueType::ANI_OBJECT:                       return "ani_object";
        case ValueType::ANI_STRING:                       return "ani_string";
        case ValueType::ANI_VALUE_ARGS:                   return "const ani_value *";
        case ValueType::ANI_UTF8_BUFFER:                  return "char *";
        case ValueType::ANI_UTF8_STRING:                  return "const char *";
        case ValueType::ANI_UTF16_BUFFER:                 return "uint16_t *";
        case ValueType::ANI_UTF16_STRING:                 return "const uint16_t *";
        case ValueType::ANI_ENV_STORAGE:                  return "ani_env **";
        case ValueType::ANI_VM_STORAGE:                   return "ani_env **";
        case ValueType::ANI_BOOLEAN_STORAGE:              return "ani_boolean *";
        case ValueType::ANI_CHAR_STORAGE:                 return "ani_char *";
        case ValueType::ANI_BYTE_STORAGE:                 return "ani_byte *";
        case ValueType::ANI_SHORT_STORAGE:                return "ani_short *";
        case ValueType::ANI_INT_STORAGE:                  return "ani_int *";
        case ValueType::ANI_LONG_STORAGE:                 return "ani_long *";
        case ValueType::ANI_FLOAT_STORAGE:                return "ani_float *";
        case ValueType::ANI_DOUBLE_STORAGE:               return "ani_double *";
        case ValueType::ANI_REF_STORAGE:                  return "ani_ref *";
        case ValueType::ANI_OBJECT_STORAGE:               return "ani_object *";
        case ValueType::ANI_STRING_STORAGE:               return "ani_string *";
        case ValueType::ANI_SIZE_STORAGE:                 return "ani_size *";
        case ValueType::ANI_BOOLEAN:                      return "ani_boolean";
        case ValueType::UINT32:                           return "uint32_t";
        case ValueType::ANI_ERROR:                        return "ani_error";
        case ValueType::ANI_ERROR_STORAGE:                return "ani_error *";
        case ValueType::ANI_FIXED_ARRAY_BOOLEAN_STORAGE:  return "ani_fixedarray_boolean *";
        case ValueType::ANI_FIXED_ARRAY_CHAR_STORAGE:     return "ani_fixedarray_char *";
        case ValueType::ANI_FIXED_ARRAY_BYTE_STORAGE:     return "ani_fixedarray_byte *";
        case ValueType::ANI_FIXED_ARRAY_SHORT_STORAGE:    return "ani_fixedarray_short *";
        case ValueType::ANI_FIXED_ARRAY_INT_STORAGE:      return "ani_fixedarray_int *";
        case ValueType::ANI_FIXED_ARRAY_LONG_STORAGE:     return "ani_fixedarray_long *";
        case ValueType::ANI_FIXED_ARRAY_FLOAT_STORAGE:    return "ani_fixedarray_float *";
        case ValueType::ANI_FIXED_ARRAY_DOUBLE_STORAGE:   return "ani_fixedarray_double *";
        case ValueType::ANI_RESOLVER:                     return "ani_resolver";
        case ValueType::ANI_RESOLVER_STORAGE:             return "ani_resolver *";
        default:                                          UNREACHABLE(); return "";
        case ValueType::METHOD_ARGS:
            if (action_ == Action::VERIFY_METHOD_A_ARGS) {
                return "ani_value *";
            } else if (action_ == Action::VERIFY_METHOD_V_ARGS) {
                return "";
            } else {
                UNREACHABLE();
            }
            break;
    }
    // clang-format on
    UNREACHABLE();
}

static bool IsValidRawAniValue(EnvANIVerifier *envANIVerifier, ani_value v, panda_file::Type type, bool isVaArgs)
{
    panda_file::Type::TypeId typeId = type.GetId();

    if (isVaArgs) {
        constexpr ani_long BOOLEAN_MASK = ~0x1ULL;
        constexpr ani_long CHAR_MASK = ~0xffffULL;
        using I8Limits = std::numeric_limits<int8_t>;
        using I16Limits = std::numeric_limits<int16_t>;
        using I32Limits = std::numeric_limits<int32_t>;

        switch (typeId) {
            // clang-format off
            case panda_file::Type::TypeId::U1:  return 0 == (v.l & BOOLEAN_MASK);  // NOLINT(hicpp-signed-bitwise)
            case panda_file::Type::TypeId::U16: return 0 == (v.l & CHAR_MASK);     // NOLINT(hicpp-signed-bitwise)
            case panda_file::Type::TypeId::I8:  return v.i >= I8Limits::min() && v.i <= I8Limits::max();
            case panda_file::Type::TypeId::I16: return v.i >= I16Limits::min() && v.i <= I16Limits::max();
            case panda_file::Type::TypeId::I32: return v.i >= I32Limits::min() && v.i <= I32Limits::max();
            case panda_file::Type::TypeId::I64: return true;
            case panda_file::Type::TypeId::F32: return true;  // uses double
            case panda_file::Type::TypeId::F64: return true;
            case panda_file::Type::TypeId::REFERENCE:
                return envANIVerifier->IsValidRefInCurrentFrame(reinterpret_cast<VRef *>(v.r));
            // clang-format on
            default:
                break;
        }
    } else {
        switch (typeId) {
            // clang-format off
            case panda_file::Type::TypeId::U1:  return v.b == ANI_FALSE || v.b == ANI_TRUE;
            case panda_file::Type::TypeId::U16: return true;
            case panda_file::Type::TypeId::I8:  return true;
            case panda_file::Type::TypeId::I16: return true;
            case panda_file::Type::TypeId::I32: return true;
            case panda_file::Type::TypeId::I64: return true;
            case panda_file::Type::TypeId::F32: return true;
            case panda_file::Type::TypeId::F64: return true;
            case panda_file::Type::TypeId::REFERENCE:
                return envANIVerifier->IsValidRefInCurrentFrame(reinterpret_cast<VRef *>(v.r));
            // clang-format on
            default:
                break;
        }
    }

    LOG(FATAL, ANI) << "Unexpected argument type: " << static_cast<int>(typeId);
    return false;
}

static bool IsValidMethodArgRefType(ani_env *env, EtsMethod *method, VRef *ref, size_t refArgNumber)
{
    auto realRef = ref->GetRef();
    if (ManagedCodeAccessor::IsUndefined(realRef)) {
        return true;
    }

    ASSERT(env != nullptr);
    ScopedManagedCodeFix s(env);

    const char *methodArgDesc = method->GetRefArgType(refArgNumber);
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto ctx = method->GetClass()->GetLoadContext();
    Class *methodKlass = classLinker->FindLoadedClass(utf::CStringAsMutf8(methodArgDesc), ctx);

    // NOTE(ypigunova): Add check for union types #31219
    if (methodKlass == nullptr) {
        return true;
    }

    Class *inputKlass = s.ToInternalType(realRef)->GetClass()->GetRuntimeClass();
    return methodKlass->IsAssignableFrom(inputKlass);
}

class Verifier {
public:
    explicit Verifier(VVm *vvm, VEnv *venv) : vvm_(vvm), venv_(venv) {}

    std::optional<PandaString> VerifyVm(VVm *vvm)
    {
        if (UNLIKELY(vvm != vvm_)) {
            return "wrong VM pointer";
        }
        return {};
    }

    std::optional<PandaString> VerifyEnv(VEnv *venv, bool checkPendingError)
    {
        if (UNLIKELY(venv_ == nullptr)) {
            return "current native thread is not attached";
        }
        if (UNLIKELY(venv != venv_)) {
            return "called from incorrect the native scope";
        }
        PandaEnv *pandaEnv = PandaEnv::FromAniEnv(venv_->GetEnv());
        ASSERT(pandaEnv == EtsCoroutine::GetCurrent()->GetEtsNapiEnv());
        if (UNLIKELY(checkPendingError && pandaEnv->HasPendingException())) {
            return "has unhandled an error";
        }
        return {};
    }

    std::optional<PandaString> VerifyOptions(const ani_options *options)
    {
        if (options == nullptr) {
            return {};
        }
        if (options->options == nullptr) {
            return "wrong 'options' pointer, options->options == NULL";
        }
        const size_t maxNrOptions = 4096;
        if (options->nr_options > maxNrOptions) {
            PandaStringStream ss;
            ss << "'nr_options' value is too large. options->nr_options == " << options->nr_options;
            return ss.str();
        }

        for (size_t i = 0; i < options->nr_options; ++i) {
            const ani_option &opt = options->options[i];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (opt.option == nullptr) {
                PandaStringStream ss;
                ss << "wrong 'option' pointer, options->options[" << i << "].option == NULL";
                return ss.str();
            }
            constexpr std::string_view EXT_PREFIX = "--ext:";
            std::string_view name = opt.option;
            if (name.substr(0, EXT_PREFIX.size()) == EXT_PREFIX) {
                std::string_view subName = name.substr(EXT_PREFIX.size());
                if (subName.empty() || !(isascii(subName[0]) != 0 && std::isalpha(subName[0]) != 0)) {
                    PandaStringStream ss;
                    ss << "wrong 'option' value, options->options[" << i << "].option == " << name;
                    return ss.str();
                }
            }
        }
        return {};
    }

    template <typename Type>
    std::optional<PandaString> VerifyTypeStorage(Type value, std::string_view typeName)
    {
        if (value == nullptr) {
            PandaStringStream ss;
            ss << "wrong pointer for storing '" << typeName << "'";
            return ss.str();
        }
        return {};
    }

    std::optional<PandaString> VerifyBoolean(ani_boolean value)
    {
        if (value != ANI_TRUE && value != ANI_FALSE) {
            return "wrong value for ani_boolean";
        }
        return {};
    }

    template <typename Type>
    std::optional<PandaString> VerifyTypePtr(Type value, std::string_view typeName)
    {
        if (value == nullptr) {
            PandaStringStream ss;
            ss << "wrong pointer to use as argument in '" << typeName << "'";
            return ss.str();
        }
        return {};
    }

    std::optional<PandaString> VerifyEnvVersion(uint32_t version)
    {
        if (!IsVersionSupported(version)) {
            return "unsupported ANI version";
        }
        return {};
    }

    std::optional<PandaString> VerifyNrRefs(ani_size nrRefs)
    {
        if (nrRefs == 0) {
            return "wrong value";
        }
        if (nrRefs > std::numeric_limits<uint16_t>::max()) {
            PandaStringStream ss;
            ss << "it is too big";
            return ss.str();
        }
        return {};
    }

    std::optional<PandaString> VerifyRef(VRef *vref)
    {
        if (GetEnvANIVerifier()->IsValidRefInCurrentFrame(vref)) {
            return {};
        }
        if (GetEnvANIVerifier()->IsValidGlobalVerifiedRef(vref)) {
            return {};
        }
        return "wrong reference";
    }

    std::optional<PandaString> VerifyClass(VClass *vclass)
    {
        auto err = VerifyRef(vclass);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsClass(s, vclass->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vclass->GetRef());
            return ss.str();
        }

        class_ = s.ToInternalType(vclass->GetRef());
        return {};
    }

    std::optional<PandaString> VerifyString(VString *vstr)
    {
        auto err = VerifyRef(vstr);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsString(s, vstr->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vstr->GetRef());
            return ss.str();
        }
        return {};
    }

    std::optional<PandaString> VerifyError(VError *verr)
    {
        auto errMessage = VerifyRef(verr);
        if (errMessage) {
            return errMessage;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsError(s, verr->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, verr->GetRef());
            return ss.str();
        }
        return {};
    }

    std::optional<PandaString> VerifyDelLocalRef(VRef *vref)
    {
        EnvANIVerifier *envANIVerifier = GetEnvANIVerifier();
        if (!envANIVerifier->IsValidRefInCurrentFrame(vref)) {
            return "it is not local reference";
        }
        if (!envANIVerifier->CanBeDeletedFromCurrentScope(vref)) {
            return "a local reference can only be deleted in the scope where it was created";
        }
        return {};
    }

    std::optional<PandaString> VerifyThisObject(VObject *vobject)
    {
        auto err = VerifyRef(vobject);
        if (err) {
            return err;
        }

        ScopedManagedCodeFix s(venv_->GetEnv());
        if (!ANIRefTypeChecker::IsObject(s, vobject->GetRef())) {
            PandaStringStream ss;
            ss << "wrong reference type: " << ANIRefTypeToString(s, vobject->GetRef());
            return ss.str();
        }

        EtsObject *etsObject = s.ToInternalType(vobject->GetRef());
        class_ = etsObject->GetClass();
        return {};
    }

    std::optional<PandaString> DoVerifyMethod(impl::VMethod *vmethod, impl::VMethod::ANIMethodType type,
                                              EtsType returnType)
    {
        impl::VMethod::ANIMethodType methodType = vmethod->GetType();
        if (methodType != type) {
            PandaStringStream ss;
            ss << "wrong type: " << ANIFuncTypeToString(methodType) << ", expected: " << ANIFuncTypeToString(type);
            return ss.str();
        }

        EtsType methodReturnType = vmethod->GetEtsMethod()->GetReturnValueType();
        if (methodReturnType != returnType) {
            return "wrong return type";
        }
        return {};
    }

    std::optional<PandaString> VerifyCtor(VMethod *vctor, EtsType returnType)
    {
        if (!GetEnvANIVerifier()->IsValidMethod(vctor)) {
            return "wrong ctor";
        }

        std::optional<PandaString> err = DoVerifyMethod(vctor, impl::VMethod::ANIMethodType::METHOD, returnType);
        if (err) {
            return err;
        }

        if (!vctor->GetEtsMethod()->IsConstructor()) {
            return "method is not ctor";
        }

        if (vctor->GetEtsMethod()->GetClass() != class_) {
            return "wrong class for ctor";
        }
        return {};
    }

    std::optional<PandaString> VerifyMethod(VMethod *vmethod, EtsType returnType)
    {
        if (!GetEnvANIVerifier()->IsValidMethod(vmethod)) {
            return "wrong method";
        }

        std::optional<PandaString> err = DoVerifyMethod(vmethod, impl::VMethod::ANIMethodType::METHOD, returnType);
        if (err) {
            return err;
        }

        if (vmethod->GetEtsMethod()->IsConstructor()) {
            return "method is ctor";
        }

        if (class_ == nullptr || !vmethod->GetEtsMethod()->GetClass()->IsAssignableFrom(class_)) {
            return "wrong object for method";
        }
        return {};
    }

    std::optional<PandaString> VerifyStaticMethod(VStaticMethod *vstaticmethod, EtsType returnType)
    {
        if (!GetEnvANIVerifier()->IsValidMethod(vstaticmethod)) {
            return "wrong static method";
        }
        std::optional<PandaString> err =
            DoVerifyMethod(vstaticmethod, impl::VMethod::ANIMethodType::STATIC_METHOD, returnType);
        if (err) {
            return err;
        }

        if (class_ == nullptr || !vstaticmethod->GetEtsMethod()->GetClass()->IsAssignableFrom(class_)) {
            return "wrong class for method";
        }
        return {};
    }

    std::optional<PandaString> VerifyFunction(VFunction *vfunction, EtsType returnType)
    {
        if (!GetEnvANIVerifier()->IsValidMethod(vfunction)) {
            return "wrong function";
        }
        std::optional<PandaString> err = DoVerifyMethod(vfunction, impl::VMethod::ANIMethodType::FUNCTION, returnType);
        if (err) {
            return err;
        }

        return {};
    }

    std::optional<PandaString> DoVerifyField(impl::VField *vfield, impl::VField::ANIFieldType type, EtsType returnType)
    {
        impl::VField::ANIFieldType fieldType = vfield->GetType();
        if (fieldType != type) {
            PandaStringStream ss;
            ss << "wrong type: " << ANIFieldTypeToString(fieldType) << ", expected: " << ANIFieldTypeToString(type);
            return ss.str();
        }

        EtsType fieldReturnType = vfield->GetEtsField()->GetEtsType();
        if (fieldReturnType != returnType) {
            return "wrong return type";
        }
        return {};
    }

    std::optional<PandaString> VerifyField(VField *vfield, EtsType returnType)
    {
        if (!GetEnvANIVerifier()->IsValidField(vfield)) {
            return "wrong field";
        }
        std::optional<PandaString> err = DoVerifyField(vfield, impl::VField::ANIFieldType::FIELD, returnType);
        if (err) {
            return err;
        }

        if (class_ == nullptr || !vfield->GetEtsField()->GetDeclaringClass()->IsAssignableFrom(class_)) {
            return "wrong object for field";
        }
        return {};
    }

    std::optional<PandaString> VerifyMethodAArgs(ANIArg::AniMethodArgs *methodArgs)
    {
        ASSERT(methodArgs != nullptr);
        if (methodArgs->vargs == nullptr) {
            return "wrong arguments value";
        }
        return DoVerifyMethodArgs(methodArgs);
    }

    std::optional<PandaString> VerifyMethodVArgs(ANIArg::AniMethodArgs *methodArgs)
    {
        ASSERT(methodArgs != nullptr);
        return DoVerifyMethodArgs(methodArgs);
    }

    std::optional<PandaString> DoVerifyMethodArgs(ANIArg::AniMethodArgs *methodArgs)
    {
        if (methodArgs->method == nullptr) {
            return "wrong method";
        }

        CallArgs callArgs(methodArgs->method, methodArgs->vargs);
        EnvANIVerifier *envANIVerifier = GetEnvANIVerifier();
        std::optional<PandaString> err;

        callArgs.ForEachArgs([&](ani_value value, panda_file::Type type, size_t refIndex) -> bool {
            if (UNLIKELY(!IsValidRawAniValue(envANIVerifier, value, type, methodArgs->isVaArgs))) {
                err = "wrong method arguments";
                return false;
            }
            if (type.IsReference()) {
                if (UNLIKELY(!IsValidMethodArgRefType(venv_->GetEnv(), methodArgs->method,
                                                      reinterpret_cast<VRef *>(value.r), refIndex))) {
                    err = "wrong method arguments";
                    return false;
                }
            }
            return true;
        });
        return err;
    }

    std::optional<PandaString> VerifyResolver(VResolver *vresolver)
    {
        if (!GetEnvANIVerifier()->IsValidGlobalVerifiedResolver(vresolver)) {
            return "wrong resolver";
        }

        return {};
    }

private:
    EnvANIVerifier *GetEnvANIVerifier()
    {
        ASSERT(venv_ != nullptr);
        return PandaEnv::FromAniEnv(venv_->GetEnv())->GetEnvANIVerifier();
    }

    VVm *vvm_ {};
    VEnv *venv_ {};

    EtsClass *class_ {};
};

using CheckerHandler = std::optional<PandaString> (*)(Verifier &, const ANIArg &);

static std::optional<PandaString> VerifyVm(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_VM);
    return v.VerifyVm(arg.GetValueVm());
}

static std::optional<PandaString> VerifyEnv(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV);
    return v.VerifyEnv(arg.GetValueEnv(), true);
}

static std::optional<PandaString> VerifyEnvSkipPendingError(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV_SKIP_PENDING_ERROR);
    return v.VerifyEnv(arg.GetValueEnv(), false);
}

static std::optional<PandaString> VerifyOptions(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_OPTIONS);
    return v.VerifyOptions(arg.GetValueOptions());
}

static std::optional<PandaString> VerifyEnvVersion(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV_VERSION);
    return v.VerifyEnvVersion(arg.GetValueU32());
}

static std::optional<PandaString> VerifyNrRefs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_NR_REFS);
    return v.VerifyNrRefs(arg.GetValueSize());
}

static std::optional<PandaString> VerifyRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_REF);
    return v.VerifyRef(arg.GetValueRef());
}

static std::optional<PandaString> VerifyClass(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CLASS);
    return v.VerifyClass(arg.GetValueClass());
}

static std::optional<PandaString> VerifyString(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STRING);
    return v.VerifyString(arg.GetValueString());
}

static std::optional<PandaString> VerifyUTF8Buffer(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF8_BUFFER);
    return v.VerifyTypePtr(arg.GetValueUTF8Buffer(), "char *");
}

static std::optional<PandaString> VerifyError(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ERROR);
    return v.VerifyError(arg.GetValueError());
}

static std::optional<PandaString> VerifyUTF8String(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF8_STRING);
    return v.VerifyTypePtr(arg.GetValueUTF8String(), "const char *");
}

static std::optional<PandaString> VerifyUTF16Buffer(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF16_BUFFER);
    return v.VerifyTypePtr(arg.GetValueUTF16Buffer(), "uint16_t *");
}

static std::optional<PandaString> VerifyUTF16String(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_UTF16_STRING);
    return v.VerifyTypePtr(arg.GetValueUTF16String(), "const uint16_t *");
}

static std::optional<PandaString> VerifyDelLocalRef(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_DEL_LOCAL_REF);
    return v.VerifyDelLocalRef(arg.GetValueRef());
}

static std::optional<PandaString> VerifyThisObject(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_THIS_OBJECT);
    return v.VerifyThisObject(arg.GetValueObject());
}

static std::optional<PandaString> VerifyCtor(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CTOR);
    return v.VerifyCtor(arg.GetValueMethod(), arg.GetReturnType());
}

static std::optional<PandaString> VerifyMethod(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD);
    return v.VerifyMethod(arg.GetValueMethod(), arg.GetReturnType());
}

static std::optional<PandaString> VerifyStaticMethod(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STATIC_METHOD);
    return v.VerifyStaticMethod(arg.GetValueStaticMethod(), arg.GetReturnType());
}

static std::optional<PandaString> VerifyFunction(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FUNCTION);
    return v.VerifyFunction(arg.GetValueFunction(), arg.GetReturnType());
}

static std::optional<PandaString> VerifyField(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIELD);
    return v.VerifyField(arg.GetValueField(), arg.GetReturnType());
}

static std::optional<PandaString> VerifyMethodAArgs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD_A_ARGS);
    return v.VerifyMethodAArgs(arg.GetValueMethodArgs());
}

static std::optional<PandaString> VerifyMethodVArgs(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_METHOD_V_ARGS);
    return v.VerifyMethodVArgs(arg.GetValueMethodArgs());
}

static std::optional<PandaString> VerifyVmStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_VM_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueVmStorage(), "ani_vm *");
}

static std::optional<PandaString> VerifyEnvStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ENV_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueEnvStorage(), "ani_env *");
}

static std::optional<PandaString> VerifyBooleanStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_BOOLEAN_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueBooleanStorage(), "ani_boolean");
}

static std::optional<PandaString> VerifyCharStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_CHAR_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueCharStorage(), "ani_char");
}

static std::optional<PandaString> VerifyByteStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_BYTE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueByteStorage(), "ani_byte");
}

static std::optional<PandaString> VerifyShortStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_SHORT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueShortStorage(), "ani_short");
}

static std::optional<PandaString> VerifyIntStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_INT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueIntStorage(), "ani_int");
}

static std::optional<PandaString> VerifyLongStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_LONG_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueLongStorage(), "ani_long");
}

static std::optional<PandaString> VerifyFloatStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FLOAT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFloatStorage(), "ani_float");
}

static std::optional<PandaString> VerifyDoubleStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_DOUBLE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueDoubleStorage(), "ani_double");
}

static std::optional<PandaString> VerifyRefStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_REF_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueRefStorage(), "ani_ref");
}

static std::optional<PandaString> VerifyStringStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_STRING_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueStringStorage(), "ani_string");
}

static std::optional<PandaString> VerifySizeStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_SIZE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueSizeStorage(), "ani_size");
}

static std::optional<PandaString> VerifySize([[maybe_unused]] Verifier &v, [[maybe_unused]] const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_SIZE);
    return {};
}

static std::optional<PandaString> VerifyBoolean(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_BOOLEAN);
    return v.VerifyBoolean(arg.GetValueBoolean());
}

static std::optional<PandaString> VerifyObjectStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_OBJECT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueObjectStorage(), "ani_object");
}

static std::optional<PandaString> VerifyErrorStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_ERROR_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueErrorStorage(), "ani_error");
}

static std::optional<PandaString> VerifyFixedArrayBooleanStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_BOOLEAN_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayBooleanStorage(), "ani_fixedarray_boolean");
}

static std::optional<PandaString> VerifyFixedArrayCharStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_CHAR_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayCharStorage(), "ani_fixedarray_char");
}

static std::optional<PandaString> VerifyFixedArrayByteStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_BYTE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayByteStorage(), "ani_fixedarray_byte");
}

static std::optional<PandaString> VerifyFixedArrayShortStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_SHORT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayShortStorage(), "ani_fixedarray_short");
}

static std::optional<PandaString> VerifyFixedArrayIntStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_INT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayIntStorage(), "ani_fixedarray_int");
}

static std::optional<PandaString> VerifyFixedArrayLongStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_LONG_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayLongStorage(), "ani_fixedarray_long");
}

static std::optional<PandaString> VerifyFixedArrayFloatStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_FLOAT_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayFloatStorage(), "ani_fixedarray_float");
}

static std::optional<PandaString> VerifyFixedArrayDoubleStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_FIXED_ARRAY_DOUBLE_STORAGE);
    return v.VerifyTypeStorage(arg.GetValueFixedArrayDoubleStorage(), "ani_fixedarray_double");
}

static std::optional<PandaString> VerifyResolver(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_RESOLVER);
    return v.VerifyResolver(arg.GetValueResolver());
}

static std::optional<PandaString> VerifyResolverStorage(Verifier &v, const ANIArg &arg)
{
    ASSERT(arg.GetAction() == ANIArg::Action::VERIFY_RESOLVER_STORAGE);
    return v.VerifyTypeStorage<VResolver **>(arg.GetValueResolverStorage(), "ani_resolver");
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
static constexpr std::array<CheckerHandler, helpers::ToUnderlying(ANIArg::Action::NUMBER_OF_ELEMENTS)> HANDLERS = {
// CC-OFFNXT(G.PRE.02) code generation
#define X(ACTION, FN) FN,
    ANI_VERIFICATION_MAP
#undef X
};
// NOLINTEND(cppcoreguidelines-macro-usage)

static void DoAbortANI(PandaEtsVM *etsVm, std::string_view functionName, std::string_view message)
{
    PandaStringStream ss;
    ss << "DETECT AN ERROR WHEN USING ANI:";
    ss << "\n  ANI method: " << functionName;
    ss << "\n" << message;
    etsVm->GetANIVerifier()->Abort(ss.str());
}

struct ArgInfo {
    ANIArg arg;
    std::optional<PandaString> err;
};

struct ExtArgInfo {
    PandaString name;
    int64_t value;
    panda_file::Type type;
    bool isValid;
};

struct ArgsInfo {
    PandaVector<ArgInfo> argInfoList;
    std::optional<PandaVector<ExtArgInfo>> extArgInfoList;
};

static PandaString GetAniTypeByType(panda_file::Type type)
{
    // clang-format off
    switch (type.GetId()) {
        case panda_file::Type::TypeId::U1:        return "ani_boolean";
        case panda_file::Type::TypeId::U16:       return "ani_char";
        case panda_file::Type::TypeId::I8:        return "ani_byte";
        case panda_file::Type::TypeId::I16:       return "ani_short";
        case panda_file::Type::TypeId::I32:       return "ani_int";
        case panda_file::Type::TypeId::I64:       return "ani_long";
        case panda_file::Type::TypeId::F32:       return "ani_float";
        case panda_file::Type::TypeId::F64:       return "ani_double";
        case panda_file::Type::TypeId::REFERENCE: return "ani_ref";
        default: UNREACHABLE();
    }
    // clang-format on
    UNREACHABLE();
}

// CC-OFFNXT(huge_method[C++]) solid logic
static void DoANIArgsAbort(PandaEtsVM *etsVm, std::string_view functionName, const ArgsInfo &argsInfo)
{
    struct MsgArgInfo {
        std::string_view name;
        PandaString value;
        PandaString type;
        PandaString error;
    };
    size_t maxNameSize = 0;
    size_t maxValueSize = 0;
    size_t maxTypeSize = 0;

    PandaVector<MsgArgInfo> msgArgInfoList;
    for (const ArgInfo &v : argsInfo.argInfoList) {
        PandaStringStream ssError;
        if (v.err) {
            ssError << "INVALID: " << v.err.value();
        } else {
            ssError << "VALID";
        }
        MsgArgInfo argInfo {v.arg.GetName(), v.arg.GetStringValue(), v.arg.GetStringType(), ssError.str()};
        if (argInfo.name.size() > maxNameSize) {
            maxNameSize = argInfo.name.size();
        }
        if (argInfo.value.size() > maxValueSize) {
            maxValueSize = argInfo.value.size();
        }
        if (argInfo.type.size() > maxTypeSize) {
            maxTypeSize = argInfo.type.size();
        }
        msgArgInfoList.emplace_back(argInfo);
    }
    if (argsInfo.extArgInfoList) {
        for (const ExtArgInfo &v : argsInfo.extArgInfoList.value()) {
            PandaStringStream ssError;
            if (v.isValid) {
                ssError << "VALID";
            } else {
                ssError << "INVALID: wrong value";
            }
            PandaStringStream ssValue;
            ssValue << "0x" << std::hex << v.value;
            MsgArgInfo argInfo {v.name, ssValue.str(), GetAniTypeByType(v.type), ssError.str()};
            if (argInfo.name.size() > maxNameSize) {
                maxNameSize = argInfo.name.size();
            }
            if (argInfo.value.size() > maxValueSize) {
                maxValueSize = argInfo.value.size();
            }
            if (argInfo.type.size() > maxTypeSize) {
                maxTypeSize = argInfo.type.size();
            }
            msgArgInfoList.emplace_back(argInfo);
        }
    }

    PandaStringStream ss;
    ss << std::setfill(' ');
    bool isFirst = true;
    for (auto &arg : msgArgInfoList) {
        std::string_view name = arg.name;
        PandaString &value = arg.value;
        PandaString &type = arg.type;
        PandaString &err = arg.error;
        if (isFirst) {
            isFirst = false;
        } else {
            ss << "\n";
        }
        ss << "    " << std::right << std::setw(maxNameSize) << name << ": " << std::setw(maxValueSize) << value
           << " | " << std::left << std::setw(maxTypeSize) << type << " | " << err;
    }

    DoAbortANI(etsVm, functionName, ss.str());
}

static PandaVector<ExtArgInfo> MakeExtArgInfoList(PandaEnv *pandaEnv, ANIArg::AniMethodArgs *methodArgs)
{
    if (methodArgs->method == nullptr || methodArgs->vargs == nullptr) {
        return {};
    }

    ASSERT(methodArgs != nullptr);
    auto getName = [](size_t index) -> PandaString {
        PandaStringStream ss;
        ss << '[' << index << ']';
        return ss.str();
    };

    PandaVector<ExtArgInfo> extArgInfoList;
    size_t i = 0;

    CallArgs callArgs(methodArgs->method, methodArgs->vargs);
    auto envANIVerifier = pandaEnv->GetEnvANIVerifier();
    callArgs.ForEachArgs([&](ani_value value, panda_file::Type type, size_t refIndex) -> bool {
        PandaString name = getName(i++);
        bool isValid = IsValidRawAniValue(envANIVerifier, value, type, methodArgs->isVaArgs);
        if (isValid && type.IsReference()) {
            isValid =
                IsValidMethodArgRefType(pandaEnv, methodArgs->method, reinterpret_cast<VRef *>(value.r), refIndex);
        }
        extArgInfoList.emplace_back(ExtArgInfo {std::move(name), value.l, type, isValid});
        return true;
    });
    return extArgInfoList;
}

// CC-OFFNXT(G.NAM.03) false positive
bool VerifyANIArgs(std::string_view functionName, std::initializer_list<ANIArg> args)
{
    VVm *vvm = VVm::GetInstance();
    VEnv *venv = VEnv::GetCurrent();

    bool success = true;
    PandaVector<ArgInfo> argInfoList;
    Verifier verifier(vvm, venv);
    for (const ANIArg &arg : args) {
        auto id = helpers::ToUnderlying(arg.GetAction());
        ASSERT(id < HANDLERS.size());
        CheckerHandler handler = HANDLERS[id];
        ASSERT(handler != nullptr);
        auto err = handler(verifier, arg);
        success &= !err.has_value();
        argInfoList.emplace_back(ArgInfo {arg, std::move(err)});
    }

    if (!success) {
        const ArgInfo &lastArgInfo = argInfoList.back();
        auto action = lastArgInfo.arg.GetAction();
        ArgsInfo argsInfo {};
        if (action == ANIArg::Action::VERIFY_METHOD_V_ARGS || action == ANIArg::Action::VERIFY_METHOD_A_ARGS) {
            auto *methodArgs = lastArgInfo.arg.GetValueMethodArgs();
            PandaEnv *pandaEnv = PandaEnv::FromAniEnv(venv->GetEnv());
            argsInfo.extArgInfoList = MakeExtArgInfoList(pandaEnv, methodArgs);
        }
        argsInfo.argInfoList = std::move(argInfoList);
        DoANIArgsAbort(PandaEtsVM::FromAniVM(vvm->GetVm()), functionName, argsInfo);
    }
    return success;
}

void VerifyAbortANI(std::string_view functionName, std::string_view message)
{
    PandaEtsVM *etsVm = PandaEtsVM::GetCurrent();
    DoAbortANI(etsVm, functionName, "    " + std::string(message));
}

}  // namespace ark::ets::ani::verify
