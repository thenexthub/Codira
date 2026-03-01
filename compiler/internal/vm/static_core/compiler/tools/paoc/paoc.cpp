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

#include "libarkfile/class_data_accessor.h"
#include "paoc_options.h"
#include "aot/compiled_method.h"
#include "paoc_clusters.h"
#include "compiler.h"
#include "compiler_options.h"
#include "compiler_events_gen.h"
#include "compiler_logger.h"
#include "libarkbase/events/events.h"
#include "include/runtime.h"
#include "mem/gc/gc_types.h"
#include "optimizer_run.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "libarkbase/os/filesystem.h"
#include "libarkbase/panda_gen_options/generated/logger_options.h"

#include "paoc.h"
#ifdef PANDA_LLVM_AOT
#include "paoc_llvm.h"
#endif

using namespace ark::compiler;  // NOLINT(*-build-using-namespace)

namespace ark::paoc {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_PAOC(level) LOG(level, COMPILER) << "PAOC: "

Paoc::CompilingContext::CompilingContext(Method *methodPtr, size_t methodIndex, std::ofstream *statisticsDump)
    : method(methodPtr),
      allocator(ark::SpaceType::SPACE_TYPE_COMPILER, PandaVM::GetCurrent()->GetMemStats(), true),
      graphLocalAllocator(ark::SpaceType::SPACE_TYPE_COMPILER, PandaVM::GetCurrent()->GetMemStats(), true),
      index(methodIndex),
      stats(statisticsDump)
{
}

Paoc::CompilingContext::~CompilingContext()
{
    if (graph != nullptr) {
        graph->~Graph();
    }
    ASSERT(stats != nullptr);
    if (compilationStatus && *stats) {
        DumpStatistics();
    }
}
void Paoc::CompilingContext::DumpStatistics() const
{
    ASSERT(stats);
    char sep = ',';
    *stats << method->GetFullName() << sep;
    *stats << "paoc-summary" << sep;
    *stats << allocator.GetAllocatedSize() << sep;
    *stats << graphLocalAllocator.GetAllocatedSize() << '\n';
}

/// A class that encloses paoc's initialization:
class Paoc::PaocInitializer {
public:
    explicit PaocInitializer(Paoc *paoc) : paoc_(paoc) {}
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    int Init(const ark::Span<const char *> &args)
    {
        ASSERT(args.Data() != nullptr);
        ark::PandArgParser paParser;

        paoc_->runtimeOptions_ = std::make_unique<decltype(paoc_->runtimeOptions_)::element_type>(args[0]);
        paoc_->runtimeOptions_->AddOptions(&paParser);
        paoc_->paocOptions_ = std::make_unique<decltype(paoc_->paocOptions_)::element_type>(args[0]);
        paoc_->paocOptions_->AddOptions(&paParser);
        ark::compiler::g_options.AddOptions(&paParser);

        logger::Options loggerOptions("");
        loggerOptions.AddOptions((&paParser));

        paoc_->AddExtraOptions(&paParser);

        if (!paParser.Parse(args.Size(), args.Data())) {
            std::cerr << "Error: " << paParser.GetErrorString() << "\n";
            return -1;
        }
        Logger::Initialize(loggerOptions);
        if (paoc_->paocOptions_->GetPaocPandaFiles().empty()) {
            paoc_->PrintUsage(paParser);
            return 1;
        }

        const std::string parentDir = os::GetParentDir(paoc_->paocOptions_->GetPaocOutput());
        if (!os::IsDirExists(parentDir) && !paoc_->paocOptions_->GetPaocZipPandaFile().empty()) {
            os::CreateDirectories(parentDir);
        }

        if (!os::IsDirExists(parentDir)) {
            std::cerr << "Error: directory for output file \"" << paoc_->paocOptions_->GetPaocOutput()
                      << "\" doesn't exist "
                      << "\n";
            return -1;
        }
        if (InitRuntime() != 0) {
            return -1;
        }
        if (InitCompiler() != 0) {
            return -1;
        }
        if (paoc_->paocOptions_->WasSetPaocClusters() && !InitPaocClusters(&paParser)) {
            return -1;
        }
        if (paoc_->IsLLVMAotMode()) {
            paoc_->PrepareLLVM(args);
        }

        return 0;
    }

private:
    int InitRuntime()
    {
        auto runtimeOptionsErr = paoc_->runtimeOptions_->Validate();
        if (runtimeOptionsErr) {
            std::cerr << "Error: " << runtimeOptionsErr.value().GetMessage() << std::endl;
            return 1;
        }
#ifndef PANDA_ASAN_ON
        if (!paoc_->runtimeOptions_->WasSetInternalMemorySizeLimit()) {
            // 2Gb - for emitting code
            constexpr size_t CODE_SIZE_LIMIT = 0x7f000000;
            paoc_->runtimeOptions_->SetInternalMemorySizeLimit(CODE_SIZE_LIMIT);
        }
#endif
        paoc_->runtimeOptions_->SetArkAot(true);
        if (!paoc_->runtimeOptions_->WasSetTaskmanagerWorkersCount()) {
            paoc_->runtimeOptions_->SetTaskmanagerWorkersCount(1);
        }
        if (!ark::Runtime::Create(*paoc_->runtimeOptions_)) {
            std::cerr << "Failed to create runtime!\n";
            return -1;
        }
        paoc_->runtime_ = Runtime::GetCurrent()->GetPandaVM()->GetCompilerRuntimeInterface();
        return 0;
    }

