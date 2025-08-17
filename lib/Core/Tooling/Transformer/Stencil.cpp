//===--- Stencil.cpp - Stencil implementation -------------------*- C++ -*-===//
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

#include "language/Core/Tooling/Transformer/Stencil.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/ASTTypeTraits.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/ASTMatchers/ASTMatchFinder.h"
#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Lex/Lexer.h"
#include "language/Core/Tooling/Transformer/SourceCode.h"
#include "language/Core/Tooling/Transformer/SourceCodeBuilders.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/Twine.h"
#include "toolchain/Support/Errc.h"
#include "toolchain/Support/Error.h"
#include <memory>
#include <string>

using namespace language::Core;
using namespace transformer;

using ast_matchers::BoundNodes;
using ast_matchers::MatchFinder;
using toolchain::errc;
using toolchain::Error;
using toolchain::Expected;
using toolchain::StringError;

static toolchain::Expected<DynTypedNode> getNode(const BoundNodes &Nodes,
                                            StringRef Id) {
  auto &NodesMap = Nodes.getMap();
  auto It = NodesMap.find(Id);
  if (It == NodesMap.end())
    return toolchain::make_error<toolchain::StringError>(toolchain::errc::invalid_argument,
                                               "Id not bound: " + Id);
  return It->second;
}

static Error printNode(StringRef Id, const MatchFinder::MatchResult &Match,
                       std::string *Result) {
  std::string Output;
  toolchain::raw_string_ostream Os(Output);
  auto NodeOrErr = getNode(Match.Nodes, Id);
  if (auto Err = NodeOrErr.takeError())
    return Err;
  const PrintingPolicy PP(Match.Context->getLangOpts());
  if (const auto *ND = NodeOrErr->get<NamedDecl>()) {
    // For NamedDecls, we can do a better job than printing the whole thing.
    ND->getNameForDiagnostic(Os, PP, false);
  } else {
    NodeOrErr->print(Os, PP);
  }
  *Result += Output;
  return Error::success();
}

namespace {
// An arbitrary fragment of code within a stencil.
class RawTextStencil : public StencilInterface {
  std::string Text;

public:
  explicit RawTextStencil(std::string T) : Text(std::move(T)) {}

  std::string toString() const override {
    std::string Result;
    toolchain::raw_string_ostream OS(Result);
    OS << "\"";
    OS.write_escaped(Text);
    OS << "\"";
    return Result;
  }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {
    Result->append(Text);
    return Error::success();
  }
};

// A debugging operation to dump the AST for a particular (bound) AST node.
class DebugPrintNodeStencil : public StencilInterface {
  std::string Id;

public:
  explicit DebugPrintNodeStencil(std::string S) : Id(std::move(S)) {}

  std::string toString() const override {
    return (toolchain::Twine("dPrint(\"") + Id + "\")").str();
  }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {
    return printNode(Id, Match, Result);
  }
};

// Operators that take a single node Id as an argument.
enum class UnaryNodeOperator {
  Parens,
  Deref,
  MaybeDeref,
  AddressOf,
  MaybeAddressOf,
  Describe,
};

// Generic container for stencil operations with a (single) node-id argument.
class UnaryOperationStencil : public StencilInterface {
  UnaryNodeOperator Op;
  std::string Id;

public:
  UnaryOperationStencil(UnaryNodeOperator Op, std::string Id)
      : Op(Op), Id(std::move(Id)) {}

