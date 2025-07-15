//===--- SpecializationMangler.cpp - mangling of specializations ----------===//
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

#include "language/SILOptimizer/Utils/SpecializationMangler.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/SubstitutionMap.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/MD5Stream.h"
#include "language/Demangling/ManglingMacros.h"
#include "language/SIL/SILGlobalVariable.h"
#include "toolchain/ADT/StringExtras.h"

using namespace language;
using namespace Mangle;

//===----------------------------------------------------------------------===//
//                         Partial Generic Specialization
//===----------------------------------------------------------------------===//

std::string PartialSpecializationMangler::mangle() {
  beginMangling();
  appendType(SpecializedFnTy, nullptr);
  appendSpecializationOperator(isReAbstracted ? "Tp" : "TP");
  return finalize();
}

//===----------------------------------------------------------------------===//
//                      Function Signature Optimizations
//===----------------------------------------------------------------------===//

FunctionSignatureSpecializationMangler::FunctionSignatureSpecializationMangler(ASTContext &Ctx,
    Demangle::SpecializationPass P, SerializedKind_t Serialized, SILFunction *F)
    : SpecializationMangler(Ctx, P, Serialized, F) {
  for (unsigned i = 0, e = F->getConventions().getNumSILArguments(); i != e;
       ++i) {
    (void)i;
    OrigArgs.push_back(
        {ArgumentModifierIntBase(ArgumentModifier::Unmodified), nullptr});
  }
  ReturnValue = ReturnValueModifierIntBase(ReturnValueModifier::Unmodified);
}

void FunctionSignatureSpecializationMangler::setArgumentDead(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first |= ArgumentModifierIntBase(ArgumentModifier::Dead);
}

void FunctionSignatureSpecializationMangler::setArgumentClosureProp(
    unsigned OrigArgIdx, PartialApplyInst *PAI) {
  auto &Info = OrigArgs[OrigArgIdx];
  Info.first = ArgumentModifierIntBase(ArgumentModifier::ClosureProp);
  Info.second = PAI;
}

void FunctionSignatureSpecializationMangler::setArgumentClosureProp(
    unsigned OrigArgIdx, ThinToThickFunctionInst *TTTFI) {
  auto &Info = OrigArgs[OrigArgIdx];
  Info.first = ArgumentModifierIntBase(ArgumentModifier::ClosureProp);
  Info.second = TTTFI;
}

void FunctionSignatureSpecializationMangler::setArgumentConstantProp(
    unsigned OrigArgIdx, SILInstruction *constInst) {
  auto &Info = OrigArgs[OrigArgIdx];
  Info.first = ArgumentModifierIntBase(ArgumentModifier::ConstantProp);
  Info.second = constInst;
}

void FunctionSignatureSpecializationMangler::setArgumentOwnedToGuaranteed(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first |=
      ArgumentModifierIntBase(ArgumentModifier::OwnedToGuaranteed);
}

void FunctionSignatureSpecializationMangler::setArgumentSROA(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first |= ArgumentModifierIntBase(ArgumentModifier::SROA);
}

void FunctionSignatureSpecializationMangler::setArgumentGuaranteedToOwned(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first |=
      ArgumentModifierIntBase(ArgumentModifier::GuaranteedToOwned);
}

void FunctionSignatureSpecializationMangler::setArgumentExistentialToGeneric(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first |=
      ArgumentModifierIntBase(ArgumentModifier::ExistentialToGeneric);
}

void FunctionSignatureSpecializationMangler::setArgumentBoxToValue(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first =
      ArgumentModifierIntBase(ArgumentModifier::BoxToValue);
}

void FunctionSignatureSpecializationMangler::setArgumentBoxToStack(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first =
      ArgumentModifierIntBase(ArgumentModifier::BoxToStack);
}

void FunctionSignatureSpecializationMangler::setArgumentInOutToOut(
    unsigned OrigArgIdx) {
  OrigArgs[OrigArgIdx].first =
      ArgumentModifierIntBase(ArgumentModifier::InOutToOut);
}

void
FunctionSignatureSpecializationMangler::
setReturnValueOwnedToUnowned() {
  ReturnValue |= ReturnValueModifierIntBase(ReturnValueModifier::OwnedToUnowned);
}

