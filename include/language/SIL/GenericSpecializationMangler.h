//===- GenericSpecializationMangler.h - generic specializations -*- C++ -*-===//
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

#ifndef LANGUAGE_SIL_UTILS_GENERICSPECIALIZATIONMANGLER_H
#define LANGUAGE_SIL_UTILS_GENERICSPECIALIZATIONMANGLER_H

#include "language/AST/ASTContext.h"
#include "language/AST/ASTMangler.h"
#include "language/AST/Effects.h"
#include "language/Basic/NullablePtr.h"
#include "language/Demangling/Demangler.h"
#include "language/SIL/SILFunction.h"

namespace language {
namespace Mangle {

enum class SpecializationKind : uint8_t {
  Generic,
  NotReAbstractedGeneric,
  FunctionSignature,
};

/// Inject SpecializationPass into the Mangle namespace.
using SpecializationPass = Demangle::SpecializationPass;

/// The base class for specialization mangles.
class SpecializationMangler : public Mangle::ASTMangler {
protected:
  /// The specialization pass.
  SpecializationPass Pass;

  language::SerializedKind_t Serialized;

  /// The original function which is specialized.
  SILFunction *Function;
  std::string FunctionName;

  toolchain::SmallVector<char, 32> ArgOpStorage;
  toolchain::raw_svector_ostream ArgOpBuffer;

  // Effects that are removed from the original function in this specialization.
  PossibleEffects RemovedEffects;

protected:
  SpecializationMangler(ASTContext &Ctx, SpecializationPass P, language::SerializedKind_t Serialized,
                        SILFunction *F)
      : ASTMangler(Ctx), Pass(P), Serialized(Serialized), Function(F),
        ArgOpBuffer(ArgOpStorage) {}

  SpecializationMangler(ASTContext &Ctx, SpecializationPass P, language::SerializedKind_t Serialized,
                        std::string functionName)
      : ASTMangler(Ctx), Pass(P), Serialized(Serialized), Function(nullptr),
        FunctionName(functionName), ArgOpBuffer(ArgOpStorage) {}

  void beginMangling();

  /// Finish the mangling of the symbol and return the mangled name.
  std::string finalize();

  void appendSpecializationOperator(StringRef Op) {
    appendOperator(Op, StringRef(ArgOpStorage.data(), ArgOpStorage.size()));
  }
};

// The mangler for specialized generic functions.
class GenericSpecializationMangler : public SpecializationMangler {

  GenericSpecializationMangler(ASTContext &Ctx, std::string origFuncName)
      : SpecializationMangler(Ctx, SpecializationPass::GenericSpecializer,
                              IsNotSerialized, origFuncName) {}

  GenericSignature getGenericSignature() {
    assert(Function && "Need a SIL function to get a generic signature");
    return Function->getLoweredFunctionType()->getInvocationGenericSignature();
  }

  void appendSubstitutions(GenericSignature sig, SubstitutionMap subs);

  std::string manglePrespecialized(GenericSignature sig,
                                      SubstitutionMap subs);

  void appendRemovedParams(const SmallBitVector &paramsRemoved);

public:
  GenericSpecializationMangler(ASTContext &Ctx, SILFunction *F, language::SerializedKind_t Serialized)
      : SpecializationMangler(Ctx, SpecializationPass::GenericSpecializer,
                              Serialized, F) {}

  std::string mangleNotReabstracted(SubstitutionMap subs,
                                    const SmallBitVector &paramsRemoved = SmallBitVector());

  /// Mangle a generic specialization with re-abstracted parameters.
  ///
  /// Re-abstracted means that indirect (generic) parameters/returns are
  /// converted to direct parameters/returns.
  ///
  /// This is the default for generic specializations.
  ///
  /// \param alternativeMangling   true for specialized functions with a
  ///                              different resilience expansion.
  /// \param metatyeParamsRemoved  true if non-generic metatype parameters are
  ///                              removed in the specialized function.
  std::string mangleReabstracted(SubstitutionMap subs, bool alternativeMangling,
                                 const SmallBitVector &paramsRemoved = SmallBitVector());

  std::string mangleForDebugInfo(GenericSignature sig, SubstitutionMap subs,
                                 bool forInlining);

  std::string manglePrespecialized(SubstitutionMap subs) {
    return manglePrespecialized(getGenericSignature(), subs);
  }
                                    
  static std::string manglePrespecialization(ASTContext &Ctx, std::string unspecializedName,
                                             GenericSignature genericSig,
                                             GenericSignature specializedSig);
};

} // end namespace Mangle
} // end namespace language

#endif
