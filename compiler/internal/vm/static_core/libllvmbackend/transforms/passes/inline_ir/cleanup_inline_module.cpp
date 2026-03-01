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

#include "cleanup_inline_module.h"

#include "llvm_ark_interface.h"
#include "inline_ir_utils.h"
#include "llvm_compiler_options.h"

#include <sstream>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <llvm/Transforms/Utils/FunctionImportUtils.h>

#include "transforms/transform_utils.h"

#define DEBUG_TYPE "cleanup-inline-module"

using llvm::Argument;
using llvm::BasicBlock;
using llvm::cast;
using llvm::Constant;
using llvm::convertToDeclaration;
using llvm::DenseMap;
using llvm::DenseSet;
using llvm::for_each;
using llvm::Function;
using llvm::GlobalVariable;
using llvm::InlineAsm;
using llvm::isa;
using llvm::ListSeparator;
using llvm::Module;
using llvm::SmallVector;
using llvm::SmallVectorImpl;
using llvm::StringRef;
using llvm::User;
using llvm::Value;

namespace {

/**
 * 1. Inserts a given element to a vector
 * 2. Removes the last element from the vector
 * 3. Checks if the last element was the initially given element
 *
 * Used to track a dfs path
 *
 * @tparam T type of vector elements
 */
template <typename T>
class ScopedVectorElement final {
public:
    explicit ScopedVectorElement(llvm::SmallVectorImpl<T> *vector, T value) : vector_(vector), value_(value)
    {
        ASSERT(vector != nullptr);
        ASSERT(value != nullptr);
        vector->push_back(value);
    }

    ~ScopedVectorElement()
    {
        [[maybe_unused]] auto value = vector_->pop_back_val();
        ASSERT(value == value_);
    }

    ScopedVectorElement(const ScopedVectorElement &) = delete;
    ScopedVectorElement &operator=(const ScopedVectorElement &) = delete;
    ScopedVectorElement(ScopedVectorElement &&) = delete;
    ScopedVectorElement &operator=(ScopedVectorElement &&) = delete;

private:
    SmallVectorImpl<T> *vector_;
    T value_;
};

using InlineFailurePath = SmallVector<Value *, 4U>;
using ScopedInlineFailurePathElement = ScopedVectorElement<InlineFailurePath::value_type>;

enum class InlineFailureReason {
    HAS_ADDRESS_TAKEN,
    USES_INTERNAL_VARIABLE,
};

class InlineFailure {
public:
    InlineFailure(InlineFailurePath inlineFailurePath, InlineFailureReason reason)
        : inlineFailurePath_(std::move(inlineFailurePath)), reason_(reason)
    {
    }

    const InlineFailurePath &GetInlineFailurePath() const
    {
        return inlineFailurePath_;
    }

    InlineFailureReason GetReason() const
    {
        return reason_;
    }

    void Print(llvm::raw_ostream *output)
    {
        ASSERT(output != nullptr);

        if (reason_ == InlineFailureReason::HAS_ADDRESS_TAKEN) {
            PrintForHasAddressTaken(output);
        } else if (reason_ == InlineFailureReason::USES_INTERNAL_VARIABLE) {
            PrintForUsesInternalVariable(output);
        } else {
            LLVM_DEBUG(llvm::dbgs() << "static_cast<int>(reason_) = " << static_cast<int>(reason_) << "\n");
            llvm_unreachable("Unsupported reason");
        }
    }

private:
    void PrintForHasAddressTaken(llvm::raw_ostream *output)
    {
        ASSERT(!inlineFailurePath_.empty());
        ASSERT(reason_ == InlineFailureReason::HAS_ADDRESS_TAKEN);
        auto function = inlineFailurePath_[inlineFailurePath_.size() - 1];
        ASSERT(isa<Function>(function));
        *output << "address of the function = '" << function->getName() << "' is taken. Won't inline chain: ";
        PrintPath(output);
    }

    void PrintForUsesInternalVariable(llvm::raw_ostream *output)
    {
        ASSERT(reason_ == InlineFailureReason::USES_INTERNAL_VARIABLE);
        ASSERT(inlineFailurePath_.size() >= 2U);
        // inline_failure_path_ = [..., functionUsingInternalVariable, ...not function..., internalVariable]
        auto variable = inlineFailurePath_[inlineFailurePath_.size() - 1];
        auto function = FindLastFunctionInPath();
        *output << "Internal variable = '" << variable->getName() << "' is used in function = '" << function->getName()
                << "'. Won't inline chain: ";
        PrintPath(output);
    }

    Function *FindLastFunctionInPath()
    {
        for (auto it = inlineFailurePath_.rbegin(); it != inlineFailurePath_.rend(); it++) {
            if (auto function = llvm::dyn_cast<Function>(*it)) {
                return function;
            }
        }
        LLVM_DEBUG(llvm::dbgs() << "inline_failure_path_ = ");
        LLVM_DEBUG(PrintPath(&llvm::dbgs()));
        LLVM_DEBUG(llvm::dbgs() << "\n");
        llvm_unreachable("Could not find function in inline_failure_path_");
    }

