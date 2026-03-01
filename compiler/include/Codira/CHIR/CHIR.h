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
 * This file declares the entry of CHIR.
 */

#ifndef CODIRA_CHIR_CHIR_H
#define CODIRA_CHIR_CHIR_H

#include "Codira/CHIR/AST2CHIR/AST2CHIR.h"
#include "Codira/CHIR/Analysis/ValueRangeAnalysis.h"
#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/DiagAdapter.h"

namespace Codira::CHIR {
class ToCHIR {
public:
    ToCHIR(CompilerInstance& ci, AST::Package& pkg, AnalysisWrapper<ConstAnalysis, ConstDomain>& constAnalysisWrapper,
        CHIRBuilder& builder)
        : ci(ci),
          opts(ci.invocation.globalOptions),
          typeManager(ci.typeManager),
          sourceManager(ci.GetSourceManager()),
          importManager(ci.importManager),
          gim(ci.gim),
          diagEngine(ci.diag),
          codiraHome(ci.codiraHome),
          pkg(pkg),
          outputPath(ci.invocation.globalOptions.output),
          kind(ci.kind),
          cachedInfo(ci.cachedInfo),
          releaseCHIRMemory(ci.releaseCHIRMemory),
          needToOptString(ci.needToOptString),
          needToOptGenericDecl(ci.needToOptGenericDecl),
          builder(builder),
          constAnalysisWrapper(constAnalysisWrapper),
          diag(diagEngine)
    {
    }
    ~ToCHIR() = default;

    bool Run();

    /// Compute Annotation values, and save the results for AOP checkings.
    /// Only AST2CHIR and necessary CHIR opt's are executed.
    bool ComputeAnnotations(std::vector<const AST::Decl*>&& annoOnly);

    CHIR::Package* GetPackage() const
    {
        return chirPkg;
    }

    OptEffectStrMap GetOptEffectMap() const
    {
        return strEffectMap;
    }

    VirtualWrapperDepMap GetCurVirtualFuncWrapperDepForIncr()
    {
        return curVirtFuncWrapDep;
    }

    VirtualWrapperDepMap GetDeleteVirtualFuncWrapperForIncr()
    {
        return delVirtFuncWrapForIncr;
    }

    std::set<std::string> GetCCOutFuncsRawMangle()
    {
        return ccOutFuncsRawMangle;
    }

    VarInitDepMap GetVarInitDepMap() const;
    std::vector<std::unique_ptr<CHIR::CHIRBuilder>> ConstructSubBuilders(size_t threadNum, size_t funcNum)
    {
        std::vector<Codira::Utils::TaskResult<std::unique_ptr<CHIR::CHIRBuilder>>> results;
        Utils::TaskQueue builderTaskQueue(threadNum);

        for (size_t i = 0; i < funcNum; i++) {
            results.emplace_back(builderTaskQueue.AddTask<std::unique_ptr<CHIR::CHIRBuilder>>(
                [this, i]() { return std::make_unique<CHIR::CHIRBuilder>(builder.GetChirContext(), i); }));
        }
        builderTaskQueue.RunAndWaitForAllTasksCompleted();
        std::vector<std::unique_ptr<CHIR::CHIRBuilder>> builderList;
        for (auto& result : results) {
            auto res = result.get();
            builderList.emplace_back(std::move(res));
        }
        return builderList;
    }

    std::unordered_map<std::string, CHIR::FuncBase*> GetImplicitFuncs() const
    {
        return implicitFuncs;
    }

    std::vector<CHIR::FuncBase*> GetConstVarInitFuncs() const
    {
        return initFuncsForConstVar;
    }
    const std::vector<std::pair<const AST::Decl*, Func*>>& GetAnnoFactoryFuncs() const
    {
        return annoFactoryFuncs;
    }

    const AST2CHIRNodeMap<CustomTypeDef>& GetGlobalNominalCache() const
    {
        return globalNominalCache;
    }

