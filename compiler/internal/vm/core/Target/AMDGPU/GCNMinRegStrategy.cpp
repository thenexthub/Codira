//===- GCNMinRegStrategy.cpp ----------------------------------------------===//
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
///
/// \file
/// This file defines and implements the class GCNMinRegScheduler, which
/// implements an experimental, simple scheduler whose main goal is to learn
/// ways about consuming less possible registers for a region.
///
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/ScheduleDAG.h"
using namespace vm::core;

#define DEBUG_TYPE "machine-scheduler"

namespace {

class GCNMinRegScheduler {
  struct Candidate : ilist_node<Candidate> {
    const SUnit *SU;
    int Priority;

    Candidate(const SUnit *SU_, int Priority_ = 0)
      : SU(SU_), Priority(Priority_) {}
  };

  SpecificBumpPtrAllocator<Candidate> Alloc;
  using Queue = simple_ilist<Candidate>;
  Queue RQ; // Ready queue

  std::vector<unsigned> NumPreds;

  bool isScheduled(const SUnit *SU) const {
    assert(!SU->isBoundaryNode());
    return NumPreds[SU->NodeNum] == std::numeric_limits<unsigned>::max();
  }

  void setIsScheduled(const SUnit *SU)  {
    assert(!SU->isBoundaryNode());
    NumPreds[SU->NodeNum] = std::numeric_limits<unsigned>::max();
  }

  unsigned getNumPreds(const SUnit *SU) const {
    assert(!SU->isBoundaryNode());
    assert(NumPreds[SU->NodeNum] != std::numeric_limits<unsigned>::max());
    return NumPreds[SU->NodeNum];
  }

  unsigned decNumPreds(const SUnit *SU) {
    assert(!SU->isBoundaryNode());
    assert(NumPreds[SU->NodeNum] != std::numeric_limits<unsigned>::max());
    return --NumPreds[SU->NodeNum];
  }

  void initNumPreds(const decltype(ScheduleDAG::SUnits) &SUnits);

  int getReadySuccessors(const SUnit *SU) const;
  int getNotReadySuccessors(const SUnit *SU) const;

  template <typename Calc>
  unsigned findMax(unsigned Num, Calc C);

  Candidate* pickCandidate();

  void bumpPredsPriority(const SUnit *SchedSU, int Priority);
  void releaseSuccessors(const SUnit* SU, int Priority);

public:
  std::vector<const SUnit*> schedule(ArrayRef<const SUnit*> TopRoots,
                                     const ScheduleDAG &DAG);
};

} // end anonymous namespace

void GCNMinRegScheduler::initNumPreds(const decltype(ScheduleDAG::SUnits) &SUnits) {
  NumPreds.resize(SUnits.size());
  for (unsigned I = 0; I < SUnits.size(); ++I)
    NumPreds[I] = SUnits[I].NumPredsLeft;
}

int GCNMinRegScheduler::getReadySuccessors(const SUnit *SU) const {
  unsigned NumSchedSuccs = 0;
  for (auto SDep : SU->Succs) {
    bool wouldBeScheduled = true;
    for (auto PDep : SDep.getSUnit()->Preds) {
      auto *PSU = PDep.getSUnit();
      assert(!PSU->isBoundaryNode());
      if (PSU != SU && !isScheduled(PSU)) {
        wouldBeScheduled = false;
        break;
      }
    }
    NumSchedSuccs += wouldBeScheduled ? 1 : 0;
  }
  return NumSchedSuccs;
}

int GCNMinRegScheduler::getNotReadySuccessors(const SUnit *SU) const {
  return SU->Succs.size() - getReadySuccessors(SU);
}

template <typename Calc>
unsigned GCNMinRegScheduler::findMax(unsigned Num, Calc C) {
  assert(!RQ.empty() && Num <= RQ.size());

  using T = decltype(C(*RQ.begin())) ;

  T Max = std::numeric_limits<T>::min();
  unsigned NumMax = 0;
  for (auto I = RQ.begin(); Num; --Num) {
    T Cur = C(*I);
    if (Cur >= Max) {
      if (Cur > Max) {
        Max = Cur;
        NumMax = 1;
      } else
        ++NumMax;
      auto &Cand = *I++;
      RQ.remove(Cand);
      RQ.push_front(Cand);
      continue;
    }
    ++I;
  }
  return NumMax;
}

