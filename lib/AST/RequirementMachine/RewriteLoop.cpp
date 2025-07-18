//===--- RewriteLoop.cpp - Identities between rewrite rules ---------------===//
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
// This file defines data types used for representing redundancies among
// rewrite rules. The information encoded in these types is ultimately used
// for generic signature minimization.
//
// A RewriteStep is a single primitive transformation; the canonical example is
// the application of a rewrite rule, possibly to a subterm.
//
// A RewritePath is a composition of RewriteSteps describing the transformation
// of a term into another term. One place where a RewritePath originates is
// RewriteSystem::simplify(); that method takes an optional RewritePath argument
// to which the series of RewriteSteps performed during simplification are
// appended. If the term was already canonical, the resulting path is empty,
// otherwise it will consist of at least one RewriteStep.
//
// Simplification always applies rules by replacing a subterm equal to the LHS
// with the RHS where LHS > RHS, so the RewriteSteps constructed there always
// make the term shorter. However, more generally, RewriteSteps  can also
// express the inverse rewrite, where RHS is replaced with LHS, making the term
// longer.
//
// Inverted RewriteSteps are constructed in the Knuth-Bendix completion
// algorithm. A simple example is where two rules (U.V => X) and (V.W => Y)
// overlap on the term U.V.W. Then the induced rule (X.W => U.Y) (assuming that
// X.W > U.Y) can be described by a RewritePath which begins at X.W, applies
// the inverted rule (X => U.V) to the subterm X to obtain U.V.W, then applies
// the rule (V.W => Y) to the subterm V.W to obtain U.Y.
//
// A RewriteLoop is a path that begins and ends at the same term. A RewriteLoop
// describes a _redundancy_. For example, when completion adds a new rule to
// resolve an overlap, it constructs a RewritePath describing this new rule in
// terms of existing rules; by adding an additional rewrite step corresponding
// to the new rule, we get a loop that begins and ends at the same point, or in
// other words, a RewriteLoop.
//
// In the above example, we have a RewritePath from X.W to U.Y via the two
// existing rewrite rules with the overlap term U.V.W in the middle. If we then
// add a third rewrite step for the new rule inverted, (U.Y => X.W), we get a
// loop that begins and ends at X.W. This loop encodes that the new rule
// (X.W => U.Y) is redundant because it can be expressed in terms of other rules.
//
// The homotopy reduction algorithm in HomotopyReduction.cpp uses rewrite loops
// to find a minimal set of rewrite rules, which are then used to construct a
// minimal generic signature.
//
//===----------------------------------------------------------------------===//

#include "language/AST/Type.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Range.h"
#include "toolchain/Support/raw_ostream.h"
#include <algorithm>
#include "RewriteSystem.h"

using namespace language;
using namespace rewriting;

/// Dumps the rewrite step that was applied to \p term. Mutates \p term to
/// reflect the application of the rule.
void RewriteStep::dump(toolchain::raw_ostream &out,
                       RewritePathEvaluator &evaluator,
                       const RewriteSystem &system) const {
  switch (Kind) {
  case Rule: {
    auto result = evaluator.applyRewriteRule(*this, system);

    if (!result.prefix.empty()) {
      out << result.prefix;
      out << ".";
    }
    out << "(" << result.lhs << " => " << result.rhs << ")";
    if (!result.suffix.empty()) {
      out << ".";
      out << result.suffix;
    }

    break;
  }
  case PrefixSubstitutions: {
    auto pair = evaluator.applyPrefixSubstitutions(*this, system);

    out << "(σ";
    out << (Inverse ? " - " : " + ");
    out << pair.first << ")";

    if (!pair.second.empty())
      out << "." << pair.second;

    break;
  }
  case Shift: {
    evaluator.applyShift(*this, system);

    out << (Inverse ? "B>A" : "A>B");
    break;
  }
  case Decompose: {
    evaluator.applyDecompose(*this, system);

    out << (Inverse ? "Compose(" : "Decompose(");
    out << Arg << ")";
    break;
  }
  case Relation: {
    auto result = evaluator.applyRelation(*this, system);

    if (!result.prefix.empty()) {
      out << result.prefix;
      out << ".";
    }
    out << "(" << result.lhs << " =>> " << result.rhs << ")";
    if (!result.suffix.empty()) {
      out << ".";
      out << result.suffix;
    }

    break;
  }
  case DecomposeConcrete: {
    evaluator.applyDecomposeConcrete(*this, system);

    out << (Inverse ? "ComposeConcrete(" : "DecomposeConcrete(");

    const auto &difference = system.getTypeDifference(Arg);

    out << difference.LHS << " : " << difference.RHS << ")";
    break;
  }
  case LeftConcreteProjection: {
    evaluator.applyLeftConcreteProjection(*this, system);

    out << "LeftConcrete" << (Inverse ? "In" : "Pro") << "jection(";

    const auto &difference = system.getTypeDifference(
        getTypeDifferenceID());

    out << difference.LHS << " : " << difference.RHS << ")";
    break;
  }
  case RightConcreteProjection: {
    evaluator.applyRightConcreteProjection(*this, system);

    out << "RightConcrete" << (Inverse ? "In" : "Pro") << "jection(";

    const auto &difference = system.getTypeDifference(
        getTypeDifferenceID());

    out << difference.LHS << " : " << difference.RHS << ")";
    break;
  }
  }
}

