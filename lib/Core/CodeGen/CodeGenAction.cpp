//===--- CodeGenAction.cpp - LLVM Code Generation Frontend Action ---------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "language/Core/CodeGen/CodeGenAction.h"
#include "BackendConsumer.h"
#include "CGCall.h"
#include "CodeGenModule.h"
#include "CoverageMappingGen.h"
#include "MacroPPCallbacks.h"
#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclGroup.h"
#include "language/Core/Basic/DiagnosticFrontend.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/LangStandard.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/CodeGen/BackendUtil.h"
#include "language/Core/CodeGen/ModuleBuilder.h"
#include "language/Core/Driver/DriverDiagnostic.h"
#include "language/Core/Frontend/CompilerInstance.h"
#include "language/Core/Frontend/MultiplexConsumer.h"
#include "language/Core/Lex/Preprocessor.h"
#include "language/Core/Serialization/ASTWriter.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/Bitcode/BitcodeReader.h"
#include "toolchain/CodeGen/MachineOptimizationRemarkEmitter.h"
#include "toolchain/Demangle/Demangle.h"
#include "toolchain/IR/DebugInfo.h"
#include "toolchain/IR/DiagnosticInfo.h"
#include "toolchain/IR/DiagnosticPrinter.h"
#include "toolchain/IR/GlobalValue.h"
#include "toolchain/IR/LLVMContext.h"
#include "toolchain/IR/LLVMRemarkStreamer.h"
#include "toolchain/IR/Module.h"
#include "toolchain/IR/Verifier.h"
#include "toolchain/IRReader/IRReader.h"
#include "toolchain/LTO/LTOBackend.h"
#include "toolchain/Linker/Linker.h"
#include "toolchain/Pass.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/SourceMgr.h"
#include "toolchain/Support/TimeProfiler.h"
#include "toolchain/Support/Timer.h"
#include "toolchain/Support/ToolOutputFile.h"
#include "toolchain/Transforms/IPO/Internalize.h"
#include "toolchain/Transforms/Utils/Cloning.h"

#include <optional>
using namespace language::Core;
using namespace toolchain;

#define DEBUG_TYPE "codegenaction"

namespace language::Core {
class BackendConsumer;
class ClangDiagnosticHandler final : public DiagnosticHandler {
public:
  ClangDiagnosticHandler(const CodeGenOptions &CGOpts, BackendConsumer *BCon)
      : CodeGenOpts(CGOpts), BackendCon(BCon) {}

  bool handleDiagnostics(const DiagnosticInfo &DI) override;

  bool isAnalysisRemarkEnabled(StringRef PassName) const override {
    return CodeGenOpts.OptimizationRemarkAnalysis.patternMatches(PassName);
  }
  bool isMissedOptRemarkEnabled(StringRef PassName) const override {
    return CodeGenOpts.OptimizationRemarkMissed.patternMatches(PassName);
  }
  bool isPassedOptRemarkEnabled(StringRef PassName) const override {
    return CodeGenOpts.OptimizationRemark.patternMatches(PassName);
  }

