//===-- StringSaver.cpp ---------------------------------------------------===//
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

#include "vm/core/Support/StringSaver.h"

#include "vm/core/ADT/SmallString.h"

using namespace vm::core;

StringRef StringSaver::save(StringRef S) {
  char *P = Alloc.Allocate<char>(S.size() + 1);
  if (!S.empty())
    memcpy(P, S.data(), S.size());
  P[S.size()] = '\0';
  return StringRef(P, S.size());
}

StringRef StringSaver::save(const Twine &S) {
  SmallString<128> Storage;
  return save(S.toStringRef(Storage));
}

StringRef UniqueStringSaver::save(StringRef S) {
  auto R = Unique.insert(S);
  if (R.second)                 // cache miss, need to actually save the string
    *R.first = Strings.save(S); // safe replacement with equal value
  return *R.first;
}

StringRef UniqueStringSaver::save(const Twine &S) {
  SmallString<128> Storage;
  return save(S.toStringRef(Storage));
}
