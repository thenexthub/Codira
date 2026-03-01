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

#include "libarkbase/macros.h"
#include "runtime/include/imtable_builder.h"
#include "runtime/include/class_linker.h"

namespace ark {
void IMTableBuilder::Build(const panda_file::ClassDataAccessor *cda, ITable itable)
{
    if (cda->IsInterface() || itable.Size() == 0U) {
        return;
    }

    size_t ifmNum = 0;
    for (size_t i = 0; i < itable.Size(); i++) {
        auto &entry = itable[i];
        auto methods = entry.GetMethods();
        ifmNum += methods.Size();
    }

    // set imtable size rules
    // (1) as interface methods number when it's smaller than fixed IMTABLE_SIZE
    // (2) as IMTABLE_SIZE when it's in [IMTABLE_SIZE, IMTABLE_SIZE * OVERSIZE_MULTIPLE], eg. [32,64]
    // (3) as 0 when it's much more bigger than fixed IMTABLE_SIZE, since conflict probability is high and IMTable will
    // be almost empty
    SetIMTSize((ifmNum <= Class::IMTABLE_SIZE)
                   ? ifmNum
                   : ((ifmNum <= Class::IMTABLE_SIZE * OVERSIZE_MULTIPLE) ? Class::IMTABLE_SIZE : 0));
}

void IMTableBuilder::Build(ITable itable, bool isInterface)
{
    if (isInterface || itable.Size() == 0U) {
        return;
    }

    size_t ifmNum = 0;
    for (size_t i = 0; i < itable.Size(); i++) {
        auto &entry = itable[i];
        auto methods = entry.GetMethods();
        ifmNum += methods.Size();
    }

    // set imtable size rules: the same as function above
    SetIMTSize((ifmNum <= Class::IMTABLE_SIZE)
                   ? ifmNum
                   : ((ifmNum <= Class::IMTABLE_SIZE * OVERSIZE_MULTIPLE) ? Class::IMTABLE_SIZE : 0));
}

void IMTableBuilder::UpdateClass(Class *klass)
{
    if (klass->IsInterface() || klass->IsAbstract()) {
        return;
    }

    auto imtableSize = klass->GetIMTSize();
    if (imtableSize == 0U) {
        return;
    }

    std::array<bool, Class::IMTABLE_SIZE> isMethodConflict = {false};

    auto itable = klass->GetITable();
    auto imtable = klass->GetIMT();

    for (size_t i = 0; i < itable.Size(); i++) {
        auto &entry = itable[i];
        auto itfMethods = entry.GetInterface()->GetVirtualMethods();
        auto impMethods = entry.GetMethods();

        for (size_t j = 0; j < itfMethods.Size(); j++) {
            auto impMethod = impMethods[j];
            auto itfMethodId = klass->GetIMTableIndex(itfMethods[j].GetFileId().GetOffset());
            if (!isMethodConflict.at(itfMethodId)) {
                auto ret = AddMethod(imtable, imtableSize, itfMethodId, impMethod);
                isMethodConflict[itfMethodId] = !ret;
            }
        }
    }

#ifndef NDEBUG
    DumpIMTable(klass);
#endif  // NDEBUG
}

bool IMTableBuilder::AddMethod(ark::Span<ark::Method *> imtable, [[maybe_unused]] uint32_t imtableSize, uint32_t id,
                               Method *method)
{
    bool result = false;
    ASSERT(id < imtableSize);
    if (imtable[id] == nullptr) {
        imtable[id] = method;
        result = true;
    } else {
        imtable[id] = nullptr;
    }
    return result;
}

void IMTableBuilder::DumpIMTable(Class *klass)
{
    LOG(DEBUG, CLASS_LINKER) << "imtable of class " << klass->GetName() << ":";
    auto imtable = klass->GetIMT();
    auto imtableSize = klass->GetIMTSize();
    for (size_t i = 0; i < imtableSize; i++) {
        auto method = imtable[i];
        if (method != nullptr) {
            LOG(DEBUG, CLASS_LINKER) << "[ " << i << " ] " << method->GetFullName();
        } else {
            LOG(DEBUG, CLASS_LINKER) << "[ " << i << " ] "
                                     << "FREE SLOT";
        }
    }
}

}  // namespace ark
