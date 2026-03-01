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
#ifndef LIBPANDAFILE_FILE_READER_H_
#define LIBPANDAFILE_FILE_READER_H_

#include <type_traits>
#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/class_data_accessor.h"
#include "libarkfile/code_data_accessor.h"
#include "libarkfile/debug_data_accessor.h"
#include "libarkfile/field_data_accessor.h"
#include "libarkfile/file_item_container.h"
#include "libarkfile/helpers.h"
#include "libarkfile/literal_data_accessor.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/method_handle_data_accessor.h"
#include "libarkbase/os/file.h"
#include "libarkfile/param_annotations_data_accessor.h"
#include "libarkfile/proto_data_accessor.h"
#include "libarkbase/utils/pandargs.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/type_helpers.h"
#include "libarkbase/utils/leb128.h"
#if !PANDA_TARGET_WINDOWS
#include "securec.h"
#endif

#include <cstdint>
#include <cerrno>

#include <limits>
#include <vector>

namespace ark::panda_file {

class FileReader {
public:
    // default methods
    explicit FileReader(std::unique_ptr<const File> &&file) : file_(std::move(file)) {}
    virtual ~FileReader() = default;

    bool ReadContainer(bool shouldRebuildIndices = true);

    ItemContainer *GetContainerPtr()
    {
        return &container_;
    }

    const File *GetFilePtr() const
    {
        return file_.get();
    }

    const std::map<File::EntityId, BaseItem *> *GetItems() const
    {
        return &itemsDone_;
    }

    void ComputeLayoutAndUpdateIndices();

    NO_COPY_SEMANTIC(FileReader);
    NO_MOVE_SEMANTIC(FileReader);

private:
    bool ReadLiteralArrayItems();
    bool ReadRegionHeaders();
    bool ReadClasses();

    void EmplaceLiteralVals(std::vector<panda_file::LiteralItem> &literalArray,
                            const panda_file::LiteralDataAccessor::LiteralValue &value,
                            const panda_file::LiteralTag &tag);
    bool CreateLiteralArrayItem(LiteralDataAccessor *litArrayAccessor, File::EntityId arrayId, uint32_t index);
    ValueItem *SetElemValueItem(AnnotationDataAccessor::Tag &annTag, AnnotationDataAccessor::Elem &annElem);
    AnnotationItem *CreateAnnotationItem(File::EntityId annId);
    BaseClassItem *GetCatchTypeItem(CodeDataAccessor::CatchBlock &catchBlock, File::EntityId methodId,
                                    MethodItem *methodItem);
    void SetMethodCodeIfPresent(std::optional<File::EntityId> &codeId, MethodItem *methodItem,
                                File::EntityId &methodId);
    TypeItem *SetRetType(ProtoDataAccessor &protoAcc, size_t &referenceNum);
    MethodItem *CreateMethodItem(ClassItem *cls, File::EntityId methodId);
    ForeignMethodItem *CreateForeignMethodItem(BaseClassItem *fcls, File::EntityId methodId);
    void SetFieldValue(FieldItem *fieldItem, Type fieldType, FieldDataAccessor &fieldAcc);
    FieldItem *CreateFieldItem(ClassItem *cls, File::EntityId fieldId);
    ForeignFieldItem *CreateForeignFieldItem(BaseClassItem *fcls, File::EntityId fieldId);
    BaseItem *CheckAndGetExistingFileItem(File::EntityId id, ItemTypes itemType);
    ClassItem *CreateClassItem(File::EntityId classId);
    ForeignClassItem *CreateForeignClassItem(File::EntityId classId);
    MethodHandleItem *CreateMethodHandleItem(File::EntityId mhId);
    TypeItem *CreateParamTypeItem(ProtoDataAccessor *protoAcc, size_t paramNum, size_t referenceNum);
    std::vector<MethodParamItem> CreateMethodParamItems(ProtoDataAccessor *protoAcc, MethodDataAccessor *methodAcc,
                                                        size_t referenceNum);
    DebugInfoItem *CreateDebugInfoItem(File::EntityId debugInfoId);
    void UpdateDebugInfoDependecies(File::EntityId debugInfoId);
    void UpdateDebugInfo(DebugInfoItem *debugInfoItem, File::EntityId debugInfoId);

