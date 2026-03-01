/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "intrinsics.h"
#include "plugins/ets/runtime/interop_js/intrinsics_api_impl.h"
#include "plugins/ets/runtime/interop_js/js_value.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "plugins/ets/runtime/intrinsics/gc_task_tracker.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "interop_js/call/call.h"

namespace ark::ets::interop::js::intrinsics {

void JSRuntimeFinalizationRegistryCallbackIntrinsic(EtsObject *obj)
{
    JSRuntimeFinalizationRegistryCallback(obj);
}

JSValue *JSRuntimeNewJSValueDoubleIntrinsic(double v)
{
    return JSRuntimeNewJSValueDouble(v);
}

JSValue *JSRuntimeNewJSValueBooleanIntrinsic(uint8_t v)
{
    return JSRuntimeNewJSValueBoolean(v);
}

JSValue *JSRuntimeNewJSValueStringIntrinsic(EtsString *v)
{
    return JSRuntimeNewJSValueString(v);
}

JSValue *JSRuntimeNewJSValueObjectIntrinsic(EtsObject *v)
{
    return JSRuntimeNewJSValueObject(v);
}

uint8_t JSRuntimeIsJSValueIntrinsic(EtsObject *v)
{
    return JSRuntimeIsJSValue(v);
}

JSValue *JSRuntimeNewJSValueBigIntIntrinsic(EtsBigInt *v)
{
    return JSRuntimeNewJSValueBigInt(v);
}

double JSRuntimeGetValueDoubleIntrinsic(JSValue *etsJsValue)
{
    return JSRuntimeGetValueDouble(etsJsValue);
}

uint8_t JSRuntimeGetValueBooleanIntrinsic(JSValue *etsJsValue)
{
    return JSRuntimeGetValueBoolean(etsJsValue);
}

EtsString *JSRuntimeGetValueStringIntrinsic(JSValue *etsJsValue)
{
    return JSRuntimeGetValueString(etsJsValue);
}

EtsObject *JSRuntimeGetValueObjectIntrinsic(EtsObject *etsValue, EtsClass *cls)
{
    return JSRuntimeGetValueObject(etsValue, cls);
}

JSValue *JSRuntimeGetPropertyJSValueIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return JSValueNamedGetter<JSConvertJSValue>(etsJsValue, etsPropName);
}

JSValue *JSRuntimeGetPropertyJSValueyByKeyIntrinsic(JSValue *objectValue, JSValue *keyValue)
{
    return JSRuntimeGetPropertyJSValueyByKey(objectValue, keyValue);
}

double JSRuntimeGetPropertyDoubleIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return JSValueNamedGetter<JSConvertF64>(etsJsValue, etsPropName);
}

EtsString *JSRuntimeGetPropertyStringIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return JSValueNamedGetter<JSConvertString>(etsJsValue, etsPropName);
}

uint8_t JSRuntimeGetPropertyBooleanIntrinsic(JSValue *etsJsValue, EtsString *etsPropName)
{
    return static_cast<uint8_t>(JSValueNamedGetter<JSConvertU1>(etsJsValue, etsPropName));
}

void JSRuntimeSetPropertyJSValueIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, JSValue *value)
{
    JSValueNamedSetter<JSConvertJSValue>(etsJsValue, etsPropName, value);
}

void JSRuntimeSetPropertyObjectIntrinsic(EtsObject *etsValue, EtsString *etsPropName, EtsObject *value)
{
    ESValueAnyNamedSetter(etsValue, etsPropName, value);
}

void JSRuntimeSetPropertyDoubleIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, double value)
{
    JSValueNamedSetter<JSConvertF64>(etsJsValue, etsPropName, value);
}

void JSRuntimeSetPropertyStringIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, EtsString *value)
{
    JSValueNamedSetter<JSConvertString>(etsJsValue, etsPropName, value);
}

void JSRuntimeSetPropertyBooleanIntrinsic(JSValue *etsJsValue, EtsString *etsPropName, uint8_t value)
{
    JSValueNamedSetter<JSConvertU1>(etsJsValue, etsPropName, static_cast<bool>(value));
}

