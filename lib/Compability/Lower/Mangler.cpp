//===-- Mangler.cpp -------------------------------------------------------===//
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

#include "language/Compability/Lower/Mangler.h"
#include "language/Compability/Common/reference.h"
#include "language/Compability/Lower/Support/Utils.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Semantics/tools.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/MD5.h"

/// Return all ancestor module and submodule scope names; all host procedure
/// and statement function scope names; and the innermost blockId containing
/// \p scope, including scope itself.
static std::tuple<toolchain::SmallVector<toolchain::StringRef>,
                  toolchain::SmallVector<toolchain::StringRef>, std::int64_t>
ancestors(const language::Compability::semantics::Scope &scope,
          language::Compability::lower::mangle::ScopeBlockIdMap &scopeBlockIdMap) {
  toolchain::SmallVector<const language::Compability::semantics::Scope *> scopes;
  for (auto *scp = &scope; !scp->IsGlobal(); scp = &scp->parent())
    scopes.push_back(scp);
  toolchain::SmallVector<toolchain::StringRef> modules;
  toolchain::SmallVector<toolchain::StringRef> procs;
  std::int64_t blockId = 0;
  for (auto iter = scopes.rbegin(), rend = scopes.rend(); iter != rend;
       ++iter) {
    auto *scp = *iter;
    switch (scp->kind()) {
    case language::Compability::semantics::Scope::Kind::Module:
      modules.emplace_back(toStringRef(scp->symbol()->name()));
      break;
    case language::Compability::semantics::Scope::Kind::Subprogram:
      procs.emplace_back(toStringRef(scp->symbol()->name()));
      break;
    case language::Compability::semantics::Scope::Kind::MainProgram:
      // Do not use the main program name, if any, because it may collide
      // with a procedure of the same name in another compilation unit.
      // This is nonconformant, but universally allowed.
      procs.emplace_back(toolchain::StringRef(""));
      break;
    case language::Compability::semantics::Scope::Kind::BlockConstruct: {
      auto it = scopeBlockIdMap.find(scp);
      assert(it != scopeBlockIdMap.end() && it->second &&
             "invalid block identifier");
      blockId = it->second;
    } break;
    default:
      break;
    }
  }
  return {modules, procs, blockId};
}

/// Return all ancestor module and submodule scope names; all host procedure
/// and statement function scope names; and the innermost blockId containing
/// \p symbol.
static std::tuple<toolchain::SmallVector<toolchain::StringRef>,
                  toolchain::SmallVector<toolchain::StringRef>, std::int64_t>
ancestors(const language::Compability::semantics::Symbol &symbol,
          language::Compability::lower::mangle::ScopeBlockIdMap &scopeBlockIdMap) {
  return ancestors(symbol.owner(), scopeBlockIdMap);
}

/// Return a globally unique string for a compiler generated \p name.
std::string
language::Compability::lower::mangle::mangleName(std::string &name,
                                   const language::Compability::semantics::Scope &scope,
                                   ScopeBlockIdMap &scopeBlockIdMap) {
  toolchain::SmallVector<toolchain::StringRef> modules;
  toolchain::SmallVector<toolchain::StringRef> procs;
  std::int64_t blockId;
  std::tie(modules, procs, blockId) = ancestors(scope, scopeBlockIdMap);
  return fir::NameUniquer::doGenerated(modules, procs, blockId, name);
}

