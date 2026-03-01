/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tooling/dynamic/base/pt_types.h"
#include "ecmascript/napi/jsnapi_helper.h"
#include "ecmascript/debugger/js_debugger.h"

namespace panda::ecmascript::tooling {
using ObjectType = RemoteObject::TypeName;
using ObjectSubType = RemoteObject::SubTypeName;
using ObjectClassName = RemoteObject::ClassName;

const std::string ObjectType::Object = "object";        // NOLINT (readability-identifier-naming)
const std::string ObjectType::Function = "function";    // NOLINT (readability-identifier-naming)
const std::string ObjectType::Undefined = "undefined";  // NOLINT (readability-identifier-naming)
const std::string ObjectType::String = "string";        // NOLINT (readability-identifier-naming)
const std::string ObjectType::Number = "number";        // NOLINT (readability-identifier-naming)
const std::string ObjectType::Boolean = "boolean";      // NOLINT (readability-identifier-naming)
const std::string ObjectType::Symbol = "symbol";        // NOLINT (readability-identifier-naming)
const std::string ObjectType::Bigint = "bigint";        // NOLINT (readability-identifier-naming)
const std::string ObjectType::Wasm = "wasm";            // NOLINT (readability-identifier-naming)

const std::string ObjectSubType::Array = "array";              // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Null = "null";                // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Node = "node";                // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Regexp = "regexp";            // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Date = "date";                // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Map = "map";                  // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Set = "set";                  // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Weakmap = "weakmap";          // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Weakset = "weakset";          // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Iterator = "iterator";        // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Generator = "generator";      // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Error = "error";              // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Proxy = "proxy";              // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Promise = "promise";          // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Typedarray = "typedarray";    // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Arraybuffer = "arraybuffer";  // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Dataview = "dataview";        // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::I32 = "i32";                  // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::I64 = "i64";                  // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::F32 = "f32";                  // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::F64 = "f64";                  // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::V128 = "v128";                // NOLINT (readability-identifier-naming)
const std::string ObjectSubType::Externref = "externref";      // NOLINT (readability-identifier-naming)

const std::string ObjectClassName::Object = "Object";                  // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Function = "Function";              // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Array = "Array";                    // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Regexp = "RegExp";                  // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Date = "Date";                      // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Map = "Map";                        // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Set = "Set";                        // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Weakmap = "Weakmap";                // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Weakset = "Weakset";                // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Dataview = "Dataview";                // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::ArrayIterator = "ArrayIterator";    // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::StringIterator = "StringIterator";  // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::SetIterator = "SetIterator";        // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::MapIterator = "MapIterator";        // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Iterator = "Iterator";              // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Error = "Error";                    // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Proxy = "Object";                   // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Promise = "Promise";                // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Typedarray = "Typedarray";          // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Arraybuffer = "Arraybuffer";        // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Global = "global";                  // NOLINT (readability-identifier-naming)
const std::string ObjectClassName::Generator = "Generator";            // NOLINT (readability-identifier-naming)

const std::string RemoteObject::ObjectDescription = "Object";    // NOLINT (readability-identifier-naming)
const std::string RemoteObject::GlobalDescription = "global";    // NOLINT (readability-identifier-naming)
const std::string RemoteObject::ProxyDescription = "Proxy";      // NOLINT (readability-identifier-naming)
const std::string RemoteObject::PromiseDescription = "Promise";  // NOLINT (readability-identifier-naming)
const std::string RemoteObject::ArrayIteratorDescription =       // NOLINT (readability-identifier-naming)
    "ArrayIterator";
const std::string RemoteObject::StringIteratorDescription =  // NOLINT (readability-identifier-naming)
    "StringIterator";
const std::string RemoteObject::SetIteratorDescription = "SetIterator";  // NOLINT (readability-identifier-naming)
const std::string RemoteObject::MapIteratorDescription = "MapIterator";  // NOLINT (readability-identifier-naming)
const std::string RemoteObject::WeakRefDescription = "WeakRef";          // NOLINT (readability-identifier-naming)
const std::string RemoteObject::WeakMapDescription = "WeakMap";          // NOLINT (readability-identifier-naming)
const std::string RemoteObject::WeakSetDescription = "WeakSet";          // NOLINT (readability-identifier-naming)
const std::string RemoteObject::DataViewDescription = "DataView";          // NOLINT (readability-identifier-naming)
const std::string RemoteObject::JSPrimitiveNumberDescription =           // NOLINT (readability-identifier-naming)
    "Number";
const std::string RemoteObject::JSPrimitiveBooleanDescription =          // NOLINT (readability-identifier-naming)
    "Boolean";
const std::string RemoteObject::JSPrimitiveStringDescription =           // NOLINT (readability-identifier-naming)
    "String";
const std::string RemoteObject::JSPrimitiveSymbolDescription =           // NOLINT (readability-identifier-naming)
    "Symbol";
const std::string RemoteObject::DateTimeFormatDescription =                // NOLINT (readability-identifier-naming)
    "DateTimeFormat";
const std::string RemoteObject::JSIntlDescription = "Intl";                // NOLINT (readability-identifier-naming)
const std::string RemoteObject::NumberFormatDescription = "NumberFormat";  // NOLINT (readability-identifier-naming)
const std::string RemoteObject::CollatorDescription = "Collator";          // NOLINT (readability-identifier-naming)
const std::string RemoteObject::PluralRulesDescription = "PluralRules";    // NOLINT (readability-identifier-naming)
const std::string RemoteObject::JSLocaleDescription = "Locale";            // NOLINT (readability-identifier-naming)
const std::string RemoteObject::JSListFormatDescription = "ListFormat";    // NOLINT (readability-identifier-naming)
const std::string RemoteObject::JSRelativeTimeFormatDescription =          // NOLINT (readability-identifier-naming)
    "RelativeTimeFormat";
const std::string RemoteObject::ArrayMarker = "Array";                     // NOLINT (readability-identifier-naming)
const std::string RemoteObject::ContainerMarker = "Container";             // NOLINT (readability-identifier-naming)

std::unique_ptr<RemoteObject> RemoteObject::FromTagged(const EcmaVM *ecmaVm, Local<JSValueRef> tagged)
{
    if (tagged->IsNull() || tagged->IsUndefined() ||
        tagged->IsBoolean() || tagged->IsNumber() ||
        tagged->IsBigInt(ecmaVm)) {
        return std::make_unique<PrimitiveRemoteObject>(ecmaVm, tagged);
    }
    if (tagged->IsString(ecmaVm)) {
        return std::make_unique<StringRemoteObject>(ecmaVm, Local<StringRef>(tagged));
    }
    if (tagged->IsSymbol(ecmaVm)) {
        return std::make_unique<SymbolRemoteObject>(ecmaVm, Local<SymbolRef>(tagged));
    }
    // proxy must be placed in front of all object types
    if (tagged->IsProxy(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Proxy, ObjectSubType::Proxy);
    }
    if (tagged->IsGeneratorFunction(ecmaVm)) {
        return std::make_unique<GeneratorFunctionRemoteObject>(ecmaVm, tagged);
    }
    if (tagged->IsFunction(ecmaVm)) {
        return std::make_unique<FunctionRemoteObject>(ecmaVm, tagged);
    }
    if (tagged->IsArray(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Array, ObjectSubType::Array);
    }
    if (tagged->IsRegExp(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Regexp, ObjectSubType::Regexp);
    }
    if (tagged->IsDate(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Date, ObjectSubType::Date);
    }
    if (tagged->IsMap(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Map, ObjectSubType::Map);
    }
    if (tagged->IsWeakMap(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Weakmap, ObjectSubType::Weakmap);
    }
    if (tagged->IsSet(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Set, ObjectSubType::Set);
    }
    if (tagged->IsWeakSet(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Weakset, ObjectSubType::Weakset);
    }
    if (tagged->IsDataView(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Dataview, ObjectSubType::Dataview);
    }
    if (tagged->IsError(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Error, ObjectSubType::Error);
    }
    if (tagged->IsPromise(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Promise, ObjectSubType::Promise);
    }
    if (tagged->IsArrayBuffer(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Arraybuffer,
            ObjectSubType::Arraybuffer);
    }
    if (tagged->IsArrayIterator(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::ArrayIterator);
    }
    if (tagged->IsStringIterator(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::StringIterator);
    }
    if (tagged->IsSetIterator(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::SetIterator,
            ObjectSubType::Iterator);
    }
    if (tagged->IsMapIterator(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::MapIterator,
            ObjectSubType::Iterator);
    }
    if (tagged->IsArrayList(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsDeque(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsHashMap(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsHashSet(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsLightWeightMap(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsLightWeightSet(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsLinkedList(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsList(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsPlainArray(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsQueue(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsStack(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsTreeMap(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsTreeSet(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsVector(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsObject(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    if (tagged->IsNativePointer(ecmaVm)) {
        return std::make_unique<ObjectRemoteObject>(ecmaVm, tagged, ObjectClassName::Object);
    }
    std::unique_ptr<RemoteObject> object = std::make_unique<RemoteObject>();
    object->SetType(ObjectType::Undefined);
    return object;
}

void RemoteObject::AppendingHashToDescription(const EcmaVM *ecmaVm, Local<JSValueRef> tagged,
    std::string &description)
{
    if (ecmaVm->GetJsDebuggerManager() != nullptr && ecmaVm->GetJsDebuggerManager()->IsObjHashDisplayEnabled()) {
        JSHandle<JSTaggedValue> valueHandle = JSNApiHelper::ToJSHandle(tagged);
        int32_t hash = DebuggerApi::GetObjectHash(ecmaVm, valueHandle);
        if (hash != 0) {
            std::stringstream stringstream;
            stringstream << std::hex << hash;
            description += "@" + stringstream.str();
        }
    }
}

void RemoteObject::AppendingSendableDescription(Local<JSValueRef> tagged, std::string &description)
{
    if (JSNApiHelper::ToJSTaggedValue(*tagged).IsInSharedHeap()) {
        description += " [Sendable]";
    }
}

std::string RemoteObject::ResolveClassNameToDescription(const EcmaVM *ecmaVm, Local<JSValueRef> tagged)
{
    std::string description = RemoteObject::ObjectDescription;
    if (!tagged->IsObject(ecmaVm)) {
        return description;
    }
    DebuggerApi::GetObjectClassName(ecmaVm, tagged, description);
    return description.empty() ? RemoteObject::ObjectDescription : description;
}

PrimitiveRemoteObject::PrimitiveRemoteObject(const EcmaVM *ecmaVm, Local<JSValueRef> tagged)
{
    if (tagged->IsNull()) {
        SetType(ObjectType::Object)
            .SetSubType(ObjectSubType::Null)
            .SetPreviewValue(ObjectSubType::Null);
    } else if (tagged->IsBoolean()) {
        std::string description = tagged->IsTrue() ? "true" : "false";
        SetType(ObjectType::Boolean)
            .SetValue(tagged)
            .SetPreviewValue(description)
            .SetUnserializableValue(description)
            .SetDescription(description);
    } else if (tagged->IsUndefined()) {
        SetType(ObjectType::Undefined).SetPreviewValue(ObjectType::Undefined);
    } else if (tagged->IsNumber()) {
        std::string description = tagged->ToString(ecmaVm)->ToString(ecmaVm);
        SetType(ObjectType::Number)
            .SetValue(tagged)
            .SetPreviewValue(description)
            .SetUnserializableValue(description)
            .SetDescription(description);
    } else if (tagged->IsBigInt(ecmaVm)) {
        std::string description = tagged->ToString(ecmaVm)->ToString(ecmaVm) + "n";  // n : BigInt literal postfix
        SetType(ObjectType::Bigint)
            .SetValue(tagged)
            .SetPreviewValue(description)
            .SetUnserializableValue(description)
            .SetDescription(description);
    }
}

StringRemoteObject::StringRemoteObject([[maybe_unused]] const EcmaVM *ecmaVm, Local<StringRef> tagged)
{
    std::string description = tagged->DebuggerToString(ecmaVm);
    SetType(RemoteObject::TypeName::String)
        .SetValue(tagged)
        .SetPreviewValue(description)
        .SetUnserializableValue(description)
        .SetDescription(description);
}

SymbolRemoteObject::SymbolRemoteObject(const EcmaVM *ecmaVm, Local<SymbolRef> tagged)
{
    std::string description = DescriptionForSymbol(ecmaVm, tagged);
    SetPreviewValue(description);
    AppendingHashToDescription(ecmaVm, tagged, description);
    AppendingSendableDescription(tagged, description);
    SetType(RemoteObject::TypeName::Symbol)
        .SetValue(tagged)
        .SetUnserializableValue(description)
        .SetDescription(description);
}

FunctionRemoteObject::FunctionRemoteObject(const EcmaVM *ecmaVm, Local<JSValueRef> tagged)
{
    std::string description = DescriptionForFunction(ecmaVm, tagged);
    AppendingHashToDescription(ecmaVm, tagged, description);
    AppendingSendableDescription(tagged, description);
    SetType(RemoteObject::TypeName::Function)
        .SetClassName(RemoteObject::ClassName::Function)
        .SetValue(tagged)
        .SetPreviewValue(RemoteObject::ClassName::Function)
        .SetUnserializableValue(description)
        .SetDescription(description);
}

GeneratorFunctionRemoteObject::GeneratorFunctionRemoteObject(const EcmaVM *ecmaVm, Local<JSValueRef> tagged)
{
    std::string description = DescriptionForGeneratorFunction(ecmaVm, tagged);
    AppendingHashToDescription(ecmaVm, tagged, description);
    AppendingSendableDescription(tagged, description);
    SetType(RemoteObject::TypeName::Function)
        .SetClassName(RemoteObject::ClassName::Generator)
        .SetValue(tagged)
        .SetPreviewValue(RemoteObject::ClassName::Generator)
        .SetUnserializableValue(description)
        .SetDescription(description);
}

ObjectRemoteObject::ObjectRemoteObject(const EcmaVM *ecmaVm, Local<JSValueRef> tagged,
                                       const std::string &classname)
{
    std::string arrayOrContainer;
    std::string description = DescriptionForObject(ecmaVm, tagged, arrayOrContainer);
    SetPreviewValue(description);
    AppendingHashToDescription(ecmaVm, tagged, description);
    AppendingSendableDescription(tagged, description);
    SetType(RemoteObject::TypeName::Object)
        .SetClassName(classname)
        .SetValue(tagged)
        .SetUnserializableValue(description)
        .SetDescription(description)
        .SetArrayOrContainer(arrayOrContainer);
}

ObjectRemoteObject::ObjectRemoteObject(const EcmaVM *ecmaVm, Local<JSValueRef> tagged,
                                       const std::string &classname, const std::string &subtype)
{
    std::string arrayOrContainer;
    std::string description = DescriptionForObject(ecmaVm, tagged, arrayOrContainer);
    SetPreviewValue(description);
    AppendingHashToDescription(ecmaVm, tagged, description);
    AppendingSendableDescription(tagged, description);
    SetType(RemoteObject::TypeName::Object)
        .SetSubType(subtype)
        .SetClassName(classname)
        .SetValue(tagged)
        .SetUnserializableValue(description)
        .SetDescription(description)
        .SetArrayOrContainer(arrayOrContainer);
}

std::string ObjectRemoteObject::DescriptionForObject(const EcmaVM *ecmaVm, Local<JSValueRef> tagged,
    std::string &arrayOrContainer)
{
    // proxy must be placed in front of all object types
    if (tagged->IsProxy(ecmaVm)) {
        return RemoteObject::ProxyDescription;
    }
    if (tagged->IsArray(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForArray(ecmaVm, Local<ArrayRef>(tagged));
    }
    if (tagged->IsRegExp(ecmaVm)) {
        return DescriptionForRegexp(ecmaVm, Local<RegExpRef>(tagged));
    }
    if (tagged->IsDate(ecmaVm)) {
        return DescriptionForDate(ecmaVm, Local<DateRef>(tagged));
    }
    if (tagged->IsMap(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForMap(ecmaVm, Local<MapRef>(tagged));
    }
    if (tagged->IsWeakMap(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForWeakMap(ecmaVm, Local<WeakMapRef>(tagged));
    }
    if (tagged->IsSet(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForSet(ecmaVm, Local<SetRef>(tagged));
    }
    if (tagged->IsWeakSet(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForWeakSet(ecmaVm, Local<WeakSetRef>(tagged));
    }
    if (tagged->IsDataView(ecmaVm)) {
        return DescriptionForDataView(Local<DataViewRef>(tagged));
    }
    if (tagged->IsError(ecmaVm)) {
        return DescriptionForError(ecmaVm, tagged);
    }
    if (tagged->IsPromise(ecmaVm)) {
        return RemoteObject::PromiseDescription;
    }
    if (tagged->IsArrayIterator(ecmaVm)) {
        return DescriptionForArrayIterator();
    }
    if (tagged->IsStringIterator(ecmaVm)) {
        return RemoteObject::StringIteratorDescription;
    }
    if (tagged->IsSetIterator(ecmaVm)) {
        return DescriptionForSetIterator();
    }
    if (tagged->IsMapIterator(ecmaVm)) {
        return DescriptionForMapIterator();
    }
    if (tagged->IsArrayBuffer(ecmaVm)) {
        return DescriptionForArrayBuffer(ecmaVm, Local<ArrayBufferRef>(tagged));
    }
    if (tagged->IsSharedArrayBuffer(ecmaVm)) {
        return DescriptionForSharedArrayBuffer(ecmaVm, Local<ArrayBufferRef>(tagged));
    }
    if (tagged->IsUint8Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForUint8Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsUint8ClampedArray(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForUint8ClampedArray(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsInt8Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForInt8Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsUint16Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForUint16Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsInt16Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForInt16Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsUint32Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForUint32Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsInt32Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForInt32Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsFloat32Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForFloat32Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsBigInt64Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForBigInt64Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsBigUint64Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForBigUint64Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsFloat64Array(ecmaVm)) {
        arrayOrContainer = RemoteObject::ArrayMarker;
        return DescriptionForFloat64Array(ecmaVm, Local<TypedArrayRef>(tagged));
    }
    if (tagged->IsJSPrimitiveRef(ecmaVm) && tagged->IsJSPrimitiveNumber(ecmaVm)) {
        return DescriptionForPrimitiveNumber(ecmaVm, tagged);
    }
    if (tagged->IsJSPrimitiveRef(ecmaVm) && tagged->IsJSPrimitiveString(ecmaVm)) {
        return DescriptionForPrimitiveString(ecmaVm, tagged);
    }
    if (tagged->IsJSPrimitiveRef(ecmaVm) && tagged->IsJSPrimitiveBoolean(ecmaVm)) {
        return DescriptionForPrimitiveBoolean(ecmaVm, tagged);
    }
    if (tagged->IsGeneratorObject(ecmaVm)) {
        return DescriptionForGeneratorObject(ecmaVm, tagged);
    }
    if (tagged->IsWeakRef(ecmaVm)) {
        return DescriptionForWeakRef();
    }
    if (tagged->IsJSLocale(ecmaVm)) {
        return DescriptionForJSLocale();
    }
    if (tagged->IsJSDateTimeFormat(ecmaVm)) {
        return DescriptionForDateTimeFormat();
    }
    if (tagged->IsJSRelativeTimeFormat(ecmaVm)) {
        return DescriptionForJSRelativeTimeFormat();
    }
    if (tagged->IsJSIntl(ecmaVm)) {
        return RemoteObject::JSIntlDescription;
    }
    if (tagged->IsJSNumberFormat(ecmaVm)) {
        return DescriptionForNumberFormat();
    }
    if (tagged->IsJSCollator(ecmaVm)) {
        return DescriptionForCollator();
    }
    if (tagged->IsJSPluralRules(ecmaVm)) {
        return DescriptionForPluralRules();
    }
    if (tagged->IsJSListFormat(ecmaVm)) {
        return DescriptionForJSListFormat();
    }
    if (tagged->IsArrayList(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForArrayList();
    }
    if (tagged->IsDeque(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForDeque();
    }
    if (tagged->IsHashMap(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForHashMap();
    }
    if (tagged->IsHashSet(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForHashSet();
    }
    if (tagged->IsLightWeightMap(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForLightWeightMap();
    }
    if (tagged->IsLightWeightSet(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForLightWeightSet();
    }
    if (tagged->IsLinkedList(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForLinkedList();
    }
    if (tagged->IsList(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForList();
    }
    if (tagged->IsPlainArray(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForPlainArray();
    }
    if (tagged->IsQueue(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForQueue();
    }
    if (tagged->IsStack(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForStack();
    }
    if (tagged->IsTreeMap(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForTreeMap();
    }
    if (tagged->IsTreeSet(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForTreeSet();
    }
    if (tagged->IsVector(ecmaVm)) {
        arrayOrContainer = RemoteObject::ContainerMarker;
        return DescriptionForVector();
    }
    if (tagged->IsNativePointer(ecmaVm)) {
        return DescriptionForNativePointer(Local<NativePointerRef>(tagged));
    }
    return ResolveClassNameToDescription(ecmaVm, tagged);
}

std::string ObjectRemoteObject::DescriptionForNativePointer(const Local<NativePointerRef> &tagged)
{
    std::stringstream address;
    address << std::hex << tagged->Value();
    std::string description = "[External: " + address.str() + "]";
    return description;
}

std::string ObjectRemoteObject::DescriptionForArray(const EcmaVM *ecmaVm, Local<ArrayRef> tagged)
{
    std::string description = "Array(" + std::to_string(tagged->Length(ecmaVm)) + ")";
    return description;
}

std::string ObjectRemoteObject::DescriptionForRegexp(const EcmaVM *ecmaVm, Local<RegExpRef> tagged)
{
    std::string regExpSource = tagged->GetOriginalSource(ecmaVm)->DebuggerToString(ecmaVm);
    std::string regExpFlags = tagged->GetOriginalFlags(ecmaVm);
    return "/" + regExpSource + "/" + regExpFlags;
}

std::string ObjectRemoteObject::DescriptionForDate(const EcmaVM *ecmaVm, Local<DateRef> tagged)
{
    std::string description = tagged->ToString(ecmaVm)->ToString(ecmaVm);
    return description;
}

std::string ObjectRemoteObject::DescriptionForMap(const EcmaVM *ecmaVm, Local<MapRef> tagged)
{
    int32_t len = tagged->GetTotalElements(ecmaVm);
    int32_t index = 0;
    std::string description = "Map(" + std::to_string(tagged->GetSize(ecmaVm)) + ")";
    if (!len) {
        return description;
    }
    description += " {";
    char cPre = '\'';
    for (int32_t i = 0; i < len; ++i) {
        // add Key
        Local<JSValueRef> jsVKey = tagged->GetKey(ecmaVm, i);
        if (jsVKey->IsHole()) {
            continue;
        }

        Local<JSValueRef> jsVValue = tagged->GetValue(ecmaVm, i);
        if (jsVKey->IsObject(ecmaVm)) {
            description += "Object";
        } else if (jsVKey->IsString(ecmaVm)) {
            description += cPre + jsVKey->ToString(ecmaVm)->DebuggerToString(ecmaVm) + cPre;
        } else {
            description += jsVKey->ToString(ecmaVm)->DebuggerToString(ecmaVm);
        }

        description += " => ";
        // add Value
        if (jsVValue->IsObject(ecmaVm)) {
            description += "Object";
        } else if (jsVValue->IsString(ecmaVm)) {
            description += cPre + jsVValue->ToString(ecmaVm)->DebuggerToString(ecmaVm) + cPre;
        } else {
            description += jsVValue->ToString(ecmaVm)->DebuggerToString(ecmaVm);
        }
        if (index == tagged->GetSize(ecmaVm) - 1 || index >= 4) { // 4:The count of elements
            description += tagged->GetSize(ecmaVm) > 5 ? ", ..." : ""; // 5:The count of elements
            break;
        }
        description += ", ";
        index++;
    }
    description += "}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForWeakMap(const EcmaVM *ecmaVm, Local<WeakMapRef> tagged)
{
    int32_t len = tagged->GetTotalElements(ecmaVm);
    int32_t index = 0;
    std::string description = "WeakMap(" + std::to_string(tagged->GetSize(ecmaVm)) + ")";
    if (!len) {
        return description;
    }
    description += " {";
    char cPre = '\'';
    for (int32_t i = 0; i < len; ++i) {
        Local<JSValueRef> jsVKey = tagged->GetKey(ecmaVm, i);
        if (jsVKey->IsHole()) {
            continue;
        }
        Local<JSValueRef> jsVValue = tagged->GetValue(ecmaVm, i);
        if (jsVKey->IsObject(ecmaVm)) {
            description += "Object";
        } else if (jsVKey->IsString(ecmaVm)) {
            description += cPre + jsVKey->ToString(ecmaVm)->DebuggerToString(ecmaVm) + cPre;
        } else {
            description += jsVKey->ToString(ecmaVm)->DebuggerToString(ecmaVm);
        }

        description += " => ";

        if (jsVValue->IsObject(ecmaVm)) {
            description += "Object";
        } else if (jsVValue->IsString(ecmaVm)) {
            description += cPre + jsVValue->ToString(ecmaVm)->DebuggerToString(ecmaVm) + cPre;
        } else {
            description += jsVValue->ToString(ecmaVm)->DebuggerToString(ecmaVm);
        }
        if (index == tagged->GetSize(ecmaVm) - 1 || index >= 4) { // 4:The count of elements
            description += tagged->GetSize(ecmaVm) > 5 ? ", ..." : ""; // 5:The count of elements
            break;
        }
        description += ", ";
        index++;
    }
    description += "}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForSet(const EcmaVM *ecmaVm, Local<SetRef> tagged)
{
    int32_t len = tagged->GetTotalElements(ecmaVm);
    int32_t index = 0;
    std::string description = ("Set(" + std::to_string(tagged->GetSize(ecmaVm)) + ")");
    if (!len) {
        return description;
    }
    description += " {";
    char cPre = '\'';
    for (int32_t i = 0; i < len; ++i) {
        // add Key
        Local<JSValueRef> jsValue = tagged->GetValue(ecmaVm, i);
        if (jsValue->IsHole()) {
            continue;
        }
        // add Value
        if (jsValue->IsObject(ecmaVm)) {
            description += "Object";
        } else if (jsValue->IsString(ecmaVm)) {
            description += cPre + jsValue->ToString(ecmaVm)->DebuggerToString(ecmaVm) + cPre;
        } else {
            description += jsValue->ToString(ecmaVm)->DebuggerToString(ecmaVm);
        }
        if (index == tagged->GetSize(ecmaVm) - 1 || index >= 4) { // 4:The count of elements
            description += tagged->GetSize(ecmaVm) > 5 ? ", ..." : ""; // 5:The count of elements
            break;
        }
        description += ", ";
        index++;
    }
    description += "}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForWeakSet(const EcmaVM *ecmaVm, Local<WeakSetRef> tagged)
{
    int32_t len = tagged->GetTotalElements(ecmaVm);
    int32_t index = 0;
    std::string description = ("WeakSet(" + std::to_string(tagged->GetSize(ecmaVm)) + ")");
    if (!len) {
        return description;
    }
    description += " {";
    char cPre = '\'';
    for (int32_t i = 0; i < len; ++i) {
        Local<JSValueRef> jsValue = tagged->GetValue(ecmaVm, i);
        if (jsValue->IsHole()) {
            continue;
        }
        if (jsValue->IsObject(ecmaVm)) {
            description += "Object";
        } else if (jsValue->IsString(ecmaVm)) {
            description += cPre + jsValue->ToString(ecmaVm)->DebuggerToString(ecmaVm) + cPre;
        } else {
            description += jsValue->ToString(ecmaVm)->DebuggerToString(ecmaVm);
        }
        if (index == tagged->GetSize(ecmaVm) - 1 || index >= 4) { // 4:The count of elements
            description += tagged->GetSize(ecmaVm) > 5 ? ", ..." : ""; // 5:The count of elements
            break;
        }
        description += ", ";
        index++;
    }
    description += "}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForDataView(Local<DataViewRef> tagged)
{
    std::string description = ("DataView(" + std::to_string(tagged->ByteLength()) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForError(const EcmaVM *ecmaVm, Local<JSValueRef> tagged)
{
    // add name
    Local<JSValueRef> name = StringRef::NewFromUtf8(ecmaVm, "name");
    std::string strName = Local<ObjectRef>(tagged)->Get(ecmaVm, name)->ToString(ecmaVm)->ToString(ecmaVm);
    // add message
    Local<JSValueRef> message = StringRef::NewFromUtf8(ecmaVm, "message");
    std::string strMessage = Local<ObjectRef>(tagged)->Get(ecmaVm, message)->ToString(ecmaVm)->DebuggerToString(ecmaVm);
    if (strMessage.empty()) {
        return strName;
    } else {
        return strName + ": " + strMessage;
    }
}

std::string ObjectRemoteObject::DescriptionForArrayIterator()
{
    std::string description = RemoteObject::ArrayIteratorDescription + "{}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForSetIterator()
{
    std::string description = RemoteObject::SetIteratorDescription + "{}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForMapIterator()
{
    std::string description = RemoteObject::MapIteratorDescription + "{}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForArrayBuffer(const EcmaVM *ecmaVm, Local<ArrayBufferRef> tagged)
{
    int32_t len = tagged->ByteLength(ecmaVm);
    std::string description = ObjectClassName::Arraybuffer + "(" + std::to_string(len) + ")";
    return description;
}

std::string ObjectRemoteObject::DescriptionForSharedArrayBuffer(const EcmaVM *ecmaVm, Local<ArrayBufferRef> tagged)
{
    int32_t len = tagged->ByteLength(ecmaVm);
    std::string description = ("SharedArrayBuffer(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForUint8Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm);
    std::string description = ("Uint8Array(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForUint8ClampedArray(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm);
    std::string description = ("Uint8ClampedArray(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForInt8Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm);
    std::string description = ("Int8Array(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForUint16Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_16BITS;
    std::string description = ("Uint16Array(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForInt16Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_16BITS;
    std::string description = ("Int16Array(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForInt32Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_32BITS;
    std::string description = ("Int32Array(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForUint32Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_32BITS;
    std::string description = ("Uint32Array(" + std::to_string(len) + ")");
    return description;
}
 
std::string ObjectRemoteObject::DescriptionForFloat32Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_32BITS;
    std::string description = ("Float32Array(" + std::to_string(len) + ")");
    return description;
}
 
std::string ObjectRemoteObject::DescriptionForBigInt64Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_64BITS;
    std::string description = ("BigInt64Array(" + std::to_string(len) + ")");
    return description;
}
 
std::string ObjectRemoteObject::DescriptionForBigUint64Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_64BITS;
    std::string description = ("BigUint64Array(" + std::to_string(len) + ")");
    return description;
}
 
std::string ObjectRemoteObject::DescriptionForFloat64Array(const EcmaVM *ecmaVm, Local<TypedArrayRef> tagged)
{
    uint32_t len = tagged->ByteLength(ecmaVm) / NumberSize::BYTES_OF_64BITS;
    std::string description = ("Float64Array(" + std::to_string(len) + ")");
    return description;
}

std::string ObjectRemoteObject::DescriptionForPrimitiveNumber(const EcmaVM *ecmaVm, const Local<JSValueRef> &tagged)
{
    std::string strValue = tagged->ToString(ecmaVm)->ToString(ecmaVm);
    std::string description = RemoteObject::JSPrimitiveNumberDescription + "{[[PrimitiveValue]]: " + strValue + "}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForPrimitiveString(const EcmaVM *ecmaVm, const Local<JSValueRef> &tagged)
{
    std::string strValue = tagged->ToString(ecmaVm)->DebuggerToString(ecmaVm);
    std::string description = RemoteObject::JSPrimitiveStringDescription + "{[[PrimitiveValue]]: " + strValue + "}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForPrimitiveBoolean(const EcmaVM *ecmaVm, const Local<JSValueRef> &tagged)
{
    std::string strValue = tagged->ToString(ecmaVm)->ToString(ecmaVm);
    std::string description = RemoteObject::JSPrimitiveBooleanDescription + "{[[PrimitiveValue]]: " + strValue + "}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForGeneratorObject(const EcmaVM *ecmaVm, const Local<JSValueRef> &tagged)
{
    Local<GeneratorObjectRef> genObjectRef = tagged->ToObject(ecmaVm);
    // add Status
    Local<JSValueRef> jsValueRef = genObjectRef->GetGeneratorState(ecmaVm);
    std::string strState = genObjectRef->GetGeneratorState(ecmaVm)->ToString(ecmaVm)->ToString(ecmaVm);
    // add FuncName
    jsValueRef = genObjectRef->GetGeneratorFunction(ecmaVm);
    Local<JSValueRef> name = StringRef::NewFromUtf8(ecmaVm, "name");
    std::string strFuncName = Local<ObjectRef>(jsValueRef)->Get(ecmaVm, name)->ToString(ecmaVm)->ToString(ecmaVm);

    std::string description = strFuncName + " {<" + strState + ">}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForWeakRef()
{
    std::string description = RemoteObject::WeakRefDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForDateTimeFormat()
{
    std::string description = RemoteObject::DateTimeFormatDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForNumberFormat()
{
    std::string description = RemoteObject::NumberFormatDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForCollator()
{
    std::string description = RemoteObject::CollatorDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForPluralRules()
{
    std::string description = RemoteObject::PluralRulesDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForJSLocale()
{
    std::string description = RemoteObject::JSLocaleDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForJSRelativeTimeFormat()
{
    std::string description = RemoteObject::JSRelativeTimeFormatDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForJSListFormat()
{
    std::string description = RemoteObject::JSListFormatDescription + " {}";
    return description;
}

std::string ObjectRemoteObject::DescriptionForArrayList()
{
    std::string description = "ArrayList";
    return description;
}

std::string ObjectRemoteObject::DescriptionForDeque()
{
    std::string description = "Deque";
    return description;
}

std::string ObjectRemoteObject::DescriptionForHashMap()
{
    std::string description = "HashMap";
    return description;
}

std::string ObjectRemoteObject::DescriptionForHashSet()
{
    std::string description = "HashSet";
    return description;
}

std::string ObjectRemoteObject::DescriptionForLightWeightMap()
{
    std::string description = "LightWeightMap";
    return description;
}

std::string ObjectRemoteObject::DescriptionForLightWeightSet()
{
    std::string description = "LightWeightSet";
    return description;
}

std::string ObjectRemoteObject::DescriptionForLinkedList()
{
    std::string description = "LinkedList";
    return description;
}

std::string ObjectRemoteObject::DescriptionForList()
{
    std::string description = "List";
    return description;
}

std::string ObjectRemoteObject::DescriptionForPlainArray()
{
    std::string description = "PlainArray";
    return description;
}

std::string ObjectRemoteObject::DescriptionForQueue()
{
    std::string description = "Queue";
    return description;
}

std::string ObjectRemoteObject::DescriptionForStack()
{
    std::string description = "Stack";
    return description;
}

std::string ObjectRemoteObject::DescriptionForTreeMap()
{
    std::string description = "TreeMap";
    return description;
}

std::string ObjectRemoteObject::DescriptionForTreeSet()
{
    std::string description = "TreeSet";
    return description;
}

std::string ObjectRemoteObject::DescriptionForVector()
{
    std::string description = "Vector";
    return description;
}

std::string SymbolRemoteObject::DescriptionForSymbol(const EcmaVM *ecmaVm, Local<SymbolRef> tagged) const
{
    std::string description = "Symbol(" + tagged->GetDescription(ecmaVm)->DebuggerToString(ecmaVm) + ")";
    return description;
}

std::string FunctionRemoteObject::DescriptionForFunction(const EcmaVM *ecmaVm, Local<FunctionRef> tagged) const
{
    std::string sourceCode;
    if (tagged->IsNative(ecmaVm)) {
        sourceCode = "[native code]";
    } else {
        sourceCode = "[js code]";
    }
    Local<StringRef> name = tagged->GetName(ecmaVm);
    std::string description = "function " + name->ToString(ecmaVm) + "( { " + sourceCode + " }";
    return description;
}

std::string GeneratorFunctionRemoteObject::DescriptionForGeneratorFunction(const EcmaVM *ecmaVm,
    Local<FunctionRef> tagged) const
{
    std::string sourceCode;
    if (tagged->IsNative(ecmaVm)) {
        sourceCode = "[native code]";
    } else {
        sourceCode = "[js code]";
    }
    Local<StringRef> name = tagged->GetName(ecmaVm);
    std::string description = "function* " + name->ToString(ecmaVm) + "( { " + sourceCode + " }";
    return description;
}

std::unique_ptr<RemoteObject> RemoteObject::Create(const PtJson &params)
{
    std::string error;
    auto remoteObject = std::make_unique<RemoteObject>();
    Result ret;

    std::string type;
    ret = params.GetString("type", &type);
    if (ret == Result::SUCCESS) {
        if (ObjectType::Valid(type)) {
            remoteObject->type_ = std::move(type);
        } else {
            error += "'type' is invalid;";
        }
    } else {
        error += "Unknown or wrong type of 'type';";
    }

    std::string subType;
    ret = params.GetString("subtype", &subType);
    if (ret == Result::SUCCESS) {
        if (ObjectSubType::Valid(subType)) {
            remoteObject->subType_ = std::move(subType);
        } else {
            error += "'subtype' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'subtype';";
    }

    std::string className;
    ret = params.GetString("className", &className);
    if (ret == Result::SUCCESS) {
        remoteObject->className_ = std::move(className);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'className';";
    }

    std::string unserializableValue;
    ret = params.GetString("unserializableValue", &unserializableValue);
    if (ret == Result::SUCCESS) {
        remoteObject->unserializableValue_ = std::move(unserializableValue);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'unserializableValue';";
    }

    std::string description;
    ret = params.GetString("description", &description);
    if (ret == Result::SUCCESS) {
        remoteObject->description_ = std::move(description);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'description';";
    }

    std::string objectId;
    ret = params.GetString("objectId", &objectId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(objectId, remoteObject->objectId_)) {
            error += "Failed to convert 'objectId' from string to int;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'objectId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "RemoteObject::Create " << error;
        return nullptr;
    }

    return remoteObject;
}

std::unique_ptr<PtJson> RemoteObject::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("type", type_.c_str());
    if (subType_) {
        result->Add("subtype", subType_->c_str());
    }
    if (className_) {
        result->Add("className", className_->c_str());
    }
    if (unserializableValue_) {
        result->Add("unserializableValue", unserializableValue_->c_str());
    }
    if (description_) {
        result->Add("description", description_->c_str());
    }
    if (objectId_) {
        result->Add("objectId", std::to_string(objectId_.value()).c_str());
    }
    if (arrayOrContainer_ && !(arrayOrContainer_.value().empty())) {
        result->Add("arrayOrContainer", arrayOrContainer_->c_str());
    }

    return result;
}

std::unique_ptr<ExceptionDetails> ExceptionDetails::Create(const PtJson &params)
{
    std::string error;
    auto exceptionDetails = std::make_unique<ExceptionDetails>();
    Result ret;

    int32_t exceptionId;
    ret = params.GetInt("exceptionId", &exceptionId);
    if (ret == Result::SUCCESS) {
        exceptionDetails->exceptionId_ = exceptionId;
    } else {
        error += "Unknown or wrong type of 'exceptionId';";
    }

    std::string text;
    ret = params.GetString("text", &text);
    if (ret == Result::SUCCESS) {
        exceptionDetails->text_ = std::move(text);
    } else {
        error += "Unknown or wrong type of 'text';";
    }

    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        exceptionDetails->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }

    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        exceptionDetails->columnNumber_ = columnNumber;
    } else {
        error += "Unknown or wrong type of 'columnNumber';";
    }

    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(scriptId, exceptionDetails->scriptId_)) {
            error += "Failed to convert 'scriptId' from string to int;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'scriptId';";
    }

    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        exceptionDetails->url_ = std::move(url);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'url';";
    }

    std::unique_ptr<PtJson> exception;
    ret = params.GetObject("exception", &exception);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<RemoteObject> obj = RemoteObject::Create(*exception);
        if (obj == nullptr) {
            error += "'exception' format error;";
        } else {
            exceptionDetails->exception_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'exception';";
    }

    int32_t executionContextId;
    ret = params.GetInt("executionContextId", &executionContextId);
    if (ret == Result::SUCCESS) {
        exceptionDetails->executionContextId_ = executionContextId;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'executionContextId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ExceptionDetails::Create " << error;
        return nullptr;
    }

    return exceptionDetails;
}

std::unique_ptr<PtJson> ExceptionDetails::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("exceptionId", exceptionId_);
    result->Add("text", text_.c_str());
    result->Add("lineNumber", lineNumber_);
    result->Add("columnNumber", columnNumber_);

    if (scriptId_) {
        result->Add("scriptId", std::to_string(scriptId_.value()).c_str());
    }
    if (url_) {
        result->Add("url", url_->c_str());
    }
    if (exception_) {
        ASSERT(exception_.value() != nullptr);
        result->Add("exception", exception_.value()->ToJson());
    }
    if (executionContextId_) {
        result->Add("executionContextId", executionContextId_.value());
    }

    return result;
}

std::unique_ptr<InternalPropertyDescriptor> InternalPropertyDescriptor::Create(const PtJson &params)
{
    std::string error;
    auto internalPropertyDescriptor = std::make_unique<InternalPropertyDescriptor>();
    Result ret;

    std::string name;
    ret = params.GetString("name", &name);
    if (ret == Result::SUCCESS) {
        internalPropertyDescriptor->name_ = std::move(name);
    } else {
        error += "Unknown or wrong type of 'name';";
    }

    std::unique_ptr<PtJson> value;
    ret = params.GetObject("value", &value);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<RemoteObject> obj = RemoteObject::Create(*value);
        if (obj == nullptr) {
            error += "'value' format error;";
        } else {
            internalPropertyDescriptor->value_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'value';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "InternalPropertyDescriptor::Create " << error;
        return nullptr;
    }

    return internalPropertyDescriptor;
}

std::unique_ptr<PtJson> InternalPropertyDescriptor::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("name", name_.c_str());
    if (value_) {
        ASSERT(value_.value() != nullptr);
        result->Add("value", value_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<PrivatePropertyDescriptor> PrivatePropertyDescriptor::Create(const PtJson &params)
{
    std::string error;
    auto privatePropertyDescriptor = std::make_unique<PrivatePropertyDescriptor>();
    Result ret;

    std::string name;
    ret = params.GetString("name", &name);
    if (ret == Result::SUCCESS) {
        privatePropertyDescriptor->name_ = std::move(name);
    } else {
        error += "Unknown or wrong type of 'name';";
    }

    std::unique_ptr<PtJson> value;
    ret = params.GetObject("value", &value);
    std::unique_ptr<RemoteObject> obj;
    if (ret == Result::SUCCESS) {
        obj = RemoteObject::Create(*value);
        if (obj == nullptr) {
            error += "'value' format error;";
        } else {
            privatePropertyDescriptor->value_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'value';";
    }

    std::unique_ptr<PtJson> get;
    ret = params.GetObject("get", &get);
    if (ret == Result::SUCCESS) {
        obj = RemoteObject::Create(*get);
        if (obj == nullptr) {
            error += "'get' format error;";
        } else {
            privatePropertyDescriptor->get_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'get';";
    }

    std::unique_ptr<PtJson> set;
    ret = params.GetObject("set", &set);
    if (ret == Result::SUCCESS) {
        obj = RemoteObject::Create(*set);
        if (obj == nullptr) {
            error += "'set' format error;";
        } else {
            privatePropertyDescriptor->set_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'set';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "PrivatePropertyDescriptor::Create " << error;
        return nullptr;
    }

    return privatePropertyDescriptor;
}

std::unique_ptr<PtJson> PrivatePropertyDescriptor::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("name", name_.c_str());
    if (value_) {
        ASSERT(value_.value() != nullptr);
        result->Add("value", value_.value()->ToJson());
    }
    if (get_) {
        ASSERT(get_.value() != nullptr);
        result->Add("get", get_.value()->ToJson());
    }
    if (set_) {
        ASSERT(set_.value() != nullptr);
        result->Add("set", set_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<PropertyDescriptor> PropertyDescriptor::FromProperty(const EcmaVM *ecmaVm,
    Local<JSValueRef> name, const PropertyAttribute &property)
{
    std::unique_ptr<PropertyDescriptor> debuggerProperty = std::make_unique<PropertyDescriptor>();

    std::string nameStr;
    if (name->IsSymbol(ecmaVm)) {
        Local<SymbolRef> symbol(name);
        nameStr = "Symbol(" + Local<SymbolRef>(name)->GetDescription(ecmaVm)->DebuggerToString(ecmaVm) + ")";
        debuggerProperty->symbol_ = RemoteObject::FromTagged(ecmaVm, name);
    } else {
        nameStr = name->ToString(ecmaVm)->ToString(ecmaVm);
    }

    debuggerProperty->name_ = nameStr;
    if (property.HasValue()) {
        auto propValue = property.GetValue(ecmaVm);
        if (propValue->IsFunction(ecmaVm) && nameStr.empty()) {
            // if the value is a function and missing function name,
            // fill the name of the debuggerProperty with <anonymous>
            debuggerProperty->name_ = ANONYMOUS_FUNCTION_NAME_PLACEHOLDER;
        }
        debuggerProperty->value_ = RemoteObject::FromTagged(ecmaVm, propValue);
    }
    if (property.HasWritable()) {
        debuggerProperty->writable_ = property.IsWritable();
    }
    if (property.HasGetter()) {
        debuggerProperty->get_ = RemoteObject::FromTagged(ecmaVm, property.GetGetter(ecmaVm));
    }
    if (property.HasSetter()) {
        debuggerProperty->set_ = RemoteObject::FromTagged(ecmaVm, property.GetSetter(ecmaVm));
    }
    debuggerProperty->configurable_ = property.IsConfigurable();
    debuggerProperty->enumerable_ = property.IsEnumerable();
    debuggerProperty->isOwn_ = true;

    return debuggerProperty;
}

std::unique_ptr<PropertyDescriptor> PropertyDescriptor::Create(const PtJson &params)
{
    std::string error;
    auto propertyDescriptor = std::make_unique<PropertyDescriptor>();
    Result ret;

    std::string name;
    ret = params.GetString("name", &name);
    if (ret == Result::SUCCESS) {
        propertyDescriptor->name_ = std::move(name);
    } else {
        error += "Unknown or wrong type of 'name';";
    }

    std::unique_ptr<PtJson> value;
    std::unique_ptr<RemoteObject> obj;
    ret = params.GetObject("value", &value);
    if (ret == Result::SUCCESS) {
        obj = RemoteObject::Create(*value);
        if (obj == nullptr) {
            error += "'value' format error;";
        } else {
            propertyDescriptor->value_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'value';";
    }

    bool writable = false;
    ret = params.GetBool("writable", &writable);
    if (ret == Result::SUCCESS) {
        propertyDescriptor->writable_ = writable;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'writable';";
    }

    std::unique_ptr<PtJson> get;
    ret = params.GetObject("get", &get);
    if (ret == Result::SUCCESS) {
        obj = RemoteObject::Create(*get);
        if (obj == nullptr) {
            error += "'get' format error;";
        } else {
            propertyDescriptor->get_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'get';";
    }

    std::unique_ptr<PtJson> set;
    ret = params.GetObject("set", &set);
    if (ret == Result::SUCCESS) {
        obj = RemoteObject::Create(*set);
        if (obj == nullptr) {
            error += "'set' format error;";
        } else {
            propertyDescriptor->set_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'set';";
    }

    bool configurable = false;
    ret = params.GetBool("configurable", &configurable);
    if (ret == Result::SUCCESS) {
        propertyDescriptor->configurable_ = configurable;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'configurable';";
    }

    bool enumerable = false;
    ret = params.GetBool("enumerable", &enumerable);
    if (ret == Result::SUCCESS) {
        propertyDescriptor->enumerable_ = enumerable;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'enumerable';";
    }

    bool wasThrown = false;
    ret = params.GetBool("wasThrown", &wasThrown);
    if (ret == Result::SUCCESS) {
        propertyDescriptor->wasThrown_ = wasThrown;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'wasThrown';";
    }

    bool isOwn = false;
    ret = params.GetBool("isOwn", &isOwn);
    if (ret == Result::SUCCESS) {
        propertyDescriptor->isOwn_ = isOwn;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'isOwn';";
    }

    std::unique_ptr<PtJson> symbol;
    ret = params.GetObject("symbol", &symbol);
    if (ret == Result::SUCCESS) {
        obj = RemoteObject::Create(*symbol);
        if (obj == nullptr) {
            error += "'symbol' format error;";
        } else {
            propertyDescriptor->symbol_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'symbol';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "PropertyDescriptor::Create " << error;
        return nullptr;
    }

    return propertyDescriptor;
}

std::unique_ptr<PtJson> PropertyDescriptor::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("name", name_.c_str());
    if (value_) {
        ASSERT(value_.value() != nullptr);
        result->Add("value", value_.value()->ToJson());
    }
    if (writable_) {
        result->Add("writable", writable_.value());
    }
    if (get_) {
        ASSERT(get_.value() != nullptr);
        result->Add("get", get_.value()->ToJson());
    }
    if (set_) {
        ASSERT(set_.value() != nullptr);
        result->Add("set", set_.value()->ToJson());
    }
    result->Add("configurable", configurable_);
    result->Add("enumerable", enumerable_);
    if (wasThrown_) {
        result->Add("wasThrown", wasThrown_.value());
    }
    if (isOwn_) {
        result->Add("isOwn", isOwn_.value());
    }
    if (symbol_) {
        ASSERT(symbol_.value() != nullptr);
        result->Add("symbol", symbol_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<CallArgument> CallArgument::Create(const PtJson &params)
{
    auto callArgument = std::make_unique<CallArgument>();
    std::string error;
    Result ret;

    std::string unserializableValue;
    ret = params.GetString("unserializableValue", &unserializableValue);
    if (ret == Result::SUCCESS) {
        callArgument->unserializableValue_ = std::move(unserializableValue);
    } else if (ret == Result::TYPE_ERROR) {  // optional value
        error += "Wrong type of 'unserializableValue';";
    }
    std::string objectId;
    ret = params.GetString("objectId", &objectId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(objectId, callArgument->objectId_)) {
            error += "Failed to convert 'objectId' from string to int;";
        }
    } else if (ret == Result::TYPE_ERROR) {  // optional value
        error += "Wrong type of 'objectId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "CallArgument::Create " << error;
        return nullptr;
    }

    return callArgument;
}

std::unique_ptr<PtJson> CallArgument::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    if (unserializableValue_) {
        result->Add("unserializableValue", unserializableValue_->c_str());
    }
    if (objectId_) {
        result->Add("objectId", std::to_string(objectId_.value()).c_str());
    }

    return result;
}

std::unique_ptr<Location> Location::Create(const PtJson &params)
{
    auto location = std::make_unique<Location>();
    std::string error;
    Result ret;

    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(scriptId, location->scriptId_)) {
            error += "Failed to convert 'scriptId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'scriptId';";
    }
    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        location->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }
    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        location->columnNumber_ = columnNumber;
    } else if (ret == Result::TYPE_ERROR) {  // optional value
        error += "Wrong type of 'columnNumber';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "Location::Create " << error;
        return nullptr;
    }

    return location;
}

std::unique_ptr<PtJson> Location::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptId", std::to_string(scriptId_).c_str());
    result->Add("lineNumber", lineNumber_);
    if (columnNumber_) {
        result->Add("columnNumber", columnNumber_.value());
    }

    return result;
}

std::unique_ptr<ScriptPosition> ScriptPosition::Create(const PtJson &params)
{
    auto scriptPosition = std::make_unique<ScriptPosition>();
    std::string error;
    Result ret;

    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        scriptPosition->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }
    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        scriptPosition->columnNumber_ = columnNumber;
    } else {
        error += "Unknown or wrong type of 'columnNumber';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ScriptPosition::Create " << error;
        return nullptr;
    }

    return scriptPosition;
}

std::unique_ptr<PtJson> ScriptPosition::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("lineNumber", lineNumber_);
    result->Add("columnNumber", columnNumber_);

    return result;
}

std::unique_ptr<SearchMatch> SearchMatch::Create(const PtJson &params)
{
    std::string error;
    auto locationSearch = std::make_unique<SearchMatch>();
    Result ret;

    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        locationSearch->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }

    std::string lineContent;
    ret = params.GetString("lineContent", &lineContent);
    if (ret == Result::SUCCESS) {
        locationSearch->lineContent_ = std::move(lineContent);
    } else {
        error += "Unknown or wrong type of 'lineContent';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SearchMatch::Create " << error;
        return nullptr;
    }

    return locationSearch;
}

std::unique_ptr<PtJson> SearchMatch::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("lineNumber", lineNumber_);
    result->Add("lineContent", lineContent_.c_str());

    return result;
}

std::unique_ptr<LocationRange> LocationRange::Create(const PtJson &params)
{
    std::string error;
    auto locationRange = std::make_unique<LocationRange>();
    Result ret;

    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(scriptId, locationRange->scriptId_)) {
            error += "Failed to convert 'scriptId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'scriptId';";
    }

    std::unique_ptr<PtJson> start;
    std::unique_ptr<ScriptPosition> obj;
    ret = params.GetObject("start", &start);
    if (ret == Result::SUCCESS) {
        obj = ScriptPosition::Create(*start);
        if (obj == nullptr) {
            error += "'start' format error;";
        } else {
            locationRange->start_ = std::move(obj);
        }
    } else {
        error += "Unknown or wrong type of 'start';";
    }

    std::unique_ptr<PtJson> end;
    ret = params.GetObject("end", &end);
    if (ret == Result::SUCCESS) {
        obj = ScriptPosition::Create(*end);
        if (obj == nullptr) {
            error += "'end' format error;";
        } else {
            locationRange->end_ = std::move(obj);
        }
    } else {
        error += "Unknown or wrong type of 'end';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "LocationRange::Create " << error;
        return nullptr;
    }

    return locationRange;
}

std::unique_ptr<PtJson> LocationRange::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptId", std::to_string(scriptId_).c_str());
    ASSERT(start_ != nullptr);
    result->Add("start", start_->ToJson());
    ASSERT(end_ != nullptr);
    result->Add("end", end_->ToJson());

    return result;
}

std::unique_ptr<NativeRange> NativeRange::Create(const PtJson &params)
{
    std::string error;
    auto nativeRange = std::make_unique<NativeRange>();
    Result ret;

    uint64_t start;
    ret = params.GetUInt64("start", &start);
    if (ret == Result::SUCCESS) {
        nativeRange->start_ = std::move(start);
    } else {
        error += "Unknown 'start';";
    }

    uint64_t end;
    ret = params.GetUInt64("end", &end);
    if (ret == Result::SUCCESS) {
        nativeRange->end_ = std::move(end);
    } else {
        error += "Unknown 'end';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "NativeRange::Create " << error;
        return nullptr;
    }

    return nativeRange;
}

std::unique_ptr<BreakLocation> BreakLocation::Create(const PtJson &params)
{
    std::string error;
    auto breakLocation = std::make_unique<BreakLocation>();
    Result ret;

    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(scriptId, breakLocation->scriptId_)) {
            error += "Failed to convert 'scriptId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'scriptId';";
    }

    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        breakLocation->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }

    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        breakLocation->columnNumber_ = columnNumber;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'columnNumber';";
    }

    std::string type;
    ret = params.GetString("type", &type);
    if (ret == Result::SUCCESS) {
        if (BreakType::Valid(type)) {
            breakLocation->type_ = std::move(type);
        } else {
            error += "'type' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "type 'scriptId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "Location::Create " << error;
        return nullptr;
    }

    return breakLocation;
}

std::unique_ptr<PtJson> BreakLocation::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptId", std::to_string(scriptId_).c_str());
    result->Add("lineNumber", lineNumber_);
    if (columnNumber_) {
        result->Add("columnNumber", columnNumber_.value());
    }
    if (type_) {
        result->Add("type", type_->c_str());
    }

    return result;
}

std::unique_ptr<Scope> Scope::Create(const PtJson &params)
{
    std::string error;
    auto scope = std::make_unique<Scope>();
    Result ret;

    std::string type;
    ret = params.GetString("type", &type);
    if (ret == Result::SUCCESS) {
        if (Scope::Type::Valid(type)) {
            scope->type_ = std::move(type);
        } else {
            error += "'type' is invalid;";
        }
    } else {
        error += "Unknown or wrong type of 'type';";
    }

    std::unique_ptr<PtJson> object;
    std::unique_ptr<RemoteObject> remoteObject;
    ret = params.GetObject("object", &object);
    if (ret == Result::SUCCESS) {
        remoteObject = RemoteObject::Create(*object);
        if (remoteObject == nullptr) {
            error += "'object' format error;";
        } else {
            scope->object_ = std::move(remoteObject);
        }
    } else {
        error += "Unknown or wrong type of 'object';";
    }

    std::string name;
    ret = params.GetString("name", &name);
    if (ret == Result::SUCCESS) {
        scope->name_ = std::move(name);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'name';";
    }

    std::unique_ptr<PtJson> startLocation;
    std::unique_ptr<Location> obj;
    ret = params.GetObject("startLocation", &startLocation);
    if (ret == Result::SUCCESS) {
        obj = Location::Create(*startLocation);
        if (obj == nullptr) {
            error += "'startLocation' format error;";
        } else {
            scope->startLocation_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'startLocation';";
    }

    std::unique_ptr<PtJson> endLocation;
    ret = params.GetObject("endLocation", &endLocation);
    if (ret == Result::SUCCESS) {
        obj = Location::Create(*endLocation);
        if (obj == nullptr) {
            error += "'endLocation' format error;";
        } else {
            scope->endLocation_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'endLocation';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "Location::Create " << error;
        return nullptr;
    }

    return scope;
}

std::unique_ptr<PtJson> Scope::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("type", type_.c_str());
    ASSERT(object_ != nullptr);
    result->Add("object", object_->ToJson());
    if (name_) {
        result->Add("name", name_->c_str());
    }
    if (startLocation_) {
        ASSERT(startLocation_.value() != nullptr);
        result->Add("startLocation", startLocation_.value()->ToJson());
    }
    if (endLocation_) {
        ASSERT(endLocation_.value() != nullptr);
        result->Add("endLocation", endLocation_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<CallFrame> CallFrame::Create(const PtJson &params)
{
    std::string error;
    auto callFrame = std::make_unique<CallFrame>();
    Result ret;

    std::string callFrameId;
    ret = params.GetString("callFrameId", &callFrameId);
    if (ret == Result::SUCCESS) {
        if (!ToolchainUtils::StrToInt32(callFrameId, callFrame->callFrameId_)) {
            error += "Failed to convert 'callFrameId' from string to int;";
        }
    } else {
        error += "Unknown or wrong type of 'callFrameId';";
    }

    std::string functionName;
    ret = params.GetString("functionName", &functionName);
    if (ret == Result::SUCCESS) {
        callFrame->functionName_ = std::move(functionName);
    } else {
        error += "Unknown or wrong type of 'functionName';";
    }

    std::unique_ptr<PtJson> functionLocation;
    std::unique_ptr<Location> obj;
    ret = params.GetObject("functionLocation", &functionLocation);
    if (ret == Result::SUCCESS) {
        obj = Location::Create(*functionLocation);
        if (obj == nullptr) {
            error += "'functionLocation' format error;";
        } else {
            callFrame->functionLocation_ = std::move(obj);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'functionLocation';";
    }

    std::unique_ptr<PtJson> location;
    ret = params.GetObject("location", &location);
    if (ret == Result::SUCCESS) {
        obj = Location::Create(*location);
        if (obj == nullptr) {
            error += "'location' format error;";
        } else {
            callFrame->location_ = std::move(obj);
        }
    } else {
        error += "Unknown or wrong type of 'location';";
    }

    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        callFrame->url_ = std::move(url);
    } else {
        error += "Unknown or wrong type of 'url';";
    }

    std::unique_ptr<PtJson> scopeChain;
    ret = params.GetArray("scopeChain", &scopeChain);
    if (ret == Result::SUCCESS) {
        int32_t len = scopeChain->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> arrayEle = scopeChain->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<Scope> scope = Scope::Create(*arrayEle);
            if (scope == nullptr) {
                error += "'scopeChain' format error;";
            } else {
                callFrame->scopeChain_.emplace_back(std::move(scope));
            }
        }
    } else {
        error += "Unknown or wrong type of 'scopeChain';";
    }

    std::unique_ptr<PtJson> thisObj;
    std::unique_ptr<RemoteObject> remoteObject;
    ret = params.GetObject("this", &thisObj);
    if (ret == Result::SUCCESS) {
        remoteObject = RemoteObject::Create(*thisObj);
        if (remoteObject == nullptr) {
            error += "'this' format error;";
        } else {
            callFrame->this_ = std::move(remoteObject);
        }
    } else {
        error += "Unknown or wrong type of 'this';";
    }

    std::unique_ptr<PtJson> returnValue;
    ret = params.GetObject("returnValue", &returnValue);
    if (ret == Result::SUCCESS) {
        remoteObject = RemoteObject::Create(*returnValue);
        if (remoteObject == nullptr) {
            error += "'returnValue' format error;";
        } else {
            callFrame->returnValue_ = std::move(remoteObject);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'returnValue';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "CallFrame::Create " << error;
        return nullptr;
    }

    return callFrame;
}

std::unique_ptr<PtJson> CallFrame::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("callFrameId", std::to_string(callFrameId_).c_str());
    result->Add("functionName", functionName_.c_str());

    if (functionLocation_) {
        ASSERT(functionLocation_.value() != nullptr);
        result->Add("functionLocation", functionLocation_.value()->ToJson());
    }
    ASSERT(location_ != nullptr);
    result->Add("location", location_->ToJson());
    result->Add("url", url_.c_str());

    size_t len = scopeChain_.size();
    std::unique_ptr<PtJson> values = PtJson::CreateArray();
    for (size_t i = 0; i < len; i++) {
        ASSERT(scopeChain_[i] != nullptr);
        std::unique_ptr<PtJson> scope = scopeChain_[i]->ToJson();
        values->Push(scope);
    }
    result->Add("scopeChain", values);

    ASSERT(this_ != nullptr);
    result->Add("this", this_->ToJson());
    if (returnValue_) {
        ASSERT(returnValue_.value() != nullptr);
        result->Add("returnValue", returnValue_.value()->ToJson());
    }

    return result;
}

std::unique_ptr<SamplingHeapProfileSample> SamplingHeapProfileSample::Create(const PtJson &params)
{
    std::string error;
    auto samplingHeapProfileSample = std::make_unique<SamplingHeapProfileSample>();
    Result ret;

    int32_t size;
    ret = params.GetInt("size", &size);
    if (ret == Result::SUCCESS) {
        samplingHeapProfileSample->size_ = size;
    } else {
        error += "Unknown or wrong type of 'size';";
    }
    int32_t nodeId;
    ret = params.GetInt("nodeId", &nodeId);
    if (ret == Result::SUCCESS) {
        samplingHeapProfileSample->nodeId_ = nodeId;
    } else {
        error += "Unknown or wrong type of 'nodeId';";
    }
    int32_t ordinal;
    ret = params.GetInt("ordinal", &ordinal);
    if (ret == Result::SUCCESS) {
        samplingHeapProfileSample->ordinal_ = ordinal;
    } else {
        error += "Unknown or wrong type of 'ordinal';";
    }
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SamplingHeapProfileSample::Create " << error;
        return nullptr;
    }

    return samplingHeapProfileSample;
}

std::unique_ptr<PtJson> SamplingHeapProfileSample::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("size", size_);
    result->Add("nodeId", nodeId_);
    result->Add("ordinal", ordinal_);

    return result;
}

std::unique_ptr<RuntimeCallFrame> RuntimeCallFrame::Create(const PtJson &params)
{
    std::string error;
    auto runtimeCallFrame = std::make_unique<RuntimeCallFrame>();
    Result ret;
    
    std::string functionName;
    ret = params.GetString("functionName", &functionName);
    if (ret == Result::SUCCESS) {
        runtimeCallFrame->functionName_ = std::move(functionName);
    } else {
        error += "Unknown or wrong type of 'functionName';";
    }

    std::string moduleName;
    ret = params.GetString("moduleName", &moduleName);
    if (ret == Result::SUCCESS) {
        runtimeCallFrame->moduleName_ = std::move(moduleName);
    } else {
        error += "Unknown or wrong type of 'moduleName';";
    }

    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        runtimeCallFrame->scriptId_ = std::move(scriptId);
    } else {
        error += "Unknown or wrong type of 'scriptId';";
    }

    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        runtimeCallFrame->url_ = std::move(url);
    } else {
        error += "Unknown or wrong type of 'url';";
    }
    
    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        runtimeCallFrame->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }

    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        runtimeCallFrame->columnNumber_ = columnNumber;
    } else {
        error += "Unknown or wrong type of 'columnNumber';";
    }
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "RuntimeCallFrame::Create " << error;
        return nullptr;
    }

    return runtimeCallFrame;
}

std::unique_ptr<RuntimeCallFrame> RuntimeCallFrame::FromFrameInfo(const FrameInfo &cpuFrameInfo)
{
    auto runtimeCallFrame = std::make_unique<RuntimeCallFrame>();
    runtimeCallFrame->SetFunctionName(cpuFrameInfo.functionName);
    runtimeCallFrame->SetModuleName(cpuFrameInfo.moduleName);
    runtimeCallFrame->SetScriptId(std::to_string(cpuFrameInfo.scriptId));
    runtimeCallFrame->SetColumnNumber(cpuFrameInfo.columnNumber);
    runtimeCallFrame->SetLineNumber(cpuFrameInfo.lineNumber);
    runtimeCallFrame->SetUrl(cpuFrameInfo.url);
    return runtimeCallFrame;
}

std::unique_ptr<PtJson> RuntimeCallFrame::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("functionName", functionName_.c_str());
    result->Add("moduleName", moduleName_.c_str());
    result->Add("scriptId", scriptId_.c_str());
    result->Add("url", url_.c_str());
    result->Add("lineNumber", lineNumber_);
    result->Add("columnNumber", columnNumber_);

    return result;
}

std::unique_ptr<SamplingHeapProfileNode> SamplingHeapProfileNode::Create(const PtJson &params)
{
    std::string error;
    auto samplingHeapProfileNode = std::make_unique<SamplingHeapProfileNode>();
    Result ret;

    std::unique_ptr<PtJson> callFrame;
    ret = params.GetObject("callFrame", &callFrame);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<RuntimeCallFrame> runtimeCallFrame = RuntimeCallFrame::Create(*callFrame);
        if (runtimeCallFrame == nullptr) {
            error += "'callFrame' format invalid;";
        } else {
            samplingHeapProfileNode->callFrame_ = std::move(runtimeCallFrame);
        }
    } else {
        error += "Unknown or wrong type of 'callFrame';";
    }
    
    int32_t selfSize;
    ret = params.GetInt("selfSize", &selfSize);
    if (ret == Result::SUCCESS) {
        samplingHeapProfileNode->selfSize_ = selfSize;
    } else {
        error += "Unknown or wrong type of 'selfSize';";
    }

    int32_t id;
    ret = params.GetInt("id", &id);
    if (ret == Result::SUCCESS) {
        samplingHeapProfileNode->id_ = id;
    } else {
        error += "Unknown or wrong type of 'id';";
    }

    std::unique_ptr<PtJson> children;
    ret = params.GetArray("children", &children);
    if (ret == Result::SUCCESS) {
        int32_t len = children->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> arrayEle = children->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<SamplingHeapProfileNode> node = SamplingHeapProfileNode::Create(*arrayEle);
            if (node == nullptr) {
                error += "'children' format invalid;";
            } else {
                samplingHeapProfileNode->children_.emplace_back(std::move(node));
            }
        }
    } else {
        error += "Unknown or wrong type of 'children';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SamplingHeapProfileNode::Create " << error;
        return nullptr;
    }

    return samplingHeapProfileNode;
}

std::unique_ptr<PtJson> SamplingHeapProfileNode::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();
    ASSERT(callFrame_ != nullptr);
    result->Add("callFrame", callFrame_->ToJson());
    result->Add("selfSize", selfSize_);
    result->Add("id", id_);

    std::unique_ptr<PtJson> childrens = PtJson::CreateArray();
    size_t len = children_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(children_[i] != nullptr);
        childrens->Push(children_[i]->ToJson());
    }
    result->Add("children", childrens);

    return result;
}

std::unique_ptr<SamplingHeapProfile> SamplingHeapProfile::Create(const PtJson &params)
{
    std::string error;
    auto samplingHeapProfile = std::make_unique<SamplingHeapProfile>();
    Result ret;
    
    std::unique_ptr<PtJson> head;
    ret = params.GetObject("head", &head);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<SamplingHeapProfileNode> pHead = SamplingHeapProfileNode::Create(*head);
        if (pHead == nullptr) {
            error += "'sample' format invalid;";
        } else {
            samplingHeapProfile->head_ = std::move(pHead);
        }
    } else {
        error += "Unknown or wrong type of 'head';";
    }

    std::unique_ptr<PtJson> samples;
    ret = params.GetArray("samples", &samples);
    if (ret == Result::SUCCESS) {
        int32_t len = samples->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> arrayEle = samples->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<SamplingHeapProfileSample> pSample = SamplingHeapProfileSample::Create(*arrayEle);
            if (pSample == nullptr) {
                error += "'sample' format invalid;";
            } else {
                samplingHeapProfile->samples_.emplace_back(std::move(pSample));
            }
        }
    } else {
        error += "Unknown or wrong type of 'samples';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "SamplingHeapProfile::Create " << error;
        return nullptr;
    }

    return samplingHeapProfile;
}

std::unique_ptr<PtJson> SamplingHeapProfile::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    ASSERT(head_ != nullptr);
    result->Add("head", head_->ToJson());
    
    std::unique_ptr<PtJson> samples = PtJson::CreateArray();
    size_t len = samples_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(samples_[i] != nullptr);
        samples->Push(samples_[i]->ToJson());
    }
    result->Add("samples", samples);
    return result;
}

std::unique_ptr<SamplingHeapProfile> SamplingHeapProfile::FromSamplingInfo(
    const SamplingInfo *samplingInfo)
{
    std::unique_ptr<SamplingHeapProfile> profile = std::make_unique<SamplingHeapProfile>();
    auto node = TransferHead(&(samplingInfo->head_));
    profile->head_ = std::move(node);
    const CVector<struct Sample> &samples = samplingInfo->samples_;
    for (const auto &sample : samples) {
        std::unique_ptr<SamplingHeapProfileSample> sampleTemp = std::make_unique<SamplingHeapProfileSample>();
        sampleTemp->SetSize(sample.size_ * sample.count_);
        sampleTemp->SetNodeId(sample.nodeId_);
        sampleTemp->SetOrdinal(sample.ordinal_);
        profile->samples_.emplace_back(std::move(sampleTemp));
    }
    return profile;
}

std::unique_ptr<SamplingHeapProfileNode> SamplingHeapProfile::TransferHead(const SamplingNode *samplingNode)
{
    std::unique_ptr<SamplingHeapProfileNode> node = std::make_unique<SamplingHeapProfileNode>();
    node->SetSelfSize(samplingNode->selfSize_);
    node->SetId(samplingNode->id_);
    std::unique_ptr<RuntimeCallFrame> callFrame = std::make_unique<RuntimeCallFrame>();
    callFrame->SetFunctionName(samplingNode->callFrameInfo_.functionName_);
    callFrame->SetScriptId(std::to_string(samplingNode->callFrameInfo_.scriptId_));
    callFrame->SetUrl(samplingNode->callFrameInfo_.url_);
    callFrame->SetLineNumber(samplingNode->callFrameInfo_.lineNumber_);
    callFrame->SetColumnNumber(samplingNode->callFrameInfo_.columnNumber_);
    node->SetCallFrame(std::move(callFrame));
    std::vector<std::unique_ptr<SamplingHeapProfileNode>> profileNode;
    for (const auto &it : samplingNode->children_) {
        auto proNode = TransferHead(it.second.get());
        profileNode.push_back(std::move(proNode));
    }
    node->SetChildren(std::move(profileNode));
    return node;
}

std::unique_ptr<PositionTickInfo> PositionTickInfo::Create(const PtJson &params)
{
    std::string error;
    auto positionTickInfo = std::make_unique<PositionTickInfo>();
    Result ret;
    
    int32_t line;
    ret = params.GetInt("line", &line);
    if (ret == Result::SUCCESS) {
        positionTickInfo->line_ = line;
    } else {
        error += "Unknown or wrong type of 'line';";
    }

    int32_t ticks;
    ret = params.GetInt("ticks", &ticks);
    if (ret == Result::SUCCESS) {
        positionTickInfo->ticks_ = ticks;
    } else {
        error += "Unknown or wrong type of 'ticks';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "PositionTickInfo::Create " << error;
        return nullptr;
    }

    return positionTickInfo;
}

std::unique_ptr<PtJson> PositionTickInfo::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("line", line_);
    result->Add("ticks", ticks_);

    return result;
}

std::unique_ptr<ProfileNode> ProfileNode::Create(const PtJson &params)
{
    std::string error;
    auto profileNode = std::make_unique<ProfileNode>();
    Result ret;
    
    int32_t id;
    ret = params.GetInt("id", &id);
    if (ret == Result::SUCCESS) {
        profileNode->id_ = id;
    } else {
        error += "Unknown or wrong type of 'id';";
    }

    std::unique_ptr<PtJson> callFrame;
    ret = params.GetObject("callFrame", &callFrame);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<RuntimeCallFrame> runtimeCallFrame = RuntimeCallFrame::Create(*callFrame);
        if (runtimeCallFrame == nullptr) {
            error += "'callFrame' format invalid;";
        } else {
            profileNode->callFrame_ = std::move(runtimeCallFrame);
        }
    } else {
        error += "Unknown or wrong type of 'callFrame';";
    }

    int32_t hitCount;
    ret = params.GetInt("hitCount", &hitCount);
    if (ret == Result::SUCCESS) {
        profileNode->hitCount_ = hitCount;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'hitCount';";
    }

    std::unique_ptr<PtJson> children;
    ret = params.GetArray("children", &children);
    if (ret == Result::SUCCESS) {
        int32_t childrenLen = children->GetSize();
        for (int32_t i = 0; i < childrenLen; ++i) {
            int32_t pChildren = children->Get(i)->GetInt();
            profileNode->children_.value().emplace_back(pChildren);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'children';";
    }

    std::unique_ptr<PtJson> positionTicks;
    ret = params.GetArray("positionTicks", &positionTicks);
    if (ret == Result::SUCCESS) {
        int32_t positionTicksLen = positionTicks->GetSize();
        for (int32_t i = 0; i < positionTicksLen; ++i) {
            std::unique_ptr<PtJson> arrayEle = positionTicks->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<PositionTickInfo> tmpPositionTicks = PositionTickInfo::Create(*arrayEle);
            if (tmpPositionTicks == nullptr) {
                error += "'positionTicks' format invalid;";
            } else {
                profileNode->positionTicks_.value().emplace_back(std::move(tmpPositionTicks));
            }
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'positionTicks';";
    }

    std::string deoptReason;
    ret = params.GetString("deoptReason", &deoptReason);
    if (ret == Result::SUCCESS) {
        profileNode->deoptReason_ = std::move(deoptReason);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'deoptReason';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ProfileNode::Create " << error;
        return nullptr;
    }

    return profileNode;
}

std::unique_ptr<ProfileNode> ProfileNode::FromCpuProfileNode(const CpuProfileNode &cpuProfileNode)
{
    auto profileNode = std::make_unique<ProfileNode>();
    profileNode->SetId(cpuProfileNode.id);
    profileNode->SetHitCount(cpuProfileNode.hitCount);

    size_t childrenLen = cpuProfileNode.children.size();
    std::vector<int32_t> tmpChildren;
    tmpChildren.reserve(childrenLen);
    for (size_t i = 0; i < childrenLen; ++i) {
        tmpChildren.push_back(cpuProfileNode.children[i]);
    }
    profileNode->SetChildren(tmpChildren);
    profileNode->SetCallFrame(RuntimeCallFrame::FromFrameInfo(cpuProfileNode.codeEntry));
    return profileNode;
}

std::unique_ptr<PtJson> ProfileNode::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("id", id_);
    ASSERT(callFrame_ != nullptr);
    result->Add("callFrame", callFrame_->ToJson());
    if (hitCount_) {
        result->Add("hitCount", hitCount_.value());
    }
    if (children_) {
        std::unique_ptr<PtJson> childrens = PtJson::CreateArray();
        size_t len = children_->size();
        for (size_t i = 0; i < len; i++) {
            childrens->Push(children_.value()[i]);
        }
        result->Add("children", childrens);
    }
    if (positionTicks_) {
        std::unique_ptr<PtJson> positionTicks = PtJson::CreateArray();
        size_t len = positionTicks_->size();
        for (size_t i = 0; i < len; i++) {
            ASSERT(positionTicks_.value()[i] != nullptr);
            positionTicks->Push(positionTicks_.value()[i]->ToJson());
        }
        result->Add("positionTicks", positionTicks);
    }

    if (deoptReason_) {
        result->Add("deoptReason", deoptReason_.value().c_str());
    }
    
    return result;
}

std::unique_ptr<Profile> Profile::Create(const PtJson &params)
{
    std::string error;
    auto profile = std::make_unique<Profile>();
    Result ret;
    
    std::unique_ptr<PtJson> nodes;
    ret = params.GetArray("nodes", &nodes);
    if (ret == Result::SUCCESS) {
        int32_t nodesLen = nodes->GetSize();
        for (int32_t i = 0; i < nodesLen; ++i) {
            std::unique_ptr<PtJson> arrayEle = nodes->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<ProfileNode> profileNode = ProfileNode::Create(*arrayEle);
            if (profileNode == nullptr) {
                error += "'nodes' format invalid;";
            } else {
                profile->nodes_.emplace_back(std::move(profileNode));
            }
        }
    } else {
        error += "Unknown or wrong type of 'nodes';";
    }

    int64_t tid;
    ret = params.GetInt64("tid", &tid);
    if (ret == Result::SUCCESS) {
        profile->tid_ = tid;
    } else {
        error += "Unknown or wrong type of 'tid';";
    }

    int64_t startTime;
    ret = params.GetInt64("startTime", &startTime);
    if (ret == Result::SUCCESS) {
        profile->startTime_ = startTime;
    } else {
        error += "Unknown or wrong type of 'startTime';";
    }

    int64_t endTime;
    ret = params.GetInt64("endTime", &endTime);
    if (ret == Result::SUCCESS) {
        profile->endTime_ = endTime;
    } else {
        error += "Unknown or wrong type of 'endTime';";
    }

    int64_t gcTime;
    ret = params.GetInt64("gcTime", &gcTime);
    if (ret == Result::SUCCESS) {
        profile->gcTime_ = gcTime;
    } else {
        error += "Unknown or wrong type of 'gcTime';";
    }

    int64_t cInterpreterTime;
    ret = params.GetInt64("cInterpreterTime", &cInterpreterTime);
    if (ret == Result::SUCCESS) {
        profile->cInterpreterTime_ = cInterpreterTime;
    } else {
        error += "Unknown or wrong type of 'cInterpreterTime';";
    }

    int64_t asmInterpreterTime;
    ret = params.GetInt64("asmInterpreterTime", &asmInterpreterTime);
    if (ret == Result::SUCCESS) {
        profile->asmInterpreterTime_ = asmInterpreterTime;
    } else {
        error += "Unknown or wrong type of 'asmInterpreterTime';";
    }

    int64_t aotTime;
    ret = params.GetInt64("aotTime", &aotTime);
    if (ret == Result::SUCCESS) {
        profile->aotTime_ = aotTime;
    } else {
        error += "Unknown or wrong type of 'aotTime';";
    }

    int64_t builtinTime;
    ret = params.GetInt64("builtinTime", &builtinTime);
    if (ret == Result::SUCCESS) {
        profile->builtinTime_ = builtinTime;
    } else {
        error += "Unknown or wrong type of 'builtinTime';";
    }

    int64_t napiTime;
    ret = params.GetInt64("napiTime", &napiTime);
    if (ret == Result::SUCCESS) {
        profile->napiTime_ = napiTime;
    } else {
        error += "Unknown or wrong type of 'napiTime';";
    }

    int64_t arkuiEngineTime;
    ret = params.GetInt64("arkuiEngineTime", &arkuiEngineTime);
    if (ret == Result::SUCCESS) {
        profile->arkuiEngineTime_ = arkuiEngineTime;
    } else {
        error += "Unknown or wrong type of 'arkuiEngineTime';";
    }

    int64_t runtimeTime;
    ret = params.GetInt64("runtimeTime", &runtimeTime);
    if (ret == Result::SUCCESS) {
        profile->runtimeTime_ = runtimeTime;
    } else {
        error += "Unknown or wrong type of 'runtimeTime';";
    }

    int64_t otherTime;
    ret = params.GetInt64("otherTime", &otherTime);
    if (ret == Result::SUCCESS) {
        profile->otherTime_ = otherTime;
    } else {
        error += "Unknown or wrong type of 'otherTime';";
    }

    std::unique_ptr<PtJson> samples;
    ret = params.GetArray("samples", &samples);
    if (ret == Result::SUCCESS) {
        int32_t samplesLen = samples->GetSize();
        for (int32_t i = 0; i < samplesLen; ++i) {
            int32_t pSamples = samples->Get(i)->GetInt();
            profile->samples_.value().emplace_back(pSamples);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'samples';";
    }

    std::unique_ptr<PtJson> timeDeltas;
    ret = params.GetArray("timeDeltas", &timeDeltas);
    if (ret == Result::SUCCESS) {
        int32_t timeDeltasLen = timeDeltas->GetSize();
        for (int32_t i = 0; i < timeDeltasLen; ++i) {
            int32_t pTimeDeltas = timeDeltas->Get(i)->GetInt();
            profile->timeDeltas_.value().emplace_back(pTimeDeltas);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'timeDeltas';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "Profile::Create " << error;
        return nullptr;
    }

    return profile;
}

std::unique_ptr<Profile> Profile::FromProfileInfo(const ProfileInfo &profileInfo)
{
    auto profile = std::make_unique<Profile>();
    profile->SetTid(static_cast<int64_t>(profileInfo.tid));
    profile->SetStartTime(static_cast<int64_t>(profileInfo.startTime));
    profile->SetEndTime(static_cast<int64_t>(profileInfo.stopTime));
    profile->SetGcTime(static_cast<int64_t>(profileInfo.gcTime));
    profile->SetCInterpreterTime(static_cast<int64_t>(profileInfo.cInterpreterTime));
    profile->SetAsmInterpreterTime(static_cast<int64_t>(profileInfo.asmInterpreterTime));
    profile->SetAotTime(static_cast<int64_t>(profileInfo.aotTime));
    profile->SetBuiltinTime(static_cast<int64_t>(profileInfo.builtinTime));
    profile->SetNapiTime(static_cast<int64_t>(profileInfo.napiTime));
    profile->SetArkuiEngineTime(static_cast<int64_t>(profileInfo.arkuiEngineTime));
    profile->SetRuntimeTime(static_cast<int64_t>(profileInfo.runtimeTime));
    profile->SetOtherTime(static_cast<int64_t>(profileInfo.otherTime));
    size_t samplesLen = profileInfo.samples.size();
    std::vector<int32_t> tmpSamples;
    tmpSamples.reserve(samplesLen);
    for (size_t i = 0; i < samplesLen; ++i) {
        tmpSamples.push_back(profileInfo.samples[i]);
    }
    profile->SetSamples(tmpSamples);

    size_t timeDeltasLen = profileInfo.timeDeltas.size();
    std::vector<int32_t> tmpTimeDeltas;
    tmpTimeDeltas.reserve(timeDeltasLen);
    for (size_t i = 0; i < timeDeltasLen; ++i) {
        tmpTimeDeltas.push_back(profileInfo.timeDeltas[i]);
    }
    profile->SetTimeDeltas(tmpTimeDeltas);

    std::vector<std::unique_ptr<ProfileNode>> profileNode;
    size_t nodesLen = profileInfo.nodeCount;
    for (size_t i = 0; i < nodesLen; ++i) {
        const auto &cpuProfileNode = profileInfo.nodes[i];
        profileNode.push_back(ProfileNode::FromCpuProfileNode(cpuProfileNode));
    }
    profile->SetNodes(std::move(profileNode));
    return profile;
}

std::unique_ptr<PtJson> Profile::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("tid", tid_);
    result->Add("startTime", startTime_);
    result->Add("endTime", endTime_);
    result->Add("gcTime", gcTime_);
    result->Add("cInterpreterTime", cInterpreterTime_);
    result->Add("asmInterpreterTime", asmInterpreterTime_);
    result->Add("aotTime", aotTime_);
    result->Add("builtinTime", builtinTime_);
    result->Add("napiTime", napiTime_);
    result->Add("arkuiEngineTime", arkuiEngineTime_);
    result->Add("runtimeTime", runtimeTime_);
    result->Add("otherTime", otherTime_);

    std::unique_ptr<PtJson> nodes = PtJson::CreateArray();
    size_t nodesLen = nodes_.size();
    for (size_t i = 0; i < nodesLen; i++) {
        ASSERT(nodes_[i] != nullptr);
        nodes->Push(nodes_[i]->ToJson());
    }
    result->Add("nodes", nodes);

    if (samples_) {
        std::unique_ptr<PtJson> samples = PtJson::CreateArray();
        size_t samplesLen = samples_->size();
        for (size_t i = 0; i < samplesLen; i++) {
            samples->Push(samples_.value()[i]);
        }
        result->Add("samples", samples);
    }

    if (timeDeltas_) {
        std::unique_ptr<PtJson> timeDeltas = PtJson::CreateArray();
        size_t timeDeltasLen = timeDeltas_->size();
        for (size_t i = 0; i < timeDeltasLen; i++) {
            timeDeltas->Push(timeDeltas_.value()[i]);
        }
        result->Add("timeDeltas", timeDeltas);
    }
    
    return result;
}

std::unique_ptr<Coverage> Coverage::Create(const PtJson &params)
{
    std::string error;
    auto coverage = std::make_unique<Coverage>();
    Result ret;
    
    int32_t startOffset;
    ret = params.GetInt("startOffset", &startOffset);
    if (ret == Result::SUCCESS) {
        coverage->startOffset_ = startOffset;
    } else {
        error += "Unknown or wrong type of 'startOffset';";
    }

    int32_t endOffset;
    ret = params.GetInt("endOffset", &endOffset);
    if (ret == Result::SUCCESS) {
        coverage->endOffset_ = endOffset;
    } else {
        error += "Unknown or wrong type of 'endOffset';";
    }

    int32_t count;
    ret = params.GetInt("count", &count);
    if (ret == Result::SUCCESS) {
        coverage->count_ = count;
    } else {
        error += "Unknown or wrong type of 'count';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "Coverage::Create " << error;
        return nullptr;
    }

    return coverage;
}

std::unique_ptr<PtJson> Coverage::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("startOffset", startOffset_);
    result->Add("endOffset", endOffset_);
    result->Add("count", count_);
    
    return result;
}

std::unique_ptr<FunctionCoverage> FunctionCoverage::Create(const PtJson &params)
{
    std::string error;
    auto functionCoverage = std::make_unique<FunctionCoverage>();
    Result ret;
    
    std::string functionName;
    ret = params.GetString("functionName", &functionName);
    if (ret == Result::SUCCESS) {
        functionCoverage->functionName_ = std::move(functionName);
    } else {
        error += "Unknown or wrong type of 'functionName';";
    }

    std::unique_ptr<PtJson> ranges;
    ret = params.GetArray("ranges", &ranges);
    if (ret == Result::SUCCESS) {
        int32_t len = ranges->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> arrayEle = ranges->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<Coverage> pRanges = Coverage::Create(*arrayEle);
            if (pRanges == nullptr) {
                error += "'ranges' format invalid;";
            } else {
                functionCoverage->ranges_.emplace_back(std::move(pRanges));
            }
        }
    } else {
        error += "Unknown or wrong type of 'ranges';";
    }

    bool isBlockCoverage = false;
    ret = params.GetBool("isBlockCoverage", &isBlockCoverage);
    if (ret == Result::SUCCESS) {
        functionCoverage->isBlockCoverage_ = isBlockCoverage;
    } else {
        error += "Unknown or wrong type of 'isBlockCoverage';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "FunctionCoverage::Create " << error;
        return nullptr;
    }

    return functionCoverage;
}

std::unique_ptr<PtJson> FunctionCoverage::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("functionName", functionName_.c_str());

    std::unique_ptr<PtJson> ranges = PtJson::CreateArray();
    size_t len = ranges_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(ranges_[i] != nullptr);
        ranges->Push(ranges_[i]->ToJson());
    }
    result->Add("ranges", ranges);

    result->Add("isBlockCoverage", isBlockCoverage_);
    
    return result;
}

std::unique_ptr<ScriptCoverage> ScriptCoverage::Create(const PtJson &params)
{
    std::string error;
    auto scriptCoverage = std::make_unique<ScriptCoverage>();
    Result ret;
    
    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        scriptCoverage->scriptId_ = std::move(scriptId);
    } else {
        error += "Unknown or wrong type of 'scriptId';";
    }

    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        scriptCoverage->url_ = std::move(url);
    } else {
        error += "Unknown or wrong type of 'url';";
    }

    std::unique_ptr<PtJson> functions;
    ret = params.GetArray("functions", &functions);
    if (ret == Result::SUCCESS) {
        int32_t len = functions->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> arrayEle = functions->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<FunctionCoverage> pFunctions = FunctionCoverage::Create(*arrayEle);
            if (pFunctions == nullptr) {
                error += "'functions' format invalid;";
            } else {
                scriptCoverage->functions_.emplace_back(std::move(pFunctions));
            }
        }
    } else {
        error += "Unknown or wrong type of 'functions';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ScriptCoverage::Create " << error;
        return nullptr;
    }

    return scriptCoverage;
}

std::unique_ptr<PtJson> ScriptCoverage::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptId", scriptId_.c_str());
    result->Add("url", url_.c_str());

    std::unique_ptr<PtJson> functions = PtJson::CreateArray();
    size_t len = functions_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(functions_[i] != nullptr);
        functions->Push(functions_[i]->ToJson());
    }
    result->Add("functions", functions);
    
    return result;
}

std::unique_ptr<TypeObject> TypeObject::Create(const PtJson &params)
{
    std::string error;
    auto typeObject = std::make_unique<TypeObject>();
    Result ret;
    
    std::string name;
    ret = params.GetString("name", &name);
    if (ret == Result::SUCCESS) {
        typeObject->name_ = std::move(name);
    } else {
        error += "Unknown or wrong type of 'name';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "TypeObject::Create " << error;
        return nullptr;
    }

    return typeObject;
}

std::unique_ptr<PtJson> TypeObject::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("name", name_.c_str());
    
    return result;
}

std::unique_ptr<TypeProfileEntry> TypeProfileEntry::Create(const PtJson &params)
{
    std::string error;
    auto typeProfileEntry = std::make_unique<TypeProfileEntry>();
    Result ret;
    
    int32_t offset;
    ret = params.GetInt("offset", &offset);
    if (ret == Result::SUCCESS) {
        typeProfileEntry->offset_ = offset;
    } else {
        error += "Unknown or wrong type of 'offset';";
    }

    std::unique_ptr<PtJson> types;
    ret = params.GetArray("types", &types);
    if (ret == Result::SUCCESS) {
        int32_t len = types->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> arrayEle = types->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<TypeObject> pTypes = TypeObject::Create(*arrayEle);
            if (pTypes == nullptr) {
                error += "'types' format invalid;";
            } else {
                typeProfileEntry->types_.emplace_back(std::move(pTypes));
            }
        }
    } else {
        error += "Unknown or wrong type of 'types';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "TypeProfileEntry::Create " << error;
        return nullptr;
    }

    return typeProfileEntry;
}

std::unique_ptr<PtJson> TypeProfileEntry::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("offset", offset_);

    std::unique_ptr<PtJson> types = PtJson::CreateArray();
    size_t len = types_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(types_[i] != nullptr);
        types->Push(types_[i]->ToJson());
    }
    result->Add("types", types);

    return result;
}

std::unique_ptr<ScriptTypeProfile> ScriptTypeProfile::Create(const PtJson &params)
{
    std::string error;
    auto scriptTypeProfile = std::make_unique<ScriptTypeProfile>();
    Result ret;

    std::string scriptId;
    ret = params.GetString("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        scriptTypeProfile->scriptId_ = std::move(scriptId);
    } else {
        error += "Unknown or wrong type of 'scriptId';";
    }

    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        scriptTypeProfile->url_ = std::move(url);
    } else {
        error += "Unknown or wrong type of 'url';";
    }

    std::unique_ptr<PtJson> entries;
    ret = params.GetArray("entries", &entries);
    if (ret == Result::SUCCESS) {
        int32_t len = entries->GetSize();
        for (int32_t i = 0; i < len; ++i) {
            std::unique_ptr<PtJson> arrayEle = entries->Get(i);
            ASSERT(arrayEle != nullptr);
            std::unique_ptr<TypeProfileEntry> pEntries = TypeProfileEntry::Create(*arrayEle);
            if (pEntries == nullptr) {
                error += "'entries' format invalid;";
            } else {
                scriptTypeProfile->entries_.emplace_back(std::move(pEntries));
            }
        }
    } else {
        error += "Unknown or wrong type of 'entries';";
    }
    
    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "ScriptTypeProfile::Create " << error;
        return nullptr;
    }

    return scriptTypeProfile;
}

std::unique_ptr<PtJson> ScriptTypeProfile::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    result->Add("scriptId", scriptId_.c_str());
    result->Add("url", url_.c_str());

    std::unique_ptr<PtJson> entries = PtJson::CreateArray();
    size_t len = entries_.size();
    for (size_t i = 0; i < len; i++) {
        ASSERT(entries_[i] != nullptr);
        entries->Push(entries_[i]->ToJson());
    }
    result->Add("entries", entries);
    
    return result;
}

std::unique_ptr<TraceConfig> TraceConfig::Create(const PtJson &params)
{
    std::string error;
    auto traceConfig = std::make_unique<TraceConfig>();
    Result ret;

    std::string recordMode;
    ret = params.GetString("recordMode", &recordMode);
    if (ret == Result::SUCCESS) {
        if (TraceConfig::RecordModeValues::Valid(recordMode)) {
            traceConfig->recordMode_ = std::move(recordMode);
        } else {
            error += "'recordMode' is invalid;";
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'recordMode';";
    }

    bool enableSampling = false;
    ret = params.GetBool("enableSampling", &enableSampling);
    if (ret == Result::SUCCESS) {
        traceConfig->enableSampling_ = enableSampling;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'enableSampling';";
    }

    bool enableSystrace = false;
    ret = params.GetBool("enableSystrace", &enableSystrace);
    if (ret == Result::SUCCESS) {
        traceConfig->enableSystrace_ = enableSystrace;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'enableSystrace';";
    }

    bool enableArgumentFilter = false;
    ret = params.GetBool("enableArgumentFilter", &enableArgumentFilter);
    if (ret == Result::SUCCESS) {
        traceConfig->enableArgumentFilter_ = enableArgumentFilter;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'enableArgumentFilter';";
    }

    std::unique_ptr<PtJson> includedCategories;
    ret = params.GetArray("includedCategories", &includedCategories);
    if (ret == Result::SUCCESS) {
        int32_t includedCategoriesLen = includedCategories->GetSize();
        traceConfig->includedCategories_= std::vector<std::string>();
        for (int32_t i = 0; i < includedCategoriesLen; ++i) {
            std::string pIncludedCategories = includedCategories->Get(i)->GetString();
            traceConfig->includedCategories_.value().emplace_back(pIncludedCategories);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'includedCategories';";
    }

    std::unique_ptr<PtJson> excludedCategories;
    ret = params.GetArray("excludedCategories", &excludedCategories);
    if (ret == Result::SUCCESS) {
        int32_t excludedCategoriesLen = excludedCategories->GetSize();
        traceConfig->excludedCategories_ = std::vector<std::string>();
        for (int32_t i = 0; i < excludedCategoriesLen; ++i) {
            std::string pExcludedCategories = excludedCategories->Get(i)->GetString();
            traceConfig->excludedCategories_.value().emplace_back(pExcludedCategories);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'excludedCategories';";
    }

    std::unique_ptr<PtJson> syntheticDelays;
    ret = params.GetArray("syntheticDelays", &syntheticDelays);
    if (ret == Result::SUCCESS) {
        int32_t syntheticDelaysLen = syntheticDelays->GetSize();
        traceConfig->syntheticDelays_ = std::vector<std::string>();
        for (int32_t i = 0; i < syntheticDelaysLen; ++i) {
            std::string pSyntheticDelays = syntheticDelays->Get(i)->GetString();
            traceConfig->syntheticDelays_.value().emplace_back(pSyntheticDelays);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'syntheticDelays';";
    }

    std::unique_ptr<PtJson> memoryDumpConfig;
    ret = params.GetObject("memoryDumpConfig", &memoryDumpConfig);
    if (ret == Result::SUCCESS) {
        std::unique_ptr<MemoryDumpConfig> tmpMemory = std::move(memoryDumpConfig);
        if (tmpMemory == nullptr) {
            error += "'memoryDumpConfig' format error;";
        } else {
            traceConfig->memoryDumpConfig_ = std::move(tmpMemory);
        }
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'memoryDumpConfig';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "TraceConfig::Create " << error;
        return nullptr;
    }

    return traceConfig;
}

std::unique_ptr<PtJson> TraceConfig::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();

    if (recordMode_) {
        result->Add("recordMode", recordMode_.value().c_str());
    }

    if (enableSampling_) {
        result->Add("enableSampling", enableSampling_.value());
    }

    if (enableSystrace_) {
        result->Add("enableSystrace", enableSystrace_.value());
    }

    if (enableArgumentFilter_) {
        result->Add("enableArgumentFilter", enableArgumentFilter_.value());
    }

    if (includedCategories_) {
        std::unique_ptr<PtJson> includedCategories = PtJson::CreateArray();
        size_t includedCategoriesLen = includedCategories_->size();
        for (size_t i = 0; i < includedCategoriesLen; i++) {
            includedCategories->Push(includedCategories_.value()[i].c_str());
        }
        result->Add("includedCategories", includedCategories);
    }

    if (excludedCategories_) {
        std::unique_ptr<PtJson> excludedCategories = PtJson::CreateArray();
        size_t excludedCategoriesLen = excludedCategories_->size();
        for (size_t i = 0; i < excludedCategoriesLen; i++) {
            excludedCategories->Push(excludedCategories_.value()[i].c_str());
        }
        result->Add("excludedCategories", excludedCategories);
    }

    if (syntheticDelays_) {
        std::unique_ptr<PtJson> syntheticDelays = PtJson::CreateArray();
        size_t syntheticDelaysLen = syntheticDelays_->size();
        for (size_t i = 0; i < syntheticDelaysLen; i++) {
            syntheticDelays->Push(syntheticDelays_.value()[i].c_str());
        }
        result->Add("syntheticDelays", syntheticDelays);
    }

    if (memoryDumpConfig_) {
        result->Add("functionLocation", memoryDumpConfig_.value());
    }

    return result;
}

std::unique_ptr<BreakpointInfo> BreakpointInfo::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<BreakpointInfo>();
    std::string error;
    Result ret;
    // lineNumber
    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        paramsObject->lineNumber_ = lineNumber;
    } else {
        error += "Unknown or wrong type of 'lineNumber';";
    }
    // columnNumber
    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        paramsObject->columnNumber_ = columnNumber;
    } else {
        error += "Unknown or wrong type of 'columnNumber';";
    }
    // url
    std::string url;
    ret = params.GetString("url", &url);
    if (ret == Result::SUCCESS) {
        paramsObject->url_ = std::move(url);
    } else {
        error += "Unknown or wrong type of 'url';";
    }
    // condition
    std::string condition;
    ret = params.GetString("condition", &condition);
    if (ret == Result::SUCCESS) {
        paramsObject->condition_ = std::move(condition);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'condition';";
    }
    // urlRegex
    std::string urlRegex;
    ret = params.GetString("urlRegex", &urlRegex);
    if (ret == Result::SUCCESS) {
        paramsObject->urlRegex_ = std::move(urlRegex);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'urlRegex';";
    }
    // scriptHash
    std::string scriptHash;
    ret = params.GetString("scriptHash", &scriptHash);
    if (ret == Result::SUCCESS) {
        paramsObject->scriptHash_ = std::move(scriptHash);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'scriptHash';";
    }
    // restrictToFunction
    bool restrictToFunction = false;
    ret = params.GetBool("restrictToFunction", &restrictToFunction);
    if (ret == Result::SUCCESS) {
        paramsObject->restrictToFunction_ = restrictToFunction;
    } else if (ret == Result::TYPE_ERROR) {  // optional value
        error += "Wrong type of 'restrictToFunction';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "BreakpointInfo::Create " << error;
        return nullptr;
    }
    return paramsObject;
}

std::shared_ptr<BreakpointInfo> BreakpointInfo::CreateAsSharedPtr(int32_t line, int32_t column,
    std::string url, std::string condition)
{
    auto result = std::make_shared<BreakpointInfo>();
    result->lineNumber_ = line;
    result->columnNumber_ = column;
    result->url_ = url;
    result->condition_ = condition;

    return result;
}

std::unique_ptr<PtJson> BreakpointInfo::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();
    result->Add("lineNumber", lineNumber_);
    result->Add("columnNumber", columnNumber_);
    result->Add("url", url_.c_str());
    if (condition_) {
        result->Add("condition", condition_.value().c_str());
    }
    if (urlRegex_) {
        result->Add("urlRegex", urlRegex_.value().c_str());
    }
    if (scriptHash_) {
        result->Add("scriptHash", scriptHash_.value().c_str());
    }
    if (restrictToFunction_) {
        result->Add("restrictToFunction", restrictToFunction_.value());
    }

    return result;
}

std::unique_ptr<BreakpointReturnInfo> BreakpointReturnInfo::Create(const PtJson &params)
{
    auto paramsObject = std::make_unique<BreakpointReturnInfo>();
    std::string error;
    Result ret;
    // lineNumber
    int32_t lineNumber;
    ret = params.GetInt("lineNumber", &lineNumber);
    if (ret == Result::SUCCESS) {
        paramsObject->lineNumber_ = lineNumber;
    } else {
        error += "Wrong type of 'lineNumber';";
    }
    // columnNumber
    int32_t columnNumber;
    ret = params.GetInt("columnNumber", &columnNumber);
    if (ret == Result::SUCCESS) {
        paramsObject->columnNumber_ = columnNumber;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'columnNumber';";
    }
    // id
    std::string id;
    ret = params.GetString("id", &id);
    if (ret == Result::SUCCESS) {
        paramsObject->id_ = std::move(id);
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'id';";
    }
    // scriptId
    int32_t scriptId;
    ret = params.GetInt("scriptId", &scriptId);
    if (ret == Result::SUCCESS) {
        paramsObject->scriptId_ = scriptId;
    } else if (ret == Result::TYPE_ERROR) {
        error += "Wrong type of 'scriptId';";
    }

    if (!error.empty()) {
        LOG_DEBUGGER(ERROR) << "BreakpointReturnInfo::Create " << error;
        return nullptr;
    }
    
    return paramsObject;
}

std::unique_ptr<PtJson> BreakpointReturnInfo::ToJson() const
{
    std::unique_ptr<PtJson> result = PtJson::CreateObject();
    result->Add("lineNumber", lineNumber_);
    result->Add("columnNumber", columnNumber_);
    result->Add("id", id_.c_str());
    result->Add("scriptId", scriptId_);

    return result;
}
}  // namespace panda::ecmascript::tooling
