//===-- MPITypes.h - Functionality to model MPI concepts --------*- C++ -*-===//
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
///
/// \file
/// This file provides definitions to model concepts of MPI. The mpi::Request
/// class defines a wrapper class, in order to make MPI requests trackable for
/// path-sensitive analysis.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MPICHECKER_MPITYPES_H
#define LANGUAGE_CORE_LIB_STATICANALYZER_CHECKERS_MPICHECKER_MPITYPES_H

#include "language/Core/StaticAnalyzer/Checkers/MPIFunctionClassifier.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "toolchain/ADT/SmallSet.h"

namespace language::Core {
namespace ento {
namespace mpi {

class Request {
public:
  enum State : unsigned char { Nonblocking, Wait };

  Request(State S) : CurrentState{S} {}

  void Profile(toolchain::FoldingSetNodeID &Id) const {
    Id.AddInteger(CurrentState);
  }

  bool operator==(const Request &ToCompare) const {
    return CurrentState == ToCompare.CurrentState;
  }

  const State CurrentState;
};

// The RequestMap stores MPI requests which are identified by their memory
// region. Requests are used in MPI to complete nonblocking operations with wait
// operations. A custom map implementation is used, in order to make it
// available in an arbitrary amount of translation units.
struct RequestMap {};
typedef toolchain::ImmutableMap<const language::Core::ento::MemRegion *,
                           language::Core::ento::mpi::Request>
    RequestMapImpl;

} // end of namespace: mpi

template <>
struct ProgramStateTrait<mpi::RequestMap>
    : public ProgramStatePartialTrait<mpi::RequestMapImpl> {
  static void *GDMIndex() {
    static int index = 0;
    return &index;
  }
};

} // end of namespace: ento
} // end of namespace: clang
#endif