void JSRuntimeSetElementJSValueIntrinsic(JSValue *etsJsValue, int32_t index, JSValue *value)
{
    JSValueIndexedSetter<JSConvertJSValue>(etsJsValue, index, value);
}

void JSRuntimeSetElementObjectIntrinsic(EtsObject *etsObject, int32_t index, EtsObject *value)
{
    ESValueAnyIndexedSetter(etsObject, index, value);
}

void JSRuntimeSetPropertyIntrinsic(EtsObject *etsObject, EtsObject *key, EtsObject *value)
{
    ESValueAnyStoreByValue(etsObject, key, value);
}

uint8_t JSRuntimeIsStaticValueIntrinsic(EtsObject *etsObject)
{
    return ESValueAnyIsStaticValue(etsObject);
}

JSValue *JSRuntimeGetElementJSValueIntrinsic(JSValue *etsJsValue, int32_t index)
{
    return JSValueIndexedGetter<JSConvertJSValue>(etsJsValue, index);
}

double JSRuntimeGetElementDoubleIntrinsic(JSValue *etsJsValue, int32_t index)
{
    return JSValueIndexedGetter<JSConvertF64>(etsJsValue, index);
}

JSValue *JSRuntimeGetUndefinedIntrinsic()
{
    return JSRuntimeGetUndefined();
}

JSValue *JSRuntimeGetNullIntrinsic()
{
    return JSRuntimeGetNull();
}

JSValue *JSRuntimeGetGlobalIntrinsic()
{
    return JSRuntimeGetGlobal();
}

JSValue *JSRuntimeCreateObjectIntrinsic()
{
    return JSRuntimeCreateObject();
}

EtsObject *JSRuntimeCreateArrayIntrinsic()
{
    return JSRuntimeCreateArray();
}

uint8_t JSRuntimeInstanceOfDynamicIntrinsic(EtsObject *object, EtsObject *ctor)
{
    return ESValueAnyInstanceOf(object, ctor);
}

uint8_t JSRuntimeInstanceOfStaticIntrinsic(JSValue *object, EtsClass *clsObj)
{
    return JSRuntimeInstanceOfStatic(object, clsObj);
}

uint8_t JSRuntimeInitJSCallClassIntrinsic()
{
    return JSRuntimeInitJSCallClass();
}

uint8_t JSRuntimeInitJSNewClassIntrinsic()
{
    return JSRuntimeInitJSNewClass();
}

JSValue *JSRuntimeLoadModuleIntrinsic(EtsString *module)
{
    return JSRuntimeLoadModule(module);
}

uint8_t JSRuntimeStrictEqualIntrinsic(JSValue *lhs, JSValue *rhs)
{
    return JSRuntimeStrictEqual(lhs, rhs);
}

uint8_t JSRuntimeHasPropertyIntrinsic(EtsObject *object, EtsString *name)
{
    return ESValueAnyHasProperty(object, name);
}

EtsObject *JSRuntimeGetPropertyIntrinsic(EtsObject *object, EtsObject *property)
{
    return ESValueAnyLoadByValue(object, property);
}

EtsObject *JSRuntimeGetElementObjectIntrinsic(EtsObject *etsValue, int32_t index)
{
    return ESValueAnyIndexedGetter(etsValue, index);
}

EtsObject *JSRuntimeGetPropertyObjectIntrinsic(EtsObject *etsValue, EtsString *etsPropName)
{
    return ESValueAnyNamedGetter(etsValue, etsPropName);
}

uint8_t JSRuntimeHasPropertyObjectIntrinsic(EtsObject *object, EtsObject *property)
{
    return ESValueAnyHasPropertyObject(object, property);
}

uint8_t JSRuntimeHasElementIntrinsic(EtsObject *object, int index)
{
    return ESValueAnyHasElement(object, index);
}

uint8_t JSRuntimeHasOwnPropertyIntrinsic(EtsObject *object, EtsString *name)
{
    return ESValueAnyHasOwnProperty(object, name);
}