  bool isAnyRemarkEnabled() const override {
    return CodeGenOpts.OptimizationRemarkAnalysis.hasValidPattern() ||
           CodeGenOpts.OptimizationRemarkMissed.hasValidPattern() ||
           CodeGenOpts.OptimizationRemark.hasValidPattern();
  }

private:
  const CodeGenOptions &CodeGenOpts;
  BackendConsumer *BackendCon;
};

static void reportOptRecordError(Error E, DiagnosticsEngine &Diags,
                                 const CodeGenOptions &CodeGenOpts) {
  handleAllErrors(
      std::move(E),
    [&](const LLVMRemarkSetupFileError &E) {
        Diags.Report(diag::err_cannot_open_file)
            << CodeGenOpts.OptRecordFile << E.message();
      },
    [&](const LLVMRemarkSetupPatternError &E) {
        Diags.Report(diag::err_drv_optimization_remark_pattern)
            << E.message() << CodeGenOpts.OptRecordPasses;
      },
    [&](const LLVMRemarkSetupFormatError &E) {
        Diags.Report(diag::err_drv_optimization_remark_format)
            << CodeGenOpts.OptRecordFormat;
      });
}

BackendConsumer::BackendConsumer(CompilerInstance &CI, BackendAction Action,
                                 IntrusiveRefCntPtr<toolchain::vfs::FileSystem> VFS,
                                 LLVMContext &C,
                                 SmallVector<LinkModule, 4> LinkModules,
                                 StringRef InFile,
                                 std::unique_ptr<raw_pwrite_stream> OS,
                                 CoverageSourceInfo *CoverageInfo,
                                 toolchain::Module *CurLinkModule)
    : CI(CI), Diags(CI.getDiagnostics()), CodeGenOpts(CI.getCodeGenOpts()),
      TargetOpts(CI.getTargetOpts()), LangOpts(CI.getLangOpts()),
      AsmOutStream(std::move(OS)), FS(VFS), Action(Action),
      Gen(CreateLLVMCodeGen(Diags, InFile, std::move(VFS),
                            CI.getHeaderSearchOpts(), CI.getPreprocessorOpts(),
                            CI.getCodeGenOpts(), C, CoverageInfo)),
      LinkModules(std::move(LinkModules)), CurLinkModule(CurLinkModule) {
  TimerIsEnabled = CodeGenOpts.TimePasses;
  toolchain::TimePassesIsEnabled = CodeGenOpts.TimePasses;
  toolchain::TimePassesPerRun = CodeGenOpts.TimePassesPerRun;
  if (CodeGenOpts.TimePasses)
    LLVMIRGeneration.init("irgen", "LLVM IR generation", CI.getTimerGroup());
}

toolchain::Module* BackendConsumer::getModule() const {
  return Gen->GetModule();
}

std::unique_ptr<toolchain::Module> BackendConsumer::takeModule() {
  return std::unique_ptr<toolchain::Module>(Gen->ReleaseModule());
}

CodeGenerator* BackendConsumer::getCodeGenerator() {
  return Gen.get();
}

void BackendConsumer::HandleCXXStaticMemberVarInstantiation(VarDecl *VD) {
  Gen->HandleCXXStaticMemberVarInstantiation(VD);
}

void BackendConsumer::Initialize(ASTContext &Ctx) {
  assert(!Context && "initialized multiple times");

  Context = &Ctx;

  if (TimerIsEnabled)
    LLVMIRGeneration.startTimer();

  Gen->Initialize(Ctx);

  if (TimerIsEnabled)
    LLVMIRGeneration.stopTimer();
}

bool BackendConsumer::HandleTopLevelDecl(DeclGroupRef D) {
  PrettyStackTraceDecl CrashInfo(*D.begin(), SourceLocation(),
                                 Context->getSourceManager(),
                                 "LLVM IR generation of declaration");

  // Recurse.
  if (TimerIsEnabled && !LLVMIRGenerationRefCount++)
    CI.getFrontendTimer().yieldTo(LLVMIRGeneration);

  Gen->HandleTopLevelDecl(D);

  if (TimerIsEnabled && !--LLVMIRGenerationRefCount)
    LLVMIRGeneration.yieldTo(CI.getFrontendTimer());

  return true;
}

void BackendConsumer::HandleInlineFunctionDefinition(FunctionDecl *D) {
  PrettyStackTraceDecl CrashInfo(D, SourceLocation(),
                                 Context->getSourceManager(),
                                 "LLVM IR generation of inline function");
  if (TimerIsEnabled)
    CI.getFrontendTimer().yieldTo(LLVMIRGeneration);

  Gen->HandleInlineFunctionDefinition(D);

  if (TimerIsEnabled)
    LLVMIRGeneration.yieldTo(CI.getFrontendTimer());
}

void BackendConsumer::HandleInterestingDecl(DeclGroupRef D) {
  // Ignore interesting decls from the AST reader after IRGen is finished.
  if (!IRGenFinished)
    HandleTopLevelDecl(D);
}

// Links each entry in LinkModules into our module. Returns true on error.
bool BackendConsumer::LinkInModules(toolchain::Module *M) {
  for (auto &LM : LinkModules) {
    assert(LM.Module && "LinkModule does not actually have a module");

    if (LM.PropagateAttrs)
      for (Function &F : *LM.Module) {
        // Skip intrinsics. Keep consistent with how intrinsics are created
        // in LLVM IR.
        if (F.isIntrinsic())
          continue;
        CodeGen::mergeDefaultFunctionDefinitionAttributes(
          F, CodeGenOpts, LangOpts, TargetOpts, LM.Internalize);
      }

    CurLinkModule = LM.Module.get();
    bool Err;

    if (LM.Internalize) {
      Err = Linker::linkModules(
          *M, std::move(LM.Module), LM.LinkFlags,
          [](toolchain::Module &M, const toolchain::StringSet<> &GVS) {
            internalizeModule(M, [&GVS](const toolchain::GlobalValue &GV) {
              return !GV.hasName() || (GVS.count(GV.getName()) == 0);
            });
          });
    } else
      Err = Linker::linkModules(*M, std::move(LM.Module), LM.LinkFlags);

    if (Err)
      return true;
  }

  LinkModules.clear();
  return false; // success
}

void BackendConsumer::HandleTranslationUnit(ASTContext &C) {
  {
    toolchain::TimeTraceScope TimeScope("Frontend");
    PrettyStackTraceString CrashInfo("Per-file LLVM IR generation");
    if (TimerIsEnabled && !LLVMIRGenerationRefCount++)
      CI.getFrontendTimer().yieldTo(LLVMIRGeneration);

    Gen->HandleTranslationUnit(C);

    if (TimerIsEnabled && !--LLVMIRGenerationRefCount)
      LLVMIRGeneration.yieldTo(CI.getFrontendTimer());

    IRGenFinished = true;
  }

  // Silently ignore if we weren't initialized for some reason.
  if (!getModule())
    return;

  LLVMContext &Ctx = getModule()->getContext();
  std::unique_ptr<DiagnosticHandler> OldDiagnosticHandler =
    Ctx.getDiagnosticHandler();
  Ctx.setDiagnosticHandler(std::make_unique<ClangDiagnosticHandler>(
      CodeGenOpts, this));

  Ctx.setDefaultTargetCPU(TargetOpts.CPU);
  Ctx.setDefaultTargetFeatures(toolchain::join(TargetOpts.Features, ","));

  Expected<std::unique_ptr<toolchain::ToolOutputFile>> OptRecordFileOrErr =
    setupLLVMOptimizationRemarks(
      Ctx, CodeGenOpts.OptRecordFile, CodeGenOpts.OptRecordPasses,
      CodeGenOpts.OptRecordFormat, CodeGenOpts.DiagnosticsWithHotness,
      CodeGenOpts.DiagnosticsHotnessThreshold);

  if (Error E = OptRecordFileOrErr.takeError()) {
    reportOptRecordError(std::move(E), Diags, CodeGenOpts);
    return;
  }

  std::unique_ptr<toolchain::ToolOutputFile> OptRecordFile =
    std::move(*OptRecordFileOrErr);

  if (OptRecordFile && CodeGenOpts.getProfileUse() !=
                           toolchain::driver::ProfileInstrKind::ProfileNone)
    Ctx.setDiagnosticsHotnessRequested(true);

  if (CodeGenOpts.MisExpect) {
    Ctx.setMisExpectWarningRequested(true);
  }

  if (CodeGenOpts.DiagnosticsMisExpectTolerance) {
    Ctx.setDiagnosticsMisExpectTolerance(
      CodeGenOpts.DiagnosticsMisExpectTolerance);
  }

  // Link each LinkModule into our module.
  if (!CodeGenOpts.LinkBitcodePostopt && LinkInModules(getModule()))
    return;

  for (auto &F : getModule()->functions()) {
    if (const Decl *FD = Gen->GetDeclForMangledName(F.getName())) {
      auto Loc = FD->getASTContext().getFullLoc(FD->getLocation());
      // TODO: use a fast content hash when available.
      auto NameHash = toolchain::hash_value(F.getName());
      ManglingFullSourceLocs.push_back(std::make_pair(NameHash, Loc));
    }
  }

  if (CodeGenOpts.ClearASTBeforeBackend) {
    LLVM_DEBUG(toolchain::dbgs() << "Clearing AST...\n");
    // Access to the AST is no longer available after this.
    // Other things that the ASTContext manages are still available, e.g.
    // the SourceManager. It'd be nice if we could separate out all the
    // things in ASTContext used after this point and null out the
    // ASTContext, but too many various parts of the ASTContext are still
    // used in various parts.
    C.cleanup();
    C.getAllocator().Reset();
  }

  EmbedBitcode(getModule(), CodeGenOpts, toolchain::MemoryBufferRef());

  emitBackendOutput(CI, CI.getCodeGenOpts(),
                    C.getTargetInfo().getDataLayoutString(), getModule(),
                    Action, FS, std::move(AsmOutStream), this);

  Ctx.setDiagnosticHandler(std::move(OldDiagnosticHandler));

  if (OptRecordFile)
    OptRecordFile->keep();
}

void BackendConsumer::HandleTagDeclDefinition(TagDecl *D) {
  PrettyStackTraceDecl CrashInfo(D, SourceLocation(),
                                 Context->getSourceManager(),
                                 "LLVM IR generation of declaration");
  Gen->HandleTagDeclDefinition(D);
}

void BackendConsumer::HandleTagDeclRequiredDefinition(const TagDecl *D) {
  Gen->HandleTagDeclRequiredDefinition(D);
}

void BackendConsumer::CompleteTentativeDefinition(VarDecl *D) {
  Gen->CompleteTentativeDefinition(D);
}

void BackendConsumer::CompleteExternalDeclaration(DeclaratorDecl *D) {
  Gen->CompleteExternalDeclaration(D);
}

void BackendConsumer::AssignInheritanceModel(CXXRecordDecl *RD) {
  Gen->AssignInheritanceModel(RD);
}

void BackendConsumer::HandleVTable(CXXRecordDecl *RD) {
  Gen->HandleVTable(RD);
}

void BackendConsumer::anchor() { }

} // namespace language::Core