/// Invert a rewrite path, producing a path that rewrites the original path's
/// destination back to the original path's source.
void RewritePath::invert() {
  std::reverse(Steps.begin(), Steps.end());

  for (auto &step : Steps)
    step.invert();
}

/// Given a rewrite rule which appears exactly once in a loop
/// without context, return a new definition for this rewrite rule.
/// The new definition is the path obtained by deleting the
/// rewrite rule from the loop.
RewritePath RewritePath::splitCycleAtRule(unsigned ruleID) const {
  // A cycle is a path from the basepoint to the basepoint.
  // Somewhere in this path, an application of \p ruleID
  // appears in an empty context.

  // First, we split the cycle into two paths:
  //
  // (1) A path from the basepoint to the rule's
  // left hand side,
  RewritePath basepointToLhs;
  // (2) And a path from the rule's right hand side
  // to the basepoint.
  RewritePath rhsToBasepoint;

  // Because the rule only appears once, we know that basepointToLhs
  // and rhsToBasepoint do not involve the rule itself.

  // If the rule is inverted, we have to invert the whole thing
  // again at the end.
  bool ruleWasInverted = false;

  bool sawRule = false;

  for (auto step : Steps) {
    switch (step.Kind) {
    case RewriteStep::Rule: {
      if (step.getRuleID() != ruleID)
        break;

      ASSERT(!sawRule && "Rule appears more than once?");
      ASSERT(!step.isInContext() && "Rule appears in context?");

      ruleWasInverted = step.Inverse;
      sawRule = true;
      continue;
    }
    case RewriteStep::PrefixSubstitutions:
    case RewriteStep::Shift:
    case RewriteStep::Decompose:
    case RewriteStep::Relation:
    case RewriteStep::DecomposeConcrete:
    case RewriteStep::LeftConcreteProjection:
    case RewriteStep::RightConcreteProjection:
      break;
    }

    if (sawRule)
      rhsToBasepoint.add(step);
    else
      basepointToLhs.add(step);
  }

  // Build a path from the rule's lhs to the rule's rhs via the
  // basepoint.
  RewritePath result = rhsToBasepoint;
  result.append(basepointToLhs);

  // We want a path from the lhs to the rhs, so invert it unless
  // the rewrite step was also inverted.
  if (!ruleWasInverted)
    result.invert();

  return result;
}

