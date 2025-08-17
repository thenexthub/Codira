//===-- SimplifyConstraints.cpp ---------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/FlowSensitive/SimplifyConstraints.h"
#include "toolchain/ADT/EquivalenceClasses.h"

namespace language::Core {
namespace dataflow {

// Substitutes all occurrences of a given atom in `F` by a given formula and
// returns the resulting formula.
static const Formula &
substitute(const Formula &F,
           const toolchain::DenseMap<Atom, const Formula *> &Substitutions,
           Arena &arena) {
  switch (F.kind()) {
  case Formula::AtomRef:
    if (auto iter = Substitutions.find(F.getAtom());
        iter != Substitutions.end())
      return *iter->second;
    return F;
  case Formula::Literal:
    return F;
  case Formula::Not:
    return arena.makeNot(substitute(*F.operands()[0], Substitutions, arena));
  case Formula::And:
    return arena.makeAnd(substitute(*F.operands()[0], Substitutions, arena),
                         substitute(*F.operands()[1], Substitutions, arena));
  case Formula::Or:
    return arena.makeOr(substitute(*F.operands()[0], Substitutions, arena),
                        substitute(*F.operands()[1], Substitutions, arena));
  case Formula::Implies:
    return arena.makeImplies(
        substitute(*F.operands()[0], Substitutions, arena),
        substitute(*F.operands()[1], Substitutions, arena));
  case Formula::Equal:
    return arena.makeEquals(substitute(*F.operands()[0], Substitutions, arena),
                            substitute(*F.operands()[1], Substitutions, arena));
  }
  toolchain_unreachable("Unknown formula kind");
}

// Returns the result of replacing atoms in `Atoms` with the leader of their
// equivalence class in `EquivalentAtoms`.
// Atoms that don't have an equivalence class in `EquivalentAtoms` are inserted
// into it as single-member equivalence classes.
static toolchain::DenseSet<Atom>
projectToLeaders(const toolchain::DenseSet<Atom> &Atoms,
                 toolchain::EquivalenceClasses<Atom> &EquivalentAtoms) {
  toolchain::DenseSet<Atom> Result;

  for (Atom Atom : Atoms)
    Result.insert(EquivalentAtoms.getOrInsertLeaderValue(Atom));

  return Result;
}

// Returns the atoms in the equivalence class for the leader identified by
// `LeaderIt`.
static toolchain::SmallVector<Atom>
atomsInEquivalenceClass(const toolchain::EquivalenceClasses<Atom> &EquivalentAtoms,
                        const Atom &At) {
  toolchain::SmallVector<Atom> Result;
  for (auto MemberIt = EquivalentAtoms.findLeader(At);
       MemberIt != EquivalentAtoms.member_end(); ++MemberIt)
    Result.push_back(*MemberIt);
  return Result;
}

void simplifyConstraints(toolchain::SetVector<const Formula *> &Constraints,
                         Arena &arena, SimplifyConstraintsInfo *Info) {
  auto contradiction = [&]() {
    Constraints.clear();
    Constraints.insert(&arena.makeLiteral(false));
  };

  toolchain::EquivalenceClasses<Atom> EquivalentAtoms;
  toolchain::DenseSet<Atom> TrueAtoms;
  toolchain::DenseSet<Atom> FalseAtoms;

  while (true) {
    for (const auto *Constraint : Constraints) {
      switch (Constraint->kind()) {
      case Formula::AtomRef:
        TrueAtoms.insert(Constraint->getAtom());
        break;
      case Formula::Not:
        if (Constraint->operands()[0]->kind() == Formula::AtomRef)
          FalseAtoms.insert(Constraint->operands()[0]->getAtom());
        break;
      case Formula::Equal: {
        ArrayRef<const Formula *> operands = Constraint->operands();
        if (operands[0]->kind() == Formula::AtomRef &&
            operands[1]->kind() == Formula::AtomRef) {
          EquivalentAtoms.unionSets(operands[0]->getAtom(),
                                    operands[1]->getAtom());
        }
        break;
      }
      default:
        break;
      }
    }

    TrueAtoms = projectToLeaders(TrueAtoms, EquivalentAtoms);
    FalseAtoms = projectToLeaders(FalseAtoms, EquivalentAtoms);

    toolchain::DenseMap<Atom, const Formula *> Substitutions;
    for (const auto &E : EquivalentAtoms) {
      Atom TheAtom = E->getData();
      Atom Leader = EquivalentAtoms.getLeaderValue(TheAtom);
      if (TrueAtoms.contains(Leader)) {
        if (FalseAtoms.contains(Leader)) {
          contradiction();
          return;
        }
        Substitutions.insert({TheAtom, &arena.makeLiteral(true)});
      } else if (FalseAtoms.contains(Leader)) {
        Substitutions.insert({TheAtom, &arena.makeLiteral(false)});
      } else if (TheAtom != Leader) {
        Substitutions.insert({TheAtom, &arena.makeAtomRef(Leader)});
      }
    }

    toolchain::SetVector<const Formula *> NewConstraints;
    for (const auto *Constraint : Constraints) {
      const Formula &NewConstraint =
          substitute(*Constraint, Substitutions, arena);
      if (NewConstraint.isLiteral(true))
        continue;
      if (NewConstraint.isLiteral(false)) {
        contradiction();
        return;
      }
      if (NewConstraint.kind() == Formula::And) {
        NewConstraints.insert(NewConstraint.operands()[0]);
        NewConstraints.insert(NewConstraint.operands()[1]);
        continue;
      }
      NewConstraints.insert(&NewConstraint);
    }

    if (NewConstraints == Constraints)
      break;
    Constraints = std::move(NewConstraints);
  }

  if (Info) {
    for (const auto &E : EquivalentAtoms) {
      if (!E->isLeader())
        continue;
      Atom At = *EquivalentAtoms.findLeader(*E);
      if (TrueAtoms.contains(At) || FalseAtoms.contains(At))
        continue;
      toolchain::SmallVector<Atom> Atoms =
          atomsInEquivalenceClass(EquivalentAtoms, At);
      if (Atoms.size() == 1)
        continue;
      std::sort(Atoms.begin(), Atoms.end());
      Info->EquivalentAtoms.push_back(std::move(Atoms));
    }
    for (Atom At : TrueAtoms)
      Info->TrueAtoms.append(atomsInEquivalenceClass(EquivalentAtoms, At));
    std::sort(Info->TrueAtoms.begin(), Info->TrueAtoms.end());
    for (Atom At : FalseAtoms)
      Info->FalseAtoms.append(atomsInEquivalenceClass(EquivalentAtoms, At));
    std::sort(Info->FalseAtoms.begin(), Info->FalseAtoms.end());
  }
}

} // namespace dataflow
} // namespace language::Core
