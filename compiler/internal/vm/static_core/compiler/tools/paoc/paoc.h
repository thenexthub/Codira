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

#ifndef PANDA_PAOC_H
#define PANDA_PAOC_H

#include "paoc_options.h"
#include "aot_builder/aot_builder.h"
#include "paoc_clusters.h"
#include "runtime/compiler.h"
#include "runtime/jit/profiling_loader.h"
#include "libarkbase/utils/expected.h"
#include "libarkbase/utils/span.h"

namespace ark::compiler {
class Graph;
class SharedSlowPathData;
}  // namespace ark::compiler

namespace ark::paoc {

struct SkipInfo {
    bool isFirstCompiled;
    bool isLastCompiled;
};

enum class PaocMode : uint8_t { AOT, JIT, OSR, LLVM };

class Paoc {
public:
    int Run(const ark::Span<const char *> &args);

protected:
    enum class LLVMCompilerStatus { FALLBACK = 0, COMPILED = 1, SKIP = 2, ERROR = 3 };

    // Wrapper for CompileJit(), CompileOsr() and CompileAot() arguments:
    struct CompilingContext {
        NO_COPY_SEMANTIC(CompilingContext);
        NO_MOVE_SEMANTIC(CompilingContext);
        CompilingContext(Method *methodPtr, size_t methodIndex, std::ofstream *statisticsDump);
        ~CompilingContext();
        void DumpStatistics() const;

    public:
        Method *method {nullptr};                 // NOLINT(misc-non-private-member-variables-in-classes)
        ark::ArenaAllocator allocator;            // NOLINT(misc-non-private-member-variables-in-classes)
        ark::ArenaAllocator graphLocalAllocator;  // NOLINT(misc-non-private-member-variables-in-classes)
        ark::compiler::Graph *graph {nullptr};    // NOLINT(misc-non-private-member-variables-in-classes)
        size_t index;                             // NOLINT(misc-non-private-member-variables-in-classes)
        std::ofstream *stats {nullptr};           // NOLINT(misc-non-private-member-variables-in-classes)
        bool compilationStatus {true};            // NOLINT(misc-non-private-member-variables-in-classes)
    };

    virtual std::unique_ptr<compiler::AotBuilder> CreateAotBuilder()
    {
        return std::make_unique<compiler::AotBuilder>();
    }

private:
    void RunAotMode(const ark::Span<const char *> &args);
    void StartAotFile(const panda_file::File &pfileRef);
    bool CompileFiles();
    bool TryLoadPandaFile(const std::string &fileName, PandaVM *vm);
    std::unique_ptr<const panda_file::File> TryLoadZipPandaFile(const std::string &fileName);
    bool TryLoadAotProfile();
    bool TryLoadAotFileProfile(ProfilingLoader &profilingLoader, const panda_file::File &pfileRef, size_t pfileIdx);
    bool TryLoadAotMethodProfile(ProfilingLoader &profilingLoader,
                                 PandaMap<uint32_t, ark::pgo::AotProfilingData::AotMethodProfilingData> &methodProfiles,
                                 uint32_t classId, Method &method);
    std::string BuildClassContext();
    bool CompilePandaFile(const panda_file::File &pfileRef);
    ark::Class *ResolveClass(const panda_file::File &pfileRef, panda_file::File::EntityId classId);
    bool PossibleToCompile(const panda_file::File &pfileRef, const ark::Class *klass,
                           panda_file::File::EntityId classId);
    bool Compile(Class *klass, const panda_file::File &pfileRef);

    bool Compile(Method *method, size_t methodIndex);
    bool CompileInGraph(CompilingContext *ctx, std::string methodName, bool isOsr);
    bool RunOptimizations(CompilingContext *ctx);
    bool CompileJit(CompilingContext *ctx);
    bool CompileOsr(CompilingContext *ctx);
    bool CompileAot(CompilingContext *ctx);
    void MakeAotData(CompilingContext *ctx, uintptr_t codeAddress);
    bool FinalizeCompileAot(CompilingContext *ctx, [[maybe_unused]] uintptr_t codeAddress);
    void PrintError(const std::string &error);
    void PrintUsage(const ark::PandArgParser &paParser);
    bool IsMethodShouldBeCompiled(const std::string &methodFullName);
    bool Skip(Method *method);
    static std::string GetFileLocation(const panda_file::File &pfileRef, std::string location);
    static bool CompareBootFiles(std::string filename, std::string paocLocation);
    bool LoadPandaFiles();
    bool TryCreateGraph(CompilingContext *ctx);
    void BuildClassHashTable(const panda_file::File &pfileRef);
    std::string GetFilePath(std::string fileName);

    bool IsAotMode()
    {
        return mode_ == PaocMode::AOT || mode_ == PaocMode::LLVM;
    }

    class ErrorHandler : public ClassLinkerErrorHandler {
        void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
    };

    class PaocInitializer;

protected:
    virtual void AddExtraOptions([[maybe_unused]] PandArgParser *parser) {}
    virtual void ValidateExtraOptions() {}

    ark::paoc::Options *GetPaocOptions()
    {
        return paocOptions_.get();
    }

    compiler::RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }
    ArenaAllocator *GetCodeAllocator()
    {
        return codeAllocator_;
    }

    bool ShouldIgnoreFailures();
    bool IsLLVMAotMode()
    {
        return mode_ == PaocMode::LLVM;
    }

    virtual void Clear(ark::mem::InternalAllocatorPtr allocator);
    virtual void PrepareLLVM([[maybe_unused]] const ark::Span<const char *> &args)
    {
        LOG(FATAL, COMPILER) << "--paoc-mode=llvm is not supported in this configuration";
    }
    virtual LLVMCompilerStatus TryLLVM([[maybe_unused]] CompilingContext *ctx)
    {
        UNREACHABLE();
        return LLVMCompilerStatus::FALLBACK;
    }
    virtual bool EndLLVM()
    {
        UNREACHABLE();
    }

protected:
    std::unique_ptr<compiler::AotBuilder> aotBuilder_;  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    std::unique_ptr<ark::RuntimeOptions> runtimeOptions_ {nullptr};
    std::unique_ptr<ark::paoc::Options> paocOptions_ {nullptr};

    compiler::RuntimeInterface *runtime_ {nullptr};

    PaocMode mode_ {PaocMode::AOT};
    ClassLinker *loader_ {nullptr};
    ArenaAllocator *codeAllocator_ {nullptr};
    std::set<std::string> methodsList_;
    std::unordered_map<std::string, std::string> locationMapping_;
    std::unordered_map<std::string, const panda_file::File *> preloadedFiles_;
    size_t compilationIndex_ {0};
    SkipInfo skipInfo_ {false, false};

    PaocClusters clustersInfo_;
    compiler::SharedSlowPathData *slowPathData_ {nullptr};

    unsigned successMethods_ {0};
    unsigned failedMethods_ {0};

    std::ofstream statisticsDump_;
};

}  // namespace ark::paoc
#endif  // PANDA_PAOC_H
