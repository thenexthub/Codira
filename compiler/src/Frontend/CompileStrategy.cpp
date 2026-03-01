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

/**
 * @file
 *
 * This file implements the CompileStrategy related classes.
 */

#include "Codira/Frontend/CompileStrategy.h"

#include "MergeAnnoFromCoded.h"
#include "Codira/AST/PrintNode.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/ConditionalCompilation/ConditionalCompilation.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Macro/MacroExpansion.h"
#include "Codira/Parse/Parser.h"
#if defined(CMAKE_ENABLE_ASSERT) || !defined(NDEBUG)
#include "Codira/Parse/ASTChecker.h"
#endif
#include "Codira/Sema/Desugar.h"
#include "Codira/Sema/TypeChecker.h"
#if (defined RELEASE)
#include "Codira/Utils/Signal.h"
#endif
#include "Codira/Utils/ProfileRecorder.h"

using namespace Codira;
using namespace Utils;
using namespace FileUtil;

void CompileStrategy::TypeCheck() const
{
    if (!ci->typeChecker) {
        ci->typeChecker = new TypeChecker(ci);
        CODEC_NULLPTR_CHECK(ci->typeChecker);
    }
    ci->typeChecker->TypeCheckForPackages(ci->GetSourcePackages());
}

bool CompileStrategy::ConditionCompile() const
{
    auto beforeErrCnt = ci->diag.GetErrorCount();
    ConditionalCompilation cc{ci};
    for (auto& pkg : ci->srcPkgs) {
        cc.HandleConditionalCompilation(*pkg.get());
    }
    return beforeErrCnt == ci->diag.GetErrorCount();
}

void CompileStrategy::DesugarAfterSema() const
{
    auto packages = ci->GetSourcePackages();
    ci->typeChecker->PerformDesugarAfterSema(packages);
}

bool CompileStrategy::OverflowStrategy() const
{
    if (!ci->typeChecker) {
        auto typeChecker = new TypeChecker(ci);
        CODEC_NULLPTR_CHECK(typeChecker);
        ci->typeChecker = typeChecker;
    }
    CODEC_ASSERT(ci->invocation.globalOptions.overflowStrategy != OverflowStrategy::NA);
    ci->typeChecker->SetOverflowStrategy(ci->GetSourcePackages());
    return true;
}

void CompileStrategy::PerformDesugar() const
{
    for (auto& [pkg, ctx] : ci->pkgCtxMap) {
        Codira::PerformDesugarBeforeTypeCheck(*pkg, ci->invocation.globalOptions.enableMacroInLSP);
    }
}

namespace Codira {
class FullCompileStrategyImpl final {
public:
    explicit FullCompileStrategyImpl(FullCompileStrategy& strategy) : s{strategy}
    {
    }

    void MergePackage(const Ptr<Package> target, const Ptr<Package> source)
    {
        if (target->accessible != source->accessible) {
            s.ci->diag.DiagnoseRefactor(DiagKindRefactor::packages_visibility_inconsistent, DEFAULT_POSITION,
                AST::GetAccessLevelStr(*target), AST::GetAccessLevelStr(*source));
        }
        if (target->isMacroPackage != source->isMacroPackage) {
            s.ci->diag.DiagnoseRefactor(DiagKindRefactor::packages_macro_inconsistent, DEFAULT_POSITION);
        }

        for (auto& file : source->files) {
            file->curPackage = target;
            if (target->files.size() > 0) {
                file->indexOfPackage = target->files.at(0)->indexOfPackage;
            }
            target->files.push_back(std::move(file));
        }
    }

    bool NeedToAddPackage(const Ptr<Package> package)
    {
        bool packageAlreadyExist = false;
        for (auto& srcPackage : s.ci->srcPkgs) {
            if (package->fullPackageName == srcPackage->fullPackageName) {
                MergePackage(srcPackage, package);
                packageAlreadyExist = true;
            }
        }
        if (!packageAlreadyExist) {
            bool isCODELint = s.ci->isCODELint;

            if (s.ci->srcPkgs.size() > 0 && !isCODELint) {
                // We can't validate it before because we can have multi-folder packages.
                s.ci->diag.DiagnoseRefactor(DiagKindRefactor::driver_require_one_package_directory, DEFAULT_POSITION);
                return false;
            }

            if (package->fullPackageName != "default" || package->files.size() != 0 || isCODELint) {
                return true;
            }
        }

        return false;
    }

