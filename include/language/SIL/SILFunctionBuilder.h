//===--- SILFunctionBuilder.h ---------------------------------------------===//
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

#ifndef LANGUAGE_SIL_SILFUNCTIONBUILDER_H
#define LANGUAGE_SIL_SILFUNCTIONBUILDER_H

#include "language/AST/AvailabilityRange.h"
#include "language/SIL/SILModule.h"

namespace language {

class SILParserFunctionBuilder;
class SILSerializationFunctionBuilder;
class SILOptFunctionBuilder;
namespace Lowering {
class SILGenFunctionBuilder;
} // namespace Lowering

/// A class for creating SILFunctions in a specific SILModule.
///
/// The intention is that this class is not used directly, but rather that each
/// part of the compiler that needs to create functions create a composition
/// type with SILFunctionBuilder as a field. This enables subsystems that use
/// SIL to:
///
/// 1. Enforce invariants in the type system. An example of this is in the
///    SILOptimizer where we want to ensure that the pass manager properly
///    notifies analyses whenever functions are created/destroyed.
///
/// 2. Have a convenient place to place utility functionality for creating
///    functions. Today the compiler has many small utility functions for
///    creating the underlying SILFunction that are generally quite verbose and
///    have shared code. These SILFunctionBuilder composition types will enable
///    code-reuse in between these different SILFunction creation sites.
class SILFunctionBuilder {
  SILModule &mod;
  AvailabilityRange availCtx;

  friend class SILParserFunctionBuilder;
  friend class SILSerializationFunctionBuilder;
  friend class SILOptFunctionBuilder;
  friend class Lowering::SILGenFunctionBuilder;

  SILFunctionBuilder(SILModule &mod)
      : SILFunctionBuilder(
            mod, AvailabilityRange::forDeploymentTarget(mod.getASTContext())) {}

  SILFunctionBuilder(SILModule &mod, AvailabilityRange availCtx)
      : mod(mod), availCtx(availCtx) {}

  /// Return the declaration of a utility function that can, but needn't, be
  /// shared between different parts of a program.
  SILFunction *getOrCreateSharedFunction(
      SILLocation loc, StringRef name, CanSILFunctionType type,
      IsBare_t isBareSILFunction, IsTransparent_t isTransparent,
      SerializedKind_t serializedKind, ProfileCounter entryCount,
      IsThunk_t isThunk, IsDynamicallyReplaceable_t isDynamic,
      IsDistributed_t isDistributed, IsRuntimeAccessible_t isRuntimeAccessible);

  /// Return the declaration of a function, or create it if it doesn't exist.
  SILFunction *getOrCreateFunction(
      SILLocation loc, StringRef name, SILLinkage linkage,
      CanSILFunctionType type, IsBare_t isBareSILFunction,
      IsTransparent_t isTransparent, SerializedKind_t serializedKind,
      IsDynamicallyReplaceable_t isDynamic, IsDistributed_t isDistributed,
      IsRuntimeAccessible_t isRuntimeAccessible,
      ProfileCounter entryCount = ProfileCounter(),
      IsThunk_t isThunk = IsNotThunk,
      SubclassScope subclassScope = SubclassScope::NotApplicable);

  /// Return the declaration of a function, or create it if it doesn't exist.
  /// Creating a function that references another function (a dynamic
  /// replacement) requires getting/creating a SIL function for this reference.
  /// In this case the function will call the \p getOrCreateDeclaration
  /// callback.
  SILFunction *getOrCreateFunction(
      SILLocation loc, SILDeclRef constant, ForDefinition_t forDefinition,
      toolchain::function_ref<SILFunction *(SILLocation loc, SILDeclRef constant)>
          getOrCreateDeclaration = [](SILLocation loc, SILDeclRef constant)
          -> SILFunction * { return nullptr; },
      ProfileCounter entryCount = ProfileCounter());

  /// Create a function declaration.
  ///
  /// This signature is a direct copy of the signature of SILFunction::create()
  /// in order to simplify refactoring all SILFunction creation use-sites to use
  /// SILFunctionBuilder. Eventually the uses should probably be refactored.
  SILFunction *createFunction(
      SILLinkage linkage, StringRef name, CanSILFunctionType loweredType,
      GenericEnvironment *genericEnv, std::optional<SILLocation> loc,
      IsBare_t isBareSILFunction, IsTransparent_t isTrans,
      SerializedKind_t serializedKind, IsDynamicallyReplaceable_t isDynamic,
      IsDistributed_t isDistributed, IsRuntimeAccessible_t isRuntimeAccessible,
      ProfileCounter entryCount = ProfileCounter(),
      IsThunk_t isThunk = IsNotThunk,
      SubclassScope subclassScope = SubclassScope::NotApplicable,
      Inline_t inlineStrategy = InlineDefault,
      EffectsKind EK = EffectsKind::Unspecified,
      SILFunction *InsertBefore = nullptr,
      const SILDebugScope *DebugScope = nullptr);

  void addFunctionAttributes(
      SILFunction *F, DeclAttributes &Attrs, SILModule &M,
      toolchain::function_ref<SILFunction *(SILLocation loc, SILDeclRef constant)>
          getOrCreateDeclaration,
      SILDeclRef constant = SILDeclRef());

  /// We do not expose this to everyone, instead we allow for our users to opt
  /// into this if they need to. Please do not do this in general! We only want
  /// to use this when deserializing a function body.
  static void setHasOwnership(SILFunction *F, bool newValue) {
    F->setHasOwnership(newValue);
  }
};

} // namespace language

#endif