void
FunctionSignatureSpecializationMangler::
setRemovedEffect(EffectKind effect) {
  assert(effect == EffectKind::Async && "unimplemented effect kind!");
  RemovedEffects |= effect;
}

void
FunctionSignatureSpecializationMangler::mangleConstantProp(SILInstruction *constInst) {
  // Append the prefix for constant propagation 'p'.
  ArgOpBuffer << 'p';

  // Then append the unique identifier of our literal.
  switch (constInst->getKind()) {
  default:
    toolchain_unreachable("unknown literal");
  case SILInstructionKind::PreviousDynamicFunctionRefInst:
  case SILInstructionKind::DynamicFunctionRefInst:
  case SILInstructionKind::FunctionRefInst: {
    SILFunction *F =
        cast<FunctionRefBaseInst>(constInst)->getInitiallyReferencedFunction();
    ArgOpBuffer << 'f';
    appendIdentifier(F->getName());
    break;
  }
  case SILInstructionKind::GlobalAddrInst: {
    SILGlobalVariable *G = cast<GlobalAddrInst>(constInst)->getReferencedGlobal();
    ArgOpBuffer << 'g';
    appendIdentifier(G->getName());
    break;
  }
  case SILInstructionKind::IntegerLiteralInst: {
    APInt apint = cast<IntegerLiteralInst>(constInst)->getValue();
    ArgOpBuffer << 'i' << apint;
    break;
  }
  case SILInstructionKind::FloatLiteralInst: {
    APInt apint = cast<FloatLiteralInst>(constInst)->getBits();
    ArgOpBuffer << 'd' << apint;
    break;
  }
  case SILInstructionKind::StringLiteralInst: {
    StringLiteralInst *SLI = cast<StringLiteralInst>(constInst);
    StringRef V = SLI->getValue();
    assert(V.size() <= 32 && "Cannot encode string of length > 32");
    std::string VBuffer;
    if (!V.empty() && (isDigit(V[0]) || V[0] == '_')) {
      VBuffer = "_";
      VBuffer.append(V.data(), V.size());
      V = VBuffer;
    }
    appendIdentifier(V);

    ArgOpBuffer << 's';
    switch (SLI->getEncoding()) {
      case StringLiteralInst::Encoding::Bytes: ArgOpBuffer << 'B'; break;
      case StringLiteralInst::Encoding::UTF8: ArgOpBuffer << 'b'; break;
      case StringLiteralInst::Encoding::UTF8_OSLOG: ArgOpBuffer << 'o'; break;
      case StringLiteralInst::Encoding::ObjCSelector: ArgOpBuffer << 'c'; break;
    }
    break;
  }
  case SILInstructionKind::KeyPathInst: {
    // Mangle a keypath instruction by creating a MD5 hash of the printed
    // instruction. Everything else would be too complicated.
  
    auto *kp = cast<KeyPathInst>(constInst);
    KeyPathPattern *pattern = kp->getPattern();
    
    MD5Stream md5Stream;
    SILPrintContext printCtxt(md5Stream);
    for (auto &component : pattern->getComponents()) {
      component.print(printCtxt);
    }
    toolchain::MD5::MD5Result md5Hash;
    md5Stream.final(md5Hash);
    SmallString<32> resultStr;
    toolchain::MD5::stringifyResult(md5Hash, resultStr);
    appendStringAsIdentifier(resultStr);

    // Also, mangle the involved types.
    appendType(pattern->getRootType(), nullptr);
    appendType(pattern->getValueType(), nullptr);

    ArgOpBuffer << 'k';
    break;
  }
  }
}

void
FunctionSignatureSpecializationMangler::appendStringAsIdentifier(StringRef str) {
  std::string buffer;
  if (!str.empty() && (isDigit(str[0]) || str[0] == '_')) {
    buffer = "_";
    buffer.append(str.data(), str.size());
    str = buffer;
  }
  appendIdentifier(str);
}

