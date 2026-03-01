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

#include "abc_file_processor.h"
#include "abc_file_utils.h"
#include "abc_method_processor.h"
#include "libarkfile/annotation_data_accessor.h"
#include "libarkfile/class_data_accessor.h"

namespace ark::abc2program {
void AbcFileProcessor::GetETSMetadata(const std::map<std::string, pandasm::Function> &functionTable)
{
    for (auto &pair : functionTable) {
        if (pair.second.language == ark::panda_file::SourceLang::ETS) {
            const auto methodId = keyData_.GetMethodNameToIdTable()[pair.first];

            GetETSMetadata(const_cast<pandasm::Function *>(&pair.second), methodId);
        }
    }
}

// CC-OFFNXT(G.FUD.01) project codestyle
void AbcFileProcessor::GeteTSMetadata()
{
    LOG(DEBUG, ABC2PROGRAM) << "\n[getting ETS-specific metadata]\n";

    for (auto &pair : program_->recordTable) {
        if (pair.second.language == ark::panda_file::SourceLang::ETS) {
            const auto recordId = keyData_.GetRecordNameToIdTable()[pair.first];
            if (file_->IsExternal(recordId)) {
                continue;
            }

            GetETSMetadata(&pair.second, recordId);

            panda_file::ClassDataAccessor classAccessor(*file_, recordId);

            size_t fieldIdx = 0;
            classAccessor.EnumerateFields([&](panda_file::FieldDataAccessor &fieldAccessor) {
                GetETSMetadata(&pair.second.fieldList[fieldIdx++], fieldAccessor.GetFieldId());
            });
        }
    }
    GetETSMetadata(program_->functionStaticTable);
    GetETSMetadata(program_->functionInstanceTable);
}

void AbcFileProcessor::GetETSMetadata(pandasm::Record *record, const panda_file::File::EntityId &recordId)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting ETS metadata]\nrecord id: " << recordId;
    if (record == nullptr) {
        LOG(ERROR, ABC2PROGRAM) << "> nullptr recieved!";
        return;
    }

    panda_file::ClassDataAccessor classAccessor(*file_, recordId);
    const auto recordName = keyData_.GetFullRecordNameById(classAccessor.GetClassId());
    SetETSAttributes(record, recordId);
    AnnotationList annList {};
    LOG(DEBUG, ABC2PROGRAM) << "[getting ets annotations]\nrecord id: " << recordId;

    classAccessor.EnumerateAnnotations([&](panda_file::File::EntityId id) {
        const auto annListPart = GetETSAnnotation(id, "class");
        annList.reserve(annList.size() + annListPart.size());
        annList.insert(annList.end(), annListPart.begin(), annListPart.end());
    });

    classAccessor.EnumerateRuntimeAnnotations([&](panda_file::File::EntityId id) {
        const auto annListPart = GetETSAnnotation(id, "runtime");
        annList.reserve(annList.size() + annListPart.size());
        annList.insert(annList.end(), annListPart.begin(), annListPart.end());
    });

    for (const auto &anno : annList) {
        AbcFileEntityProcessor::SetEntityAttributeValue(
            *record, []() { return true; }, anno.first, anno.second.c_str());
    }
}

void AbcFileProcessor::SetETSAttributes(pandasm::Record *record, const panda_file::File::EntityId &recordId) const
{
    // id of std.core.Object
    [[maybe_unused]] static const size_t OBJ_ENTITY_ID = 0;

    panda_file::ClassDataAccessor classAccessor(*file_, recordId);
    uint32_t accFlags = classAccessor.GetAccessFlags();
    if ((accFlags & ACC_INTERFACE) != 0) {
        record->metadata->SetAttribute("ets.interface");
    }
    if ((accFlags & ACC_ABSTRACT) != 0) {
        record->metadata->SetAttribute("ets.abstract");
    }
    if ((accFlags & ACC_ANNOTATION) != 0) {
        record->metadata->SetAttribute("ets.annotation");
    }
    if ((accFlags & ACC_ENUM) != 0) {
        record->metadata->SetAttribute("ets.enum");
    }
    if ((accFlags & ACC_SYNTHETIC) != 0) {
        record->metadata->SetAttribute("ets.synthetic");
    }

    if (classAccessor.GetSuperClassId().GetOffset() != OBJ_ENTITY_ID) {
        const auto superClassName = keyData_.GetFullRecordNameById(classAccessor.GetSuperClassId());
        record->metadata->SetAttributeValue("ets.extends", superClassName);
    }

    classAccessor.EnumerateInterfaces([&](panda_file::File::EntityId id) {
        const auto ifaceName = keyData_.GetFullRecordNameById(id);
        record->metadata->SetAttributeValue("ets.implements", ifaceName);
    });
}

