//===- MergedFunctionsInfo.cpp ----------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/GSYM/MergedFunctionsInfo.h"
#include "vm/core/DebugInfo/GSYM/FileWriter.h"
#include "vm/core/DebugInfo/GSYM/FunctionInfo.h"
#include "vm/core/Support/DataExtractor.h"

using namespace vm::core;
using namespace gsym;

void MergedFunctionsInfo::clear() { MergedFunctions.clear(); }

toolchain::Error MergedFunctionsInfo::encode(FileWriter &Out) const {
  Out.writeU32(MergedFunctions.size());
  for (const auto &F : MergedFunctions) {
    Out.writeU32(0);
    const auto StartOffset = Out.tell();
    // Encode the FunctionInfo with no padding so later we can just read them
    // one after the other without knowing the offset in the stream for each.
    toolchain::Expected<uint64_t> result = F.encode(Out, /*NoPadding =*/true);
    if (!result)
      return result.takeError();
    const auto Length = Out.tell() - StartOffset;
    Out.fixup32(static_cast<uint32_t>(Length), StartOffset - 4);
  }
  return Error::success();
}

toolchain::Expected<MergedFunctionsInfo>
MergedFunctionsInfo::decode(DataExtractor &Data, uint64_t BaseAddr) {
  MergedFunctionsInfo MFI;
  auto FuncExtractorsOrError = MFI.getFuncsDataExtractors(Data);

  if (!FuncExtractorsOrError)
    return FuncExtractorsOrError.takeError();

  for (DataExtractor &FuncData : *FuncExtractorsOrError) {
    toolchain::Expected<FunctionInfo> FI = FunctionInfo::decode(FuncData, BaseAddr);
    if (!FI)
      return FI.takeError();
    MFI.MergedFunctions.push_back(std::move(*FI));
  }

  return MFI;
}

toolchain::Expected<std::vector<DataExtractor>>
MergedFunctionsInfo::getFuncsDataExtractors(DataExtractor &Data) {
  std::vector<DataExtractor> Results;
  uint64_t Offset = 0;

  // Ensure there is enough data to read the function count.
  if (!Data.isValidOffsetForDataOfSize(Offset, 4))
    return createStringError(
        std::errc::io_error,
        "unable to read the function count at offset 0x%8.8" PRIx64, Offset);

  uint32_t Count = Data.getU32(&Offset);

  for (uint32_t i = 0; i < Count; ++i) {
    // Ensure there is enough data to read the function size.
    if (!Data.isValidOffsetForDataOfSize(Offset, 4))
      return createStringError(
          std::errc::io_error,
          "unable to read size of function %u at offset 0x%8.8" PRIx64, i,
          Offset);

    uint32_t FnSize = Data.getU32(&Offset);

    // Ensure there is enough data for the function content.
    if (!Data.isValidOffsetForDataOfSize(Offset, FnSize))
      return createStringError(
          std::errc::io_error,
          "function data is truncated for function %u at offset 0x%8.8" PRIx64
          ", expected size %u",
          i, Offset, FnSize);

    // Extract the function data.
    Results.emplace_back(Data.getData().substr(Offset, FnSize),
                         Data.isLittleEndian(), Data.getAddressSize());

    Offset += FnSize;
  }
  return Results;
}

bool operator==(const MergedFunctionsInfo &LHS,
                const MergedFunctionsInfo &RHS) {
  return LHS.MergedFunctions == RHS.MergedFunctions;
}
