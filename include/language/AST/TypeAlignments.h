//===--- TypeAlignments.h - Alignments of various Codira types ---*- C++ -*-===//
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
/// \file This file defines the alignment of various Codira AST classes.
///
/// It's useful to do this in a dedicated place to avoid recursive header
/// problems. To make sure we don't have any ODR violations, this header
/// should be included in every header that defines one of the forward-
/// declared types listed here.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_TYPEALIGNMENTS_H
#define LANGUAGE_TYPEALIGNMENTS_H

#include <cstddef>

namespace language {
  class AbstractClosureExpr;
  class AbstractSpecializeAttr;
  class AbstractStorageDecl;
  class ArchetypeType;
  class AssociatedTypeDecl;
  class ASTContext;
  class AttributeBase;
  class BraceStmt;
  class CaptureListExpr;
  class Decl;
  class DeclContext;
  class DifferentiableAttr;
  class Expr;
  class ExtensionDecl;
  class FileUnit;
  class GenericEnvironment;
  class GenericParamList;
  class GenericTypeParamDecl;
  class NominalTypeDecl;
  class NormalProtocolConformance;
  class OpaqueValueExpr;
  class OperatorDecl;
  class PackConformance;
  class Pattern;
  class ProtocolDecl;
  class ProtocolConformance;
  class RequirementRepr;
  class SILFunction;
  class SILFunctionType;
  class SpecializeAttr;
  class SpecializedAttr;
  class Stmt;
  class TrailingWhereClause;
  class TypeVariableType;
  class TypeBase;
  class TypeDecl;
  class TypeRepr;
  class ValueDecl;
  class CaseLabelItem;
  class StmtConditionElement;

  /// We frequently use three tag bits on all of these types.
  constexpr size_t AttrAlignInBits = 3;
  constexpr size_t DeclAlignInBits = 3;
  constexpr size_t DeclContextAlignInBits = 3;
  constexpr size_t ExprAlignInBits = 3;
  constexpr size_t StmtAlignInBits = 3;
  constexpr size_t TypeAlignInBits = 3;
  constexpr size_t PatternAlignInBits = 3;
  constexpr size_t TypeReprAlignInBits = 3;

  constexpr size_t SILFunctionAlignInBits = 2;
  constexpr size_t ASTContextAlignInBits = 2;
  constexpr size_t TypeVariableAlignInBits = 4;
  constexpr size_t StoredDefaultArgumentAlignInBits = 3;

  // Well, this is the *minimum* pointer alignment; it's going to be 3 on
  // 64-bit targets, but that doesn't matter.
  constexpr size_t PointerAlignInBits = 2;
}

namespace toolchain {
  /// Helper class for declaring the expected alignment of a pointer.
  /// TODO: LLVM should provide this.
  template <class T, size_t AlignInBits> struct MoreAlignedPointerTraits {
    enum { NumLowBitsAvailable = AlignInBits };
    static inline void *getAsVoidPointer(T *ptr) { return ptr; }
    static inline T *getFromVoidPointer(void *ptr) {
      return static_cast<T*>(ptr);
    }
  };

  template <class T> struct PointerLikeTypeTraits;
}

/// Declare the expected alignment of pointers to the given type.
/// This macro should be invoked from a top-level file context.
#define TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(CLASS, ALIGNMENT)     \
namespace toolchain {                                          \
template <> struct PointerLikeTypeTraits<CLASS*>          \
  : public MoreAlignedPointerTraits<CLASS, ALIGNMENT> {}; \
}

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::Decl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::AbstractStorageDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::AssociatedTypeDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::GenericTypeParamDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::OperatorDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::ProtocolDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::TypeDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::ValueDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::NominalTypeDecl, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::ExtensionDecl, language::DeclAlignInBits)

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::TypeBase, language::TypeAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::ArchetypeType, language::TypeAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::TypeVariableType, language::TypeVariableAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::SILFunctionType,
                            language::TypeVariableAlignInBits)

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::Stmt, language::StmtAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::BraceStmt, language::StmtAlignInBits)

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::ASTContext, language::ASTContextAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::DeclContext, language::DeclContextAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::FileUnit, language::DeclContextAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::DifferentiableAttr, language::PointerAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::Expr, language::ExprAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::CaptureListExpr, language::ExprAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::AbstractClosureExpr, language::ExprAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::OpaqueValueExpr, language::ExprAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::PackConformance, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::ProtocolConformance, language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::NormalProtocolConformance,
                            language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::GenericEnvironment,
                            language::DeclAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::GenericParamList,
                            language::PointerAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::Pattern,
                            language::PatternAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::RequirementRepr,
                            language::PointerAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::SILFunction,
                            language::SILFunctionAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::SpecializeAttr, language::PointerAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::SpecializedAttr, language::PointerAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::AbstractSpecializeAttr, language::PointerAlignInBits)
TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::TrailingWhereClause,
                            language::PointerAlignInBits)

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::AttributeBase, language::AttrAlignInBits)

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::TypeRepr, language::TypeReprAlignInBits)

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::CaseLabelItem, language::PatternAlignInBits)

TOOLCHAIN_DECLARE_TYPE_ALIGNMENT(language::StmtConditionElement,
                            language::PatternAlignInBits)

static_assert(alignof(void*) >= 2, "pointer alignment is too small");

#endif
