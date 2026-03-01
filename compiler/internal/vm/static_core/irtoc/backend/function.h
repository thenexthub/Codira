/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef PANDA_FUNCTION_H
#define PANDA_FUNCTION_H

#include "compiler/optimizer/ir/ir_constructor.h"
#include "compiler/optimizer/code_generator/relocations.h"
#include "libarkbase/utils/expected.h"
#include "source_languages.h"
#include "irtoc_options.h"

#ifdef PANDA_LLVM_IRTOC
#include "llvm_compiler_creator.h"
#include "llvm_options.h"
#endif  // PANDA_LLVM_IRTOC

namespace ark::irtoc {

enum class CompilationResult {
    INVALID,
    // This unit was compiled by Ark's default compiler because it must be compiled by Ark, not by llvm.
    // It happens for the following reasons:
    // 1. PANDA_LLVM_BACKEND is disabled
    // 2. Current function is an interpreter handler for Ark's compiler (no "_LLVM" suffix)
    // 3. Current function is FastPath, but PANDA_LLVM_FASTPATH is disabled
    // 4. Irtoc Unit tests are compiled
    ARK,
    // Successfully compiled by llvm
    LLVM,
    // llvm_compiler->CanCompile had returned false, and we fell back to Ark's default compiler
    ARK_BECAUSE_FALLBACK,
    // See SKIPPED_HANDLERS and SKIPPED_FASTPATHS
    ARK_BECAUSE_SKIP
};

#if USE_VIXL_ARM64 && !PANDA_MINIMAL_VIXL && PANDA_LLVM_INTERPRETER
#define LLVM_INTERPRETER_CHECK_REGS_MASK
#endif

#ifdef LLVM_INTERPRETER_CHECK_REGS_MASK
struct UsedRegisters {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    RegMask gpr;
    VRegMask fp;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    constexpr UsedRegisters &operator|=(UsedRegisters that)
    {
        gpr |= that.gpr;
        fp |= that.fp;
        return *this;
    }
};
#endif

class Function : public compiler::RelocationHandler {
public:
    using Result = Expected<CompilationResult, const char *>;

#ifdef PANDA_LLVM_IRTOC
    Function() : llvmCompiler_(nullptr) {}
#else
    Function() = default;
#endif
    NO_COPY_SEMANTIC(Function);
    NO_MOVE_SEMANTIC(Function);

    virtual ~Function() = default;

    virtual void MakeGraphImpl() = 0;
    virtual const char *GetName() const = 0;

    Result Compile(Arch arch, ArenaAllocator *allocator, ArenaAllocator *localAllocator);

    auto GetCode() const
    {
        return Span(code_);
    }

    compiler::Graph *GetGraph()
    {
        return graph_;
    }

    const compiler::Graph *GetGraph() const
    {
        return graph_;
    }

    size_t WordSize() const
    {
        return PointerSize(GetArch());
    }

    void AddRelocation(const compiler::RelocationInfo &info) override;

    const auto &GetRelocations() const
    {
        return relocationEntries_;
    }

    const char *GetExternalFunction(size_t index) const
    {
        CHECK_LT(index, externalFunctions_.size());
        return externalFunctions_[index].c_str();
    }

    SourceLanguage GetLanguage() const
    {
        return lang_;
    }

    uint32_t AddSourceDir(std::string_view dir)
    {
        auto it = std::find(sourceDirs_.begin(), sourceDirs_.end(), dir);
        if (it != sourceDirs_.end()) {
            return std::distance(sourceDirs_.begin(), it);
        }
        sourceDirs_.emplace_back(dir);
        return sourceDirs_.size() - 1;
    }

    uint32_t AddSourceFile(std::string_view filename)
    {
        auto it = std::find(sourceFiles_.begin(), sourceFiles_.end(), filename);
        if (it != sourceFiles_.end()) {
            return std::distance(sourceFiles_.begin(), it);
        }
        sourceFiles_.emplace_back(filename);
        return sourceFiles_.size() - 1;
    }

    uint32_t GetSourceFileIndex(const char *filename) const
    {
        auto it = std::find(sourceFiles_.begin(), sourceFiles_.end(), filename);
        ASSERT(it != sourceFiles_.end());
        return std::distance(sourceFiles_.begin(), it);
    }

    uint32_t GetSourceDirIndex(const char *dir) const
    {
        auto it = std::find(sourceDirs_.begin(), sourceDirs_.end(), dir);
        ASSERT(it != sourceDirs_.end());
        return std::distance(sourceDirs_.begin(), it);
    }

