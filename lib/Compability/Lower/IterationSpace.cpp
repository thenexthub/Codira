//===-- IterationSpace.cpp ------------------------------------------------===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Lower/IterationSpace.h"
#include "language/Compability/Evaluate/expression.h"
#include "language/Compability/Lower/AbstractConverter.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "toolchain/Support/Debug.h"
#include <optional>

#define DEBUG_TYPE "flang-lower-iteration-space"

namespace {

/// This class can recover the base array in an expression that contains
/// explicit iteration space symbols. Most of the class can be ignored as it is
/// boilerplate language::Compability::evaluate::Expr traversal.
class ArrayBaseFinder {
public:
  using RT = bool;

  ArrayBaseFinder(toolchain::ArrayRef<language::Compability::lower::FrontEndSymbol> syms)
      : controlVars(syms) {}

  template <typename T>
  void operator()(const T &x) {
    (void)find(x);
  }

  /// Get the list of bases.
  toolchain::ArrayRef<language::Compability::lower::ExplicitIterSpace::ArrayBases>
  getBases() const {
    LLVM_DEBUG(toolchain::dbgs()
               << "number of array bases found: " << bases.size() << '\n');
    return bases;
  }

private:
  // First, the cases that are of interest.
  RT find(const language::Compability::semantics::Symbol &symbol) {
    if (symbol.Rank() > 0) {
      bases.push_back(&symbol);
      return true;
    }
    return {};
  }
  RT find(const language::Compability::evaluate::Component &x) {
    auto found = find(x.base());
    if (!found && x.base().Rank() == 0 && x.Rank() > 0) {
      bases.push_back(&x);
      return true;
    }
    return found;
  }
  RT find(const language::Compability::evaluate::ArrayRef &x) {
    for (const auto &sub : x.subscript())
      (void)find(sub);
    if (x.base().IsSymbol()) {
      if (x.Rank() > 0 || intersection(x.subscript())) {
        bases.push_back(&x);
        return true;
      }
      return {};
    }
    auto found = find(x.base());
    if (!found && ((x.base().Rank() == 0 && x.Rank() > 0) ||
                   intersection(x.subscript()))) {
      bases.push_back(&x);
      return true;
    }
    return found;
  }
  RT find(const language::Compability::evaluate::Triplet &x) {
    if (const auto *lower = x.GetLower())
      (void)find(*lower);
    if (const auto *upper = x.GetUpper())
      (void)find(*upper);
    return find(x.GetStride());
  }
  RT find(const language::Compability::evaluate::IndirectSubscriptIntegerExpr &x) {
    return find(x.value());
  }
  RT find(const language::Compability::evaluate::Subscript &x) { return find(x.u); }
  RT find(const language::Compability::evaluate::DataRef &x) { return find(x.u); }
  RT find(const language::Compability::evaluate::CoarrayRef &x) {
    assert(false && "coarray reference");
    return {};
  }

  template <typename A>
  bool intersection(const A &subscripts) {
    return language::Compability::lower::symbolsIntersectSubscripts(controlVars, subscripts);
  }

  // The rest is traversal boilerplate and can be ignored.
  RT find(const language::Compability::evaluate::Substring &x) { return find(x.parent()); }
  template <typename A>
  RT find(const language::Compability::semantics::SymbolRef x) {
    return find(*x);
  }
  RT find(const language::Compability::evaluate::NamedEntity &x) {
    if (x.IsSymbol())
      return find(x.GetFirstSymbol());
    return find(x.GetComponent());
  }

