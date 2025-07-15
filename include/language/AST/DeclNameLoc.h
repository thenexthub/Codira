//===--- DeclNameLoc.h - Declaration Name Location Info ---------*- C++ -*-===//
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
// This file defines the DeclNameLoc class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_DECL_NAME_LOC_H
#define LANGUAGE_AST_DECL_NAME_LOC_H

#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceLoc.h"

#include "toolchain/ADT/ArrayRef.h"

class BridgedDeclNameLoc;

namespace language {

class ASTContext;

/// Source location information for a declaration name (\c DeclName)
/// written in the source.
class DeclNameLoc {
  friend class ::BridgedDeclNameLoc;

  /// Source location information.
  ///
  /// If \c NumArgumentLabels == 0, this is the SourceLoc for the base name.
  /// Otherwise, it points to an array of SourceLocs, which contains:
  /// * The base name location
  /// * The left parentheses location
  /// * The right parentheses location
  /// * The locations of each of the argument labels.
  const void *LocationInfo;

  /// The number of argument labels stored in the name.
  unsigned NumArgumentLabels;

  enum {
    BaseNameIndex = 0,
    LParenIndex = 1,
    RParenIndex = 2,
    FirstArgumentLabelIndex = 3,
  };

  /// Retrieve a pointer to either the only source location that was
  /// stored or to the array of source locations that was stored.
  SourceLoc const * getSourceLocs() const {
    if (NumArgumentLabels == 0) 
      return reinterpret_cast<SourceLoc const *>(&LocationInfo);

    return reinterpret_cast<SourceLoc const *>(LocationInfo);
  }

  DeclNameLoc(const void *LocationInfo, unsigned NumArgumentLabels)
      : LocationInfo(LocationInfo), NumArgumentLabels(NumArgumentLabels) {}

public:
  /// Create an invalid declaration name location.
  DeclNameLoc() : DeclNameLoc(nullptr, 0) {}

  /// Create declaration name location information for a base name.
  explicit DeclNameLoc(SourceLoc baseNameLoc)
      : DeclNameLoc(baseNameLoc.getOpaquePointerValue(), 0) {}

  explicit DeclNameLoc(ASTContext &ctx, SourceLoc moduleSelectorLoc,
                       SourceLoc baseNameLoc)
    : DeclNameLoc(baseNameLoc) { }

  /// Create declaration name location information for a compound
  /// name.
  DeclNameLoc(ASTContext &ctx, SourceLoc baseNameLoc,
              SourceLoc lParenLoc,
              ArrayRef<SourceLoc> argumentLabelLocs,
              SourceLoc rParenLoc);

  DeclNameLoc(ASTContext &ctx, SourceLoc moduleSelectorLoc,
              SourceLoc baseNameLoc,
              SourceLoc lParenLoc,
              ArrayRef<SourceLoc> argumentLabelLocs,
              SourceLoc rParenLoc)
    : DeclNameLoc(ctx, baseNameLoc, lParenLoc, argumentLabelLocs, rParenLoc) { }

  /// Whether the location information is valid.
  bool isValid() const { return getBaseNameLoc().isValid(); }

  /// Whether the location information is invalid.
  bool isInvalid() const { return getBaseNameLoc().isInvalid(); }

  /// Whether this was written as a compound name.
  bool isCompound() const { return NumArgumentLabels > 0; }

  /// Retrieve the location of the base name.
  SourceLoc getBaseNameLoc() const {
    return getSourceLocs()[BaseNameIndex];
  }

  /// Retrieve the location of the left parentheses.
  SourceLoc getLParenLoc() const {
    if (NumArgumentLabels == 0) return SourceLoc();
    return getSourceLocs()[LParenIndex];
  }

  /// Retrieve the location of the right parentheses.
  SourceLoc getRParenLoc() const {
    if (NumArgumentLabels == 0) return SourceLoc();
    return getSourceLocs()[RParenIndex];
  }

  /// Retrieve the location of an argument label.
  SourceLoc getArgumentLabelLoc(unsigned index) const {
    if (index >= NumArgumentLabels)
      return SourceLoc();
    return getSourceLocs()[FirstArgumentLabelIndex + index];
  }

  SourceLoc getModuleSelectorLoc() const {
    return SourceLoc();
  }

  SourceLoc getStartLoc() const {
    return getBaseNameLoc();
  }

  SourceLoc getEndLoc() const {
    return NumArgumentLabels == 0 ? getBaseNameLoc() : getRParenLoc();
  }
  
  /// Retrieve the complete source range for this declaration name.
  SourceRange getSourceRange() const {
    if (NumArgumentLabels == 0) return getBaseNameLoc();

    return SourceRange(getBaseNameLoc(), getRParenLoc());
  }
};

}

#endif // LANGUAGE_AST_DECL_NAME_LOC_H
