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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H

#include "plugins/ets/runtime/ani/ani.h"
#include "runtime/include/mem/panda_containers.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "runtime/include/mem/panda_string.h"

// clang-format off
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define ANI_VERIFICATION_MAP                                                      \
    X(VERIFY_VM,                            VerifyVm)                             \
    X(VERIFY_ENV,                           VerifyEnv)                            \
    X(VERIFY_ENV_SKIP_PENDING_ERROR,        VerifyEnvSkipPendingError)            \
    X(VERIFY_OPTIONS,                       VerifyOptions)                        \
    X(VERIFY_ENV_VERSION,                   VerifyEnvVersion)                     \
    X(VERIFY_NR_REFS,                       VerifyNrRefs)                         \
    X(VERIFY_REF,                           VerifyRef)                            \
    X(VERIFY_CLASS,                         VerifyClass)                          \
    X(VERIFY_STRING,                        VerifyString)                         \
    X(VERIFY_ERROR,                         VerifyError)                          \
    X(VERIFY_DEL_LOCAL_REF,                 VerifyDelLocalRef)                    \
    X(VERIFY_THIS_OBJECT,                   VerifyThisObject)                     \
    X(VERIFY_CTOR,                          VerifyCtor)                           \
    X(VERIFY_METHOD,                        VerifyMethod)                         \
    X(VERIFY_STATIC_METHOD,                 VerifyStaticMethod)                   \
    X(VERIFY_FUNCTION,                      VerifyFunction)                       \
    X(VERIFY_FIELD,                         VerifyField)                          \
    X(VERIFY_METHOD_A_ARGS,                 VerifyMethodAArgs)                    \
    X(VERIFY_METHOD_V_ARGS,                 VerifyMethodVArgs)                    \
    X(VERIFY_VM_STORAGE,                    VerifyVmStorage)                      \
    X(VERIFY_ENV_STORAGE,                   VerifyEnvStorage)                     \
    X(VERIFY_BOOLEAN_STORAGE,               VerifyBooleanStorage)                 \
    X(VERIFY_CHAR_STORAGE,                  VerifyCharStorage)                    \
    X(VERIFY_BYTE_STORAGE,                  VerifyByteStorage)                    \
    X(VERIFY_SHORT_STORAGE,                 VerifyShortStorage)                   \
    X(VERIFY_INT_STORAGE,                   VerifyIntStorage)                     \
    X(VERIFY_LONG_STORAGE,                  VerifyLongStorage)                    \
    X(VERIFY_FLOAT_STORAGE,                 VerifyFloatStorage)                   \
    X(VERIFY_DOUBLE_STORAGE,                VerifyDoubleStorage)                  \
    X(VERIFY_REF_STORAGE,                   VerifyRefStorage)                     \
    X(VERIFY_OBJECT_STORAGE,                VerifyObjectStorage)                  \
    X(VERIFY_STRING_STORAGE,                VerifyStringStorage)                  \
    X(VERIFY_SIZE_STORAGE,                  VerifySizeStorage)                    \
    X(VERIFY_BOOLEAN,                       VerifyBoolean)                        \
    X(VERIFY_UTF16_BUFFER,                  VerifyUTF16Buffer)                    \
    X(VERIFY_UTF16_STRING,                  VerifyUTF16String)                    \
    X(VERIFY_UTF8_BUFFER,                   VerifyUTF8Buffer)                     \
    X(VERIFY_UTF8_STRING,                   VerifyUTF8String)                     \
    X(VERIFY_SIZE,                          VerifySize)                           \
    X(VERIFY_RESOLVER,                      VerifyResolver)                       \
    X(VERIFY_ERROR_STORAGE,                 VerifyErrorStorage)                   \
    X(VERIFY_FIXED_ARRAY_BOOLEAN_STORAGE,   VerifyFixedArrayBooleanStorage)       \
    X(VERIFY_FIXED_ARRAY_CHAR_STORAGE,      VerifyFixedArrayCharStorage)          \
    X(VERIFY_FIXED_ARRAY_BYTE_STORAGE,      VerifyFixedArrayByteStorage)          \
    X(VERIFY_FIXED_ARRAY_SHORT_STORAGE,     VerifyFixedArrayShortStorage)         \
    X(VERIFY_FIXED_ARRAY_INT_STORAGE,       VerifyFixedArrayIntStorage)           \
    X(VERIFY_FIXED_ARRAY_LONG_STORAGE,      VerifyFixedArrayLongStorage)          \
    X(VERIFY_FIXED_ARRAY_FLOAT_STORAGE,     VerifyFixedArrayFloatStorage)         \
    X(VERIFY_FIXED_ARRAY_DOUBLE_STORAGE,    VerifyFixedArrayDoubleStorage)        \
    X(VERIFY_RESOLVER_STORAGE,              VerifyResolverStorage)                \

