//===- NewPMDriver.cpp - Driver for llc using new PM ----------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file is just a split of the code that logically belongs in llc.cpp but
/// that includes the new pass manager headers.
///
//===----------------------------------------------------------------------===//

#include "NewPMDriver.h"
#include "vm/core/Analysis/CGSCCPassManager.h"
#include "vm/core/Analysis/RuntimeLibcallInfo.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/CodeGen/CommandFlags.h"
#include "vm/core/CodeGen/LibcallLoweringInfo.h"
#include "vm/core/CodeGen/MIRParser/MIRParser.h"
#include "vm/core/CodeGen/MIRPrinter.h"
#include "vm/core/CodeGen/MachineFunctionAnalysis.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/MachinePassManager.h"
#include "vm/core/CodeGen/MachineVerifier.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/DiagnosticPrinter.h"
#include "vm/core/IR/IRPrintingPasses.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/Verifier.h"
#include "vm/core/IRReader/IRReader.h"
#include "vm/core/Passes/PassBuilder.h"
#include "vm/core/Passes/StandardInstrumentations.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/FormattedStream.h"
#include "vm/core/Support/ToolOutputFile.h"
#include "vm/core/Support/WithColor.h"
#include "vm/core/Target/CGPassBuilderOption.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Target/TargetOptions.h"
#include "vm/core/Transforms/Scalar/LoopPassManager.h"
#include "vm/core/Transforms/Utils/Cloning.h"

using namespace vm::core;

static cl::opt<RegAllocType, false, RegAllocTypeParser>
RegAlloc("regalloc-npm",
    cl::desc("Register allocator to use for new pass manager"),
    cl::Hidden, cl::init(RegAllocType::Unset));

static cl::opt<bool>
DebugPM("debug-pass-manager", cl::Hidden,
    cl::desc("Print pass management debugging information"));

bool LLCDiagnosticHandler::handleDiagnostics(const DiagnosticInfo& DI) {
    DiagnosticHandler::handleDiagnostics(DI);
    if (DI.getKind() == llvm::DK_SrcMgr) {
        const auto& DISM = cast<DiagnosticInfoSrcMgr>(DI);
        const SMDiagnostic& SMD = DISM.getSMDiag();

        SMD.print(nullptr, errs());

        // For testing purposes, we print the LocCookie here.
        if (DISM.isInlineAsmDiag() && DISM.getLocCookie())
            WithColor::note() << "!srcloc = " << DISM.getLocCookie() << "\n";

        return true;
    }

    if (auto* Remark = dyn_cast<DiagnosticInfoOptimizationBase>(&DI))
        if (!Remark->isEnabled())
            return true;

    DiagnosticPrinterRawOStream DP(errs());
    errs() << LLVMContext::getDiagnosticMessagePrefix(DI.getSeverity()) << ": ";
    DI.print(DP);
    errs() << "\n";
    return true;
}

static llvm::ExitOnError ExitOnErr;

int llvm::compileModuleWithNewPM(
    StringRef Arg0, std::unique_ptr<Module> M, std::unique_ptr<MIRParser> MIR,
    std::unique_ptr<TargetMachine> Target, std::unique_ptr<ToolOutputFile> Out,
    std::unique_ptr<ToolOutputFile> DwoOut, LLVMContext& Context,
    const TargetLibraryInfoImpl& TLII, VerifierKind VK, StringRef PassPipeline,
    CodeGenFileType FileType) {

    if (!PassPipeline.empty() && TargetPassConfig::hasLimitedCodeGenPipeline()) {
        WithColor::error(errs(), Arg0)
            << "--passes cannot be used with "
            << TargetPassConfig::getLimitedCodeGenPipelineReason() << ".\n";
        return 1;
    }

    raw_pwrite_stream* OS = &Out->os();

    std::unique_ptr<buffer_ostream> BOS;
    if (codegen::getFileType() != CodeGenFileType::AssemblyFile &&
        !Out->os().supportsSeeking()) {
        BOS = std::make_unique<buffer_ostream>(Out->os());
        OS = BOS.get();
    }

    // Fetch options from TargetPassConfig
    CGPassBuilderOption Opt = getCGPassBuilderOption();
    Opt.DisableVerify = VK != VerifierKind::InputOutput;
    Opt.DebugPM = DebugPM;
    Opt.RegAlloc = RegAlloc;

    MachineModuleInfo MMI(Target.get());

    PassInstrumentationCallbacks PIC;
    StandardInstrumentations SI(Context, Opt.DebugPM,
        VK == VerifierKind::EachPass);
    registerCodeGenCallback(PIC, *Target);

    MachineFunctionAnalysisManager MFAM;
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;
    PassBuilder PB(Target.get(), PipelineTuningOptions(), std::nullopt, &PIC);
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.registerMachineFunctionAnalyses(MFAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM, &MFAM);
    SI.registerCallbacks(PIC, &MAM);

    FAM.registerPass([&] { return TargetLibraryAnalysis(TLII); });

    MAM.registerPass([&] {
        const TargetOptions& Options = Target->Options;
        return RuntimeLibraryAnalysis(
            M->getTargetTriple(), Target->Options.ExceptionModel,
            Target->Options.FloatABIType, Target->Options.EABIVersion,
            Options.MCOptions.ABIName, Target->Options.VecLib);
        });
    MAM.registerPass([&] { return LibcallLoweringModuleAnalysis(); });

    MAM.registerPass([&] { return MachineModuleAnalysis(MMI); });

    ModulePassManager MPM;
    FunctionPassManager FPM;

    if (!PassPipeline.empty()) {
        // Construct a custom pass pipeline that starts after instruction
        // selection.

        if (!MIR) {
            WithColor::error(errs(), Arg0) << "-passes is for .mir file only.\n";
            return 1;
        }

        // FIXME: verify that there are no IR passes.
        ExitOnErr(PB.parsePassPipeline(MPM, PassPipeline));
        MPM.addPass(PrintMIRPreparePass(*OS));
        MachineFunctionPassManager MFPM;
        if (VK == VerifierKind::InputOutput)
            MFPM.addPass(MachineVerifierPass());
        MFPM.addPass(PrintMIRPass(*OS));
        FPM.addPass(createFunctionToMachineFunctionPassAdaptor(std::move(MFPM)));
        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));

    }
    else {
        ExitOnErr(
            Target->buildCodeGenPipeline(MPM, *OS, DwoOut ? &DwoOut->os() : nullptr,
                FileType, Opt, MMI.getContext(), &PIC));
    }

    // If user only wants to print the pipeline, print it before parsing the MIR.
    if (PrintPipelinePasses) {
        std::string PipelineStr;
        raw_string_ostream OS(PipelineStr);
        MPM.printPipeline(OS, [&PIC](StringRef ClassName) {
            auto PassName = PIC.getPassNameForClassName(ClassName);
            return PassName.empty() ? ClassName : PassName;
            });
        outs() << PipelineStr << '\n';
        return 0;
    }

    if (MIR && MIR->parseMachineFunctions(*M, MAM))
        return 1;

    // Before executing passes, print the final values of the LLVM options.
    cl::PrintOptionValues();

    MPM.run(*M, MAM);

    if (Context.getDiagHandlerPtr()->HasErrors)
        return 1;

    // Declare success.
    Out->keep();
    if (DwoOut)
        DwoOut->keep();

    return 0;
}