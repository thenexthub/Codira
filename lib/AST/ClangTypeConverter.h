//===-- ClangTypeConverter.h - Converting Codira types to C types-*- C++ -*-===//
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
//  This file defines utilities for translating Codira types to C types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_CLANG_TYPE_CONVERTER_H
#define LANGUAGE_AST_CLANG_TYPE_CONVERTER_H

#include "language/AST/ASTContext.h"
#include "language/AST/ClangModuleLoader.h"
#include "language/AST/Type.h"
#include "language/AST/TypeVisitor.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Type.h"

namespace language {

/// Compute C types corresponding to Codira AST types.
class ClangTypeConverter :
    public TypeVisitor<ClangTypeConverter, language::Core::QualType> {

  using super = TypeVisitor<ClangTypeConverter, language::Core::QualType>;

  toolchain::DenseMap<Type, language::Core::QualType> Cache;
  toolchain::DenseMap<const language::Core::Decl *, language::Decl *> ReversedExportMap;

  bool StdlibTypesAreCached = false;

  ASTContext &Context;

  language::Core::ASTContext &ClangASTContext;

  const toolchain::Triple Triple;

  ClangTypeConverter(const ClangTypeConverter &) = delete;
  ClangTypeConverter &operator=(const ClangTypeConverter &) = delete;

public:

  /// Create a ClangTypeConverter.
  ClangTypeConverter(ASTContext &ctx, language::Core::ASTContext &clangCtx,
                     toolchain::Triple triple)
    : Context(ctx), ClangASTContext(clangCtx), Triple(triple)
  {
  };

  /// Compute the C function type for a Codira function type.
  ///
  /// It is the caller's responsibility to make sure this method is only
  /// called in the appropriate context. For example, it makes sense to use
  /// this method for the output type of a @convention(c) function.
  ///
  /// Since we do not check the context, the translation is unconditional.
  /// For example, String will automatically get translated to NSString
  /// when bridging is available.
  ///
  /// Additionally, the API is expected to be used only from 
  ///
  /// \returns The appropriate clang type on success, nullptr on failure.
  ///
  /// Precondition: The representation argument must be C-compatible.
  template <bool templateArgument>
  const language::Core::Type *getFunctionType(ArrayRef<AnyFunctionType::Param> params,
                                     Type resultTy,
                                     AnyFunctionType::Representation repr);

  /// Compute the C function type for a SIL function type.
  template <bool templateArgument>
  const language::Core::Type *getFunctionType(ArrayRef<SILParameterInfo> params,
                                     std::optional<SILResultInfo> result,
                                     SILFunctionType::Representation repr);

  /// Check whether the given Clang declaration is an export of a Codira
  /// declaration introduced by this converter, and if so, return the original
  /// Codira declaration.
  Decl *getCodiraDeclForExportedClangDecl(const language::Core::Decl *decl) const;

  /// Translate Codira generic arguments to Clang C++ template arguments.
  ///
  /// \p templateArgs must be empty. \p templateParams and \p genericArgs must
  /// be equal in size.
  ///
  /// \returns nullptr if successful. If an error occurs, returns a list of
  /// types that couldn't be converted.
  std::unique_ptr<TemplateInstantiationError> getClangTemplateArguments(
      const language::Core::TemplateParameterList *templateParams,
      ArrayRef<Type> genericArgs,
      SmallVectorImpl<language::Core::TemplateArgument> &templateArgs);

private:
  enum class PointerKind {
    UnsafeMutablePointer,
    UnsafePointer,
    AutoreleasingUnsafeMutablePointer,
    Unmanaged,
    CFunctionPointer,
  };

  std::optional<PointerKind> classifyPointer(Type type);

  std::optional<unsigned> classifySIMD(Type type);

  friend ASTContext; // HACK: expose `convert` method to ASTContext

  language::Core::QualType convert(Type type);

  language::Core::QualType convertMemberType(NominalTypeDecl *DC,
                                    StringRef memberName);

  /// Convert Codira types that are used as C++ function template arguments.
  ///
  /// C++ function templates can only be instantiated with types originally
  /// imported from Clang, and a handful of builtin Codira types (e.g., integers
  /// and floats).
  language::Core::QualType convertTemplateArgument(Type type);

  language::Core::QualType convertClangDecl(Type type, const language::Core::Decl *decl);

  template <bool templateArgument>
  language::Core::QualType convertSIMDType(CanType scalarType, unsigned width);

  template <bool templateArgument>
  language::Core::QualType convertPointerType(CanType pointeeType, PointerKind kind);

  void registerExportedClangDecl(Decl *languageDecl,
                                 const language::Core::Decl *clangDecl);

  language::Core::QualType reverseImportedTypeMapping(StructType *type);
  language::Core::QualType reverseBuiltinTypeMapping(StructType *type);

  friend TypeVisitor<ClangTypeConverter, language::Core::QualType>;

  language::Core::QualType visitStructType(StructType *type);
  language::Core::QualType visitTupleType(TupleType *type);
  language::Core::QualType visitMetatypeType(MetatypeType *type);
  language::Core::QualType visitExistentialMetatypeType(ExistentialMetatypeType *type);
  language::Core::QualType visitProtocolType(ProtocolType *type);
  language::Core::QualType visitClassType(ClassType *type);
  language::Core::QualType visitBoundGenericClassType(BoundGenericClassType *type);
  language::Core::QualType visitBoundGenericType(BoundGenericType *type);
  language::Core::QualType visitEnumType(EnumType *type);
  template <bool templateArgument = false>
  language::Core::QualType visitFunctionType(FunctionType *type);
  language::Core::QualType visitProtocolCompositionType(ProtocolCompositionType *type);
  language::Core::QualType visitExistentialType(ExistentialType *type);
  language::Core::QualType visitBuiltinRawPointerType(BuiltinRawPointerType *type);
  language::Core::QualType visitBuiltinIntegerType(BuiltinIntegerType *type);
  language::Core::QualType visitBuiltinFloatType(BuiltinFloatType *type);
  language::Core::QualType visitBuiltinVectorType(BuiltinVectorType *type);
  language::Core::QualType visitArchetypeType(ArchetypeType *type);
  language::Core::QualType visitDependentMemberType(DependentMemberType *type);
  template <bool templateArgument = false>
  language::Core::QualType visitSILFunctionType(SILFunctionType *type);
  language::Core::QualType visitGenericTypeParamType(GenericTypeParamType *type);
  language::Core::QualType visitDynamicSelfType(DynamicSelfType *type);
  language::Core::QualType visitSILBlockStorageType(SILBlockStorageType *type);
  language::Core::QualType visitSugarType(SugarType *type);
  language::Core::QualType visitType(TypeBase *type);
  language::Core::QualType visit(Type type);
};

} // end namespace language

#endif /* LANGUAGE_AST_CLANG_TYPE_CONVERTER_H */
