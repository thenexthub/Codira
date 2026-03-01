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

#include <algorithm>

#include "libarkbase/macros.h"
#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/logger.h"
#include "libarkfile/class_data_accessor.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/gc/gc_root.h"

namespace ark {

std::ostream &operator<<(std::ostream &os, const Class::State &state)
{
    switch (state) {
        case Class::State::INITIAL: {
            os << "INITIAL";
            break;
        }
        case Class::State::LOADED: {
            os << "LOADED";
            break;
        }
        case Class::State::VERIFIED: {
            os << "VERIFIED";
            break;
        }
        case Class::State::INITIALIZING: {
            os << "INITIALIZING";
            break;
        }
        case Class::State::ERRONEOUS: {
            os << "ERRONEOUS";
            break;
        }
        case Class::State::INITIALIZED: {
            os << "INITIALIZED";
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }

    return os;
}

mem::GCRoot BaseClass::GetGCRoot()
{
    return {mem::RootType::ROOT_CLASS, &managedObject_};
}

Class::UniqId Class::CalcUniqId(const panda_file::File *file, panda_file::File::EntityId fileId)
{
    constexpr uint64_t HALF = 32ULL;
    uint64_t uid = file->GetUniqId();
    uid <<= HALF;
    uid |= fileId.GetOffset();
    return uid;
}

Class::UniqId Class::CalcUniqId(const uint8_t *descriptor)
{
    uint64_t uid = 0;
    uid = GetHash32String(descriptor);
    constexpr uint64_t HALF = 32ULL;
    constexpr uint64_t NO_FILE = 0xFFFFFFFFULL << HALF;
    uid = NO_FILE | uid;
    return uid;
}

Class::UniqId Class::CalcUniqId() const
{
    if (pandaFile_ != nullptr) {
        return CalcUniqId(pandaFile_, fileId_);
    }
    return CalcUniqId(descriptor_);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
Class::Class(const uint8_t *descriptor, panda_file::SourceLang lang, uint32_t vtableSize, uint32_t imtSize,
             uint32_t size)
    : BaseClass(lang), descriptor_(descriptor), vtableSize_(vtableSize), imtSize_(imtSize), classSize_(size)
{
    state_ = State::INITIAL;
    // Initializa all static fields with 0 value.
    auto staticsOffset = GetStaticFieldsOffset();
    auto sp = GetClassSpan();
    ASSERT(sp.size() >= staticsOffset);
    auto sizeToSet = sp.size() - staticsOffset;
    if (sizeToSet > 0) {
        memset_s(&sp[staticsOffset], sizeToSet, 0, sizeToSet);
    }
}
void Class::SetState(Class::State state)
{
    if ((state_ == State::ERRONEOUS && state != State::ERRONEOUS) || state < state_ ||
        (state_ == State::LOADED && state == State::INITIALIZED)) {
        LOG(FATAL, RUNTIME) << "Invalid class state transition " << state_ << " -> " << state;
    }

    state_ = state;
}

std::string Class::GetName() const
{
    return ClassHelper::GetName(descriptor_);
}

void Class::DumpClass(std::ostream &os, size_t flags)
{
    if ((flags & DUMPCLASSFULLDETAILS) == 0) {
        os << GetDescriptor();
        if ((flags & DUMPCLASSCLASSLODER) != 0) {
            LOG(INFO, RUNTIME) << " Panda can't get classloader at now\n";
        }
        if ((flags & DUMPCLASSINITIALIZED) != 0) {
            LOG(INFO, RUNTIME) << " There is no status structure of class in Panda at now\n";
        }
        os << "\n";
        return;
    }
    os << "\n";
    os << "----- " << (IsInterface() ? "interface" : "class") << " "
       << "'" << GetDescriptor() << "' -----\n";
    os << "  objectSize=" << BaseClass::GetObjectSize() << " \n";
    os << "  accessFlags=" << GetAccessFlags() << " \n";
    if (IsArrayClass()) {
        os << "  componentType=" << GetComponentType() << "\n";
    }
    size_t numDirectInterfaces = this->numIfaces_;
    if (numDirectInterfaces > 0) {
        os << "  interfaces (" << numDirectInterfaces << "):\n";
    }
    if (!IsLoaded()) {
        os << "  class not yet loaded";
    } else {
        os << "  vtable (" << this->GetVTable().size() << " entries)\n";
        if (this->numSfields_ > 0) {
            os << "  static fields (" << this->numSfields_ << " entries)\n";
        }
        if (this->numFields_ - this->numSfields_ > 0) {
            os << "  instance fields (" << this->numFields_ - this->numSfields_ << " entries)\n";
        }
    }
}

void Class::CalcHaveNoRefsInParents()
{
    if (base_ == nullptr || base_->HaveNoRefClass()) {
        SetFlags(GetFlags() | NO_REFS_IN_PARENTS);
    } else {
        ASSERT(!HaveNoRefsInParents());
    }
}

Class *Class::FromClassObject(const ObjectHeader *obj)
{
    return Runtime::GetCurrent()->GetClassLinker()->ObjectToClass(obj);
}

size_t Class::GetClassObjectSizeFromClass(Class *cls, panda_file::SourceLang lang)
{
    return Runtime::GetCurrent()->GetClassLinker()->GetClassObjectSize(cls, lang);
}

}  // namespace ark
