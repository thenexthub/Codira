//===--- TerminatorUtils.h ------------------------------------------------===//
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
///
/// \file
///
/// ADTs for working with various forms of terminators.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_TERMINATORUTILS_H
#define LANGUAGE_SIL_TERMINATORUTILS_H

#include "language/Basic/Toolchain.h"
#include "language/SIL/SILInstruction.h"

#include "toolchain/ADT/PointerUnion.h"

namespace language {

/// An ADT for writing generic code against SwitchEnumAddrInst and
/// SwitchEnumInst.
///
/// We use this instead of SwitchEnumInstBase for this purpose in order to avoid
/// the need for templating SwitchEnumInstBase from causing this ADT type of
/// usage to require templates.
class SwitchEnumTermInst {
  PointerUnion<SwitchEnumAddrInst *, SwitchEnumInst *> value;

public:
  SwitchEnumTermInst(SwitchEnumAddrInst *seai) : value(seai) {}
  SwitchEnumTermInst(SwitchEnumInst *seai) : value(seai) {}
  SwitchEnumTermInst(SILInstruction *i) : value(nullptr) {
    if (auto *seai = dyn_cast<SwitchEnumAddrInst>(i)) {
      value = seai;
      return;
    }

    if (auto *sei = dyn_cast<SwitchEnumInst>(i)) {
      value = sei;
      return;
    }
  }

  SwitchEnumTermInst(const SILInstruction *i)
      : SwitchEnumTermInst(const_cast<SILInstruction *>(i)) {}

  operator TermInst *() const {
    if (auto *seai = value.dyn_cast<SwitchEnumAddrInst *>())
      return seai;
    return value.get<SwitchEnumInst *>();
  }

  TermInst *operator*() const {
    if (auto *seai = value.dyn_cast<SwitchEnumAddrInst *>())
      return seai;
    return value.get<SwitchEnumInst *>();
  }

  TermInst *operator->() const {
    if (auto *seai = value.dyn_cast<SwitchEnumAddrInst *>())
      return seai;
    return value.get<SwitchEnumInst *>();
  }

  operator bool() const { return bool(value); }

  SILValue getOperand() {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getOperand();
    return value.get<SwitchEnumAddrInst *>()->getOperand();
  }

  unsigned getNumCases() {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getNumCases();
    return value.get<SwitchEnumAddrInst *>()->getNumCases();
  }

  std::pair<EnumElementDecl *, SILBasicBlock *> getCase(unsigned i) const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getCase(i);
    return value.get<SwitchEnumAddrInst *>()->getCase(i);
  }

  SILBasicBlock *getCaseDestination(EnumElementDecl *decl) const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getCaseDestination(decl);
    return value.get<SwitchEnumAddrInst *>()->getCaseDestination(decl);
  }

  ProfileCounter getCaseCount(unsigned i) const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getCaseCount(i);
    return value.get<SwitchEnumAddrInst *>()->getCaseCount(i);
  }

  ProfileCounter getDefaultCount() const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getDefaultCount();
    return value.get<SwitchEnumAddrInst *>()->getDefaultCount();
  }

  bool hasDefault() const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->hasDefault();
    return value.get<SwitchEnumAddrInst *>()->hasDefault();
  }

  SILBasicBlock *getDefaultBB() const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getDefaultBB();
    return value.get<SwitchEnumAddrInst *>()->getDefaultBB();
  }

  NullablePtr<SILBasicBlock> getDefaultBBOrNull() const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getDefaultBBOrNull();
    return value.get<SwitchEnumAddrInst *>()->getDefaultBBOrNull();
  }

  /// If the default refers to exactly one case decl, return it.
  NullablePtr<EnumElementDecl> getUniqueCaseForDefault() const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getUniqueCaseForDefault();
    return value.get<SwitchEnumAddrInst *>()->getUniqueCaseForDefault();
  }

  /// If the given block only has one enum element decl matched to it,
  /// return it.
  NullablePtr<EnumElementDecl>
  getUniqueCaseForDestination(SILBasicBlock *BB) const {
    if (auto *sei = value.dyn_cast<SwitchEnumInst *>())
      return sei->getUniqueCaseForDestination(BB);
    return value.get<SwitchEnumAddrInst *>()->getUniqueCaseForDestination(BB);
  }
};

} // namespace language

#endif