    int InitCompiler()
    {
        CompilerLogger::Init(ark::compiler::g_options.GetCompilerLog());

        if (ark::compiler::g_options.IsCompilerEnableEvents()) {
            EventWriter::Init(ark::compiler::g_options.GetCompilerEventsPath());
        }
        ValidateCompilerOptions();

        if (paoc_->paocOptions_->WasSetPaocMethodsFromFilePath()) {
            InitPaocMethodsFromFile();
        }
        paoc_->skipInfo_.isFirstCompiled = !paoc_->paocOptions_->WasSetPaocSkipUntil();
        paoc_->skipInfo_.isLastCompiled = false;

        if (!InitAotBuilder()) {
            return 1;
        }
        if (paoc_->paocOptions_->WasSetPaocDumpStatsCsv()) {
            paoc_->statisticsDump_ = std::ofstream(paoc_->paocOptions_->GetPaocDumpStatsCsv(), std::ofstream::trunc);
        }
        InitPaocMode();
        paoc_->codeAllocator_ = new ArenaAllocator(ark::SpaceType::SPACE_TYPE_INTERNAL, nullptr, true);
        paoc_->loader_ = ark::Runtime::GetCurrent()->GetClassLinker();

        return 0;
    }

    void CheckOptionsErr()
    {
        auto compilerOptionsErr = ark::compiler::g_options.Validate();
        if (compilerOptionsErr) {
            LOG_PAOC(FATAL) << compilerOptionsErr.value().GetMessage();
        }
        auto paocOptionsErr = paoc_->paocOptions_->Validate();
        if (paocOptionsErr) {
            LOG_PAOC(FATAL) << paocOptionsErr.value().GetMessage();
        }
    }

    void ValidateCompilerOptions()
    {
        CheckOptionsErr();
        paoc_->ValidateExtraOptions();

        if (paoc_->paocOptions_->WasSetPaocOutput() && paoc_->paocOptions_->WasSetPaocBootOutput()) {
            LOG_PAOC(FATAL) << "combination of --paoc-output and --paoc-boot-output is not compatible\n";
        }
        if (paoc_->paocOptions_->WasSetPaocBootPandaLocations()) {
            if (paoc_->paocOptions_->WasSetPaocBootLocation()) {
                LOG_PAOC(FATAL) << "We can't use --paoc-boot-panda-locations with --paoc-boot-location\n";
            }
            auto locations = paoc_->paocOptions_->GetPaocBootPandaLocations();
            size_t i = 0;
            // In fact boot files are preloaded by Runtime
            for (auto bpf : Runtime::GetCurrent()->GetClassLinker()->GetBootPandaFiles()) {
                if (i >= locations.size()) {
                    EVENT_PAOC("Numbers of files in --paoc-boot-panda-locations and --boot-panda-files are different");
                    LOG_PAOC(FATAL)
                        << "number of locations in --paoc-boot-panda-locations less then files in --boot-panda-files\n";
                }
                auto filename = locations[i];
                auto bpfName = bpf->GetFilename();
                if (!CompareBootFiles(bpfName, filename)) {
                    EVENT_PAOC("different files in --paoc-boot-panda-locations and --boot-panda-files");
                    LOG_PAOC(ERROR) << "The file from --paoc-boot-panda-locations: " << filename;
                    LOG_PAOC(ERROR) << "isn't eqaul the file from --boot-panda-files: " << bpf->GetFilename();
                    LOG_PAOC(FATAL) << "Files posotions " << i;
                }
                paoc_->locationMapping_[bpfName] = filename;
                paoc_->preloadedFiles_[paoc_->GetFilePath(bpfName)] = bpf;
                ++i;
            }
            if (i != locations.size()) {
                EVENT_PAOC("Numbers of files in --paoc-boot-panda-locations and --boot-panda-files are different");
                LOG_PAOC(FATAL)
                    << "number of locations in --paoc-boot-panda-locations more then files in --boot-panda-files\n";
            }
        }
        // In fact boot files are preloaded by Runtime
        for (auto bpf : Runtime::GetCurrent()->GetClassLinker()->GetBootPandaFiles()) {
            if (!paoc_->paocOptions_->GetPaocBootLocation().empty()) {
                std::string filename = GetFileLocation(*bpf, paoc_->paocOptions_->GetPaocBootLocation());
                paoc_->locationMapping_[bpf->GetFilename()] = filename;
            }
            paoc_->preloadedFiles_[paoc_->GetFilePath(bpf->GetFilename())] = bpf;
        }
    }

    void InitPaocMethodsFromFile()
    {
        std::ifstream mfile(paoc_->paocOptions_->GetPaocMethodsFromFilePath());
        std::string line;
        if (mfile) {
            while (getline(mfile, line)) {
                paoc_->methodsList_.insert(line);
            }
        }
        LOG_PAOC(INFO) << "Method list size: " << paoc_->methodsList_.size();
    }

    void InitPaocMode()
    {
        const auto &modeStr = paoc_->paocOptions_->GetPaocMode();
        if (modeStr == "aot") {
            paoc_->mode_ = PaocMode::AOT;
        } else if (modeStr == "jit") {
            paoc_->mode_ = PaocMode::JIT;
        } else if (modeStr == "osr") {
            paoc_->mode_ = PaocMode::OSR;
        } else if (modeStr == "llvm") {
            paoc_->mode_ = PaocMode::LLVM;
        } else {
            UNREACHABLE();
        }
    }