  std::string toString() const override {
    StringRef OpName;
    switch (Op) {
    case UnaryNodeOperator::Parens:
      OpName = "expression";
      break;
    case UnaryNodeOperator::Deref:
      OpName = "deref";
      break;
    case UnaryNodeOperator::MaybeDeref:
      OpName = "maybeDeref";
      break;
    case UnaryNodeOperator::AddressOf:
      OpName = "addressOf";
      break;
    case UnaryNodeOperator::MaybeAddressOf:
      OpName = "maybeAddressOf";
      break;
    case UnaryNodeOperator::Describe:
      OpName = "describe";
      break;
    }
    return (OpName + "(\"" + Id + "\")").str();
  }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {
    // The `Describe` operation can be applied to any node, not just
    // expressions, so it is handled here, separately.
    if (Op == UnaryNodeOperator::Describe)
      return printNode(Id, Match, Result);

    const auto *E = Match.Nodes.getNodeAs<Expr>(Id);
    if (E == nullptr)
      return toolchain::make_error<StringError>(errc::invalid_argument,
                                           "Id not bound or not Expr: " + Id);
    std::optional<std::string> Source;
    switch (Op) {
    case UnaryNodeOperator::Parens:
      Source = tooling::buildParens(*E, *Match.Context);
      break;
    case UnaryNodeOperator::Deref:
      Source = tooling::buildDereference(*E, *Match.Context);
      break;
    case UnaryNodeOperator::MaybeDeref:
      if (E->getType()->isAnyPointerType() ||
          tooling::isKnownPointerLikeType(E->getType(), *Match.Context)) {
        // Strip off any operator->. This can only occur inside an actual arrow
        // member access, so we treat it as equivalent to an actual object
        // expression.
        if (const auto *OpCall = dyn_cast<language::Core::CXXOperatorCallExpr>(E)) {
          if (OpCall->getOperator() == language::Core::OO_Arrow &&
              OpCall->getNumArgs() == 1) {
            E = OpCall->getArg(0);
          }
        }
        Source = tooling::buildDereference(*E, *Match.Context);
        break;
      }
      *Result += tooling::getText(*E, *Match.Context);
      return Error::success();
    case UnaryNodeOperator::AddressOf:
      Source = tooling::buildAddressOf(*E, *Match.Context);
      break;
    case UnaryNodeOperator::MaybeAddressOf:
      if (E->getType()->isAnyPointerType() ||
          tooling::isKnownPointerLikeType(E->getType(), *Match.Context)) {
        // Strip off any operator->. This can only occur inside an actual arrow
        // member access, so we treat it as equivalent to an actual object
        // expression.
        if (const auto *OpCall = dyn_cast<language::Core::CXXOperatorCallExpr>(E)) {
          if (OpCall->getOperator() == language::Core::OO_Arrow &&
              OpCall->getNumArgs() == 1) {
            E = OpCall->getArg(0);
          }
        }
        *Result += tooling::getText(*E, *Match.Context);
        return Error::success();
      }
      Source = tooling::buildAddressOf(*E, *Match.Context);
      break;
    case UnaryNodeOperator::Describe:
      toolchain_unreachable("This case is handled at the start of the function");
    }
    if (!Source)
      return toolchain::make_error<StringError>(
          errc::invalid_argument,
          "Could not construct expression source from ID: " + Id);
    *Result += *Source;
    return Error::success();
  }
};

// The fragment of code corresponding to the selected range.
class SelectorStencil : public StencilInterface {
  RangeSelector Selector;

public:
  explicit SelectorStencil(RangeSelector S) : Selector(std::move(S)) {}

  std::string toString() const override { return "selection(...)"; }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {
    auto RawRange = Selector(Match);
    if (!RawRange)
      return RawRange.takeError();
    CharSourceRange Range = Lexer::makeFileCharRange(
        *RawRange, *Match.SourceManager, Match.Context->getLangOpts());
    if (Range.isInvalid()) {
      // Validate the original range to attempt to get a meaningful error
      // message. If it's valid, then something else is the cause and we just
      // return the generic failure message.
      if (auto Err = tooling::validateRange(*RawRange, *Match.SourceManager,
                                            /*AllowSystemHeaders=*/true))
        return handleErrors(std::move(Err), [](std::unique_ptr<StringError> E) {
          assert(E->convertToErrorCode() ==
                     toolchain::make_error_code(errc::invalid_argument) &&
                 "Validation errors must carry the invalid_argument code");
          return toolchain::createStringError(
              errc::invalid_argument,
              "selected range could not be resolved to a valid source range; " +
                  E->getMessage());
        });
      return toolchain::createStringError(
          errc::invalid_argument,
          "selected range could not be resolved to a valid source range");
    }
    // Validate `Range`, because `makeFileCharRange` accepts some ranges that
    // `validateRange` rejects.
    if (auto Err = tooling::validateRange(Range, *Match.SourceManager,
                                          /*AllowSystemHeaders=*/true))
      return joinErrors(
          toolchain::createStringError(errc::invalid_argument,
                                  "selected range is not valid for editing"),
          std::move(Err));
    *Result += tooling::getText(Range, *Match.Context);
    return Error::success();
  }
};

// A stencil operation to build a member access `e.m` or `e->m`, as appropriate.
class AccessStencil : public StencilInterface {
  std::string BaseId;
  Stencil Member;

public:
  AccessStencil(StringRef BaseId, Stencil Member)
      : BaseId(std::string(BaseId)), Member(std::move(Member)) {}