bool ClangDiagnosticHandler::handleDiagnostics(const DiagnosticInfo &DI) {
  BackendCon->DiagnosticHandlerImpl(DI);
  return true;
}

/// ConvertBackendLocation - Convert a location in a temporary toolchain::SourceMgr
/// buffer to be a valid FullSourceLoc.
static FullSourceLoc ConvertBackendLocation(const toolchain::SMDiagnostic &D,
                                            SourceManager &CSM) {
  // Get both the clang and toolchain source managers.  The location is relative to
  // a memory buffer that the LLVM Source Manager is handling, we need to add
  // a copy to the Clang source manager.
  const toolchain::SourceMgr &LSM = *D.getSourceMgr();

  // We need to copy the underlying LLVM memory buffer because toolchain::SourceMgr
  // already owns its one and language::Core::SourceManager wants to own its one.
  const MemoryBuffer *LBuf =
  LSM.getMemoryBuffer(LSM.FindBufferContainingLoc(D.getLoc()));

  // Create the copy and transfer ownership to language::Core::SourceManager.
  // TODO: Avoid copying files into memory.
  std::unique_ptr<toolchain::MemoryBuffer> CBuf =
      toolchain::MemoryBuffer::getMemBufferCopy(LBuf->getBuffer(),
                                           LBuf->getBufferIdentifier());
  // FIXME: Keep a file ID map instead of creating new IDs for each location.
  FileID FID = CSM.createFileID(std::move(CBuf));

  // Translate the offset into the file.
  unsigned Offset = D.getLoc().getPointer() - LBuf->getBufferStart();
  SourceLocation NewLoc =
  CSM.getLocForStartOfFile(FID).getLocWithOffset(Offset);
  return FullSourceLoc(NewLoc, CSM);
}

#define ComputeDiagID(Severity, GroupName, DiagID)                             \
  do {                                                                         \
    switch (Severity) {                                                        \
    case toolchain::DS_Error:                                                       \
      DiagID = diag::err_fe_##GroupName;                                       \
      break;                                                                   \
    case toolchain::DS_Warning:                                                     \
      DiagID = diag::warn_fe_##GroupName;                                      \
      break;                                                                   \
    case toolchain::DS_Remark:                                                      \
      toolchain_unreachable("'remark' severity not expected");                      \
      break;                                                                   \
    case toolchain::DS_Note:                                                        \
      DiagID = diag::note_fe_##GroupName;                                      \
      break;                                                                   \
    }                                                                          \
  } while (false)

#define ComputeDiagRemarkID(Severity, GroupName, DiagID)                       \
  do {                                                                         \
    switch (Severity) {                                                        \
    case toolchain::DS_Error:                                                       \
      DiagID = diag::err_fe_##GroupName;                                       \
      break;                                                                   \
    case toolchain::DS_Warning:                                                     \
      DiagID = diag::warn_fe_##GroupName;                                      \
      break;                                                                   \
    case toolchain::DS_Remark:                                                      \
      DiagID = diag::remark_fe_##GroupName;                                    \
      break;                                                                   \
    case toolchain::DS_Note:                                                        \
      DiagID = diag::note_fe_##GroupName;                                      \
      break;                                                                   \
    }                                                                          \
  } while (false)

void BackendConsumer::SrcMgrDiagHandler(const toolchain::DiagnosticInfoSrcMgr &DI) {
  const toolchain::SMDiagnostic &D = DI.getSMDiag();

  unsigned DiagID;
  if (DI.isInlineAsmDiag())
    ComputeDiagID(DI.getSeverity(), inline_asm, DiagID);
  else
    ComputeDiagID(DI.getSeverity(), source_mgr, DiagID);

  // This is for the empty BackendConsumer that uses the clang diagnostic
  // handler for IR input files.
  if (!Context) {
    D.print(nullptr, toolchain::errs());
    Diags.Report(DiagID).AddString("cannot compile inline asm");
    return;
  }

  // There are a couple of different kinds of errors we could get here.
  // First, we re-format the SMDiagnostic in terms of a clang diagnostic.

  // Strip "error: " off the start of the message string.
  StringRef Message = D.getMessage();
  (void)Message.consume_front("error: ");

  // If the SMDiagnostic has an inline asm source location, translate it.
  FullSourceLoc Loc;
  if (D.getLoc() != SMLoc())
    Loc = ConvertBackendLocation(D, Context->getSourceManager());

  // If this problem has clang-level source location information, report the
  // issue in the source with a note showing the instantiated
  // code.
  if (DI.isInlineAsmDiag()) {
    SourceLocation LocCookie =
        SourceLocation::getFromRawEncoding(DI.getLocCookie());
    if (LocCookie.isValid()) {
      Diags.Report(LocCookie, DiagID).AddString(Message);

      if (D.getLoc().isValid()) {
        DiagnosticBuilder B = Diags.Report(Loc, diag::note_fe_inline_asm_here);
        // Convert the SMDiagnostic ranges into SourceRange and attach them
        // to the diagnostic.
        for (const std::pair<unsigned, unsigned> &Range : D.getRanges()) {
          unsigned Column = D.getColumnNo();
          B << SourceRange(Loc.getLocWithOffset(Range.first - Column),
                           Loc.getLocWithOffset(Range.second - Column));
        }
      }
      return;
    }
  }

  // Otherwise, report the backend issue as occurring in the generated .s file.
  // If Loc is invalid, we still need to report the issue, it just gets no
  // location info.
  Diags.Report(Loc, DiagID).AddString(Message);
}

