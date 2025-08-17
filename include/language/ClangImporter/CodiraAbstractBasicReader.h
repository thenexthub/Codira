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

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/AbstractTypeReader.h"

// This include is required to instantiate the template code in
// AbstractBasicReader.h, i.e. it's a workaround to an include-what-you-use
// violation.
#include "language/Core/AST/DeclObjC.h"

namespace language {

/// An implementation of Clang's AbstractBasicReader interface for a Codira
/// datastream-based reader.  This is paired with the AbstractBasicWriter
/// implementation in CodiraAbstractBasicWriter.h.  Note that the general
/// expectation is that the types and declarations involved will have passed
/// a serializability check when this is used for actual deserialization.
///
/// The subclass must implement:
///   uint64_t readUInt64();
///   language::Core::IdentifierInfo *readIdentifier();
///   language::Core::Stmt *readStmtRef();
///   language::Core::Decl *readDeclRef();
template <class Impl>
class DataStreamBasicReader
       : public language::Core::serialization::DataStreamBasicReader<Impl> {
  using super = language::Core::serialization::DataStreamBasicReader<Impl>;
public:
  using super::asImpl;
  using super::getASTContext;

  DataStreamBasicReader(language::Core::ASTContext &ctx) : super(ctx) {}

  /// Perform all the calls necessary to write out the given type.
  language::Core::QualType readTypeRef() {
    auto kind = language::Core::Type::TypeClass(asImpl().readUInt64());
    return language::Core::serialization::AbstractTypeReader<Impl>(asImpl()).read(kind);
  }

  bool readBool() {
    return asImpl().readUInt64() != 0;
  }

  uint32_t readUInt32() {
    return uint32_t(asImpl().readUInt64());
  }

  language::Core::Selector readSelector() {
    uint64_t numArgsPlusOne = asImpl().readUInt64();

    // The null case.
    if (numArgsPlusOne == 0)
      return language::Core::Selector();

    unsigned numArgs = unsigned(numArgsPlusOne - 1);
    SmallVector<const language::Core::IdentifierInfo *, 4> chunks;
    for (unsigned i = 0, e = std::max(numArgs, 1U); i != e; ++i)
      chunks.push_back(asImpl().readIdentifier());

    return getASTContext().Selectors.getSelector(numArgs, chunks.data());
  }

  language::Core::SourceLocation readSourceLocation() {
    // Always read null.
    return language::Core::SourceLocation();
  }

  language::Core::QualType readQualType() {
    language::Core::Qualifiers quals = asImpl().readQualifiers();
    language::Core::QualType type = asImpl().readTypeRef();
    return getASTContext().getQualifiedType(type, quals);
  }

  const language::Core::BTFTypeTagAttr *readBTFTypeTagAttr() {
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
