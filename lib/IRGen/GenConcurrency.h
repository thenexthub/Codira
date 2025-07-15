//===--- GenConcurrency.h - IRGen for concurrency features ------*- C++ -*-===//
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
// This file defines interfaces for emitting code for various concurrency
// features.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENCONCURRENCY_H
#define LANGUAGE_IRGEN_GENCONCURRENCY_H

#include "language/AST/Types.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/ApplySite.h"
#include "toolchain/IR/CallingConv.h"

#include "Callee.h"
#include "GenHeap.h"
#include "IRGenModule.h"

namespace toolchain {
class Value;
}

namespace language {
class CanType;
class ProtocolConformanceRef;
class SILType;

namespace irgen {
class Explosion;
class OptionalExplosion;
class IRGenFunction;

/// Emit the buildMainActorExecutorRef builtin.
void emitBuildMainActorExecutorRef(IRGenFunction &IGF, Explosion &out);

/// Emit the buildDefaultActorExecutorRef builtin.
void emitBuildDefaultActorExecutorRef(IRGenFunction &IGF, toolchain::Value *actor,
                                      Explosion &out);

/// Emit the buildOrdinaryTaskExecutorRef builtin.
void emitBuildOrdinaryTaskExecutorRef(
    IRGenFunction &IGF, toolchain::Value *executor, CanType executorType,
    ProtocolConformanceRef executorConformance, Explosion &out);

/// Emit the buildOrdinarySerialExecutorRef builtin.
void emitBuildOrdinarySerialExecutorRef(IRGenFunction &IGF,
                                        toolchain::Value *executor,
                                        CanType executorType,
                                        ProtocolConformanceRef executorConformance,
                                        Explosion &out);

/// Emit the buildComplexEqualitySerialExecutorRef builtin.
void emitBuildComplexEqualitySerialExecutorRef(IRGenFunction &IGF,
                                        toolchain::Value *executor,
                                        CanType executorType,
                                        ProtocolConformanceRef executorConformance,
                                        Explosion &out);

/// Emit the getCurrentExecutor builtin.
void emitGetCurrentExecutor(IRGenFunction &IGF, Explosion &out);

/// Emit the createAsyncLet builtin.
toolchain::Value *emitBuiltinStartAsyncLet(IRGenFunction &IGF,
                                      toolchain::Value *taskOptions,
                                      toolchain::Value *taskFunction,
                                      toolchain::Value *localContextInfo,
                                      toolchain::Value *resultBuffer,
                                      SubstitutionMap subs);

/// Emit the endAsyncLet builtin.
void emitEndAsyncLet(IRGenFunction &IGF, toolchain::Value *alet);

/// Emit the createTaskGroup builtin.
toolchain::Value *emitCreateTaskGroup(IRGenFunction &IGF, SubstitutionMap subs,
                                 toolchain::Value *groupFlags);

/// Emit the destroyTaskGroup builtin.
void emitDestroyTaskGroup(IRGenFunction &IGF, toolchain::Value *group);

void emitTaskRunInline(IRGenFunction &IGF, SubstitutionMap subs,
                       toolchain::Value *result, toolchain::Value *closure,
                       toolchain::Value *closureContext);

void emitTaskCancel(IRGenFunction &IGF, toolchain::Value *task);

toolchain::Value *maybeAddEmbeddedCodiraResultTypeInfo(IRGenFunction &IGF,
                                                 toolchain::Value *taskOptions,
                                                 CanType formalResultType);

/// Emit a call to language_task_create[_f] with the given flags, options, and
/// task function.
std::pair<toolchain::Value *, toolchain::Value *>
emitTaskCreate(IRGenFunction &IGF, toolchain::Value *flags,
               OptionalExplosion &initialExecutor,
               OptionalExplosion &taskGroup,
               OptionalExplosion &taskExecutorUnowned,
               OptionalExplosion &taskExecutorExistential,
               OptionalExplosion &taskName,
               Explosion &taskFunction,
               SubstitutionMap subs);

} // end namespace irgen
} // end namespace language

#endif