bool
BackendConsumer::InlineAsmDiagHandler(const toolchain::DiagnosticInfoInlineAsm &D) {
  unsigned DiagID;
  ComputeDiagID(D.getSeverity(), inline_asm, DiagID);
  std::string Message = D.getMsgStr().str();

  // If this problem has clang-level source location information, report the
  // issue as being a problem in the source with a note showing the instantiated
  // code.
  SourceLocation LocCookie =
      SourceLocation::getFromRawEncoding(D.getLocCookie());
  if (LocCookie.isValid())
    Diags.Report(LocCookie, DiagID).AddString(Message);
  else {
    // Otherwise, report the backend diagnostic as occurring in the generated
    // .s file.
    // If Loc is invalid, we still need to report the diagnostic, it just gets
    // no location info.
    FullSourceLoc Loc;
    Diags.Report(Loc, DiagID).AddString(Message);
  }
  // We handled all the possible severities.
  return true;
}

bool
BackendConsumer::StackSizeDiagHandler(const toolchain::DiagnosticInfoStackSize &D) {
  if (D.getSeverity() != toolchain::DS_Warning)
    // For now, the only support we have for StackSize diagnostic is warning.
    // We do not know how to format other severities.
    return false;

  auto Loc = getFunctionSourceLocation(D.getFunction());
  if (!Loc)
    return false;

  Diags.Report(*Loc, diag::warn_fe_frame_larger_than)
      << D.getStackSize() << D.getStackLimit()
      << toolchain::demangle(D.getFunction().getName());
  return true;
}

bool BackendConsumer::ResourceLimitDiagHandler(
    const toolchain::DiagnosticInfoResourceLimit &D) {
  auto Loc = getFunctionSourceLocation(D.getFunction());
  if (!Loc)
    return false;
  unsigned DiagID = diag::err_fe_backend_resource_limit;
  ComputeDiagID(D.getSeverity(), backend_resource_limit, DiagID);

  Diags.Report(*Loc, DiagID)
      << D.getResourceName() << D.getResourceSize() << D.getResourceLimit()
      << toolchain::demangle(D.getFunction().getName());
  return true;
}

const FullSourceLoc BackendConsumer::getBestLocationFromDebugLoc(
    const toolchain::DiagnosticInfoWithLocationBase &D, bool &BadDebugInfo,
    StringRef &Filename, unsigned &Line, unsigned &Column) const {
  SourceManager &SourceMgr = Context->getSourceManager();
  FileManager &FileMgr = SourceMgr.getFileManager();
  SourceLocation DILoc;

  if (D.isLocationAvailable()) {
    D.getLocation(Filename, Line, Column);
    if (Line > 0) {
      auto FE = FileMgr.getOptionalFileRef(Filename);
      if (!FE)
        FE = FileMgr.getOptionalFileRef(D.getAbsolutePath());
      if (FE) {
        // If -gcolumn-info was not used, Column will be 0. This upsets the
        // source manager, so pass 1 if Column is not set.
        DILoc = SourceMgr.translateFileLineCol(*FE, Line, Column ? Column : 1);
      }
    }
    BadDebugInfo = DILoc.isInvalid();
  }

  // If a location isn't available, try to approximate it using the associated
  // function definition. We use the definition's right brace to differentiate
  // from diagnostics that genuinely relate to the function itself.
  FullSourceLoc Loc(DILoc, SourceMgr);
  if (Loc.isInvalid()) {
    if (auto MaybeLoc = getFunctionSourceLocation(D.getFunction()))
      Loc = *MaybeLoc;
  }

  if (DILoc.isInvalid() && D.isLocationAvailable())
    // If we were not able to translate the file:line:col information
    // back to a SourceLocation, at least emit a note stating that
    // we could not translate this location. This can happen in the
    // case of #line directives.
    Diags.Report(Loc, diag::note_fe_backend_invalid_loc)
        << Filename << Line << Column;

  return Loc;
}

std::optional<FullSourceLoc>
BackendConsumer::getFunctionSourceLocation(const Function &F) const {
  auto Hash = toolchain::hash_value(F.getName());
  for (const auto &Pair : ManglingFullSourceLocs) {
    if (Pair.first == Hash)
      return Pair.second;
  }
  return std::nullopt;
}

void BackendConsumer::UnsupportedDiagHandler(
    const toolchain::DiagnosticInfoUnsupported &D) {
  // We only support warnings or errors.
  assert(D.getSeverity() == toolchain::DS_Error ||
         D.getSeverity() == toolchain::DS_Warning);

  StringRef Filename;
  unsigned Line, Column;
  bool BadDebugInfo = false;
  FullSourceLoc Loc;
  std::string Msg;
  raw_string_ostream MsgStream(Msg);

  // Context will be nullptr for IR input files, we will construct the diag
  // message from toolchain::DiagnosticInfoUnsupported.
  if (Context != nullptr) {
    Loc = getBestLocationFromDebugLoc(D, BadDebugInfo, Filename, Line, Column);
    MsgStream << D.getMessage();
  } else {
    DiagnosticPrinterRawOStream DP(MsgStream);
    D.print(DP);
  }

  auto DiagType = D.getSeverity() == toolchain::DS_Error
                      ? diag::err_fe_backend_unsupported
                      : diag::warn_fe_backend_unsupported;
  Diags.Report(Loc, DiagType) << Msg;

  if (BadDebugInfo)
    // If we were not able to translate the file:line:col information
    // back to a SourceLocation, at least emit a note stating that
    // we could not translate this location. This can happen in the
    // case of #line directives.
    Diags.Report(Loc, diag::note_fe_backend_invalid_loc)
        << Filename << Line << Column;
}

