//===- CallSiteInfo.cpp -----------------------------------------*- C++ -*-===//
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

#include "vm/core/DebugInfo/GSYM/CallSiteInfo.h"
#include "vm/core/DebugInfo/GSYM/FileWriter.h"
#include "vm/core/DebugInfo/GSYM/FunctionInfo.h"
#include "vm/core/DebugInfo/GSYM/GsymCreator.h"
#include "vm/core/MC/StringTableBuilder.h"
#include "vm/core/Support/DataExtractor.h"
#include "vm/core/Support/InterleavedRange.h"
#include "vm/core/Support/YAMLParser.h"
#include "vm/core/Support/YAMLTraits.h"
#include "vm/core/Support/raw_ostream.h"
#include <string>
#include <vector>

using namespace vm::core;
using namespace gsym;

Error CallSiteInfo::encode(FileWriter &O) const {
  O.writeU64(ReturnOffset);
  O.writeU8(Flags);
  O.writeU32(MatchRegex.size());
  for (uint32_t Entry : MatchRegex)
    O.writeU32(Entry);
  return Error::success();
}

Expected<CallSiteInfo> CallSiteInfo::decode(DataExtractor &Data,
                                            uint64_t &Offset) {
  CallSiteInfo CSI;

  // Read ReturnOffset
  if (!Data.isValidOffsetForDataOfSize(Offset, sizeof(uint64_t)))
    return createStringError(std::errc::io_error,
                             "0x%8.8" PRIx64 ": missing ReturnOffset", Offset);
  CSI.ReturnOffset = Data.getU64(&Offset);

  // Read Flags
  if (!Data.isValidOffsetForDataOfSize(Offset, sizeof(uint8_t)))
    return createStringError(std::errc::io_error,
                             "0x%8.8" PRIx64 ": missing Flags", Offset);
  CSI.Flags = Data.getU8(&Offset);

  // Read number of MatchRegex entries
  if (!Data.isValidOffsetForDataOfSize(Offset, sizeof(uint32_t)))
    return createStringError(std::errc::io_error,
                             "0x%8.8" PRIx64 ": missing MatchRegex count",
                             Offset);
  uint32_t NumEntries = Data.getU32(&Offset);

  CSI.MatchRegex.reserve(NumEntries);
  for (uint32_t i = 0; i < NumEntries; ++i) {
    if (!Data.isValidOffsetForDataOfSize(Offset, sizeof(uint32_t)))
      return createStringError(std::errc::io_error,
                               "0x%8.8" PRIx64 ": missing MatchRegex entry",
                               Offset);
    uint32_t Entry = Data.getU32(&Offset);
    CSI.MatchRegex.push_back(Entry);
  }

  return CSI;
}

Error CallSiteInfoCollection::encode(FileWriter &O) const {
  O.writeU32(CallSites.size());
  for (const CallSiteInfo &CSI : CallSites)
    if (Error Err = CSI.encode(O))
      return Err;

  return Error::success();
}

Expected<CallSiteInfoCollection>
CallSiteInfoCollection::decode(DataExtractor &Data) {
  CallSiteInfoCollection CSC;
  uint64_t Offset = 0;

  // Read number of CallSiteInfo entries
  if (!Data.isValidOffsetForDataOfSize(Offset, sizeof(uint32_t)))
    return createStringError(std::errc::io_error,
                             "0x%8.8" PRIx64 ": missing CallSiteInfo count",
                             Offset);
  uint32_t NumCallSites = Data.getU32(&Offset);

  CSC.CallSites.reserve(NumCallSites);
  for (uint32_t i = 0; i < NumCallSites; ++i) {
    Expected<CallSiteInfo> ECSI = CallSiteInfo::decode(Data, Offset);
    if (!ECSI)
      return ECSI.takeError();
    CSC.CallSites.emplace_back(*ECSI);
  }

  return CSC;
}

/// Structures necessary for reading CallSiteInfo from YAML.
namespace vm::core {
namespace yaml {

struct CallSiteYAML {
  // The offset of the return address of the call site - relative to the start
  // of the function.
  Hex64 return_offset;
  std::vector<std::string> match_regex;
  std::vector<std::string> flags;
};

struct FunctionYAML {
  std::string name;
  std::vector<CallSiteYAML> callsites;
};

struct FunctionsYAML {
  std::vector<FunctionYAML> functions;
};

template <> struct MappingTraits<CallSiteYAML> {
  static void mapping(IO &io, CallSiteYAML &callsite) {
    io.mapRequired("return_offset", callsite.return_offset);
    io.mapRequired("match_regex", callsite.match_regex);
    io.mapOptional("flags", callsite.flags);
  }
};

template <> struct MappingTraits<FunctionYAML> {
  static void mapping(IO &io, FunctionYAML &func) {
    io.mapRequired("name", func.name);
    io.mapOptional("callsites", func.callsites);
  }
};

template <> struct MappingTraits<FunctionsYAML> {
  static void mapping(IO &io, FunctionsYAML &FuncYAMLs) {
    io.mapRequired("functions", FuncYAMLs.functions);
  }
};

} // namespace yaml
} // namespace vm::core