    void PrintPath(llvm::raw_ostream *output)
    {
        ListSeparator separator(" -> ");
        for (const auto &pathElement : inlineFailurePath_) {
            *output << separator << "'" << pathElement->getName() << "'";
        }
    }

private:
    InlineFailurePath inlineFailurePath_;
    InlineFailureReason reason_;
};
}  // namespace

namespace ark::llvmbackend::passes {

/**
 * Remove functions and variables unsuitable for inlining from module.
 *
 * Function is unsuitable for inlining if:
 *
 * 1. It has local linkage and address taken
 * 2. Function does not have external linkage, references variable with local linkage and the variable is not a
 * constant. Examples:
 *     1. Function has 'static int x' in its body
 *     2. Function assigns a value to a 'static thread_local'
 */
class CleanupInlineModule::InlineModuleCleaner {
public:
    bool Run(Module &module)
    {
        RemoveNonInlinableFunctions(module);
        RemoveObjectFileGlobals(module);
        RemoveDanglingAliases(module);
        return true;
    }

private:
    enum class FunctionState {
        UNKNOWN,        // white
        IN_PROGRESS,    // gray
        NOT_INLINABLE,  // black
        INLINABLE,      // black
    };

    using DfsState = DenseMap<Function *, FunctionState>;

    void RemoveNonInlinableFunctions(Module &module)
    {
        for (auto &function : module.functions()) {
            if (function.isDeclaration()) {
                continue;
            }

            ScopedInlineFailurePathElement inlineFailurePathElement {&inlineFailurePath_, &function};
            VisitFunction(&function);
        }
        ASSERT(inlineFailurePath_.empty());

        for_each(state_, [](auto entry) -> void {
            auto functionState = entry.getSecond();
            ASSERT(functionState == FunctionState::INLINABLE || functionState == FunctionState::NOT_INLINABLE);
            if (functionState == FunctionState::NOT_INLINABLE) {
                auto function = entry.getFirst();
                LLVM_DEBUG(llvm::dbgs() << "InlineModuleCleaner: removed '" << function->getName() << "'\n");
                convertToDeclaration(*function);
            }
        });
        LLVM_DEBUG(PrintInlineReport(&llvm::errs()));
    }

    /**
     * Visit the function during depth first search.
     *
     * After returning the dfs_state_ will contain either:
     * 1. FunctionState::NOT_INLINABLE
     * 2. FunctionState::INLINABLE
     * for the function.
     */
    void VisitFunction(Function *function)
    {
        static_assert(FunctionState() == FunctionState::UNKNOWN, "FunctionState::UNKNOWN must be the default value");
        auto functionState = state_.lookup(function);
        if (functionState != FunctionState::UNKNOWN) {
            return;
        }
        if (function->hasLocalLinkage() && function->hasAddressTaken()) {
            state_[function] = FunctionState::NOT_INLINABLE;
            ReportInlineFailure(InlineFailureReason::HAS_ADDRESS_TAKEN, inlineFailurePath_);
            return;
        }

        state_.insert({function, FunctionState::IN_PROGRESS});

        DenseSet<Value *> visited;
        for (auto &basicBlock : *function) {
            if (!IsInlinable(&basicBlock, visited)) {
                state_[function] = FunctionState::NOT_INLINABLE;
                return;
            }
        }
        state_[function] = FunctionState::INLINABLE;
    }