void BackendConsumer::EmitOptimizationMessage(
    const toolchain::DiagnosticInfoOptimizationBase &D, unsigned DiagID) {
  // We only support warnings and remarks.
  assert(D.getSeverity() == toolchain::DS_Remark ||
         D.getSeverity() == toolchain::DS_Warning);

  StringRef Filename;
  unsigned Line, Column;
  bool BadDebugInfo = false;
  FullSourceLoc Loc;
  std::string Msg;
  raw_string_ostream MsgStream(Msg);

  // Context will be nullptr for IR input files, we will construct the remark
  // message from toolchain::DiagnosticInfoOptimizationBase.
  if (Context != nullptr) {
    Loc = getBestLocationFromDebugLoc(D, BadDebugInfo, Filename, Line, Column);
    MsgStream << D.getMsg();
  } else {
    DiagnosticPrinterRawOStream DP(MsgStream);
    D.print(DP);
  }

  if (D.getHotness())
    MsgStream << " (hotness: " << *D.getHotness() << ")";

  Diags.Report(Loc, DiagID) << AddFlagValue(D.getPassName()) << Msg;

  if (BadDebugInfo)
    // If we were not able to translate the file:line:col information
    // back to a SourceLocation, at least emit a note stating that
    // we could not translate this location. This can happen in the
    // case of #line directives.
    Diags.Report(Loc, diag::note_fe_backend_invalid_loc)
        << Filename << Line << Column;
}

void BackendConsumer::OptimizationRemarkHandler(
    const toolchain::DiagnosticInfoOptimizationBase &D) {
  // Without hotness information, don't show noisy remarks.
  if (D.isVerbose() && !D.getHotness())
    return;

  if (D.isPassed()) {
    // Optimization remarks are active only if the -Rpass flag has a regular
    // expression that matches the name of the pass name in \p D.
    if (CodeGenOpts.OptimizationRemark.patternMatches(D.getPassName()))
      EmitOptimizationMessage(D, diag::remark_fe_backend_optimization_remark);
  } else if (D.isMissed()) {
    // Missed optimization remarks are active only if the -Rpass-missed
    // flag has a regular expression that matches the name of the pass
    // name in \p D.
    if (CodeGenOpts.OptimizationRemarkMissed.patternMatches(D.getPassName()))
      EmitOptimizationMessage(
          D, diag::remark_fe_backend_optimization_remark_missed);
  } else {
    assert(D.isAnalysis() && "Unknown remark type");

    bool ShouldAlwaysPrint = false;
    if (auto *ORA = dyn_cast<toolchain::OptimizationRemarkAnalysis>(&D))
      ShouldAlwaysPrint = ORA->shouldAlwaysPrint();

    if (ShouldAlwaysPrint ||
        CodeGenOpts.OptimizationRemarkAnalysis.patternMatches(D.getPassName()))
      EmitOptimizationMessage(
          D, diag::remark_fe_backend_optimization_remark_analysis);
  }
}

void BackendConsumer::OptimizationRemarkHandler(
    const toolchain::OptimizationRemarkAnalysisFPCommute &D) {
  // Optimization analysis remarks are active if the pass name is set to
  // toolchain::DiagnosticInfo::AlwasyPrint or if the -Rpass-analysis flag has a
  // regular expression that matches the name of the pass name in \p D.

  if (D.shouldAlwaysPrint() ||
      CodeGenOpts.OptimizationRemarkAnalysis.patternMatches(D.getPassName()))
    EmitOptimizationMessage(
        D, diag::remark_fe_backend_optimization_remark_analysis_fpcommute);
}

void BackendConsumer::OptimizationRemarkHandler(
    const toolchain::OptimizationRemarkAnalysisAliasing &D) {
  // Optimization analysis remarks are active if the pass name is set to
  // toolchain::DiagnosticInfo::AlwasyPrint or if the -Rpass-analysis flag has a
  // regular expression that matches the name of the pass name in \p D.

  if (D.shouldAlwaysPrint() ||
      CodeGenOpts.OptimizationRemarkAnalysis.patternMatches(D.getPassName()))
    EmitOptimizationMessage(
        D, diag::remark_fe_backend_optimization_remark_analysis_aliasing);
}

void BackendConsumer::OptimizationFailureHandler(
    const toolchain::DiagnosticInfoOptimizationFailure &D) {
  EmitOptimizationMessage(D, diag::warn_fe_backend_optimization_failure);
}

void BackendConsumer::DontCallDiagHandler(const DiagnosticInfoDontCall &D) {
  SourceLocation LocCookie =
      SourceLocation::getFromRawEncoding(D.getLocCookie());

  // FIXME: we can't yet diagnose indirect calls. When/if we can, we
  // should instead assert that LocCookie.isValid().
  if (!LocCookie.isValid())
    return;

  Diags.Report(LocCookie, D.getSeverity() == DiagnosticSeverity::DS_Error
                              ? diag::err_fe_backend_error_attr
                              : diag::warn_fe_backend_warning_attr)
      << toolchain::demangle(D.getFunctionName()) << D.getNote();
}

void BackendConsumer::MisExpectDiagHandler(
    const toolchain::DiagnosticInfoMisExpect &D) {
  StringRef Filename;
  unsigned Line, Column;
  bool BadDebugInfo = false;
  FullSourceLoc Loc =
      getBestLocationFromDebugLoc(D, BadDebugInfo, Filename, Line, Column);

  Diags.Report(Loc, diag::warn_profile_data_misexpect) << D.getMsg().str();

  if (BadDebugInfo)
    // If we were not able to translate the file:line:col information
    // back to a SourceLocation, at least emit a note stating that
    // we could not translate this location. This can happen in the
    // case of #line directives.
    Diags.Report(Loc, diag::note_fe_backend_invalid_loc)
        << Filename << Line << Column;
}

