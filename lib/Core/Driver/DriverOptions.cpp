//===--- DriverOptions.cpp - Driver Options Table -------------------------===//
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

#include "language/Core/Driver/Options.h"
#include "toolchain/Option/OptTable.h"
#include <cassert>

using namespace language::Core::driver;
using namespace language::Core::driver::options;
using namespace toolchain::opt;

#define OPTTABLE_STR_TABLE_CODE
#include "language/Core/Driver/Options.inc"
#undef OPTTABLE_STR_TABLE_CODE

#define OPTTABLE_VALUES_CODE
#include "language/Core/Driver/Options.inc"
#undef OPTTABLE_VALUES_CODE

#define OPTTABLE_PREFIXES_TABLE_CODE
#include "language/Core/Driver/Options.inc"
#undef OPTTABLE_PREFIXES_TABLE_CODE

#define OPTTABLE_PREFIXES_UNION_CODE
#include "language/Core/Driver/Options.inc"
#undef OPTTABLE_PREFIXES_UNION_CODE

static constexpr OptTable::Info InfoTable[] = {
#define OPTION(...) LLVM_CONSTRUCT_OPT_INFO(__VA_ARGS__),
#include "language/Core/Driver/Options.inc"
#undef OPTION
};

namespace {

class DriverOptTable : public PrecomputedOptTable {
public:
  DriverOptTable()
      : PrecomputedOptTable(OptionStrTable, OptionPrefixesTable, InfoTable,
                            OptionPrefixesUnion) {}
};
}

const toolchain::opt::OptTable &language::Core::driver::getDriverOptTable() {
  static DriverOptTable Table;
  return Table;
}
