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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_BASE_INL_H
#define PANDA_RUNTIME_VTABLE_BUILDER_BASE_INL_H

#include "runtime/include/vtable_builder_base.h"

namespace ark {

void OnVTableConflict(ClassLinkerErrorHandler *errHandler, ClassLinker::Error error, std::string_view msg,
                      MethodInfo const *info1, MethodInfo const *info2);

void OnVTableConflict(ClassLinkerErrorHandler *errHandler, ClassLinker::Error error, std::string_view msg,
                      Method const *info1, Method const *info2);

inline void VTableInfo::AddEntry(const MethodInfo *info)
{
    [[maybe_unused]] auto res = vmethods_.insert({info, MethodEntry(vmethods_.size())});
    ASSERT(res.second);
}

inline void VTableInfo::ReplaceEntryWith(const MethodInfo *prev, const MethodInfo *current)
{
    ASSERT(prev != current);

    auto it = vmethods_.find(prev);
    ASSERT(it != vmethods_.end());

    size_t idx = it->second.GetIndex();

    vmethods_.erase(it);

    [[maybe_unused]] auto res = vmethods_.insert({current, MethodEntry(idx)});
    ASSERT(res.second);
}

inline VTableInfo::CopiedMethodEntry &VTableInfo::AddCopiedEntry(const MethodInfo *info)
{
    auto res = copiedMethods_.insert({info, CopiedMethodEntry(copiedMethods_.size())});
    ASSERT(res.second);
    return res.first->second;
}

inline VTableInfo::CopiedMethodEntry &VTableInfo::UpdateCopiedEntry(const MethodInfo *orig, const MethodInfo *repl)
{
    auto it = copiedMethods_.find(orig);
    ASSERT(it != copiedMethods_.end());
    CopiedMethodEntry entry = it->second;
    copiedMethods_.erase(it);
    return copiedMethods_.emplace_hint(it, repl, entry)->second;
}

template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::BuildForInterface(panda_file::ClassDataAccessor *cda)
{
    ASSERT(cda->IsInterface());
    cda->EnumerateMethods([this](panda_file::MethodDataAccessor &mda) {
        if (mda.IsStatic()) {
            return;
        }

        if (!mda.IsAbstract()) {
            hasDefaultMethods_ = true;
        }

        ++numVmethods_;
    });
}

template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::BuildForInterface(Span<Method> methods)
{
    for (const auto &method : methods) {
        if (method.IsStatic()) {
            continue;
        }

        if (!method.IsAbstract()) {
            hasDefaultMethods_ = true;
        }

        ++numVmethods_;
    }
}

template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::AddBaseMethods(Class *baseClass)
{
    auto baseMethods = allocator_.New<ArenaForwardList<MethodInfo>>(allocator_.Adapter());
    ASSERT(baseMethods != nullptr);

    if (baseClass != nullptr) {
        for (auto const &method : baseClass->GetVTable()) {
            vtable_.AddEntry(&baseMethods->emplace_front(method, 0, true));
        }
    }
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddClassMethods(panda_file::ClassDataAccessor *cda, ClassLinkerContext *ctx)
{
    ASSERT(cda != nullptr);
    auto classMethods = allocator_.New<ArenaForwardList<MethodInfo>>(allocator_.Adapter());
    ASSERT(classMethods != nullptr);

    cda->EnumerateMethods([this, ctx, &classMethods](panda_file::MethodDataAccessor &mda) {
        if (!mda.IsStatic()) {
            classMethods->emplace_front(mda, numVmethods_++, ctx);
        }
    });

    for (auto &info : *classMethods) {
        if (!ProcessClassMethod(&info)) {
            return false;
        }
    }
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddClassMethods(Span<Method> methods)
{
    auto classMethods = allocator_.New<ArenaForwardList<MethodInfo>>(allocator_.Adapter());
    ASSERT(classMethods != nullptr);

    for (auto &method : methods) {
        if (!method.IsStatic()) {
            classMethods->emplace_front(&method, numVmethods_++);
        }
    }

    for (auto const &info : *classMethods) {
        if (!ProcessClassMethod(&info)) {
            return false;
        }
    }
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddProxyClassMethods(Span<Method *> methods)
{
    auto *classMethods = allocator_.New<ArenaForwardList<MethodInfo>>(allocator_.Adapter());
    ASSERT(classMethods != nullptr);

    for (auto *method : methods) {
        classMethods->emplace_front(method, numVmethods_++);
    }

    for (auto const &info : *classMethods) {
        if (!ProcessProxyClassMethod(&info)) {
            return false;
        }
    }
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddDefaultInterfaceMethods(ITable itable, size_t superItableSize)
{
    auto defaultMethods = allocator_.New<ArenaForwardList<MethodInfo>>(allocator_.Adapter());
    ASSERT(defaultMethods != nullptr);

    // NOTE(vpukhov): avoid traversing the whole itable and handle conflicting super vtable methods in a separate pass
    size_t const traverseUpTo = VISIT_SUPERITABLE ? 0 : superItableSize;

    for (size_t i = itable.Size(); i != traverseUpTo;) {
        i--;
        auto iface = itable[i].GetInterface();
        if (!iface->HasDefaultMethods()) {
            continue;
        }

        auto methods = iface->GetVirtualMethods();
        for (auto &method : methods) {
            if (method.IsAbstract()) {
                continue;
            }
            MethodInfo *info = &defaultMethods->emplace_front(&method);
            if (!ProcessDefaultMethod(itable, i, info)) {
                return false;
            }
        }
    }

    ASSERT(orderedCopiedMethods_.empty());
    orderedCopiedMethods_.resize(vtable_.CopiedMethods().size());

    for (auto const &[info, entry] : vtable_.CopiedMethods()) {
        CopiedMethod copied(info->GetMethod());
        copied.SetStatus(entry.GetStatus());
        orderedCopiedMethods_[entry.GetIndex()] = copied;
    }
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::Build(panda_file::ClassDataAccessor *cda, Class *baseClass, ITable itable,
                                                 ClassLinkerContext *ctx)
{
    if (cda->IsInterface()) {
        BuildForInterface(cda);
        return true;
    }

    AddBaseMethods(baseClass);
    if (!AddClassMethods(cda, ctx)) {
        return false;
    }
    if (!AddDefaultInterfaceMethods(itable, baseClass != nullptr ? baseClass->GetITable().Size() : 0)) {
        return false;
    }
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::Build(Span<Method> methods, Class *baseClass, ITable itable,
                                                 bool isInterface)
{
    if (isInterface) {
        BuildForInterface(methods);
        return true;
    }

    AddBaseMethods(baseClass);
    if (!AddClassMethods(methods)) {
        return false;
    }
    if (!AddDefaultInterfaceMethods(itable, baseClass != nullptr ? baseClass->GetITable().Size() : 0)) {
        return false;
    }
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::FilterProxyClassMethods(Span<Method *> input, PandaVector<Method *> *output,
                                                                   Class *baseClass)
{
    AddBaseMethods(baseClass);
    if (!AddProxyClassMethods(input)) {
        return false;
    }

    if (!CollectProxyMethods(output)) {
        return false;
    }

    return true;
}

template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::UpdateClass(Class *klass) const
{
    if (klass->IsInterface()) {
        if (hasDefaultMethods_) {
            klass->SetHasDefaultMethods();
        }

        size_t idx = 0;
        for (auto &method : klass->GetVirtualMethods()) {
            method.SetVTableIndex(idx++);
        }
    }

    vtable_.UpdateClass(klass);
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::CollectProxyMethods(PandaVector<Method *> *output)
{
    auto &methodsUmap = vtable_.Methods();
    for (auto it : methodsUmap) {
        if (!it.first->IsBase()) {
            output->push_back(it.first->GetMethod());
        }
    }
    for (auto it : orderedCopiedMethods_) {
        output->push_back(it.GetMethod());
    }
    return true;
}

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_BASE_H
