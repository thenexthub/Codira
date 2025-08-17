//===- PlistSupport.h - Plist Output Utilities ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_BASIC_PLISTSUPPORT_H
#define LANGUAGE_CORE_BASIC_PLISTSUPPORT_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/SourceManager.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>

namespace language::Core {
namespace markup {

using FIDMap = toolchain::DenseMap<FileID, unsigned>;

inline unsigned AddFID(FIDMap &FIDs, SmallVectorImpl<FileID> &V,
                   FileID FID) {
  auto [I, Inserted] = FIDs.try_emplace(FID, V.size());
  if (Inserted)
    V.push_back(FID);
  return I->second;
}

inline unsigned AddFID(FIDMap &FIDs, SmallVectorImpl<FileID> &V,
                   const SourceManager &SM, SourceLocation L) {
  FileID FID = SM.getFileID(SM.getExpansionLoc(L));
  return AddFID(FIDs, V, FID);
}

inline unsigned GetFID(const FIDMap &FIDs, FileID FID) {
  FIDMap::const_iterator I = FIDs.find(FID);
  assert(I != FIDs.end());
  return I->second;
}

inline unsigned GetFID(const FIDMap &FIDs, const SourceManager &SM,
                       SourceLocation L) {
  FileID FID = SM.getFileID(SM.getExpansionLoc(L));
  return GetFID(FIDs, FID);
}

inline raw_ostream &Indent(raw_ostream &o, const unsigned indent) {
  for (unsigned i = 0; i < indent; ++i)
    o << ' ';
  return o;
}

inline raw_ostream &EmitPlistHeader(raw_ostream &o) {
  static const char *PlistHeader =
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" "
      "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
      "<plist version=\"1.0\">\n";
  return o << PlistHeader;
}

inline raw_ostream &EmitInteger(raw_ostream &o, int64_t value) {
  o << "<integer>";
  o << value;
  o << "</integer>";
  return o;
}

inline raw_ostream &EmitString(raw_ostream &o, StringRef s) {
  o << "<string>";
  for (char c : s) {
    switch (c) {
    default:
      o << c;
      break;
    case '&':
      o << "&amp;";
      break;
    case '<':
      o << "&lt;";
      break;
    case '>':
      o << "&gt;";
      break;
    case '\'':
      o << "&apos;";
      break;
    case '\"':
      o << "&quot;";
      break;
    }
  }
  o << "</string>";
  return o;
}

inline void EmitLocation(raw_ostream &o, const SourceManager &SM,
                         SourceLocation L, const FIDMap &FM, unsigned indent) {
  if (L.isInvalid()) return;

  FullSourceLoc Loc(SM.getExpansionLoc(L), const_cast<SourceManager &>(SM));

  Indent(o, indent) << "<dict>\n";
  Indent(o, indent) << " <key>line</key>";
  EmitInteger(o, Loc.getExpansionLineNumber()) << '\n';
  Indent(o, indent) << " <key>col</key>";
  EmitInteger(o, Loc.getExpansionColumnNumber()) << '\n';
  Indent(o, indent) << " <key>file</key>";
  EmitInteger(o, GetFID(FM, SM, Loc)) << '\n';
  Indent(o, indent) << "</dict>\n";
}

inline void EmitRange(raw_ostream &o, const SourceManager &SM,
                      CharSourceRange R, const FIDMap &FM, unsigned indent) {
  if (R.isInvalid()) return;

  assert(R.isCharRange() && "cannot handle a token range");
  Indent(o, indent) << "<array>\n";
  EmitLocation(o, SM, R.getBegin(), FM, indent + 1);

  // The ".getLocWithOffset(-1)" emulates the behavior of an off-by-one bug
  // in Lexer that is already fixed. It is here for backwards compatibility
  // even though it is incorrect.
  EmitLocation(o, SM, R.getEnd().getLocWithOffset(-1), FM, indent + 1);
  Indent(o, indent) << "</array>\n";
}

} // namespace markup
} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_PLISTSUPPORT_H