uint8_t JSRuntimeHasOwnPropertyObjectIntrinsic(EtsObject *object, EtsObject *property)
{
    return ESValueAnyHasOwnPropertyObject(object, property);
}

EtsString *JSRuntimeTypeOfIntrinsic(JSValue *object)
{
    return JSRuntimeTypeOf(object);
}

uint8_t JSRuntimeIsPromiseIntrinsic(JSValue *object)
{
    return JSRuntimeIsPromise(object);
}

uint8_t JSRuntimeInstanceOfStaticTypeIntrinsic(JSValue *object, EtsTypeAPIType *paramType)
{
    return JSRuntimeInstanceOfStaticType(object, paramType);
}

EtsObject *JSRuntimeInvokeIntrinsic(EtsObject *recv, EtsObject *func, EtsObjectArray *args)
{
    return JSRuntimeInvoke(recv, func, reinterpret_cast<EtsArray *>(args));
}

EtsObject *JSRuntimeInstantiateIntrinsic(EtsObject *callable, EtsObjectArray *args)
{
    return JSRuntimeInstantiate(callable, reinterpret_cast<EtsArray *>(args));
}

EtsLong JSRuntimeXgcStartIntrinsic()
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *gc = coro->GetPandaVM()->GetGC();
    auto &gcTaskTracker = ets::intrinsics::GCTaskTracker::InitIfNeededAndGet(gc);
    auto task = MakePandaUnique<GCTask>(GCTaskCause::CROSSREF_CAUSE, time::GetCurrentTimeInNanos());
    ASSERT(task != nullptr);
    auto id = task->GetId();
    gcTaskTracker.AddTaskId(id);
    if (!XGC::GetInstance()->Trigger(gc, std::move(task))) {
        gcTaskTracker.RemoveId(id);
        return static_cast<EtsLong>(-1);
    }
    return static_cast<EtsLong>(id);
}

EtsString *JSValueToStringIntrinsic(JSValue *object)
{
    return JSValueToString(object);
}

EtsString *JSONStringifyIntrinsic(JSValue *obj)
{
    return JSONStringify(obj);
}

// Compiler intrinsics for fast interop
void *CompilerGetJSNamedPropertyIntrinsic(void *val, void *propName)
{
    return CompilerGetJSNamedProperty(val, reinterpret_cast<char *>(propName));
}

void *CompilerGetJSPropertyIntrinsic(void *val, void *prop)
{
    return CompilerGetJSProperty(val, prop);
}

void *CompilerGetJSElementIntrinsic(void *val, int32_t index)
{
    return CompilerGetJSElement(val, index);
}

void *CompilerJSCallCheckIntrinsic(void *fn)
{
    return CompilerJSCallCheck(fn);
}

void *CompilerJSCallFunctionIntrinsic(void *obj, void *fn, uint32_t argc, void *args)
{
    return CompilerJSCallFunction<true>(obj, fn, argc, args);
}

void CompilerJSCallVoidFunctionIntrinsic(void *obj, void *fn, uint32_t argc, void *args)
{
    CompilerJSCallFunction<false>(obj, fn, argc, args);
}

void *CompilerJSNewInstanceIntrinsic(void *fn, uint32_t argc, void *args)
{
    return CompilerJSNewInstance(fn, argc, args);
}