/// This function is invoked when the backend needs
/// to report something to the user.
void BackendConsumer::DiagnosticHandlerImpl(const DiagnosticInfo &DI) {
  unsigned DiagID = diag::err_fe_inline_asm;
  toolchain::DiagnosticSeverity Severity = DI.getSeverity();
  // Get the diagnostic ID based.
  switch (DI.getKind()) {
  case toolchain::DK_InlineAsm:
    if (InlineAsmDiagHandler(cast<DiagnosticInfoInlineAsm>(DI)))
      return;
    ComputeDiagID(Severity, inline_asm, DiagID);
    break;
  case toolchain::DK_SrcMgr:
    SrcMgrDiagHandler(cast<DiagnosticInfoSrcMgr>(DI));
    return;
  case toolchain::DK_StackSize:
    if (StackSizeDiagHandler(cast<DiagnosticInfoStackSize>(DI)))
      return;
    ComputeDiagID(Severity, backend_frame_larger_than, DiagID);
    break;
  case toolchain::DK_ResourceLimit:
    if (ResourceLimitDiagHandler(cast<DiagnosticInfoResourceLimit>(DI)))
      return;
    ComputeDiagID(Severity, backend_resource_limit, DiagID);
    break;
  case DK_Linker:
    ComputeDiagID(Severity, linking_module, DiagID);
    break;
  case toolchain::DK_OptimizationRemark:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<OptimizationRemark>(DI));
    return;
  case toolchain::DK_OptimizationRemarkMissed:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<OptimizationRemarkMissed>(DI));
    return;
  case toolchain::DK_OptimizationRemarkAnalysis:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<OptimizationRemarkAnalysis>(DI));
    return;
  case toolchain::DK_OptimizationRemarkAnalysisFPCommute:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<OptimizationRemarkAnalysisFPCommute>(DI));
    return;
  case toolchain::DK_OptimizationRemarkAnalysisAliasing:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<OptimizationRemarkAnalysisAliasing>(DI));
    return;
  case toolchain::DK_MachineOptimizationRemark:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<MachineOptimizationRemark>(DI));
    return;
  case toolchain::DK_MachineOptimizationRemarkMissed:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<MachineOptimizationRemarkMissed>(DI));
    return;
  case toolchain::DK_MachineOptimizationRemarkAnalysis:
    // Optimization remarks are always handled completely by this
    // handler. There is no generic way of emitting them.
    OptimizationRemarkHandler(cast<MachineOptimizationRemarkAnalysis>(DI));
    return;
  case toolchain::DK_OptimizationFailure:
    // Optimization failures are always handled completely by this
    // handler.
    OptimizationFailureHandler(cast<DiagnosticInfoOptimizationFailure>(DI));
    return;
  case toolchain::DK_Unsupported:
    UnsupportedDiagHandler(cast<DiagnosticInfoUnsupported>(DI));
    return;
  case toolchain::DK_DontCall:
    DontCallDiagHandler(cast<DiagnosticInfoDontCall>(DI));
    return;
  case toolchain::DK_MisExpect:
    MisExpectDiagHandler(cast<DiagnosticInfoMisExpect>(DI));
    return;
  default:
    // Plugin IDs are not bound to any value as they are set dynamically.
    ComputeDiagRemarkID(Severity, backend_plugin, DiagID);
    break;
  }
  std::string MsgStorage;
  {
    raw_string_ostream Stream(MsgStorage);
    DiagnosticPrinterRawOStream DP(Stream);
    DI.print(DP);
  }

  if (DI.getKind() == DK_Linker) {
    assert(CurLinkModule && "CurLinkModule must be set for linker diagnostics");
    Diags.Report(DiagID) << CurLinkModule->getModuleIdentifier() << MsgStorage;
    return;
  }

  // Report the backend message using the usual diagnostic mechanism.
  FullSourceLoc Loc;
  Diags.Report(Loc, DiagID).AddString(MsgStorage);
}
#undef ComputeDiagID

CodeGenAction::CodeGenAction(unsigned _Act, LLVMContext *_VMContext)
    : Act(_Act), VMContext(_VMContext ? _VMContext : new LLVMContext),
      OwnsVMContext(!_VMContext) {}

CodeGenAction::~CodeGenAction() {
  TheModule.reset();
  if (OwnsVMContext)
    delete VMContext;
}

bool CodeGenAction::loadLinkModules(CompilerInstance &CI) {
  if (!LinkModules.empty())
    return false;

  for (const CodeGenOptions::BitcodeFileToLink &F :
       CI.getCodeGenOpts().LinkBitcodeFiles) {
    auto BCBuf = CI.getFileManager().getBufferForFile(F.Filename);
    if (!BCBuf) {
      CI.getDiagnostics().Report(diag::err_cannot_open_file)
          << F.Filename << BCBuf.getError().message();
      LinkModules.clear();
      return true;
    }

    Expected<std::unique_ptr<toolchain::Module>> ModuleOrErr =
        getOwningLazyBitcodeModule(std::move(*BCBuf), *VMContext);
    if (!ModuleOrErr) {
      handleAllErrors(ModuleOrErr.takeError(), [&](ErrorInfoBase &EIB) {
        CI.getDiagnostics().Report(diag::err_cannot_open_file)
            << F.Filename << EIB.message();
      });
      LinkModules.clear();
      return true;
    }
    LinkModules.push_back({std::move(ModuleOrErr.get()), F.PropagateAttrs,
                           F.Internalize, F.LinkFlags});
  }
  return false;
}

bool CodeGenAction::hasIRSupport() const { return true; }

void CodeGenAction::EndSourceFileAction() {
  ASTFrontendAction::EndSourceFileAction();

  // If the consumer creation failed, do nothing.
  if (!getCompilerInstance().hasASTConsumer())
    return;

  // Steal the module from the consumer.
  TheModule = BEConsumer->takeModule();
}

std::unique_ptr<toolchain::Module> CodeGenAction::takeModule() {
  return std::move(TheModule);
}

toolchain::LLVMContext *CodeGenAction::takeLLVMContext() {
  OwnsVMContext = false;
  return VMContext;
}

CodeGenerator *CodeGenAction::getCodeGenerator() const {
  return BEConsumer->getCodeGenerator();
}

bool CodeGenAction::BeginSourceFileAction(CompilerInstance &CI) {
  if (CI.getFrontendOpts().GenReducedBMI)
    CI.getLangOpts().setCompilingModule(LangOptions::CMK_ModuleInterface);
  return ASTFrontendAction::BeginSourceFileAction(CI);
}

