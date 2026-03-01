/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_GVN_H
#define LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_GVN_H

#include <llvm/IR/PassManager.h>

namespace ark::llvmbackend {
struct LLVMCompilerOptions;
}  // namespace ark::llvmbackend

namespace llvm {
class DominatorTree;
}  // namespace llvm

namespace ark::llvmbackend {
class LLVMArkInterface;
}  // namespace ark::llvmbackend

namespace ark::llvmbackend::passes {

class ArkGVN : public llvm::PassInfoMixin<ArkGVN> {
public:
    explicit ArkGVN(LLVMArkInterface *arkInterface = nullptr);

    static bool ShouldInsert([[maybe_unused]] const ark::llvmbackend::LLVMCompilerOptions *options)
    {
        return true;
    }
    static ArkGVN Create(LLVMArkInterface *arkInterface, const ark::llvmbackend::LLVMCompilerOptions *options);

    // NOLINTNEXTLINE(readability-identifier-naming)
    llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &analysisManager);

private:
    enum BuiltinType {
        LOAD_CLASS,              //
        LOAD_AND_INIT_CLASS,     //
        LOAD_STRING,             //
        RESOLVE_VIRTUAL_METHOD,  //
        NONE,                    //
    };

    struct BuiltinKey {
        BuiltinType builtinTy = BuiltinType::NONE;
        llvm::SmallVector<llvm::Value *, 2U> args;
        // To store as a key in llvm::SmallDenseMap
        bool isValid = true;
    };

    struct MapInfo {
        // NOLINTNEXTLINE(readability-identifier-naming)
        static inline BuiltinKey getEmptyKey()
        {
            return BuiltinKey();
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        static inline BuiltinKey getTombstoneKey()
        {
            auto key = BuiltinKey();
            key.isValid = false;
            return key;
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        static unsigned getHashValue(const BuiltinKey &key)
        {
            return static_cast<unsigned>(llvm::hash_combine(
                llvm::hash_value(key.builtinTy), llvm::hash_combine_range(key.args.begin(), key.args.end())));
        }

        // NOLINTNEXTLINE(readability-identifier-naming)
        static bool isEqual(const BuiltinKey &a, const BuiltinKey &b)
        {
            return a.builtinTy == b.builtinTy && a.args == b.args;
        }
    };

    using LocalTable = llvm::SmallDenseMap<BuiltinKey, llvm::Value *, 4U, MapInfo>;
    using GvnBuiltins = llvm::SmallVector<llvm::Function *, BuiltinType::NONE>;

private:
    bool RunOnFunction(const llvm::DominatorTree &tree, const GvnBuiltins &builtins);
    bool RunOnBasicBlock(llvm::BasicBlock *block, const llvm::DominatorTree &tree, const GvnBuiltins &builtins);

    BuiltinKey ParseBuiltin(const llvm::CallInst *callInst, const GvnBuiltins &builtins);

    llvm::Value *FindDominantCall(const BuiltinKey &curBuiltin, llvm::BasicBlock *block,
                                  const llvm::DominatorTree &tree);

private:
    LLVMArkInterface *arkInterface_;
    std::vector<llvm::BasicBlock *> workStack_;
    llvm::DenseMap<llvm::BasicBlock *, LocalTable> bbTables_;

public:
    static constexpr llvm::StringRef ARG_NAME = "ark-gvn";
};

}  // namespace ark::llvmbackend::passes
#endif  // LIBLLVMBACKEND_TRANSFORMS_PASSES_ARK_GVN_H
