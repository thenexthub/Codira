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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_DEFS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_DEFS_H_

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// ETS runtime type dependencies list. Each entry MUST follow the common naming schema.
// CC-OFFNXT(G.PRE.06) macro list
#define ETS_PLATFORM_TYPES_LIST(T, I, S)                                                                             \
    /* Core runtime type system */                                                                                   \
    T("Lstd/core/Object;", coreObject)                                                                               \
    T("Lstd/core/Class;", coreClass)                                                                                 \
    T("Lstd/core/String;", coreString)                                                                               \
    T("Lstd/core/LineString;", coreLineString)                                                                       \
    T("Lstd/core/SlicedString;", coreSlicedString)                                                                   \
    T("Lstd/core/TreeString;", coreTreeString)                                                                       \
    /* Builtin numerics */                                                                                           \
    T("Lstd/core/Boolean;", coreBoolean)                                                                             \
    T("Lstd/core/Byte;", coreByte)                                                                                   \
    T("Lstd/core/Char;", coreChar)                                                                                   \
    T("Lstd/core/Short;", coreShort)                                                                                 \
    T("Lstd/core/Int;", coreInt)                                                                                     \
    T("Lstd/core/Long;", coreLong)                                                                                   \
    T("Lstd/core/Float;", coreFloat)                                                                                 \
    T("Lstd/core/Double;", coreDouble)                                                                               \
    /* Builtin language support */                                                                                   \
    T("Lstd/core/BigInt;", coreBigInt)                                                                               \
    T("Lescompat/Error;", escompatError)                                                                             \
    T("Lstd/core/Function;", coreFunction)                                                                           \
    T("Lstd/core/Tuple;", coreTuple)                                                                                 \
    T("Lstd/core/TupleN;", coreTupleN)                                                                               \
    /* Runtime linkage */                                                                                            \
    T("Lstd/core/RuntimeLinker;", coreRuntimeLinker)                                                                 \
    I("Lstd/core/RuntimeLinker;", "loadClass", "Lstd/core/String;Lstd/core/Boolean;:Lstd/core/Class;",               \
      coreRuntimeLinkerLoadClass)                                                                                    \
    T("Lstd/core/BootRuntimeLinker;", coreBootRuntimeLinker)                                                         \
    T("Lstd/core/AbcRuntimeLinker;", coreAbcRuntimeLinker)                                                           \
    T("Lstd/core/MemoryRuntimeLinker;", coreMemoryRuntimeLinker)                                                     \
    T("Lstd/core/AbcFile;", coreAbcFile)                                                                             \
    /* Error handling */                                                                                             \
    T("Lstd/core/OutOfMemoryError;", coreOutOfMemoryError)                                                           \
    T("Lstd/core/StackTraceElement;", coreStackTraceElement)                                                         \
    T("Lstd/core/LinkerClassNotFoundError;", coreLinkerClassNotFoundError)                                           \
    T("Lescompat/ErrorOptions;", escompatErrorOptions)                                                               \
    T("Lescompat/ErrorOptionsImpl;", escompatErrorOptionsImpl)                                                       \
    /* StringBuilder */                                                                                              \
    T("Lstd/core/StringBuilder;", coreStringBuilder)                                                                 \
    I("Lstd/core/StringBuilder;", "<ctor>", ":V", coreStringBuilderDefaultConstructor)                               \
    I("Lstd/core/StringBuilder;", "<ctor>", "[C:V", coreStringBuilderConstructorWithCharArrayArg)                    \
    I("Lstd/core/StringBuilder;", "<ctor>", "Lstd/core/String;:V", coreStringBuilderConstructorWithStringArg)        \
    I("Lstd/core/StringBuilder;", "%%get-stringLength", ":I", coreStringBuilderStringLength)                         \
    I("Lstd/core/StringBuilder;", "append", "Lstd/core/String;:Lstd/core/StringBuilder;",                            \
      coreStringBuilderAppendString)                                                                                 \
    I("Lstd/core/StringBuilder;", "toString", ":Lstd/core/String;", coreStringBuilderToString)                       \
    /* String */                                                                                                     \
    I("Lstd/core/String;", "concat", "Lstd/core/Array;:Lstd/core/String;", coreStringConcat)                         \
    I("Lstd/core/String;", "getLength", ":I", coreStringGetLength)                                                   \
    I("Lstd/core/String;", "%%get-length", ":I", coreStringLength)                                                   \
    /* Object array */                                                                                               \
    T("[Lstd/core/Object;", coreObjectArray)                                                                         \
    /* Concurrency */                                                                                                \
    T("Lstd/core/Promise;", corePromise)                                                                             \
    T("Lstd/core/Job;", coreJob)                                                                                     \
    I("Lstd/core/Promise;", "subscribeOnAnotherPromise", "Lstd/core/PromiseLike;:V",                                 \
      corePromiseSubscribeOnAnotherPromise)                                                                          \
    T("Lstd/core/PromiseRef;", corePromiseRef)                                                                       \
    T("Lstd/core/WaitersList;", coreWaitersList)                                                                     \
    T("Lstd/core/Mutex;", coreMutex)                                                                                 \
    T("Lstd/core/Event;", coreEvent)                                                                                 \
    T("Lstd/core/CondVar;", coreCondVar)                                                                             \
    T("Lstd/core/QueueSpinlock;", coreQueueSpinlock)                                                                 \
    T("Lstd/core/RWLock;", coreRWLock)                                                                               \
    /* Finalization */                                                                                               \
    T("Lstd/core/BaseWeakRef;", coreBaseWeakRef)                                                                     \
    T("Lstd/core/FinalizableWeakRef;", coreFinalizableWeakRef)                                                       \
    T("Lstd/core/FinalizationRegistry;", coreFinalizationRegistry)                                                   \
    S("Lstd/core/FinalizationRegistry;", "execCleanup", "Lstd/core/FinRegNode;:V",                                   \
      coreFinalizationRegistryExecCleanup)                                                                           \
    T("Lstd/core/FinRegNode;", coreFinRegNode)                                                                       \
    /* Containers */                                                                                                 \
    T("Lstd/core/Array;", escompatArray)                                                                             \
    I("Lstd/core/Array;", "pop", ":Lstd/core/Object;", escompatArrayPop)                                             \
    I("Lstd/core/Array;", "%%get-length", ":I", escompatArrayGetLength)                                              \
    I("Lstd/core/Array;", "$_get", "I:Lstd/core/Object;", escompatArrayGet)                                          \
    I("Lstd/core/Array;", "$_set", "ILstd/core/Object;:V", escompatArraySet)                                         \
    /* ArrayBuffer */                                                                                                \
    T("Lstd/core/ArrayBuffer;", coreArrayBuffer)                                                                     \
    T("Lescompat/Int8Array;", escompatInt8Array)                                                                     \
    T("Lescompat/Uint8Array;", escompatUint8Array)                                                                   \
    T("Lescompat/Uint8ClampedArray;", escompatUint8ClampedArray)                                                     \
    T("Lescompat/Int16Array;", escompatInt16Array)                                                                   \
    T("Lescompat/Uint16Array;", escompatUint16Array)                                                                 \
    T("Lescompat/Int32Array;", escompatInt32Array)                                                                   \
    T("Lescompat/Uint32Array;", escompatUint32Array)                                                                 \
    T("Lescompat/Float32Array;", escompatFloat32Array)                                                               \
    T("Lescompat/Float64Array;", escompatFloat64Array)                                                               \
    T("Lescompat/BigInt64Array;", escompatBigInt64Array)                                                             \
    T("Lescompat/BigUint64Array;", escompatBigUint64Array)                                                           \
    /* Containers */                                                                                                 \
    T("Lstd/containers/containers/ArrayAsListInt;", containersArrayAsListInt)                                        \
    T("Lstd/core/ArrayLike;", coreArrayLike)                                                                         \
    T("Lstd/core/Record;", coreRecord)                                                                               \
    I("Lstd/core/Record;", "$_get", "{ULstd/core/BaseEnum;Lstd/core/Numeric;Lstd/core/String;}:Lstd/core/Object;",   \
      coreRecordGet)                                                                                                 \
    I("Lstd/core/Record;", "$_set", "{ULstd/core/BaseEnum;Lstd/core/Numeric;Lstd/core/String;}Lstd/core/Object;:V",  \
      coreRecordSet)                                                                                                 \
    T("Lstd/core/Map;", coreMap)                                                                                     \
    T("Lstd/core/Set;", coreSet)                                                                                     \
    /* Interop */                                                                                                    \
    T("Lstd/interop/js/JSValue;", interopJSValue)                                                                    \
    T("Lstd/interop/ESValue;", interopESValue)                                                                       \
    /* TypeAPI */                                                                                                    \
    T("Lstd/core/Field;", coreField)                                                                                 \
    T("Lstd/core/Method;", coreMethod)                                                                               \
    T("Lstd/core/TypeAPIParameter;", coreTypeAPIParameter)                                                           \
    T("Lstd/core/ClassType;", coreClassType)                                                                         \
    T("Lstd/core/reflect/InstanceField;", coreReflectInstanceField)                                                  \
    T("Lstd/core/reflect/InstanceMethod;", coreReflectInstanceMethod)                                                \
    T("Lstd/core/reflect/StaticField;", coreReflectStaticField)                                                      \
    T("Lstd/core/reflect/StaticMethod;", coreReflectStaticMethod)                                                    \
    T("Lstd/core/reflect/Constructor;", coreReflectConstructor)                                                      \
    /* Proxy */                                                                                                      \
    T("Lstd/core/reflect/Proxy;", coreReflectProxy)                                                                  \
    I("Lstd/core/reflect/Proxy;", "<ctor>", "Lstd/core/reflect/InvocationHandler;:V", coreReflectProxyConstructor)   \
    I("Lstd/core/reflect/Proxy;", "getHandler", ":Lstd/core/reflect/InvocationHandler;", coreReflectProxyGetHandler) \
    S("Lstd/core/reflect/Proxy;", "invoke",                                                                          \
      "Lstd/core/reflect/Proxy;Lstd/core/reflect/InstanceMethod;[Lstd/core/Object;:Lstd/core/Object;",               \
      coreReflectProxyInvoke)                                                                                        \
    S("Lstd/core/reflect/Proxy;", "invokeSet",                                                                       \
      "Lstd/core/reflect/Proxy;Lstd/core/reflect/InstanceMethod;Lstd/core/Object;:V", coreReflectProxyInvokeSet)     \
    S("Lstd/core/reflect/Proxy;", "invokeGet",                                                                       \
      "Lstd/core/reflect/Proxy;Lstd/core/reflect/InstanceMethod;:Lstd/core/Object;", coreReflectProxyInvokeGet)      \
    /* Process */                                                                                                    \
    T("Lstd/core/StdProcess;", coreStdProcess)                                                                       \
    S("Lstd/core/StdProcess;", "listUnhandledJobs", "Lstd/core/Array;:V", coreStdProcessListUnhandledJobs)           \
    S("Lstd/core/StdProcess;", "listUnhandledPromises", "Lstd/core/Array;:V", coreStdProcessListUnhandledPromises)   \
    S("Lstd/core/StdProcess;", "HandleUncaughtError", "Lstd/core/Object;:V", coreStdProcessHandleUncaughtError)      \
    /* JSON */                                                                                                       \
    T("Lstd/core/JsonReplacer;", coreJsonReplacer)                                                                   \
    T("Lstd/core/jsonx/JsonElementSerializable;", coreJsonElementSerializable)                                       \
    T("Lstd/core/JsonSerializable;", coreJsonSerializable)                                                           \
    I("Lstd/core/JsonSerializable;", "toJSON", ":Lstd/core/String;", coreJsonSerializableToJSON)                     \
    /* RegExp */                                                                                                     \
    T("Lstd/core/RegExpResultArray;", coreRegExpResultArray)                                                         \
    /* Date */                                                                                                       \
    T("Lstd/core/Date;", coreDate)                                                                                   \
    /* Do not add anything below this line. */

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif  // PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_DEFS_H_
