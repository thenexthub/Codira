//===- PPCMachineScheduler.h - Custom PowerPC MI scheduler --*- C++ -*-===//
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
// Custom PowerPC MI scheduler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_POWERPC_POWERPCMACHINESCHEDULER_H
#define LLVM_LIB_TARGET_POWERPC_POWERPCMACHINESCHEDULER_H

#include "vm/core/CodeGen/MachineScheduler.h"

namespace vm::core {

/// A MachineSchedStrategy implementation for PowerPC pre RA scheduling.
class PPCPreRASchedStrategy : public GenericScheduler {
public:
  PPCPreRASchedStrategy(const MachineSchedContext *C) :
    GenericScheduler(C) {}
protected:
  bool tryCandidate(SchedCandidate &Cand, SchedCandidate &TryCand,
                    SchedBoundary *Zone) const override;

private:
  bool biasAddiLoadCandidate(SchedCandidate &Cand,
                             SchedCandidate &TryCand,
                             SchedBoundary &Zone) const;
};

/// A MachineSchedStrategy implementation for PowerPC post RA scheduling.
class PPCPostRASchedStrategy : public PostGenericScheduler {
public:
  PPCPostRASchedStrategy(const MachineSchedContext *C) :
    PostGenericScheduler(C) {}

protected:
  void initialize(ScheduleDAGMI *Dag) override;
  SUnit *pickNode(bool &IsTopNode) override;
  void enterMBB(MachineBasicBlock *MBB) override;
  void leaveMBB() override;

  bool tryCandidate(SchedCandidate &Cand, SchedCandidate &TryCand) override;
  bool biasAddiCandidate(SchedCandidate &Cand, SchedCandidate &TryCand) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_POWERPC_POWERPCMACHINESCHEDULER_H