// Mangle the name of \p symbol to make it globally unique.
std::string language::Compability::lower::mangle::mangleName(
    const language::Compability::semantics::Symbol &symbol, ScopeBlockIdMap &scopeBlockIdMap,
    bool keepExternalInScope, bool underscoring) {
  // Resolve module and host associations before mangling.
  const auto &ultimateSymbol = symbol.GetUltimate();

  // The Fortran and BIND(C) namespaces are counterintuitive. A BIND(C) name is
  // substituted early, and has precedence over the Fortran name. This allows
  // multiple procedures or objects with identical Fortran names to legally
  // coexist. The BIND(C) name is unique.
  if (auto *overrideName = ultimateSymbol.GetBindName())
    return *overrideName;

  toolchain::StringRef symbolName = toStringRef(ultimateSymbol.name());
  toolchain::SmallVector<toolchain::StringRef> modules;
  toolchain::SmallVector<toolchain::StringRef> procs;
  std::int64_t blockId;

  // mangle ObjectEntityDetails or AssocEntityDetails symbols.
  auto mangleObject = [&]() -> std::string {
    std::tie(modules, procs, blockId) =
        ancestors(ultimateSymbol, scopeBlockIdMap);
    if (language::Compability::semantics::IsNamedConstant(ultimateSymbol))
      return fir::NameUniquer::doConstant(modules, procs, blockId, symbolName);
    return fir::NameUniquer::doVariable(modules, procs, blockId, symbolName);
  };

  return language::Compability::common::visit(
      language::Compability::common::visitors{
          [&](const language::Compability::semantics::MainProgramDetails &) {
            return fir::NameUniquer::doProgramEntry().str();
          },
          [&](const language::Compability::semantics::SubprogramDetails &subpDetails) {
            // Mangle external procedure without any scope prefix.
            if (!keepExternalInScope &&
                language::Compability::semantics::IsExternal(ultimateSymbol))
              return fir::NameUniquer::doProcedure({}, {}, symbolName);
            // A separate module procedure must be mangled according to its
            // declaration scope, not its definition scope.
            const language::Compability::semantics::Symbol *interface = &ultimateSymbol;
            if (interface->attrs().test(language::Compability::semantics::Attr::MODULE) &&
                interface->owner().IsSubmodule() && !subpDetails.isInterface())
              interface = subpDetails.moduleInterface();
            std::tie(modules, procs, blockId) = ancestors(
                interface ? *interface : ultimateSymbol, scopeBlockIdMap);
            return fir::NameUniquer::doProcedure(modules, procs, symbolName);
          },
          [&](const language::Compability::semantics::ProcEntityDetails &) {
            // Mangle procedure pointers and dummy procedures as variables.
            if (language::Compability::semantics::IsPointer(ultimateSymbol) ||
                language::Compability::semantics::IsDummy(ultimateSymbol)) {
              std::tie(modules, procs, blockId) =
                  ancestors(ultimateSymbol, scopeBlockIdMap);
              return fir::NameUniquer::doVariable(modules, procs, blockId,
                                                  symbolName);
            }
            // Otherwise, this is an external procedure, with or without an
            // explicit EXTERNAL attribute. Mangle it without any prefix.
            return fir::NameUniquer::doProcedure({}, {}, symbolName);
          },
          [&](const language::Compability::semantics::ObjectEntityDetails &) {
            return mangleObject();
          },
          [&](const language::Compability::semantics::AssocEntityDetails &) {
            return mangleObject();
          },
          [&](const language::Compability::semantics::NamelistDetails &) {
            std::tie(modules, procs, blockId) =
                ancestors(ultimateSymbol, scopeBlockIdMap);
            return fir::NameUniquer::doNamelistGroup(modules, procs,
                                                     symbolName);
          },
          [&](const language::Compability::semantics::CommonBlockDetails &) {
            return language::Compability::semantics::GetCommonBlockObjectName(ultimateSymbol,
                                                                underscoring);
          },
          [&](const language::Compability::semantics::ProcBindingDetails &procBinding) {
            return mangleName(procBinding.symbol(), scopeBlockIdMap,
                              keepExternalInScope, underscoring);
          },
          [&](const language::Compability::semantics::GenericDetails &generic)
              -> std::string {
            if (generic.specific())
              return mangleName(*generic.specific(), scopeBlockIdMap,
                                keepExternalInScope, underscoring);
            else
              toolchain::report_fatal_error(
                  "attempt to mangle a generic name but "
                  "it has no specific procedure of the same name");
          },
          [&](const language::Compability::semantics::DerivedTypeDetails &) -> std::string {
            // Derived type mangling must use mangleName(DerivedTypeSpec) so
            // that kind type parameter values can be mangled.
            toolchain::report_fatal_error(
                "only derived type instances can be mangled");
          },
          [](const auto &) -> std::string { TODO_NOLOC("symbol mangling"); },
      },
      ultimateSymbol.details());
}

std::string
language::Compability::lower::mangle::mangleName(const language::Compability::semantics::Symbol &symbol,
                                   bool keepExternalInScope,
                                   bool underscoring) {
  assert((symbol.owner().kind() !=
              language::Compability::semantics::Scope::Kind::BlockConstruct ||
          symbol.has<language::Compability::semantics::SubprogramDetails>() ||
          language::Compability::semantics::IsBindCProcedure(symbol)) &&
         "block object mangling must specify a scopeBlockIdMap");
  ScopeBlockIdMap scopeBlockIdMap;
  return mangleName(symbol, scopeBlockIdMap, keepExternalInScope, underscoring);
}