LLVM_YAML_IS_SEQUENCE_VECTOR(CallSiteYAML)
LLVM_YAML_IS_SEQUENCE_VECTOR(FunctionYAML)

Error CallSiteInfoLoader::loadYAML(StringRef YAMLFile) {
  // Step 1: Read YAML file
  auto BufferOrError = MemoryBuffer::getFile(YAMLFile, /*IsText=*/true);
  if (!BufferOrError)
    return errorCodeToError(BufferOrError.getError());

  std::unique_ptr<MemoryBuffer> Buffer = std::move(*BufferOrError);

  // Step 2: Parse YAML content
  yaml::FunctionsYAML FuncsYAML;
  yaml::Input Yin(Buffer->getMemBufferRef());
  Yin >> FuncsYAML;
  if (Yin.error())
    return createStringError(Yin.error(), "Error parsing YAML file: %s\n",
                             Buffer->getBufferIdentifier().str().c_str());

  // Step 3: Build function map from Funcs
  auto FuncMap = buildFunctionMap();

  // Step 4: Process parsed YAML functions and update FuncMap
  return processYAMLFunctions(FuncsYAML, FuncMap);
}

StringMap<FunctionInfo *> CallSiteInfoLoader::buildFunctionMap() {
  // If the function name is already in the map, don't update it. This way we
  // preferentially use the first encountered function. Since symbols are
  // loaded from dSYM first, we end up preferring keeping track of symbols
  // from dSYM rather than from the symbol table - which is what we want to
  // do.
  StringMap<FunctionInfo *> FuncMap;
  for (auto &Func : Funcs) {
    FuncMap.try_emplace(GCreator.getString(Func.Name), &Func);
    if (auto &MFuncs = Func.MergedFunctions)
      for (auto &MFunc : MFuncs->MergedFunctions)
        FuncMap.try_emplace(GCreator.getString(MFunc.Name), &MFunc);
  }
  return FuncMap;
}

Error CallSiteInfoLoader::processYAMLFunctions(
    const yaml::FunctionsYAML &FuncYAMLs, StringMap<FunctionInfo *> &FuncMap) {
  // For each function in the YAML file
  for (const auto &FuncYAML : FuncYAMLs.functions) {
    auto It = FuncMap.find(FuncYAML.name);
    if (It == FuncMap.end())
      return createStringError(
          std::errc::invalid_argument,
          "Can't find function '%s' specified in callsite YAML\n",
          FuncYAML.name.c_str());

    FunctionInfo *FuncInfo = It->second;
    // Create a CallSiteInfoCollection if not already present
    if (!FuncInfo->CallSites)
      FuncInfo->CallSites = CallSiteInfoCollection();
    for (const auto &CallSiteYAML : FuncYAML.callsites) {
      CallSiteInfo CSI;
      // Since YAML has specifies relative return offsets, add the function
      // start address to make the offset absolute.
      CSI.ReturnOffset = CallSiteYAML.return_offset;
      for (const auto &Regex : CallSiteYAML.match_regex) {
        uint32_t StrOffset = GCreator.insertString(Regex);
        CSI.MatchRegex.push_back(StrOffset);
      }

      // Parse flags and combine them
      for (const auto &FlagStr : CallSiteYAML.flags) {
        if (FlagStr == "InternalCall") {
          CSI.Flags |= static_cast<uint8_t>(CallSiteInfo::InternalCall);
        } else if (FlagStr == "ExternalCall") {
          CSI.Flags |= static_cast<uint8_t>(CallSiteInfo::ExternalCall);
        } else {
          return createStringError(std::errc::invalid_argument,
                                   "Unknown flag in callsite YAML: %s\n",
                                   FlagStr.c_str());
        }
      }
      FuncInfo->CallSites->CallSites.push_back(CSI);
    }
  }
  return Error::success();
}

raw_ostream &gsym::operator<<(raw_ostream &OS, const CallSiteInfo &CSI) {
  OS << "  Return=" << HEX64(CSI.ReturnOffset);
  OS << "  Flags=" << HEX8(CSI.Flags);
  OS << "  RegEx=" << toolchain::interleaved(CSI.MatchRegex, ",");
  return OS;
}

raw_ostream &gsym::operator<<(raw_ostream &OS,
                              const CallSiteInfoCollection &CSIC) {
  for (const auto &CS : CSIC.CallSites) {
    OS << CS;
    OS << "\n";
  }
  return OS;
}