// CC-OFFNXT(G.PRE.02-CPP, G.PRE.06) solid logic
#define ANI_ARG_TYPES_MAP                                                                    \
    X(ANI_VM,                           Vm,                        VVm *)                    \
    X(ANI_ENV,                          Env,                       VEnv *)                   \
    X(ANI_OPTIONS,                      Options,                   const ani_options *)      \
    X(ANI_SIZE,                         Size,                      ani_size)                 \
    X(ANI_BOOLEAN,                      Boolean,                   ani_boolean)              \
    X(ANI_CHAR,                         Char,                      ani_char)                 \
    X(ANI_BYTE,                         Byte,                      ani_byte)                 \
    X(ANI_SHORT,                        Short,                     ani_short)                \
    X(ANI_INT,                          Int,                       ani_int)                  \
    X(ANI_LONG,                         Long,                      ani_long)                 \
    X(ANI_FLOAT,                        Float,                     ani_float)                \
    X(ANI_DOUBLE,                       Double,                    ani_double)               \
    X(ANI_REF,                          Ref,                       VRef *)                   \
    X(ANI_CLASS,                        Class,                     VClass *)                 \
    X(ANI_OBJECT,                       Object,                    VObject *)                \
    X(ANI_METHOD,                       Method,                    VMethod *)                \
    X(ANI_STATIC_METHOD,                StaticMethod,              VStaticMethod *)          \
    X(ANI_FUNCTION,                     Function,                  VFunction *)              \
    X(ANI_FIELD,                        Field,                     VField *)                 \
    X(ANI_STRING,                       String,                    VString *)                \
    X(ANI_ERROR,                        Error,                     VError *)                 \
    X(ANI_VALUE_ARGS,                   ValueArgs,                 const ani_value *)        \
    X(ANI_ENV_STORAGE,                  EnvStorage,                VEnv **)                  \
    X(ANI_VM_STORAGE,                   VmStorage,                 ani_vm **)                \
    X(ANI_BOOLEAN_STORAGE,              BooleanStorage,            ani_boolean *)            \
    X(ANI_CHAR_STORAGE,                 CharStorage,               ani_char *)               \
    X(ANI_BYTE_STORAGE,                 ByteStorage,               ani_byte *)               \
    X(ANI_SHORT_STORAGE,                ShortStorage,              ani_short *)              \
    X(ANI_INT_STORAGE,                  IntStorage,                ani_int *)                \
    X(ANI_LONG_STORAGE,                 LongStorage,               ani_long *)               \
    X(ANI_FLOAT_STORAGE,                FloatStorage,              ani_float *)              \
    X(ANI_DOUBLE_STORAGE,               DoubleStorage,             ani_double *)             \
    X(ANI_REF_STORAGE,                  RefStorage,                VRef **)                  \
    X(ANI_OBJECT_STORAGE,               ObjectStorage,             VObject **)               \
    X(ANI_STRING_STORAGE,               StringStorage,             VString **)               \
    X(ANI_SIZE_STORAGE,                 SizeStorage,               ani_size *)               \
    X(ANI_UTF8_BUFFER,                  UTF8Buffer,                char *)                   \
    X(ANI_UTF16_BUFFER,                 UTF16Buffer,               uint16_t *)               \
    X(ANI_UTF8_STRING,                  UTF8String,                const char *)             \
    X(ANI_UTF16_STRING,                 UTF16String,               const uint16_t *)         \
    X(ANI_ERROR_STORAGE,                ErrorStorage,              VError **)                \
    X(UINT32,                           U32,                       uint32_t)                 \
    X(METHOD_ARGS,                      MethodArgs,                AniMethodArgs *)          \
    X(ANI_FIXED_ARRAY_BOOLEAN_STORAGE,  FixedArrayBooleanStorage,  VFixedArrayBoolean **)    \
    X(ANI_FIXED_ARRAY_CHAR_STORAGE,     FixedArrayCharStorage,     VFixedArrayChar **)       \
    X(ANI_FIXED_ARRAY_BYTE_STORAGE,     FixedArrayByteStorage,     VFixedArrayByte **)       \
    X(ANI_FIXED_ARRAY_SHORT_STORAGE,    FixedArrayShortStorage,    VFixedArrayShort **)      \
    X(ANI_FIXED_ARRAY_INT_STORAGE,      FixedArrayIntStorage,      VFixedArrayInt **)        \
    X(ANI_FIXED_ARRAY_LONG_STORAGE,     FixedArrayLongStorage,     VFixedArrayLong **)       \
    X(ANI_FIXED_ARRAY_FLOAT_STORAGE,    FixedArrayFloatStorage,    VFixedArrayFloat **)      \
    X(ANI_FIXED_ARRAY_DOUBLE_STORAGE,   FixedArrayDoubleStorage,   VFixedArrayDouble **)     \
    X(ANI_RESOLVER,                     Resolver,                  VResolver *)              \
    X(ANI_RESOLVER_STORAGE,             ResolverStorage,           VResolver **)             \

