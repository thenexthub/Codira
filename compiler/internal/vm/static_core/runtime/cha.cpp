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

#include "runtime/cha.h"

#include "libarkbase/events/events.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/deoptimization.h"

namespace ark {

using os::memory::LockHolder;  // NOLINT(misc-unused-using-decls)

void ClassHierarchyAnalysis::Update(Class *klass)
{
    auto parent = klass->GetBase();

    if (klass->IsInterface()) {
        return;
    }

    if (parent == nullptr) {
        for (const auto &method : klass->GetVTable()) {
            SetHasSingleImplementation(method, true);
        }
        return;
    }

    ASSERT(klass->GetVTableSize() >= parent->GetVTableSize());

    PandaSet<Method *> invalidatedMethods;

    for (size_t i = 0; i < parent->GetVTableSize(); ++i) {
        auto method = klass->GetVTable()[i];
        auto parentMethod = parent->GetVTable()[i];
        if (method == parentMethod || method->IsDefaultInterfaceMethod()) {
            continue;
        }

        if (HasSingleImplementation(parentMethod)) {
            EVENT_CHA_INVALIDATE(std::string(parentMethod->GetFullName()), klass->GetName());
            invalidatedMethods.insert(parentMethod);
        }
        UpdateMethod(method);
    }

    for (size_t i = parent->GetVTableSize(); i < klass->GetVTableSize(); ++i) {
        auto method = klass->GetVTable()[i];
        if (method->IsDefaultInterfaceMethod()) {
            continue;
        }
        UpdateMethod(method);
    }

    InvalidateMethods(invalidatedMethods);
}

bool ClassHierarchyAnalysis::HasSingleImplementation(Method *method)
{
    LockHolder lock(GetLock());
    return method->HasSingleImplementation();
}

void ClassHierarchyAnalysis::SetHasSingleImplementation(Method *method, bool singleImplementation)
{
    LockHolder lock(GetLock());
    method->SetHasSingleImplementation(singleImplementation);
}

void ClassHierarchyAnalysis::UpdateMethod(Method *method)
{
    // NOTE(msherstennikov): Currently panda is allowed to execute abstract method, thus we cannot propagate single
    // implementation property of the non-abstract method to the all overriden abstract methods.
    SetHasSingleImplementation(method, !method->IsAbstract());
}

void ClassHierarchyAnalysis::InvalidateMethods(const PandaSet<Method *> &methods)
{
    PandaSet<Method *> dependentMethods;

    {
        LockHolder lock(GetLock());
        for (auto method : methods) {
            InvalidateMethod(method, &dependentMethods);
        }
    }

    if (dependentMethods.empty()) {
        return;
    }

    InvalidateCompiledEntryPoint(dependentMethods, true);
}

void ClassHierarchyAnalysis::InvalidateMethod(Method *method, PandaSet<Method *> *dependentMethods) REQUIRES(GetLock())
{
    if (!method->HasSingleImplementation()) {
        return;
    }

    method->SetHasSingleImplementation(false);

    LOG(DEBUG, CLASS_LINKER) << "[CHA] Invalidate method " << method->GetFullName();

    auto it = dependencyMap_.find(method);
    if (it == dependencyMap_.end()) {
        return;
    }

    for (auto depMethod : it->second) {
        dependentMethods->insert(depMethod);
    }

    dependencyMap_.erase(method);
}

void ClassHierarchyAnalysis::AddDependency(Method *callee, Method *caller)
{
    LockHolder lock(GetLock());
    LOG(DEBUG, CLASS_LINKER) << "[CHA] Add dependency: caller " << caller->GetFullName() << ", callee "
                             << callee->GetFullName();
    // Other thread can remove single implementation of the callee method while we compile caller method.
    if (!callee->HasSingleImplementation()) {
        return;
    }
    // There is no sense to store dependencies for abstract methods.
    ASSERT(!callee->IsAbstract());
    dependencyMap_[callee].insert(caller);
}

}  // namespace ark
