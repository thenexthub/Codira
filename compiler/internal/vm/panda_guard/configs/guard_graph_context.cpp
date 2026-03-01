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

#include "guard_graph_context.h"

#include "assembly-type.h"
#include "mem/pool_manager.h"
#include "compiler/compiler_options.h"
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/file-inl.h"
#include "libpandafile/method_data_accessor.h"

#include "util/assert_util.h"

namespace {
constexpr std::string_view TAG = "[Guard_Graph]";
constexpr uint32_t MAX_SIZE = ~0U;
}  // namespace

void panda::guard::GraphContext::Init()
{
    PoolManager::Initialize(PoolType::MALLOC);

    auto records = file_.GetClasses();
    for (auto id : records) {
        panda_file::File::EntityId recordId {id};
        if (file_.IsExternal(recordId)) {
            continue;
        }
        std::string name = GetStringById(recordId);
        pandasm::Type type = pandasm::Type::FromDescriptor(name);
        std::string recordFullName = type.GetName();
        recordNameMap_.emplace(recordFullName, id);
    }

    panda::compiler::options.SetCompilerUseSafepoint(false);
    panda::compiler::options.SetCompilerMaxBytecodeSize(MAX_SIZE);
    panda::compiler::options.SetCompilerMaxVregsNum(MAX_SIZE);

    LOG(INFO, PANDAGUARD) << TAG << "[graph context init success]";
}

void panda::guard::GraphContext::Finalize()
{
    PoolManager::Finalize();
    LOG(INFO, PANDAGUARD) << TAG << "[graph context finalize success]";
}

const panda::panda_file::File &panda::guard::GraphContext::GetAbcFile() const
{
    return file_;
}

uint32_t panda::guard::GraphContext::FillMethodPtr(const std::string &recordName, const std::string &rawName) const
{
    auto it = recordNameMap_.find(recordName);
    PANDA_GUARD_ASSERT_PRINT(it == recordNameMap_.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "can not find record: " << recordName);

    uint32_t methodId = 0;
    uint32_t id = it->second;
    panda_file::File::EntityId recordId {id};
    panda_file::ClassDataAccessor cda {this->GetAbcFile(), recordId};
    cda.EnumerateMethods([this, rawName, &methodId](const panda_file::MethodDataAccessor &mda) {
        if (mda.IsExternal()) {
            return;
        }
        std::string methodNameRaw = this->GetStringById(mda.GetNameId());
        if (methodNameRaw == rawName) {
            methodId = mda.GetMethodId().GetOffset();
        }
    });
    return methodId;
}

std::string panda::guard::GraphContext::GetStringById(const panda_file::File::EntityId &entity_id) const
{
    panda_file::File::StringData sd = file_.GetStringData(entity_id);
    return (reinterpret_cast<const char *>(sd.data));
}