  std::string toString() const override {
    return (toolchain::Twine("access(\"") + BaseId + "\", " + Member->toString() +
            ")")
        .str();
  }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {
    const auto *E = Match.Nodes.getNodeAs<Expr>(BaseId);
    if (E == nullptr)
      return toolchain::make_error<StringError>(errc::invalid_argument,
                                           "Id not bound: " + BaseId);
    std::optional<std::string> S = tooling::buildAccess(*E, *Match.Context);
    if (!S)
      return toolchain::make_error<StringError>(
          errc::invalid_argument,
          "Could not construct object text from ID: " + BaseId);
    *Result += *S;
    return Member->eval(Match, Result);
  }
};

class IfBoundStencil : public StencilInterface {
  std::string Id;
  Stencil TrueStencil;
  Stencil FalseStencil;

public:
  IfBoundStencil(StringRef Id, Stencil TrueStencil, Stencil FalseStencil)
      : Id(std::string(Id)), TrueStencil(std::move(TrueStencil)),
        FalseStencil(std::move(FalseStencil)) {}

  std::string toString() const override {
    return (toolchain::Twine("ifBound(\"") + Id + "\", " + TrueStencil->toString() +
            ", " + FalseStencil->toString() + ")")
        .str();
  }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {
    auto &M = Match.Nodes.getMap();
    return (M.find(Id) != M.end() ? TrueStencil : FalseStencil)
        ->eval(Match, Result);
  }
};

class SelectBoundStencil : public language::Core::transformer::StencilInterface {
  static bool containsNoNullStencils(
      const std::vector<std::pair<std::string, Stencil>> &Cases) {
    for (const auto &S : Cases)
      if (S.second == nullptr)
        return false;
    return true;
  }

public:
  SelectBoundStencil(std::vector<std::pair<std::string, Stencil>> Cases,
                     Stencil Default)
      : CaseStencils(std::move(Cases)), DefaultStencil(std::move(Default)) {
    assert(containsNoNullStencils(CaseStencils) &&
           "cases of selectBound may not be null");
  }
  ~SelectBoundStencil() override {}

  toolchain::Error eval(const MatchFinder::MatchResult &match,
                   std::string *result) const override {
    const BoundNodes::IDToNodeMap &NodeMap = match.Nodes.getMap();
    for (const auto &S : CaseStencils) {
      if (NodeMap.count(S.first) > 0) {
        return S.second->eval(match, result);
      }
    }

    if (DefaultStencil != nullptr) {
      return DefaultStencil->eval(match, result);
    }

    toolchain::SmallVector<toolchain::StringRef, 2> CaseIDs;
    CaseIDs.reserve(CaseStencils.size());
    for (const auto &S : CaseStencils)
      CaseIDs.emplace_back(S.first);

    return toolchain::createStringError(
        errc::result_out_of_range,
        toolchain::Twine("selectBound failed: no cases bound and no default: {") +
            toolchain::join(CaseIDs, ", ") + "}");
  }

  std::string toString() const override {
    std::string Buffer;
    toolchain::raw_string_ostream Stream(Buffer);
    Stream << "selectBound({";
    bool First = true;
    for (const auto &S : CaseStencils) {
      if (First)
        First = false;
      else
        Stream << "}, ";
      Stream << "{\"" << S.first << "\", " << S.second->toString();
    }
    Stream << "}}";
    if (DefaultStencil != nullptr) {
      Stream << ", " << DefaultStencil->toString();
    }
    Stream << ")";
    return Buffer;
  }

private:
  std::vector<std::pair<std::string, Stencil>> CaseStencils;
  Stencil DefaultStencil;
};

class SequenceStencil : public StencilInterface {
  std::vector<Stencil> Stencils;

public:
  SequenceStencil(std::vector<Stencil> Stencils)
      : Stencils(std::move(Stencils)) {}

