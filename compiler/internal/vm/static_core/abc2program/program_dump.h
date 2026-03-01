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

#ifndef ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
#define ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H

#include <iostream>
#include <assembly-program.h>
#include "abc_string_table.h"

namespace ark::abc2program {

class PandasmProgramDumper {
public:
    PandasmProgramDumper(const panda_file::File &file, const AbcStringTable &stringTable)
        : file_(&file), stringTable_(&stringTable)
    {
    }
    PandasmProgramDumper() = default;
    void Dump(std::ostream &os, const pandasm::Program &program) const;

private:
    bool HasNoAbcInput() const;
    void DumpAbcFilePath(std::ostream &os) const;
    void DumpProgramLanguage(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayTable(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayTableWithKey(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayTableWithoutKey(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArrayWithKey(std::ostream &os, const std::string &key, const pandasm::LiteralArray &litArray,
                                 const pandasm::Program &program) const;
    void DumpRecordTable(std::ostream &os, const pandasm::Program &program) const;
    void DumpRecord(std::ostream &os, const pandasm::Record &record) const;
    void DumpFieldList(std::ostream &os, const pandasm::Record &record) const;
    void DumpField(std::ostream &os, const pandasm::Field &field) const;
    bool DumpMetaData(std::ostream &os, const pandasm::ItemMetadata &meta) const;
    void DumpRecordSourceFile(std::ostream &os, const pandasm::Record &record) const;
    void DumpFunctionTable(std::ostream &os, const pandasm::Program &program) const;
    void DumpFunction(std::ostream &os, const pandasm::Function &function) const;
    void DumpInstructions(std::ostream &os, const pandasm::Function &function) const;
    void DumpStrings(std::ostream &os, const pandasm::Program &program) const;
    void DumpStringsByStringTable(std::ostream &os, const AbcStringTable &stringTable) const;
    void DumpStringsByProgram(std::ostream &os, const pandasm::Program &program) const;
    void DumpLiteralArray(std::ostream &os, const pandasm::LiteralArray &litArray,
                          const pandasm::Program &program) const;
    std::string LiteralTagToString(const panda_file::LiteralTag &tag, const pandasm::Program &program) const;
    std::string LiteralValueToString(const pandasm::LiteralArray::Literal &lit) const;
    void DumpValues(const pandasm::LiteralArray &litArray, const bool isConst, std::ostream &os,
                    const pandasm::Program &program) const;
    const panda_file::File *file_ = nullptr;
    const AbcStringTable *stringTable_ = nullptr;
};

}  // namespace ark::abc2program

#endif  // ABC2PROGRAM_PROGRANM_DUMPER_PROGRAM_DUMP_H
