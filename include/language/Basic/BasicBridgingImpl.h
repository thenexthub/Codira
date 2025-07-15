//===--- BasicBridgingImpl.h - header for the language BasicBridging module --===//
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

#ifndef LANGUAGE_BASIC_BASICBRIDGINGIMPL_H
#define LANGUAGE_BASIC_BASICBRIDGINGIMPL_H

#include "language/Basic/Assertions.h"
#include "language/Basic/SourceLoc.h"
#include "toolchain/ADT/StringRef.h"

LANGUAGE_BEGIN_NULLABILITY_ANNOTATIONS

void ASSERT_inBridgingHeader(bool condition) { ASSERT(condition); }

//===----------------------------------------------------------------------===//
// MARK: BridgedStringRef
//===----------------------------------------------------------------------===//

BridgedStringRef::BridgedStringRef(toolchain::StringRef sref)
    : Data(sref.data()), Length(sref.size()) {}

toolchain::StringRef BridgedStringRef::unbridged() const {
  return toolchain::StringRef(Data, Length);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedOwnedString
//===----------------------------------------------------------------------===//

toolchain::StringRef BridgedOwnedString::unbridgedRef() const { return toolchain::StringRef(Data, Length); }

//===----------------------------------------------------------------------===//
// MARK: BridgedSourceLoc
//===----------------------------------------------------------------------===//

BridgedSourceLoc::BridgedSourceLoc(language::SourceLoc loc)
  : Raw(loc.getOpaquePointerValue()) {}

language::SourceLoc BridgedSourceLoc::unbridged() const {
  return language::SourceLoc(
      toolchain::SMLoc::getFromPointer(static_cast<const char *>(Raw)));
}

BridgedSourceLoc BridgedSourceLoc::advancedBy(size_t n) const {
  return BridgedSourceLoc(unbridged().getAdvancedLoc(n));
}

//===----------------------------------------------------------------------===//
// MARK: BridgedSourceRange
//===----------------------------------------------------------------------===//

BridgedSourceRange::BridgedSourceRange(language::SourceRange range)
    : Start(range.Start), End(range.End) {}

language::SourceRange BridgedSourceRange::unbridged() const {
  return language::SourceRange(Start.unbridged(), End.unbridged());
}

//===----------------------------------------------------------------------===//
// MARK: BridgedCharSourceRange
//===----------------------------------------------------------------------===//

BridgedCharSourceRange::BridgedCharSourceRange(language::CharSourceRange range)
    : Start(range.getStart()), ByteLength(range.getByteLength()) {}

language::CharSourceRange BridgedCharSourceRange::unbridged() const {
  return language::CharSourceRange(Start.unbridged(), ByteLength);
}

//===----------------------------------------------------------------------===//
// MARK: BridgedCodiraVersion
//===----------------------------------------------------------------------===//

BridgedCodiraVersion::BridgedCodiraVersion(CodiraInt major, CodiraInt minor)
    : Major(major), Minor(minor) {
  ASSERT(major >= 0 && minor >= 0);
  ASSERT(major == Major && minor == Minor);
}

extern "C" void
language_ASTGen_bridgedCodiraClosureCall_1(const void *_Nonnull closure,
                                       const void *_Nullable arg1);

void BridgedCodiraClosure::operator()(const void *_Nullable arg1) {
#if LANGUAGE_BUILD_LANGUAGE_SYNTAX
  language_ASTGen_bridgedCodiraClosureCall_1(closure, arg1);
#else
  toolchain_unreachable("Must not be used in C++-only build");
#endif
}

LANGUAGE_END_NULLABILITY_ANNOTATIONS

#endif // LANGUAGE_BASIC_BASICBRIDGINGIMPL_H
