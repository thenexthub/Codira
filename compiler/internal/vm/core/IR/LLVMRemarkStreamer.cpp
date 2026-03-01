//===- toolchain/IR/LLVMRemarkStreamer.cpp - Remark Streamer -*- C++ ---------*-===//
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
//
// This file contains the implementation of the conversion between IR
// Diagnostics and serializable remarks::Remark objects.
//
//===----------------------------------------------------------------------===//

#include "vm/core/IR/LLVMRemarkStreamer.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/Remarks/RemarkStreamer.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/ToolOutputFile.h"
#include <optional>

using namespace vm::core;

/// DiagnosticKind -> remarks::Type
static remarks::Type toRemarkType(enum DiagnosticKind Kind) {
  switch (Kind) {
  default:
    return remarks::Type::Unknown;
  case DK_OptimizationRemark:
  case DK_MachineOptimizationRemark:
    return remarks::Type::Passed;
  case DK_OptimizationRemarkMissed:
  case DK_MachineOptimizationRemarkMissed:
    return remarks::Type::Missed;
  case DK_OptimizationRemarkAnalysis:
  case DK_MachineOptimizationRemarkAnalysis:
    return remarks::Type::Analysis;
  case DK_OptimizationRemarkAnalysisFPCommute:
    return remarks::Type::AnalysisFPCommute;
  case DK_OptimizationRemarkAnalysisAliasing:
    return remarks::Type::AnalysisAliasing;
  case DK_OptimizationFailure:
    return remarks::Type::Failure;
  }
}

/// DiagnosticLocation -> remarks::RemarkLocation.
static std::optional<remarks::RemarkLocation>
toRemarkLocation(const DiagnosticLocation &DL) {
  if (!DL.isValid())
    return std::nullopt;
  StringRef File = DL.getRelativePath();
  unsigned Line = DL.getLine();
  unsigned Col = DL.getColumn();
  return remarks::RemarkLocation{File, Line, Col};
}

/// LLVM Diagnostic -> Remark
remarks::Remark
LLVMRemarkStreamer::toRemark(const DiagnosticInfoOptimizationBase &Diag) const {
  remarks::Remark R; // The result.
  R.RemarkType = toRemarkType(static_cast<DiagnosticKind>(Diag.getKind()));
  R.PassName = Diag.getPassName();
  R.RemarkName = Diag.getRemarkName();
  R.FunctionName =
      GlobalValue::dropLLVMManglingEscape(Diag.getFunction().getName());
  R.Loc = toRemarkLocation(Diag.getLocation());
  R.Hotness = Diag.getHotness();

  for (const DiagnosticInfoOptimizationBase::Argument &Arg : Diag.getArgs()) {
    R.Args.emplace_back();
    R.Args.back().Key = Arg.Key;
    R.Args.back().Val = Arg.Val;
    R.Args.back().Loc = toRemarkLocation(Arg.Loc);
  }

  return R;
}

void LLVMRemarkStreamer::emit(const DiagnosticInfoOptimizationBase &Diag) {
  if (!RS.matchesFilter(Diag.getPassName()))
      return;

  // First, convert the diagnostic to a remark.
  remarks::Remark R = toRemark(Diag);
  // Then, emit the remark through the serializer.
  RS.getSerializer().emit(R);
}

char LLVMRemarkSetupFileError::ID = 0;
char LLVMRemarkSetupPatternError::ID = 0;
char LLVMRemarkSetupFormatError::ID = 0;

