//===- DirectiveNameParser.cpp --------------------------------------------===//
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

#include "vm/core/Frontend/OpenMP/DirectiveNameParser.h"
#include "vm/core/ADT/Sequence.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Frontend/OpenMP/OMP.h"

#include <cassert>
#include <memory>

namespace vm::core::omp {
DirectiveNameParser::DirectiveNameParser(SourceLanguage L) {
  // Take every directive, get its name in every version, break the name up
  // into whitespace-separated tokens, and insert each token.
  for (size_t I : toolchain::seq<size_t>(Directive_enumSize)) {
    auto D = static_cast<Directive>(I);
    if (D == Directive::OMPD_unknown || !(getDirectiveLanguages(D) & L))
      continue;
    for (unsigned Ver : getOpenMPVersions())
      insertName(getOpenMPDirectiveName(D, Ver), D);
  }
}

const DirectiveNameParser::State *
DirectiveNameParser::consume(const State *Current, StringRef Tok) const {
  if (!Current)
    return Current;
  assert(Current->isValid() && "Invalid input state");
  if (const State *Next = Current->next(Tok))
    return Next->isValid() ? Next : nullptr;
  return nullptr;
}

SmallVector<StringRef> DirectiveNameParser::tokenize(StringRef Str) {
  SmallVector<StringRef> Tokens;
  SplitString(Str, Tokens);
  return Tokens;
}

void DirectiveNameParser::insertName(StringRef Name, Directive D) {
  State *Where = &InitialState;

  for (StringRef Tok : tokenize(Name))
    Where = insertTransition(Where, Tok);

  Where->Value = D;
}

DirectiveNameParser::State *
DirectiveNameParser::insertTransition(State *From, StringRef Tok) {
  assert(From && "Expecting state");
  if (!From->Transition)
    From->Transition = std::make_unique<State::TransitionMapTy>();
  if (State *Next = From->next(Tok))
    return Next;

  auto [Where, DidIt] = From->Transition->try_emplace(Tok, State());
  assert(DidIt && "Map insertion failed");
  return &Where->second;
}

const DirectiveNameParser::State *
DirectiveNameParser::State::next(StringRef Tok) const {
  if (!Transition)
    return nullptr;
  auto F = Transition->find(Tok);
  return F != Transition->end() ? &F->second : nullptr;
}

DirectiveNameParser::State *DirectiveNameParser::State::next(StringRef Tok) {
  if (!Transition)
    return nullptr;
  auto F = Transition->find(Tok);
  return F != Transition->end() ? &F->second : nullptr;
}
} // namespace vm::core::omp