/// Replace every rewrite step involving the given rewrite rule with
/// either the replacement path (or its inverse, if the step was
/// inverted).
///
/// The replacement path is re-contextualized at each occurrence of a
/// rewrite step involving the given rule.
///
/// Returns true if any rewrite steps were replaced; false means the
/// rule did not appear in this path.
bool RewritePath::replaceRulesWithPaths(
    toolchain::function_ref<const RewritePath *(unsigned)> fn) {
  bool foundAny = false;

  for (const auto &step : Steps) {
    if (step.Kind == RewriteStep::Rule &&
        fn(step.getRuleID()) != nullptr) {
      foundAny = true;
      break;
    }
  }

  if (!foundAny)
    return false;

  SmallVector<RewriteStep, 4> newSteps;

  for (const auto &step : Steps) {
    switch (step.Kind) {
    case RewriteStep::Rule: {
      auto *replacementPath = fn(step.getRuleID());
      if (replacementPath == nullptr) {
        newSteps.push_back(step);
        break;
      }

      // Ok, we found a rewrite step referencing a redundant rule.
      // Replace this step with the provided path. If this rewrite step has
      // context, the path's own steps must be re-contextualized.

      // Keep track of rewrite step pairs which push and pop the stack. Any
      // rewrite steps enclosed with a push/pop are not re-contextualized.
      unsigned pushCount = 0;

      auto recontextualizeStep = [&](RewriteStep newStep) {
        bool inverse = newStep.Inverse ^ step.Inverse;

        if (newStep.pushesTermsOnStack() && inverse) {
          ASSERT(pushCount > 0);
          --pushCount;
        }

        if (pushCount == 0) {
          newStep.StartOffset += step.StartOffset;
          newStep.EndOffset += step.EndOffset;
        }

        newStep.Inverse = inverse;
        newSteps.push_back(newStep);

        if (newStep.pushesTermsOnStack() && !inverse) {
          ++pushCount;
        }
      };

      // If this rewrite step is inverted, invert the entire path.
      if (step.Inverse) {
        for (auto newStep : toolchain::reverse(*replacementPath))
          recontextualizeStep(newStep);
      } else {
        for (auto newStep : *replacementPath)
          recontextualizeStep(newStep);
      }

      // Rewrite steps which push and pop the stack must come in balanced pairs.
      ASSERT(pushCount == 0);

      break;
    }
    case RewriteStep::PrefixSubstitutions:
    case RewriteStep::Shift:
    case RewriteStep::Decompose:
    case RewriteStep::Relation:
    case RewriteStep::DecomposeConcrete:
    case RewriteStep::LeftConcreteProjection:
    case RewriteStep::RightConcreteProjection:
      newSteps.push_back(step);
      break;
    }
  }

  std::swap(newSteps, Steps);
  return true;
}

bool RewritePath::replaceRuleWithPath(unsigned ruleID,
                                      const RewritePath &path) {
  return replaceRulesWithPaths(
      [&](unsigned otherRuleID) -> const RewritePath * {
        if (ruleID == otherRuleID)
          return &path;

        return nullptr;
      });
}

SmallVector<unsigned, 1>
RewritePath::findRulesAppearingOnceInEmptyContext(const MutableTerm &term,
                                                  const RewriteSystem &system) const {
  // Rules appearing in empty context (possibly more than once).
  toolchain::SmallDenseSet<unsigned, 2> rulesInEmptyContext;
  // The number of times each rule appears (with or without context).
  toolchain::SmallDenseMap<unsigned, unsigned, 2> ruleFrequency;

  RewritePathEvaluator evaluator(term);
  for (auto step : Steps) {
    switch (step.Kind) {
    case RewriteStep::Rule: {
      if (!step.isInContext() && !evaluator.isInContext())
        rulesInEmptyContext.insert(step.getRuleID());

      ++ruleFrequency[step.getRuleID()];
      break;
    }

    case RewriteStep::LeftConcreteProjection:
    case RewriteStep::Decompose:
    case RewriteStep::PrefixSubstitutions:
    case RewriteStep::Shift:
    case RewriteStep::Relation:
    case RewriteStep::DecomposeConcrete:
    case RewriteStep::RightConcreteProjection:
      break;
    }

    evaluator.apply(step, system);
  }

  // Collect all rules that we saw exactly once in empty context.
  SmallVector<unsigned, 1> rulesOnceInEmptyContext;
  for (auto rule : rulesInEmptyContext) {
    auto found = ruleFrequency.find(rule);
    ASSERT(found != ruleFrequency.end());

    if (found->second == 1)
      rulesOnceInEmptyContext.push_back(rule);
  }

  return rulesOnceInEmptyContext;
}

/// Dumps a series of rewrite steps applied to \p term.
void RewritePath::dump(toolchain::raw_ostream &out,
                       MutableTerm term,
                       const RewriteSystem &system) const {
  RewritePathEvaluator evaluator(term);
  bool first = true;

  for (const auto &step : Steps) {
    if (!first) {
      out << " ⊗ ";
    } else {
      first = false;
    }

    step.dump(out, evaluator, system);
  }
}