    void ParseModule(bool& success)
    {
        std::string moduleSrcPath = s.ci->invocation.globalOptions.moduleSrcPath;
        std::unordered_set<std::string> includeFileSet;
        if (!s.ci->invocation.globalOptions.srcFiles.empty()) {
            includeFileSet.insert(
                s.ci->invocation.globalOptions.srcFiles.begin(), s.ci->invocation.globalOptions.srcFiles.end());
        }
        for (auto& srcDir : s.ci->srcDirs) {
            std::vector<std::string> allSrcFiles;
            auto currentPkg = DEFAULT_PACKAGE_NAME;
            if (!moduleSrcPath.empty()) {
                auto basePath = IsDir(moduleSrcPath) ? JoinPath(moduleSrcPath, "") : moduleSrcPath;
                currentPkg = GetPkgNameFromRelativePath(GetRelativePath(basePath, srcDir) | IdenticalFunc);
            }
            auto parseTest = s.ci->invocation.globalOptions.parseTest;
            auto compileTestsOnly = s.ci->invocation.globalOptions.compileTestsOnly;
            for (auto& srcFile : GetAllFilesUnderCurrentPath(srcDir, "code", !parseTest, compileTestsOnly)) {
                std::string filename = JoinPath(srcDir, srcFile);
                if (includeFileSet.empty()) {
                    // If no srcFiles, compile the whole module defaultly
                    allSrcFiles.push_back(filename);
                } else if (includeFileSet.find(filename) != includeFileSet.end()) {
                    // If there are srcFiles, use them to select files to compile
                    allSrcFiles.push_back(filename);
                }
            }
            auto package = ParseOnePackage(allSrcFiles, success, currentPkg);
            if (srcDir == moduleSrcPath) {
                package->needExported = false;
            }
            if (NeedToAddPackage(package)) {
                s.ci->srcPkgs.emplace_back(std::move(package));
            }
        }

        for (auto& package : s.ci->srcPkgs) {
            std::sort(package->files.begin(), package->files.end(),
                [](const OwnedPtr<File>& fileOne, const OwnedPtr<File>& fileTwo) {
                    return fileOne->fileName < fileTwo->fileName;
                });
        }

        if (s.ci->srcPkgs.empty()) {
            s.ci->srcPkgs.emplace_back(MakeOwned<Package>());
        }

        bool compilePackage = s.ci->invocation.globalOptions.compilePackage;
        // codelint support multipackage compile
        if (compilePackage && s.ci->srcPkgs.size() > 1 && !s.ci->isCODELint) {
            s.ci->diag.DiagnoseRefactor(DiagKindRefactor::driver_require_one_package_directory, DEFAULT_POSITION);
        }
    }

    bool PreReadCommonPartCodeo() const
    {
        bool hasInputCHIR = s.ci->invocation.globalOptions.IsCompilingCODEMP();
        if (hasInputCHIR) {
            auto mbFilesFromCommonPart = s.ci->importManager.GetCodeoManager()->PreReadCommonPartCodeoFiles();
            if (!mbFilesFromCommonPart) {
                return false;
            }
            std::vector<std::string> filesFromCommonPart = *mbFilesFromCommonPart;
            s.ci->GetSourceManager().ReserveCommonPartSources(filesFromCommonPart);
        }

        return true;
    }

