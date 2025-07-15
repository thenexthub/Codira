//===--- Options.cpp - Option info & table --------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/Option/Options.h"

#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Option/OptTable.h"
#include "toolchain/Option/Option.h"

using namespace language::options;
using namespace toolchain::opt;

#define PREFIX(NAME, VALUE)                                                    \
  constexpr toolchain::StringLiteral NAME##_init[] = VALUE;                         \
  constexpr toolchain::ArrayRef<toolchain::StringLiteral> NAME(                          \
      NAME##_init, std::size(NAME##_init) - 1);
#include "language/Option/Options.inc"
#undef PREFIX

static const toolchain::opt::GenericOptTable::Info InfoTable[] = {
#define OPTION(...) TOOLCHAIN_CONSTRUCT_OPT_INFO(__VA_ARGS__),
#include "language/Option/Options.inc"
#undef OPTION
};

namespace {

class CodiraOptTable : public toolchain::opt::GenericOptTable {
public:
  CodiraOptTable() : GenericOptTable(InfoTable) {}
};

} // end anonymous namespace

std::unique_ptr<OptTable> language::createCodiraOptTable() {
  return std::unique_ptr<GenericOptTable>(new CodiraOptTable());
}
