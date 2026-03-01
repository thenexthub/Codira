//===- Var.cpp ------------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "Var.h"
#include "DimLvlMap.h"

using namespace mlir;
using namespace mlir::sparse_tensor;
using namespace mlir::sparse_tensor::ir_detail;

//===----------------------------------------------------------------------===//
// `VarKind` helpers.
//===----------------------------------------------------------------------===//

/// For use in foreach loops.
static constexpr const VarKind everyVarKind[] = {
    VarKind::Dimension, VarKind::Symbol, VarKind::Level};

//===----------------------------------------------------------------------===//
// `Var` implementation.
//===----------------------------------------------------------------------===//

std::string Var::str() const {
  std::string str;
  toolchain::raw_string_ostream os(str);
  print(os);
  return str;
}

void Var::print(AsmPrinter &printer) const { print(printer.getStream()); }

void Var::print(toolchain::raw_ostream &os) const {
  os << toChar(getKind()) << getNum();
}

void Var::dump() const {
  print(toolchain::errs());
  toolchain::errs() << "\n";
}

//===----------------------------------------------------------------------===//
// `Ranks` implementation.
//===----------------------------------------------------------------------===//

bool Ranks::operator==(Ranks const &other) const {
  for (const auto vk : everyVarKind)
    if (getRank(vk) != other.getRank(vk))
      return false;
  return true;
}

bool Ranks::isValid(DimLvlExpr expr) const {
  assert(expr);
  // Compute the maximum identifiers for symbol-vars and dim/lvl-vars
  // (each `DimLvlExpr` only allows one kind of non-symbol variable).
  int64_t maxSym = -1, maxVar = -1;
  mlir::getMaxDimAndSymbol<ArrayRef<AffineExpr>>({{expr.getAffineExpr()}},
                                                 maxVar, maxSym);
  return maxSym < getSymRank() && maxVar < getRank(expr.getAllowedVarKind());
}

//===----------------------------------------------------------------------===//
// `VarSet` implementation.
//===----------------------------------------------------------------------===//

VarSet::VarSet(Ranks const &ranks) {
  for (const auto vk : everyVarKind)
    impl[vk] = toolchain::SmallBitVector(ranks.getRank(vk));
  assert(getRanks() == ranks);
}

bool VarSet::contains(Var var) const {
  // NOTE: We make sure to return false on OOB, for consistency with
  // the `anyCommon` implementation of `VarSet::occursIn(VarSet)`.
  // However beware that, as always with silencing OOB, this can hide
  // bugs in client code.
  const toolchain::SmallBitVector &bits = impl[var.getKind()];
  const auto num = var.getNum();
  return num < bits.size() && bits[num];
}

void VarSet::add(Var var) {
  // NOTE: `SmallBitVector::operator[]` will raise assertion errors for OOB.
  impl[var.getKind()][var.getNum()] = true;
}

void VarSet::add(VarSet const &other) {
  // NOTE: `SmallBitVector::operator&=` will implicitly resize
  // the bitvector (unlike `BitVector::operator&=`), so we add an
  // assertion against OOB for consistency with the implementation
  // of `VarSet::add(Var)`.
  for (const auto vk : everyVarKind) {
    assert(impl[vk].size() >= other.impl[vk].size());
    impl[vk] &= other.impl[vk];
  }
}

void VarSet::add(DimLvlExpr expr) {
  if (!expr)
    return;
  switch (expr.getAffineKind()) {
  case AffineExprKind::Constant:
    return;
  case AffineExprKind::SymbolId:
    add(expr.castSymVar());
    return;
  case AffineExprKind::DimId:
    add(expr.castDimLvlVar());
    return;
  case AffineExprKind::Add:
  case AffineExprKind::Mul:
  case AffineExprKind::Mod:
  case AffineExprKind::FloorDiv:
  case AffineExprKind::CeilDiv: {
    const auto [lhs, op, rhs] = expr.unpackBinop();
    (void)op;
    add(lhs);
    add(rhs);
    return;
  }
  }
  llvm_unreachable("unknown AffineExprKind");
}

//===----------------------------------------------------------------------===//
// `VarInfo` implementation.
//===----------------------------------------------------------------------===//

