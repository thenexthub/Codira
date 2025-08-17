//===--- PPConditionalDirectiveRecord.h - Preprocessing Directives-*- C++ -*-=//
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
//  This file implements the PPConditionalDirectiveRecord class, which maintains
//  a record of conditional directive regions.
//
//===----------------------------------------------------------------------===//
#include "language/Core/Lex/PPConditionalDirectiveRecord.h"
#include "toolchain/Support/Capacity.h"

using namespace language::Core;

PPConditionalDirectiveRecord::PPConditionalDirectiveRecord(SourceManager &SM)
  : SourceMgr(SM) {
  CondDirectiveStack.push_back(SourceLocation());
}

bool PPConditionalDirectiveRecord::rangeIntersectsConditionalDirective(
                                                      SourceRange Range) const {
  if (Range.isInvalid())
    return false;

  CondDirectiveLocsTy::const_iterator low = toolchain::lower_bound(
      CondDirectiveLocs, Range.getBegin(), CondDirectiveLoc::Comp(SourceMgr));
  if (low == CondDirectiveLocs.end())
    return false;

  if (SourceMgr.isBeforeInTranslationUnit(Range.getEnd(), low->getLoc()))
    return false;

  CondDirectiveLocsTy::const_iterator
    upp = std::upper_bound(low, CondDirectiveLocs.end(),
                           Range.getEnd(), CondDirectiveLoc::Comp(SourceMgr));
  SourceLocation uppRegion;
  if (upp != CondDirectiveLocs.end())
    uppRegion = upp->getRegionLoc();

  return low->getRegionLoc() != uppRegion;
}

SourceLocation PPConditionalDirectiveRecord::findConditionalDirectiveRegionLoc(
                                                     SourceLocation Loc) const {
  if (Loc.isInvalid())
    return SourceLocation();
  if (CondDirectiveLocs.empty())
    return SourceLocation();

  if (SourceMgr.isBeforeInTranslationUnit(CondDirectiveLocs.back().getLoc(),
                                          Loc))
    return CondDirectiveStack.back();

  CondDirectiveLocsTy::const_iterator low = toolchain::lower_bound(
      CondDirectiveLocs, Loc, CondDirectiveLoc::Comp(SourceMgr));
  assert(low != CondDirectiveLocs.end());
  return low->getRegionLoc();
}

void PPConditionalDirectiveRecord::addCondDirectiveLoc(
                                                      CondDirectiveLoc DirLoc) {
  // Ignore directives in system headers.
  if (SourceMgr.isInSystemHeader(DirLoc.getLoc()))
    return;

  assert(CondDirectiveLocs.empty() ||
         SourceMgr.isBeforeInTranslationUnit(CondDirectiveLocs.back().getLoc(),
                                             DirLoc.getLoc()));
  CondDirectiveLocs.push_back(DirLoc);
}

void PPConditionalDirectiveRecord::If(SourceLocation Loc,
                                      SourceRange ConditionRange,
                                      ConditionValueKind ConditionValue) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.push_back(Loc);
}

void PPConditionalDirectiveRecord::Ifdef(SourceLocation Loc,
                                         const Token &MacroNameTok,
                                         const MacroDefinition &MD) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.push_back(Loc);
}

void PPConditionalDirectiveRecord::Ifndef(SourceLocation Loc,
                                          const Token &MacroNameTok,
                                          const MacroDefinition &MD) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.push_back(Loc);
}

void PPConditionalDirectiveRecord::Elif(SourceLocation Loc,
                                        SourceRange ConditionRange,
                                        ConditionValueKind ConditionValue,
                                        SourceLocation IfLoc) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}

void PPConditionalDirectiveRecord::Elifdef(SourceLocation Loc, const Token &,
                                           const MacroDefinition &) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}
void PPConditionalDirectiveRecord::Elifdef(SourceLocation Loc, SourceRange,
                                           SourceLocation) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}

void PPConditionalDirectiveRecord::Elifndef(SourceLocation Loc, const Token &,
                                            const MacroDefinition &) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}
void PPConditionalDirectiveRecord::Elifndef(SourceLocation Loc, SourceRange,
                                            SourceLocation) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}

void PPConditionalDirectiveRecord::Else(SourceLocation Loc,
                                        SourceLocation IfLoc) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  CondDirectiveStack.back() = Loc;
}

void PPConditionalDirectiveRecord::Endif(SourceLocation Loc,
                                         SourceLocation IfLoc) {
  addCondDirectiveLoc(CondDirectiveLoc(Loc, CondDirectiveStack.back()));
  assert(!CondDirectiveStack.empty());
  CondDirectiveStack.pop_back();
}

size_t PPConditionalDirectiveRecord::getTotalMemory() const {
  return toolchain::capacity_in_bytes(CondDirectiveLocs);
}
