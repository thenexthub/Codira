//===---------------------------- Context.cpp -------------------*- C++ -*-===//
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
/// This file defines a class for holding ownership of various simulated
/// hardware units.  A Context also provides a utility routine for constructing
/// a default out-of-order pipeline with fetch, dispatch, execute, and retire
/// stages.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/Context.h"
#include "vm/core/MCA/HardwareUnits/RegisterFile.h"
#include "vm/core/MCA/HardwareUnits/RetireControlUnit.h"
#include "vm/core/MCA/HardwareUnits/Scheduler.h"
#include "vm/core/MCA/Stages/DispatchStage.h"
#include "vm/core/MCA/Stages/EntryStage.h"
#include "vm/core/MCA/Stages/ExecuteStage.h"
#include "vm/core/MCA/Stages/InOrderIssueStage.h"
#include "vm/core/MCA/Stages/MicroOpQueueStage.h"
#include "vm/core/MCA/Stages/RetireStage.h"

namespace vm::core {
namespace mca {

std::unique_ptr<Pipeline>
Context::createDefaultPipeline(const PipelineOptions &Opts, SourceMgr &SrcMgr,
                               CustomBehaviour &CB) {
  const MCSchedModel &SM = STI.getSchedModel();

  if (!SM.isOutOfOrder())
    return createInOrderPipeline(Opts, SrcMgr, CB);

  // Create the hardware units defining the backend.
  auto RCU = std::make_unique<RetireControlUnit>(SM);
  auto PRF = std::make_unique<RegisterFile>(SM, MRI, Opts.RegisterFileSize);
  auto LSU = std::make_unique<LSUnit>(SM, Opts.LoadQueueSize,
                                      Opts.StoreQueueSize, Opts.AssumeNoAlias);
  auto HWS = std::make_unique<Scheduler>(SM, *LSU);

  // Create the pipeline stages.
  auto Fetch = std::make_unique<EntryStage>(SrcMgr);
  auto Dispatch =
      std::make_unique<DispatchStage>(STI, MRI, Opts.DispatchWidth, *RCU, *PRF);
  auto Execute =
      std::make_unique<ExecuteStage>(*HWS, Opts.EnableBottleneckAnalysis);
  auto Retire = std::make_unique<RetireStage>(*RCU, *PRF, *LSU);

  // Pass the ownership of all the hardware units to this Context.
  addHardwareUnit(std::move(RCU));
  addHardwareUnit(std::move(PRF));
  addHardwareUnit(std::move(LSU));
  addHardwareUnit(std::move(HWS));

  // Build the pipeline.
  auto StagePipeline = std::make_unique<Pipeline>();
  StagePipeline->appendStage(std::move(Fetch));
  if (Opts.MicroOpQueueSize)
    StagePipeline->appendStage(std::make_unique<MicroOpQueueStage>(
        Opts.MicroOpQueueSize, Opts.DecodersThroughput));
  StagePipeline->appendStage(std::move(Dispatch));
  StagePipeline->appendStage(std::move(Execute));
  StagePipeline->appendStage(std::move(Retire));
  return StagePipeline;
}

std::unique_ptr<Pipeline>
Context::createInOrderPipeline(const PipelineOptions &Opts, SourceMgr &SrcMgr,
                               CustomBehaviour &CB) {
  const MCSchedModel &SM = STI.getSchedModel();
  auto PRF = std::make_unique<RegisterFile>(SM, MRI, Opts.RegisterFileSize);
  auto LSU = std::make_unique<LSUnit>(SM, Opts.LoadQueueSize,
                                      Opts.StoreQueueSize, Opts.AssumeNoAlias);

  // Create the pipeline stages.
  auto Entry = std::make_unique<EntryStage>(SrcMgr);
  auto InOrderIssue = std::make_unique<InOrderIssueStage>(STI, *PRF, CB, *LSU);
  auto StagePipeline = std::make_unique<Pipeline>();

  // Pass the ownership of all the hardware units to this Context.
  addHardwareUnit(std::move(PRF));
  addHardwareUnit(std::move(LSU));

  // Build the pipeline.
  StagePipeline->appendStage(std::move(Entry));
  StagePipeline->appendStage(std::move(InOrderIssue));
  return StagePipeline;
}

} // namespace mca
} // namespace vm::core
