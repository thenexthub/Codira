//===-- Environment.cpp ---------------------------------------------------===//
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

#include "lldb/Utility/Environment.h"

using namespace lldb_private;

char *Environment::Envp::make_entry(llvm::StringRef Key,
                                    llvm::StringRef Value) {
  const size_t size = Key.size() + 1 /*=*/ + Value.size() + 1 /*\0*/;
  char *Result = static_cast<char *>(
      Allocator.Allocate(sizeof(char) * size, alignof(char)));
  char *Next = Result;

  Next = std::copy(Key.begin(), Key.end(), Next);
  *Next++ = '=';
  Next = std::copy(Value.begin(), Value.end(), Next);
  *Next++ = '\0';

  return Result;
}

Environment::Envp::Envp(const Environment &Env) {
  Data = static_cast<char **>(
      Allocator.Allocate(sizeof(char *) * (Env.size() + 1), alignof(char *)));
  char **Next = Data;
  for (const auto &KV : Env)
    *Next++ = make_entry(KV.first(), KV.second);
  *Next++ = nullptr;
}

Environment::Environment(const char *const *Env) {
  if (!Env)
    return;
  while (*Env)
    insert(*Env++);
}

void Environment::insert(iterator first, iterator last) {
  while (first != last) {
    try_emplace(first->first(), first->second);
    ++first;
  }
}
