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

#ifndef SWIFT_IRGEN_GENCONCURRENCY_H
#define SWIFT_IRGEN_GENCONCURRENCY_H

#include "language/AST/Types.h"
#include "language/Basic/LLVM.h"
#include "language/SIL/ApplySite.h"
#include "llvm/IR/CallingConv.h"

#include "Callee.h"
#include "GenHeap.h"
#include "IRGenModule.h"

namespace llvm {
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
void emitBuildDefaultActorExecutorRef(IRGenFunction &IGF, llvm::Value *actor,
                                      Explosion &out);

/// Emit the buildOrdinaryTaskExecutorRef builtin.
void emitBuildOrdinaryTaskExecutorRef(
    IRGenFunction &IGF, llvm::Value *executor, CanType executorType,
    ProtocolConformanceRef executorConformance, Explosion &out);

/// Emit the buildOrdinarySerialExecutorRef builtin.
void emitBuildOrdinarySerialExecutorRef(IRGenFunction &IGF,
                                        llvm::Value *executor,
                                        CanType executorType,
                                        ProtocolConformanceRef executorConformance,
                                        Explosion &out);

/// Emit the buildComplexEqualitySerialExecutorRef builtin.
void emitBuildComplexEqualitySerialExecutorRef(IRGenFunction &IGF,
                                        llvm::Value *executor,
                                        CanType executorType,
                                        ProtocolConformanceRef executorConformance,
                                        Explosion &out);

/// Emit the getCurrentExecutor builtin.
void emitGetCurrentExecutor(IRGenFunction &IGF, Explosion &out);

/// Emit the createAsyncLet builtin.
llvm::Value *emitBuiltinStartAsyncLet(IRGenFunction &IGF,
                                      llvm::Value *taskOptions,
                                      llvm::Value *taskFunction,
                                      llvm::Value *localContextInfo,
                                      llvm::Value *resultBuffer,
                                      SubstitutionMap subs);

/// Emit the endAsyncLet builtin.
void emitEndAsyncLet(IRGenFunction &IGF, llvm::Value *alet);

/// Emit the createTaskGroup builtin.
llvm::Value *emitCreateTaskGroup(IRGenFunction &IGF, SubstitutionMap subs,
                                 llvm::Value *groupFlags);

/// Emit the destroyTaskGroup builtin.
void emitDestroyTaskGroup(IRGenFunction &IGF, llvm::Value *group);

void emitTaskRunInline(IRGenFunction &IGF, SubstitutionMap subs,
                       llvm::Value *result, llvm::Value *closure,
                       llvm::Value *closureContext);

void emitTaskCancel(IRGenFunction &IGF, llvm::Value *task);

llvm::Value *maybeAddEmbeddedSwiftResultTypeInfo(IRGenFunction &IGF,
                                                 llvm::Value *taskOptions,
                                                 CanType formalResultType);

/// Emit a call to swift_task_create[_f] with the given flags, options, and
/// task function.
std::pair<llvm::Value *, llvm::Value *>
emitTaskCreate(IRGenFunction &IGF, llvm::Value *flags,
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