    bool InitAotBuilder()
    {
        Arch arch = RUNTIME_ARCH;
        bool crossCompilation = false;
        if (compiler::g_options.WasSetCompilerCrossArch()) {
            arch = GetArchFromString(compiler::g_options.GetCompilerCrossArch());
            crossCompilation = arch != RUNTIME_ARCH;
        }
        ark::compiler::g_options.AdjustCpuFeatures(crossCompilation);

        if (arch == Arch::NONE) {
            LOG_PAOC(ERROR) << "Invalid --compiler-cross-arch option:" << compiler::g_options.GetCompilerCrossArch();
            return false;
        }
        if (!BackendSupport(arch)) {
            LOG_PAOC(ERROR) << "Backend is not supported: " << compiler::g_options.GetCompilerCrossArch();
            return false;
        }
        paoc_->aotBuilder_ = paoc_->CreateAotBuilder();
        paoc_->aotBuilder_->SetArch(arch);

        // Initialize GC:
        auto runtimeLang = paoc_->runtimeOptions_->GetRuntimeType();
#ifdef ARK_HYBRID
        if (!paoc_->runtimeOptions_->WasSetGcType(runtimeLang)) {
            paoc_->runtimeOptions_->SetGcType("cmc-gc");
            LOG_PAOC(INFO) << "Not set the GC type, and use cmc-gc by default when Ark hybrid mode is enable";
        }
#endif
        // Fix it in issue 8164:
        auto gcType = ark::mem::GCTypeFromString(paoc_->runtimeOptions_->GetGcType(runtimeLang));
        ASSERT(gcType != ark::mem::GCType::INVALID_GC);

        paoc_->aotBuilder_->SetGcType(static_cast<uint32_t>(gcType));

#ifndef NDEBUG
        paoc_->aotBuilder_->SetGenerateSymbols(true);
#else
        paoc_->aotBuilder_->SetGenerateSymbols(paoc_->paocOptions_->IsPaocGenerateSymbols());
#endif
#ifdef PANDA_COMPILER_DEBUG_INFO
        paoc_->aotBuilder_->SetEmitDebugInfo(ark::compiler::g_options.IsCompilerEmitDebugInfo());
#endif
        return true;
    }

