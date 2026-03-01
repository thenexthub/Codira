/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_QUICKENER_QUICKENER_H
#define PANDA_QUICKENER_QUICKENER_H

#include "libarkfile/file.h"
#include "libarkfile/file_item_container.h"
#include "libarkfile/file_items.h"
#include "libarkfile/class_data_accessor-inl.h"
#include <libarkfile/include/source_lang_enum.h>
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include <bytecode_instruction_enum_gen.h>
#include "libarkfile/debug_data_accessor.h"
#include <cstdint>

namespace ark::quick {
class Quickener final {
public:
    Quickener(panda_file::ItemContainer *container, panda_file::File *file,
              const std::map<panda_file::File::EntityId, panda_file::BaseItem *> *items)
        : container_(container), file_(file), items_(items) {};

    void QuickContainer();

private:
    panda_file::CodeItem *GetQuickenedCode(panda_file::CodeItem *code,
                                           const std::unordered_map<uint32_t, uint32_t> *translation_map);

    panda_file::DebugInfoItem *CreateDebugInfoItem(panda_file::File::EntityId debug_info_id);

    void UpdateDebugInfo(panda_file::DebugInfoItem *debug_info_item, panda_file::File::EntityId debug_info_id);

    panda_file::ItemContainer *container_;

    panda_file::File *file_;

    std::unordered_map<panda_file::File::EntityId, panda_file::BaseItem *> ids_done;

    const std::map<panda_file::File::EntityId, panda_file::BaseItem *> *items_;
};

#include <translation_table_gen.h>
}  // namespace ark::quick

#endif  // PANDA_QUICKENER_QUICKENER_H
