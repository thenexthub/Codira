//===--- ASTDemangler.h - Codira AST symbol demangling -----------*- C++ -*-===//
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
// Defines a builder concept for the TypeDecoder and MetadataReader which builds
// AST Types, and a utility function wrapper which takes a mangled string and
// feeds it through the TypeDecoder instance.
//
// The RemoteAST library defines a MetadataReader instance that uses this
// concept, together with some additional utilities.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_ASTDEMANGLER_H
#define LANGUAGE_AST_ASTDEMANGLER_H

#include "language/AST/ASTContext.h"
#include "language/AST/Types.h"
#include "language/Demangling/Demangler.h"
#include "language/Demangling/NamespaceMacros.h"
#include "language/Demangling/TypeDecoder.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include <optional>

namespace language {
 
class TypeDecl;

namespace Demangle {
LANGUAGE_BEGIN_INLINE_NAMESPACE

Type getTypeForMangling(ASTContext &ctx,
                        toolchain::StringRef mangling,
                        GenericSignature genericSig=GenericSignature());

TypeDecl *getTypeDeclForMangling(ASTContext &ctx,
                                 toolchain::StringRef mangling,
                                 GenericSignature genericSig=GenericSignature());

TypeDecl *getTypeDeclForUSR(ASTContext &ctx,
                            toolchain::StringRef usr,
                            GenericSignature genericSig=GenericSignature());

/// An implementation of MetadataReader's BuilderType concept that
/// just finds and builds things in the AST.
class ASTBuilder {
  ASTContext &Ctx;
  Mangle::ManglingFlavor ManglingFlavor;
  Demangle::NodeFactory Factory;

  /// The notional context in which we're writing and type-checking code.
  /// Created lazily.
  DeclContext *NotionalDC = nullptr;

  /// The depth and index of each parameter pack in the current generic
  /// signature. We need this because the mangling for a type parameter
  /// doesn't record whether it is a pack or not; we find the correct
  /// depth and index in this array, and use its pack-ness.
  toolchain::SmallVector<std::pair<unsigned, unsigned>, 2> ParameterPacks;

  /// For saving and restoring generic parameters.
  toolchain::SmallVector<decltype(ParameterPacks), 2> ParameterPackStack;

  /// The depth and index of each value parameter in the current generic
  /// signature. We need this becasue the mangling for a type parameter
  /// doesn't record whether it is a value or not; we find the correct
  /// depth and index in this array, and use its value-ness.
  toolchain::SmallVector<std::tuple<std::pair<unsigned, unsigned>, Type>, 1> ValueParameters;

  /// For saving and restoring generic parameters.
  toolchain::SmallVector<decltype(ValueParameters), 1> ValueParametersStack;

  /// This builder doesn't perform "on the fly" substitutions, so we preserve
  /// all pack expansions. We still need an active expansion stack though,
  /// for the dummy implementation of these methods:
  /// - beginPackExpansion()
  /// - advancePackExpansion()
  /// - createExpandedPackElement()
  /// - endPackExpansion()
  toolchain::SmallVector<Type, 2> ActivePackExpansions;

public:
  using BuiltType = language::Type;
  using BuiltTypeDecl = language::GenericTypeDecl *; // nominal or type alias
  using BuiltProtocolDecl = language::ProtocolDecl *;
  using BuiltGenericSignature = language::GenericSignature;
  using BuiltRequirement = language::Requirement;
  using BuiltInverseRequirement = language::InverseRequirement;
  using BuiltSubstitutionMap = language::SubstitutionMap;

  static constexpr bool needsToPrecomputeParentGenericContextShapes = false;

  explicit ASTBuilder(ASTContext &ctx, GenericSignature genericSig) : Ctx(ctx) {
    ManglingFlavor = ctx.LangOpts.hasFeature(Feature::Embedded)
                 ? Mangle::ManglingFlavor::Embedded
                 : Mangle::ManglingFlavor::Default;

    for (auto *paramTy : genericSig.getGenericParams()) {
      if (paramTy->isParameterPack())
        ParameterPacks.emplace_back(paramTy->getDepth(), paramTy->getIndex());

      if (paramTy->isValue()) {
        auto pair = std::make_pair(paramTy->getDepth(), paramTy->getIndex());
        auto tuple = std::make_tuple(pair, paramTy->getValueType());
        ValueParameters.emplace_back(tuple);
      }
    }
  }

