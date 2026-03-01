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

#include "abc_file_utils.h"

#include "libarkbase/os/file.h"
#include "libarkfile/method_data_accessor.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "libarkfile/class_data_accessor.h"

namespace ark::abc2program {

static std::string_view g_globalTypeName = "_GLOBAL";

static const char ARRAY_TYPE_POSTFIX = ']';

bool AbcFileUtils::IsSystemTypeName(const std::string &typeName)
{
    bool isArrayType = typeName.back() == ARRAY_TYPE_POSTFIX;
    bool isGlobal = typeName == g_globalTypeName;

    return isArrayType || isGlobal;
}

std::string AbcFileUtils::GetFileNameByAbsolutePath(const std::string &absolutePath)
{
    LOG(DEBUG, ABC2PROGRAM) << "Absolute path is : " << absolutePath;
    size_t pos = absolutePath.find_last_of(ark::os::file::File::GetPathDelim());
    ASSERT(pos != std::string::npos);
    std::string fileName = absolutePath.substr(pos + 1);
    return fileName;
}

// Helper function to determine EntityId type
EntityType AbcFileUtils::GetEntityType(const panda_file::File &file, panda_file::File::EntityId entityId)
{
    // Check if it's external first
    if (file.IsExternal(entityId)) {
        return EntityType::EXTERNAL;
    }

    // Get the region header for this entity
    auto *regionHeader = file.GetRegionHeader(entityId);
    if (regionHeader == nullptr) {
        return EntityType::UNKNOWN;
    }

    // Check if it's a class
    auto classIndex = file.GetClassIndex(regionHeader);
    for (const auto &classId : classIndex) {
        if (classId == entityId) {
            return EntityType::CLASS;
        }
    }

    auto methodIndex = file.GetMethodIndex(regionHeader);
    for (const auto &methodId : methodIndex) {
        if (methodId == entityId) {
            return EntityType::METHOD;
        }
    }
    // Check if it's a field
    auto fieldIndex = file.GetFieldIndex(regionHeader);
    for (const auto &fieldId : fieldIndex) {
        if (fieldId == entityId) {
            return EntityType::FIELD;
        }
    }

    if (entityId.GetOffset() < file.GetHeader()->fileSize) {
        panda_file::MethodDataAccessor mda(file, entityId);
        if (mda.GetSize() > 0 && mda.GetSize() < file.GetHeader()->fileSize) {
            auto methodNameId = mda.GetNameId();
            if (methodNameId.IsValid() && methodNameId.GetOffset() < file.GetHeader()->fileSize) {
                LOG(DEBUG, ABC2PROGRAM) << "EntityId " << entityId.GetOffset() << " confirmed as METHOD";
                return EntityType::METHOD;
            }
        }
    }

    // If none of the above, it might be a string or other type
    return EntityType::STRING;
}

}  // namespace ark::abc2program
