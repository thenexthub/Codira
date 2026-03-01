//===-- LVSort.cpp --------------------------------------------------------===//
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
// Support for LVObject sorting.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/LogicalView/Core/LVSort.h"
#include "vm/core/DebugInfo/LogicalView/Core/LVReader.h"
#include <string>

using namespace vm::core;
using namespace vm::core::logicalview;

#define DEBUG_TYPE "Sort"

//===----------------------------------------------------------------------===//
// Callback functions to sort objects.
//===----------------------------------------------------------------------===//
// Callback comparator based on ID.
LVSortValue toolchain::logicalview::compareID(const LVObject *LHS,
                                         const LVObject *RHS) {
  return LHS->getID() < RHS->getID();
}

// Callback comparator based on kind.
LVSortValue toolchain::logicalview::compareKind(const LVObject *LHS,
                                           const LVObject *RHS) {
  return std::string(LHS->kind()) < std::string(RHS->kind());
}

// Callback comparator based on line.
LVSortValue toolchain::logicalview::compareLine(const LVObject *LHS,
                                           const LVObject *RHS) {
  return LHS->getLineNumber() < RHS->getLineNumber();
}

// Callback comparator based on name.
LVSortValue toolchain::logicalview::compareName(const LVObject *LHS,
                                           const LVObject *RHS) {
  return LHS->getName() < RHS->getName();
}

// Callback comparator based on DIE offset.
LVSortValue toolchain::logicalview::compareOffset(const LVObject *LHS,
                                             const LVObject *RHS) {
  return LHS->getOffset() < RHS->getOffset();
}

// Callback comparator for Range compare.
LVSortValue toolchain::logicalview::compareRange(const LVObject *LHS,
                                            const LVObject *RHS) {
  if (LHS->getLowerAddress() < RHS->getLowerAddress())
    return true;

  // If the lower address is the same, use the upper address value in
  // order to put first the smallest interval.
  if (LHS->getLowerAddress() == RHS->getLowerAddress())
    return LHS->getUpperAddress() < RHS->getUpperAddress();

  return false;
}

// Callback comparator based on multiple keys (First: Kind).
LVSortValue toolchain::logicalview::sortByKind(const LVObject *LHS,
                                          const LVObject *RHS) {
  // Order in which the object attributes are used for comparison:
  // kind, name, line number, offset.
  std::tuple<std::string, StringRef, uint32_t, LVOffset> Left(
      LHS->kind(), LHS->getName(), LHS->getLineNumber(), LHS->getOffset());
  std::tuple<std::string, StringRef, uint32_t, LVOffset> Right(
      RHS->kind(), RHS->getName(), RHS->getLineNumber(), RHS->getOffset());
  return Left < Right;
}

// Callback comparator based on multiple keys (First: Line).
LVSortValue toolchain::logicalview::sortByLine(const LVObject *LHS,
                                          const LVObject *RHS) {
  // Order in which the object attributes are used for comparison:
  // line number, name, kind, offset.
  std::tuple<uint32_t, StringRef, std::string, LVOffset> Left(
      LHS->getLineNumber(), LHS->getName(), LHS->kind(), LHS->getOffset());
  std::tuple<uint32_t, StringRef, std::string, LVOffset> Right(
      RHS->getLineNumber(), RHS->getName(), RHS->kind(), RHS->getOffset());
  return Left < Right;
}

// Callback comparator based on multiple keys (First: Name).
LVSortValue toolchain::logicalview::sortByName(const LVObject *LHS,
                                          const LVObject *RHS) {
  // Order in which the object attributes are used for comparison:
  // name, line number, kind, offset.
  std::tuple<StringRef, uint32_t, std::string, LVOffset> Left(
      LHS->getName(), LHS->getLineNumber(), LHS->kind(), LHS->getOffset());
  std::tuple<StringRef, uint32_t, std::string, LVOffset> Right(
      RHS->getName(), RHS->getLineNumber(), RHS->kind(), RHS->getOffset());
  return Left < Right;
}

LVSortFunction toolchain::logicalview::getSortFunction() {
  using LVSortInfo = std::map<LVSortMode, LVSortFunction>;
  static LVSortInfo SortInfo = {
      {LVSortMode::None, nullptr},    {LVSortMode::ID, compareID},
      {LVSortMode::Kind, sortByKind}, {LVSortMode::Line, sortByLine},
      {LVSortMode::Name, sortByName}, {LVSortMode::Offset, compareOffset},
  };

  LVSortFunction SortFunction = nullptr;
  LVSortInfo::iterator Iter = SortInfo.find(options().getSortMode());
  if (Iter != SortInfo.end())
    SortFunction = Iter->second;
  return SortFunction;
}