    enum Phase : uint8_t {
        RAW, // after translation,
        OPT, // after compiler optimization,
        PLUGIN, // after perform pulgin
        ANALYSIS_FOR_CODELINT, // after analysis for codelint
        PHASE_MIN = RAW,
        PHASE_MAX = ANALYSIS_FOR_CODELINT,
    };

private:
    /// \param annoOnly pass the decls of which only annoFactoryFuncs are to be translated, during
    /// computing annotations stage. Empty in normal AST2CHIR translation.
    bool TranslateToCHIR(std::vector<const AST::Decl*>&& annoOnly);
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    bool PerformPlugin(CHIR::Package& package);
#endif
    void DumpCHIRToFile(const std::string& suffix, bool checkFlag = true);
    void DoClosureConversion();
    void ReportUnusedCode();
    void Devirtualization(DevirtualizationInfo& devirtInfo);
    void UnreachableBlockElimination();
    void UnreachableBlockReporter();
    void NothingTypeExprElimination();
    void UselessExprElimination();
    void UnreachableBranchReporter();
    void UselessFuncElimination();
    void RedundantLoadElimination();
    void UselessAllocateElimination();
    void RunGetRefToArrayElemOpt();
    void RedundantGetOrThrowElimination();
    void FlatForInExpr();
    void RunUnreachableMarkBlockRemoval();
    void RunMarkClassHasInited();
    void RunMergingBlocks(const std::string& firstName, const std::string& secondName);
    bool RunVarInitChecking();
    bool RunConstantPropagationAndSafetyCheck();
    bool RunConstantPropagation();
    void RunRangePropagation();
    bool RunNativeFFIChecks();
    void RunArrayListConstStartOpt();
    void RunFunctionInline(DevirtualizationInfo& devirtInfo);
    void RunArrayLambdaOpt();
    void RunRedundantFutureOpt();
    void RunNoSideEffectMarkerOpt();
    void RunSanitizerCoverage();
    bool RunOptimizationPassAndRulesChecking();
    void MarkNoSideEffect();
    void RunUnitUnify();
    DevirtualizationInfo CollectDevirtualizationInfo();
    bool RunConstantEvaluation();
    bool RunIRChecker(const Phase& phase);
    void UpdatePosOfMacroExpandNode();
    void RecordCodeInfoAtTheBegin();
    void RecordCodeInfoAtTheEnd();
    void RecordCHIRExprNum(const std::string& suffix);
    bool RunAnalysisForCODELint();
    void RunConstantAnalysis();
    // run semantic checks that have to be performed on CHIR
    bool RunAnnotationChecks();
    void EraseDebugExpr();
    void CFFIFuncWrapper();
    void ReplaceSrcCodeImportedValueWithSymbol();
    void CreateBoxTypeForRecursionValueType();
    void CreateVTableAndUpdateFuncCall();
    void UpdateMemberVarPath();

    template <typename T>
    std::pair<Value*, Apply*> DoCFFIFuncWrapper(T& curFunc, bool isForeign, bool isExternal = true);

    template <typename T> bool IsAllApply(const T* curFunc);

    CompilerInstance& ci;
    const GlobalOptions& opts;
    TypeManager* typeManager;
    SourceManager& sourceManager;
    ImportManager& importManager;
    const GenericInstantiationManager* gim;
    DiagnosticEngine& diagEngine;
    const std::string& codiraHome;
    AST::Package& pkg;
    std::string outputPath;
    IncreKind kind;
    CompilationCache& cachedInfo;
    uint64_t ccEnvCounter = 0;
    CHIR::Package* chirPkg{nullptr};
    bool releaseCHIRMemory = true;
    // This flag is served for const propagation. The codira kernel const propagation doesn't need to optimize
    // string, but the codelint need to do it. This flag is for differentiating this behavior.
    bool needToOptString = false;
    bool needToOptGenericDecl = false;
    CHIRBuilder& builder;
    uint64_t debugFileIndex{0};
    AnalysisWrapper<ConstAnalysis, ConstDomain>& constAnalysisWrapper;
    OptEffectCHIRMap effectMap;
    OptEffectStrMap strEffectMap;
    VirtualWrapperDepMap curVirtFuncWrapDep;
    VirtualWrapperDepMap delVirtFuncWrapForIncr;
    // Raw mangled name of top or mem funcs had closure convert. If there is
    // any change in incremental compilation, rollback is required.
    std::set<std::string> ccOutFuncsRawMangle;
    class DiagAdapter diag;
    std::unordered_set<Func*> srcCodeImportedFuncs;
    std::unordered_set<GlobalVar*> srcCodeImportedVars;
    std::unordered_set<ClassDef*> uselessClasses;
    std::unordered_set<Func*> uselessLambda;
    std::unordered_map<std::string, FuncBase*> implicitFuncs;
    std::vector<CHIR::FuncBase*> initFuncsForConstVar;
    std::unordered_map<Block*, Terminator*> maybeUnreachable;
    /// Whether this CHIR convertor is translating Annotations
    bool isComputingAnnos{false};
    std::vector<std::pair<const AST::Decl*, Func*>> annoFactoryFuncs;
    AST2CHIRNodeMap<CustomTypeDef> globalNominalCache;
};
} // namespace Codira::CHIR
#endif
