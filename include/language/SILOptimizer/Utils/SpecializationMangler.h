//===--- SpecializationMangler.h - mangling of specializations --*- C++ -*-===//
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

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_SPECIALIZATIONMANGLER_H
#define LANGUAGE_SILOPTIMIZER_UTILS_SPECIALIZATIONMANGLER_H

#include "language/SIL/GenericSpecializationMangler.h"

#include "language/Demangling/Demangler.h"
#include "language/Demangling/NamespaceMacros.h"
#include "language/Basic/NullablePtr.h"
#include "language/AST/ASTMangler.h"
#include "language/SIL/SILLinkage.h"
#include "language/SIL/SILFunction.h"

namespace language {
namespace Mangle {
LANGUAGE_BEGIN_INLINE_NAMESPACE

class PartialSpecializationMangler : public SpecializationMangler {

  CanSILFunctionType SpecializedFnTy;
  bool isReAbstracted;
  
public:
  PartialSpecializationMangler(ASTContext &Ctx, SILFunction *F,
                               CanSILFunctionType SpecializedFnTy,
                               language::SerializedKind_t Serialized, bool isReAbstracted)
      : SpecializationMangler(Ctx, SpecializationPass::GenericSpecializer,
                              Serialized, F),
        SpecializedFnTy(SpecializedFnTy), isReAbstracted(isReAbstracted) {}

  std::string mangle();
};

// The mangler for functions where arguments or effects are specialized.
class FunctionSignatureSpecializationMangler : public SpecializationMangler {

  using ReturnValueModifierIntBase = uint16_t;
  enum class ReturnValueModifier : ReturnValueModifierIntBase {
    // Option Space 4 bits (i.e. 16 options).
    Unmodified=0,
    First_Option=0, Last_Option=31,

    // Option Set Space. 12 bits (i.e. 12 option).
    Dead=32,
    OwnedToUnowned=64,
    First_OptionSetEntry=32, LastOptionSetEntry=32768,
  };

  // We use this private typealias to make it easy to expand ArgumentModifier's
  // size if we need to.
  using ArgumentModifierIntBase = uint16_t;
  enum class ArgumentModifier : ArgumentModifierIntBase {
    // Option Space 4 bits (i.e. 16 options).
    Unmodified = 0,
    ConstantProp = 1,
    ClosureProp = 2,
    BoxToValue = 3,
    BoxToStack = 4,
    InOutToOut = 5,

    First_Option = 0,
    Last_Option = 31,

    // Option Set Space. 12 bits (i.e. 12 option).
    Dead = 32,
    OwnedToGuaranteed = 64,
    SROA = 128,
    GuaranteedToOwned = 256,
    ExistentialToGeneric = 512,
    First_OptionSetEntry = 32,
    LastOptionSetEntry = 32768,
  };

  using ArgInfo = std::pair<ArgumentModifierIntBase,
                            NullablePtr<SILInstruction>>;
  // Information for each SIL argument in the original function before
  // specialization. This includes SIL indirect result argument required for
  // the original function type at the current stage of compilation.
  toolchain::SmallVector<ArgInfo, 8> OrigArgs;

  ReturnValueModifierIntBase ReturnValue;

public:
  FunctionSignatureSpecializationMangler(ASTContext &Ctx, SpecializationPass Pass,
                                         language::SerializedKind_t Serialized,
                                         SILFunction *F);
  // For arguments / return values
  void setArgumentConstantProp(unsigned OrigArgIdx, SILInstruction *constInst);
  void appendStringAsIdentifier(StringRef str);

  void setArgumentClosureProp(unsigned OrigArgIdx, PartialApplyInst *PAI);
  void setArgumentClosureProp(unsigned OrigArgIdx,
                              ThinToThickFunctionInst *TTTFI);
  void setArgumentDead(unsigned OrigArgIdx);
  void setArgumentOwnedToGuaranteed(unsigned OrigArgIdx);
  void setArgumentGuaranteedToOwned(unsigned OrigArgIdx);
  void setArgumentExistentialToGeneric(unsigned OrigArgIdx);
  void setArgumentSROA(unsigned OrigArgIdx);
  void setArgumentBoxToValue(unsigned OrigArgIdx);
  void setArgumentBoxToStack(unsigned OrigArgIdx);
  void setArgumentInOutToOut(unsigned OrigArgIdx);
  void setReturnValueOwnedToUnowned();

  // For effects
  void setRemovedEffect(EffectKind effect);

  std::string mangle();
  
private:
  void mangleConstantProp(SILInstruction *constInst);
  void mangleClosureProp(SILInstruction *Inst);
  void mangleArgument(ArgumentModifierIntBase ArgMod,
                      NullablePtr<SILInstruction> Inst);
  void mangleReturnValue(ReturnValueModifierIntBase RetMod);
};

LANGUAGE_END_INLINE_NAMESPACE
} // end namespace Mangle
} // end namespace language

#endif
