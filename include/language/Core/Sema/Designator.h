//===--- Designator.h - Initialization Designator ---------------*- C++ -*-===//
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
// This file defines interfaces used to represent designators (a la
// C99 designated initializers) during parsing.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_SEMA_DESIGNATOR_H
#define LANGUAGE_CORE_SEMA_DESIGNATOR_H

#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {

class Expr;
class IdentifierInfo;

/// Designator - A designator in a C99 designated initializer.
///
/// This class is a discriminated union which holds the various
/// different sorts of designators possible. A Designation is an array of
/// these.  An example of a designator are things like this:
///
///      [8] .field [47]        // C99 designation: 3 designators
///      [8 ... 47]  field:     // GNU extensions: 2 designators
///
/// These occur in initializers, e.g.:
///
///      int a[10] = {2, 4, [8]=9, 10};
///
class Designator {
  /// A field designator, e.g., ".x = 42".
  struct FieldDesignatorInfo {
    /// Refers to the field being initialized.
    const IdentifierInfo *FieldName;

    /// The location of the '.' in the designated initializer.
    SourceLocation DotLoc;

    /// The location of the field name in the designated initializer.
    SourceLocation FieldLoc;

    FieldDesignatorInfo(const IdentifierInfo *FieldName, SourceLocation DotLoc,
                        SourceLocation FieldLoc)
        : FieldName(FieldName), DotLoc(DotLoc), FieldLoc(FieldLoc) {}
  };

  /// An array designator, e.g., "[42] = 0".
  struct ArrayDesignatorInfo {
    Expr *Index;

    // The location of the '[' in the designated initializer.
    SourceLocation LBracketLoc;

    // The location of the ']' in the designated initializer.
    mutable SourceLocation RBracketLoc;

    ArrayDesignatorInfo(Expr *Index, SourceLocation LBracketLoc)
        : Index(Index), LBracketLoc(LBracketLoc) {}
  };

  /// An array range designator, e.g. "[42 ... 50] = 1".
  struct ArrayRangeDesignatorInfo {
    Expr *Start;
    Expr *End;

    // The location of the '[' in the designated initializer.
    SourceLocation LBracketLoc;

    // The location of the '...' in the designated initializer.
    SourceLocation EllipsisLoc;

    // The location of the ']' in the designated initializer.
    mutable SourceLocation RBracketLoc;

    ArrayRangeDesignatorInfo(Expr *Start, Expr *End, SourceLocation LBracketLoc,
                             SourceLocation EllipsisLoc)
        : Start(Start), End(End), LBracketLoc(LBracketLoc),
          EllipsisLoc(EllipsisLoc) {}
  };

  /// The kind of designator this describes.
  enum DesignatorKind {
    FieldDesignator,
    ArrayDesignator,
    ArrayRangeDesignator
  };

  DesignatorKind Kind;

  union {
    FieldDesignatorInfo FieldInfo;
    ArrayDesignatorInfo ArrayInfo;
    ArrayRangeDesignatorInfo ArrayRangeInfo;
  };

  Designator(DesignatorKind Kind) : Kind(Kind) {}

public:
  bool isFieldDesignator() const { return Kind == FieldDesignator; }
  bool isArrayDesignator() const { return Kind == ArrayDesignator; }
  bool isArrayRangeDesignator() const { return Kind == ArrayRangeDesignator; }

  //===--------------------------------------------------------------------===//
  // FieldDesignatorInfo

  /// Creates a field designator.
  static Designator CreateFieldDesignator(const IdentifierInfo *FieldName,
                                          SourceLocation DotLoc,
                                          SourceLocation FieldLoc) {
    Designator D(FieldDesignator);
    new (&D.FieldInfo) FieldDesignatorInfo(FieldName, DotLoc, FieldLoc);
    return D;
  }

  const IdentifierInfo *getFieldDecl() const {
    assert(isFieldDesignator() && "Invalid accessor");
    return FieldInfo.FieldName;
  }

  SourceLocation getDotLoc() const {
    assert(isFieldDesignator() && "Invalid accessor");
    return FieldInfo.DotLoc;
  }

  SourceLocation getFieldLoc() const {
    assert(isFieldDesignator() && "Invalid accessor");
    return FieldInfo.FieldLoc;
  }

  //===--------------------------------------------------------------------===//
  // ArrayDesignatorInfo:

  /// Creates an array designator.
  static Designator CreateArrayDesignator(Expr *Index,
                                          SourceLocation LBracketLoc) {
    Designator D(ArrayDesignator);
    new (&D.ArrayInfo) ArrayDesignatorInfo(Index, LBracketLoc);
    return D;
  }

  Expr *getArrayIndex() const {
    assert(isArrayDesignator() && "Invalid accessor");
    return ArrayInfo.Index;
  }

  SourceLocation getLBracketLoc() const {
    assert((isArrayDesignator() || isArrayRangeDesignator()) &&
           "Invalid accessor");
    return isArrayDesignator() ? ArrayInfo.LBracketLoc
                               : ArrayRangeInfo.LBracketLoc;
  }

  SourceLocation getRBracketLoc() const {
    assert((isArrayDesignator() || isArrayRangeDesignator()) &&
           "Invalid accessor");
    return isArrayDesignator() ? ArrayInfo.RBracketLoc
                               : ArrayRangeInfo.RBracketLoc;
  }

  //===--------------------------------------------------------------------===//
  // ArrayRangeDesignatorInfo:

  /// Creates a GNU array-range designator.
  static Designator CreateArrayRangeDesignator(Expr *Start, Expr *End,
                                               SourceLocation LBracketLoc,
                                               SourceLocation EllipsisLoc) {
    Designator D(ArrayRangeDesignator);
    new (&D.ArrayRangeInfo)
        ArrayRangeDesignatorInfo(Start, End, LBracketLoc, EllipsisLoc);
    return D;
  }

  Expr *getArrayRangeStart() const {
    assert(isArrayRangeDesignator() && "Invalid accessor");
    return ArrayRangeInfo.Start;
  }

  Expr *getArrayRangeEnd() const {
    assert(isArrayRangeDesignator() && "Invalid accessor");
    return ArrayRangeInfo.End;
  }

  SourceLocation getEllipsisLoc() const {
    assert(isArrayRangeDesignator() && "Invalid accessor");
    return ArrayRangeInfo.EllipsisLoc;
  }

  void setRBracketLoc(SourceLocation RBracketLoc) const {
    assert((isArrayDesignator() || isArrayRangeDesignator()) &&
           "Invalid accessor");
    if (isArrayDesignator())
      ArrayInfo.RBracketLoc = RBracketLoc;
    else
      ArrayRangeInfo.RBracketLoc = RBracketLoc;
  }
};

/// Designation - Represent a full designation, which is a sequence of
/// designators.  This class is mostly a helper for InitListDesignations.
class Designation {
  /// Designators - The actual designators for this initializer.
  SmallVector<Designator, 2> Designators;

public:
  /// AddDesignator - Add a designator to the end of this list.
  void AddDesignator(Designator D) { Designators.push_back(D); }

  bool empty() const { return Designators.empty(); }

  unsigned getNumDesignators() const { return Designators.size(); }
  const Designator &getDesignator(unsigned Idx) const {
    assert(Idx < Designators.size());
    return Designators[Idx];
  }
};

} // end namespace language::Core

#endif
