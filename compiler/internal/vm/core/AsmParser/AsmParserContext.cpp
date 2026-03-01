//===----------------------------------------------------------------------===//
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

#include "vm/core/AsmParser/AsmParserContext.h"

namespace vm::core {

std::optional<FileLocRange>
AsmParserContext::getFunctionLocation(const Function *F) const {
  if (auto FIt = Functions.find(F); FIt != Functions.end())
    return FIt->second;
  return std::nullopt;
}

std::optional<FileLocRange>
AsmParserContext::getBlockLocation(const BasicBlock *BB) const {
  if (auto BBIt = Blocks.find(BB); BBIt != Blocks.end())
    return BBIt->second;
  return std::nullopt;
}

std::optional<FileLocRange>
AsmParserContext::getInstructionLocation(const Instruction *I) const {
  if (auto IIt = Instructions.find(I); IIt != Instructions.end())
    return IIt->second;
  return std::nullopt;
}

Function *
AsmParserContext::getFunctionAtLocation(const FileLocRange &Query) const {
  auto It = FunctionsInverse.find(Query.Start);
  if (It.stop() <= Query.End)
    return *It;
  return nullptr;
}

Function *AsmParserContext::getFunctionAtLocation(const FileLoc &Query) const {
  return FunctionsInverse.lookup(Query, nullptr);
}

BasicBlock *
AsmParserContext::getBlockAtLocation(const FileLocRange &Query) const {
  auto It = BlocksInverse.find(Query.Start);
  if (It.stop() <= Query.End)
    return *It;
  return nullptr;
}

BasicBlock *AsmParserContext::getBlockAtLocation(const FileLoc &Query) const {
  return BlocksInverse.lookup(Query, nullptr);
}

Instruction *
AsmParserContext::getInstructionAtLocation(const FileLocRange &Query) const {
  auto It = InstructionsInverse.find(Query.Start);
  if (It.stop() <= Query.End)
    return *It;
  return nullptr;
}

Instruction *
AsmParserContext::getInstructionAtLocation(const FileLoc &Query) const {
  return InstructionsInverse.lookup(Query, nullptr);
}

bool AsmParserContext::addFunctionLocation(Function *F,
                                           const FileLocRange &Loc) {
  bool Inserted = Functions.insert({F, Loc}).second;
  if (Inserted)
    FunctionsInverse.insert(Loc.Start, Loc.End, F);
  return Inserted;
}

bool AsmParserContext::addBlockLocation(BasicBlock *BB,
                                        const FileLocRange &Loc) {
  bool Inserted = Blocks.insert({BB, Loc}).second;
  if (Inserted)
    BlocksInverse.insert(Loc.Start, Loc.End, BB);
  return Inserted;
}

bool AsmParserContext::addInstructionLocation(Instruction *I,
                                              const FileLocRange &Loc) {
  bool Inserted = Instructions.insert({I, Loc}).second;
  if (Inserted)
    InstructionsInverse.insert(Loc.Start, Loc.End, I);
  return Inserted;
}

} // namespace vm::core
