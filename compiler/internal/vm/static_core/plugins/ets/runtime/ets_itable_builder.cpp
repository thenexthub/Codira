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

#include "ets_itable_builder.h"

#include "include/method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_vtable_builder.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::ets {

static std::optional<Method *> FindMethodInVTable(Class *klass, Method *imethod, ClassLinkerErrorHandler *errHandler)
{
    auto vtable = klass->GetVTable();
    auto imethodName = imethod->GetName();
    auto ctx = klass->GetLoadContext();

    Method *candidate = nullptr;

    for (size_t i = vtable.size(); i != 0;) {
        i--;
        auto kmethod = vtable[i];
        if (LIKELY(kmethod->GetName() != imethodName)) {
            continue;
        }
        if (!ETSProtoIsOverriddenBy(ctx, imethod->GetProtoId(), kmethod->GetProtoId())) {
            continue;
        }
        if (candidate != nullptr) {
            OnVTableConflict(errHandler, ClassLinker::Error::MULTIPLE_IMPLEMENT, "Multiple implementations", kmethod,
                             candidate);
            return std::nullopt;
        }
        candidate = kmethod;
    }
    return candidate;
}

static Span<ITable::Entry> CloneBaseITable(ClassLinker *classLinker, Class *base, size_t size)
{
    auto allocator = classLinker->GetAllocator();
    Span<ITable::Entry> itable {size == 0 ? nullptr : allocator->AllocArray<ITable::Entry>(size), size};

    for (auto &entry : itable) {
        entry.SetMethods({nullptr, nullptr});
    }

    if (base != nullptr) {
        auto superItable = base->GetITable().Get();
        ASSERT(superItable.Size() <= itable.size());
        for (size_t i = 0; i < superItable.size(); i++) {
            itable[i] = superItable[i].Copy(allocator);
        }
    }
    return itable;
}

static Span<ITable::Entry> LinearizeITable(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces)
{
    auto allocator = classLinker->GetAllocator();

    PandaUnorderedMap<Class *, bool> interfaces;

    if (base != nullptr) {
        auto superItable = base->GetITable().Get();
        for (auto item : superItable) {
            interfaces.insert({item.GetInterface(), true});
        }
    }

    for (auto interface : classInterfaces) {
        ASSERT(interface != nullptr);
        auto table = interface->GetITable().Get();
        if (interfaces.insert({interface, false}).second) {
            for (auto item : table) {
                interfaces.insert({item.GetInterface(), false});
            }
        }
    }

    auto itable = CloneBaseITable(classLinker, base, interfaces.size());
    auto shift = base != nullptr ? base->GetITable().Size() : 0;

    for (auto interface : classInterfaces) {
        auto iterator = interfaces.find(interface);
        if (!(iterator != interfaces.end() && !iterator->second)) {
            continue;
        }
        auto interfaceItable = interface->GetITable().Get();
        for (auto &item : interfaceItable) {
            auto subIterator = interfaces.find(item.GetInterface());
            if (subIterator != interfaces.end() && !subIterator->second) {
                itable[shift++] = item.Copy(allocator);
                subIterator->second = true;
            }
        }
        itable[shift++].SetInterface(interface);
        iterator->second = true;
    }

    return itable;
}

bool EtsITableBuilder::Build(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces, bool isInterface)
{
    Span<ITable::Entry> itable =
        classInterfaces.Size() == 0 ? CloneBaseITable(classLinker, base, base != nullptr ? base->GetITable().Size() : 0)
                                    : LinearizeITable(classLinker, base, classInterfaces);

    if (!isInterface) {
        size_t const superItableSize = base != nullptr ? base->GetITable().Size() : 0;
        for (size_t i = superItableSize; i < itable.Size(); i++) {
            auto &entry = itable[i];
            auto methods = entry.GetInterface()->GetVirtualMethods();
            Method **methodsAlloc = nullptr;
            if (!methods.Empty()) {
                methodsAlloc = classLinker->GetAllocator()->AllocArray<Method *>(methods.size());
            }
            Span<Method *> methodsArray = {methodsAlloc, methods.size()};
            entry.SetMethods(methodsArray);
        }
    }

    itable_ = ITable(itable);
    return true;
}

// CC-OFFNXT(G.FUN.01-CPP, G.FUD.05) solid logic
bool EtsITableBuilder::Resolve(Class *klass)
{
    if (klass->IsInterface()) {
        return true;
    }
    UpdateClass(klass);

    auto const baseClass = klass->GetBase();
    auto const baseITableSize = baseClass == nullptr ? 0 : baseClass->GetITable().Size();

    for (size_t i = itable_.Size(); i > 0; i--) {
        auto const entryIdx = i - 1;
        auto const entry = &itable_[entryIdx];
        auto const baseEntry = entryIdx < baseITableSize ? &baseClass->GetITable()[entryIdx] : nullptr;
        auto methods = entry->GetInterface()->GetVirtualMethods();

        for (size_t j = 0; j < methods.size(); j++) {
            auto const imethod = &methods[j];
            Method *resolved = nullptr;
            if (baseEntry != nullptr) {
                auto baseMethod = baseEntry->GetMethods()[j];
                resolved =
                    baseMethod->GetClass()->IsInterface() ? nullptr : klass->GetVTable()[baseMethod->GetVTableIndex()];
            }
            if (resolved == nullptr) {
                auto opt = FindMethodInVTable(klass, imethod, errorHandler_);
                // CC-OFFNXT(G.FUN.01-CPP, C_RULE_ID_FUNCTION_NESTING_LEVEL) solid logic
                if (!opt.has_value()) {
                    return false;
                }
                resolved = opt.value();
            }
            entry->GetMethods()[j] = resolved != nullptr ? resolved : imethod;
        }
    }

    return true;
}

void EtsITableBuilder::UpdateClass(Class *klass)
{
    klass->SetITable(itable_);
    DumpITable(klass);
}

void EtsITableBuilder::DumpITable([[maybe_unused]] Class *klass)
{
#ifndef NDEBUG
    LOG(DEBUG, CLASS_LINKER) << "itable of class " << klass->GetName() << ":";
    auto itable = klass->GetITable();
    size_t idxI = 0;
    for (size_t i = 0; i < itable.Size(); i++) {
        auto entry = itable[i];
        auto interface = entry.GetInterface();
        LOG(DEBUG, CLASS_LINKER) << "[ interface - " << idxI++ << " ] " << interface->GetName() << ":";
        // #22950 itable has nullptr entries
    }
#endif  // NDEBUG
}

}  // namespace ark::ets