void AbcFileProcessor::GetETSMetadata(pandasm::Function *method, const panda_file::File::EntityId &methodId)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting ETS metadata]\nmethod id: " << methodId;
    if (method == nullptr) {
        LOG(ERROR, ABC2PROGRAM) << "> nullptr recieved!";
        return;
    }

    panda_file::MethodDataAccessor methodAccessor(*file_, methodId);
    SetETSAttributes(method, methodId);
    AnnotationList annList {};
    LOG(DEBUG, ABC2PROGRAM) << "[getting ETS annotations]\nmethod id: " << methodId;

    methodAccessor.EnumerateAnnotations([&](panda_file::File::EntityId id) {
        const auto annListPart = GetETSAnnotation(id, "class");
        annList.reserve(annList.size() + annListPart.size());
        annList.insert(annList.end(), annListPart.begin(), annListPart.end());
    });

    methodAccessor.EnumerateRuntimeAnnotations([&](panda_file::File::EntityId id) {
        const auto annListPart = GetETSAnnotation(id, "runtime");
        annList.reserve(annList.size() + annListPart.size());
        annList.insert(annList.end(), annListPart.begin(), annListPart.end());
    });

    for (const auto &anno : annList) {
        AbcFileEntityProcessor::SetEntityAttributeValue(
            *method, []() { return true; }, anno.first, anno.second.c_str());
    }
}

void AbcFileProcessor::SetETSAttributes(pandasm::Function *method, const panda_file::File::EntityId &methodId) const
{
    panda_file::MethodDataAccessor methodAccessor(*file_, methodId);
    uint32_t accFlags = methodAccessor.GetAccessFlags();
    if ((accFlags & ACC_ABSTRACT) != 0) {
        method->metadata->SetAttribute("ets.abstract");
    }
}

void AbcFileProcessor::GetETSMetadata(pandasm::Field *field, const panda_file::File::EntityId &fieldId)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting ETS metadata]\nfield id: " << fieldId;
    if (field == nullptr) {
        LOG(ERROR, ABC2PROGRAM) << "> nullptr recieved!";
        return;
    }

    panda_file::FieldDataAccessor fieldAccessor(*file_, fieldId);
    SetETSAttributes(field, fieldId);
    AnnotationList annList {};

    LOG(DEBUG, ABC2PROGRAM) << "[getting ETS annotations]\nfield id: " << fieldId;

    fieldAccessor.EnumerateAnnotations([&](panda_file::File::EntityId id) {
        const auto annListPart = GetETSAnnotation(id, "class");
        annList.reserve(annList.size() + annListPart.size());
        annList.insert(annList.end(), annListPart.begin(), annListPart.end());
    });

    fieldAccessor.EnumerateRuntimeAnnotations([&](panda_file::File::EntityId id) {
        const auto annListPart = GetETSAnnotation(id, "runtime");
        annList.reserve(annList.size() + annListPart.size());
        annList.insert(annList.end(), annListPart.begin(), annListPart.end());
    });

    for (const auto &anno : annList) {
        AbcFileEntityProcessor::SetEntityAttributeValue(
            *field, []() { return true; }, anno.first, anno.second.c_str());
    }
}

