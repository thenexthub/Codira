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
 * This file implements the DefaultCompilerInstance.
 */

#include "Codira/FrontendTool/DefaultCompilerInstance.h"

#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Verifier.h"

#include "Codira/Basic/StringConvertor.h"
#include "Codira/CodeGen/EmitPackageIR.h"
#include "Codira/Driver/StdlibMap.h"
#include "Codira/Driver/TempFileManager.h"
#include "Codira/Modules/PackageManager.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/ProfileRecorder.h"
#include "Codira/AST/Walker.h"

#if (defined RELEASE)
#include "Codira/Utils/Signal.h"
#endif

using namespace Codira;
using namespace AST;

namespace Codira {
class DefaultCIImpl final {
public:
    explicit DefaultCIImpl(DefaultCompilerInstance& ref) : ci{ref}
    {
    }
    ~DefaultCIImpl();

    bool PerformCodeGen();
    bool PerformCodeoAndBchirSaving();
    void DumpDepPackage();
    bool SaveCodeoAndBchir(AST::Package& pkg);
    bool SaveCodeo(const AST::Package& pkg);
    void RearrangeImportedPackageDependence();
    bool CodegenOnePackage(AST::Package& pkg, bool enableIncrement);

private:
    DefaultCompilerInstance& ci;

    bool EmitLLVMSimilarBytecode(AST::Package& pkg, bool enableIncrement);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    void SaveBchir([[maybe_unused]] const AST::Package& pkg) const
    {
    }
#endif
    std::string GenerateFileName(const std::string& fullPackageName, const std::string& idx) const;
    std::string GenerateBCFilePathAndUpdateToInvocation(
        const TempFileKind& kind, const std::string& pkgName, const std::string& idx = "");

