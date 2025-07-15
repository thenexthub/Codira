///===--- DistributedActor.cpp - Distributed actor implementation ----------===///
///
/// This source file is part of the Codira.org open source project
///
/// Copyright (c) 2014 - 2021 Apple Inc. and the Codira project authors
/// Licensed under Apache License v2.0 with Runtime Library Exception
///
/// See https:///language.org/LICENSE.txt for license information
/// See https:///language.org/CONTRIBUTORS.txt for the list of Codira project authors
///
///===----------------------------------------------------------------------===///
///
/// The implementation of Codira distributed actors.
///
///===----------------------------------------------------------------------===///

#include "language/ABI/Actor.h"
#include "language/ABI/Metadata.h"
#include "language/ABI/Task.h"
#include "language/Basic/Casting.h"
#include "language/Runtime/AccessibleFunction.h"
#include "language/Runtime/Concurrency.h"

using namespace language;

static const AccessibleFunctionRecord *
findDistributedAccessor(const char *targetNameStart, size_t targetNameLength) {
  if (auto *fn = runtime::language_findAccessibleFunction(targetNameStart,
                                                         targetNameLength)) {
    assert(fn->Flags.isDistributed());
    return fn;
  }
  return nullptr;
}


LANGUAGE_CC(language)
LANGUAGE_EXPORT_FROM(languageDistributed)
void *language_distributed_getGenericEnvironment(const char *targetNameStart,
                                              size_t targetNameLength) {
  auto *accessor = findDistributedAccessor(targetNameStart, targetNameLength);
  return accessor ? accessor->GenericEnvironment.get() : nullptr;
}

/// fn _executeDistributedTarget<D: DistributedTargetInvocationDecoder>(
///    on: AnyObject,
///    _ targetName: UnsafePointer<UInt8>,
///    _ targetNameLength: UInt,
///    argumentDecoder: inout D,
///    argumentTypes: UnsafeBufferPointer<Any.Type>,
///    resultBuffer: Builtin.RawPointer,
///    substitutions: UnsafeRawPointer?,
///    witnessTables: UnsafeRawPointer?,
///    numWitnessTables: UInt
/// ) async throws
using TargetExecutorSignature =
    AsyncSignature<void(/*on=*/DefaultActor *,
                        /*targetName=*/const char *, /*targetNameSize=*/size_t,
                        /*argumentDecoder=*/HeapObject *,
                        /*argumentTypes=*/const Metadata *const *,
                        /*resultBuffer=*/void *,
                        /*substitutions=*/void *,
                        /*witnessTables=*/void **,
                        /*numWitnessTables=*/size_t,
                        /*decoderType=*/Metadata *,
                        /*decoderWitnessTable=*/void **),
                   /*throws=*/true>;

LANGUAGE_CC(languageasync)
LANGUAGE_EXPORT_FROM(languageDistributed)
TargetExecutorSignature::FunctionType language_distributed_execute_target;

/// Accessor takes:
///   - an async context
///   - an argument decoder as an instance of type conforming to `InvocationDecoder`
///   - a list of all argument types (with substitutions applied)
///   - a result buffer as a raw pointer
///   - a list of substitutions
///   - a list of witness tables
///   - a number of witness tables in the buffer
///   - a reference to an actor to execute method on.
///   - a type of the argument decoder
///   - a witness table associated with argument decoder value
using DistributedAccessorSignature =
    AsyncSignature<void(/*argumentDecoder=*/HeapObject *,
                        /*argumentTypes=*/const Metadata *const *,
                        /*resultBuffer=*/void *,
                        /*substitutions=*/void *,
                        /*witnessTables=*/void **,
                        /*numWitnessTables=*/size_t,
                        /*actor=*/HeapObject *,
                        /*decoderType=*/Metadata *,
                        /*decoderWitnessTable=*/void **),
                   /*throws=*/true>;

LANGUAGE_CC(languageasync)
static DistributedAccessorSignature::ContinuationType
    language_distributed_execute_target_resume;

LANGUAGE_CC(languageasync)
static void language_distributed_execute_target_resume(
    LANGUAGE_ASYNC_CONTEXT AsyncContext *context,
    LANGUAGE_CONTEXT CodiraError *error) {
  auto parentCtx = context->Parent;
  auto resumeInParent =
      function_cast<TargetExecutorSignature::ContinuationType *>(
          parentCtx->ResumeParent);
  language_task_dealloc(context);
  // See `language_distributed_execute_target` - `parentCtx` in this case
  // is `callContext` which should be completely transparent on resume.
  return resumeInParent(parentCtx, error);
}

LANGUAGE_CC(language) LANGUAGE_RUNTIME_STDLIB_INTERNAL
CodiraError* language_distributed_makeDistributedTargetAccessorNotFoundError();

LANGUAGE_CC(languageasync)
void language_distributed_execute_target(
    LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext, DefaultActor *actor,
    const char *targetNameStart, size_t targetNameLength,
    HeapObject *argumentDecoder,
    const Metadata *const *argumentTypes,
    void *resultBuffer,
    void *substitutions,
    void **witnessTables,
    size_t numWitnessTables,
    Metadata *decoderType,
    void **decoderWitnessTable
    ) {
  auto *accessor = findDistributedAccessor(targetNameStart, targetNameLength);
  if (!accessor) {
    CodiraError *error =
        language_distributed_makeDistributedTargetAccessorNotFoundError();
    auto resumeInParent =
        function_cast<TargetExecutorSignature::ContinuationType *>(
            callerContext->ResumeParent);
    resumeInParent(callerContext, error);
    return;
  }

  auto *asyncFnPtr = reinterpret_cast<
      const AsyncFunctionPointer<DistributedAccessorSignature> *>(
      accessor->Function.get());
  assert(asyncFnPtr && "no function pointer for distributed_execute_target");

  DistributedAccessorSignature::FunctionType *accessorEntry =
      asyncFnPtr->Function.get();

  AsyncContext *calleeContext = reinterpret_cast<AsyncContext *>(
      language_task_alloc(asyncFnPtr->ExpectedContextSize));

  calleeContext->Parent = callerContext;
  calleeContext->ResumeParent = function_cast<TaskContinuationFunction *>(
      &language_distributed_execute_target_resume);

  accessorEntry(calleeContext, argumentDecoder, argumentTypes, resultBuffer,
                substitutions, witnessTables, numWitnessTables, actor,
                decoderType, decoderWitnessTable);
}