    bool InitPaocClusters(ark::PandArgParser *paParser)
    {
        std::ifstream fstream(paoc_->paocOptions_->GetPaocClusters());
        if (!fstream) {
            LOG_PAOC(ERROR) << "Can't open `paoc-clusters` file";
            return false;
        }
        JsonObject obj(fstream.rdbuf());
        fstream.close();
        paoc_->clustersInfo_.Init(obj, paParser);
        return true;
    }

private:
    Paoc *paoc_ {nullptr};
};

int Paoc::Run(const ark::Span<const char *> &args)
{
    if (PaocInitializer(this).Init(args) != 0) {
        return -1;
    }
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    if (compiler::g_options.WasSetCompilerProfile()) {
        auto res = runtime_->AddProfile(compiler::g_options.GetCompilerProfile());
        if (!res) {
            LOG(FATAL, COMPILER) << "Cannot open profile `" << compiler::g_options.GetCompilerProfile()
                                 << "`: " << res.Error();
        }
    }
    aotBuilder_->SetRuntime(runtime_);
    slowPathData_ = allocator->New<compiler::SharedSlowPathData>();

    if (!LoadPandaFiles()) {
        LOG_PAOC(FATAL) << "Can not load panda files";
    }
    if (!TryLoadAotProfile()) {
        LOG_PAOC(ERROR) << "Can not load AOT profile";
        if (paocOptions_->IsPaocUseProfileForce()) {
            LOG_PAOC(FATAL) << "Can not continue execution without profiling file, since 'force' option is set.";
        }
    }
    bool errorOccurred = !CompileFiles();
    bool attemptedToCompile = (compilationIndex_ != 0);
    errorOccurred |= !attemptedToCompile;
    if (IsAotMode()) {
        if (aotBuilder_->GetCurrentCodeAddress() != 0 || !aotBuilder_->GetClassHashTableSize()->empty() ||
            IsLLVMAotMode()) {
            RunAotMode(args);
        }
        if (aotBuilder_->GetCurrentCodeAddress() == 0 && aotBuilder_->GetClassHashTableSize()->empty()) {
            LOG_PAOC(ERROR) << "There are no compiled methods";
            Clear(allocator);
            return -1;
        }
    }
    if (!attemptedToCompile) {
        LOG_PAOC(WARNING) << "There are no compiled methods";
    }
    Clear(allocator);
    return ShouldIgnoreFailures() ? 0 : (errorOccurred ? 1 : 0);
}

std::string Paoc::BuildClassContext()
{
    std::string classCtx;
    loader_->EnumeratePandaFiles([this, &classCtx](const panda_file::File &pf) {
        if (locationMapping_.find(pf.GetFilename()) == locationMapping_.end()) {
            classCtx += GetFilePath(pf.GetFilename());
        } else {
            classCtx += locationMapping_[pf.GetFilename()];
        }
        classCtx += AotClassContextCollector::HASH_DELIMETER;
        classCtx += pf.GetPaddedChecksum();
        classCtx += AotClassContextCollector::DELIMETER;
        return true;
    });
    classCtx.pop_back();
    return classCtx;
}

void Paoc::RunAotMode(const ark::Span<const char *> &args)
{
    std::string cmdline;
    for (auto arg : args) {
        cmdline += arg;
        cmdline += " ";
    }
    std::string classCtx = BuildClassContext();
    aotBuilder_->SetClassContext(classCtx);
    LOG(DEBUG, COMPILER) << "PAOC: ClassContext '" << classCtx << '\'';
    auto outputFile = paocOptions_->GetPaocOutput();
    if (paocOptions_->WasSetPaocBootOutput()) {
        aotBuilder_->SetBootAot(true);
        outputFile = paocOptions_->GetPaocBootOutput();
    }
    aotBuilder_->SetWithCha(paocOptions_->IsPaocUseCha());

    if (IsLLVMAotMode()) {
        bool writeAot = EndLLVM();
        if (!writeAot) {
            return;
        }
    }

    aotBuilder_->Write(cmdline, outputFile);
    LOG_PAOC(INFO) << "METHODS COMPILED (success/all): " << successMethods_ << '/' << successMethods_ + failedMethods_;
    LOG_PAOC(DEBUG) << "Successfully compiled to " << outputFile;
}

void Paoc::Clear(ark::mem::InternalAllocatorPtr allocator)
{
    delete codeAllocator_;
    codeAllocator_ = nullptr;
    allocator->Delete(slowPathData_);
    methodsList_.clear();
    ark::Runtime::Destroy();
}

void Paoc::StartAotFile(const panda_file::File &pfileRef)
{
    ASSERT(IsAotMode());
    std::string filename;
    if (paocOptions_->GetPaocLocation().empty()) {
        filename = GetFilePath(pfileRef.GetFilename());
    } else {
        filename = GetFileLocation(pfileRef, paocOptions_->GetPaocLocation());
        locationMapping_[pfileRef.GetFilename()] = filename;
    }
    ASSERT(!filename.empty());
    aotBuilder_->StartFile(filename, pfileRef.GetHeader()->checksum);
}

static bool CheckFilesInClassContext(std::string_view profileContext, std::string_view context)
{
    size_t start = 0;
    size_t end = profileContext.find(AotClassContextCollector::DELIMETER, start);
    while (end != std::string::npos) {
        auto fileContext = profileContext.substr(start, end - start);
        if (context.find(fileContext) == std::string::npos) {
            size_t nameEnd = fileContext.find(AotClassContextCollector::HASH_DELIMETER);
            if (nameEnd == std::string::npos) {
                LOG_PAOC(WARNING) << "Invalid profile context";
                return false;
            }
            auto file = fileContext.substr(0, nameEnd);
            if (context.find(file) != std::string::npos) {
                LOG_PAOC(WARNING) << "Hash mismatch for file " << file << " in profile context";
                return false;
            }
            LOG_PAOC(DEBUG) << "Will skip file " << file << " profile";
        }
        start = end + 1;
        end = profileContext.find(AotClassContextCollector::DELIMETER, start);
    }
    return true;
}

bool Paoc::TryLoadAotProfile()
{
    if (!paocOptions_->IsPaocUseProfile()) {
        return true;
    }

    if (!paocOptions_->WasSetPaocUseProfilePath()) {
        LOG_PAOC(ERROR) << "AOT profile path not set";
        return false;
    }

    PandaString profileClassCtxStr;
    ProfilingLoader profilingLoader;
    {
        auto profileCtxOrError = profilingLoader.LoadProfile(ConvertToString(paocOptions_->GetPaocUseProfilePath()));
        if (!profileCtxOrError) {
            LOG_PAOC(ERROR) << "Could not load AOT profile: " << profileCtxOrError.Error();
            return false;
        }
        profileClassCtxStr = std::move(*profileCtxOrError);
    }

    auto classCtxStr = BuildClassContext();
    if (!CheckFilesInClassContext(profileClassCtxStr, classCtxStr)) {
        LOG_PAOC(WARNING) << "Cannot use profile '" << paocOptions_->GetPaocUseProfilePath() << "'";
        LOG_PAOC(WARNING) << "\tAOT class context:";
        LOG_PAOC(WARNING) << classCtxStr;
        LOG_PAOC(WARNING) << "\tProfile class context:";
        LOG_PAOC(WARNING) << profileClassCtxStr;
        return false;
    }

    bool errorOccurred = false;
    loader_->EnumeratePandaFiles([this, &profilingLoader, &errorOccurred](const panda_file::File &pfileRef) {
        std::string filename;
        if (auto mappedIt = locationMapping_.find(pfileRef.GetFilename()); mappedIt != locationMapping_.end()) {
            filename = mappedIt->second;
        } else {
            filename = GetFilePath(pfileRef.GetFilename());
        }
        if (auto pfileIdx = profilingLoader.GetAotProfilingData().GetPandaFileIdxByName(filename); pfileIdx != -1) {
            errorOccurred = errorOccurred || !TryLoadAotFileProfile(profilingLoader, pfileRef, pfileIdx);
        }
        return true;
    });
    return !errorOccurred;
}

template <typename Callback>
static void IterateClassMethods(const panda_file::File &pfileRef, ark::Class *klass, Callback &&callback)
{
    auto methods = klass->GetMethods();
    panda_file::ClassDataAccessor cda(pfileRef, klass->GetFileId());
    size_t methodIndex = 0;
    size_t smethodIdx = klass->GetNumVirtualMethods();
    size_t vmethodIdx = 0;
    cda.EnumerateMethods([&smethodIdx, &vmethodIdx, &methods, &methodIndex,
                          &callback](panda_file::MethodDataAccessor &methodDataAccessor) {
        Method &method = methodDataAccessor.IsStatic() ? methods[smethodIdx++] : methods[vmethodIdx++];
        callback(method);
        methodIndex++;
    });
}

bool Paoc::TryLoadAotFileProfile(ProfilingLoader &profilingLoader, const panda_file::File &pfileRef, size_t pfileIdx)
{
    auto &allMethods = profilingLoader.GetAotProfilingData().GetAllMethods();
    auto fileProfiles = allMethods.find(pfileIdx);
    if (fileProfiles == allMethods.end()) {
        return true;
    }
    auto &methodProfiles = fileProfiles->second;

    auto classes = pfileRef.GetClasses();
    bool errorOccurred = false;
    for (auto &classId : classes) {
        panda_file::File::EntityId id(classId);
        ark::Class *klass = ResolveClass(pfileRef, id);
        if (klass == nullptr) {
            continue;
        }

        IterateClassMethods(
            pfileRef, klass, [this, &profilingLoader, classId, &methodProfiles, &errorOccurred](Method &method) {
                errorOccurred =
                    errorOccurred || !TryLoadAotMethodProfile(profilingLoader, methodProfiles, classId, method);
            });
    }

    return !errorOccurred;
}

bool Paoc::TryLoadAotMethodProfile(
    ProfilingLoader &profilingLoader,
    PandaMap<uint32_t, ark::pgo::AotProfilingData::AotMethodProfilingData> &methodProfiles, uint32_t classId,
    Method &method)
{
    // CC-OFFNXT(G.FMT.14-CPP) project code style
    auto classResolver = [this, &profilingLoader](uint32_t classIdx, size_t fileIdx) -> ark::Class * {
        const auto &fileMapRev = profilingLoader.GetAotProfilingData().GetPandaFileMapReverse();
        auto fileNameIt = fileMapRev.find(fileIdx);
        if (fileNameIt == fileMapRev.end()) {
            return nullptr;
        }
        auto fileIt = preloadedFiles_.find(std::string(fileNameIt->second));
        if (fileIt == preloadedFiles_.end()) {
            return nullptr;
        }
        return ResolveClass(*fileIt->second, panda_file::File::EntityId(classIdx));
    };

    auto methodId = method.GetFileId();
    auto profileIt = methodProfiles.find(methodId.GetOffset());
    if (profileIt == methodProfiles.end()) {
        return true;
    }

    const pgo::AotProfilingData::AotMethodProfilingData &profile = profileIt->second;
    if (profile.GetClassIdx() != classId) {
        LOG_PAOC(ERROR) << "ClassIdx mismatch for method '" << runtime_->GetMethodFullName(&method) << "' AOT profile";
        return false;
    }

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    ProfilingData *profilingData = profilingLoader.CreateProfilingData(profile, allocator, classResolver);
    if (!method.InitProfilingData(profilingData)) {
        allocator->Free(profilingData);
        return false;
    }

    return true;
}

bool Paoc::TryLoadPandaFile(const std::string &fileName, PandaVM *vm)
{
    const panda_file::File *pfile;
    bool errorOccurred = false;

    auto filePath = GetFilePath(fileName);
    if (preloadedFiles_.find(filePath) != preloadedFiles_.end()) {
        pfile = preloadedFiles_[filePath];
    } else {
        std::unique_ptr<const panda_file::File> file;
        if (paocOptions_->GetPaocZipPandaFile().empty()) {
            file = vm->OpenPandaFile(fileName);
        } else {
            file = TryLoadZipPandaFile(fileName);
        }
        if (!file) {
            if (!ShouldIgnoreFailures()) {
                LOG_PAOC(FATAL) << "Can not open file: " << fileName;
            }
            LOG_PAOC(WARNING) << "Can not open file: " << fileName;
            return false;
        }
        pfile = file.get();
        loader_->AddPandaFile(std::move(file));
        LOG_PAOC(DEBUG) << "Added panda file: " << fileName;
    }
    auto &pfileRef = *pfile;

    if (IsAotMode()) {
        StartAotFile(pfileRef);
    }

    if (!CompilePandaFile(pfileRef)) {
        errorOccurred = true;
    }

    if (IsAotMode()) {
        aotBuilder_->EndFile();
    }
    return !errorOccurred || ShouldIgnoreFailures();
}

std::unique_ptr<const panda_file::File> Paoc::TryLoadZipPandaFile(const std::string &fileName)
{
    std::string zipPandaFilePath = paocOptions_->GetPaocZipPandaFile();
    size_t pos = fileName.rfind(zipPandaFilePath);
    if (pos == std::string::npos) {
        LOG_PAOC(FATAL) << "Invalid zip panda file path";
        return nullptr;
    }

    std::string location = fileName.substr(0, pos - 1);  // 1: for '/'
    FILE *fp = fopen(location.c_str(), "rbe");
    if (fp == nullptr) {
        LOG_PAOC(ERROR) << "Can't fopen location: " << location;
        return nullptr;
    }
    std::unique_ptr<const panda_file::File> file = panda_file::OpenZipPandaFile(fp, fileName, zipPandaFilePath);
    fclose(fp);
    return file;
}

/**
 * Iterate over `--paoc-panda-files`.
 * @return `false` on error.
 */
bool Paoc::CompileFiles()
{
    auto pfiles = paocOptions_->GetPaocPandaFiles();
    auto *vm = ark::Runtime::GetCurrent()->GetPandaVM();
    for (auto &fileName : pfiles) {
        if (!TryLoadPandaFile(fileName, vm)) {
            return false;
        }
    }
    return true;
}

std::string Paoc::GetFilePath(std::string fileName)
{
    if (runtimeOptions_->IsAotVerifyAbsPath()) {
        return os::GetAbsolutePath(fileName);
    }
    return fileName;
}

/**
 * Compile every method from loaded panda files that matches `--compiler-regex`,
 * `--paoc-skip-until`, `--paoc-compile-until` and `--paoc-methods-from-file` options:
 * @return `false` on error.
 */
bool Paoc::CompilePandaFile(const panda_file::File &pfileRef)
{
    auto classes = pfileRef.GetClasses();
    bool errorOccurred = false;
    for (auto &classId : classes) {
        panda_file::File::EntityId id(classId);
        ark::Class *klass = ResolveClass(pfileRef, id);
        std::string className = ClassHelper::GetName(pfileRef.GetStringData(id).data);

        if (!PossibleToCompile(pfileRef, klass, id)) {
            if (paocOptions_->IsPaocVerbose()) {
                LOG_PAOC(DEBUG) << "Ignoring a class `" << className << "` (id = " << id << ", file `"
                                << pfileRef.GetFilename() << "`)";
            }
            continue;
        }

        ASSERT(klass != nullptr);
        if (paocOptions_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Compiling class `" << className << "` (id = " << id << ", file `"
                            << pfileRef.GetFilename() << "`)";
        }

        // Check that all of the methods are compiled correctly:
        ASSERT(klass->GetPandaFile() != nullptr);
        if (!Compile(klass, *klass->GetPandaFile())) {
            errorOccurred = true;
            std::string errMsg = "Class `" + className + "` from " + pfileRef.GetFilename() + " compiled with errors";
            PrintError(errMsg);
            if (!ShouldIgnoreFailures()) {
                break;
            }
        }
    }

    BuildClassHashTable(pfileRef);

    return !errorOccurred;
}

ark::Class *Paoc::ResolveClass(const panda_file::File &pfileRef, panda_file::File::EntityId classId)
{
    ErrorHandler handler;
    ScopedMutatorLock lock;
    if (pfileRef.IsExternal(classId)) {
        return nullptr;
    }
    panda_file::ClassDataAccessor cda(pfileRef, classId);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(&cda);
    auto klass = loader_->GetExtension(ctx)->GetClass(pfileRef, classId, nullptr, &handler);
    return klass;
}

/**
 * Check if it is possible to compile a class.
 * @return true if the class isn't external, abstract, interface and isn't an array class.
 */
bool Paoc::PossibleToCompile(const panda_file::File &pfileRef, const ark::Class *klass,
                             panda_file::File::EntityId classId)
{
    std::string className = ClassHelper::GetName(pfileRef.GetStringData(classId).data);
    if (klass == nullptr) {
        if (paocOptions_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Class is nullptr: `" << className << "`";
            LOG_PAOC(ERROR) << "ClassLinker can't load a class `" << className << "`";
        }
        return false;
    }

    // Skip external classes
    if (pfileRef.IsExternal(classId) || klass->GetFileId().GetOffset() != classId.GetOffset()) {
        if (paocOptions_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Can't compile external class `" << className << "`";
        }
        return false;
    }
    // Skip object array class
    if (klass->IsArrayClass()) {
        if (paocOptions_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Can't compile array class `" << className << "`";
        }
        return false;
    }

    if (klass->IsAbstract()) {
        if (paocOptions_->IsPaocVerbose()) {
            LOG_PAOC(DEBUG) << "Will compile abstract class `" << className << "`";
        }
    }

    return true;
}

/**
 * Compile the class.
 * @return `false` on error.
 */
bool Paoc::Compile(Class *klass, const panda_file::File &pfileRef)
{
    ASSERT(klass != nullptr);

    if (IsAotMode()) {
        aotBuilder_->StartClass(*klass);
    }
    bool errorOccurred = false;
    size_t methodIndex = 0;
    IterateClassMethods(pfileRef, klass, [this, &methodIndex, &pfileRef, &errorOccurred](Method &method) {
        if (errorOccurred && !ShouldIgnoreFailures()) {
            return;
        }
        // NOTE(pishin, msherstennikov): revisit
        // Method (or the whole class?) may already have a definition in another file,
        // in this case it should not be added into AOT file.
        auto methodName = runtime_->GetMethodFullName(&method, g_options.WasSetCompilerRegexWithSignature());
        if (method.GetPandaFile()->GetFilename() == pfileRef.GetFilename() && !Skip(&method) &&
            IsMethodShouldBeCompiled(methodName) && g_options.MatchesRegex(methodName) &&
            !Compile(&method, methodIndex)) {
            errorOccurred = true;
        }
        methodIndex++;
    });

    if (IsAotMode()) {
        aotBuilder_->EndClass();
    }

    return !errorOccurred;
}

bool Paoc::Compile(Method *method, size_t methodIndex)
{
    if (method == nullptr) {
        LOG_PAOC(WARNING) << "Method is nullptr";
        return false;
    }
#ifndef NDEBUG
    const auto *pfile = method->GetPandaFile();
    auto methodId = method->GetFileId();
    ASSERT(pfile != nullptr);
    ASSERT(!pfile->IsExternal(methodId));
#endif

    if (method->IsAbstract() || method->IsNative() || method->IsIntrinsic()) {
        return true;
    }
    auto methodName = runtime_->GetMethodFullName(method, false);

    ++compilationIndex_;
    LOG_PAOC(INFO) << "[" << compilationIndex_ << "] Compile method (id=" << method->GetFileId() << "): " << methodName;

    CompilingContext ctx(method, methodIndex, &statisticsDump_);

    PaocClusters::ScopedApplySpecialOptions to(methodName, &clustersInfo_);
    switch (mode_) {
        case PaocMode::AOT:
        case PaocMode::LLVM:
            if (!CompileAot(&ctx)) {
                EVENT_COMPILATION(methodName, false, ctx.method->GetCodeSize(), 0, 0, 0,
                                  events::CompilationStatus::FAILED);
                failedMethods_++;
                ctx.compilationStatus = false;
            } else {
                successMethods_++;
            }
            break;
        case PaocMode::JIT:
            ctx.compilationStatus = CompileJit(&ctx);
            break;
        case PaocMode::OSR:
            ctx.compilationStatus = CompileOsr(&ctx);
            break;
        default:
            UNREACHABLE();
    }
    return ctx.compilationStatus;
}

bool Paoc::CompileInGraph(CompilingContext *ctx, std::string methodName, bool isOsr)
{
    compiler::InPlaceCompilerTaskRunner taskRunner;
    auto &taskCtx = taskRunner.GetContext();
    taskCtx.SetMethod(ctx->method);
    taskCtx.SetOsr(isOsr);
    taskCtx.SetAllocator(&ctx->allocator);
    taskCtx.SetLocalAllocator(&ctx->graphLocalAllocator);
    taskCtx.SetMethodName(std::move(methodName));
    taskRunner.AddFinalize(
        [&graph = ctx->graph](InPlaceCompilerContext &compilerCtx) { graph = compilerCtx.GetGraph(); });

    bool success = true;
    taskRunner.AddCallbackOnFail([&success]([[maybe_unused]] InPlaceCompilerContext &compilerCtx) { success = false; });
    auto arch = ChooseArch(Arch::NONE);
    bool isDynamic = ark::panda_file::IsDynamicLanguage(ctx->method->GetClass()->GetSourceLang());
    compiler::CompileInGraph<compiler::INPLACE_MODE>(runtime_, isDynamic, arch, std::move(taskRunner));
    return success;
}

/**
 * Compiles a method in JIT mode (i.e. no code generated).
 * @return `false` on error.
 */
bool Paoc::CompileJit(CompilingContext *ctx)
{
    ASSERT(ctx != nullptr);
    ASSERT(mode_ == PaocMode::JIT);
    auto name = runtime_->GetMethodFullName(ctx->method, false);
    if (!CompileInGraph(ctx, name, false)) {
        std::string errMsg = "Failed to JIT-compile method: " + name;
        PrintError(errMsg);
        return false;
    }
    EVENT_COMPILATION(name, false, ctx->method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::COMPILED);
    return true;
}

/**
 * Compiles a method in OSR mode.
 * @return `false` on error.
 */
bool Paoc::CompileOsr(CompilingContext *ctx)
{
    ASSERT(ctx != nullptr);
    ASSERT(mode_ == PaocMode::OSR);
    auto name = runtime_->GetMethodFullName(ctx->method, false);
    if (!CompileInGraph(ctx, name, true)) {
        std::string errMsg = "Failed to OSR-compile method: " + name;
        PrintError(errMsg);
        return false;
    }
    EVENT_COMPILATION(name, true, ctx->method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::COMPILED);
    return true;
}

bool Paoc::TryCreateGraph(CompilingContext *ctx)
{
    auto sourceLang = ctx->method->GetClass()->GetSourceLang();
    bool isDynamic = ark::panda_file::IsDynamicLanguage(sourceLang);

    ctx->graph = ctx->allocator.New<Graph>(
        Graph::GraphArgs {&ctx->allocator, &ctx->graphLocalAllocator, aotBuilder_->GetArch(), ctx->method, runtime_},
        nullptr, false, isDynamic);
    if (ctx->graph == nullptr) {
        PrintError("Graph creation failed!");
        return false;
    }
    ctx->graph->SetLanguage(sourceLang);
    return true;
}

bool Paoc::FinalizeCompileAot(CompilingContext *ctx, [[maybe_unused]] uintptr_t codeAddress)
{
    CompiledMethod compiledMethod(ctx->graph->GetArch(), ctx->method, ctx->index);
    compiledMethod.SetCode(ctx->graph->GetCode().ToConst());
    compiledMethod.SetCodeInfo(ctx->graph->GetCodeInfoData());
#ifdef PANDA_COMPILER_DEBUG_INFO
    compiledMethod.SetCfiInfo(ctx->graph->GetCallingConvention()->GetCfiInfo());
#endif
    if (ctx->graph->GetCode().empty() || ctx->graph->GetCodeInfoData().empty()) {
        LOG(INFO, COMPILER) << "Emit code failed";
        return false;
    }

    LOG(INFO, COMPILER) << "Ark AOT successfully compiled method: " << runtime_->GetMethodFullName(ctx->method, false);
    EVENT_PAOC("Compiling " + runtime_->GetMethodFullName(ctx->method, false) + " using panda");
    ASSERT(ctx->graph != nullptr);

    aotBuilder_->AddMethod(compiledMethod);

    EVENT_COMPILATION(runtime_->GetMethodFullName(ctx->method, false), false, ctx->method->GetCodeSize(), codeAddress,
                      compiledMethod.GetCode().size(), compiledMethod.GetCodeInfo().size(),
                      events::CompilationStatus::COMPILED);
    return true;
}

bool Paoc::RunOptimizations(CompilingContext *ctx)
{
    compiler::InPlaceCompilerTaskRunner taskRunner;
    taskRunner.GetContext().SetGraph(ctx->graph);
    bool success = true;
    taskRunner.AddCallbackOnFail([&success]([[maybe_unused]] InPlaceCompilerContext &compilerCtx) { success = false; });

    compiler::RunOptimizations<compiler::INPLACE_MODE>(std::move(taskRunner));
    return success;
}

/**
 * Compiles a method in AOT mode.
 * @return `false` on error.
 */
bool Paoc::CompileAot(CompilingContext *ctx)
{
    ASSERT(ctx != nullptr);
    ASSERT(IsAotMode());

    LOG_IF(IsLLVMAotMode() && !paocOptions_->IsPaocUseCha(), FATAL, COMPILER)
        << "LLVM AOT compiler supports only --paoc-use-cha=true";

    std::string className = ClassHelper::GetName(ctx->method->GetClassName().data);
    if (runtimeOptions_->WasSetEventsOutput()) {
        EVENT_PAOC("Trying to compile method: " + className +
                   "::" + reinterpret_cast<const char *>(ctx->method->GetName().data));
    }

    if (!TryCreateGraph(ctx)) {
        return false;
    }

    uintptr_t codeAddress = aotBuilder_->GetCurrentCodeAddress();
    MakeAotData(ctx, codeAddress);

    if (!ctx->graph->RunPass<IrBuilder>()) {
        PrintError("IrBuilder failed!");
        return false;
    }

    if (IsLLVMAotMode()) {
        auto result = TryLLVM(ctx);
        switch (result) {
            case LLVMCompilerStatus::COMPILED:
                return true;
            case LLVMCompilerStatus::SKIP:
                return false;
            case LLVMCompilerStatus::ERROR:
                LOG_PAOC(FATAL) << "LLVM AOT failed (unknown instruction)";
                break;
            case LLVMCompilerStatus::FALLBACK:
                // Fallback to Ark Compiler AOT compilation
                break;
            default:
                UNREACHABLE();
                break;
        }
        LOG_PAOC(INFO) << "LLVM fallback to ARK AOT on method: " << runtime_->GetMethodFullName(ctx->method, false);
    }

    if (!RunOptimizations(ctx)) {
        PrintError("RunOptimizations failed!");
        return false;
    }

    return FinalizeCompileAot(ctx, codeAddress);
}

void Paoc::MakeAotData(CompilingContext *ctx, uintptr_t codeAddress)
{
    auto *aotData = ctx->graph->GetAllocator()->New<AotData>(
        AotData::AotDataArgs {ctx->method->GetPandaFile(),
                              ctx->graph,
                              slowPathData_,
                              codeAddress,
                              aotBuilder_->GetIntfInlineCacheIndex(),
                              {aotBuilder_->GetGotPlt(), aotBuilder_->GetGotVirtIndexes(), aotBuilder_->GetGotClass(),
                               aotBuilder_->GetGotString()},
                              {aotBuilder_->GetGotIntfInlineCache(), aotBuilder_->GetGotCommon()}});

    ASSERT(aotData != nullptr);
    aotData->SetUseCha(paocOptions_->IsPaocUseCha());
    aotData->SetHasProfileData(ctx->method->GetProfilingData() != nullptr);
    aotData->SetIsLLVMAotMode(IsLLVMAotMode());
    ctx->graph->SetAotData(aotData);
}

void Paoc::PrintError(const std::string &error)
{
    if (ShouldIgnoreFailures()) {
        LOG_PAOC(WARNING) << error;
    } else {
        LOG_PAOC(ERROR) << error;
    }
}

bool Paoc::ShouldIgnoreFailures()
{
    return compiler::g_options.IsCompilerIgnoreFailures();
}

void Paoc::PrintUsage(const ark::PandArgParser &paParser)
{
    std::cerr << "Usage: ark_aot [OPTIONS] --paoc-panda-files <list>\n";
    std::cerr << "    --paoc-panda-files          list of input panda files, it is a mandatory option\n";
    std::cerr << "    --paoc-mode                 specify compilation mode (aot, llvm, jit or osr) \n";
    std::cerr << "    --paoc-output               path to output file, default is out.an\n";
    std::cerr << "    --paoc-location             location path of the input panda file, that will be written into"
                 " the AOT file\n";
    std::cerr << "    --paoc-skip-until           first method to compile, skip all previous\n";
    std::cerr << "    --paoc-compile-until        last method to compile, skip all following\n";
    std::cerr << "    --paoc-methods-from-file    use white or black lists to filter methods to compile\n";
#ifndef NDEBUG
    std::cerr << "    --paoc-generate-symbols     generate symbols for compiled methods, disabled by default\n";
#endif
    std::cerr << "    --compiler-ignore-failures  ignore failures in methods/classes/files compilation\n";
    std::cerr << " You can also use other Ark compiler options\n";

    std::cerr << paParser.GetHelpString() << std::endl;
}

bool Paoc::IsMethodShouldBeCompiled(const std::string &methodFullName)
{
    if (paocOptions_->IsPaocMethodsFromFileIswhite()) {
        return !paocOptions_->WasSetPaocMethodsFromFilePath() ||
               (methodsList_.find(methodFullName) != methodsList_.end());
    }
    if (!paocOptions_->IsPaocMethodsFromFileIswhite()) {
        return !(paocOptions_->WasSetPaocMethodsFromFilePath() &&
                 (methodsList_.find(methodFullName) != methodsList_.end()));
    }
    return true;
}

/*
 * Check if needed to skip method, considering  'paoc-skip-until' and 'paoc-compile-until' options
 */
bool Paoc::Skip(Method *method)
{
    if (method == nullptr) {
        return true;
    }

    auto methodName = runtime_->GetMethodFullName(method, false);
    if (!skipInfo_.isFirstCompiled) {
        if (methodName == paocOptions_->GetPaocSkipUntil()) {
            skipInfo_.isFirstCompiled = true;
        } else {
            return true;
        }
    }
    if (skipInfo_.isLastCompiled) {
        return true;
    }
    if (paocOptions_->WasSetPaocCompileUntil() && methodName == paocOptions_->GetPaocCompileUntil()) {
        skipInfo_.isLastCompiled = true;
    }
    return false;
}

std::string Paoc::GetFileLocation(const panda_file::File &pfileRef, std::string location)
{
    auto &filename = pfileRef.GetFilename();
    if (auto pos = filename.rfind('/'); pos != std::string::npos) {
        if (location.back() == '/') {
            pos++;
        }
        location += filename.substr(pos);
    } else {
        location += '/' + filename;
    }
    return location;
}

bool Paoc::CompareBootFiles(std::string filename, std::string paocLocation)
{
    if (auto pos = filename.rfind('/'); pos != std::string::npos) {
        filename = filename.substr(++pos);
    }
    if (auto pos = paocLocation.rfind('/'); pos != std::string::npos) {
        paocLocation = paocLocation.substr(++pos);
    }

    return paocLocation == filename;
}

bool Paoc::LoadPandaFiles()
{
    bool errorOccurred = false;
    auto *vm = ark::Runtime::GetCurrent()->GetPandaVM();
    auto pfiles = runtimeOptions_->GetPandaFiles();
    for (auto &fileName : pfiles) {
        auto pfile = vm->OpenPandaFile(fileName);
        if (!pfile) {
            errorOccurred = true;
            if (!ShouldIgnoreFailures()) {
                LOG_PAOC(FATAL) << "Can not open file: " << fileName;
            }
            LOG_PAOC(WARNING) << "Can not open file: " << fileName;
            continue;
        }

        if (!paocOptions_->GetPaocLocation().empty()) {
            std::string filename = GetFileLocation(*pfile, paocOptions_->GetPaocLocation());
            locationMapping_[pfile->GetFilename()] = filename;
        }

        preloadedFiles_[GetFilePath(fileName)] = pfile.get();
        loader_->AddPandaFile(std::move(pfile));
    }
    return !errorOccurred;
}

void Paoc::BuildClassHashTable(const panda_file::File &pfileRef)
{
    if (!pfileRef.GetClasses().empty()) {
        aotBuilder_->AddClassHashTable(pfileRef);
    }
}

#undef LOG_PAOC

}  // namespace ark::paoc

int main(int argc, const char *argv[])
{
#ifdef PANDA_LLVM_AOT
    ark::paoc::PaocLLVM paoc;
#else
    ark::paoc::Paoc paoc;
#endif
    ark::Span<const char *> args(argv, argc);
    return paoc.Run(args);
}
