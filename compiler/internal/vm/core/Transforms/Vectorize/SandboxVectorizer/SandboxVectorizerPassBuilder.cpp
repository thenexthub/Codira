#include "vm/core/Transforms/Vectorize/SandboxVectorizer/SandboxVectorizerPassBuilder.h"

#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/BottomUpVec.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/NullPass.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/PackReuse.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/PrintInstructionCount.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/PrintRegion.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/RegionsFromBBs.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/RegionsFromMetadata.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/SeedCollection.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/TransactionAcceptOrRevert.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/TransactionAlwaysAccept.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/TransactionAlwaysRevert.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/TransactionSave.h"

namespace vm::core::sandboxir {

std::unique_ptr<sandboxir::RegionPass>
SandboxVectorizerPassBuilder::createRegionPass(StringRef Name, StringRef Args) {
#define REGION_PASS(NAME, CLASS_NAME)                                          \
  if (Name == NAME) {                                                          \
    assert(Args.empty() && "Unexpected arguments for pass '" NAME "'.");       \
    return std::make_unique<CLASS_NAME>();                                     \
  }
// TODO: Support region passes with params.
#include "Passes/PassRegistry.def"
  return nullptr;
}

std::unique_ptr<sandboxir::FunctionPass>
SandboxVectorizerPassBuilder::createFunctionPass(StringRef Name,
                                                 StringRef Args) {
#define FUNCTION_PASS_WITH_PARAMS(NAME, CLASS_NAME)                            \
  if (Name == NAME)                                                            \
    return std::make_unique<CLASS_NAME>(Args);
#include "Passes/PassRegistry.def"
  return nullptr;
}

} // namespace vm::core::sandboxir
