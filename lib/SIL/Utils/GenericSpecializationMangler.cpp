//===--- GenericSpecializationMangler.cpp - mangling of specializations ---===//
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

#include "language/SIL/GenericSpecializationMangler.h"
#include "language/AST/ASTContext.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/SubstitutionMap.h"
#include "language/Basic/Assertions.h"
#include "language/Demangling/ManglingMacros.h"

using namespace language;
using namespace Mangle;

void SpecializationMangler::beginMangling() {
  ASTMangler::beginManglingWithoutPrefix();

  if (Serialized)
    ArgOpBuffer << 'q';

  if (RemovedEffects.contains(EffectKind::Async))
    ArgOpBuffer << 'a';

  ArgOpBuffer << char(uint8_t(Pass) + '0');
}

namespace {

/// Utility class for demangling specialization attributes.
class AttributeDemangler : public Demangle::Demangler {
public:
  void demangleAndAddAsChildren(StringRef MangledSpecialization,
                                NodePointer Parent) {
    DemangleInitRAII state(*this, MangledSpecialization, nullptr);
    if (!parseAndPushNodes()) {
      ABORT([&](auto &out) {
        out << "Can't demangle: " << MangledSpecialization;
      });
    }
    for (Node *Nd : NodeStack) {
      addChild(Parent, Nd);
    }
  }
};

} // namespace

std::string SpecializationMangler::finalize() {
  StringRef MangledSpecialization(Storage.data(), Storage.size());
  AttributeDemangler D;
  NodePointer TopLevel = D.createNode(Node::Kind::Global);
  D.demangleAndAddAsChildren(MangledSpecialization, TopLevel);

  StringRef FuncName = Function ? Function->getName() : StringRef(FunctionName);
  NodePointer FuncTopLevel = nullptr;
  if (FuncName.starts_with(MANGLING_PREFIX_STR)) {
    FuncTopLevel = D.demangleSymbol(FuncName);
    assert(FuncTopLevel);
  }
  else if (FuncName.starts_with(MANGLING_PREFIX_EMBEDDED_STR)) {
    FuncTopLevel = D.demangleSymbol(FuncName);
    assert(FuncTopLevel);
  }
  if (!FuncTopLevel) {
    FuncTopLevel = D.createNode(Node::Kind::Global);
    FuncTopLevel->addChild(D.createNode(Node::Kind::Identifier, FuncName), D);
  }
  for (NodePointer FuncChild : *FuncTopLevel) {
    TopLevel->addChild(FuncChild, D);
  }
  auto mangling = Demangle::mangleNode(TopLevel, Flavor);
  assert(mangling.isSuccess());
  std::string mangledName = mangling.result();
  verify(mangledName, Flavor);
  return mangledName;
}

//===----------------------------------------------------------------------===//
//                           Generic Specialization
//===----------------------------------------------------------------------===//

void GenericSpecializationMangler::
appendSubstitutions(GenericSignature sig, SubstitutionMap subs) {
  bool First = true;
  sig->forEachParam([&](GenericTypeParamType *ParamType, bool Canonical) {
    if (Canonical) {
      auto ty = Type(ParamType);
      auto substTy = ty.subst(subs);
      auto canTy = substTy->getCanonicalType();
      appendType(canTy, nullptr);
      appendListSeparator(First);
    }
  });
  assert(!First && "no generic substitutions");
}

std::string GenericSpecializationMangler::
manglePrespecialized(GenericSignature sig, SubstitutionMap subs) {
  beginMangling();
  appendSubstitutions(sig, subs);
  appendSpecializationOperator("Ts");
  return finalize();
}
                                  
std::string GenericSpecializationMangler::
mangleNotReabstracted(SubstitutionMap subs,
                      const SmallBitVector &paramsRemoved) {
  beginMangling();
  appendSubstitutions(getGenericSignature(), subs);
  appendOperator("T");
  appendRemovedParams(paramsRemoved);
  appendSpecializationOperator("G");
  return finalize();
}
                                  
std::string GenericSpecializationMangler::
mangleReabstracted(SubstitutionMap subs, bool alternativeMangling,
                   const SmallBitVector &paramsRemoved) {
  beginMangling();
  appendSubstitutions(getGenericSignature(), subs);
  appendOperator("T");
  appendRemovedParams(paramsRemoved);

  // See ReabstractionInfo::hasConvertedResilientParams for why and when to use
  // the alternative mangling.
  appendSpecializationOperator(alternativeMangling ? "B" : "g");
  return finalize();
}

void GenericSpecializationMangler::appendRemovedParams(const SmallBitVector &paramsRemoved) {
  for (int paramIdx : paramsRemoved.set_bits()) {
    appendOperator("t");
    if (paramIdx != 0)
      Buffer << (paramIdx - 1);
  }
}

std::string GenericSpecializationMangler::
mangleForDebugInfo(GenericSignature sig, SubstitutionMap subs, bool forInlining) {
  beginMangling();
  appendSubstitutions(sig, subs);
  appendSpecializationOperator(forInlining ? "Ti" : "TG");
  return finalize();
}


static SubstitutionMap
getSubstitutionMapForPrespecialization(GenericSignature genericSig,
                                       GenericSignature specSig) {
  auto CalleeGenericSig = genericSig;
  auto SpecializedGenericEnv = specSig.getGenericEnvironment();

  auto CalleeInterfaceToSpecializedInterfaceMap = SubstitutionMap::get(
      CalleeGenericSig,
      [&](SubstitutableType *type) -> Type {
        return type;
      },
      LookUpConformanceInModule());

  auto subs = SubstitutionMap::get(
      CalleeGenericSig,
      [&](SubstitutableType *type) -> Type {
        auto SpecializedInterfaceTy =
            Type(type).subst(CalleeInterfaceToSpecializedInterfaceMap);
        return SpecializedGenericEnv->mapTypeIntoContext(
            SpecializedInterfaceTy);
      },
      LookUpConformanceInModule());
  return subs;
}

std::string GenericSpecializationMangler::manglePrespecialization(ASTContext &Ctx,
    std::string unspecializedName, GenericSignature genericSig,
    GenericSignature specializedSig) {
  auto subs =
      getSubstitutionMapForPrespecialization(genericSig, specializedSig);
  GenericSpecializationMangler mangler(Ctx, unspecializedName);
  return mangler.manglePrespecialized(genericSig, subs);
}