GCNMinRegScheduler::Candidate* GCNMinRegScheduler::pickCandidate() {
  do {
    unsigned Num = RQ.size();
    if (Num == 1) break;

    LLVM_DEBUG(dbgs() << "\nSelecting max priority candidates among " << Num
                      << '\n');
    Num = findMax(Num, [=](const Candidate &C) { return C.Priority; });
    if (Num == 1) break;

    LLVM_DEBUG(dbgs() << "\nSelecting min non-ready producing candidate among "
                      << Num << '\n');
    Num = findMax(Num, [=](const Candidate &C) {
      const auto *SU = C.SU;
      int Res = getNotReadySuccessors(SU);
      LLVM_DEBUG(dbgs() << "SU(" << SU->NodeNum << ") would left non-ready "
                        << Res << " successors, metric = " << -Res << '\n');
      return -Res;
    });
    if (Num == 1) break;

    LLVM_DEBUG(dbgs() << "\nSelecting most producing candidate among " << Num
                      << '\n');
    Num = findMax(Num, [=](const Candidate &C) {
      const auto *SU = C.SU;
      auto Res = getReadySuccessors(SU);
      LLVM_DEBUG(dbgs() << "SU(" << SU->NodeNum << ") would make ready " << Res
                        << " successors, metric = " << Res << '\n');
      return Res;
    });
    if (Num == 1) break;

    Num = Num ? Num : RQ.size();
    LLVM_DEBUG(
        dbgs()
        << "\nCan't find best candidate, selecting in program order among "
        << Num << '\n');
    Num = findMax(Num, [=](const Candidate &C) { return -(int64_t)C.SU->NodeNum; });
    assert(Num == 1);
  } while (false);

  return &RQ.front();
}

void GCNMinRegScheduler::bumpPredsPriority(const SUnit *SchedSU, int Priority) {
  SmallPtrSet<const SUnit*, 32> Set;
  for (const auto &S : SchedSU->Succs) {
    if (S.getSUnit()->isBoundaryNode() || isScheduled(S.getSUnit()) ||
        S.getKind() != SDep::Data)
      continue;
    for (const auto &P : S.getSUnit()->Preds) {
      auto *PSU = P.getSUnit();
      assert(!PSU->isBoundaryNode());
      if (PSU != SchedSU && !isScheduled(PSU)) {
        Set.insert(PSU);
      }
    }
  }
  SmallVector<const SUnit*, 32> Worklist(Set.begin(), Set.end());
  while (!Worklist.empty()) {
    const auto *SU = Worklist.pop_back_val();
    assert(!SU->isBoundaryNode());
    for (const auto &P : SU->Preds) {
      if (!P.getSUnit()->isBoundaryNode() && !isScheduled(P.getSUnit()) &&
          Set.insert(P.getSUnit()).second)
        Worklist.push_back(P.getSUnit());
    }
  }
  LLVM_DEBUG(dbgs() << "Make the predecessors of SU(" << SchedSU->NodeNum
                    << ")'s non-ready successors of " << Priority
                    << " priority in ready queue: ");
  for (auto &C : RQ) {
    if (Set.count(C.SU)) {
      C.Priority = Priority;
      LLVM_DEBUG(dbgs() << " SU(" << C.SU->NodeNum << ')');
    }
  }
  LLVM_DEBUG(dbgs() << '\n');
}

void GCNMinRegScheduler::releaseSuccessors(const SUnit* SU, int Priority) {
  for (const auto &S : SU->Succs) {
    auto *SuccSU = S.getSUnit();
    if (S.isWeak())
      continue;
    assert(SuccSU->isBoundaryNode() || getNumPreds(SuccSU) > 0);
    if (!SuccSU->isBoundaryNode() && decNumPreds(SuccSU) == 0)
      RQ.push_front(*new (Alloc.Allocate()) Candidate(SuccSU, Priority));
  }
}

std::vector<const SUnit*>
GCNMinRegScheduler::schedule(ArrayRef<const SUnit*> TopRoots,
                             const ScheduleDAG &DAG) {
  const auto &SUnits = DAG.SUnits;
  std::vector<const SUnit*> Schedule;
  Schedule.reserve(SUnits.size());

  initNumPreds(SUnits);

  int StepNo = 0;

  for (const auto *SU : TopRoots) {
    RQ.push_back(*new (Alloc.Allocate()) Candidate(SU, StepNo));
  }
  releaseSuccessors(&DAG.EntrySU, StepNo);

  while (!RQ.empty()) {
    LLVM_DEBUG(dbgs() << "\n=== Picking candidate, Step = " << StepNo
                      << "\n"
                         "Ready queue:";
               for (auto &C
                    : RQ) dbgs()
               << ' ' << C.SU->NodeNum << "(P" << C.Priority << ')';
               dbgs() << '\n';);

    auto *C = pickCandidate();
    assert(C);
    RQ.remove(*C);
    const auto *SU = C->SU;
    LLVM_DEBUG(dbgs() << "Selected "; DAG.dumpNode(*SU));

    releaseSuccessors(SU, StepNo);
    Schedule.push_back(SU);
    setIsScheduled(SU);

    if (getReadySuccessors(SU) == 0)
      bumpPredsPriority(SU, StepNo);

    ++StepNo;
  }
  assert(SUnits.size() == Schedule.size());

  return Schedule;
}

namespace vm::core {

std::vector<const SUnit*> makeMinRegSchedule(ArrayRef<const SUnit*> TopRoots,
                                             const ScheduleDAG &DAG) {
  GCNMinRegScheduler S;
  return S.schedule(TopRoots, DAG);
}

} // end namespace vm::core