  ASTContext &getASTContext() { return Ctx; }
  Mangle::ManglingFlavor getManglingFlavor() { return ManglingFlavor; }
  DeclContext *getNotionalDC();

  Demangle::NodeFactory &getNodeFactory() { return Factory; }

  Type decodeMangledType(NodePointer node, bool forRequirement = true);
  Type createBuiltinType(StringRef builtinName, StringRef mangledName);

  TypeDecl *createTypeDecl(NodePointer node);

  GenericTypeDecl *createTypeDecl(StringRef mangledName, bool &typeAlias);
  
  GenericTypeDecl *createTypeDecl(NodePointer node,
                                  bool &typeAlias);

  ProtocolDecl *createProtocolDecl(NodePointer node);

  Type createNominalType(GenericTypeDecl *decl);

  Type createNominalType(GenericTypeDecl *decl, Type parent);

  Type createTypeAliasType(GenericTypeDecl *decl, Type parent);

  Type createBoundGenericType(GenericTypeDecl *decl, ArrayRef<Type> args);
  
  Type resolveOpaqueType(NodePointer opaqueDescriptor,
                         ArrayRef<ArrayRef<Type>> args,
                         unsigned ordinal);

  Type createBoundGenericType(GenericTypeDecl *decl, ArrayRef<Type> args,
                              Type parent);

  Type createTupleType(ArrayRef<Type> eltTypes, ArrayRef<StringRef> labels);

  Type createPackType(ArrayRef<Type> eltTypes);

  Type createSILPackType(ArrayRef<Type> eltTypes, bool isElementAddress);

  size_t beginPackExpansion(Type countType);

  void advancePackExpansion(size_t index);

  Type createExpandedPackElement(Type patternType);

  void endPackExpansion();

  Type createFunctionType(
      ArrayRef<Demangle::FunctionParam<Type>> params,
      Type output, FunctionTypeFlags flags, ExtendedFunctionTypeFlags extFlags,
      FunctionMetadataDifferentiabilityKind diffKind, Type globalActor,
      Type thrownError);

  Type createImplFunctionType(
      Demangle::ImplParameterConvention calleeConvention,
      Demangle::ImplCoroutineKind coroutineKind,
      ArrayRef<Demangle::ImplFunctionParam<Type>> params,
      ArrayRef<Demangle::ImplFunctionYield<Type>> yields,
      ArrayRef<Demangle::ImplFunctionResult<Type>> results,
      std::optional<Demangle::ImplFunctionResult<Type>> errorResult,
      ImplFunctionTypeFlags flags);

  Type createProtocolCompositionType(ArrayRef<ProtocolDecl *> protocols,
                                     Type superclass,
                                     bool isClassBound,
                                     bool forRequirement = true);

  Type createProtocolTypeFromDecl(ProtocolDecl *protocol);

  Type createConstrainedExistentialType(
      Type base,
      ArrayRef<BuiltRequirement> constraints,
      ArrayRef<BuiltInverseRequirement> inverseRequirements);

  Type createSymbolicExtendedExistentialType(NodePointer shapeNode,
                                             ArrayRef<Type> genArgs);

  Type createExistentialMetatypeType(
      Type instance,
      std::optional<Demangle::ImplMetatypeRepresentation> repr = std::nullopt);

  Type createMetatypeType(
      Type instance,
      std::optional<Demangle::ImplMetatypeRepresentation> repr = std::nullopt);

  void pushGenericParams(ArrayRef<std::pair<unsigned, unsigned>> parameterPacks);
  void popGenericParams();

  Type createGenericTypeParameterType(unsigned depth, unsigned index);

  Type createDependentMemberType(StringRef member, Type base);