    OwnedPtr<AST::Package> GetMultiThreadParseOnePackage(
        std::queue<std::future<std::tuple<OwnedPtr<File>, TokenVecMap, size_t>>>& futureQueue,
        const std::string& defaultPackageName) const
    {
        auto package = MakeOwned<Package>(defaultPackageName);
        size_t lineNumInOnePackage = 0;
        const size_t filePtrIdx = 0;
        const size_t commentIdx = 1;
        const size_t lineNumIdx = 2;
        while (!futureQueue.empty()) {
            auto curFuture = futureQueue.front().get();
            std::get<filePtrIdx>(curFuture)->curPackage = package.get();
            std::get<filePtrIdx>(curFuture)->indexOfPackage = package->files.size();
            package->files.push_back(std::move(std::get<filePtrIdx>(curFuture)));
            s.ci->GetSourceManager().AddComments(std::get<commentIdx>(curFuture));
            lineNumInOnePackage += std::get<lineNumIdx>(curFuture);
            futureQueue.pop();
        }
        Utils::ProfileRecorder::RecordCodeInfo("package line num", static_cast<int64_t>(lineNumInOnePackage));
        if (!package->files.empty()) {
            // Only update name of package node for first parsed file.
            if (auto packageSpec = package->files[0]->package.get()) {
                package->fullPackageName = packageSpec->GetPackageName();
                package->accessible = !packageSpec->modifier                  ? AccessLevel::PUBLIC
                    : packageSpec->modifier->modifier == TokenKind::PROTECTED ? AccessLevel::PROTECTED
                    : packageSpec->modifier->modifier == TokenKind::INTERNAL  ? AccessLevel::INTERNAL
                                                                              : AccessLevel::PUBLIC;
            }
        }
        // Checking package consistency: The macro definition package cannot contain the declaration of a common
        // package.
        CheckPackageConsistency(*package);
        return package;
    }

    void CheckPackageConsistency(Package& package) const
    {
        if (package.files.empty() || !package.files[0]->package) {
            return;
        }
        for (const auto& file : package.files) {
            if (file->package && package.files[0]->package->hasMacro != file->package->hasMacro) {
                (void)s.ci->diag.DiagnoseRefactor(DiagKindRefactor::package_name_inconsistent_with_macro, file->begin);
                return;
            }
        }
        package.isMacroPackage = package.files[0]->package->hasMacro;
    }

    OwnedPtr<Package> MultiThreadParseOnePackage(
        std::queue<std::tuple<std::string, unsigned>>& fileInfoQueue, const std::string& defaultPackageName) const
    {
        std::queue<std::future<std::tuple<OwnedPtr<File>, TokenVecMap, size_t>>> futureQueue;
        while (!fileInfoQueue.empty()) {
            auto curFile = fileInfoQueue.front();
            futureQueue.push(
                std::async(std::launch::async, [this, curFile]() -> std::tuple<OwnedPtr<File>, TokenVecMap, size_t> {
#if (defined RELEASE)
#if (defined __unix__)
                    // Since alternate signal stack is per thread, we have to create an alternate signal stack for each
                    // thread.
                    Codira::CreateAltSignalStack();
#elif _WIN32
                    // When the SIGABRT, SIGFPE, SIGSEGV and SIGILL signals are triggered in a subthread,
                    // the signals cannot be captured and the process exits directly. Therefore,
                    // the signal processing function must be set for each thread.
                    Codira::RegisterCrashSignalHandler();
#endif
#endif
                    auto parser = CreateParser(curFile);
                    parser->SetCompileOptions(s.ci->invocation.globalOptions);
                    auto file = parser->ParseTopLevel();
#ifdef SIGNAL_TEST
                    // The interrupt signal triggers the function. In normal cases, this function does not take effect.
                    Codira::SignalTest::ExecuteSignalTestCallbackFunc(
                        Codira::SignalTest::TriggerPointer::PARSER_POINTER);
#endif
                    return {std::move(file), parser->GetCommentsMap(), parser->GetLineNum()};
                }));
            fileInfoQueue.pop();
        }

        auto package = GetMultiThreadParseOnePackage(futureQueue, defaultPackageName);
        return package;
    }

    OwnedPtr<Parser> CreateParser(const std::tuple<std::string, unsigned>& curFile) const
    {
        return MakeOwned<Parser>(std::get<1>(curFile), std::get<0>(curFile), s.ci->diag, s.ci->GetSourceManager(),
            s.ci->invocation.globalOptions.enableAddCommentToAst, s.ci->invocation.globalOptions.compileCoded);
    }