    Span<const std::string> GetSourceFiles() const
    {
        return Span<const std::string>(sourceFiles_);
    }

    Span<const std::string> GetSourceDirs() const
    {
        return Span<const std::string>(sourceDirs_);
    }

    void SetArgsCount(size_t argsCount)
    {
        argsCount_ = argsCount;
    }

    size_t GetArgsCount() const
    {
        return GetArch() != Arch::AARCH32 ? argsCount_ : 0U;
    }

#ifdef PANDA_LLVM_IRTOC
    void ReportCompilationStatistic(std::ostream *out);

    void SetLLVMCompiler(std::shared_ptr<llvmbackend::CompilerInterface> compiler)
    {
        llvmCompiler_ = std::move(compiler);
    }

#endif  // PANDA_LLVM_IRTOC

    bool IsCompiledByLlvm() const
    {
        return compilationResult_ == CompilationResult::LLVM;
    }

    CompilationResult GetCompilationResult()
    {
        return compilationResult_;
    }

    size_t GetCodeSize()
    {
        return code_.size();
    }

    void SetCode(Span<uint8_t> code);

protected:
    Arch GetArch() const
    {
        return GetGraph()->GetArch();
    }

    compiler::RuntimeInterface *GetRuntime()
    {
        return GetGraph()->GetRuntime();
    }

    void SetExternalFunctions(std::initializer_list<std::string> funcs)
    {
        externalFunctions_ = funcs;
    }

    void SetLanguage(SourceLanguage lang)
    {
        lang_ = lang;
    }

    Result RunOptimizations();

    CompilationResult CompileByLLVM();

#ifdef PANDA_LLVM_IRTOC
    bool SkippedByLLVM();

    std::string_view GraphModeToString();

    std::string_view LLVMCompilationResultToString() const;
#endif  // PANDA_LLVM_IRTOC

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::unique_ptr<compiler::IrConstructor> builder_;

private:
    /**
     * Suffix for compilation unit names to compile by llvm.
     *
     * --interpreter-type option in Ark supports interpreters produced by stock irtoc and llvm irtoc.
     * To support interpreter compiled by stock irtoc and llvm irtoc we:
     * 1. Copy a graph for a handler with "_LLVM" suffix. For example, "ClassResolver" becomes "ClassResolver_LLVM"
     * 2. Compile handlers with suffix by LLVMIrtocCompiler
     * 3. Setup dispatch table to invoke handlers with "_LLVM" suffix at runtime in SetupLLVMDispatchTableImpl
     */
    static constexpr std::string_view LLVM_SUFFIX = "_LLVM";

    compiler::Graph *graph_ {nullptr};
    SourceLanguage lang_ {SourceLanguage::PANDA_ASSEMBLY};
    std::vector<uint8_t> code_;
    std::vector<std::string> externalFunctions_;
    std::vector<std::string> sourceDirs_;
    std::vector<std::string> sourceFiles_;
    std::vector<compiler::RelocationInfo> relocationEntries_;
    size_t argsCount_ {0U};
    CompilationResult compilationResult_ {CompilationResult::INVALID};
#ifdef PANDA_LLVM_IRTOC
    std::shared_ptr<llvmbackend::CompilerInterface> llvmCompiler_ {nullptr};
#ifdef PANDA_LLVM_INTERPRETER
    static constexpr std::array SKIPPED_HANDLERS = {
        "ExecuteImplFast",
        "ExecuteImplFastEH",
    };
#endif  // PANDA_LLVM_INTERPRETER
#ifdef PANDA_LLVM_FASTPATH
    static constexpr std::array SKIPPED_FASTPATHS = {
        "StringHashCode",            // LLVM-15 generates non-optimal code, MAdd cost is not accurate.
        "StringHashCodeCompressed",  //
    };

    // The enumeration of functions cannot be compiled with LLVM because the LLVM backend doesn't support
    // the llvm::Intrinsic::aarch64_fjcvtzs intrinsic lowering yet.
    static constexpr std::array SKIPPED_FASTPATHS_JSCVT = {
        "CreateStringFromCharCodeTlab",
        "CreateStringFromCharCodeSingleTlab",
        "CreateStringFromCharCodeSingleNoCacheTlab",
    };
#endif  // PANDA_LLVM_FASTPATH
#endif  // PANDA_LLVM_IRTOC
};

}  // namespace ark::irtoc

#endif  // PANDA_FUNCTION_H