  Type createDependentMemberType(StringRef member, Type base,
                                 ProtocolDecl *protocol);

#define REF_STORAGE(Name, ...) \
  Type create##Name##StorageType(Type base);
#include "language/AST/ReferenceStorage.def"

  Type createSILBoxType(Type base);
  using BuiltSILBoxField = toolchain::PointerIntPair<Type, 1>;
  using BuiltSubstitution = std::pair<Type, Type>;
  using BuiltLayoutConstraint = language::LayoutConstraint;
  Type createSILBoxTypeWithLayout(
      ArrayRef<BuiltSILBoxField> Fields,
      ArrayRef<BuiltSubstitution> Substitutions,
      ArrayRef<BuiltRequirement> Requirements,
      ArrayRef<BuiltInverseRequirement> inverseRequirements);

  bool isExistential(Type type) {
    return type->isExistentialType();
  }


  Type createObjCClassType(StringRef name);

  Type createBoundGenericObjCClassType(StringRef name, ArrayRef<Type> args);

  ProtocolDecl *createObjCProtocolDecl(StringRef name);

  Type createDynamicSelfType(Type selfType);

  Type createForeignClassType(StringRef mangledName);

  Type getUnnamedForeignClassType();

  Type getOpaqueType();

  Type createOptionalType(Type base);

  Type createArrayType(Type base);

  Type createInlineArrayType(Type count, Type element);

  Type createDictionaryType(Type key, Type value);

  Type createIntegerType(intptr_t value);

  Type createNegativeIntegerType(intptr_t value);

  Type createBuiltinFixedArrayType(Type size, Type element);

  BuiltGenericSignature
  createGenericSignature(ArrayRef<BuiltType> params,
                         ArrayRef<BuiltRequirement> requirements);

  BuiltSubstitutionMap createSubstitutionMap(BuiltGenericSignature sig,
                                             ArrayRef<BuiltType> replacements);

  Type subst(Type subject, const BuiltSubstitutionMap &Subs) const;

  LayoutConstraint getLayoutConstraint(LayoutConstraintKind kind);
  LayoutConstraint getLayoutConstraintWithSizeAlign(LayoutConstraintKind kind,
                                                    unsigned size,
                                                    unsigned alignment);

  InverseRequirement createInverseRequirement(
      Type subject, InvertibleProtocolKind kind);

private:
  bool validateParentType(TypeDecl *decl, Type parent);
  CanGenericSignature demangleGenericSignature(
      NominalTypeDecl *nominalDecl,
      NodePointer node);
  DeclContext *findDeclContext(NodePointer node);

  /// Find all the ModuleDecls that correspond to a module node's identifier.
  /// The module name encoded in the node is either the module's real or ABI
  /// name. Multiple modules can share the same name. This function returns
  /// all modules that contain that name.
  toolchain::ArrayRef<ModuleDecl *> findPotentialModules(NodePointer node);

  Demangle::NodePointer findModuleNode(NodePointer node);

  enum class ForeignModuleKind {
    Imported,
    SynthesizedByImporter
  };

  std::optional<ForeignModuleKind> getForeignModuleKind(NodePointer node);

  GenericTypeDecl *findTypeDecl(DeclContext *dc,
                                Identifier name,
                                Identifier privateDiscriminator,
                                Demangle::Node::Kind kind);
  GenericTypeDecl *findForeignTypeDecl(StringRef name,
                                       StringRef relatedEntityKind,
                                       ForeignModuleKind lookupKind,
                                       Demangle::Node::Kind kind);

  static GenericTypeDecl *getAcceptableTypeDeclCandidate(ValueDecl *decl,
                                              Demangle::Node::Kind kind);

  /// Returns an identifier with the given name, automatically removing any
  /// surrounding backticks that are present for raw identifiers.
  Identifier getIdentifier(StringRef name);
};

LANGUAGE_END_INLINE_NAMESPACE
}  // namespace Demangle

}  // namespace language

#endif  // LANGUAGE_AST_ASTDEMANGLER_H
