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

#include "runtime/include/vtable_builder_base-inl.h"

namespace ark {

void OnVTableConflict(ClassLinkerErrorHandler *errHandler, ClassLinker::Error error, std::string_view msg,
                      MethodInfo const *info1, MethodInfo const *info2)
{
    auto res = PandaString(msg) + " ";
    res += utf::Mutf8AsCString(info1->GetClassName());
    res += utf::Mutf8AsCString(info1->GetName().data);
    res += " ";
    res += utf::Mutf8AsCString(info2->GetClassName());
    res += utf::Mutf8AsCString(info2->GetName().data);
    LOG(DEBUG, CLASS_LINKER) << "vtable conflict: " << res;
    if (errHandler != nullptr) {
        errHandler->OnError(error, res);
    }
}

void OnVTableConflict(ClassLinkerErrorHandler *errHandler, ClassLinker::Error error, std::string_view msg,
                      Method const *info1, Method const *info2)
{
    auto res = PandaString(msg) + " ";
    res += utf::Mutf8AsCString(info1->GetClassName().data);
    res += utf::Mutf8AsCString(info1->GetName().data);
    res += " ";
    res += utf::Mutf8AsCString(info2->GetClassName().data);
    res += utf::Mutf8AsCString(info2->GetName().data);
    LOG(DEBUG, CLASS_LINKER) << "vtable conflict: " << res;
    if (errHandler != nullptr) {
        errHandler->OnError(error, res);
    }
}

void VTableInfo::DumpMappings()
{
#ifndef NDEBUG
    auto const dumpMethodInfo = [](PandaStringStream &ss, MethodInfo const *info) {
        ss << info << " " << info->GetClassName() << " " << info->GetName().data;
    };

    for (auto const &[info, entry] : vmethods_) {
        PandaStringStream ss;
        ss << "[" << entry.GetIndex() << "] ";
        dumpMethodInfo(ss, info);
        if (entry.GetCandidate() != nullptr) {
            ss << " -> ";
            dumpMethodInfo(ss, entry.GetCandidate());
        }
        LOG(DEBUG, CLASS_LINKER) << "\t" << ss.str() << "\n";
    }
#endif  // NDEBUG
}

void VTableInfo::DumpVTable([[maybe_unused]] Class *klass)
{
#ifndef NDEBUG
    LOG(DEBUG, CLASS_LINKER) << "vtable of class " << klass->GetName() << ":";
    auto vtable = klass->GetVTable();
    size_t idx = 0;
    for (auto *method : vtable) {
        LOG(DEBUG, CLASS_LINKER) << "[" << idx++ << "] " << method->GetFullName();
    }
#endif  // NDEBUG
}

void VTableInfo::UpdateClass(Class *klass) const
{
    auto vtable = klass->GetVTable();
    ASSERT(vtable.size() == GetVTableSize());

    auto const setAtIdx = [this, klass, &vtable](MethodInfo const *info, size_t idx) {
        Method *method = info->GetMethod();
        if (method == nullptr) {
            method = &klass->GetVirtualMethods()[info->GetVirtualMethodIndex()];
        } else if (info->IsInterfaceMethod() && !info->IsBase()) {
            auto it = copiedMethods_.find(info);
            ASSERT(it != copiedMethods_.end());
            method = &klass->GetCopiedMethods()[it->second.GetIndex()];
        } else if (info->IsBase()) {
            vtable[idx] = method;
            return;
        }
        method->SetVTableIndex(idx);
        vtable[idx] = method;
    };

    for (const auto &[info, entry] : vmethods_) {
        setAtIdx(entry.CandidateOr(info), entry.GetIndex());
    }
    for (const auto &[info, entry] : copiedMethods_) {
        Method *method = &klass->GetCopiedMethods()[entry.GetIndex()];
        size_t vtableIdx = vmethods_.size() + entry.GetIndex();
        method->SetVTableIndex(vtableIdx);
        vtable[vtableIdx] = method;
    }

    DumpVTable(klass);
}

}  // namespace ark