    OwnedPtr<Package> ParseOnePackage(
        const std::vector<std::string>& files, bool& success, const std::string& defaultPackageName)
    {
        std::queue<std::tuple<std::string, unsigned>> fileInfoQueue;

        // Parse source code files to File node list.
        if (s.ci->loadSrcFilesFromCache) {
            for (auto& it : s.ci->bufferCache) {
                const unsigned int fileID = s.ci->GetSourceManager().AddSource(it.first, it.second);
                if (s.fileIds.count(fileID) > 0) {
                    (void)s.ci->diag.DiagnoseRefactor(
                        DiagKindRefactor::module_read_file_conflicted, DEFAULT_POSITION, it.first);
                }
                (void)s.fileIds.insert(fileID);
                fileInfoQueue.emplace(it.second, fileID);
            }
        } else {
            // The readdir cannot guarantee stable order of inputted files, need sort before adding to sourceManager.
            std::vector<std::string> parseFiles{files};
            std::sort(parseFiles.begin(), parseFiles.end(),
                [&](auto& f, auto& second) { return GetFileName(f) < GetFileName(second); });
            std::for_each(parseFiles.begin(), parseFiles.end(), [this, &success, &fileInfoQueue](auto file) {
                std::string failedReason;
                auto content = ReadFileContent(file, failedReason);
                if (!content.has_value()) {
                    s.ci->diag.DiagnoseRefactor(
                        DiagKindRefactor::module_read_file_to_buffer_failed, DEFAULT_POSITION, file, failedReason);
                    success = false;
                    return;
                }
                const unsigned int fileID = s.ci->GetSourceManager().AddSource(file | IdenticalFunc, content.value());
                if (s.fileIds.count(fileID) > 0) {
                    (void)s.ci->diag.DiagnoseRefactor(
                        DiagKindRefactor::module_read_file_conflicted, DEFAULT_POSITION, file);
                    return;
                }

                (void)s.fileIds.insert(fileID);
                fileInfoQueue.emplace(std::move(content.value()), fileID);
            });
        }

        auto package = MultiThreadParseOnePackage(fileInfoQueue, defaultPackageName);
        s.ci->diag.EmitCategoryGroup();
        std::sort(package->files.begin(), package->files.end(),
            [](const OwnedPtr<File>& fileOne, const OwnedPtr<File>& fileTwo) {
                return fileOne->fileName < fileTwo->fileName;
            });
        return package;
    }
    FullCompileStrategy& s;
};
} // namespace Codira

FullCompileStrategy::FullCompileStrategy(CompilerInstance* ci)
    : CompileStrategy(ci), impl{new FullCompileStrategyImpl{*this}}
{
    type = StrategyType::FULL_COMPILE;
}

FullCompileStrategy::~FullCompileStrategy()
{
    delete impl;
}

bool FullCompileStrategy::Parse()
{
    if (!impl->PreReadCommonPartCodeo()) {
        return false;
    }
    bool ret = true;
    if (ci->loadSrcFilesFromCache || ci->compileOnePackageFromSrcFiles) {
        auto package = impl->ParseOnePackage(ci->srcFilePaths, ret, DEFAULT_PACKAGE_NAME);
        ci->srcPkgs.emplace_back(std::move(package));
    } else {
        impl->ParseModule(ret);
    }
    return ret;
}

bool CompileStrategy::ImportPackages() const
{
    auto ret = ci->ImportPackages();
    ParseAndMergeCodeds();
    return ret;
}

namespace {
// All instance objects share, do not clean. The coded content of the same process should not be inconsistent.
std::unordered_map<std::string, OwnedPtr<Package>> g_codedAstCache;
std::mutex g_codedAstCacheLock;
std::mutex g_sourceManageLock;

void ParseAndMergeCoded(Ptr<CompilerInstance> ci, std::pair<const std::string, std::string> codedInfo)
{
    std::string failedReason;
    auto codeoPath = codedInfo.second;
    auto sourceCode = FileUtil::ReadFileContent(codeoPath, failedReason);
    if (!failedReason.empty() || !sourceCode.has_value()) {
        // In the LSP scenario, the coded file path cannot be obtained based on the dependency package information
        // configured in the cache. The coded file path is searched in searchPath.
        auto searchPath = ci->importManager.GetSearchPath();
        auto codedPath = FileUtil::FindSerializationFile(codedInfo.first, CODE_D_FILE_EXTENSION, searchPath);
        if (codedPath.empty()) {
            return;
        }
        sourceCode = FileUtil::ReadFileContent(codedPath, failedReason);
        if (!failedReason.empty() || !sourceCode.has_value()) {
            return;
        }
    }

    // Parse
    unsigned int fileId = 0;
    SourceManager& sm = ci->diag.GetSourceManager();
    {
        std::lock_guard<std::mutex> guardOfSm(g_sourceManageLock);
        fileId = sm.AddSource(codeoPath, sourceCode.value(), codedInfo.first);
    }
    auto fileAst =
        Parser(fileId, sourceCode.value(), ci->diag, ci->diag.GetSourceManager(), false, true).ParseTopLevel();
    auto pkg = MakeOwned<Package>(codedInfo.first);
    fileAst->curPackage = pkg.get();
    pkg->files.emplace_back(std::move(fileAst));
    auto originPkg = ci->importManager.GetPackage(codedInfo.first);
    if (!originPkg) {
        InternalError(codedInfo.first + " cannot find origin ast");
    }
    MergeCusAnno(originPkg, pkg.get());
    {
        std::lock_guard<std::mutex> guard(g_codedAstCacheLock);
        g_codedAstCache[codedInfo.first] = std::move(pkg);
    }
}
} // namespace

