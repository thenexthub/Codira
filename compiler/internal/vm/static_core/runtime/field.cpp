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

#include "runtime/include/field.h"

#include "libarkbase/utils/hash.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/file-inl.h"
#include "runtime/include/runtime.h"

namespace ark {

panda_file::File::StringData Field::GetName() const
{
    const auto *pandaFile = GetPandaFile();
    auto nameId = panda_file::FieldDataAccessor::GetNameId(*pandaFile, fileId_);
    return pandaFile->GetStringData(nameId);
}

Class *Field::ResolveTypeClass(ClassLinkerErrorHandler *errorHandler) const
{
    const auto *pandaFile = GetPandaFile();
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();

    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*GetClass());
    auto *ext = classLinker->GetExtension(ctx);
    ASSERT(ext != nullptr);

    switch (GetTypeId()) {
        case panda_file::Type::TypeId::U1:
            return ext->GetClassRoot(ClassRoot::U1);
        case panda_file::Type::TypeId::I8:
            return ext->GetClassRoot(ClassRoot::I8);
        case panda_file::Type::TypeId::U8:
            return ext->GetClassRoot(ClassRoot::U8);
        case panda_file::Type::TypeId::I16:
            return ext->GetClassRoot(ClassRoot::I16);
        case panda_file::Type::TypeId::U16:
            return ext->GetClassRoot(ClassRoot::U16);
        case panda_file::Type::TypeId::I32:
            return ext->GetClassRoot(ClassRoot::I32);
        case panda_file::Type::TypeId::U32:
            return ext->GetClassRoot(ClassRoot::U32);
        case panda_file::Type::TypeId::I64:
            return ext->GetClassRoot(ClassRoot::I64);
        case panda_file::Type::TypeId::U64:
            return ext->GetClassRoot(ClassRoot::U64);
        case panda_file::Type::TypeId::F32:
            return ext->GetClassRoot(ClassRoot::F32);
        case panda_file::Type::TypeId::F64:
            return ext->GetClassRoot(ClassRoot::F64);
        case panda_file::Type::TypeId::TAGGED:
            return ext->GetClassRoot(ClassRoot::TAGGED);
        case panda_file::Type::TypeId::REFERENCE:
            return ext->GetClass(*pandaFile, panda_file::FieldDataAccessor::GetTypeId(*pandaFile, fileId_),
                                 GetClass()->GetLoadContext(), errorHandler);
        default:
            UNREACHABLE();
    }
}

const panda_file::File *Field::GetPandaFile() const
{
    return GetClass()->GetPandaFile();
}

}  // namespace ark