void AbcFileProcessor::SetETSAttributes(pandasm::Field *field, const panda_file::File::EntityId &fieldId) const
{
    panda_file::FieldDataAccessor fieldAccessor(*file_, fieldId);
    uint32_t accFlags = fieldAccessor.GetAccessFlags();
    if ((accFlags & ACC_VOLATILE) != 0) {
        field->metadata->SetAttribute("ets.volatile");
    }
    if ((accFlags & ACC_ENUM) != 0) {
        field->metadata->SetAttribute("ets.enum");
    }
    if ((accFlags & ACC_TRANSIENT) != 0) {
        field->metadata->SetAttribute("ets.transient");
    }
    if ((accFlags & ACC_SYNTHETIC) != 0) {
        field->metadata->SetAttribute("ets.synthetic");
    }
}

// CODECHECK-NOLINTNEXTLINE(C_RULE_ID_FUNCTION_SIZE)
AnnotationList AbcFileProcessor::GetETSAnnotation(const panda_file::File::EntityId &annotationId,
                                                  const std::string &type)
{
    LOG(DEBUG, ABC2PROGRAM) << "[getting ets annotation]\nid: " << annotationId;
    panda_file::AnnotationDataAccessor annotationAccessor(*file_, annotationId);
    AnnotationList annList {};
    // annotation

    const auto className = keyData_.GetFullRecordNameById(annotationAccessor.GetClassId());

    if (annotationAccessor.GetCount() == 0) {
        if (!type.empty()) {
            annList.push_back({"ets.annotation.type", type});
        }
        annList.push_back({"ets.annotation.class", className});
        annList.push_back({"ets.annotation.id", "id_" + std::to_string(annotationId.GetOffset())});
    }

    for (size_t i = 0; i < annotationAccessor.GetCount(); ++i) {
        // element
        const auto elemNameId = annotationAccessor.GetElement(i).GetNameId();
        auto elemType = AnnotationTagToString(annotationAccessor.GetTag(i).GetItem());
        const bool isArray = elemType.back() == ']';
        const auto elemCompType =
            elemType.substr(0, elemType.length() - 2);  // 2 last characters are '[' & ']' if annotation is an array
        // adding necessary annotations
        GetETSAnnotationImpl(annList, elemType, annotationAccessor.GetElement(i));
        if (!type.empty()) {
            annList.push_back({"ets.annotation.type", type});
        }

        annList.push_back({"ets.annotation.class", className});

        annList.push_back({"ets.annotation.id", "id_" + std::to_string(annotationId.GetOffset())});
        annList.push_back(
            {"ets.annotation.element.name", stringTable_->StringDataToString(file_->GetStringData(elemNameId))});
        // type
        if (isArray) {
            elemType = "array";
            annList.push_back({"ets.annotation.element.type", elemType});
            annList.push_back({"ets.annotation.element.array.component.type", elemCompType});
            // values
            const auto values = annotationAccessor.GetElement(i).GetArrayValue();
            for (size_t idx = 0; idx < values.GetCount(); ++idx) {
                annList.push_back({"ets.annotation.element.value", ArrayValueToString(values, elemCompType, idx)});
            }
        } else {
            annList.push_back({"ets.annotation.element.type", elemType});
            // value
            if (elemType != "void") {
                const auto value = annotationAccessor.GetElement(i).GetScalarValue();
                annList.push_back({"ets.annotation.element.value", ScalarValueToString(value, elemType)});
            }
        }
    }
    return annList;
}

void AbcFileProcessor::GetETSAnnotationImpl(AnnotationList &annList, const std::string &elemType,
                                            const panda_file::AnnotationDataAccessor::Elem &element)
{
    const bool isArray = elemType.back() == ']';
    const auto elemCompType = elemType.substr(0, elemType.length() - 2);
    if (elemType == "annotations") {
        const auto val = element.GetScalarValue().Get<panda_file::File::EntityId>();
        const auto annDefinition = GetETSAnnotation(val, "");
        for (const auto &elem : annDefinition) {
            annList.push_back(elem);
        }
    }
    if (isArray && elemCompType == "annotation") {
        const auto values = element.GetArrayValue();
        for (size_t idx = 0; idx < values.GetCount(); ++idx) {
            const auto val = values.Get<panda_file::File::EntityId>(idx);
            const auto annDefinition = GetETSAnnotation(val, "");
            for (const auto &elem : annDefinition) {
                annList.push_back(elem);
            }
        }
    }
}

}  // namespace ark::abc2program