Expected<LLVMRemarkFileHandle> toolchain::setupLLVMOptimizationRemarks(
    LLVMContext &Context, StringRef RemarksFilename, StringRef RemarksPasses,
    StringRef RemarksFormat, bool RemarksWithHotness,
    std::optional<uint64_t> RemarksHotnessThreshold) {
  if (RemarksWithHotness || RemarksHotnessThreshold.value_or(1))
      Context.setDiagnosticsHotnessRequested(true);

  Context.setDiagnosticsHotnessThreshold(RemarksHotnessThreshold);

  if (RemarksFilename.empty())
    return LLVMRemarkFileHandle();

  Expected<remarks::Format> Format = remarks::parseFormat(RemarksFormat);
  if (Error E = Format.takeError())
    return make_error<LLVMRemarkSetupFormatError>(std::move(E));

  std::error_code EC;
  auto Flags = *Format == remarks::Format::YAML ? sys::fs::OF_TextWithCRLF
                                                : sys::fs::OF_None;
  auto RemarksFile =
      std::make_unique<ToolOutputFile>(RemarksFilename, EC, Flags);
  // We don't use toolchain::FileError here because some diagnostics want the file
  // name separately.
  if (EC)
    return make_error<LLVMRemarkSetupFileError>(errorCodeToError(EC));

  Expected<std::unique_ptr<remarks::RemarkSerializer>> RemarkSerializer =
      remarks::createRemarkSerializer(*Format, RemarksFile->os());
  if (Error E = RemarkSerializer.takeError())
    return make_error<LLVMRemarkSetupFormatError>(std::move(E));

  auto RS = std::make_unique<remarks::RemarkStreamer>(
      std::move(*RemarkSerializer), RemarksFilename);

  if (!RemarksPasses.empty())
    if (Error E = RS->setFilter(RemarksPasses)) {
      RS->releaseSerializer();
      return make_error<LLVMRemarkSetupPatternError>(std::move(E));
    }

  // Install the main remark streamer. Only install this after setting the
  // filter, because this might fail.
  Context.setMainRemarkStreamer(std::move(RS));

  // Create LLVM's optimization remarks streamer.
  Context.setLLVMRemarkStreamer(
      std::make_unique<LLVMRemarkStreamer>(*Context.getMainRemarkStreamer()));

  return LLVMRemarkFileHandle{std::move(RemarksFile), Context};
}

void LLVMRemarkFileHandle::Finalizer::finalize() {
  if (!Context)
    return;
  finalizeLLVMOptimizationRemarks(*Context);
  Context = nullptr;
}

Error toolchain::setupLLVMOptimizationRemarks(
    LLVMContext &Context, raw_ostream &OS, StringRef RemarksPasses,
    StringRef RemarksFormat, bool RemarksWithHotness,
    std::optional<uint64_t> RemarksHotnessThreshold) {
  if (RemarksWithHotness || RemarksHotnessThreshold.value_or(1))
    Context.setDiagnosticsHotnessRequested(true);

  Context.setDiagnosticsHotnessThreshold(RemarksHotnessThreshold);

  Expected<remarks::Format> Format = remarks::parseFormat(RemarksFormat);
  if (Error E = Format.takeError())
    return make_error<LLVMRemarkSetupFormatError>(std::move(E));

  Expected<std::unique_ptr<remarks::RemarkSerializer>> RemarkSerializer =
      remarks::createRemarkSerializer(*Format, OS);
  if (Error E = RemarkSerializer.takeError())
    return make_error<LLVMRemarkSetupFormatError>(std::move(E));

  auto RS =
      std::make_unique<remarks::RemarkStreamer>(std::move(*RemarkSerializer));

  if (!RemarksPasses.empty())
    if (Error E = RS->setFilter(RemarksPasses)) {
      RS->releaseSerializer();
      return make_error<LLVMRemarkSetupPatternError>(std::move(E));
    }

  // Install the main remark streamer. Only install this after setting the
  // filter, because this might fail.
  Context.setMainRemarkStreamer(std::move(RS));

  // Create LLVM's optimization remarks streamer.
  Context.setLLVMRemarkStreamer(
      std::make_unique<LLVMRemarkStreamer>(*Context.getMainRemarkStreamer()));

  return Error::success();
}

void toolchain::finalizeLLVMOptimizationRemarks(LLVMContext &Context) {
  Context.setLLVMRemarkStreamer(nullptr);
  if (auto *RS = Context.getMainRemarkStreamer()) {
    RS->releaseSerializer();
    Context.setMainRemarkStreamer(nullptr);
  }
}
