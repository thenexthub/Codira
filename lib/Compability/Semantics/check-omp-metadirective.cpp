//===-- lib/Semantics/check-omp-metadirective.cpp -------------------------===//
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
// Semantic checks for METADIRECTIVE and related constructs/clauses.
//
//===----------------------------------------------------------------------===//

#include "check-omp-structure.h"

#include "language/Compability/Common/idioms.h"
#include "language/Compability/Common/indirection.h"
#include "language/Compability/Common/visit.h"
#include "language/Compability/Parser/characters.h"
#include "language/Compability/Parser/message.h"
#include "language/Compability/Parser/parse-tree.h"
#include "language/Compability/Semantics/openmp-modifiers.h"
#include "language/Compability/Semantics/openmp-utils.h"
#include "language/Compability/Semantics/tools.h"

#include "toolchain/Frontend/OpenMP/OMP.h"

#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <variant>

namespace language::Compability::semantics {

using namespace language::Compability::semantics::omp;

void OmpStructureChecker::Enter(const parser::OmpClause::When &x) {
  CheckAllowedClause(toolchain::omp::Clause::OMPC_when);
  OmpVerifyModifiers(
      x.v, toolchain::omp::OMPC_when, GetContext().clauseSource, context_);
}

void OmpStructureChecker::Enter(const parser::OmpContextSelector &ctx) {
  EnterDirectiveNest(ContextSelectorNest);

  using SetName = parser::OmpTraitSetSelectorName;
  std::map<SetName::Value, const SetName *> visited;

  for (const parser::OmpTraitSetSelector &traitSet : ctx.v) {
    auto &name{std::get<SetName>(traitSet.t)};
    auto [prev, unique]{visited.insert(std::make_pair(name.v, &name))};
    if (!unique) {
      std::string showName{parser::ToUpperCaseLetters(name.ToString())};
      parser::MessageFormattedText txt(
          "Repeated trait set name %s in a context specifier"_err_en_US,
          showName);
      parser::Message message(name.source, txt);
      message.Attach(prev->second->source,
          "Previous trait set %s provided here"_en_US, showName);
      context_.Say(std::move(message));
    }
    CheckTraitSetSelector(traitSet);
  }
}

void OmpStructureChecker::Leave(const parser::OmpContextSelector &) {
  ExitDirectiveNest(ContextSelectorNest);
}

const std::list<parser::OmpTraitProperty> &
OmpStructureChecker::GetTraitPropertyList(
    const parser::OmpTraitSelector &trait) {
  static const std::list<parser::OmpTraitProperty> empty{};
  auto &[_, maybeProps]{trait.t};
  if (maybeProps) {
    using PropertyList = std::list<parser::OmpTraitProperty>;
    return std::get<PropertyList>(maybeProps->t);
  } else {
    return empty;
  }
}

std::optional<toolchain::omp::Clause> OmpStructureChecker::GetClauseFromProperty(
    const parser::OmpTraitProperty &property) {
  using MaybeClause = std::optional<toolchain::omp::Clause>;

  // The parser for OmpClause will only succeed if the clause was
  // given with all required arguments.
  // If this is a string or complex extension with a clause name,
  // treat it as a clause and let the trait checker deal with it.

  auto getClauseFromString{[&](const std::string &s) -> MaybeClause {
    auto id{toolchain::omp::getOpenMPClauseKind(parser::ToLowerCaseLetters(s))};
    if (id != toolchain::omp::Clause::OMPC_unknown) {
      return id;
    } else {
      return std::nullopt;
    }
  }};

  return common::visit( //
      common::visitors{
          [&](const parser::OmpTraitPropertyName &x) -> MaybeClause {
            return getClauseFromString(x.v);
          },
          [&](const common::Indirection<parser::OmpClause> &x) -> MaybeClause {
            return x.value().Id();
          },
          [&](const parser::ScalarExpr &x) -> MaybeClause {
            return std::nullopt;
          },
          [&](const parser::OmpTraitPropertyExtension &x) -> MaybeClause {
            using ExtProperty = parser::OmpTraitPropertyExtension;
            if (auto *name{std::get_if<parser::OmpTraitPropertyName>(&x.u)}) {
              return getClauseFromString(name->v);
            } else if (auto *cpx{std::get_if<ExtProperty::Complex>(&x.u)}) {
              return getClauseFromString(
                  std::get<parser::OmpTraitPropertyName>(cpx->t).v);
            }
            return std::nullopt;
          },
      },
      property.u);
}

void OmpStructureChecker::CheckTraitSelectorList(
    const std::list<parser::OmpTraitSelector> &traits) {
  // [6.0:322:20]
  // Each trait-selector-name may only be specified once in a trait selector
  // set.

  // Cannot store OmpTraitSelectorName directly, because it's not copyable.
  using TraitName = parser::OmpTraitSelectorName;
  using BareName = decltype(TraitName::u);
  std::map<BareName, const TraitName *> visited;

  for (const parser::OmpTraitSelector &trait : traits) {
    auto &name{std::get<TraitName>(trait.t)};

    auto [prev, unique]{visited.insert(std::make_pair(name.u, &name))};
    if (!unique) {
      std::string showName{parser::ToUpperCaseLetters(name.ToString())};
      parser::MessageFormattedText txt(
          "Repeated trait name %s in a trait set"_err_en_US, showName);
      parser::Message message(name.source, txt);
      message.Attach(prev->second->source,
          "Previous trait %s provided here"_en_US, showName);
      context_.Say(std::move(message));
    }
  }
}

void OmpStructureChecker::CheckTraitSetSelector(
    const parser::OmpTraitSetSelector &traitSet) {

  // Trait Set      |           Allowed traits | D-traits | X-traits | Score |
  //
  // Construct      |     Simd, directive-name |      Yes |       No |    No |
  // Device         |          Arch, Isa, Kind |       No |      Yes |    No |
  // Implementation | Atomic_Default_Mem_Order |       No |      Yes |   Yes |
  //                |      Extension, Requires |          |          |       |
  //                |                   Vendor |          |          |       |
  // Target_Device  |    Arch, Device_Num, Isa |       No |      Yes |    No |
  //                |                Kind, Uid |          |          |       |
  // User           |                Condition |       No |       No |   Yes |

  struct TraitSetConfig {
    std::set<parser::OmpTraitSelectorName::Value> allowed;
    bool allowsDirectiveTraits;
    bool allowsExtensionTraits;
    bool allowsScore;
  };

  using SName = parser::OmpTraitSetSelectorName::Value;
  using TName = parser::OmpTraitSelectorName::Value;

  static const std::map<SName, TraitSetConfig> configs{
      {SName::Construct, //
          {{TName::Simd}, true, false, false}},
      {SName::Device, //
          {{TName::Arch, TName::Isa, TName::Kind}, false, true, false}},
      {SName::Implementation, //
          {{TName::Atomic_Default_Mem_Order, TName::Extension, TName::Requires,
               TName::Vendor},
              false, true, true}},
      {SName::Target_Device, //
          {{TName::Arch, TName::Device_Num, TName::Isa, TName::Kind,
               TName::Uid},
              false, true, false}},
      {SName::User, //
          {{TName::Condition}, false, false, true}},
  };

  auto checkTraitSet{[&](const TraitSetConfig &config) {
    auto &[setName, traits]{traitSet.t};
    auto usn{parser::ToUpperCaseLetters(setName.ToString())};

    // Check if there are any duplicate traits.
    CheckTraitSelectorList(traits);

    for (const parser::OmpTraitSelector &trait : traits) {
      // Don't use structured bindings here, because they cannot be captured
      // before C++20.
      auto &traitName = std::get<parser::OmpTraitSelectorName>(trait.t);
      auto &maybeProps =
          std::get<std::optional<parser::OmpTraitSelector::Properties>>(
              trait.t);

      // Check allowed traits
      common::visit( //
          common::visitors{
              [&](parser::OmpTraitSelectorName::Value v) {
                if (!config.allowed.count(v)) {
                  context_.Say(traitName.source,
                      "%s is not a valid trait for %s trait set"_err_en_US,
                      parser::ToUpperCaseLetters(traitName.ToString()), usn);
                }
              },
              [&](toolchain::omp::Directive) {
                if (!config.allowsDirectiveTraits) {
                  context_.Say(traitName.source,
                      "Directive name is not a valid trait for %s trait set"_err_en_US,
                      usn);
                }
              },
              [&](const std::string &) {
                if (!config.allowsExtensionTraits) {
                  context_.Say(traitName.source,
                      "Extension traits are not valid for %s trait set"_err_en_US,
                      usn);
                }
              },
          },
          traitName.u);

      // Check score
      if (maybeProps) {
        auto &[maybeScore, _]{maybeProps->t};
        if (maybeScore) {
          CheckTraitScore(*maybeScore);
        }
      }

      // Check the properties of the individual traits
      CheckTraitSelector(traitSet, trait);
    }
  }};

  checkTraitSet(
      configs.at(std::get<parser::OmpTraitSetSelectorName>(traitSet.t).v));
}

void OmpStructureChecker::CheckTraitScore(const parser::OmpTraitScore &score) {
  // [6.0:322:23]
  // A score-expression must be a non-negative constant integer expression.
  if (auto value{GetIntValue(score)}; !value || value < 0) {
    context_.Say(score.source,
        "SCORE expression must be a non-negative constant integer expression"_err_en_US);
  }
}

bool OmpStructureChecker::VerifyTraitPropertyLists(
    const parser::OmpTraitSetSelector &traitSet,
    const parser::OmpTraitSelector &trait) {
  using TraitName = parser::OmpTraitSelectorName;
  using PropertyList = std::list<parser::OmpTraitProperty>;
  auto &[traitName, maybeProps]{trait.t};

  auto checkPropertyList{[&](const PropertyList &properties, auto isValid,
                             const std::string &message) {
    bool foundInvalid{false};
    for (const parser::OmpTraitProperty &prop : properties) {
      if (!isValid(prop)) {
        if (foundInvalid) {
          context_.Say(
              prop.source, "More invalid properties are present"_err_en_US);
          break;
        }
        context_.Say(prop.source, "%s"_err_en_US, message);
        foundInvalid = true;
      }
    }
    return !foundInvalid;
  }};

  bool invalid{false};

  if (std::holds_alternative<toolchain::omp::Directive>(traitName.u)) {
    // Directive-name traits don't have properties.
    if (maybeProps) {
      context_.Say(trait.source,
          "Directive-name traits cannot have properties"_err_en_US);
      invalid = true;
    }
  }
  // Ignore properties on extension traits.

  // See `TraitSelectorParser` in openmp-parser.cpp
  if (auto *v{std::get_if<TraitName::Value>(&traitName.u)}) {
    switch (*v) {
    // name-list properties
    case parser::OmpTraitSelectorName::Value::Arch:
    case parser::OmpTraitSelectorName::Value::Extension:
    case parser::OmpTraitSelectorName::Value::Isa:
    case parser::OmpTraitSelectorName::Value::Kind:
    case parser::OmpTraitSelectorName::Value::Uid:
    case parser::OmpTraitSelectorName::Value::Vendor:
      if (maybeProps) {
        auto isName{[](const parser::OmpTraitProperty &prop) {
          return std::holds_alternative<parser::OmpTraitPropertyName>(prop.u);
        }};
        invalid = !checkPropertyList(std::get<PropertyList>(maybeProps->t),
            isName, "Trait property should be a name");
      }
      break;
    // clause-list
    case parser::OmpTraitSelectorName::Value::Atomic_Default_Mem_Order:
    case parser::OmpTraitSelectorName::Value::Requires:
    case parser::OmpTraitSelectorName::Value::Simd:
      if (maybeProps) {
        auto isClause{[&](const parser::OmpTraitProperty &prop) {
          return GetClauseFromProperty(prop).has_value();
        }};
        invalid = !checkPropertyList(std::get<PropertyList>(maybeProps->t),
            isClause, "Trait property should be a clause");
      }
      break;
    // expr-list
    case parser::OmpTraitSelectorName::Value::Condition:
    case parser::OmpTraitSelectorName::Value::Device_Num:
      if (maybeProps) {
        auto isExpr{[](const parser::OmpTraitProperty &prop) {
          return std::holds_alternative<parser::ScalarExpr>(prop.u);
        }};
        invalid = !checkPropertyList(std::get<PropertyList>(maybeProps->t),
            isExpr, "Trait property should be a scalar expression");
      }
      break;
    } // switch
  }

  return !invalid;
}

void OmpStructureChecker::CheckTraitSelector(
    const parser::OmpTraitSetSelector &traitSet,
    const parser::OmpTraitSelector &trait) {
  using TraitName = parser::OmpTraitSelectorName;
  auto &[traitName, maybeProps]{trait.t};

  // Only do the detailed checks if the property lists are valid.
  if (VerifyTraitPropertyLists(traitSet, trait)) {
    if (std::holds_alternative<toolchain::omp::Directive>(traitName.u) ||
        std::holds_alternative<std::string>(traitName.u)) {
      // No properties here: directives don't have properties, and
      // we don't implement any extension traits now.
      return;
    }

    // Specific traits we want to check.
    // Limitations:
    // (1) The properties for these traits are defined in "Additional
    // Definitions for the OpenMP API Specification". It's not clear how
    // to define them in a portable way, and how to verify their validity,
    // especially if they get replaced by their integer values (in case
    // they are defined as enums).
    // (2) These are entirely implementation-defined, and at the moment
    // there is no known schema to validate these values.
    auto v{std::get<TraitName::Value>(traitName.u)};
    switch (v) {
    case TraitName::Value::Arch:
      // Unchecked, TBD(1)
      break;
    case TraitName::Value::Atomic_Default_Mem_Order:
      CheckTraitADMO(traitSet, trait);
      break;
    case TraitName::Value::Condition:
      CheckTraitCondition(traitSet, trait);
      break;
    case TraitName::Value::Device_Num:
      CheckTraitDeviceNum(traitSet, trait);
      break;
    case TraitName::Value::Extension:
      // Ignore
      break;
    case TraitName::Value::Isa:
      // Unchecked, TBD(1)
      break;
    case TraitName::Value::Kind:
      // Unchecked, TBD(1)
      break;
    case TraitName::Value::Requires:
      CheckTraitRequires(traitSet, trait);
      break;
    case TraitName::Value::Simd:
      CheckTraitSimd(traitSet, trait);
      break;
    case TraitName::Value::Uid:
      // Unchecked, TBD(2)
      break;
    case TraitName::Value::Vendor:
      // Unchecked, TBD(1)
      break;
    }
  }
}

void OmpStructureChecker::CheckTraitADMO(
    const parser::OmpTraitSetSelector &traitSet,
    const parser::OmpTraitSelector &trait) {
  auto &traitName{std::get<parser::OmpTraitSelectorName>(trait.t)};
  auto &properties{GetTraitPropertyList(trait)};

  if (properties.size() != 1) {
    context_.Say(trait.source,
        "%s trait requires a single clause property"_err_en_US,
        parser::ToUpperCaseLetters(traitName.ToString()));
  } else {
    const parser::OmpTraitProperty &property{properties.front()};
    auto clauseId{*GetClauseFromProperty(property)};
    // Check that the clause belongs to the memory-order clause-set.
    // Clause sets will hopefully be autogenerated at some point.
    switch (clauseId) {
    case toolchain::omp::Clause::OMPC_acq_rel:
    case toolchain::omp::Clause::OMPC_acquire:
    case toolchain::omp::Clause::OMPC_relaxed:
    case toolchain::omp::Clause::OMPC_release:
    case toolchain::omp::Clause::OMPC_seq_cst:
      break;
    default:
      context_.Say(property.source,
          "%s trait requires a clause from the memory-order clause set"_err_en_US,
          parser::ToUpperCaseLetters(traitName.ToString()));
    }

    using ClauseProperty = common::Indirection<parser::OmpClause>;
    if (!std::holds_alternative<ClauseProperty>(property.u)) {
      context_.Say(property.source,
          "Invalid clause specification for %s"_err_en_US,
          parser::ToUpperCaseLetters(getClauseName(clauseId)));
    }
  }
}

void OmpStructureChecker::CheckTraitCondition(
    const parser::OmpTraitSetSelector &traitSet,
    const parser::OmpTraitSelector &trait) {
  auto &traitName{std::get<parser::OmpTraitSelectorName>(trait.t)};
  auto &properties{GetTraitPropertyList(trait)};

  if (properties.size() != 1) {
    context_.Say(trait.source,
        "%s trait requires a single expression property"_err_en_US,
        parser::ToUpperCaseLetters(traitName.ToString()));
  } else {
    const parser::OmpTraitProperty &property{properties.front()};
    auto &scalarExpr{std::get<parser::ScalarExpr>(property.u)};

    auto maybeType{GetDynamicType(scalarExpr.thing.value())};
    if (!maybeType || maybeType->category() != TypeCategory::Logical) {
      context_.Say(property.source,
          "%s trait requires a single LOGICAL expression"_err_en_US,
          parser::ToUpperCaseLetters(traitName.ToString()));
    }
  }
}

void OmpStructureChecker::CheckTraitDeviceNum(
    const parser::OmpTraitSetSelector &traitSet,
    const parser::OmpTraitSelector &trait) {
  auto &traitName{std::get<parser::OmpTraitSelectorName>(trait.t)};
  auto &properties{GetTraitPropertyList(trait)};

  if (properties.size() != 1) {
    context_.Say(trait.source,
        "%s trait requires a single expression property"_err_en_US,
        parser::ToUpperCaseLetters(traitName.ToString()));
  }
  // No other checks at the moment.
}

void OmpStructureChecker::CheckTraitRequires(
    const parser::OmpTraitSetSelector &traitSet,
    const parser::OmpTraitSelector &trait) {
  unsigned version{context_.langOptions().OpenMPVersion};
  auto &traitName{std::get<parser::OmpTraitSelectorName>(trait.t)};
  auto &properties{GetTraitPropertyList(trait)};

  for (const parser::OmpTraitProperty &property : properties) {
    auto clauseId{*GetClauseFromProperty(property)};
    if (!toolchain::omp::isAllowedClauseForDirective(
            toolchain::omp::OMPD_requires, clauseId, version)) {
      context_.Say(property.source,
          "%s trait requires a clause from the requirement clause set"_err_en_US,
          parser::ToUpperCaseLetters(traitName.ToString()));
    }

    using ClauseProperty = common::Indirection<parser::OmpClause>;
    if (!std::holds_alternative<ClauseProperty>(property.u)) {
      context_.Say(property.source,
          "Invalid clause specification for %s"_err_en_US,
          parser::ToUpperCaseLetters(getClauseName(clauseId)));
    }
  }
}

void OmpStructureChecker::CheckTraitSimd(
    const parser::OmpTraitSetSelector &traitSet,
    const parser::OmpTraitSelector &trait) {
  unsigned version{context_.langOptions().OpenMPVersion};
  auto &traitName{std::get<parser::OmpTraitSelectorName>(trait.t)};
  auto &properties{GetTraitPropertyList(trait)};

  for (const parser::OmpTraitProperty &property : properties) {
    auto clauseId{*GetClauseFromProperty(property)};
    if (!toolchain::omp::isAllowedClauseForDirective(
            toolchain::omp::OMPD_declare_simd, clauseId, version)) {
      context_.Say(property.source,
          "%s trait requires a clause that is allowed on the %s directive"_err_en_US,
          parser::ToUpperCaseLetters(traitName.ToString()),
          parser::ToUpperCaseLetters(
              getDirectiveName(toolchain::omp::OMPD_declare_simd)));
    }

    using ClauseProperty = common::Indirection<parser::OmpClause>;
    if (!std::holds_alternative<ClauseProperty>(property.u)) {
      context_.Say(property.source,
          "Invalid clause specification for %s"_err_en_US,
          parser::ToUpperCaseLetters(getClauseName(clauseId)));
    }
  }
}

void OmpStructureChecker::Enter(const parser::OmpMetadirectiveDirective &x) {
  EnterDirectiveNest(MetadirectiveNest);
  PushContextAndClauseSets(x.source, toolchain::omp::Directive::OMPD_metadirective);
}

void OmpStructureChecker::Leave(const parser::OmpMetadirectiveDirective &) {
  ExitDirectiveNest(MetadirectiveNest);
  dirContext_.pop_back();
}

} // namespace language::Compability::semantics
