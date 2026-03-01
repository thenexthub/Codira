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

#include "verification/type/type_system.h"

#include "assembler/assembly-type.h"
#include "include/class_helper.h"
#include "include/class_linker.h"
#include "include/mem/panda_string.h"
#include "runtime/include/thread_scopes.h"
#include "verification/jobs/job.h"
#include "verification/jobs/service.h"
#include "verification/public_internal.h"
#include "verification/plugins.h"
#include "verifier_messages.h"
#include "verification/type/type_type.h"

namespace ark::verifier {

static PandaString ClassNameToDescriptorString(char const *name)
{
    PandaString str = "L";
    for (char const *s = name; *s != '\0'; s++) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        char c = (*s == '.') ? '/' : *s;
        str += c;
    }
    str += ';';
    return str;
}

TypeSystem::TypeSystem(VerifierService *service, panda_file::SourceLang lang)
    : service_ {service},
      plugin_ {plugin::GetLanguagePlugin(lang)},
      langCtx_ {ark::plugins::GetLanguageContextBase(lang)},
      bootLinkerCtx_ {service->GetClassLinker()->GetExtension(LanguageContext {langCtx_})->GetBootContext()}
{
    ScopedChangeThreadStatus st(ManagedThread::GetCurrent(), ThreadStatus::RUNNING);
    auto compute = [&](uint8_t const *descr) -> Type {
        return descr != nullptr ? BootDescriptorToType(descr) : Type {};
    };

    class_ = compute(langCtx_.GetClassClassDescriptor());
    object_ = compute(langCtx_.GetObjectClassDescriptor());
    string_ = compute(langCtx_.GetStringClassDescriptor());
    // Throwable is not given to us as descriptor for some reason. NOTE(gogabr): correct this.
    auto throwableClassName = langCtx_.GetVerificationTypeThrowable();
    if (throwableClassName != nullptr) {
        auto throwableDescrStr = ClassNameToDescriptorString(throwableClassName);
        throwable_ = compute(utf::CStringAsMutf8(throwableDescrStr.c_str()));
    } else {
        throwable_ = Type {};
    }
    plugin_->TypeSystemSetup(this);
}

static Type DescriptorToType(ClassLinker *linker, ClassLinkerContext *ctx, uint8_t const *descr)
{
    Job::ErrorHandler handler;
    auto klass = linker->GetClass(descr, true, ctx, &handler);
    if (klass != nullptr) {
        return Type {klass};
    }
    return Type {};
}

Type TypeSystem::BootDescriptorToType(uint8_t const *descr)
{
    return DescriptorToType(service_->GetClassLinker(), bootLinkerCtx_, descr);
}

void TypeSystem::ExtendBySupers(PandaUnorderedSet<Type> *set, Class const *klass)
{
    // NOTE(gogabr): do we need to cache intermediate results? Measure!
    Type newTp = Type {klass};
    if (set->count(newTp) > 0) {
        return;
    }
    set->insert(newTp);

    Class const *super = klass->GetBase();
    if (super != nullptr) {
        ExtendBySupers(set, super);
    }

    for (auto const *a : klass->GetInterfaces()) {
        ExtendBySupers(set, a);
    }
}

Type TypeSystem::NormalizedTypeOf(Type type)
{
    ASSERT(!type.IsNone());
    if (type == Type::Bot() || type == Type::Top()) {
        return type;
    }
    auto t = normalizedTypeOf_.find(type);
    if (t != normalizedTypeOf_.cend()) {
        return t->second;
    }
    Type result = plugin_->NormalizeType(type, this);
    normalizedTypeOf_[type] = result;
    return result;
}

MethodSignature const *TypeSystem::GetMethodSignature(Method const *method)
{
    ScopedChangeThreadStatus st(ManagedThread::GetCurrent(), ThreadStatus::RUNNING);
    auto &&methodId = method->GetUniqId();
    auto it = signatureOfMethod_.find(methodId);
    if (it != signatureOfMethod_.end()) {
        return &it->second;
    }

    methodOfId_[methodId] = method;

    auto const resolveType = [this, method](const uint8_t *descr) {
        return DescriptorToType(service_->GetClassLinker(), method->GetClass()->GetLoadContext(), descr);
    };

    MethodSignature sig;
    if (method->GetReturnType().IsReference()) {
        sig.result = resolveType(method->GetRefReturnType().data);
    } else {
        sig.result = Type::FromTypeId(method->GetReturnType().GetId());
    }
    size_t refIdx = 0;
    for (size_t i = 0; i < method->GetNumArgs(); i++) {
        Type argType;
        if (method->GetArgType(i).IsReference()) {
            argType = resolveType(method->GetRefArgType(refIdx++).data);
        } else {
            argType = Type::FromTypeId(method->GetArgType(i).GetId());
        }
        sig.args.push_back(argType);
    }
    signatureOfMethod_[methodId] = sig;
    return &signatureOfMethod_[methodId];
}

PandaUnorderedSet<Type> const *TypeSystem::SupertypesOfClass(Class const *klass)
{
    ASSERT(!klass->IsArrayClass());
    if (supertypesCache_.count(klass) > 0) {
        return &supertypesCache_[klass];
    }

    PandaUnorderedSet<Type> toCache;
    ExtendBySupers(&toCache, klass);
    supertypesCache_[klass] = std::move(toCache);
    return &supertypesCache_[klass];
}

void TypeSystem::MentionClass(Class const *klass)
{
    knownClasses_.insert(klass);
}

void TypeSystem::DisplayTypeSystem(std::function<void(PandaString const &)> const &handler)
{
    handler("Classes:");
    DisplayClasses([&handler](auto const &name) { handler(name); });
    handler("Methods:");
    DisplayMethods([&handler](auto const &name, auto const &sig) { handler(name + " : " + sig); });
    handler("Subtyping (type <; supertype)");
    DisplaySubtyping([&handler](auto const &type, auto const &supertype) { handler(type + " <: " + supertype); });
}

void TypeSystem::DisplayClasses(std::function<void(PandaString const &)> const &handler) const
{
    for (auto const *klass : knownClasses_) {
        handler((Type {klass}).ToString(this));
    }
}

void TypeSystem::DisplayMethods(std::function<void(PandaString const &, PandaString const &)> const &handler) const
{
    for (auto const &it : signatureOfMethod_) {
        auto &id = it.first;
        auto &sig = it.second;
        auto const *method = methodOfId_.at(id);
        handler(method->GetFullName(), sig.ToString(this));
    }
}

void TypeSystem::DisplaySubtyping(std::function<void(PandaString const &, PandaString const &)> const &handler)
{
    for (auto const *klass : knownClasses_) {
        if (klass->IsArrayClass()) {
            continue;
        }
        auto klassStr = (Type {klass}).ToString(this);
        for (auto const &supertype : *SupertypesOfClass(klass)) {
            handler(klassStr, supertype.ToString(this));
        }
    }
}

}  // namespace ark::verifier
