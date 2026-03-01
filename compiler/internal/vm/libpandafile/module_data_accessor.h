/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef LIBPANDAFILE_MODULE_DATA_ACCESSOR_H
#define LIBPANDAFILE_MODULE_DATA_ACCESSOR_H

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

#include "macros.h"
#include "utils/span.h"
#include "file.h"

namespace panda::panda_file {
using StringData = panda_file::File::StringData;

enum class ModuleTag : uint8_t {
    REGULAR_IMPORT = 0x00,
    NAMESPACE_IMPORT = 0x01,
    LOCAL_EXPORT = 0x02,
    INDIRECT_EXPORT = 0x03,
    STAR_EXPORT = 0x04,
};

class ModuleDataAccessor {
public:
    PANDA_PUBLIC_API ModuleDataAccessor(const panda_file::File &panda_file, panda_file::File::EntityId module_data_id);
    ~ModuleDataAccessor() = default;
    DEFAULT_MOVE_CTOR(ModuleDataAccessor);
    DEFAULT_COPY_CTOR(ModuleDataAccessor);
    NO_MOVE_OPERATOR(ModuleDataAccessor);
    NO_COPY_OPERATOR(ModuleDataAccessor);

    template <class Callback>
    inline void EnumerateModuleRecord(const Callback &cb);

    const panda_file::File &GetPandaFile() const
    {
        return panda_file_;
    }

    panda_file::File::EntityId GetModuleDataId() const
    {
        return module_data_id_;
    }

    const std::vector<uint32_t> &getRequestModules() const
    {
        return module_requests_;
    }

    using ModuleValue = std::variant<uint32_t, StringData>;

private:
    const panda_file::File &panda_file_;
    panda_file::File::EntityId module_data_id_;
    uint32_t num_module_requests_;
    std::vector<uint32_t> module_requests_;
    Span<const uint8_t> entry_data_sp_ {nullptr, nullptr};
};
}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_MODULE_DATA_ACCESSOR_H
