//===--- LLVMEmitAsyncEntryReturnMetadata.cpp - Async function metadata ---===//
//

#include "language/LLVMPasses/Passes.h"
#include "toolchain/Pass.h"
#include "toolchain/IR/Constants.h"
#include "toolchain/IR/Module.h"
#include "toolchain/Transforms/Utils/ModuleUtils.h"

using namespace toolchain;
using namespace language;

#define DEBUG_TYPE "language-async-return"

PreservedAnalyses AsyncEntryReturnMetadataPass::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  bool changed = false;

  SmallVector<toolchain::Function *, 16> asyncEntries;
  SmallVector<toolchain::Function *, 16> asyncReturns;
  for (auto &F : M) {
    if (F.isDeclaration())
      continue;

    if (F.hasFnAttribute("async_entry"))
      asyncEntries.push_back(&F);
    if (F.hasFnAttribute("async_ret"))
      asyncReturns.push_back(&F);
  }

  auto &ctxt = M.getContext();
  auto int32Ty = toolchain::Type::getInt32Ty(ctxt);
  auto sizeTy = M.getDataLayout().getIntPtrType(ctxt, /*addrspace*/ 0);

  auto addSection = [&] (const char * sectionName, const char *globalName,
                         SmallVectorImpl<toolchain::Function *> & entries) {
    if (entries.empty())
      return;

    auto intArrayTy = toolchain::ArrayType::get(int32Ty, entries.size());
    auto global =
      new toolchain::GlobalVariable(M, intArrayTy, true,
                               toolchain::GlobalValue::InternalLinkage,
                               nullptr, /*init*/ globalName,
                               nullptr, /*insertBefore*/
                               toolchain::GlobalValue::NotThreadLocal,
                               0/*address space*/);
    global->setAlignment(Align(4));
    global->setSection(sectionName);
    size_t index = 0;
    SmallVector<toolchain::Constant*, 16> offsets;
    for (auto *fn : entries) {
      toolchain::Constant *indices[] = { toolchain::ConstantInt::get(int32Ty, 0),
        toolchain::ConstantInt::get(int32Ty, index)};
      ++index;

      toolchain::Constant *base = toolchain::ConstantExpr::getInBoundsGetElementPtr(
       intArrayTy, global, indices);
      base = toolchain::ConstantExpr::getPtrToInt(base, sizeTy);
      auto *target = toolchain::ConstantExpr::getPtrToInt(fn, sizeTy);
      toolchain::Constant *offset = toolchain::ConstantExpr::getSub(target, base);

      if (sizeTy != int32Ty) {
        offset = toolchain::ConstantExpr::getTrunc(offset, int32Ty);
      }
      offsets.push_back(offset);
    }
    auto constant = toolchain::ConstantArray::get(intArrayTy, offsets);
    global->setInitializer(constant);
    appendToUsed(M, global);

    toolchain::GlobalVariable::SanitizerMetadata Meta;
    Meta.IsDynInit = false;
    Meta.NoAddress = true;
    global->setSanitizerMetadata(Meta);

    changed = true;
  };

  addSection("__TEXT,__language_as_entry, coalesced, no_dead_strip",
             "__language_async_entry_functlets",
             asyncEntries);
  addSection("__TEXT,__language_as_ret, coalesced, no_dead_strip",
             "__language_async_ret_functlets",
             asyncReturns);

  if (!changed)
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
