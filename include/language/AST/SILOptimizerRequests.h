//===--- SILOptimizerRequests.h - SILOptimizer Requests ---------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file defines SILOptimizer requests.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_REQUESTS_H
#define LANGUAGE_SILOPTIMIZER_REQUESTS_H

#include "language/AST/ASTTypeIDs.h"
#include "language/AST/EvaluatorDependencies.h"
#include "language/AST/SILGenRequests.h"
#include "language/AST/SimpleRequest.h"

namespace language {

namespace irgen {
class IRGenModule;
}

class SILModule;
class SILPassPipelinePlan;

struct SILPipelineExecutionDescriptor {
  SILModule *SM;

  // Note that we currently store a reference to the pipeline plan on the stack.
  // If ExecuteSILPipelineRequest ever becomes cached, we will need to adjust
  // this.
  const SILPassPipelinePlan &Plan;
  bool IsMandatory;
  irgen::IRGenModule *IRMod;

  bool operator==(const SILPipelineExecutionDescriptor &other) const;
  bool operator!=(const SILPipelineExecutionDescriptor &other) const {
    return !(*this == other);
  }
};

toolchain::hash_code hash_value(const SILPipelineExecutionDescriptor &desc);

/// Executes a SIL pipeline plan on a SIL module.
class ExecuteSILPipelineRequest
    : public SimpleRequest<ExecuteSILPipelineRequest,
                           evaluator::SideEffect(SILPipelineExecutionDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  evaluator::SideEffect
  evaluate(Evaluator &evaluator, SILPipelineExecutionDescriptor desc) const;
};

void simple_display(toolchain::raw_ostream &out,
                    const SILPipelineExecutionDescriptor &desc);

SourceLoc extractNearestSourceLoc(const SILPipelineExecutionDescriptor &desc);

/// Produces lowered SIL from a Codira file or module, ready for IRGen. This runs
/// the diagnostic, optimization, and lowering SIL passes.
class LoweredSILRequest
    : public SimpleRequest<LoweredSILRequest,
                           std::unique_ptr<SILModule>(ASTLoweringDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  std::unique_ptr<SILModule> evaluate(Evaluator &evaluator,
                                      ASTLoweringDescriptor desc) const;
};

/// Report that a request of the given kind is being evaluated, so it
/// can be recorded by the stats reporter.
template <typename Request>
void reportEvaluatedRequest(UnifiedStatsReporter &stats,
                            const Request &request);

/// The zone number for SILOptimizer.
#define LANGUAGE_TYPEID_ZONE SILOptimizer
#define LANGUAGE_TYPEID_HEADER "language/AST/SILOptimizerTypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER

// Set up reporting of evaluated requests.
#define LANGUAGE_REQUEST(Zone, RequestType, Sig, Caching, LocOptions)             \
template<>                                                                     \
inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,                \
                                   const RequestType &request) {               \
  ++stats.getFrontendCounters().RequestType;                                   \
}
#include "language/AST/SILOptimizerTypeIDZone.def"
#undef LANGUAGE_REQUEST

} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_REQUESTS_H
