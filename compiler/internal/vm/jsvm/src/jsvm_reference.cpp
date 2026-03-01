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

#include "js_native_api_v8.h"
#include "jsvm_reference-inl.h"

namespace v8impl {

void RefTracker::FinalizeAll(RefList* list)
{
    while (list->next != nullptr) {
        list->next->Finalize();
    }
}

void RefTracker::Finalize()
{
    UNREACHABLE("Finalize need to be realized");
}

// UserReference
UserReference* UserReference::New(JSVM_Env env, v8::Local<v8::Value> value, uint32_t initialRefcount)
{
    auto ref = new UserReference(env, value, true, initialRefcount);

    return ref;
}

UserReference* UserReference::NewData(JSVM_Env env, v8::Local<v8::Data> value, uint32_t initialRefcount)
{
    auto ref = new UserReference(env, value, false, initialRefcount);

    return ref;
}

UserReference::UserReference(JSVM_Env env, v8::Local<v8::Data> value, bool isValue, uint32_t initialRefcount)
    : persistent(env->isolate, value), env(env), refcount(initialRefcount), isValue(isValue),
      canBeWeak(isValue && CanBeHeldWeakly(value.As<v8::Value>()))
{
    if (refcount == 0) {
        SetWeak();
    }

    Link(&env->userReferenceList);
}

UserReference::~UserReference()
{
    persistent.Reset();
    Unlink();
}

void UserReference::Finalize()
{
    persistent.Reset();
    Unlink();
}

uint32_t UserReference::RefCount()
{
    return refcount;
}

// FinalizerTracker
FinalizerTracker* FinalizerTracker::New(JSVM_Env env, JSVM_Finalize cb, void* finalizeData, void* finalizeHint)
{
    return new FinalizerTracker(env, cb, finalizeData, finalizeHint);
}

FinalizerTracker::FinalizerTracker(JSVM_Env env, JSVM_Finalize cb, void* data, void* hint)
    : env(env), cb(cb), data(data), hint(hint)
{
    Link(&env->finalizerList);
}

FinalizerTracker::~FinalizerTracker()
{
    Unlink();
}

void FinalizerTracker::ResetFinalizer()
{
    cb = nullptr;
    data = nullptr;
    hint = nullptr;
}

void FinalizerTracker::CallFinalizer()
{
    if (!cb) {
        return;
    }

    JSVM_Finalize cbTemp = cb;
    void* dataTemp = data;
    void* hintTemp = hint;
    ResetFinalizer();

    if (!env) {
        cbTemp(env, dataTemp, hintTemp);
    } else {
        env->CallIntoModule([&](JSVM_Env env) { cbTemp(env, dataTemp, hintTemp); });
    }
}

void FinalizerTracker::Finalize()
{
    CallFinalizer();
    delete this;
}

RuntimeReference::RuntimeReference(JSVM_Env env, v8::Local<v8::Value> value, JSVM_Finalize cb, void* data, void* hint)
    : FinalizerTracker(env, cb, data, hint), persistent(env->isolate, value)
{
    DCHECK(CanBeHeldWeakly(value));
}

RuntimeReference* RuntimeReference::New(JSVM_Env env, v8::Local<v8::Value> value, void* data)
{
    auto* ref = new RuntimeReference(env, value, nullptr, data, nullptr);
    // Delete self in first pass callback
    ref->SetWeak(false);

    return ref;
}

RuntimeReference* RuntimeReference::New(JSVM_Env env,
                                        v8::Local<v8::Value> value,
                                        JSVM_Finalize cb,
                                        void* data,
                                        void* hint)
{
    auto* ref = new RuntimeReference(env, value, cb, data, hint);
    // Need second pass callback to call finalizer
    ref->SetWeak(cb != nullptr);

    return ref;
}

void RuntimeReference::DeleteReference(RuntimeReference* ref)
{
    // If reference is not added into first pass callbacks, delete this direct.
    if (ref->persistent.IsWeak()) {
        delete ref;
        return;
    }

    // If reference is added into first pass callbacks, reset finalizer function.
    ref->ResetFinalizer();
}

inline void RuntimeReference::SetWeak(bool needSecondPass)
{
    if (needSecondPass) {
        persistent.SetWeak(this, FirstPassCallback, v8::WeakCallbackType::kParameter);
    } else {
        persistent.SetWeak(this, FirstPassCallbackWithoutFinalizer, v8::WeakCallbackType::kParameter);
    }
}

void RuntimeReference::FirstPassCallback(const v8::WeakCallbackInfo<RuntimeReference>& data)
{
    RuntimeReference* reference = data.GetParameter();

    reference->persistent.Reset();
    reference->Finalize();
}

void RuntimeReference::SecondPassCallback(const v8::WeakCallbackInfo<RuntimeReference>& data)
{
    RuntimeReference* reference = data.GetParameter();

    reference->Finalize();
}

void RuntimeReference::FirstPassCallbackWithoutFinalizer(const v8::WeakCallbackInfo<RuntimeReference>& data)
{
    RuntimeReference* reference = data.GetParameter();

    reference->persistent.Reset();
    delete reference;
}

} // namespace v8impl