void RewritePath::dumpLong(toolchain::raw_ostream &out,
                           MutableTerm term,
                           const RewriteSystem &system) const {
  RewritePathEvaluator evaluator(term);

  for (const auto &step : Steps) {
    evaluator.dump(out);
    evaluator.apply(step, system);
    out << "\n";
  }

  evaluator.dump(out);
}

void RewriteLoop::verify(const RewriteSystem &system) const {
  RewritePathEvaluator evaluator(Basepoint);

  for (const auto &step : Path) {
    evaluator.apply(step, system);
  }

  if (evaluator.getCurrentTerm() != Basepoint) {
    ABORT([&](auto &out) {
      out << "Not a loop: ";
      dump(out, system);
    });
  }

  if (evaluator.isInContext()) {
    ABORT([&](auto &out) {
      out << "Leftover terms on evaluator stack\n";
      evaluator.dump(out);
    });
  }
}

/// Recompute various cached values if needed.
void RewriteLoop::recompute(const RewriteSystem &system) {
  if (!Dirty)
    return;
  Dirty = 0;

  Useful = 0;
  ProjectionCount = 0;
  DecomposeCount = 0;
  HasConcreteTypeAliasRule = 0;

  RewritePathEvaluator evaluator(Basepoint);
  for (auto step : Path) {
    switch (step.Kind) {
    case RewriteStep::Rule: {
      Useful |= (!step.isInContext() && !evaluator.isInContext());

      const auto &rule = system.getRule(step.getRuleID());
      if (rule.isDerivedFromConcreteProtocolTypeAliasRule())
        HasConcreteTypeAliasRule = 1;

      break;
    }

    case RewriteStep::LeftConcreteProjection:
      ++ProjectionCount;
      break;

    case RewriteStep::Decompose:
      ++DecomposeCount;
      break;

    case RewriteStep::PrefixSubstitutions:
    case RewriteStep::Shift:
    case RewriteStep::Relation:
    case RewriteStep::DecomposeConcrete:
    case RewriteStep::RightConcreteProjection:
      break;
    }

    evaluator.apply(step, system);
  }

  RulesInEmptyContext =
      Path.findRulesAppearingOnceInEmptyContext(Basepoint, system);
}

/// A rewrite rule is redundant if it appears exactly once in a loop
/// without context.
ArrayRef<unsigned>
RewriteLoop::findRulesAppearingOnceInEmptyContext(
    const RewriteSystem &system) const {
  const_cast<RewriteLoop *>(this)->recompute(system);
  return RulesInEmptyContext;
}

/// The number of LeftConcreteProjection steps, used by the elimination order to
/// prioritize loops that are not concrete unification projections.
unsigned RewriteLoop::getProjectionCount(
    const RewriteSystem &system) const {
  const_cast<RewriteLoop *>(this)->recompute(system);
  return ProjectionCount;
}

/// The number of Decompose steps, used by the elimination order to prioritize
/// loops that are not concrete simplifications.
unsigned RewriteLoop::getDecomposeCount(
    const RewriteSystem &system) const {
  const_cast<RewriteLoop *>(this)->recompute(system);
  return DecomposeCount;
}

/// Returns true if the loop contains at least one concrete protocol typealias rule.
/// See Rule::isDerivedFromConcreteProtocolTypeAliasRule().
bool RewriteLoop::hasConcreteTypeAliasRule(
    const RewriteSystem &system) const {
  const_cast<RewriteLoop *>(this)->recompute(system);
  return HasConcreteTypeAliasRule;
}

/// Returns true if the loop contains any rules in empty context.
bool RewriteLoop::isUseful(
    const RewriteSystem &system) const {
  const_cast<RewriteLoop *>(this)->recompute(system);
  return Useful;
}

void RewriteLoop::dump(toolchain::raw_ostream &out,
                       const RewriteSystem &system) const {
  out << Basepoint << ": ";
  Path.dump(out, Basepoint, system);
  if (isDeleted())
    out << " [deleted]";
}

