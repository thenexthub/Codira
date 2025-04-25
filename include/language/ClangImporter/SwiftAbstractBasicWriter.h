//===- SwiftAbstractBasicWriter.h - Clang serialization adapter -*- C++ -*-===//
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
// This file provides an intermediate CRTP class which implements most of
// Clang's AbstractBasicWriter interface, allowing largely the same logic
// to be used for both the importer's "can this be serialized" checks and
// the serializer's actual serialization logic.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_CLANGIMPORTER_SWIFTABSTRACTBASICWRITER_H
#define SWIFT_CLANGIMPORTER_SWIFTABSTRACTBASICWRITER_H

#include "clang/AST/AbstractTypeWriter.h"
#include "clang/AST/Type.h"

namespace language {

/// An implementation of Clang's AbstractBasicWriter interface for a Swift
/// datastream-based reader.  This is paired with the AbstractBasicReader
/// implementation in SwiftAbstractBasicReader.h.  Note that the general
/// expectation is that the types and declarations involved will have passed
/// a serializability check when this is used for actual serialization.
/// The code in this class is also used when implementing that
/// serializability check and so must be a little more cautious.
///
/// The subclass must implement:
///   void writeUInt64(uint64_t value);
///   void writeIdentifier(const clang::IdentifierInfo *ident);
///   void writeStmtRef(const clang::Stmt *stmt);
///   void writeDeclRef(const clang::Decl *decl);
template <class Impl>
class DataStreamBasicWriter
       : public clang::serialization::DataStreamBasicWriter<Impl> {
  using super = clang::serialization::DataStreamBasicWriter<Impl>;
public:
  using super::asImpl;

  DataStreamBasicWriter(clang::ASTContext &ctx) : super(ctx) {}

  /// Perform all the calls necessary to write out the given type.
  void writeTypeRef(const clang::Type *type) {
    asImpl().writeUInt64(uint64_t(type->getTypeClass()));
    clang::serialization::AbstractTypeWriter<Impl>(asImpl()).write(type);
  }

  void writeBool(bool value) {
    asImpl().writeUInt64(uint64_t(value));
  }

  void writeUInt32(uint32_t value) {
    asImpl().writeUInt64(uint64_t(value));
  }

  void writeSelector(clang::Selector selector) {
    if (selector.isNull()) {
      asImpl().writeUInt64(0);
      return;
    }

    asImpl().writeUInt64(selector.getNumArgs() + 1);
    for (unsigned i = 0, e = std::max(selector.getNumArgs(), 1U); i != e; ++i)
      asImpl().writeIdentifier(selector.getIdentifierInfoForSlot(i));
  }

  void writeSourceLocation(clang::SourceLocation loc) {
    // DataStreamBasicReader will always read null; the serializability
    // check overrides this to complain about non-null source locations.
  }

  void writeQualType(clang::QualType type) {
    assert(!type.isNull());

    auto split = type.split();
    auto qualifiers = split.Quals;

    // Unwrap BTFTagAttributeType and merge any of its qualifiers.
    while (auto btfType = dyn_cast<clang::BTFTagAttributedType>(split.Ty)) {
      split = btfType->getWrappedType().split();
      qualifiers.addQualifiers(split.Quals);
    }

    asImpl().writeQualifiers(qualifiers);
    // Just recursively visit the given type.
    asImpl().writeTypeRef(split.Ty);
  }

  void writeBTFTypeTagAttr(const clang::BTFTypeTagAttr *attr) {
    // BTFTagAttributeType is explicitly unwrapped above, so we should never
    // hit any of its attributes.
    llvm::report_fatal_error("Should never hit BTFTypeTagAttr serialization");
  }
};

}

#endif
