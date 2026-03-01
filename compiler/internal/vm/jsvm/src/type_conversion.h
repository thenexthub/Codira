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

#ifndef JSVM_VALUE_H
#define JSVM_VALUE_H
#include <variant>

#include "jsvm_util.h"

struct JSVM_Script_Data__ {
public:
    using SourcePtr = std::variant<v8::Local<v8::Script>, v8::Global<v8::Script>>;
    using DataType = enum { kJsvmScript };

    template<typename T>
    JSVM_Script_Data__(T ptr, bool retained, DataType type = kJsvmScript)
        : taggedPointer(ptr), isGlobal(retained), type(type)
    {}

    template<class T>
    v8::Local<T> ToV8Local(v8::Isolate* isolate)
    {
        if (isGlobal) {
            return v8::Local<T>::New(isolate, std::get<v8::Global<T>>(taggedPointer));
        } else {
            return std::get<v8::Local<T>>(taggedPointer);
        }
    }

    SourcePtr taggedPointer;
    bool isGlobal = false;
    DataType type;
};

namespace v8impl {
// This asserts v8::Local<> will always be implemented with a single
// pointer field so that we can pass it around as a void*.
static_assert(sizeof(v8::Local<v8::Data>) == sizeof(JSVM_Data),
              "Cannot convert between v8::Local<v8::Data> and JSVM_Data");

inline JSVM_Data JsDataFromV8LocalData(v8::Local<v8::Data> local)
{
    return reinterpret_cast<JSVM_Data>(*local);
}

inline v8::Local<v8::Data> V8LocalDataFromJsData(JSVM_Data data)
{
    v8::Local<v8::Data> local;
    *reinterpret_cast<JSVM_Data*>(&local) = data;
    return local;
}

// This asserts v8::Local<> will always be implemented with a single
// pointer field so that we can pass it around as a void*.
static_assert(sizeof(v8::Local<v8::Value>) == sizeof(JSVM_Value),
              "Cannot convert between v8::Local<v8::Value> and JSVM_Value");

inline JSVM_Value JsValueFromV8LocalValue(v8::Local<v8::Value> local)
{
    return reinterpret_cast<JSVM_Value>(*local);
}

inline v8::Local<v8::Value> V8LocalValueFromJsValue(JSVM_Value value)
{
    v8::Local<v8::Value> local;
    *reinterpret_cast<JSVM_Value*>(&local) = value;
    return local;
}

inline JSVM_Deferred JsDeferredFromPersistent(v8impl::Persistent<v8::Value>* local)
{
    return reinterpret_cast<JSVM_Deferred>(local);
}

inline v8impl::Persistent<v8::Value>* PersistentFromJsDeferred(JSVM_Deferred local)
{
    return reinterpret_cast<v8impl::Persistent<v8::Value>*>(local);
}

class HandleScopeWrapper {
public:
    explicit HandleScopeWrapper(v8::Isolate* isolate) : scope(isolate) {}

private:
    v8::HandleScope scope;
};

class EscapableHandleScopeWrapper {
public:
    explicit EscapableHandleScopeWrapper(v8::Isolate* isolate) : scope(isolate), escapeCalled(false) {}
    bool IsEscapeCalled() const
    {
        return escapeCalled;
    }
    template<typename T>
    v8::Local<T> Escape(v8::Local<T> handle)
    {
        escapeCalled = true;
        return scope.Escape(handle);
    }

private:
    v8::EscapableHandleScope scope;
    bool escapeCalled;
};

inline JSVM_HandleScope JsHandleScopeFromV8HandleScope(HandleScopeWrapper* s)
{
    return reinterpret_cast<JSVM_HandleScope>(s);
}

inline HandleScopeWrapper* V8HandleScopeFromJsHandleScope(JSVM_HandleScope s)
{
    return reinterpret_cast<HandleScopeWrapper*>(s);
}

inline JSVM_EscapableHandleScope JsEscapableHandleScopeFromV8EscapableHandleScope(EscapableHandleScopeWrapper* s)
{
    return reinterpret_cast<JSVM_EscapableHandleScope>(s);
}

inline EscapableHandleScopeWrapper* V8EscapableHandleScopeFromJsEscapableHandleScope(JSVM_EscapableHandleScope s)
{
    return reinterpret_cast<EscapableHandleScopeWrapper*>(s);
}

// Convert a v8::PersistentBase, e.g. v8::Global, to a Local, with an extra
// optimization for strong persistent handles.
class PersistentToLocal {
public:
    // Convert v8::Global to v8::Local
    template<class TypeName>
    static inline v8::Local<TypeName> Default(v8::Isolate* isolate, const v8::PersistentBase<TypeName>& persistent)
    {
        if (persistent.IsWeak()) {
            return PersistentToLocal::Weak(isolate, persistent);
        } else {
            return PersistentToLocal::Strong(persistent);
        }
    }

    template<class TypeName>
    static inline v8::Local<TypeName> Strong(const v8::PersistentBase<TypeName>& persistent)
    {
        DCHECK(!persistent.IsWeak());
        return *reinterpret_cast<v8::Local<TypeName>*>(const_cast<v8::PersistentBase<TypeName>*>(&persistent));
    }

    template<class TypeName>
    static inline v8::Local<TypeName> Weak(v8::Isolate* isolate, const v8::PersistentBase<TypeName>& persistent)
    {
        return v8::Local<TypeName>::New(isolate, persistent);
    }
};
} // namespace v8impl

#endif