void RewritePathEvaluator::dump(toolchain::raw_ostream &out) const {
  for (unsigned i = 0, e = Primary.size(); i < e; ++i) {
    if (i == Primary.size() - 1)
      out << "-> ";
    else
      out << "   ";

    out << Primary[i] << "\n";
  }

  for (unsigned i = 0, e = Secondary.size(); i < e; ++i) {
    out << "   " << Secondary[Secondary.size() - i - 1] << "\n";
  }
}

void RewritePathEvaluator::checkPrimary() const {
  if (Primary.empty()) {
    ABORT([&](auto &out) {
      out << "Empty primary stack\n";
      dump(out);
    });
  }
}

void RewritePathEvaluator::checkSecondary() const {
  if (Secondary.empty()) {
    ABORT([&](auto &out) {
      out << "Empty secondary stack\n";
      dump(out);
    });
  }
}

MutableTerm &RewritePathEvaluator::getCurrentTerm() {
  checkPrimary();
  return Primary.back();
}

AppliedRewriteStep
RewritePathEvaluator::applyRewriteRule(const RewriteStep &step,
                                       const RewriteSystem &system) {
  auto &term = getCurrentTerm();

  ASSERT(step.Kind == RewriteStep::Rule);

  const auto &rule = system.getRule(step.getRuleID());

  auto lhs = (step.Inverse ? rule.getRHS() : rule.getLHS());
  auto rhs = (step.Inverse ? rule.getLHS() : rule.getRHS());

  auto bug = [&](StringRef msg) {
    ABORT([&](auto &out) {
      out << msg << "\n";
      out << "- Term: " << term << "\n";
      out << "- StartOffset: " << step.StartOffset << "\n";
      out << "- EndOffset: " << step.EndOffset << "\n";
      out << "- Expected subterm: " << lhs;
    });
  };

  if (term.size() != step.StartOffset + lhs.size() + step.EndOffset) {
    bug("Invalid whiskering");
  }

  if (!std::equal(term.begin() + step.StartOffset,
                  term.begin() + step.StartOffset + lhs.size(),
                  lhs.begin())) {
    bug("Invalid subterm");
  }

  MutableTerm prefix(term.begin(), term.begin() + step.StartOffset);
  MutableTerm suffix(term.end() - step.EndOffset, term.end());

  term = prefix;
  term.append(rhs);
  term.append(suffix);

  return {lhs, rhs, prefix, suffix};
}

std::pair<MutableTerm, MutableTerm>
RewritePathEvaluator::applyPrefixSubstitutions(const RewriteStep &step,
                                               const RewriteSystem &system) {
  ASSERT(step.Arg != 0);

  auto &term = getCurrentTerm();

  ASSERT(step.Kind == RewriteStep::PrefixSubstitutions);

  auto &ctx = system.getRewriteContext();
  MutableTerm prefix(term.begin() + step.StartOffset,
                     term.begin() + step.StartOffset + step.Arg);
  MutableTerm suffix(term.end() - step.EndOffset - 1, term.end());

  // We're either adding or removing the prefix to each concrete substitution.
  Symbol &last = *(term.end() - step.EndOffset - 1);
  if (!last.hasSubstitutions()) {
    ABORT([&](auto &out) {
      out << "Invalid rewrite path\n";
      out << "- Term: " << term << "\n";
      out << "- Start offset: " << step.StartOffset << "\n";
      out << "- End offset: " << step.EndOffset;
    });
  }

  last = last.transformConcreteSubstitutions(
    [&](Term t) -> Term {
      if (step.Inverse) {
        if (!std::equal(t.begin(),
                        t.begin() + step.Arg,
                        prefix.begin())) {
          ABORT([&](auto &out) {
            out << "Invalid rewrite path\n";
            out << "- Term: " << term << "\n";
            out << "- Substitution: " << t << "\n";
            out << "- Start offset: " << step.StartOffset << "\n";
            out << "- End offset: " << step.EndOffset << "\n";
            out << "- Expected subterm: " << prefix;
          });
        }

        MutableTerm mutTerm(t.begin() + step.Arg, t.end());
        return Term::get(mutTerm, ctx);
      } else {
        MutableTerm mutTerm(prefix);
        mutTerm.append(t);
        return Term::get(mutTerm, ctx);
      }
    }, ctx);

  return std::make_pair(prefix, suffix);
}

