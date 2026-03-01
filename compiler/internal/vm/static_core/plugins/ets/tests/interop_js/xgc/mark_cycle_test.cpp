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

#include <iostream>
#include <array>

#include "hybrid/ecma_vm_interface.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/tests/interop_js/xgc/test_xgc_vm_adaptor.h"
#include "native_reference.h"

namespace ark::ets::interop::js {

struct TestXGCEcmaAdaptorValues {
    void SetExpectedJsObject(napi_ref obj)
    {
        expectedJsObject = obj;
    }

    void SetRefToMark(ets_proxy::SharedReference *ref)
    {
        refToMark = ref;
    }

    std::vector<std::string> GetErrors()
    {
        if (!markFromObjectCalled) {
            std::stringstream err;
            err << "MarkFromObject was not called";
            errors.insert(errors.begin(), err.str());
        }
        return errors;
    }

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    std::vector<std::string> errors;
    napi_ref expectedJsObject = nullptr;
    ets_proxy::SharedReference *refToMark = nullptr;
    bool markFromObjectCalled = false;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

static TestXGCEcmaAdaptorValues g_xgcAdaptorValues;

class TestXGCEcmaVmAdaptor : public TestXGCVmAdaptor {
public:
    explicit TestXGCEcmaVmAdaptor(napi_env env, TestXGCEcmaAdaptorValues *values)
        : TestXGCVmAdaptor(env), values_(values)
    {
        ASSERT(values_ != nullptr);
    }

#if defined(ARK_HYBRID)
    void MarkFromObject(napi_ref obj, const common::RefFieldVisitor &visitor) override
    {
        values_->markFromObjectCalled = true;
        XGCVmAdaptor::MarkFromObject(obj, visitor);
    }
#endif

    void MarkFromObject(napi_ref obj) override
    {
        values_->markFromObjectCalled = true;
        if (obj != values_->expectedJsObject) {
            std::stringstream err;
            err << "MarkFromObject called with " << obj << ", but expected " << values_->expectedJsObject;
            values_->errors.push_back(err.str());
            return;
        }
        InteropCtx::Current()->GetSTSVMInterface()->MarkFromObject(values_->refToMark->GetJsRef());
    }

private:
    TestXGCEcmaAdaptorValues *values_ = nullptr;
};

class TestGCListener : public mem::GCListener {
public:
    void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
    {
        if (task.reason != GCTaskCause::CROSSREF_CAUSE) {
            std::stringstream err;
            err << "Expected GC cause CROSSREF_CAUSE, but got " << task.reason;
            errorMessages_.push_back(err.str());
            return;
        }

        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        if (xrefStorage->Size() != 2U) {
            std::stringstream err;
            err << "Expected xrefs count 2, but got " << xrefStorage->Size();
            errorMessages_.push_back(err.str());
            return;
        }
        bool js2etsRef = false;
        bool ets2jsRef = false;
        xrefStorage->VisitRoots([this, xrefStorage, &js2etsRef, &ets2jsRef](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            if (!obj->HasInteropIndex()) {
                std::stringstream err;
                err << "Found object " << obj << " in ShredReferenceStorage without interop index";
                errorMessages_.push_back(err.str());
                return;
            }
            auto *xref = xrefStorage->GetReference(obj);
            if (xref->HasETSFlag()) {
                if (js2etsRef) {
                    std::stringstream err;
                    err << "Found second object " << obj << " with 'ets' flag";
                    errorMessages_.push_back(err.str());
                    return;
                }
                js2etsRef = true;
                g_xgcAdaptorValues.SetRefToMark(xref);
            }
            if (xref->HasJSFlag()) {
                if (ets2jsRef) {
                    std::stringstream err;
                    err << "Found second object " << obj << " with 'js' flag";
                    errorMessages_.push_back(err.str());
                    return;
                }
                ets2jsRef = true;
                g_xgcAdaptorValues.SetExpectedJsObject(xref->GetJsRef());
            }
        });
    }

    void GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
        auto *xrefStorage = InteropCtx::Current()->GetSharedRefStorage();
        xrefStorage->VisitRoots([this, xrefStorage](const mem::GCRoot &root) {
            auto *obj = EtsObject::FromCoreType(root.GetObjectHeader());
            auto *xref = xrefStorage->GetReference(obj);
            if (!xref->IsMarked()) {
                std::stringstream err;
                err << "Found not marked xref";
                errorMessages_.push_back(err.str());
                return;
            }
        });
    }

    const std::vector<std::string> &GetErrors() const
    {
        return errorMessages_;
    }

private:
    std::vector<std::string> errorMessages_;
};

static TestGCListener g_gcListener;

}  // namespace ark::ets::interop::js

#include "test_module.h"
