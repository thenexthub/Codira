//===--- Marshallers.cpp ----------------------------------------*- C++ -*-===//
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

#include "Marshallers.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/Regex.h"
#include <optional>
#include <string>

static std::optional<std::string>
getBestGuess(toolchain::StringRef Search, toolchain::ArrayRef<toolchain::StringRef> Allowed,
             toolchain::StringRef DropPrefix = "", unsigned MaxEditDistance = 3) {
  if (MaxEditDistance != ~0U)
    ++MaxEditDistance;
  toolchain::StringRef Res;
  for (const toolchain::StringRef &Item : Allowed) {
    if (Item.equals_insensitive(Search)) {
      assert(Item != Search && "This should be handled earlier on.");
      MaxEditDistance = 1;
      Res = Item;
      continue;
    }
    unsigned Distance = Item.edit_distance(Search);
    if (Distance < MaxEditDistance) {
      MaxEditDistance = Distance;
      Res = Item;
    }
  }
  if (!Res.empty())
    return Res.str();
  if (!DropPrefix.empty()) {
    --MaxEditDistance; // Treat dropping the prefix as 1 edit
    for (const toolchain::StringRef &Item : Allowed) {
      auto NoPrefix = Item;
      if (!NoPrefix.consume_front(DropPrefix))
        continue;
      if (NoPrefix.equals_insensitive(Search)) {
        if (NoPrefix == Search)
          return Item.str();
        MaxEditDistance = 1;
        Res = Item;
        continue;
      }
      unsigned Distance = NoPrefix.edit_distance(Search);
      if (Distance < MaxEditDistance) {
        MaxEditDistance = Distance;
        Res = Item;
      }
    }
    if (!Res.empty())
      return Res.str();
  }
  return std::nullopt;
}

std::optional<std::string>
language::Core::ast_matchers::dynamic::internal::ArgTypeTraits<
    language::Core::attr::Kind>::getBestGuess(const VariantValue &Value) {
  static constexpr toolchain::StringRef Allowed[] = {
#define ATTR(X) "attr::" #X,
#include "language/Core/Basic/AttrList.inc"
  };
  if (Value.isString())
    return ::getBestGuess(Value.getString(), toolchain::ArrayRef(Allowed), "attr::");
  return std::nullopt;
}

std::optional<std::string>
language::Core::ast_matchers::dynamic::internal::ArgTypeTraits<
    language::Core::CastKind>::getBestGuess(const VariantValue &Value) {
  static constexpr toolchain::StringRef Allowed[] = {
#define CAST_OPERATION(Name) "CK_" #Name,
#include "language/Core/AST/OperationKinds.def"
  };
  if (Value.isString())
    return ::getBestGuess(Value.getString(), toolchain::ArrayRef(Allowed), "CK_");
  return std::nullopt;
}

std::optional<std::string>
language::Core::ast_matchers::dynamic::internal::ArgTypeTraits<
    language::Core::OpenMPClauseKind>::getBestGuess(const VariantValue &Value) {
  static constexpr toolchain::StringRef Allowed[] = {
#define GEN_CLANG_CLAUSE_CLASS
#define CLAUSE_CLASS(Enum, Str, Class) #Enum,
#include "toolchain/Frontend/OpenMP/OMP.inc"
  };
  if (Value.isString())
    return ::getBestGuess(Value.getString(), toolchain::ArrayRef(Allowed), "OMPC_");
  return std::nullopt;
}

std::optional<std::string>
language::Core::ast_matchers::dynamic::internal::ArgTypeTraits<
    language::Core::UnaryExprOrTypeTrait>::getBestGuess(const VariantValue &Value) {
  static constexpr toolchain::StringRef Allowed[] = {
#define UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) "UETT_" #Name,
#define CXX11_UNARY_EXPR_OR_TYPE_TRAIT(Spelling, Name, Key) "UETT_" #Name,
#include "language/Core/Basic/TokenKinds.def"
  };
  if (Value.isString())
    return ::getBestGuess(Value.getString(), toolchain::ArrayRef(Allowed), "UETT_");
  return std::nullopt;
}

static constexpr std::pair<toolchain::StringRef, toolchain::Regex::RegexFlags>
    RegexMap[] = {
        {"NoFlags", toolchain::Regex::RegexFlags::NoFlags},
        {"IgnoreCase", toolchain::Regex::RegexFlags::IgnoreCase},
        {"Newline", toolchain::Regex::RegexFlags::Newline},
        {"BasicRegex", toolchain::Regex::RegexFlags::BasicRegex},
};

static std::optional<toolchain::Regex::RegexFlags>
getRegexFlag(toolchain::StringRef Flag) {
  for (const auto &StringFlag : RegexMap) {
    if (Flag == StringFlag.first)
      return StringFlag.second;
  }
  return std::nullopt;
}

static std::optional<toolchain::StringRef> getCloseRegexMatch(toolchain::StringRef Flag) {
  for (const auto &StringFlag : RegexMap) {
    if (Flag.edit_distance(StringFlag.first) < 3)
      return StringFlag.first;
  }
  return std::nullopt;
}

std::optional<toolchain::Regex::RegexFlags>
language::Core::ast_matchers::dynamic::internal::ArgTypeTraits<
    toolchain::Regex::RegexFlags>::getFlags(toolchain::StringRef Flags) {
  std::optional<toolchain::Regex::RegexFlags> Flag;
  SmallVector<StringRef, 4> Split;
  Flags.split(Split, '|', -1, false);
  for (StringRef OrFlag : Split) {
    if (std::optional<toolchain::Regex::RegexFlags> NextFlag =
            getRegexFlag(OrFlag.trim()))
      Flag = Flag.value_or(toolchain::Regex::NoFlags) | *NextFlag;
    else
      return std::nullopt;
  }
  return Flag;
}

std::optional<std::string>
language::Core::ast_matchers::dynamic::internal::ArgTypeTraits<
    toolchain::Regex::RegexFlags>::getBestGuess(const VariantValue &Value) {
  if (!Value.isString())
    return std::nullopt;
  SmallVector<StringRef, 4> Split;
  toolchain::StringRef(Value.getString()).split(Split, '|', -1, false);
  for (toolchain::StringRef &Flag : Split) {
    if (std::optional<toolchain::StringRef> BestGuess =
            getCloseRegexMatch(Flag.trim()))
      Flag = *BestGuess;
    else
      return std::nullopt;
  }
  if (Split.empty())
    return std::nullopt;
  return toolchain::join(Split, " | ");
}
