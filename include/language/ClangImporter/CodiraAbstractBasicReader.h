//===- CodiraAbstractBasicReader.h - Clang serialization adapter -*- C++ -*-===//
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
// Clang's AbstractBasicReader interface, paralleling the behavior defined
// in CodiraAbstractBasicWriter.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CLANGIMPORTER_LANGUAGEABSTRACTBASICREADER_H
#define LANGUAGE_CLANGIMPORTER_LANGUAGEABSTRACTBASICREADER_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/AbstractTypeReader.h"

// This include is required to instantiate the template code in
// AbstractBasicReader.h, i.e. it's a workaround to an include-what-you-use
// violation.
#include "clang/AST/DeclObjC.h"

namespace language {

/// An implementation of Clang's AbstractBasicReader interface for a Codira
/// datastream-based reader.  This is paired with the AbstractBasicWriter
/// implementation in CodiraAbstractBasicWriter.h.  Note that the general
/// expectation is that the types and declarations involved will have passed
/// a serializability check when this is used for actual deserialization.
///
/// The subclass must implement:
///   uint64_t readUInt64();
///   clang::IdentifierInfo *readIdentifier();
///   clang::Stmt *readStmtRef();
///   clang::Decl *readDeclRef();
template <class Impl>
class DataStreamBasicReader
       : public clang::serialization::DataStreamBasicReader<Impl> {
  using super = clang::serialization::DataStreamBasicReader<Impl>;
public:
  using super::asImpl;
  using super::getASTContext;

  DataStreamBasicReader(clang::ASTContext &ctx) : super(ctx) {}

  /// Perform all the calls necessary to write out the given type.
  clang::QualType readTypeRef() {
    auto kind = clang::Type::TypeClass(asImpl().readUInt64());
    return clang::serialization::AbstractTypeReader<Impl>(asImpl()).read(kind);
  }

  bool readBool() {
    return asImpl().readUInt64() != 0;
  }

  uint32_t readUInt32() {
    return uint32_t(asImpl().readUInt64());
  }

  clang::Selector readSelector() {
    uint64_t numArgsPlusOne = asImpl().readUInt64();

    // The null case.
    if (numArgsPlusOne == 0)
      return clang::Selector();

    unsigned numArgs = unsigned(numArgsPlusOne - 1);
    SmallVector<const clang::IdentifierInfo *, 4> chunks;
    for (unsigned i = 0, e = std::max(numArgs, 1U); i != e; ++i)
      chunks.push_back(asImpl().readIdentifier());

    return getASTContext().Selectors.getSelector(numArgs, chunks.data());
  }

  clang::SourceLocation readSourceLocation() {
    // Always read null.
    return clang::SourceLocation();
  }

  clang::QualType readQualType() {
    clang::Qualifiers quals = asImpl().readQualifiers();
    clang::QualType type = asImpl().readTypeRef();
    return getASTContext().getQualifiedType(type, quals);
  }

  const clang::BTFTypeTagAttr *readBTFTypeTagAttr() {
    toolchain::report_fatal_error("Read BTFTypeTagAttr that should never have been"
                             " serialized");
  }

  template<typename T>
  T *readDeclAs() {
    return asImpl().template readDeclAs<T>();
  }
};

}

#endif
