//===- ARMErrataFix.h -------------------------------------------*- C++ -*-===//
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

#ifndef LLD_ELF_ARMA8ERRATAFIX_H
#define LLD_ELF_ARMA8ERRATAFIX_H

#include "lld/Common/LLVM.h"
#include "llvm/ADT/DenseMap.h"
#include <vector>

namespace lld::elf {
struct Ctx;
class Defined;
class InputSection;
class InputSectionDescription;
class Patch657417Section;

class ARMErr657417Patcher {
public:
  ARMErr657417Patcher(Ctx &ctx) : ctx(ctx) {}
  // Return true if Patches have been added to the OutputSections.
  bool createFixes();

private:
  std::vector<Patch657417Section *>
  patchInputSectionDescription(InputSectionDescription &isd);

  void insertPatches(InputSectionDescription &isd,
                     std::vector<Patch657417Section *> &patches);

  void init();

  Ctx &ctx;
  // A cache of the mapping symbols defined by the InputSection sorted in order
  // of ascending value with redundant symbols removed. These describe
  // the ranges of code and data in an executable InputSection.
  llvm::DenseMap<InputSection *, std::vector<const Defined *>> sectionMap;

  bool initialized = false;
};

} // namespace lld::elf

#endif
