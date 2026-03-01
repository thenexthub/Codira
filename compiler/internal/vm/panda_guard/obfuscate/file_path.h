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

#ifndef PANDA_GUARD_OBFUSCATE_FILE_PATH_H
#define PANDA_GUARD_OBFUSCATE_FILE_PATH_H

#include "entity.h"

namespace panda::guard {

struct FilePath {
    std::string prePart;
    std::string name;
    std::string obfName;
    std::string postPart;
};

enum class FilePathType {
    LOCAL_FILE_NO_PREFIX,
    LOCAL_FILE_WITH_PREFIX,
    EXTERNAL_DEPENDENCE,
};

class ReferenceFilePath {
public:
    explicit ReferenceFilePath(Program *program) : program_(program) {}

    void Init();

    void SetFilePath(const std::string &filePath);

    void Update();

    [[nodiscard]] std::string GetRawPath() const;

private:
    void DeterminePathType();

    void UpdateObfFilePath();

public:
    Program *program_ = nullptr;
    std::string filePath_;
    std::string obfFilePath_;
    std::string prefix_;
    std::string rawName_;
    uint32_t filePathIndex_ = 0;
    bool isRemoteFile_ = false;  // Remote har && External dependence
    FilePathType pathType_ = FilePathType::LOCAL_FILE_NO_PREFIX;
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_OBFUSCATE_FILE_PATH_H