static std::unique_ptr<raw_pwrite_stream>
GetOutputStream(CompilerInstance &CI, StringRef InFile, BackendAction Action) {
  switch (Action) {
  case Backend_EmitAssembly:
    return CI.createDefaultOutputFile(false, InFile, "s");
  case Backend_EmitLL:
    return CI.createDefaultOutputFile(false, InFile, "ll");
  case Backend_EmitBC:
    return CI.createDefaultOutputFile(true, InFile, "bc");
  case Backend_EmitNothing:
    return nullptr;
  case Backend_EmitMCNull:
    return CI.createNullOutputFile();
  case Backend_EmitObj:
    return CI.createDefaultOutputFile(true, InFile, "o");
  }

  toolchain_unreachable("Invalid action!");
}

std::unique_ptr<ASTConsumer>
CodeGenAction::CreateASTConsumer(CompilerInstance &CI, StringRef InFile) {
  BackendAction BA = static_cast<BackendAction>(Act);
  std::unique_ptr<raw_pwrite_stream> OS = CI.takeOutputStream();
  if (!OS)
    OS = GetOutputStream(CI, InFile, BA);

  if (BA != Backend_EmitNothing && !OS)
    return nullptr;

  // Load bitcode modules to link with, if we need to.
  if (loadLinkModules(CI))
    return nullptr;

  CoverageSourceInfo *CoverageInfo = nullptr;
  // Add the preprocessor callback only when the coverage mapping is generated.
  if (CI.getCodeGenOpts().CoverageMapping)
    CoverageInfo = CodeGen::CoverageMappingModuleGen::setUpCoverageCallbacks(
        CI.getPreprocessor());

  std::unique_ptr<BackendConsumer> Result(new BackendConsumer(
      CI, BA, CI.getVirtualFileSystemPtr(), *VMContext, std::move(LinkModules),
      InFile, std::move(OS), CoverageInfo));
  BEConsumer = Result.get();

  // Enable generating macro debug info only when debug info is not disabled and
  // also macro debug info is enabled.
  if (CI.getCodeGenOpts().getDebugInfo() != codegenoptions::NoDebugInfo &&
      CI.getCodeGenOpts().MacroDebugInfo) {
    std::unique_ptr<PPCallbacks> Callbacks =
        std::make_unique<MacroPPCallbacks>(BEConsumer->getCodeGenerator(),
                                            CI.getPreprocessor());
    CI.getPreprocessor().addPPCallbacks(std::move(Callbacks));
  }

  if (CI.getFrontendOpts().GenReducedBMI &&
      !CI.getFrontendOpts().ModuleOutputPath.empty()) {
    std::vector<std::unique_ptr<ASTConsumer>> Consumers(2);
    Consumers[0] = std::make_unique<ReducedBMIGenerator>(
        CI.getPreprocessor(), CI.getModuleCache(),
        CI.getFrontendOpts().ModuleOutputPath, CI.getCodeGenOpts());
    Consumers[1] = std::move(Result);
    return std::make_unique<MultiplexConsumer>(std::move(Consumers));
  }

  return std::move(Result);
}

std::unique_ptr<toolchain::Module>
CodeGenAction::loadModule(MemoryBufferRef MBRef) {
  CompilerInstance &CI = getCompilerInstance();
  SourceManager &SM = CI.getSourceManager();

  auto DiagErrors = [&](Error E) -> std::unique_ptr<toolchain::Module> {
    unsigned DiagID =
        CI.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error, "%0");
    handleAllErrors(std::move(E), [&](ErrorInfoBase &EIB) {
      CI.getDiagnostics().Report(DiagID) << EIB.message();
    });
    return {};
  };

  // For ThinLTO backend invocations, ensure that the context
  // merges types based on ODR identifiers. We also need to read
  // the correct module out of a multi-module bitcode file.
  if (!CI.getCodeGenOpts().ThinLTOIndexFile.empty()) {
    VMContext->enableDebugTypeODRUniquing();

    Expected<std::vector<BitcodeModule>> BMsOrErr = getBitcodeModuleList(MBRef);
    if (!BMsOrErr)
      return DiagErrors(BMsOrErr.takeError());
    BitcodeModule *Bm = toolchain::lto::findThinLTOModule(*BMsOrErr);
    // We have nothing to do if the file contains no ThinLTO module. This is
    // possible if ThinLTO compilation was not able to split module. Content of
    // the file was already processed by indexing and will be passed to the
    // linker using merged object file.
    if (!Bm) {
      auto M = std::make_unique<toolchain::Module>("empty", *VMContext);
      M->setTargetTriple(Triple(CI.getTargetOpts().Triple));
      return M;
    }
    Expected<std::unique_ptr<toolchain::Module>> MOrErr =
        Bm->parseModule(*VMContext);
    if (!MOrErr)
      return DiagErrors(MOrErr.takeError());
    return std::move(*MOrErr);
  }

  // Load bitcode modules to link with, if we need to.
  if (loadLinkModules(CI))
    return nullptr;

  // Handle textual IR and bitcode file with one single module.
  toolchain::SMDiagnostic Err;
  if (std::unique_ptr<toolchain::Module> M = parseIR(MBRef, Err, *VMContext)) {
    // For LLVM IR files, always verify the input and report the error in a way
    // that does not ask people to report an issue for it.
    std::string VerifierErr;
    raw_string_ostream VerifierErrStream(VerifierErr);
    if (toolchain::verifyModule(*M, &VerifierErrStream)) {
      CI.getDiagnostics().Report(diag::err_invalid_toolchain_ir) << VerifierErr;
      return {};
    }
    return M;
  }

  // If MBRef is a bitcode with multiple modules (e.g., -fsplit-lto-unit
  // output), place the extra modules (actually only one, a regular LTO module)
  // into LinkModules as if we are using -mlink-bitcode-file.
  Expected<std::vector<BitcodeModule>> BMsOrErr = getBitcodeModuleList(MBRef);
  if (BMsOrErr && BMsOrErr->size()) {
    std::unique_ptr<toolchain::Module> FirstM;
    for (auto &BM : *BMsOrErr) {
      Expected<std::unique_ptr<toolchain::Module>> MOrErr =
          BM.parseModule(*VMContext);
      if (!MOrErr)
        return DiagErrors(MOrErr.takeError());
      if (FirstM)
        LinkModules.push_back({std::move(*MOrErr), /*PropagateAttrs=*/false,
                               /*Internalize=*/false, /*LinkFlags=*/{}});
      else
        FirstM = std::move(*MOrErr);
    }
    if (FirstM)
      return FirstM;
  }
  // If BMsOrErr fails, consume the error and use the error message from
  // parseIR.
  consumeError(BMsOrErr.takeError());

  // Translate from the diagnostic info to the SourceManager location if
  // available.
  // TODO: Unify this with ConvertBackendLocation()
  SourceLocation Loc;
  if (Err.getLineNo() > 0) {
    assert(Err.getColumnNo() >= 0);
    Loc = SM.translateFileLineCol(SM.getFileEntryForID(SM.getMainFileID()),
                                  Err.getLineNo(), Err.getColumnNo() + 1);
  }

  // Strip off a leading diagnostic code if there is one.
  StringRef Msg = Err.getMessage();
  Msg.consume_front("error: ");

  unsigned DiagID =
      CI.getDiagnostics().getCustomDiagID(DiagnosticsEngine::Error, "%0");

  CI.getDiagnostics().Report(Loc, DiagID) << Msg;
  return {};
}