void RewritePathEvaluator::applyShift(const RewriteStep &step,
                                      const RewriteSystem &system) {
  ASSERT(step.Kind == RewriteStep::Shift);
  ASSERT(step.StartOffset == 0);
  ASSERT(step.EndOffset == 0);
  ASSERT(step.Arg == 0);

  if (!step.Inverse) {
    // Move top of primary stack to secondary stack.
    checkPrimary();
    Secondary.push_back(Primary.back());
    Primary.pop_back();
  } else {
    // Move top of secondary stack to primary stack.
    checkSecondary();
    Primary.push_back(Secondary.back());
    Secondary.pop_back();
  }
}

void RewritePathEvaluator::applyDecompose(const RewriteStep &step,
                                          const RewriteSystem &system) {
  ASSERT(step.Kind == RewriteStep::Decompose);

  unsigned numSubstitutions = step.Arg;

  if (!step.Inverse) {
    // The input term takes the form U.[concrete: C].V or U.[superclass: C].V,
    // where |V| == EndOffset.
    const auto &term = getCurrentTerm();
    auto symbol = *(term.end() - step.EndOffset - 1);
    if (!symbol.hasSubstitutions()) {
      ABORT([&](auto &out) {
        out << "Expected term with superclass or concrete type symbol"
            << " on primary stack\n";
        dump(out);
      });
    }

    // The symbol must have the expected number of substitutions.
    if (symbol.getSubstitutions().size() != numSubstitutions) {
      ABORT([&](auto &out) {
        out << "Expected " << numSubstitutions << " substitutions\n";
        dump(out);
      });
    }

    // Push each substitution on the primary stack.
    for (auto substitution : symbol.getSubstitutions()) {
      Primary.push_back(MutableTerm(substitution));
    }
  } else {
    // The primary stack must store the number of substitutions, together with
    // a term that takes the form U.[concrete: C].V or U.[superclass: C].V,
    // where |V| == EndOffset.
    if (Primary.size() < numSubstitutions + 1) {
      ABORT([&](auto &out) {
        out << "Not enough terms on primary stack\n";
        dump(out);
      });
    }

    // The term immediately underneath the substitutions is the one we're
    // updating with new substitutions.
    const auto &term = *(Primary.end() - numSubstitutions - 1);
    auto symbol = *(term.end() - step.EndOffset - 1);

    // The symbol at the end of this term must have the expected number of
    // substitutions.
    if (symbol.getSubstitutions().size() != numSubstitutions) {
      ABORT([&](auto &out) {
        out << "Expected " << numSubstitutions << " substitutions\n";
        dump(out);
      });
    }

    for (unsigned i = 0; i < numSubstitutions; ++i) {
      const auto &substitution = *(Primary.end() - numSubstitutions + i);
      if (MutableTerm(symbol.getSubstitutions()[i]) != substitution) {
        ABORT([&](auto &out) {
          out << "Expected " << symbol.getSubstitutions()[i] << "\n";
          out << "Got " << substitution << "\n";
          dump(out);
        });
      }
    }

    // Pop the substitutions from the primary stack.
    Primary.resize(Primary.size() - numSubstitutions);
  }
}

AppliedRewriteStep
RewritePathEvaluator::applyRelation(const RewriteStep &step,
                                    const RewriteSystem &system) {
  ASSERT(step.Kind == RewriteStep::Relation);

  auto relation = system.getRelation(step.Arg);
  auto &term = getCurrentTerm();

  auto lhs = (step.Inverse ? relation.second : relation.first);
  auto rhs = (step.Inverse ? relation.first : relation.second);

  auto bug = [&](StringRef msg) {
    ABORT([&](auto &out) {
      out << msg << "\n";
      out << "- Term: " << term << "\n";
      out << "- StartOffset: " << step.StartOffset << "\n";
      out << "- EndOffset: " << step.EndOffset << "\n";
      out << "- Expected subterm: " << lhs << "\n";
    });
  };

  if (term.size() != step.StartOffset + lhs.size() + step.EndOffset) {
    bug("Invalid whiskering");
  }

  if (!std::equal(term.begin() + step.StartOffset,
                  term.begin() + step.StartOffset + lhs.size(),
                  lhs.begin())) {
    bug("Invalid subterm");
  }

  MutableTerm prefix(term.begin(), term.begin() + step.StartOffset);
  MutableTerm suffix(term.end() - step.EndOffset, term.end());

  term = prefix;
  term.append(rhs);
  term.append(suffix);

  return {lhs, rhs, prefix, suffix};
}