    bool IsInlinable(Value *value, DenseSet<Value *> &visited)
    {
        ScopedInlineFailurePathElement inlineFailurePathElement {&inlineFailurePath_, value};
        if (visited.contains(value)) {
            return true;
        }
        visited.insert(value);
        if (isa<Function>(value)) {
            auto function = cast<Function>(value);
            /**
             * The value operand is a function with external linkage.
             *
             * The function's operand from VisitFunction is inlinable because we'll either:
             * 1. Inline the operand itself
             * 2. Leave a reference to external function, which linker will resolve
             *
             * Example:
             *
             * @code
             * extern void foo();
             *
             * void bar() {
             *     foo(); // We're here: the 'bar' function has external linkage
             * }
             * @endcode
             *
             * Call stack might be:
             *
             * @code
             * 1. IsInlinable(foo);
             * 2. VisitFunction(bar);
             * @endcode
             */
            if (function->hasExternalLinkage()) {
                return true;
            }

            /**
             * Function does not have external linkage, then we inline it if the function itself is inlinable.
             *
             * Example:
             *
             * @code
             * static void foo() {
             * }
             *
             * void bar() {
             *   foo(); // We're here, 'foo' function does not have external linkage
             * }
             * @endcode
             *
             * Call stack might be:
             *
             * @code
             * 1. IsInlinable(foo);
             * 2. VisitFunction(bar);
             * @endcode
             */
            VisitFunction(function);
            auto functionState = state_.lookup(function);
            return functionState == FunctionState::IN_PROGRESS || functionState == FunctionState::INLINABLE;
        }
        if (isa<GlobalVariable>(value)) {
            auto globalVariable = cast<GlobalVariable>(value);
            /**
             * Could be a constant from C++ declared in header.
             * We don't check for taken address because in different translation units address of such constant
             * could be different
             *
             * https://stackoverflow.com/a/50489130
             * > constexpr implies const and const on global/namespace scope implies static (internal linkage),
             * > which means that every translation unit including this header gets its own copy of PI. The memory
             * > for that static is only going to be allocated if an address or reference to it is taken, and the
             * > address is going to be different in each translation unit.
             */
            if (globalVariable->hasLocalLinkage() && !globalVariable->isConstant()) {
                /**
                 * Mutable variable with local linkage, can't inline.
                 *
                 * Example:
                 *
                 * @code
                 * void foo() {
                 *   static int x = 0;
                 *   std::cout << x++ << std::endl;
                 * }
                 * @endcode
                 */
                ReportInlineFailure(InlineFailureReason::USES_INTERNAL_VARIABLE, inlineFailurePath_);
                return false;
            }
            return true;
        }
        if (isa<User>(value)) {
            auto user = cast<User>(value);
            for (auto operand : user->operand_values()) {
                if (!IsInlinable(operand, visited)) {
                    return false;
                }
            }
            return true;
        }
        if (isa<BasicBlock>(value)) {
            for (auto &instruction : *cast<BasicBlock>(value)) {
                if (!IsInlinable(&instruction, visited)) {
                    return false;
                }
            }
            return true;
        }
        if (isa<Argument, InlineAsm, Constant, llvm::MetadataAsValue>(value)) {
            return true;
        }
        LLVM_DEBUG(llvm::dbgs() << "Value = ");
        LLVM_DEBUG(value->print(llvm::dbgs()));
        LLVM_DEBUG(llvm::dbgs() << "\n");
        llvm_unreachable("Unexpected value");
    }

    void ReportInlineFailure([[maybe_unused]] InlineFailureReason inlineFailureReason,
                             [[maybe_unused]] const InlineFailurePath &failurePath)
    {
        LLVM_DEBUG(InlineFailure(failurePath, inlineFailureReason).Print(&llvm::dbgs()));
        LLVM_DEBUG(llvm::dbgs() << "\n");
    }

    void PrintInlineReport(llvm::raw_ostream *output) const
    {
        std::vector<DfsState::value_type> entries {state_.begin(), state_.end()};
        std::sort(entries.begin(), entries.end(), [](auto a, auto b) {
            auto aState = a.getSecond();
            auto bState = b.getSecond();
            ASSERT(aState == FunctionState::INLINABLE || aState == FunctionState::NOT_INLINABLE);
            ASSERT(bState == FunctionState::INLINABLE || bState == FunctionState::NOT_INLINABLE);
            if (aState != bState) {
                return aState == FunctionState::NOT_INLINABLE;
            }
            auto aFunction = a.getFirst();
            auto bFunction = b.getFirst();
            return aFunction->getName() < bFunction->getName();
        });
        for (size_t i = 0; i < entries.size(); i++) {
            auto entry = entries[i];
            auto function = entry.getFirst();
            auto functionState = entry.getSecond();
            ASSERT(functionState == FunctionState::INLINABLE || functionState == FunctionState::NOT_INLINABLE);
            *output << (i + 1) << ". '" << function->getName() << "' is"
                    << (functionState == FunctionState::INLINABLE ? " inlinable" : " not inlinable") << "\n";
        }
    }

    void RemoveObjectFileGlobals(const Module &module) const
    {
        static constexpr std::array VALUES_TO_ERASE = {
            StringRef("llvm.used"),                //
            StringRef("llvm.compiler.used"),       //
            StringRef("llvm.global_ctors"),        //
            StringRef("llvm.global_dtors"),        //
            StringRef("llvm.global.annotations"),  //
        };
        for (const auto &name : VALUES_TO_ERASE) {
            auto *globalValue = module.getNamedValue(name);
            if (globalValue != nullptr) {
                LLVM_DEBUG(llvm::dbgs() << "Erase " << globalValue->getName() << "\n");
                globalValue->eraseFromParent();
            }
        }
    }

private:
    DfsState state_;
    InlineFailurePath inlineFailurePath_;
};

bool CleanupInlineModule::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->doIrtocInline;
}

CleanupInlineModule::CleanupInlineModule() : cleaner_ {std::make_unique<InlineModuleCleaner>()} {}

CleanupInlineModule::CleanupInlineModule(CleanupInlineModule &&) = default;

CleanupInlineModule &CleanupInlineModule::operator=(CleanupInlineModule &&) = default;

CleanupInlineModule::~CleanupInlineModule() = default;

llvm::PreservedAnalyses CleanupInlineModule::run(llvm::Module &module, llvm::ModuleAnalysisManager & /*AM*/)
{
    auto changed = cleaner_->Run(module);
    return changed ? llvm::PreservedAnalyses::none() : llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