  template <typename A, bool C>
  RT find(const language::Compability::common::Indirection<A, C> &x) {
    return find(x.value());
  }
  template <typename A>
  RT find(const std::unique_ptr<A> &x) {
    return find(x.get());
  }
  template <typename A>
  RT find(const std::shared_ptr<A> &x) {
    return find(x.get());
  }
  template <typename A>
  RT find(const A *x) {
    if (x)
      return find(*x);
    return {};
  }
  template <typename A>
  RT find(const std::optional<A> &x) {
    if (x)
      return find(*x);
    return {};
  }
  template <typename... A>
  RT find(const std::variant<A...> &u) {
    return language::Compability::common::visit([&](const auto &v) { return find(v); }, u);
  }
  template <typename A>
  RT find(const std::vector<A> &x) {
    for (auto &v : x)
      (void)find(v);
    return {};
  }
  RT find(const language::Compability::evaluate::BOZLiteralConstant &) { return {}; }
  RT find(const language::Compability::evaluate::NullPointer &) { return {}; }
  template <typename T>
  RT find(const language::Compability::evaluate::Constant<T> &x) {
    return {};
  }
  RT find(const language::Compability::evaluate::StaticDataObject &) { return {}; }
  RT find(const language::Compability::evaluate::ImpliedDoIndex &) { return {}; }
  RT find(const language::Compability::evaluate::BaseObject &x) {
    (void)find(x.u);
    return {};
  }
  RT find(const language::Compability::evaluate::TypeParamInquiry &) { return {}; }
  RT find(const language::Compability::evaluate::ComplexPart &x) { return {}; }
  template <typename T>
  RT find(const language::Compability::evaluate::Designator<T> &x) {
    return find(x.u);
  }
  RT find(const language::Compability::evaluate::DescriptorInquiry &) { return {}; }
  RT find(const language::Compability::evaluate::SpecificIntrinsic &) { return {}; }
  RT find(const language::Compability::evaluate::ProcedureDesignator &x) { return {}; }
  RT find(const language::Compability::evaluate::ProcedureRef &x) {
    (void)find(x.proc());
    if (x.IsElemental())
      (void)find(x.arguments());
    return {};
  }
  RT find(const language::Compability::evaluate::ActualArgument &x) {
    if (const auto *sym = x.GetAssumedTypeDummy())
      (void)find(*sym);
    else
      (void)find(x.UnwrapExpr());
    return {};
  }
  template <typename T>
  RT find(const language::Compability::evaluate::FunctionRef<T> &x) {
    (void)find(static_cast<const language::Compability::evaluate::ProcedureRef &>(x));
    return {};
  }
  template <typename T>
  RT find(const language::Compability::evaluate::ArrayConstructorValue<T> &) {
    return {};
  }
  template <typename T>
  RT find(const language::Compability::evaluate::ArrayConstructorValues<T> &) {
    return {};
  }
  template <typename T>
  RT find(const language::Compability::evaluate::ImpliedDo<T> &) {
    return {};
  }
  RT find(const language::Compability::semantics::ParamValue &) { return {}; }
  RT find(const language::Compability::semantics::DerivedTypeSpec &) { return {}; }
  RT find(const language::Compability::evaluate::StructureConstructor &) { return {}; }
  template <typename D, typename R, typename O>
  RT find(const language::Compability::evaluate::Operation<D, R, O> &op) {
    (void)find(op.left());
    return false;
  }
  template <typename D, typename R, typename LO, typename RO>
  RT find(const language::Compability::evaluate::Operation<D, R, LO, RO> &op) {
    (void)find(op.left());
    (void)find(op.right());
    return false;
  }
  RT find(const language::Compability::evaluate::Relational<language::Compability::evaluate::SomeType> &x) {
    (void)find(x.u);
    return {};
  }
  template <typename T>
  RT find(const language::Compability::evaluate::Expr<T> &x) {
    (void)find(x.u);
    return {};
  }

  toolchain::SmallVector<language::Compability::lower::ExplicitIterSpace::ArrayBases> bases;
  toolchain::SmallVector<language::Compability::lower::FrontEndSymbol> controlVars;
};

} // namespace

void language::Compability::lower::ExplicitIterSpace::leave() {
  ccLoopNest.pop_back();
  --forallContextOpen;
  conditionalCleanup();
}

void language::Compability::lower::ExplicitIterSpace::addSymbol(
    language::Compability::lower::FrontEndSymbol sym) {
  assert(!symbolStack.empty());
  symbolStack.back().push_back(sym);
}

void language::Compability::lower::ExplicitIterSpace::exprBase(language::Compability::lower::FrontEndExpr x,
                                                 bool lhs) {
  ArrayBaseFinder finder(collectAllSymbols());
  finder(*x);
  toolchain::ArrayRef<language::Compability::lower::ExplicitIterSpace::ArrayBases> bases =
      finder.getBases();
  if (rhsBases.empty())
    endAssign();
  if (lhs) {
    if (bases.empty()) {
      lhsBases.push_back(std::nullopt);
      return;
    }
    assert(bases.size() >= 1 && "must detect an array reference on lhs");
    if (bases.size() > 1)
      rhsBases.back().append(bases.begin(), bases.end() - 1);
    lhsBases.push_back(bases.back());
    return;
  }
  rhsBases.back().append(bases.begin(), bases.end());
}

