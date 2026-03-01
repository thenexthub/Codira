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

#ifndef PANDA_ASSEMBLER_ASSEMBLY_PROGRAM_H
#define PANDA_ASSEMBLER_ASSEMBLY_PROGRAM_H

#include <string>
#include <utility>

#include "assembly-function.h"
#include "assembly-record.h"
#include "assembly-type.h"
#include "assembly-literals.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Program {
    using StringT = std::set<std::string>;
    using RecordTableT = std::map<std::string, Record>;
    using FunctionTableT = std::map<std::string, Function>;
    using FunctionSynonymsT = std::unordered_map<std::string, std::vector<std::string>>;
    using LiteralArrayTableT = std::map<std::string, LiteralArray>;
    using ArrayTypesT = std::set<Type>;

    panda_file::SourceLang lang {panda_file::SourceLang::PANDA_ASSEMBLY};
    RecordTableT recordTable;
    FunctionTableT functionInstanceTable;
    FunctionTableT functionStaticTable;
    FunctionSynonymsT functionSynonyms;  // we might keep unordered, since we don't iterate over it
    LiteralArrayTableT literalarrayTable;
    StringT strings;
    ArrayTypesT arrayTypes;

    /*
     * Returns a JSON string with the program structure and scopes locations
     */
    PANDA_PUBLIC_API std::string JsonDump() const;

    void AddToFunctionTable(Function &&func, const std::string &name = {})
    {
        auto &table = func.IsStatic() ? functionStaticTable : functionInstanceTable;
        table.emplace(name.empty() ? std::string {func.name} : name, std::move(func));
    }

    void AddToRecordTable(Record &&record)
    {
        recordTable.emplace(std::string {record.name}, std::move(record));
    }

    void AddToLiteralArrayTable(LiteralArray::LiteralVector &&array, std::string const &name)
    {
        literalarrayTable.emplace(name, std::move(array));
    }

    std::optional<FunctionTableT::const_iterator> FindAmongAllFunctions(const std::string &name,
                                                                        bool isAmbiguous = false,
                                                                        bool isStatic = false) const
    {
        const auto &firstSearchTable = isStatic ? functionStaticTable : functionInstanceTable;
        const auto &secondSearchTable = isStatic ? functionInstanceTable : functionStaticTable;
        auto it = firstSearchTable.find(name);
        if (it == firstSearchTable.cend()) {
            if (isAmbiguous && !isStatic) {
                return std::nullopt;
            }
            it = secondSearchTable.find(name);
            if (it == secondSearchTable.cend()) {
                return std::nullopt;
            }
        }
        return it;
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_ASSEMBLY_PROGRAM_H