std::string language::Compability::lower::mangle::mangleName(
    const language::Compability::semantics::DerivedTypeSpec &derivedType,
    ScopeBlockIdMap &scopeBlockIdMap) {
  // Resolve module and host associations before mangling.
  const language::Compability::semantics::Symbol &ultimateSymbol =
      derivedType.typeSymbol().GetUltimate();

  toolchain::StringRef symbolName = toStringRef(ultimateSymbol.name());
  toolchain::SmallVector<toolchain::StringRef> modules;
  toolchain::SmallVector<toolchain::StringRef> procs;
  std::int64_t blockId;
  std::tie(modules, procs, blockId) =
      ancestors(ultimateSymbol, scopeBlockIdMap);
  toolchain::SmallVector<std::int64_t> kinds;
  for (const auto &param :
       language::Compability::semantics::OrderParameterDeclarations(ultimateSymbol)) {
    const auto &paramDetails =
        param->get<language::Compability::semantics::TypeParamDetails>();
    if (paramDetails.attr() == language::Compability::common::TypeParamAttr::Kind) {
      const language::Compability::semantics::ParamValue *paramValue =
          derivedType.FindParameter(param->name());
      assert(paramValue && "derived type kind parameter value not found");
      const language::Compability::semantics::MaybeIntExpr paramExpr =
          paramValue->GetExplicit();
      assert(paramExpr && "derived type kind param not explicit");
      std::optional<int64_t> init =
          language::Compability::evaluate::ToInt64(paramValue->GetExplicit());
      // TODO: put the assertion check back when parametrized derived types
      // are supported:
      // assert(init && "derived type kind param is not constant");
      //
      // The init parameter above will require a FoldingContext for proper
      // expression evaluation to an integer constant, otherwise the
      // compiler may crash here (see example in issue #127424).
      if (!init) {
        TODO_NOLOC("parameterized derived types");
      } else {
        kinds.emplace_back(*init);
      }
    }
  }
  return fir::NameUniquer::doType(modules, procs, blockId, symbolName, kinds);
}

std::string language::Compability::lower::mangle::getRecordTypeFieldName(
    const language::Compability::semantics::Symbol &component,
    ScopeBlockIdMap &scopeBlockIdMap) {
  if (!component.attrs().test(language::Compability::semantics::Attr::PRIVATE))
    return component.name().ToString();
  const language::Compability::semantics::DerivedTypeSpec *componentParentType =
      component.owner().derivedTypeSpec();
  assert(componentParentType &&
         "failed to retrieve private component parent type");
  // Do not mangle Iso C C_PTR and C_FUNPTR components. This type cannot be
  // extended as per Fortran 2018 7.5.7.1, mangling them makes the IR unreadable
  // when using ISO C modules, and lowering needs to know the component way
  // without access to semantics::Symbol.
  if (language::Compability::semantics::IsIsoCType(componentParentType))
    return component.name().ToString();
  return mangleName(*componentParentType, scopeBlockIdMap) + "." +
         component.name().ToString();
}

std::string language::Compability::lower::mangle::demangleName(toolchain::StringRef name) {
  auto result = fir::NameUniquer::deconstruct(name);
  return result.second.name;
}

//===----------------------------------------------------------------------===//
// Array Literals Mangling
//===----------------------------------------------------------------------===//

static std::string typeToString(language::Compability::common::TypeCategory cat, int kind,
                                toolchain::StringRef derivedName) {
  switch (cat) {
  case language::Compability::common::TypeCategory::Integer:
    return "i" + std::to_string(kind);
  case language::Compability::common::TypeCategory::Unsigned:
    return "u" + std::to_string(kind);
  case language::Compability::common::TypeCategory::Real:
    return "r" + std::to_string(kind);
  case language::Compability::common::TypeCategory::Complex:
    return "z" + std::to_string(kind);
  case language::Compability::common::TypeCategory::Logical:
    return "l" + std::to_string(kind);
  case language::Compability::common::TypeCategory::Character:
    return "c" + std::to_string(kind);
  case language::Compability::common::TypeCategory::Derived:
    return derivedName.str();
  }
  toolchain_unreachable("bad TypeCategory");
}

std::string language::Compability::lower::mangle::mangleArrayLiteral(
    size_t size, const language::Compability::evaluate::ConstantSubscripts &shape,
    language::Compability::common::TypeCategory cat, int kind,
    language::Compability::common::ConstantSubscript charLen, toolchain::StringRef derivedName) {
  std::string typeId;
  for (language::Compability::evaluate::ConstantSubscript extent : shape)
    typeId.append(std::to_string(extent)).append("x");
  if (charLen >= 0)
    typeId.append(std::to_string(charLen)).append("x");
  typeId.append(typeToString(cat, kind, derivedName));
  std::string name =
      fir::NameUniquer::doGenerated("ro."s.append(typeId).append("."));
  if (!size)
    name += "null.";
  return name;
}

std::string language::Compability::lower::mangle::globalNamelistDescriptorName(
    const language::Compability::semantics::Symbol &sym) {
  std::string name = mangleName(sym);
  return IsAllocatableOrObjectPointer(&sym) ? name : name + ".desc"s;
}
