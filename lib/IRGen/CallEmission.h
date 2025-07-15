//===--- CallEmission.h - Utility for emitting calls ------------*- C++ -*-===//
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
//  This file defines the CallEmitter class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_CALLEMISSION_H
#define LANGUAGE_IRGEN_CALLEMISSION_H

#include "Address.h"
#include "Callee.h"
#include "Explosion.h"
#include "Temporary.h"

namespace toolchain {
  class CallSite;
}

namespace language {
namespace irgen {

class Explosion;
class LoadableTypeInfo;
struct WitnessMetadata;
class FunctionPointer;

/// A plan for emitting a series of calls.
class CallEmission {
  enum class State { Emitting, Finished };
  State state = State::Emitting;

public:
  IRGenFunction &IGF;

protected:
  toolchain::Value *selfValue;

  /// The builtin/special arguments to pass to the call.
  SmallVector<toolchain::Value*, 8> Args;

  /// Temporaries required by the call.
  TemporarySet Temporaries;

  /// Temporaries required by the call that are not mapped to any
  /// SIL value.
  SmallVector<StackAddress, 8> RawTempraries;

  /// The function we're going to call.
  Callee CurCallee;

  /// Only valid if the SIL function has indirect return values.
  /// If the function has multiple indirect return values, this is the address
  /// of the first indirect return value.
  Address indirectReturnAddress;

  /// For C-functions: true if the return is indirect in SIL, but direct for a C-function.
  /// That can happen for "sensitive" structs.
  bool convertDirectToIndirectReturn = false;

  unsigned LastArgWritten;

  /// Whether this is a coroutine invocation.
  bool IsCoroutine;

  /// Whether this is a invocation for a coroutine that dynamically allocates
  /// in the callee.
  bool IsCalleeAllocatedCoroutine;

  /// Whether we've emitted the call for the current callee yet.  This
  /// is just for debugging purposes --- e.g. the destructor asserts
  /// that it's true --- but is otherwise derivable from
  /// RemainingArgsForCallee, at least between calls.
  bool EmittedCall;

  bool UseProfilingThunk = false;

  /// The basic block to which the call to a potentially throwing foreign
  /// function should jump to continue normal execution of the program.
  toolchain::BasicBlock *invokeNormalDest = nullptr;

  /// The basic block to which the call to a potentially throwing foreign
  /// function should jump to in case an exception has been thrown during the
  /// invocation of the call.
  toolchain::BasicBlock *invokeUnwindDest = nullptr;

  unsigned IndirectTypedErrorArgIdx = 0;

  std::optional<Explosion> typedErrorExplosion;

  virtual void setFromCallee();
  void emitToUnmappedMemory(Address addr);
  void emitToUnmappedExplosion(Explosion &out);
  virtual void emitCallToUnmappedExplosion(toolchain::CallBase *call,
                                           Explosion &out) = 0;
  void emitYieldsToExplosion(Explosion &out);
  void setKeyPathAccessorArguments(Explosion &in, bool isOutlined,
                                   Explosion &out);
  virtual FunctionPointer getCalleeFunctionPointer() = 0;
  toolchain::CallBase *emitCallSite();

  virtual toolchain::CallBase *createCall(const FunctionPointer &fn,
                                     ArrayRef<toolchain::Value *> args) = 0;

  void externalizeArguments(IRGenFunction &IGF, const Callee &callee,
                                   Explosion &in, Explosion &out,
                                   TemporarySet &temporaries,
                                   bool isOutlined);

  bool mayReturnTypedErrorDirectly() const;
  void emitToUnmappedExplosionWithDirectTypedError(SILType resultType,
                                                   toolchain::Value *result,
                                                   Explosion &out);

  CallEmission(IRGenFunction &IGF, toolchain::Value *selfValue, Callee &&callee)
      : IGF(IGF), selfValue(selfValue), CurCallee(std::move(callee)) {}

public:
  CallEmission(const CallEmission &other) = delete;
  CallEmission(CallEmission &&other);
  CallEmission &operator=(const CallEmission &other) = delete;
  virtual ~CallEmission();

  const Callee &getCallee() const { return CurCallee; }

  SubstitutionMap getSubstitutions() const {
    return CurCallee.getSubstitutions();
  }

  void useProfilingThunk() {
    UseProfilingThunk = true;
  }

  std::optional<Explosion> &getTypedErrorExplosion() {
    return typedErrorExplosion;
  }

  virtual void begin();
  virtual void end();
  virtual SILType getParameterType(unsigned index) = 0;
  /// Set the arguments to the function from an explosion.
  virtual void setArgs(Explosion &arg, bool isOutlined,
                       WitnessMetadata *witnessMetadata);
  virtual Address getCalleeErrorSlot(SILType errorType, bool isCalleeAsync) = 0;

  void addFnAttribute(toolchain::Attribute::AttrKind Attr);

  void setIndirectReturnAddress(Address addr) { indirectReturnAddress = addr; }

  void addParamAttribute(unsigned ParamIndex, toolchain::Attribute::AttrKind Attr);

  void emitToMemory(Address addr, const LoadableTypeInfo &substResultTI,
                    bool isOutlined);
  void emitToExplosion(Explosion &out, bool isOutlined);

  toolchain::CallBase *emitCoroutineAsOrdinaryFunction() {
    assert(IsCoroutine);
    IsCoroutine = false;

    return emitCallSite();
  }

  TemporarySet claimTemporaries() {
    // Move the actual temporary set out.
    auto result = std::move(Temporaries);

    // Flag that we've cleared the set.
    Temporaries.clear();

    return result;
  }

  virtual toolchain::Value *getResumeFunctionPointer() = 0;
  virtual toolchain::Value *getAsyncContext() = 0;

  virtual StackAddress getCoroStaticFrame() = 0;
  virtual toolchain::Value *getCoroAllocator() = 0;

  void setIndirectTypedErrorResultSlot(toolchain::Value *addr) {
    Args[IndirectTypedErrorArgIdx] = addr;
  }

  void setIndirectTypedErrorResultSlotArgsIndex(unsigned idx) {
    assert(idx != 0);
    IndirectTypedErrorArgIdx = idx;
  }
};

std::unique_ptr<CallEmission>
getCallEmission(IRGenFunction &IGF, toolchain::Value *selfValue, Callee &&callee);

} // end namespace irgen
} // end namespace language

#endif