void VarInfo::setNum(Var::Num n) {
  assert(!hasNum() && "Var::Num is already set");
  assert(Var::isWF_Num(n) && "Var::Num is too large");
  num = n;
}

//===----------------------------------------------------------------------===//
// `VarEnv` implementation.
//===----------------------------------------------------------------------===//

/// Helper function for `assertUsageConsistency` to better handle SMLoc
/// mismatches.
[[maybe_unused]] static toolchain::SMLoc minSMLoc(AsmParser &parser, toolchain::SMLoc sm1,
                                             toolchain::SMLoc sm2) {
  const auto loc1 = dyn_cast<FileLineColLoc>(parser.getEncodedSourceLoc(sm1));
  assert(loc1 && "Could not get `FileLineColLoc` for first `SMLoc`");
  const auto loc2 = dyn_cast<FileLineColLoc>(parser.getEncodedSourceLoc(sm2));
  assert(loc2 && "Could not get `FileLineColLoc` for second `SMLoc`");
  if (loc1.getFilename() != loc2.getFilename())
    return SMLoc();
  const auto pair1 = std::make_pair(loc1.getLine(), loc1.getColumn());
  const auto pair2 = std::make_pair(loc2.getLine(), loc2.getColumn());
  return pair1 <= pair2 ? sm1 : sm2;
}

static bool isInternalConsistent(VarEnv const &env, VarInfo::ID id,
                                 StringRef name) {
  const auto &var = env.access(id);
  return (var.getName() == name && var.getID() == id);
}

static bool isUsageConsistent(VarEnv const &env, VarInfo::ID id,
                              toolchain::SMLoc loc, VarKind vk) {
  const auto &var = env.access(id);
  return var.getKind() == vk;
}

std::optional<VarInfo::ID> VarEnv::lookup(StringRef name) const {
  const auto iter = ids.find(name);
  if (iter == ids.end())
    return std::nullopt;
  const auto id = iter->second;
  if (!isInternalConsistent(*this, id, name))
    return std::nullopt;
  return id;
}

std::optional<std::pair<VarInfo::ID, bool>>
VarEnv::create(StringRef name, toolchain::SMLoc loc, VarKind vk, bool verifyUsage) {
  const auto &[iter, didInsert] = ids.try_emplace(name, nextID());
  const auto id = iter->second;
  if (didInsert) {
    vars.emplace_back(id, name, loc, vk);
  } else {
  if (!isInternalConsistent(*this, id, name))
    return std::nullopt;
  if (verifyUsage)
    if (!isUsageConsistent(*this, id, loc, vk))
      return std::nullopt;
  }
  return std::make_pair(id, didInsert);
}

std::optional<std::pair<VarInfo::ID, bool>>
VarEnv::lookupOrCreate(Policy creationPolicy, StringRef name, toolchain::SMLoc loc,
                       VarKind vk) {
  switch (creationPolicy) {
  case Policy::MustNot: {
    const auto oid = lookup(name);
    if (!oid)
      return std::nullopt;  // Doesn't exist, but must not create.
    if (!isUsageConsistent(*this, *oid, loc, vk))
      return std::nullopt;
    return std::make_pair(*oid, false);
  }
  case Policy::May:
    return create(name, loc, vk, /*verifyUsage=*/true);
  case Policy::Must: {
    const auto res = create(name, loc, vk, /*verifyUsage=*/false);
    const auto didCreate = res->second;
    if (!didCreate)
      return std::nullopt;  // Already exists, but must create.
    return res;
  }
  }
  llvm_unreachable("unknown Policy");
}

Var VarEnv::bindUnusedVar(VarKind vk) { return Var(vk, nextNum[vk]++); }
Var VarEnv::bindVar(VarInfo::ID id) {
  auto &info = access(id);
  const auto var = bindUnusedVar(info.getKind());
  info.setNum(var.getNum());
  return var;
}

InFlightDiagnostic VarEnv::emitErrorIfAnyUnbound(AsmParser &parser) const {
  for (const auto &var : vars)
    if (!var.hasNum())
      return parser.emitError(var.getLoc(),
                              "Unbound variable: " + var.getName());
  return {};
}

//===----------------------------------------------------------------------===//
