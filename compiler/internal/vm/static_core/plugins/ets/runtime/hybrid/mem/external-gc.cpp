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

#include "libarkbase/utils/logger.h"
#include "runtime/mark_word.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
#include "common_interfaces/objects/ref_field.h"
#include "common_interfaces/heap/heap_visitor.h"
#ifdef PANDA_JS_ETS_HYBRID_MODE
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#endif  // PANDA_JS_ETS_HYBRID_MODE

namespace ark::mem::ets {

void VisitVmRoots(const GCRootVisitor &visitor, PandaVM *vm)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    RootManager<EtsLanguageConfig> rootManager(vm);
    rootManager.VisitNonHeapRoots(visitor, VisitGCRootFlags::ACCESS_ROOT_ALL);
    vm->VisitStringTable(visitor, VisitGCRootFlags::ACCESS_ROOT_ALL);
}

void UpdateVmRoots(const GCRootUpdater &updater, PandaVM *vm)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    RootManager<EtsLanguageConfig> rootManager(vm);
    rootManager.UpdateRefsToMovedObjects(updater);
}

}  // namespace ark::mem::ets

namespace common {

static ark::PandaVM *GetPandaVM()
{
    auto *runtime = ark::Runtime::GetCurrent();
    if (runtime == nullptr) {
        return nullptr;
    }
    return runtime->GetPandaVM();
}

void VisitStaticRoots(const RefFieldVisitor &visitor)
{
    ark::PandaVM *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }

    ark::GCRootVisitor rootVisitor = [&visitor](const ark::mem::GCRoot &gcRoot) {
        common::RefField<> refField {reinterpret_cast<common::BaseObject *>(gcRoot.GetObjectHeader())};
        visitor(refField);
    };
    ark::mem::ets::VisitVmRoots(rootVisitor, vm);
}

void UpdateStaticRoots(const RefFieldVisitor &visitor)
{
    ark::PandaVM *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }

    ark::GCRootUpdater rootUpdater = [&visitor](ark::ObjectHeader **object) {
        ark::ObjectHeader *prev = *object;
        visitor(reinterpret_cast<RefField<> &>(*reinterpret_cast<BaseObject **>(object)));
        return prev != *object;
    };
    ark::mem::ets::UpdateVmRoots(rootUpdater, vm);
}

void SweepStaticRoots(const WeakRefFieldVisitor &visitor)
{
    ark::PandaVM *vm = GetPandaVM();
    if (vm == nullptr) {
        return;
    }
    ark::GCObjectVisitor visitorWrap = [&visitor](ark::ObjectHeader *object) -> ark::ObjectStatus {
        // Provide the reference to a local variable.
        // The object must be forwarded during UpdateStaticRoots.
        // Here we need to know is the object alive or not.
        bool alive = visitor(reinterpret_cast<RefField<> &>(*reinterpret_cast<BaseObject **>(&object)));
        return alive ? ark::ObjectStatus::ALIVE_OBJECT : ark::ObjectStatus::DEAD_OBJECT;
    };
    vm->SweepVmRefs(visitorWrap);
    auto *refProccessor = static_cast<ark::mem::ets::EtsReferenceProcessor *>(vm->GetReferenceProcessor());
    refProccessor->ClearDeadReferences([&visitor](const ark::ObjectHeader *object) -> bool {
        return !visitor(reinterpret_cast<RefField<> &>(*reinterpret_cast<const BaseObject **>(&object)));
    });
}

void RegisterStaticRootsProcessFunc()
{
    RegisterVisitStaticRootsHook(VisitStaticRoots);
    RegisterUpdateStaticRootsHook(UpdateStaticRoots);
    RegisterSweepStaticRootsHook(SweepStaticRoots);
}

}  // namespace common
