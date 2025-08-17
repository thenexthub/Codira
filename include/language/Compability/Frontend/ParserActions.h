//===- ParserActions.h -------------------------------------------*- C++-*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_PARSER_ACTIONS_H_
#define LANGUAGE_COMPABILITY_PARSER_ACTIONS_H_

#include <string>

namespace toolchain {
class raw_string_ostream;
class raw_ostream;
class StringRef;
} // namespace toolchain

namespace language::Compability::lower {
class LoweringBridge;
} // namespace language::Compability::lower

namespace language::Compability::parser {
class Parsing;
class AllCookedSources;
} // namespace language::Compability::parser

namespace lower::pft {
class Program;
} // namespace lower::pft

//=== Frontend Parser helpers ===

namespace language::Compability::frontend {
class CompilerInstance;

parser::AllCookedSources &getAllCooked(CompilerInstance &ci);

void parseAndLowerTree(CompilerInstance &ci, lower::LoweringBridge &lb);

void dumpTree(CompilerInstance &ci);

void dumpProvenance(CompilerInstance &ci);

void dumpPreFIRTree(CompilerInstance &ci);

void formatOrDumpPrescanner(std::string &buf,
                            toolchain::raw_string_ostream &outForPP,
                            CompilerInstance &ci);

void debugMeasureParseTree(CompilerInstance &ci, toolchain::StringRef filename);

void debugUnparseNoSema(CompilerInstance &ci, toolchain::raw_ostream &out);

void debugUnparseWithSymbols(CompilerInstance &ci);

void debugUnparseWithModules(CompilerInstance &ci);

void debugDumpParsingLog(CompilerInstance &ci);
} // namespace language::Compability::frontend

#endif // FORTRAN_PARSER_ACTIONS_H_
