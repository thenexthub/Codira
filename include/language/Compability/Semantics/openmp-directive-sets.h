//===-- language/Compability/Semantics/openmp-directive-sets.h ---------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SEMANTICS_OPENMP_DIRECTIVE_SETS_H_
#define LANGUAGE_COMPABILITY_SEMANTICS_OPENMP_DIRECTIVE_SETS_H_

#include "language/Compability/Common/enum-set.h"
#include "toolchain/Frontend/OpenMP/OMPConstants.h"

using OmpDirectiveSet = language::Compability::common::EnumSet<toolchain::omp::Directive,
    toolchain::omp::Directive_enumSize>;

namespace toolchain::omp {
//===----------------------------------------------------------------------===//
// Directive sets for single directives
//===----------------------------------------------------------------------===//
// - top<Directive>Set: The directive appears alone or as the first in a
//   compound construct.
// - bottom<Directive>Set: The directive appears alone or as the last in a
//   compound construct.
// - all<Directive>Set: All standalone or compound uses of the directive.

static const OmpDirectiveSet topDistributeSet{
    Directive::OMPD_distribute,
    Directive::OMPD_distribute_parallel_do,
    Directive::OMPD_distribute_parallel_do_simd,
    Directive::OMPD_distribute_simd,
};

static const OmpDirectiveSet allDistributeSet{
    OmpDirectiveSet{
        Directive::OMPD_target_teams_distribute,
        Directive::OMPD_target_teams_distribute_parallel_do,
        Directive::OMPD_target_teams_distribute_parallel_do_simd,
        Directive::OMPD_target_teams_distribute_simd,
        Directive::OMPD_teams_distribute,
        Directive::OMPD_teams_distribute_parallel_do,
        Directive::OMPD_teams_distribute_parallel_do_simd,
        Directive::OMPD_teams_distribute_simd,
    } | topDistributeSet,
};

static const OmpDirectiveSet topDoSet{
    Directive::OMPD_do,
    Directive::OMPD_do_simd,
};

static const OmpDirectiveSet allDoSet{
    OmpDirectiveSet{
        Directive::OMPD_distribute_parallel_do,
        Directive::OMPD_distribute_parallel_do_simd,
        Directive::OMPD_parallel_do,
        Directive::OMPD_parallel_do_simd,
        Directive::OMPD_target_parallel_do,
        Directive::OMPD_target_parallel_do_simd,
        Directive::OMPD_target_teams_distribute_parallel_do,
        Directive::OMPD_target_teams_distribute_parallel_do_simd,
        Directive::OMPD_teams_distribute_parallel_do,
        Directive::OMPD_teams_distribute_parallel_do_simd,
    } | topDoSet,
};

static const OmpDirectiveSet topLoopSet{
    Directive::OMPD_loop,
};

static const OmpDirectiveSet allLoopSet{
    OmpDirectiveSet{
        Directive::OMPD_parallel_loop,
        Directive::OMPD_target_parallel_loop,
        Directive::OMPD_target_teams_loop,
        Directive::OMPD_teams_loop,
    } | topLoopSet,
};

static const OmpDirectiveSet topParallelSet{
    Directive::OMPD_parallel,
    Directive::OMPD_parallel_do,
    Directive::OMPD_parallel_do_simd,
    Directive::OMPD_parallel_loop,
    Directive::OMPD_parallel_masked_taskloop,
    Directive::OMPD_parallel_masked_taskloop_simd,
    Directive::OMPD_parallel_master_taskloop,
    Directive::OMPD_parallel_master_taskloop_simd,
    Directive::OMPD_parallel_sections,
    Directive::OMPD_parallel_workshare,
};

static const OmpDirectiveSet allParallelSet{
    OmpDirectiveSet{
        Directive::OMPD_distribute_parallel_do,
        Directive::OMPD_distribute_parallel_do_simd,
        Directive::OMPD_target_parallel,
        Directive::OMPD_target_parallel_do,
        Directive::OMPD_target_parallel_do_simd,
        Directive::OMPD_target_parallel_loop,
        Directive::OMPD_target_teams_distribute_parallel_do,
        Directive::OMPD_target_teams_distribute_parallel_do_simd,
        Directive::OMPD_teams_distribute_parallel_do,
        Directive::OMPD_teams_distribute_parallel_do_simd,
    } | topParallelSet,
};

static const OmpDirectiveSet topSimdSet{
    Directive::OMPD_simd,
};

static const OmpDirectiveSet allSimdSet{
    OmpDirectiveSet{
        Directive::OMPD_distribute_parallel_do_simd,
        Directive::OMPD_distribute_simd,
        Directive::OMPD_do_simd,
        Directive::OMPD_masked_taskloop_simd,
        Directive::OMPD_master_taskloop_simd,
        Directive::OMPD_parallel_do_simd,
        Directive::OMPD_parallel_masked_taskloop_simd,
        Directive::OMPD_parallel_master_taskloop_simd,
        Directive::OMPD_target_parallel_do_simd,
        Directive::OMPD_target_simd,
        Directive::OMPD_target_teams_distribute_parallel_do_simd,
        Directive::OMPD_target_teams_distribute_simd,
        Directive::OMPD_taskloop_simd,
        Directive::OMPD_teams_distribute_parallel_do_simd,
        Directive::OMPD_teams_distribute_simd,
    } | topSimdSet,
};

static const OmpDirectiveSet topTargetSet{
    Directive::OMPD_target,
    Directive::OMPD_target_parallel,
    Directive::OMPD_target_parallel_do,
    Directive::OMPD_target_parallel_do_simd,
    Directive::OMPD_target_parallel_loop,
    Directive::OMPD_target_simd,
    Directive::OMPD_target_teams,
    Directive::OMPD_target_teams_distribute,
    Directive::OMPD_target_teams_distribute_parallel_do,
    Directive::OMPD_target_teams_distribute_parallel_do_simd,
    Directive::OMPD_target_teams_distribute_simd,
    Directive::OMPD_target_teams_loop,
};

static const OmpDirectiveSet allTargetSet{topTargetSet};

static const OmpDirectiveSet topTaskloopSet{
    Directive::OMPD_taskloop,
    Directive::OMPD_taskloop_simd,
};

static const OmpDirectiveSet allTaskloopSet{
    OmpDirectiveSet{
        Directive::OMPD_masked_taskloop,
        Directive::OMPD_masked_taskloop_simd,
        Directive::OMPD_master_taskloop,
        Directive::OMPD_master_taskloop_simd,
        Directive::OMPD_parallel_masked_taskloop,
        Directive::OMPD_parallel_masked_taskloop_simd,
        Directive::OMPD_parallel_master_taskloop,
        Directive::OMPD_parallel_master_taskloop_simd,
    } | topTaskloopSet,
};

static const OmpDirectiveSet topTeamsSet{
    Directive::OMPD_teams,
    Directive::OMPD_teams_distribute,
    Directive::OMPD_teams_distribute_parallel_do,
    Directive::OMPD_teams_distribute_parallel_do_simd,
    Directive::OMPD_teams_distribute_simd,
    Directive::OMPD_teams_loop,
};

static const OmpDirectiveSet bottomTeamsSet{
    Directive::OMPD_target_teams,
    Directive::OMPD_teams,
};

static const OmpDirectiveSet allTeamsSet{
    OmpDirectiveSet{
        Directive::OMPD_target_teams,
        Directive::OMPD_target_teams_distribute,
        Directive::OMPD_target_teams_distribute_parallel_do,
        Directive::OMPD_target_teams_distribute_parallel_do_simd,
        Directive::OMPD_target_teams_distribute_simd,
        Directive::OMPD_target_teams_loop,
    } | topTeamsSet,
};

//===----------------------------------------------------------------------===//
// Directive sets for groups of multiple directives
//===----------------------------------------------------------------------===//

// Composite constructs
static const OmpDirectiveSet allDistributeParallelDoSet{
    allDistributeSet & allParallelSet & allDoSet};
static const OmpDirectiveSet allDistributeParallelDoSimdSet{
    allDistributeSet & allParallelSet & allDoSet & allSimdSet};
static const OmpDirectiveSet allDistributeSimdSet{
    allDistributeSet & allSimdSet};
static const OmpDirectiveSet allDoSimdSet{allDoSet & allSimdSet};
static const OmpDirectiveSet allTaskloopSimdSet{allTaskloopSet & allSimdSet};

static const OmpDirectiveSet compositeConstructSet{
    Directive::OMPD_distribute_parallel_do,
    Directive::OMPD_distribute_parallel_do_simd,
    Directive::OMPD_distribute_simd,
    Directive::OMPD_do_simd,
    Directive::OMPD_taskloop_simd,
};

static const OmpDirectiveSet blockConstructSet{
    Directive::OMPD_masked,
    Directive::OMPD_master,
    Directive::OMPD_ordered,
    Directive::OMPD_parallel,
    Directive::OMPD_parallel_masked,
    Directive::OMPD_parallel_master,
    Directive::OMPD_parallel_workshare,
    Directive::OMPD_scope,
    Directive::OMPD_single,
    Directive::OMPD_target,
    Directive::OMPD_target_data,
    Directive::OMPD_target_parallel,
    Directive::OMPD_target_teams,
    Directive::OMPD_task,
    Directive::OMPD_taskgroup,
    Directive::OMPD_teams,
    Directive::OMPD_workshare,
};

static const OmpDirectiveSet loopConstructSet{
    Directive::OMPD_distribute,
    Directive::OMPD_distribute_parallel_do,
    Directive::OMPD_distribute_parallel_do_simd,
    Directive::OMPD_distribute_simd,
    Directive::OMPD_do,
    Directive::OMPD_do_simd,
    Directive::OMPD_loop,
    Directive::OMPD_masked_taskloop,
    Directive::OMPD_masked_taskloop_simd,
    Directive::OMPD_master_taskloop,
    Directive::OMPD_master_taskloop_simd,
    Directive::OMPD_parallel_do,
    Directive::OMPD_parallel_do_simd,
    Directive::OMPD_parallel_loop,
    Directive::OMPD_parallel_masked_taskloop,
    Directive::OMPD_parallel_masked_taskloop_simd,
    Directive::OMPD_parallel_master_taskloop,
    Directive::OMPD_parallel_master_taskloop_simd,
    Directive::OMPD_simd,
    Directive::OMPD_target_loop,
    Directive::OMPD_target_parallel_do,
    Directive::OMPD_target_parallel_do_simd,
    Directive::OMPD_target_parallel_loop,
    Directive::OMPD_target_simd,
    Directive::OMPD_target_teams_distribute,
    Directive::OMPD_target_teams_distribute_parallel_do,
    Directive::OMPD_target_teams_distribute_parallel_do_simd,
    Directive::OMPD_target_teams_distribute_simd,
    Directive::OMPD_target_teams_loop,
    Directive::OMPD_taskloop,
    Directive::OMPD_taskloop_simd,
    Directive::OMPD_teams_distribute,
    Directive::OMPD_teams_distribute_parallel_do,
    Directive::OMPD_teams_distribute_parallel_do_simd,
    Directive::OMPD_teams_distribute_simd,
    Directive::OMPD_teams_loop,
    Directive::OMPD_tile,
    Directive::OMPD_unroll,
};

static const OmpDirectiveSet nonPartialVarSet{
    Directive::OMPD_allocate,
    Directive::OMPD_allocators,
    Directive::OMPD_threadprivate,
    Directive::OMPD_declare_target,
};

static const OmpDirectiveSet taskGeneratingSet{
    OmpDirectiveSet{
        Directive::OMPD_task,
    } | allTaskloopSet,
};

static const OmpDirectiveSet workShareSet{
    OmpDirectiveSet{
        Directive::OMPD_workshare,
        Directive::OMPD_parallel_workshare,
        Directive::OMPD_parallel_sections,
        Directive::OMPD_scope,
        Directive::OMPD_sections,
        Directive::OMPD_single,
    } | allDoSet,
};

//===----------------------------------------------------------------------===//
// Directive sets for parent directives that do allow/not allow a construct
//===----------------------------------------------------------------------===//

static const OmpDirectiveSet scanParentAllowedSet{allDoSet | allSimdSet};

//===----------------------------------------------------------------------===//
// Directive sets for allowed/not allowed nested directives
//===----------------------------------------------------------------------===//

static const OmpDirectiveSet nestedBarrierErrSet{
    OmpDirectiveSet{
        Directive::OMPD_atomic,
        Directive::OMPD_critical,
        Directive::OMPD_master,
        Directive::OMPD_ordered,
    } | taskGeneratingSet |
        workShareSet,
};

static const OmpDirectiveSet nestedCancelDoAllowedSet{
    Directive::OMPD_distribute_parallel_do,
    Directive::OMPD_do,
    Directive::OMPD_parallel_do,
    Directive::OMPD_target_parallel_do,
    Directive::OMPD_target_teams_distribute_parallel_do,
    Directive::OMPD_teams_distribute_parallel_do,
};

static const OmpDirectiveSet nestedCancelParallelAllowedSet{
    Directive::OMPD_parallel,
    Directive::OMPD_target_parallel,
};

static const OmpDirectiveSet nestedCancelSectionsAllowedSet{
    Directive::OMPD_parallel_sections,
    Directive::OMPD_sections,
};

static const OmpDirectiveSet nestedCancelTaskgroupAllowedSet{
    Directive::OMPD_task,
    Directive::OMPD_taskloop,
};

static const OmpDirectiveSet nestedMasterErrSet{
    OmpDirectiveSet{
        Directive::OMPD_atomic,
    } | taskGeneratingSet |
        workShareSet,
};

static const OmpDirectiveSet nestedOrderedDoAllowedSet{
    Directive::OMPD_do,
    Directive::OMPD_parallel_do,
    Directive::OMPD_target_parallel_do,
};

static const OmpDirectiveSet nestedOrderedErrSet{
    Directive::OMPD_atomic,
    Directive::OMPD_critical,
    Directive::OMPD_ordered,
    Directive::OMPD_task,
    Directive::OMPD_taskloop,
};

static const OmpDirectiveSet nestedOrderedParallelErrSet{
    Directive::OMPD_parallel,
    Directive::OMPD_parallel_sections,
    Directive::OMPD_parallel_workshare,
    Directive::OMPD_target_parallel,
};

static const OmpDirectiveSet nestedReduceWorkshareAllowedSet{
    Directive::OMPD_do,
    Directive::OMPD_do_simd,
    Directive::OMPD_sections,
};

static const OmpDirectiveSet nestedTeamsAllowedSet{
    Directive::OMPD_distribute,
    Directive::OMPD_distribute_parallel_do,
    Directive::OMPD_distribute_parallel_do_simd,
    Directive::OMPD_distribute_simd,
    Directive::OMPD_loop,
    Directive::OMPD_parallel,
    Directive::OMPD_parallel_do,
    Directive::OMPD_parallel_do_simd,
    Directive::OMPD_parallel_master,
    Directive::OMPD_parallel_master_taskloop,
    Directive::OMPD_parallel_master_taskloop_simd,
    Directive::OMPD_parallel_sections,
    Directive::OMPD_parallel_workshare,
};

static const OmpDirectiveSet nestedWorkshareErrSet{
    OmpDirectiveSet{
        Directive::OMPD_atomic,
        Directive::OMPD_critical,
        Directive::OMPD_master,
        Directive::OMPD_ordered,
        Directive::OMPD_task,
        Directive::OMPD_taskloop,
    } | workShareSet,
};

//===----------------------------------------------------------------------===//
// Misc directive sets
//===----------------------------------------------------------------------===//

// Simple standalone directives than can be erased by -fopenmp-simd.
static const OmpDirectiveSet simpleStandaloneNonSimdOnlySet{
    Directive::OMPD_taskyield,
    Directive::OMPD_barrier,
    Directive::OMPD_ordered,
    Directive::OMPD_target_enter_data,
    Directive::OMPD_target_exit_data,
    Directive::OMPD_target_update,
    Directive::OMPD_taskwait,
};

} // namespace toolchain::omp

#endif // FORTRAN_SEMANTICS_OPENMP_DIRECTIVE_SETS_H_