// NOLINTEND(cppcoreguidelines-macro-usage)
// clang-format on

namespace ark::ets {
class EtsMethod;
}  // namespace ark::ets

namespace ark::ets::ani::verify {

class VVm;
class VEnv;

class VRef;
class VObject;
class VClass;
class VMethod;
class VStaticMethod;
class VFunction;
class VField;
class VString;
class VError;
class VFixedArrayBoolean;
class VFixedArrayChar;
class VFixedArrayByte;
class VFixedArrayShort;
class VFixedArrayInt;
class VFixedArrayLong;
class VFixedArrayFloat;
class VFixedArrayDouble;
class VResolver;

class ANIArg {
public:
    struct AniMethodArgs {
        EtsMethod *method;
        const ani_value *vargs;  // NOTE: Reblace ani_value by VValue
        PandaSmallVector<ani_value> argsStorage;
        bool isVaArgs;
    };

    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    // clang-format off
    enum class Action : size_t {
    // CC-OFFNXT(G.PRE.02) code generation
#define X(ACTION, FN) ACTION,
        ANI_VERIFICATION_MAP
#undef X
        NUMBER_OF_ELEMENTS,
    };
    // clang-format on
    // NOLINTEND(cppcoreguidelines-macro-usage)

    static ANIArg MakeForVm(VVm *vm, std::string_view name)
    {
        return ANIArg(ArgValueByVm(vm), name, Action::VERIFY_VM);
    }

    static ANIArg MakeForEnvVersion(uint32_t version, std::string_view name)
    {
        return ANIArg(ArgValueByU32(version), name, Action::VERIFY_ENV_VERSION);
    }

    static ANIArg MakeForEnvStorage(VEnv **envStorage, std::string_view name)
    {
        return ANIArg(ArgValueByEnvStorage(envStorage), name, Action::VERIFY_ENV_STORAGE);
    }

    static ANIArg MakeForOptions(const ani_options *options, std::string_view name)
    {
        return ANIArg(ArgValueByOptions(options), name, Action::VERIFY_OPTIONS);
    }

    static ANIArg MakeForEnv(VEnv *venv, std::string_view name, bool checkPendingError = true)
    {
        if (checkPendingError) {
            return ANIArg(ArgValueByEnv(venv), name, Action::VERIFY_ENV);
        }
        return ANIArg(ArgValueByEnv(venv), name, Action::VERIFY_ENV_SKIP_PENDING_ERROR);
    }

    static ANIArg MakeForNrRefs(ani_size nrRefs, std::string_view name)
    {
        return ANIArg(ArgValueBySize(nrRefs), name, Action::VERIFY_NR_REFS);
    }