void RewritePathEvaluator::applyDecomposeConcrete(const RewriteStep &step,
                                                  const RewriteSystem &system) {
  ASSERT(step.Kind == RewriteStep::DecomposeConcrete);

  const auto &difference = system.getTypeDifference(step.Arg);
  auto bug = [&](StringRef msg) {
    ABORT([&](auto &out) {
      out << msg << "\n";
      out << "- StartOffset: " << step.StartOffset << "\n";
      out << "- EndOffset: " << step.EndOffset << "\n";
      out << "- DifferenceID: " << step.Arg << "\n";
      out << "\nType difference:\n";
      difference.dump(out);
      out << "\nEvaluator state:\n";
      dump(out);
    });
  };

  auto substitutions = difference.LHS.getSubstitutions();

  if (!step.Inverse) {
    auto &term = getCurrentTerm();

    auto concreteSymbol = *(term.end() - step.EndOffset - 1);
    if (concreteSymbol != difference.RHS)
      bug("Concrete symbol not equal to expected RHS");

    MutableTerm newTerm(term.begin(), term.end() - step.EndOffset - 1);
    newTerm.add(difference.LHS);
    newTerm.append(term.end() - step.EndOffset, term.end());
    term = newTerm;

    for (unsigned n : indices(substitutions))
      Primary.push_back(difference.getReplacementSubstitution(n));

  } else {
    unsigned numSubstitutions = substitutions.size();

    if (Primary.size() < numSubstitutions + 1)
      bug("Not enough terms on the stack");

    for (unsigned n : indices(substitutions)) {
      const auto &otherSubstitution = *(Primary.end() - numSubstitutions + n);
      auto expectedSubstitution = difference.getReplacementSubstitution(n);
      if (otherSubstitution != expectedSubstitution) {
        SmallString<0> message;
        toolchain::raw_svector_ostream out(message);
        out << "Unexpected substitution term on the stack\n";
        out << "Got: " << otherSubstitution << "\n";
        out << "Expected: " << expectedSubstitution << "\n";
        bug(message);
      }
    }

    Primary.resize(Primary.size() - numSubstitutions);

    auto &term = getCurrentTerm();

    auto concreteSymbol = *(term.end() - step.EndOffset - 1);
    if (concreteSymbol != difference.LHS)
      bug("Concrete symbol not equal to expected LHS");

    MutableTerm newTerm(term.begin(), term.end() - step.EndOffset - 1);
    newTerm.add(difference.RHS);
    newTerm.append(term.end() - step.EndOffset, term.end());
    term = newTerm;
  }
}

void
RewritePathEvaluator::applyLeftConcreteProjection(const RewriteStep &step,
                                                  const RewriteSystem &system) {
  ASSERT(step.Kind == RewriteStep::LeftConcreteProjection);

  const auto &difference = system.getTypeDifference(step.getTypeDifferenceID());
  unsigned index = step.getSubstitutionIndex();

  auto leftProjection = difference.getOriginalSubstitution(index);

  MutableTerm leftBaseTerm(difference.BaseTerm);
  leftBaseTerm.add(difference.LHS);

  auto bug = [&](StringRef msg) {
    ABORT([&](auto &out) {
      out << msg << "\n";
      out << "- StartOffset: " << step.StartOffset << "\n";
      out << "- EndOffset: " << step.EndOffset << "\n";
      out << "- SubstitutionIndex: " << index << "\n";
      out << "- LeftProjection: " << leftProjection << "\n";
      out << "- LeftBaseTerm: " << leftBaseTerm << "\n";
      out << "- DifferenceID: " << step.getTypeDifferenceID() << "\n";
      out << "\nType difference:\n";
      difference.dump(out);
      out << ":\n";
      difference.dump(out);
      out << "\nEvaluator state:\n";
      dump(out);
    });
  };

  if (!step.Inverse) {
    const auto &term = getCurrentTerm();

    MutableTerm subTerm(term.begin() + step.StartOffset,
                        term.end() - step.EndOffset);
    if (subTerm != MutableTerm(leftProjection))
      bug("Incorrect left projection term");

    Primary.push_back(leftBaseTerm);
  } else {
    if (Primary.size() < 2)
      bug("Too few elements on the primary stack");

    if (Primary.back() != leftBaseTerm)
      bug("Incorrect left base term");

    Primary.pop_back();

    const auto &term = getCurrentTerm();

    MutableTerm subTerm(term.begin() + step.StartOffset,
                        term.end() - step.EndOffset);
    if (subTerm != leftProjection)
      bug("Incorrect left projection term");
  }
}

