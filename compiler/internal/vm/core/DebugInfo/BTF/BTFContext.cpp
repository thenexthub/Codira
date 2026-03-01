//===- BTFContext.cpp ---------------------------------------------------===//
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
//
// Implementation of the BTFContext interface, this is used by
// toolchain-objdump tool to print source code alongside disassembly.
// In fact, currently it is a simple wrapper for BTFParser instance.
//
//===----------------------------------------------------------------------===//

#include "vm/core/DebugInfo/BTF/BTFContext.h"

#define DEBUG_TYPE "debug-info-btf-context"

using namespace vm::core;
using object::ObjectFile;
using object::SectionedAddress;

std::optional<DILineInfo>
BTFContext::getLineInfoForAddress(SectionedAddress Address,
                                  DILineInfoSpecifier Specifier) {
  const BTF::BPFLineInfo *LineInfo = BTF.findLineInfo(Address);
  DILineInfo Result;
  if (!LineInfo)
    return std::nullopt;

  Result.LineSource = BTF.findString(LineInfo->LineOff);
  Result.FileName = BTF.findString(LineInfo->FileNameOff);
  Result.Line = LineInfo->getLine();
  Result.Column = LineInfo->getCol();
  return Result;
}

std::optional<DILineInfo>
BTFContext::getLineInfoForDataAddress(SectionedAddress Address) {
  // BTF does not convey such information.
  return std::nullopt;
}

DILineInfoTable
BTFContext::getLineInfoForAddressRange(SectionedAddress Address, uint64_t Size,
                                       DILineInfoSpecifier Specifier) {
  // This function is used only from toolchain-rtdyld utility and a few
  // JITEventListener implementations. Ignore it for now.
  return {};
}

DIInliningInfo
BTFContext::getInliningInfoForAddress(SectionedAddress Address,
                                      DILineInfoSpecifier Specifier) {
  // BTF does not convey such information
  return {};
}

std::vector<DILocal> BTFContext::getLocalsForAddress(SectionedAddress Address) {
  // BTF does not convey such information
  return {};
}

std::unique_ptr<BTFContext>
BTFContext::create(const ObjectFile &Obj,
                   std::function<void(Error)> ErrorHandler) {
  auto Ctx = std::make_unique<BTFContext>();
  BTFParser::ParseOptions Opts;
  Opts.LoadLines = true;
  if (Error E = Ctx->BTF.parse(Obj, Opts))
    ErrorHandler(std::move(E));
  return Ctx;
}