    static ANIArg MakeForRef(VRef *vref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vref), name, Action::VERIFY_REF);
    }

    static ANIArg MakeForClass(VClass *vclass, std::string_view name)
    {
        return ANIArg(ArgValueByClass(vclass), name, Action::VERIFY_CLASS);
    }

    static ANIArg MakeForString(VString *str, std::string_view name)
    {
        return ANIArg(ArgValueByString(str), name, Action::VERIFY_STRING);
    }

    static ANIArg MakeForUTF8Buffer(char *utf8Buffer, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8Buffer(utf8Buffer), name, Action::VERIFY_UTF8_BUFFER);
    }

    static ANIArg MakeForError(VError *verr, std::string_view name)
    {
        return ANIArg(ArgValueByError(verr), name, Action::VERIFY_ERROR);
    }

    static ANIArg MakeForUTF8String(const char *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF8String(ptr), name, Action::VERIFY_UTF8_STRING);
    }

    static ANIArg MakeForUTF16Buffer(uint16_t *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF16Buffer(ptr), name, Action::VERIFY_UTF16_BUFFER);
    }

    static ANIArg MakeForUTF16String(const uint16_t *ptr, std::string_view name)
    {
        return ANIArg(ArgValueByUTF16String(ptr), name, Action::VERIFY_UTF16_STRING);
    }

    static ANIArg MakeForSize(ani_size size, std::string_view name)
    {
        return ANIArg(ArgValueBySize(size), name, Action::VERIFY_SIZE);
    }

    static ANIArg MakeForDelLocalRef(VRef *vref, std::string_view name)
    {
        return ANIArg(ArgValueByRef(vref), name, Action::VERIFY_DEL_LOCAL_REF);
    }

    static ANIArg MakeForObject(VObject *vobject, std::string_view name)
    {
        return ANIArg(ArgValueByObject(vobject), name, Action::VERIFY_THIS_OBJECT);
    }

    static ANIArg MakeForMethod(VMethod *vmethod, std::string_view name, EtsType returnType, bool isConstructor = false)
    {
        if (isConstructor) {
            return ANIArg(ArgValueByMethod(vmethod), name, Action::VERIFY_CTOR, returnType);
        }
        return ANIArg(ArgValueByMethod(vmethod), name, Action::VERIFY_METHOD, returnType);
    }

    static ANIArg MakeForStaticMethod(VStaticMethod *vstaticmethod, std::string_view name, EtsType returnType)
    {
        return ANIArg(ArgValueByStaticMethod(vstaticmethod), name, Action::VERIFY_STATIC_METHOD, returnType);
    }

    static ANIArg MakeForFunction(VFunction *vfunction, std::string_view name, EtsType returnType)
    {
        return ANIArg(ArgValueByFunction(vfunction), name, Action::VERIFY_FUNCTION, returnType);
    }

    static ANIArg MakeForField(VField *vfield, std::string_view name, EtsType returnType)
    {
        return ANIArg(ArgValueByField(vfield), name, Action::VERIFY_FIELD, returnType);
    }

    static ANIArg MakeForMethodArgs(AniMethodArgs *aniMethodArgs, std::string_view name)
    {
        return ANIArg(ArgValueByMethodArgs(aniMethodArgs), name, Action::VERIFY_METHOD_V_ARGS);
    }

    static ANIArg MakeForMethodAArgs(AniMethodArgs *aniMethodArgs, std::string_view name)
    {
        return ANIArg(ArgValueByMethodArgs(aniMethodArgs), name, Action::VERIFY_METHOD_A_ARGS);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForBooleanStorage(ani_boolean *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByBooleanStorage(valueStorage), name, Action::VERIFY_BOOLEAN_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForCharStorage(ani_char *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByCharStorage(valueStorage), name, Action::VERIFY_CHAR_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForByteStorage(ani_byte *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByByteStorage(valueStorage), name, Action::VERIFY_BYTE_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForShortStorage(ani_short *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByShortStorage(valueStorage), name, Action::VERIFY_SHORT_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForIntStorage(ani_int *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByIntStorage(valueStorage), name, Action::VERIFY_INT_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForLongStorage(ani_long *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByLongStorage(valueStorage), name, Action::VERIFY_LONG_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForFloatStorage(ani_float *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFloatStorage(valueStorage), name, Action::VERIFY_FLOAT_STORAGE);
    }

    // NOLINTNEXTLINE(readability-non-const-parameter)
    static ANIArg MakeForDoubleStorage(ani_double *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByDoubleStorage(valueStorage), name, Action::VERIFY_DOUBLE_STORAGE);
    }

    static ANIArg MakeForRefStorage(VRef **refStorage, std::string_view name)
    {
        return ANIArg(ArgValueByRefStorage(refStorage), name, Action::VERIFY_REF_STORAGE);
    }

    static ANIArg MakeForObjectStorage(VObject **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByObjectStorage(valueStorage), name, Action::VERIFY_OBJECT_STORAGE);
    }

    static ANIArg MakeForStringStorage(VString **strStorage, std::string_view name)
    {
        return ANIArg(ArgValueByStringStorage(strStorage), name, Action::VERIFY_STRING_STORAGE);
    }

    static ANIArg MakeForSizeStorage(ani_size *sizeStorage, std::string_view name)
    {
        return ANIArg(ArgValueBySizeStorage(sizeStorage), name, Action::VERIFY_SIZE_STORAGE);
    }

    static ANIArg MakeForBoolean(ani_boolean booleanValue, std::string_view name)
    {
        return ANIArg(ArgValueByBoolean(booleanValue), name, Action::VERIFY_BOOLEAN);
    }

    static ANIArg MakeForErrorStorage(VError **errStorage, std::string_view name)
    {
        return ANIArg(ArgValueByErrorStorage(errStorage), name, Action::VERIFY_ERROR_STORAGE);
    }

    static ANIArg MakeForArrayBooleanStorage(VFixedArrayBoolean **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayBooleanStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_BOOLEAN_STORAGE);
    }

    static ANIArg MakeForArrayCharStorage(VFixedArrayChar **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayCharStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_CHAR_STORAGE);
    }

    static ANIArg MakeForArrayByteStorage(VFixedArrayByte **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayByteStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_BYTE_STORAGE);
    }

    static ANIArg MakeForArrayShortStorage(VFixedArrayShort **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayShortStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_SHORT_STORAGE);
    }

    static ANIArg MakeForArrayIntStorage(VFixedArrayInt **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayIntStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_INT_STORAGE);
    }

    static ANIArg MakeForArrayLongStorage(VFixedArrayLong **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayLongStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_LONG_STORAGE);
    }

    static ANIArg MakeForArrayFloatStorage(VFixedArrayFloat **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayFloatStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_FLOAT_STORAGE);
    }

    static ANIArg MakeForArrayDoubleStorage(VFixedArrayDouble **arrStorage, std::string_view name)
    {
        return ANIArg(ArgValueByFixedArrayDoubleStorage(arrStorage), name, Action::VERIFY_FIXED_ARRAY_DOUBLE_STORAGE);
    }

    static ANIArg MakeForResolver(VResolver *valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByResolver(valueStorage), name, Action::VERIFY_RESOLVER);
    }

    static ANIArg MakeForResolverStorage(VResolver **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByResolverStorage(valueStorage), name, Action::VERIFY_RESOLVER_STORAGE);
    }

    static ANIArg MakeForPromiseStorage(VObject **valueStorage, std::string_view name)
    {
        return ANIArg(ArgValueByObjectStorage(valueStorage), name, Action::VERIFY_OBJECT_STORAGE);
    }

    Action GetAction() const
    {
        return action_;
    }

    std::string_view GetName() const
    {
        return name_;
    }

    PandaString GetStringValue() const;
    PandaString GetStringType() const;

    // NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)
    // Generate methods:
    //   <Type> GetValue<Name>() const;