void
FunctionSignatureSpecializationMangler::mangleClosureProp(SILInstruction *Inst) {
  ArgOpBuffer << 'c';

  // Add in the partial applies function name if we can find one. Assert
  // otherwise. The reason why this is ok to do is currently we only perform
  // closure specialization if we know the function_ref in question. When this
  // restriction is removed, the assert here will fire.
  if (auto *TTTFI = dyn_cast<ThinToThickFunctionInst>(Inst)) {
    auto *FRI = cast<FunctionRefInst>(TTTFI->getCallee());
    appendIdentifier(FRI->getReferencedFunction()->getName());
    return;
  }
  auto *PAI = cast<PartialApplyInst>(Inst);
  auto *FRI = cast<FunctionRefInst>(PAI->getCallee());
  appendIdentifier(FRI->getReferencedFunction()->getName());

  // Then we mangle the types of the arguments that the partial apply is
  // specializing.
  for (auto &Op : PAI->getArgumentOperands()) {
    SILType Ty = Op.get()->getType();
    appendType(Ty.getASTType(), nullptr);
  }
}

void FunctionSignatureSpecializationMangler::mangleArgument(
    ArgumentModifierIntBase ArgMod, NullablePtr<SILInstruction> Inst) {
  if (ArgMod == ArgumentModifierIntBase(ArgumentModifier::ConstantProp)) {
    mangleConstantProp(Inst.get());
    return;
  }

  if (ArgMod == ArgumentModifierIntBase(ArgumentModifier::ClosureProp)) {
    mangleClosureProp(Inst.get());
    return;
  }

  if (ArgMod == ArgumentModifierIntBase(ArgumentModifier::Unmodified)) {
    ArgOpBuffer << 'n';
    return;
  }

  if (ArgMod == ArgumentModifierIntBase(ArgumentModifier::BoxToValue)) {
    ArgOpBuffer << 'i';
    return;
  }

  if (ArgMod == ArgumentModifierIntBase(ArgumentModifier::BoxToStack)) {
    ArgOpBuffer << 's';
    return;
  }

  if (ArgMod == ArgumentModifierIntBase(ArgumentModifier::InOutToOut)) {
    ArgOpBuffer << 'r';
    return;
  }

  bool hasSomeMod = false;
  if (ArgMod & ArgumentModifierIntBase(ArgumentModifier::ExistentialToGeneric)) {
    ArgOpBuffer << 'e';
    hasSomeMod = true;
  }

  if (ArgMod & ArgumentModifierIntBase(ArgumentModifier::Dead)) {
    ArgOpBuffer << 'd';
    hasSomeMod = true;
  }

  if (ArgMod & ArgumentModifierIntBase(ArgumentModifier::OwnedToGuaranteed)) {
    ArgOpBuffer << (hasSomeMod ? 'G' : 'g');
    hasSomeMod = true;
  }

  if (ArgMod & ArgumentModifierIntBase(ArgumentModifier::GuaranteedToOwned)) {
    ArgOpBuffer << (hasSomeMod ? 'O' : 'o');
    hasSomeMod = true;
  }

  if (ArgMod & ArgumentModifierIntBase(ArgumentModifier::SROA)) {
    ArgOpBuffer << (hasSomeMod ? 'X' : 'x');
    hasSomeMod = true;
  }

  assert(hasSomeMod && "Unknown modifier");
}

void FunctionSignatureSpecializationMangler::
mangleReturnValue(ReturnValueModifierIntBase RetMod) {
  if (RetMod == ReturnValueModifierIntBase(ReturnValueModifier::Unmodified)) {
    ArgOpBuffer << 'n';
    return;
  }

  bool hasSomeMode = false;
  if (RetMod & ReturnValueModifierIntBase(ReturnValueModifier::Dead)) {
    ArgOpBuffer << 'd';
    hasSomeMode = true;
  }

  if (RetMod & ReturnValueModifierIntBase(ReturnValueModifier::OwnedToUnowned)) {
    ArgOpBuffer << (hasSomeMode ? 'G' : 'g');
  }
}

std::string FunctionSignatureSpecializationMangler::mangle() {
  ArgOpStorage.clear();
  beginMangling();

  for (unsigned i : indices(OrigArgs)) {
    ArgumentModifierIntBase ArgMod;
    NullablePtr<SILInstruction> Inst;
    std::tie(ArgMod, Inst) = OrigArgs[i];
    mangleArgument(ArgMod, Inst);
  }
  ArgOpBuffer << '_';
  mangleReturnValue(ReturnValue);

  appendSpecializationOperator("Tf");
  return finalize();
}
