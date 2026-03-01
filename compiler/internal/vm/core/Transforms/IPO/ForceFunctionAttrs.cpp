//===- ForceFunctionAttrs.cpp - Force function attrs for debugging --------===//
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

#include "vm/core/Transforms/IPO/ForceFunctionAttrs.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/LineIterator.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

#define DEBUG_TYPE "forceattrs"

static cl::list<std::string> ForceAttributes(
    "force-attribute", cl::Hidden,
    cl::desc(
        "Add an attribute to a function. This can be a "
        "pair of 'function-name:attribute-name', to apply an attribute to a "
        "specific function. For "
        "example -force-attribute=foo:noinline. Specifying only an attribute "
        "will apply the attribute to every function in the module. This "
        "option can be specified multiple times."));

static cl::list<std::string> ForceRemoveAttributes(
    "force-remove-attribute", cl::Hidden,
    cl::desc("Remove an attribute from a function. This can be a "
             "pair of 'function-name:attribute-name' to remove an attribute "
             "from a specific function. For "
             "example -force-remove-attribute=foo:noinline. Specifying only an "
             "attribute will remove the attribute from all functions in the "
             "module. This "
             "option can be specified multiple times."));

static cl::opt<std::string> CSVFilePath(
    "forceattrs-csv-path", cl::Hidden,
    cl::desc(
        "Path to CSV file containing lines of function names and attributes to "
        "add to them in the form of `f1,attr1` or `f2,attr2=str`."));

/// If F has any forced attributes given on the command line, add them.
/// If F has any forced remove attributes given on the command line, remove
/// them. When both force and force-remove are given to a function, the latter
/// takes precedence.
static void forceAttributes(Function &F) {
  auto ParseFunctionAndAttr = [&](StringRef S) {
    StringRef AttributeText;
    if (S.contains(':')) {
      auto KV = StringRef(S).split(':');
      if (KV.first != F.getName())
        return Attribute::None;
      AttributeText = KV.second;
    } else {
      AttributeText = S;
    }
    auto Kind = Attribute::getAttrKindFromName(AttributeText);
    if (Kind == Attribute::None || !Attribute::canUseAsFnAttr(Kind)) {
      LLVM_DEBUG(dbgs() << "ForcedAttribute: " << AttributeText
                        << " unknown or not a function attribute!\n");
    }
    return Kind;
  };

  for (const auto &S : ForceAttributes) {
    auto Kind = ParseFunctionAndAttr(S);
    if (Kind == Attribute::None || F.hasFnAttribute(Kind))
      continue;
    F.addFnAttr(Kind);
  }

  for (const auto &S : ForceRemoveAttributes) {
    auto Kind = ParseFunctionAndAttr(S);
    if (Kind == Attribute::None || !F.hasFnAttribute(Kind))
      continue;
    F.removeFnAttr(Kind);
  }
}

static bool hasForceAttributes() {
  return !ForceAttributes.empty() || !ForceRemoveAttributes.empty();
}

PreservedAnalyses ForceFunctionAttrsPass::run(Module &M,
                                              ModuleAnalysisManager &) {
  bool Changed = false;
  if (!CSVFilePath.empty()) {
    auto BufferOrError = MemoryBuffer::getFileOrSTDIN(CSVFilePath);
    if (!BufferOrError) {
      std::error_code EC = BufferOrError.getError();
      M.getContext().emitError("cannot open CSV file: " + EC.message());
      return PreservedAnalyses::all();
    }

    StringRef Buffer = BufferOrError.get()->getBuffer();
    auto MemoryBuffer = MemoryBuffer::getMemBuffer(Buffer);
    line_iterator It(*MemoryBuffer);
    for (; !It.is_at_end(); ++It) {
      auto SplitPair = It->split(',');
      if (SplitPair.second.empty())
        continue;
      Function *Func = M.getFunction(SplitPair.first);
      if (Func) {
        if (Func->isDeclaration())
          continue;
        auto SecondSplitPair = SplitPair.second.split('=');
        if (!SecondSplitPair.second.empty()) {
          Func->addFnAttr(SecondSplitPair.first, SecondSplitPair.second);
          Changed = true;
        } else {
          auto AttrKind = Attribute::getAttrKindFromName(SplitPair.second);
          if (AttrKind != Attribute::None &&
              Attribute::canUseAsFnAttr(AttrKind)) {
            // TODO: There could be string attributes without a value, we should
            // support those, too.
            Func->addFnAttr(AttrKind);
            Changed = true;
          } else
            errs() << "Cannot add " << SplitPair.second
                   << " as an attribute name.\n";
        }
      } else {
        errs() << "Function in CSV file at line " << It.line_number()
               << " does not exist.\n";
        // TODO: `report_fatal_error at end of pass for missing functions.
        continue;
      }
    }
  }
  if (hasForceAttributes()) {
    for (Function &F : M.functions())
      forceAttributes(F);
    Changed = true;
  }
  // Just conservatively invalidate analyses if we've made any changes, this
  // isn't likely to be important.
  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
