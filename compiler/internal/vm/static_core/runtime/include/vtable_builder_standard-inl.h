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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_STANDARD_INL_H
#define PANDA_RUNTIME_VTABLE_BUILDER_STANDARD_INL_H

#include "runtime/include/vtable_builder_standard.h"
#include "runtime/include/vtable_builder_base-inl.h"

namespace ark {

template <class OverridePred>
bool StandardVTableBuilder<OverridePred>::IsOverriddenBy(Method::ProtoId const &base, Method::ProtoId const &derv)
{
    if (&base.GetPandaFile() == &derv.GetPandaFile() && base.GetEntityId() == derv.GetEntityId()) {
        return true;
    }
    return VTableProtoIdentical()(base, derv);
}

template <class OverridePred>
bool StandardVTableBuilder<OverridePred>::ProcessClassMethod(const MethodInfo *info)
{
    bool compatibleFound = false;

    for (auto it = SameNameMethodInfoIterator(vtable_.Methods(), info); !it.IsEmpty(); it.Next()) {
        auto &[itInfo, itEntry] = it.Value();

        if (!itInfo->IsBase()) {
            continue;
        }
        if (IsOverriddenBy(itInfo->GetProtoId(), info->GetProtoId()) && OverridePred()(itInfo, info)) {
            if (UNLIKELY(itInfo->IsFinal())) {
                OnVTableConflict(errorHandler_, ClassLinker::Error::OVERRIDES_FINAL, "Method overrides final method",
                                 info, itInfo);
                return false;
            }
            itEntry.SetCandidate(info);
            compatibleFound = true;
        }
    }

    if (!compatibleFound) {
        vtable_.AddEntry(info);
    }
    return true;
}

template <class OverridePred>
std::optional<MethodInfo const *> StandardVTableBuilder<OverridePred>::ScanConflictingDefaultMethods(
    const MethodInfo *info)
{
    // NOTE(vpukhov): test public flag
    for (auto it = SameNameMethodInfoIterator(vtable_.Methods(), info); !it.IsEmpty(); it.Next()) {
        MethodInfo const *itinfo = it.Value().second.CandidateOr(it.Value().first);
        VTableInfo::MethodEntry *itentry = &it.Value().second;

        if (!IsOverriddenBy(info->GetProtoId(), itinfo->GetProtoId())) {
            continue;
        }
        if (!itinfo->IsInterfaceMethod()) {
            // there is at least one implementing method, so skip default method
            return std::nullopt;
        }
        if (!itinfo->IsBase()) {
            break;
        }
        // NOTE(vpukhov): that method is possibly a conflict, but we traverse the whole itable to handle such cases
        itentry->SetCandidate(info);
    }

    for (auto it = SameNameMethodInfoIterator(vtable_.CopiedMethods(), info); !it.IsEmpty(); it.Next()) {
        MethodInfo const *itinfo = it.Value().first;
        if (!IsOverriddenBy(info->GetProtoId(), itinfo->GetProtoId())) {
            continue;
        }
        if (itinfo->GetMethod()->GetClass()->Implements(info->GetMethod()->GetClass())) {
            // more specific compatible method is defined, so skip default method
            return std::nullopt;
        }
        return itinfo;
    }

    return nullptr;
}

// we have to guarantee that while we are iterating itable, the child interface has to be accessed before father
// interface. interface without inheritance has no limit.
template <class OverridePred>
bool StandardVTableBuilder<OverridePred>::IsMaxSpecificInterfaceMethod(const Class *iface, const Method &method,
                                                                       size_t startindex, const ITable &itable)
{
    for (size_t j = startindex; j < itable.Size(); j++) {
        auto currentIface = itable[j].GetInterface();
        if (!currentIface->Implements(iface)) {
            continue;
        }
        for (auto &curmethod : currentIface->GetVirtualMethods()) {
            if (method.GetName() != curmethod.GetName()) {
                continue;
            }
            if (IsOverriddenBy(method.GetProtoId(), curmethod.GetProtoId())) {
                return false;
            }
        }
    }
    return true;
}

template <class OverridePred>
bool StandardVTableBuilder<OverridePred>::ProcessDefaultMethod(ITable itable, size_t itableIdx, MethodInfo *methodInfo)
{
    auto skipOrConflict = ScanConflictingDefaultMethods(methodInfo);
    if (!skipOrConflict.has_value()) {
        return true;
    }
    MethodInfo const *conflict = skipOrConflict.value();
    Class *iface = itable[itableIdx].GetInterface();
    Method *method = methodInfo->GetMethod();

    // if the default method is added for the first time, just add it.
    if (LIKELY(conflict == nullptr)) {
        VTableInfo::CopiedMethodEntry *entry = &vtable_.AddCopiedEntry(methodInfo);
        if (!IsMaxSpecificInterfaceMethod(iface, *method, itableIdx + 1, itable)) {
            entry->SetStatus(CopiedMethod::Status::ABSTRACT);
        }
        return true;
    }

    VTableInfo::CopiedMethodEntry *entry = &vtable_.CopiedMethods().find(conflict)->second;

    // Use the following algorithm to judge whether we have to replace existing DEFAULT METHOD.
    // 1. if existing default method is ICCE, just skip.
    // 2. if new method is not max-specific method, just skip.
    //    existing default method can be AME or not, has no effect on final result. its okay.
    // 3. if new method is max-specific method, check whether existing default method is AME
    //   3.1  if no, set ICCE flag for existing method
    //   3.2  if yes, replace existing method with new method(new method becomes a candidate)
    if (entry->GetStatus() == CopiedMethod::Status::CONFLICT) {
        return true;
    }
    if (!IsMaxSpecificInterfaceMethod(iface, *method, itableIdx + 1, itable)) {
        return true;
    }
    if (entry->GetStatus() != CopiedMethod::Status::ABSTRACT) {
        entry->SetStatus(CopiedMethod::Status::CONFLICT);
        return true;
    }
    entry->SetStatus(CopiedMethod::Status::ORDINARY);
    vtable_.UpdateCopiedEntry(conflict, methodInfo);
    return true;
}

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_STANDARD_INL_H
