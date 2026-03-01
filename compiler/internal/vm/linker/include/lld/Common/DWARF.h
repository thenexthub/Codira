//===- DWARF.h --------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_DWARF_H
#define LLD_DWARF_H

#include "lld/Common/LLVM.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/DWARF/DWARFDebugLine.h"
#include <memory>
#include <string>

namespace llvm {
struct DILineInfo;
} // namespace llvm

namespace lld {

class DWARFCache {
public:
  DWARFCache(std::unique_ptr<llvm::DWARFContext> dwarf);
  std::optional<llvm::DILineInfo> getDILineInfo(uint64_t offset,
                                                uint64_t sectionIndex);
  std::optional<std::pair<std::string, unsigned>>
  getVariableLoc(StringRef name);

  llvm::DWARFContext *getContext() { return dwarf.get(); }

private:
  std::unique_ptr<llvm::DWARFContext> dwarf;
  std::vector<const llvm::DWARFDebugLine::LineTable *> lineTables;
  struct VarLoc {
    const llvm::DWARFDebugLine::LineTable *lt;
    unsigned file;
    unsigned line;
  };
  llvm::DenseMap<StringRef, VarLoc> variableLoc;
};

} // namespace lld

#endif