void CodeGenAction::ExecuteAction() {
  if (getCurrentFileKind().getLanguage() != Language::LLVM_IR) {
    this->ASTFrontendAction::ExecuteAction();
    return;
  }

  // If this is an IR file, we have to treat it specially.
  BackendAction BA = static_cast<BackendAction>(Act);
  CompilerInstance &CI = getCompilerInstance();
  auto &CodeGenOpts = CI.getCodeGenOpts();
  auto &Diagnostics = CI.getDiagnostics();
  std::unique_ptr<raw_pwrite_stream> OS =
      GetOutputStream(CI, getCurrentFileOrBufferName(), BA);
  if (BA != Backend_EmitNothing && !OS)
    return;

  SourceManager &SM = CI.getSourceManager();
  FileID FID = SM.getMainFileID();
  std::optional<MemoryBufferRef> MainFile = SM.getBufferOrNone(FID);
  if (!MainFile)
    return;

  TheModule = loadModule(*MainFile);
  if (!TheModule)
    return;

  const TargetOptions &TargetOpts = CI.getTargetOpts();
  if (TheModule->getTargetTriple().str() != TargetOpts.Triple) {
    Diagnostics.Report(SourceLocation(), diag::warn_fe_override_module)
        << TargetOpts.Triple;
    TheModule->setTargetTriple(Triple(TargetOpts.Triple));
  }

  EmbedObject(TheModule.get(), CodeGenOpts, Diagnostics);
  EmbedBitcode(TheModule.get(), CodeGenOpts, *MainFile);

  LLVMContext &Ctx = TheModule->getContext();

  // Restore any diagnostic handler previously set before returning from this
  // function.
  struct RAII {
    LLVMContext &Ctx;
    std::unique_ptr<DiagnosticHandler> PrevHandler = Ctx.getDiagnosticHandler();
    ~RAII() { Ctx.setDiagnosticHandler(std::move(PrevHandler)); }
  } _{Ctx};

  // Set clang diagnostic handler. To do this we need to create a fake
  // BackendConsumer.
  BackendConsumer Result(CI, BA, CI.getVirtualFileSystemPtr(), *VMContext,
                         std::move(LinkModules), "", nullptr, nullptr,
                         TheModule.get());

  // Link in each pending link module.
  if (!CodeGenOpts.LinkBitcodePostopt && Result.LinkInModules(&*TheModule))
    return;

  // PR44896: Force DiscardValueNames as false. DiscardValueNames cannot be
  // true here because the valued names are needed for reading textual IR.
  Ctx.setDiscardValueNames(false);
  Ctx.setDiagnosticHandler(
      std::make_unique<ClangDiagnosticHandler>(CodeGenOpts, &Result));

  Ctx.setDefaultTargetCPU(TargetOpts.CPU);
  Ctx.setDefaultTargetFeatures(toolchain::join(TargetOpts.Features, ","));

  Expected<std::unique_ptr<toolchain::ToolOutputFile>> OptRecordFileOrErr =
      setupLLVMOptimizationRemarks(
          Ctx, CodeGenOpts.OptRecordFile, CodeGenOpts.OptRecordPasses,
          CodeGenOpts.OptRecordFormat, CodeGenOpts.DiagnosticsWithHotness,
          CodeGenOpts.DiagnosticsHotnessThreshold);

  if (Error E = OptRecordFileOrErr.takeError()) {
    reportOptRecordError(std::move(E), Diagnostics, CodeGenOpts);
    return;
  }
  std::unique_ptr<toolchain::ToolOutputFile> OptRecordFile =
      std::move(*OptRecordFileOrErr);

  emitBackendOutput(CI, CI.getCodeGenOpts(),
                    CI.getTarget().getDataLayoutString(), TheModule.get(), BA,
                    CI.getFileManager().getVirtualFileSystemPtr(),
                    std::move(OS));
  if (OptRecordFile)
    OptRecordFile->keep();
}

//

void EmitAssemblyAction::anchor() { }
EmitAssemblyAction::EmitAssemblyAction(toolchain::LLVMContext *_VMContext)
  : CodeGenAction(Backend_EmitAssembly, _VMContext) {}

void EmitBCAction::anchor() { }
EmitBCAction::EmitBCAction(toolchain::LLVMContext *_VMContext)
  : CodeGenAction(Backend_EmitBC, _VMContext) {}

void EmitLLVMAction::anchor() { }
EmitLLVMAction::EmitLLVMAction(toolchain::LLVMContext *_VMContext)
  : CodeGenAction(Backend_EmitLL, _VMContext) {}

void EmitLLVMOnlyAction::anchor() { }
EmitLLVMOnlyAction::EmitLLVMOnlyAction(toolchain::LLVMContext *_VMContext)
  : CodeGenAction(Backend_EmitNothing, _VMContext) {}

void EmitCodeGenOnlyAction::anchor() { }
EmitCodeGenOnlyAction::EmitCodeGenOnlyAction(toolchain::LLVMContext *_VMContext)
  : CodeGenAction(Backend_EmitMCNull, _VMContext) {}

void EmitObjAction::anchor() { }
EmitObjAction::EmitObjAction(toolchain::LLVMContext *_VMContext)
  : CodeGenAction(Backend_EmitObj, _VMContext) {}
