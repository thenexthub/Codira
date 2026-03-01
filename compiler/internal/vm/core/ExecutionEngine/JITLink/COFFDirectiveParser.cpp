//===-- COFFDirectiveParser.cpp - JITLink coff directive parser --*- C++ -*===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// MSVC COFF directive parser
//
//===----------------------------------------------------------------------===//

#include "COFFDirectiveParser.h"

using namespace vm::core;
using namespace jitlink;

#define DEBUG_TYPE "jitlink"

#define OPTTABLE_STR_TABLE_CODE
#include "COFFOptions.inc"
#undef OPTTABLE_STR_TABLE_CODE

#define OPTTABLE_PREFIXES_TABLE_CODE
#include "COFFOptions.inc"
#undef OPTTABLE_PREFIXES_TABLE_CODE

#define OPTTABLE_PREFIXES_UNION_CODE
#include "COFFOptions.inc"
#undef OPTTABLE_PREFIXES_UNION_CODE

// Create table mapping all options defined in COFFOptions.td
using namespace vm::core::opt;
static constexpr opt::OptTable::Info infoTable[] = {
#define OPTION(...)                                                            \
  LLVM_CONSTRUCT_OPT_INFO_WITH_ID_PREFIX(COFF_OPT_, __VA_ARGS__),
#include "COFFOptions.inc"
#undef OPTION
};

class COFFOptTable : public opt::PrecomputedOptTable {
public:
  COFFOptTable()
      : PrecomputedOptTable(OptionStrTable, OptionPrefixesTable, infoTable,
                            OptionPrefixesUnion, true) {}
};

static COFFOptTable optTable;

Expected<opt::InputArgList> COFFDirectiveParser::parse(StringRef Str) {
  SmallVector<StringRef, 16> Tokens;
  SmallVector<const char *, 16> Buffer;
  cl::TokenizeWindowsCommandLineNoCopy(Str, saver, Tokens);
  for (StringRef Tok : Tokens) {
    bool HasNul = Tok.end() != Str.end() && Tok.data()[Tok.size()] == '\0';
    Buffer.push_back(HasNul ? Tok.data() : saver.save(Tok).data());
  }

  unsigned missingIndex;
  unsigned missingCount;

  auto Result = optTable.ParseArgs(Buffer, missingIndex, missingCount);

  if (missingCount)
    return make_error<JITLinkError>(Twine("COFF directive parsing failed: ") +
                                    Result.getArgString(missingIndex) +
                                    " missing argument");
  LLVM_DEBUG({
    for (auto *arg : Result.filtered(COFF_OPT_UNKNOWN))
      dbgs() << "Unknown coff option argument: " << arg->getAsString(Result)
             << "\n";
  });
  return std::move(Result);
}
