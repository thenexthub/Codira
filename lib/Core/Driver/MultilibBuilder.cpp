//===- MultilibBuilder.cpp - MultilibBuilder Implementation -===//
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

#include "language/Core/Driver/MultilibBuilder.h"
#include "language/Core/Driver/CommonArgs.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/Regex.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language::Core;
using namespace driver;

/// normalize Segment to "/foo/bar" or "".
static void normalizePathSegment(std::string &Segment) {
  StringRef seg = Segment;

  // Prune trailing "/" or "./"
  while (true) {
    StringRef last = toolchain::sys::path::filename(seg);
    if (last != ".")
      break;
    seg = toolchain::sys::path::parent_path(seg);
  }

  if (seg.empty() || seg == "/") {
    Segment.clear();
    return;
  }

  // Add leading '/'
  if (seg.front() != '/') {
    Segment = "/" + seg.str();
  } else {
    Segment = std::string(seg);
  }
}

MultilibBuilder::MultilibBuilder(StringRef GCC, StringRef OS, StringRef Include)
    : GCCSuffix(GCC), OSSuffix(OS), IncludeSuffix(Include) {
  normalizePathSegment(GCCSuffix);
  normalizePathSegment(OSSuffix);
  normalizePathSegment(IncludeSuffix);
}

MultilibBuilder::MultilibBuilder(StringRef Suffix)
    : MultilibBuilder(Suffix, Suffix, Suffix) {}

MultilibBuilder &MultilibBuilder::gccSuffix(StringRef S) {
  GCCSuffix = std::string(S);
  normalizePathSegment(GCCSuffix);
  return *this;
}

MultilibBuilder &MultilibBuilder::osSuffix(StringRef S) {
  OSSuffix = std::string(S);
  normalizePathSegment(OSSuffix);
  return *this;
}

MultilibBuilder &MultilibBuilder::includeSuffix(StringRef S) {
  IncludeSuffix = std::string(S);
  normalizePathSegment(IncludeSuffix);
  return *this;
}

bool MultilibBuilder::isValid() const {
  toolchain::StringMap<int> FlagSet;
  for (unsigned I = 0, N = Flags.size(); I != N; ++I) {
    StringRef Flag(Flags[I]);
    auto [SI, Inserted] = FlagSet.try_emplace(Flag.substr(1), I);

    assert(StringRef(Flag).front() == '-' || StringRef(Flag).front() == '!');

    if (!Inserted && Flags[I] != Flags[SI->getValue()])
      return false;
  }
  return true;
}

MultilibBuilder &MultilibBuilder::flag(StringRef Flag, bool Disallow) {
  tools::addMultilibFlag(!Disallow, Flag, Flags);
  return *this;
}

Multilib MultilibBuilder::makeMultilib() const {
  return Multilib(GCCSuffix, OSSuffix, IncludeSuffix, Flags);
}

MultilibSetBuilder &MultilibSetBuilder::Maybe(const MultilibBuilder &M) {
  MultilibBuilder Opposite;
  // Negate positive flags
  for (StringRef Flag : M.flags()) {
    if (Flag.front() == '-')
      Opposite.flag(Flag, /*Disallow=*/true);
  }
  return Either(M, Opposite);
}

MultilibSetBuilder &MultilibSetBuilder::Either(const MultilibBuilder &M1,
                                               const MultilibBuilder &M2) {
  return Either({M1, M2});
}

MultilibSetBuilder &MultilibSetBuilder::Either(const MultilibBuilder &M1,
                                               const MultilibBuilder &M2,
                                               const MultilibBuilder &M3) {
  return Either({M1, M2, M3});
}

MultilibSetBuilder &MultilibSetBuilder::Either(const MultilibBuilder &M1,
                                               const MultilibBuilder &M2,
                                               const MultilibBuilder &M3,
                                               const MultilibBuilder &M4) {
  return Either({M1, M2, M3, M4});
}

MultilibSetBuilder &MultilibSetBuilder::Either(const MultilibBuilder &M1,
                                               const MultilibBuilder &M2,
                                               const MultilibBuilder &M3,
                                               const MultilibBuilder &M4,
                                               const MultilibBuilder &M5) {
  return Either({M1, M2, M3, M4, M5});
}

static MultilibBuilder compose(const MultilibBuilder &Base,
                               const MultilibBuilder &New) {
  SmallString<128> GCCSuffix;
  toolchain::sys::path::append(GCCSuffix, "/", Base.gccSuffix(), New.gccSuffix());
  SmallString<128> OSSuffix;
  toolchain::sys::path::append(OSSuffix, "/", Base.osSuffix(), New.osSuffix());
  SmallString<128> IncludeSuffix;
  toolchain::sys::path::append(IncludeSuffix, "/", Base.includeSuffix(),
                          New.includeSuffix());

  MultilibBuilder Composed(GCCSuffix, OSSuffix, IncludeSuffix);

  MultilibBuilder::flags_list &Flags = Composed.flags();

  toolchain::append_range(Flags, Base.flags());
  toolchain::append_range(Flags, New.flags());

  return Composed;
}

MultilibSetBuilder &
MultilibSetBuilder::Either(ArrayRef<MultilibBuilder> MultilibSegments) {
  multilib_list Composed;

  if (Multilibs.empty())
    toolchain::append_range(Multilibs, MultilibSegments);
  else {
    for (const auto &New : MultilibSegments) {
      for (const auto &Base : Multilibs) {
        MultilibBuilder MO = compose(Base, New);
        if (MO.isValid())
          Composed.push_back(MO);
      }
    }

    Multilibs = Composed;
  }

  return *this;
}

MultilibSetBuilder &MultilibSetBuilder::FilterOut(const char *Regex) {
  toolchain::Regex R(Regex);
#ifndef NDEBUG
  std::string Error;
  if (!R.isValid(Error)) {
    toolchain::errs() << Error;
    toolchain_unreachable("Invalid regex!");
  }
#endif
  toolchain::erase_if(Multilibs, [&R](const MultilibBuilder &M) {
    return R.match(M.gccSuffix());
  });
  return *this;
}

MultilibSet MultilibSetBuilder::makeMultilibSet() const {
  MultilibSet Result;
  for (const auto &M : Multilibs) {
    Result.push_back(M.makeMultilib());
  }
  return Result;
}