void CompileStrategy::ParseAndMergeCodeds() const
{
    auto& option = ci->invocation.globalOptions;
    auto hasLevelFlg = option.passedWhenKeyValue.find("APILevel_level") != option.passedWhenKeyValue.end();
    auto hasSyscapFlg = option.passedWhenKeyValue.find("APILevel_syscap") != option.passedWhenKeyValue.end();
    if (!hasLevelFlg && !hasSyscapFlg) {
        return;
    }
    Utils::ProfileRecorder::Start("ImportPackages", "ParseAndMergeCodeds");
    auto codedInfos = ci->importManager.GetDepPkgCodedPaths();
    std::vector<std::future<void>> futures;
    futures.reserve(codedInfos.size());
    // Reuse current CompilerInstance, but the Parser in the macro expansion phase uses the DParser.
    option.compileCoded = true;
    // codedInfos is [fullPackageName, codedPath].
    for (auto& codedInfo : codedInfos) {
        if (option.jobs == 1) {
            ParseAndMergeCoded(ci, codedInfo);
        } else {
            // In the LSP scenario, concurrent calls may occur.
            std::lock_guard<std::mutex> guard(g_codedAstCacheLock);
            auto [iter, succ] = g_codedAstCache.try_emplace(codedInfo.first, nullptr);
            if (!succ && iter->second) {
                auto originPkg = ci->importManager.GetPackage(codedInfo.first);
                if (!originPkg) {
                    InternalError(codedInfo.first + " cannot find origin ast");
                }
                MergeCusAnno(originPkg, iter->second.get());
            } else {
                futures.emplace_back(std::async(std::launch::async, ParseAndMergeCoded, ci, codedInfo));
            }
        }
    }
    for (auto& future : futures) {
        future.get();
    }
    ci->diag.EmitCategoryGroup();
    option.compileCoded = false;
    Utils::ProfileRecorder::Stop("ImportPackages", "ParseAndMergeCodeds");
}

bool CompileStrategy::MacroExpand() const
{
    auto beforeErrCnt = ci->diag.GetErrorCount();
    MacroExpansion me(ci);
    me.Execute(ci->srcPkgs);
    ci->diag.EmitCategoryDiagnostics(DiagCategory::PARSE);

#if defined(CMAKE_ENABLE_ASSERT) || !defined(NDEBUG)
    AST::ASTChecker astChecker;
    astChecker.CheckAST(ci->srcPkgs);
    astChecker.CheckBeginEnd(ci->srcPkgs);
#endif

    ci->tokensEvalInMacro = me.tokensEvalInMacro;
    bool hasNoMacroErr = beforeErrCnt == ci->diag.GetErrorCount();
    return hasNoMacroErr;
}

bool FullCompileStrategy::Sema()
{
    {
        Utils::ProfileRecorder recorder("Semantic", "Desugar Before TypeCheck");
        PerformDesugar();
    }
    TypeCheck();
#ifdef SIGNAL_TEST
    // The interrupt signal triggers the function. In normal cases, this function does not take effect.
    Codira::SignalTest::ExecuteSignalTestCallbackFunc(Codira::SignalTest::TriggerPointer::SEMA_POINTER);
#endif
    // Report number of warnings and errors.
    if (ci->diag.GetErrorCount() != 0) {
        return false;
    }
    return true;
}