void *CompilerConvertVoidToLocalIntrinsic()
{
    return CompilerConvertVoidToLocal();
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CONVERT_LOCAL_VALUE(type, cpptype)                        \
    /* CC-OFFNXT(G.PRE.02) part name */                           \
    void *CompilerConvert##type##ToLocalIntrinsic(cpptype etsVal) \
    {                                                             \
        /* CC-OFFNXT(G.PRE.05) code generation */                 \
        return ConvertToLocal<JSConvert##type>(etsVal);           \
    }                                                             \
                                                                  \
    cpptype CompilerConvertLocalTo##type##Intrinsic(void *val)    \
    {                                                             \
        /* CC-OFFNXT(G.PRE.05) code generation */                 \
        return ConvertFromLocal<JSConvert##type>(val);            \
    }

CONVERT_LOCAL_VALUE(U1, uint8_t)
CONVERT_LOCAL_VALUE(U8, uint8_t)
CONVERT_LOCAL_VALUE(I8, int8_t)
CONVERT_LOCAL_VALUE(U16, uint16_t)
CONVERT_LOCAL_VALUE(I16, int16_t)
CONVERT_LOCAL_VALUE(U32, uint32_t)
CONVERT_LOCAL_VALUE(I32, int32_t)
CONVERT_LOCAL_VALUE(U64, uint64_t)
CONVERT_LOCAL_VALUE(I64, int64_t)
CONVERT_LOCAL_VALUE(F32, float)
CONVERT_LOCAL_VALUE(F64, double)
CONVERT_LOCAL_VALUE(JSValue, JSValue *)

#undef CONVERT_LOCAL_VALUE

void *CompilerConvertRefTypeToLocalIntrinsic(EtsObject *etsVal)
{
    return CompilerConvertRefTypeToLocal(etsVal);
}
EtsString *CompilerConvertLocalToStringIntrinsic(void *val)
{
    return CompilerConvertLocalToString(val);
}

EtsObject *CompilerConvertLocalToRefTypeIntrinsic(void *klassPtr, void *val)
{
    return CompilerConvertLocalToRefType(klassPtr, val);
}

void CompilerCreateLocalScopeIntrinsic()
{
    CreateLocalScope();
}

void CompilerDestroyLocalScopeIntrinsic()
{
    CompilerDestroyLocalScope();
}

void *CompilerLoadJSConstantPoolIntrinsic()
{
    return CompilerLoadJSConstantPool();
}

void CompilerInitJSCallClassForCtxIntrinsic(void *klass)
{
    return CompilerInitJSCallClassForCtx(klass);
}

void PromiseInteropResolveIntrinsic(EtsObject *value, EtsLong deferred)
{
    PromiseInteropResolve(value, deferred);
}

void PromiseInteropRejectIntrinsic(EtsObject *value, EtsLong deferred)
{
    PromiseInteropReject(value, deferred);
}

EtsEscompatArrayBuffer *InteropTransferHelperTransferArrayBufferToStaticImplIntrinsic(ESValue *object)
{
    return TransferArrayBufferToStatic(object);
}

EtsObject *InteropTransferHelperTransferArrayBufferToDynamicImplIntrinsic(EtsEscompatArrayBuffer *object)
{
    return TransferArrayBufferToDynamic(object);
}

EtsObject *InteropTransferHelperCreateDynamicTypedArrayIntrinsic(EtsEscompatArrayBuffer *object, int32_t typedArrayType,
                                                                 double length, double byteOffset)
{
    return CreateDynamicTypedArray(object, typedArrayType, length, byteOffset);
}

EtsObject *InteropTransferHelperCreateDynamicDataViewIntrinsic(EtsEscompatArrayBuffer *object, double length,
                                                               double byteOffset)
{
    return CreateDynamicDataView(object, length, byteOffset);
}

void InteropContextSetInteropRuntimeLinkerImplIntrinsic(EtsRuntimeLinker *linker)
{
    SetInteropRuntimeLinker(linker);
}

EtsRuntimeLinker *InteropContextGetInteropRuntimeLinkerImplIntrinsic()
{
    return GetInteropRuntimeLinker();
}

EtsBoolean InteropSerializeHelperIsJSInteropRefImplIntrinsic(EtsObject *value)
{
    return IsJSInteropRef(value);
}

EtsLong InteropSerializeHelperSerializeHandleImplIntrinsic(JSValue *value)
{
    return SerializeHandle(value);
}

JSValue *InteropSerializeHelperDeserializeHandleImplIntrinsic(EtsLong value)
{
    return DeserializeHandle(value);
}

EtsObject *JSRuntimeInvokeDynamicFunctionIntrinsic(EtsObject *thisObject, EtsObjectArray *args)
{
    return JSRuntimeInvokeDynamicFunction(thisObject, args);
}

}  // namespace ark::ets::interop::js::intrinsics
