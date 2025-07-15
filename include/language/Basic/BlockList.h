//===--- BlockList.h ---------------------------------------------*- C++ -*-===//
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
//
// This file defines some miscellaneous overloads of hash_value() and
// simple_display().
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_BLOCKLIST_H
#define LANGUAGE_BASIC_BLOCKLIST_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/StringRef.h"

namespace language {

class SourceManager;

enum class BlockListAction: uint8_t {
  Undefined = 0,
#define BLOCKLIST_ACTION(NAME) NAME,
#include "BlockListAction.def"
};

enum class BlockListKeyKind: uint8_t {
  Undefined = 0,
  ModuleName,
  ProjectName
};

class BlockListStore {
public:
  struct Implementation;
  void addConfigureFilePath(StringRef path);
  bool hasBlockListAction(StringRef key, BlockListKeyKind keyKind,
                          BlockListAction action);
  BlockListStore(SourceManager &SM);
  ~BlockListStore();
private:
  Implementation &Impl;
};

} // namespace language

#endif // LANGUAGE_BASIC_BLOCKLIST_H
