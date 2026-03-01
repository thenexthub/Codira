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

#ifndef LIBPANDAFILE_FILE_READER_H
#define LIBPANDAFILE_FILE_READER_H

#include <type_traits>
#include "annotation_data_accessor.h"
#include "bytecode_instruction.h"
#include "class_data_accessor.h"
#include "code_data_accessor.h"
#include "debug_data_accessor.h"
#include "field_data_accessor.h"
#include "file_item_container.h"
#include "libpandafile/helpers.h"
#include "literal_data_accessor.h"
#include "method_data_accessor.h"
#include "method_handle_data_accessor.h"
#include "os/file.h"
#include "param_annotations_data_accessor.h"
#include "proto_data_accessor.h"
#include "utils/pandargs.h"
#include "utils/span.h"
#include "utils/type_helpers.h"
#include "utils/leb128.h"
#if !PANDA_TARGET_WINDOWS
#include "securec.h"
#endif

#include <cstdint>
#include <cerrno>

#include <limits>
#include <vector>

namespace panda::panda_file {

class FileReader {
public:
    // default methods
    explicit FileReader(std::unique_ptr<const File> &&file) : file_(std::move(file)) {}
    virtual ~FileReader() = default;

    bool ReadContainer();

    ItemContainer *GetContainerPtr()
    {
        return &container_;
    }

    void ComputeLayoutAndUpdateIndices();

    NO_COPY_SEMANTIC(FileReader);
    NO_MOVE_SEMANTIC(FileReader);

private:
    bool ReadLiteralArrayItems();
    bool ReadIndexHeaders();
    bool ReadClasses();

    bool CreateLiteralArrayItem(const LiteralDataAccessor::LiteralValue &lit_value, const LiteralTag &tag,
                                File::EntityId array_id);
    AnnotationItem *CreateAnnotationItem(File::EntityId ann_id);
    MethodItem *CreateMethodItem(ClassItem *cls, File::EntityId method_id);
    ForeignMethodItem *CreateForeignMethodItem(BaseClassItem *fcls, File::EntityId method_id);
    FieldItem *CreateFieldItem(ClassItem *cls, File::EntityId field_id);
    ForeignFieldItem *CreateForeignFieldItem(BaseClassItem *fcls, File::EntityId field_id);
    ClassItem *CreateClassItem(File::EntityId class_id);
    ForeignClassItem *CreateForeignClassItem(File::EntityId class_id);
    MethodHandleItem *CreateMethodHandleItem(File::EntityId mh_id);
    TypeItem *CreateParamTypeItem(ProtoDataAccessor *proto_acc, size_t param_num, size_t reference_num);
    TypeItem *GetReturnTypeItem(Type type, size_t &reference_num);
    std::vector<MethodParamItem> CreateMethodParamItems(ProtoDataAccessor *proto_acc, MethodDataAccessor *method_acc,
                                                        size_t reference_num);
    DebugInfoItem *CreateDebugInfoItem(File::EntityId debug_info_id);
    void EnumerateBlocks(MethodDataAccessor method_acc, MethodItem *method_item);
    void UpdateDebugInfoDependecies(File::EntityId debug_info_id);
    void UpdateDebugInfo(DebugInfoItem *debug_info_item, File::EntityId debug_info_id);
    void CreateSuperClassItem(ClassDataAccessor& class_acc, ClassItem* class_item, const std::string& class_name);
    void AddIndexDependencyInstFlag(CodeItem *code_item, MethodItem *method_item,
                                    const std::unordered_map<File::EntityId, File::EntityId> &reverse_done);
    void UpdateIdInstFlag(CodeItem *code_item, MethodItem *method_item,
                          const std::unordered_map<File::EntityId, File::EntityId> &reverse_done);

    template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    void SetIntegerFieldValue(FieldDataAccessor *field_acc, FieldItem *field_item)
    {
        auto value = field_acc->GetValue<T>();
        if (!value) {
            return;
        }

        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (is_same_v<T, int64_t> || is_same_v<T, uint64_t>) {
            auto *value_item = container_.GetOrCreateLongValueItem(value.value());
            field_item->SetValue(value_item);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            auto *value_item = container_.GetOrCreateIntegerValueItem(value.value());
            field_item->SetValue(value_item);
        }
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    void SetFloatFieldValue(FieldDataAccessor *field_acc, FieldItem *field_item)
    {
        auto value = field_acc->GetValue<T>();
        if (!value) {
            return;
        }

        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (is_same_v<T, double>) {
            auto *value_item = container_.GetOrCreateDoubleValueItem(value.value());
            field_item->SetValue(value_item);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            auto *value_item = container_.GetOrCreateFloatValueItem(value.value());
            field_item->SetValue(value_item);
        }
    }

    void SetStringFieldValue(FieldDataAccessor *field_acc, FieldItem *field_item)
    {
        auto value = field_acc->GetValue<uint32_t>();
        if (value) {
            panda_file::File::EntityId string_id(value.value());
            auto data = file_->GetStringData(string_id);
            std::string string_data(reinterpret_cast<const char *>(data.data));
            auto *string_item = container_.GetOrCreateStringItem(string_data);
            auto *value_item = container_.GetOrCreateIdValueItem(string_item);
            field_item->SetValue(value_item);
        }
    }

    // Creates foreign or non-foreign method item
    inline BaseItem *CreateGenericMethodItem(BaseClassItem *class_item, File::EntityId method_id)
    {
        if (file_->IsExternal(method_id)) {
            return CreateForeignMethodItem(class_item, method_id);
        }
        return CreateMethodItem(static_cast<ClassItem *>(class_item), method_id);
    }

    // Creates foreign or non-foreign field item
    inline BaseItem *CreateGenericFieldItem(BaseClassItem *class_item, File::EntityId field_id)
    {
        if (file_->IsExternal(field_id)) {
            return CreateForeignFieldItem(class_item, field_id);
        }
        return CreateFieldItem(static_cast<ClassItem *>(class_item), field_id);
    }

    // Creates foreign or non-foreign class item
    inline BaseClassItem *CreateGenericClassItem(File::EntityId class_id)
    {
        if (file_->IsExternal(class_id)) {
            return CreateForeignClassItem(class_id);
        }
        return CreateClassItem(class_id);
    }

    void UpdateCodeAndDebugInfoDependencies(const std::map<BaseItem *, File::EntityId> &reverse_done);

    std::unique_ptr<const File> file_;
    ItemContainer container_;
    std::map<File::EntityId, BaseItem *> items_done_;
};

}  // namespace panda::panda_file

#endif  // LIBPANDAFILE_FILE_READER_H