    std::vector<std::unique_ptr<llvm::Module>> llvmModules;
};

DefaultCompilerInstance::DefaultCompilerInstance(CompilerInvocation& invocation, DiagnosticEngine& diag)
    : CompilerInstance(invocation, diag), impl{new DefaultCIImpl{*this}}
{
    buildTrie = false;
}

DefaultCIImpl::~DefaultCIImpl()
{
    CodeGen::ClearPackageModules(llvmModules);
}
DefaultCompilerInstance::~DefaultCompilerInstance()
{
    delete impl;
}

bool DefaultCompilerInstance::PerformParse()
{
    Utils::ProfileRecorder recorder("Main Stage", "Parser");
    return CompilerInstance::PerformParse();
}

bool DefaultCompilerInstance::PerformConditionCompile()
{
    Utils::ProfileRecorder recorder("Main Stage", "ConditionalCompilation");
    return CompilerInstance::PerformConditionCompile();
}

bool DefaultCompilerInstance::PerformImportPackage()
{
    Utils::ProfileRecorder recorder("Main Stage", "ImportPackages");
    return CompilerInstance::PerformImportPackage();
}

bool DefaultCompilerInstance::PerformMacroExpand()
{
    Utils::ProfileRecorder recorder("Main Stage", "MacroExpand");
    return CompilerInstance::PerformMacroExpand();
}

bool DefaultCompilerInstance::PerformSema()
{
    Utils::ProfileRecorder recorder("Main Stage", "Semantic");
    return CompilerInstance::PerformSema();
}

bool DefaultCompilerInstance::PerformOverflowStrategy()
{
    Utils::ProfileRecorder recorder("Main Stage", "Overflow Strategy");
    return CompilerInstance::PerformOverflowStrategy();
}

bool DefaultCompilerInstance::PerformDesugarAfterSema()
{
    Utils::ProfileRecorder recorder("Main Stage", "Desugar after Sema");
    return CompilerInstance::PerformDesugarAfterSema();
}

bool DefaultCompilerInstance::PerformGenericInstantiation()
{
    Utils::ProfileRecorder recorder("Main Stage", "Generic Instantiation");
    return CompilerInstance::PerformGenericInstantiation();
}

bool DefaultCompilerInstance::PerformCHIRCompilation()
{
    Utils::ProfileRecorder recorder("Main Stage", "CHIR");
    return CompilerInstance::PerformCHIRCompilation();
}

std::string DefaultCIImpl::GenerateFileName(const std::string& fullPackageName, const std::string& idx) const
{
    std::string fileName;
    auto pkgNameSuffix = FileUtil::ToCodeoFileName(fullPackageName);
    if (ci.invocation.globalOptions.compilePackage) {
        fileName = (idx.empty() ? "" : (idx + "-")) + pkgNameSuffix;
    } else if (fullPackageName != DEFAULT_PACKAGE_NAME) {
        fileName = (idx.empty() ? "" : (idx + "-")) + pkgNameSuffix;
    } else if (ci.invocation.globalOptions.srcFiles.empty()) {
        fileName = (idx.empty() ? "" : (idx + "-")) + pkgNameSuffix;
    } else {
        fileName = (idx.empty() ? "" : (idx + "-")) +
            FileUtil::GetFileNameWithoutExtension(ci.invocation.globalOptions.srcFiles[0]);
    }
    return fileName;
}

std::string DefaultCIImpl::GenerateBCFilePathAndUpdateToInvocation(
    const TempFileKind& kind, const std::string& pkgName, const std::string& idx)
{
    std::string fileName = GenerateFileName(pkgName, idx);
    TempFileInfo bcFileInfo = TempFileManager::Instance().CreateNewFileInfo(TempFileInfo{fileName, "", "", true}, kind);
    ci.invocation.globalOptions.frontendOutputFiles.emplace_back(bcFileInfo);

    auto bcFilePath = bcFileInfo.filePath;
    if (FileUtil::FileExist(bcFilePath) && !FileUtil::Remove(bcFilePath)) {
        Errorln("The file " + bcFilePath + " already exists, but it fails to be removed before being updated.");
        return "";
    }
    if (auto dir{FileUtil::GetDirPath(bcFilePath)}; !FileUtil::FileExist(dir) && FileUtil::CreateDirs(dir + "/") != 0) {
        Errorln("The directory " + dir + " fails to be created before creating " + bcFilePath);
        return "";
    }
    // If file is deleted, no more data written to the file.
    if (TempFileManager::Instance().IsDeleted()) {
        return "";
    }
    return bcFilePath;
}

bool DefaultCIImpl::SaveCodeo(const AST::Package& pkg)
{
    if (pkg.IsEmpty()) {
        return true;
    }
    auto pkgName = FileUtil::ToCodeoFileName(pkg.fullPackageName);
    // If compiled with the `-g` or '--coverage', files should be saved with absolute paths.
    // When compiling stdlib without options '--coverage', do not save file with abs path.
    // Then can not debugging stdlib with abs path.
    bool saveFileWithAbsPath =
        ci.invocation.globalOptions.enableCompileDebug || ci.invocation.globalOptions.enableCoverage;
    if ((STANDARD_LIBS.find(pkg.fullPackageName) != STANDARD_LIBS.end())) {
        saveFileWithAbsPath = ci.invocation.globalOptions.enableCoverage;
    }
    std::vector<uint8_t> astData;
    Utils::ProfileRecorder::Start("Save codeo and bchir", "Serialize ast");
    ci.importManager.ExportAST(saveFileWithAbsPath, astData, pkg);
    Utils::ProfileRecorder::Stop("Save codeo and bchir", "Serialize ast");
    // Write astData into file according to given package name by '--output' opt.
    TempFileInfo astFileInfo =
        TempFileManager::Instance().CreateNewFileInfo(TempFileInfo{pkgName, ""}, TempFileKind::O_CODEO);
    std::string astFileName = astFileInfo.filePath;
    Utils::ProfileRecorder::Start("Save codeo and bchir", "Save ast");
    bool res = FileUtil::WriteBufferToASTFile(astFileName, astData);
    Utils::ProfileRecorder::Stop("Save codeo and bchir", "Save ast");
    if (!res) {
        Errorln("fail to generate file: " + astFileName);
    }
    return res;
}

bool DefaultCIImpl::SaveCodeoAndBchir(Package& pkg)
{
    SaveBchir(pkg);
    return SaveCodeo(pkg);
}

bool DefaultCIImpl::CodegenOnePackage(Package& pkg, bool enableIncrement)
{
    if (pkg.IsEmpty()) {
        return true;
    }
    if (ci.invocation.globalOptions.disableCodeGen) {
        return true;
    }

    auto backend = ci.invocation.globalOptions.backend;
    switch (backend) {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        case Triple::BackendType::CODENATIVE: {
            if (!EmitLLVMSimilarBytecode(pkg, enableIncrement)) {
                return false;
            }
            break;
        }
#endif
        case Triple::BackendType::UNKNOWN: {
            Errorln("unknown backend");
            break;
        }
        default:
            return false;
    }

#ifdef SIGNAL_TEST
    // The interrupt signal triggers the function. In normal cases, this function does not take effect.
    Codira::SignalTest::ExecuteSignalTestCallbackFunc(Codira::SignalTest::TriggerPointer::CODEGEN_POINTER);
#endif
    return true;
}
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
bool DefaultCIImpl::EmitLLVMSimilarBytecode(Package& pkg, bool enableIncrement)
{
    // 1. translate CHIR to LLVM IR
    CHIR::CHIRBuilder builder(ci.chirData.GetCHIRContext());
    llvmModules = CodeGen::GenPackageModules(builder, ci.chirData, ci.invocation.globalOptions, ci, enableIncrement);

    // 2. save LLVM IR to bc file
    Utils::ProfileRecorder recorder("CodeGen", "Save bc file");
    ci.invocation.globalOptions.UpdateCachedDirName(pkg.fullPackageName);
    if (llvmModules.size() == 1) {
        auto filePath = GenerateBCFilePathAndUpdateToInvocation(TempFileKind::T_BC, pkg.fullPackageName);
        if (filePath.empty()) {
            return false;
        }
        CodeGen::SavePackageModule(*llvmModules[0], filePath);
    } else {
        Utils::TaskQueue taskQueueSaveBitcode(llvmModules.size());
        std::vector<std::string> allBCFilePath;
        for (size_t i = 0; i < llvmModules.size(); ++i) {
            auto filePath =
                GenerateBCFilePathAndUpdateToInvocation(TempFileKind::T_BC, pkg.fullPackageName, std::to_string(i));
            if (filePath.empty()) {
                return false;
            }
            allBCFilePath.emplace_back(filePath);
        }
        for (size_t i = 0; i < llvmModules.size(); ++i) {
            auto& module = *llvmModules[i];
            auto& bcFilePath = allBCFilePath[i];
            taskQueueSaveBitcode.AddTask<void>(
                [&module, &bcFilePath]() { CodeGen::SavePackageModule(module, bcFilePath); });
        }
        taskQueueSaveBitcode.RunAndWaitForAllTasksCompleted();
    }

    if (ci.invocation.globalOptions.enIncrementalCompilation) {
        auto fileName = GenerateFileName(pkg.fullPackageName, "");
        ci.cachedInfo.bitcodeFilesName = std::vector<std::string>{fileName};
    }
    return true;
}
#endif

bool DefaultCompilerInstance::PerformMangling()
{
    Utils::ProfileRecorder recorder("Main Stage", "Perform Mangling");
    return CompilerInstance::PerformMangling();
}

bool DefaultCIImpl::PerformCodeGen()
{
    Utils::ProfileRecorder recorder("Main Stage", "CodeGen");
    // Before CodeGen, the dependency relationship of a package contains only some packages.
    // So this function rearranges the dependencies of all packages.
    RearrangeImportedPackageDependence();
    bool ret = true;
    for (auto& srcPkg : ci.GetSourcePackages()) {
        ret = ret && CodegenOnePackage(*srcPkg, false);
    }
    return ret;
}

bool DefaultCIImpl::PerformCodeoAndBchirSaving()
{
    Utils::ProfileRecorder recorder("Main Stage", "Save codeo and bchir");
    bool ret = true;
    for (auto& srcPkg : ci.GetSourcePackages()) {
        ret = ret && SaveCodeoAndBchir(*srcPkg);
    }
    return ret;
}

void DefaultCompilerInstance::DumpDepPackage()
{
    for (auto& depPkgInfo : GetDepPkgInfo()) {
        Println(depPkgInfo);
    }
}

void DefaultCIImpl::RearrangeImportedPackageDependence()
{
    Utils::ProfileRecorder recorder("CodeGen", "RearrangeImportedPackageDependence");
    std::vector<Ptr<Package>> allImportedPackages;
    for (auto pd : ci.importManager.GetAllImportedPackages(true)) {
        CODEC_ASSERT(pd && pd->srcPackage);
        allImportedPackages.push_back(pd->srcPackage);
    }
    ci.packageManager->ResolveDependence(allImportedPackages);
}

bool DefaultCompilerInstance::PerformCodeGen()
{
    return impl->PerformCodeGen();
}
bool DefaultCompilerInstance::PerformCodeoAndBchirSaving()
{
    return impl->PerformCodeoAndBchirSaving();
}
bool DefaultCompilerInstance::SaveCodeoAndBchir(AST::Package& pkg) const
{
    return impl->SaveCodeoAndBchir(pkg);
}
bool DefaultCompilerInstance::SaveCodeo(const AST::Package& pkg) const
{
    return impl->SaveCodeo(pkg);
}
void DefaultCompilerInstance::RearrangeImportedPackageDependence() const
{
    return impl->RearrangeImportedPackageDependence();
}
bool DefaultCompilerInstance::CodegenOnePackage(AST::Package& pkg, bool enableIncrement) const
{
    return impl->CodegenOnePackage(pkg, enableIncrement);
}
}