#define X(ENUM_VALUE, NAME, TYPE)                                                   \
    TYPE GetValue##NAME() const                                                     \
    {                                                                               \
        ASSERT(type_ == ValueType::ENUM_VALUE); /* CC-OFF(G.PRE.02) function gen */ \
        return value_.v##NAME;                  /* CC-OFF(G.PRE.05) function gen */ \
    }
    ANI_ARG_TYPES_MAP
#undef X
    // NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)

    void *GetValueRawPointer() const
    {
        return reinterpret_cast<void *>(value_.vVm);  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    EtsType GetReturnType() const
    {
        return returnType_;
    }

private:
    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    union RawValue {
// CC-OFFNXT(G.PRE.02,G.PRE.09) code generation
#define X(ENUM_VALUE, NAME, TYPE) TYPE v##NAME;
        ANI_ARG_TYPES_MAP
#undef X
    };
    static_assert(sizeof(RawValue) == sizeof(double));

    enum class ValueType {
// CC-OFFNXT(G.PRE.02) code generation
#define X(ENUM_VALUE, NAME, TYPE) ENUM_VALUE,
        ANI_ARG_TYPES_MAP
#undef X
    };
    // NOLINTEND(cppcoreguidelines-macro-usage)

    struct ArgValue {
        RawValue value;
        ValueType type;
    };

    explicit ANIArg(ArgValue value, std::string_view name, Action action)
        : value_(value.value), type_(value.type), name_(name), action_(action)
    {
    }

    explicit ANIArg(ArgValue value, std::string_view name, Action action, EtsType returnType)
        : value_(value.value), type_(value.type), name_(name), action_(action), returnType_(returnType)
    {
    }

    // NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)
    // Generate methods:
    //   static ArgValue ArgValueBy<Name>(<Type> value);
#define X(ENUM_VALUE, NAME, TYPE)                                                         \
    static ArgValue ArgValueBy##NAME(TYPE value)                                          \
    {                                                                                     \
        RawValue raw {};                                                                  \
        raw.v##NAME = value;                                                              \
        return {raw, ValueType::ENUM_VALUE}; /* CC-OFF(G.PRE.02,G.PRE.05) function gen */ \
    }
    ANI_ARG_TYPES_MAP
#undef X
    // NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-pro-type-union-access)

    RawValue value_;
    ValueType type_;
    std::string_view name_;
    Action action_ {};

    EtsType returnType_ {EtsType::UNKNOWN};
};

bool VerifyANIArgs(std::string_view functionName, std::initializer_list<ANIArg> args);
void VerifyAbortANI(std::string_view functionName, std::string_view message);

}  // namespace ark::ets::ani::verify

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_VERIFY_VERIFY_CHECKER_H