  std::string toString() const override {
    toolchain::SmallVector<std::string, 2> Parts;
    Parts.reserve(Stencils.size());
    for (const auto &S : Stencils)
      Parts.push_back(S->toString());
    return (toolchain::Twine("seq(") + toolchain::join(Parts, ", ") + ")").str();
  }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {
    for (const auto &S : Stencils)
      if (auto Err = S->eval(Match, Result))
        return Err;
    return Error::success();
  }
};

class RunStencil : public StencilInterface {
  MatchConsumer<std::string> Consumer;

public:
  explicit RunStencil(MatchConsumer<std::string> C) : Consumer(std::move(C)) {}

  std::string toString() const override { return "run(...)"; }

  Error eval(const MatchFinder::MatchResult &Match,
             std::string *Result) const override {

    Expected<std::string> Value = Consumer(Match);
    if (!Value)
      return Value.takeError();
    *Result += *Value;
    return Error::success();
  }
};
} // namespace

Stencil transformer::detail::makeStencil(StringRef Text) {
  return std::make_shared<RawTextStencil>(std::string(Text));
}

Stencil transformer::detail::makeStencil(RangeSelector Selector) {
  return std::make_shared<SelectorStencil>(std::move(Selector));
}

Stencil transformer::dPrint(StringRef Id) {
  return std::make_shared<DebugPrintNodeStencil>(std::string(Id));
}

Stencil transformer::expression(toolchain::StringRef Id) {
  return std::make_shared<UnaryOperationStencil>(UnaryNodeOperator::Parens,
                                                 std::string(Id));
}

Stencil transformer::deref(toolchain::StringRef ExprId) {
  return std::make_shared<UnaryOperationStencil>(UnaryNodeOperator::Deref,
                                                 std::string(ExprId));
}

Stencil transformer::maybeDeref(toolchain::StringRef ExprId) {
  return std::make_shared<UnaryOperationStencil>(UnaryNodeOperator::MaybeDeref,
                                                 std::string(ExprId));
}

Stencil transformer::addressOf(toolchain::StringRef ExprId) {
  return std::make_shared<UnaryOperationStencil>(UnaryNodeOperator::AddressOf,
                                                 std::string(ExprId));
}

Stencil transformer::maybeAddressOf(toolchain::StringRef ExprId) {
  return std::make_shared<UnaryOperationStencil>(
      UnaryNodeOperator::MaybeAddressOf, std::string(ExprId));
}

Stencil transformer::describe(StringRef Id) {
  return std::make_shared<UnaryOperationStencil>(UnaryNodeOperator::Describe,
                                                 std::string(Id));
}

Stencil transformer::access(StringRef BaseId, Stencil Member) {
  return std::make_shared<AccessStencil>(BaseId, std::move(Member));
}

Stencil transformer::ifBound(StringRef Id, Stencil TrueStencil,
                             Stencil FalseStencil) {
  return std::make_shared<IfBoundStencil>(Id, std::move(TrueStencil),
                                          std::move(FalseStencil));
}

Stencil transformer::selectBound(
    std::vector<std::pair<std::string, Stencil>> CaseStencils,
    Stencil DefaultStencil) {
  return std::make_shared<SelectBoundStencil>(std::move(CaseStencils),
                                              std::move(DefaultStencil));
}

Stencil transformer::run(MatchConsumer<std::string> Fn) {
  return std::make_shared<RunStencil>(std::move(Fn));
}

Stencil transformer::catVector(std::vector<Stencil> Parts) {
  // Only one argument, so don't wrap in sequence.
  if (Parts.size() == 1)
    return std::move(Parts[0]);
  return std::make_shared<SequenceStencil>(std::move(Parts));
}