void
RewritePathEvaluator::applyRightConcreteProjection(const RewriteStep &step,
                                                   const RewriteSystem &system) {
  ASSERT(step.Kind == RewriteStep::RightConcreteProjection);

  const auto &difference = system.getTypeDifference(step.getTypeDifferenceID());
  unsigned index = step.getSubstitutionIndex();

  auto leftProjection = difference.getOriginalSubstitution(index);
  auto rightProjection = difference.getReplacementSubstitution(index);

  MutableTerm leftBaseTerm(difference.BaseTerm);
  leftBaseTerm.add(difference.LHS);

  MutableTerm rightBaseTerm(difference.BaseTerm);
  rightBaseTerm.add(difference.RHS);

  auto bug = [&](StringRef msg) {
    ABORT([&](auto &out) {
      out << msg << "\n";
      out << "- StartOffset: " << step.StartOffset << "\n";
      out << "- EndOffset: " << step.EndOffset << "\n";
      out << "- SubstitutionIndex: " << index << "\n";
      out << "- LeftProjection: " << leftProjection << "\n";
      out << "- RightProjection: " << rightProjection << "\n";
      out << "- LeftBaseTerm: " << leftBaseTerm << "\n";
      out << "- RightBaseTerm: " << rightBaseTerm << "\n";
      out << "- DifferenceID: " << step.getTypeDifferenceID() << "\n";
      out << "\nType difference:\n";
      difference.dump(out);
      out << "\nEvaluator state:\n";
      dump(out);
    });
  };

  if (!step.Inverse) {
    auto &term = getCurrentTerm();

    MutableTerm subTerm(term.begin() + step.StartOffset,
                        term.end() - step.EndOffset);

    if (subTerm != rightProjection)
      bug("Incorrect right projection term");

    MutableTerm newTerm(term.begin(), term.begin() + step.StartOffset);
    newTerm.append(leftProjection);
    newTerm.append(term.end() - step.EndOffset, term.end());

    term = newTerm;

    Primary.push_back(rightBaseTerm);
  } else {
    if (Primary.size() < 2)
      bug("Too few elements on the primary stack");

    if (Primary.back() != rightBaseTerm)
      bug("Incorrect right base term");

    Primary.pop_back();

    auto &term = getCurrentTerm();

    MutableTerm subTerm(term.begin() + step.StartOffset,
                        term.end() - step.EndOffset);
    if (subTerm != leftProjection)
      bug("Incorrect left projection term");

    MutableTerm newTerm(term.begin(), term.begin() + step.StartOffset);
    newTerm.append(rightProjection);
    newTerm.append(term.end() - step.EndOffset, term.end());

    term = newTerm;
  }
}

void RewritePathEvaluator::apply(const RewriteStep &step,
                                 const RewriteSystem &system) {
  switch (step.Kind) {
  case RewriteStep::Rule:
    (void) applyRewriteRule(step, system);
    break;

  case RewriteStep::PrefixSubstitutions:
    (void) applyPrefixSubstitutions(step, system);
    break;

  case RewriteStep::Shift:
    applyShift(step, system);
    break;

  case RewriteStep::Decompose:
    applyDecompose(step, system);
    break;

  case RewriteStep::Relation:
    applyRelation(step, system);
    break;

  case RewriteStep::DecomposeConcrete:
    applyDecomposeConcrete(step, system);
    break;

  case RewriteStep::LeftConcreteProjection:
    applyLeftConcreteProjection(step, system);
    break;

  case RewriteStep::RightConcreteProjection:
    applyRightConcreteProjection(step, system);
    break;
  }
}
