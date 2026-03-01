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

#include "libabckit/src/wrappers/abcfile_wrapper.h"

#include "libpandafile/bytecode_instruction.h"
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/code_data_accessor.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/field_data_accessor.h"
#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "libpandafile/method_data_accessor.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandafile/type_helper.h"

namespace libabckit {
// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda;

FileWrapper::~FileWrapper()
{
    delete reinterpret_cast<const panda_file::File *>(abcFile_);
}

static panda_file::File::EntityId MethodCast(void *method)
{
    return panda_file::File::EntityId(reinterpret_cast<uintptr_t>(method));
}

uint32_t FileWrapper::ResolveOffsetByIndex(void *parentMethod, uint16_t index) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    return pf->ResolveOffsetByIndex(MethodCast(parentMethod), index).GetOffset();
}

size_t FileWrapper::GetMethodTotalArgumentsCount(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));

    ASSERT(!mda.IsExternal());
    panda_file::CodeDataAccessor cda(*pf, mda.GetCodeId().value());

    return cda.GetNumArgs();
}

size_t FileWrapper::GetMethodArgumentsCount([[maybe_unused]] void *caller, uint32_t id) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());

    return pda.GetNumArgs();
}

size_t FileWrapper::GetMethodRegistersCount(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));
    ASSERT(!mda.IsExternal());
    panda_file::CodeDataAccessor cda(*pf, mda.GetCodeId().value());
    return cda.GetNumVregs();
}

const uint8_t *FileWrapper::GetMethodCode(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));
    ASSERT(!mda.IsExternal());
    panda_file::CodeDataAccessor cda(*pf, mda.GetCodeId().value());
    return cda.GetInstructions();
}

size_t FileWrapper::GetMethodCodeSize(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));

    ASSERT(!mda.IsExternal());
    panda_file::CodeDataAccessor cda(*pf, mda.GetCodeId().value());

    return cda.GetCodeSize();
}

uint8_t FileWrapper::GetMethodSourceLanguage(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));

    ASSERT(!mda.IsExternal());

    auto sourceLang = mda.GetSourceLang();
    ASSERT(sourceLang.has_value());

    return static_cast<uint8_t>(sourceLang.value());
}

size_t FileWrapper::GetClassIdForMethod(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));
    return static_cast<size_t>(mda.GetClassId().GetOffset());
}

std::string FileWrapper::GetClassNameFromMethod(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));
    auto stringData = pf->GetStringData(mda.GetClassId());
    return std::string(reinterpret_cast<const char *>(stringData.data));
}

std::string FileWrapper::GetMethodName(void *method) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));
    auto stringData = pf->GetStringData(mda.GetNameId());
    return std::string(reinterpret_cast<const char *>(stringData.data));
}

void FileWrapper::EnumerateTryBlocks(void *method, const std::function<void(void *)> &cb) const
{
    auto *pf = reinterpret_cast<const panda_file::File *>(abcFile_);
    panda_file::MethodDataAccessor mda(*pf, MethodCast(method));
    panda_file::CodeDataAccessor cda(*pf, mda.GetCodeId().value());

    cda.EnumerateTryBlocks([&cb](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
        cb(reinterpret_cast<void *>(&tryBlock));
        return true;
    });
}

std::pair<int, int> FileWrapper::GetTryBlockBoundaries(void *tryBlock) const
{
    auto *tryBlockP = reinterpret_cast<panda_file::CodeDataAccessor::TryBlock *>(tryBlock);
    return {tryBlockP->GetStartPc(), tryBlockP->GetStartPc() + tryBlockP->GetLength()};
}

void FileWrapper::EnumerateCatchBlocksForTryBlock(void *tryBlock, const std::function<void(void *)> &cb) const
{
    auto *tryBlockP = reinterpret_cast<panda_file::CodeDataAccessor::TryBlock *>(tryBlock);
    tryBlockP->EnumerateCatchBlocks([&cb](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
        cb(reinterpret_cast<void *>(&catchBlock));
        return true;
    });
}

uint32_t FileWrapper::GetCatchBlockHandlerPc(void *catchBlock) const
{
    auto *tryBlockP = reinterpret_cast<panda_file::CodeDataAccessor::CatchBlock *>(catchBlock);
    return tryBlockP->GetHandlerPc();
}

}  // namespace libabckit
