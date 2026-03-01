//===- Arg.cpp - Argument Implementations ---------------------------------===//
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

#include "vm/core/Option/Arg.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/Option/ArgList.h"
#include "vm/core/Option/Option.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/InterleavedRange.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace vm::core::opt;

Arg::Arg(const Option Opt, StringRef S, unsigned Index, const Arg *BaseArg)
    : Opt(Opt), BaseArg(BaseArg), Spelling(S), Index(Index), Claimed(false),
      IgnoredTargetSpecific(false), OwnsValues(false) {}

Arg::Arg(const Option Opt, StringRef S, unsigned Index, const char *Value0,
         const Arg *BaseArg)
    : Opt(Opt), BaseArg(BaseArg), Spelling(S), Index(Index), Claimed(false),
      IgnoredTargetSpecific(false), OwnsValues(false) {
  Values.push_back(Value0);
}

Arg::Arg(const Option Opt, StringRef S, unsigned Index, const char *Value0,
         const char *Value1, const Arg *BaseArg)
    : Opt(Opt), BaseArg(BaseArg), Spelling(S), Index(Index), Claimed(false),
      IgnoredTargetSpecific(false), OwnsValues(false) {
  Values.push_back(Value0);
  Values.push_back(Value1);
}

Arg::~Arg() {
  if (OwnsValues) {
    for (const char *V : Values)
      delete[] V;
  }
}

void Arg::print(raw_ostream& O) const {
  O << "<Opt:";
  Opt.print(O, /*AddNewLine=*/false);

  O << " Index:" << Index;

  O << " Values: [";
  for (unsigned i = 0, e = Values.size(); i != e; ++i) {
    if (i) O << ", ";
    O << "'" << Values[i] << "'";
  }

  O << "]>\n";
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void Arg::dump() const { print(dbgs()); }
#endif

std::string Arg::getAsString(const ArgList &Args) const {
  if (Alias)
    return Alias->getAsString(Args);

  SmallString<256> Res;
  raw_svector_ostream OS(Res);

  ArgStringList ASL;
  render(Args, ASL);
  OS << toolchain::interleaved(ASL, " ");
  return std::string(OS.str());
}

void Arg::renderAsInput(const ArgList &Args, ArgStringList &Output) const {
  if (!getOption().hasNoOptAsInput()) {
    render(Args, Output);
    return;
  }

  Output.append(Values.begin(), Values.end());
}

void Arg::render(const ArgList &Args, ArgStringList &Output) const {
  switch (getOption().getRenderStyle()) {
  case Option::RenderValuesStyle:
    Output.append(Values.begin(), Values.end());
    break;

  case Option::RenderCommaJoinedStyle: {
    SmallString<256> Res;
    raw_svector_ostream OS(Res);
    OS << getSpelling() << toolchain::interleaved(getValues(), ",");
    Output.push_back(Args.MakeArgString(OS.str()));
    break;
  }

 case Option::RenderJoinedStyle:
    Output.push_back(Args.GetOrMakeJoinedArgString(
                       getIndex(), getSpelling(), getValue(0)));
    Output.append(Values.begin() + 1, Values.end());
    break;

  case Option::RenderSeparateStyle:
    Output.push_back(Args.MakeArgString(getSpelling()));
    Output.append(Values.begin(), Values.end());
    break;
  }
}
