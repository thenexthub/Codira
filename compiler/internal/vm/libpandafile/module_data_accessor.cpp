/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "module_data_accessor.h"
#include "file_items.h"

namespace panda::panda_file {
ModuleDataAccessor::ModuleDataAccessor(const panda_file::File &panda_file, panda_file::File::EntityId module_data_id)
    : panda_file_(panda_file), module_data_id_(module_data_id)
{
    auto sp = panda_file_.GetSpanFromId(module_data_id);

    auto module_sp = sp.SubSpan(panda_file::ID_SIZE);  // skip literalnum

    num_module_requests_ = panda_file::helpers::Read<panda_file::ID_SIZE>(&module_sp);

    for (size_t idx = 0; idx < num_module_requests_; idx++) {
        auto value = static_cast<uint32_t>(panda_file::helpers::Read<sizeof(uint32_t)>(&module_sp));
        module_requests_.emplace_back(value);
    }

    entry_data_sp_ = module_sp;
}
}  // namespace panda::panda_file