    void InstUpdateId(CodeItem *codeItem, MethodItem *methodItem, std::map<BaseItem *, File::EntityId> &reverseDone);

    bool TryCreateMethodItem(File::EntityId methodId);
    bool TryCreateFieldItem(File::EntityId fieldId);

    template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
    void SetIntegerFieldValue(FieldDataAccessor *fieldAcc, FieldItem *fieldItem)
    {
        auto value = fieldAcc->GetValue<T>();
        if (!value) {
            return;
        }

        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (is_same_v<T, int64_t> || is_same_v<T, uint64_t>) {
            auto *valueItem = container_.GetOrCreateLongValueItem(value.value());
            fieldItem->SetValue(valueItem);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            auto *valueItem = container_.GetOrCreateIntegerValueItem(value.value());
            fieldItem->SetValue(valueItem);
        }
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
    void SetFloatFieldValue(FieldDataAccessor *fieldAcc, FieldItem *fieldItem)
    {
        auto value = fieldAcc->GetValue<T>();
        if (!value) {
            return;
        }

        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (is_same_v<T, double>) {
            auto *valueItem = container_.GetOrCreateDoubleValueItem(value.value());
            fieldItem->SetValue(valueItem);
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            auto *valueItem = container_.GetOrCreateFloatValueItem(value.value());
            fieldItem->SetValue(valueItem);
        }
    }

    void SetStringFieldValue(FieldDataAccessor *fieldAcc, FieldItem *fieldItem)
    {
        auto value = fieldAcc->GetValue<uint32_t>();
        if (value) {
            panda_file::File::EntityId stringId(value.value());
            auto data = file_->GetStringData(stringId);
            std::string stringData(reinterpret_cast<const char *>(data.data));
            auto *stringItem = container_.GetOrCreateStringItem(stringData);
            auto *valueItem = container_.GetOrCreateIdValueItem(stringItem);
            fieldItem->SetValue(valueItem);
        }
    }

    // Creates foreign or non-foreign method item
    inline BaseItem *CreateGenericMethodItem(BaseClassItem *classItem, File::EntityId methodId)
    {
        if (file_->IsExternal(methodId)) {
            return CreateForeignMethodItem(classItem, methodId);
        }
        LOG_IF(classItem->IsForeign(), FATAL, PANDAFILE) << "Non-foreign methods should not belong to foreign class.";
        return CreateMethodItem(static_cast<ClassItem *>(classItem), methodId);
    }

    // Creates foreign or non-foreign field item
    inline BaseItem *CreateGenericFieldItem(BaseClassItem *classItem, File::EntityId fieldId)
    {
        if (file_->IsExternal(fieldId)) {
            return CreateForeignFieldItem(classItem, fieldId);
        }
        LOG_IF(classItem->IsForeign(), FATAL, PANDAFILE) << "Non-foreign fields should not belong to foreign class.";
        return CreateFieldItem(static_cast<ClassItem *>(classItem), fieldId);
    }

    // Creates foreign or non-foreign class item
    inline BaseClassItem *CreateGenericClassItem(File::EntityId classId)
    {
        if (file_->IsExternal(classId)) {
            return CreateForeignClassItem(classId);
        }
        return CreateClassItem(classId);
    }

    void InstCheckByFlags(BytecodeInstruction &inst, MethodItem *methodItem,
                          const std::map<BaseItem *, File::EntityId> &reverseDone);
    void UpdateCodeAndDebugInfoDependencies(const std::map<BaseItem *, File::EntityId> &reverseDone);

    std::unique_ptr<const File> file_;
    ItemContainer container_;
    std::map<File::EntityId, BaseItem *> itemsDone_;
};

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_FILE_READER_H_