void language::Compability::lower::ExplicitIterSpace::endAssign() { rhsBases.emplace_back(); }

void language::Compability::lower::ExplicitIterSpace::pushLevel() {
  symbolStack.push_back(toolchain::SmallVector<language::Compability::lower::FrontEndSymbol>{});
}

void language::Compability::lower::ExplicitIterSpace::popLevel() { symbolStack.pop_back(); }

void language::Compability::lower::ExplicitIterSpace::conditionalCleanup() {
  if (forallContextOpen == 0) {
    // Exiting the outermost FORALL context.
    // Cleanup any residual mask buffers.
    outermostContext().finalizeAndReset();
    // Clear and reset all the cached information.
    symbolStack.clear();
    lhsBases.clear();
    rhsBases.clear();
    loadBindings.clear();
    ccLoopNest.clear();
    innerArgs.clear();
    outerLoop = std::nullopt;
    clearLoops();
    counter = 0;
  }
}

std::optional<size_t>
language::Compability::lower::ExplicitIterSpace::findArgPosition(fir::ArrayLoadOp load) {
  if (lhsBases[counter]) {
    auto ld = loadBindings.find(*lhsBases[counter]);
    std::optional<size_t> optPos;
    if (ld != loadBindings.end() && ld->second == load)
      optPos = static_cast<size_t>(0u);
    assert(optPos.has_value() && "load does not correspond to lhs");
    return optPos;
  }
  return std::nullopt;
}

toolchain::SmallVector<language::Compability::lower::FrontEndSymbol>
language::Compability::lower::ExplicitIterSpace::collectAllSymbols() {
  toolchain::SmallVector<language::Compability::lower::FrontEndSymbol> result;
  for (toolchain::SmallVector<FrontEndSymbol> vec : symbolStack)
    result.append(vec.begin(), vec.end());
  return result;
}

toolchain::raw_ostream &
language::Compability::lower::operator<<(toolchain::raw_ostream &s,
                           const language::Compability::lower::ImplicitIterSpace &e) {
  for (const toolchain::SmallVector<
           language::Compability::lower::ImplicitIterSpace::FrontEndMaskExpr> &xs :
       e.getMasks()) {
    s << "{ ";
    for (const language::Compability::lower::ImplicitIterSpace::FrontEndMaskExpr &x : xs)
      x->AsFortran(s << '(') << "), ";
    s << "}\n";
  }
  return s;
}

toolchain::raw_ostream &
language::Compability::lower::operator<<(toolchain::raw_ostream &s,
                           const language::Compability::lower::ExplicitIterSpace &e) {
  auto dump = [&](const auto &u) {
    language::Compability::common::visit(
        language::Compability::common::visitors{
            [&](const language::Compability::semantics::Symbol *y) {
              s << "  " << *y << '\n';
            },
            [&](const language::Compability::evaluate::ArrayRef *y) {
              s << "  ";
              if (y->base().IsSymbol())
                s << y->base().GetFirstSymbol();
              else
                s << y->base().GetComponent().GetLastSymbol();
              s << '\n';
            },
            [&](const language::Compability::evaluate::Component *y) {
              s << "  " << y->GetLastSymbol() << '\n';
            }},
        u);
  };
  s << "LHS bases:\n";
  for (const std::optional<language::Compability::lower::ExplicitIterSpace::ArrayBases> &u :
       e.lhsBases)
    if (u)
      dump(*u);
  s << "RHS bases:\n";
  for (const toolchain::SmallVector<language::Compability::lower::ExplicitIterSpace::ArrayBases>
           &bases : e.rhsBases) {
    for (const language::Compability::lower::ExplicitIterSpace::ArrayBases &u : bases)
      dump(u);
    s << '\n';
  }
  return s;
}

void language::Compability::lower::ImplicitIterSpace::dump() const {
  toolchain::errs() << *this << '\n';
}

void language::Compability::lower::ExplicitIterSpace::dump() const {
  toolchain::errs() << *this << '\n';
}
