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

#ifndef ABC2PROGRAM_ABC2PROGRAM_KEY_DATA_H
#define ABC2PROGRAM_ABC2PROGRAM_KEY_DATA_H

#include <string>
#include <map>
#include <assembly-program.h>
#include "libarkfile/file.h"
#include "abc_string_table.h"

namespace ark::abc2program {

using ExternalFieldTable = std::map<std::string, std::vector<pandasm::Field>>;
using NameToIdTable = std::map<std::string, panda_file::File::EntityId>;

class Abc2ProgramKeyData {
public:
    Abc2ProgramKeyData(const panda_file::File &file, AbcStringTable &stringTable, pandasm::Program &program)
        : file_(file), stringTable_(stringTable), program_(program)
    {
    }
    const panda_file::File &GetAbcFile() const;
    std::map<panda_file::File::EntityId, const pandasm::Function *> GetFunctionTable();
    ExternalFieldTable &GetExternalFieldTable();
    NameToIdTable &GetMethodNameToIdTable();
    NameToIdTable &GetRecordNameToIdTable();
    AbcStringTable &GetAbcStringTable() const;
    pandasm::Program &GetProgram() const;
    std::string GetFullRecordNameById(const panda_file::File::EntityId &classId) const;
    std::string GetFullFunctionNameById(const panda_file::File::EntityId &methodId) const;
    ark::panda_file::SourceLang GetFileLanguage() const;
    void SetFileLanguage(ark::panda_file::SourceLang language);
    std::string GetLiteralArrayIdName(uint32_t literalArrayId);

private:
    inline bool IsSystemType(const std::string &typeName) const;

    ExternalFieldTable externalFieldTable_ {};
    std::map<std::string, panda_file::File::EntityId> recordNameToId_ {};
    std::map<std::string, panda_file::File::EntityId> methodNameToId_ {};

    const panda_file::File &file_;
    AbcStringTable &stringTable_;
    pandasm::Program &program_;
    ark::panda_file::SourceLang fileLanguage_ = panda_file::SourceLang::PANDA_ASSEMBLY;
};  // class Abc2ProgramKeyData

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_ABC2PROGRAM_KEY_DATA_H
