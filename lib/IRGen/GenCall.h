//===--- GenCall.h - IR generation for calls and prologues ------*- C++ -*-===//
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
//  This file provides the private interface to the function call
//  and prologue emission support code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_GENCALL_H
#define LANGUAGE_IRGEN_GENCALL_H

#include <stdint.h>

#include "language/AST/Types.h"
#include "language/Basic/Toolchain.h"
#include "language/SIL/ApplySite.h"
#include "toolchain/IR/CallingConv.h"

#include "Callee.h"
#include "GenHeap.h"
#include "IRGenModule.h"

namespace toolchain {
  class AttributeList;
  class Constant;
  class Twine;
  class Type;
  class Value;
}

namespace clang {
  template <class> class CanQual;
  class Type;
}

namespace language {
enum class CoroAllocatorKind : uint8_t;
namespace irgen {
  class Address;
  class Alignment;
  class Callee;
  class CalleeInfo;
  class Explosion;
  class ExplosionSchema;
  class ForeignFunctionInfo;
  class IRGenFunction;
  class IRGenModule;
  class LoadableTypeInfo;
  class NativeCCEntryPointArgumentEmission;
  class Size;
  class TypeInfo;

  enum class TranslationDirection : bool {
    ToForeign,
    ToNative
  };
  inline TranslationDirection reverse(TranslationDirection direction) {
    return TranslationDirection(!bool(direction));
  }

  // struct CodiraContext {
  //   CodiraContext * __ptrauth(...) callerContext;
  //   CodiraPartialFunction * __ptrauth(...) returnToCaller;
  //   CodiraActor * __ptrauth(...) callerActor;
  //   CodiraPartialFunction * __ptrauth(...) yieldToCaller?;
  // };
  struct AsyncContextLayout : StructLayout {
    struct ArgumentInfo {
      SILType type;
      ParameterConvention convention;
    };
    struct TrailingWitnessInfo {};

  private:
    enum class FixedIndex : unsigned {
      Parent = 0,
      ResumeParent = 1,
      Flags = 2,
    };
    enum class FixedCount : unsigned {
      Parent = 1,
      ResumeParent = 1,
    };
    CanSILFunctionType originalType;
    CanSILFunctionType substitutedType;
    SubstitutionMap substitutionMap;
  
    unsigned getParentIndex() { return (unsigned)FixedIndex::Parent; }
    unsigned getResumeParentIndex() {
      return (unsigned)FixedIndex::ResumeParent;
    }
    unsigned getFlagsIndex() { return (unsigned)FixedIndex::Flags; }

  public:
    ElementLayout getParentLayout() { return getElement(getParentIndex()); }
    ElementLayout getResumeParentLayout() {
      return getElement(getResumeParentIndex());
    }
    ElementLayout getFlagsLayout() { return getElement(getFlagsIndex()); }

    AsyncContextLayout(
        IRGenModule &IGM, LayoutStrategy strategy, ArrayRef<SILType> fieldTypes,
        ArrayRef<const TypeInfo *> fieldTypeInfos,
        CanSILFunctionType originalType, CanSILFunctionType substitutedType,
        SubstitutionMap substitutionMap);
  };

  AsyncContextLayout getAsyncContextLayout(IRGenModule &IGM,
                                           SILFunction *function);

  AsyncContextLayout getAsyncContextLayout(IRGenModule &IGM,
                                           CanSILFunctionType originalType,
                                           CanSILFunctionType substitutedType,
                                           SubstitutionMap substitutionMap);

  struct CombinedResultAndErrorType {
    toolchain::Type *combinedTy;
    toolchain::SmallVector<unsigned, 2> errorValueMapping;
  };
  CombinedResultAndErrorType
  combineResultAndTypedErrorType(const IRGenModule &IGM,
                                 const NativeConventionSchema &resultSchema,
                                 const NativeConventionSchema &errorSchema);

  /// Given an async function, get the pointer to the function to be called and
  /// the size of the context to be allocated.
  ///
  /// \param values Whether any code should be emitted to retrieve the function
  ///               pointer and the size, respectively.  If false is passed, no
  ///               code will be emitted to generate that value and null will
  ///               be returned for it.
  ///
  /// \return {function, size}
  std::pair<toolchain::Value *, toolchain::Value *>
  getAsyncFunctionAndSize(IRGenFunction &IGF, FunctionPointer functionPointer,
                          std::pair<bool, bool> values = {true, true});
  std::pair<toolchain::Value *, toolchain::Value *>
  getCoroFunctionAndSize(IRGenFunction &IGF, FunctionPointer functionPointer,
                         std::pair<bool, bool> values = {true, true});
  toolchain::CallingConv::ID
  expandCallingConv(IRGenModule &IGM, SILFunctionTypeRepresentation convention,
                    bool isAsync, bool isCalleeAllocatedCoro);

  Signature emitCastOfFunctionPointer(IRGenFunction &IGF, toolchain::Value *&fnPtr,
                                      CanSILFunctionType fnType,
                                      bool forAsyncReturn = false);

  /// Does the given function have a self parameter that should be given
  /// the special treatment for self parameters?
  bool hasSelfContextParameter(CanSILFunctionType fnType);

  /// Add function attributes to an attribute set for a byval argument.
  void addByvalArgumentAttributes(IRGenModule &IGM,
                                  toolchain::AttributeList &attrs,
                                  unsigned argIndex,
                                  Alignment align,
                                  toolchain::Type *storageType);

  /// Can a series of values be simply pairwise coerced to (or from) an
  /// explosion schema, or do they need to traffic through memory?
  bool canCoerceToSchema(IRGenModule &IGM,
                         ArrayRef<toolchain::Type*> types,
                         const ExplosionSchema &schema);

  void emitForeignParameter(IRGenFunction &IGF, Explosion &params,
                            ForeignFunctionInfo foreignInfo,
                            unsigned foreignParamIndex, SILType paramTy,
                            const LoadableTypeInfo &paramTI,
                            Explosion &paramExplosion, bool isOutlined);

  void emitClangExpandedParameter(IRGenFunction &IGF,
                                  Explosion &in, Explosion &out,
                                  clang::CanQual<clang::Type> clangType,
                                  SILType languageType,
                                  const LoadableTypeInfo &languageTI);

  bool addNativeArgument(IRGenFunction &IGF,
                         Explosion &in,
                         CanSILFunctionType fnTy,
                         SILParameterInfo origParamInfo, Explosion &args,
                         bool isOutlined);

  /// Allocate a stack buffer of the appropriate size to bitwise-coerce a value
  /// between two LLVM types.
  std::pair<Address, Size>
  allocateForCoercion(IRGenFunction &IGF,
                      toolchain::Type *fromTy,
                      toolchain::Type *toTy,
                      const toolchain::Twine &basename);

  void extractScalarResults(IRGenFunction &IGF, toolchain::Type *bodyType,
                            toolchain::Value *call, Explosion &out);

  Callee getBlockPointerCallee(IRGenFunction &IGF, toolchain::Value *blockPtr,
                               CalleeInfo &&info);

  Callee getCFunctionPointerCallee(IRGenFunction &IGF, toolchain::Value *fnPtr,
                                   CalleeInfo &&info);

  Callee getCodiraFunctionPointerCallee(IRGenFunction &IGF,
                                       toolchain::Value *fnPtr,
                                       toolchain::Value *contextPtr,
                                       CalleeInfo &&info,
                                       bool castOpaqueToRefcountedContext,
                                       bool isClosure);

  Address emitAllocYieldOnceCoroutineBuffer(IRGenFunction &IGF);
  void emitDeallocYieldOnceCoroutineBuffer(IRGenFunction &IGF, Address buffer);
  void
  emitYieldOnceCoroutineEntry(IRGenFunction &IGF,
                              CanSILFunctionType coroutineType,
                              NativeCCEntryPointArgumentEmission &emission);

  toolchain::Value *
  emitYieldOnce2CoroutineAllocator(IRGenFunction &IGF,
                                   std::optional<CoroAllocatorKind> kind);
  StackAddress emitAllocYieldOnce2CoroutineFrame(IRGenFunction &IGF,
                                                 toolchain::Value *size);
  void emitDeallocYieldOnce2CoroutineFrame(IRGenFunction &IGF,
                                           StackAddress allocation);
  void
  emitYieldOnce2CoroutineEntry(IRGenFunction &IGF, LinkEntity coroFunction,
                               CanSILFunctionType coroutineType,
                               NativeCCEntryPointArgumentEmission &emission);
  void emitYieldOnce2CoroutineEntry(IRGenFunction &IGF,
                                    CanSILFunctionType fnType,
                                    toolchain::Value *buffer, toolchain::Value *allocator,
                                    toolchain::GlobalVariable *cfp);

  Address emitAllocYieldManyCoroutineBuffer(IRGenFunction &IGF);
  void emitDeallocYieldManyCoroutineBuffer(IRGenFunction &IGF, Address buffer);
  void
  emitYieldManyCoroutineEntry(IRGenFunction &IGF,
                              CanSILFunctionType coroutineType,
                              NativeCCEntryPointArgumentEmission &emission);

  /// Allocate task local storage for the provided dynamic size.
  Address emitAllocAsyncContext(IRGenFunction &IGF, toolchain::Value *sizeValue);
  void emitDeallocAsyncContext(IRGenFunction &IGF, Address context);
  Address emitStaticAllocAsyncContext(IRGenFunction &IGF, Size size);
  void emitStaticDeallocAsyncContext(IRGenFunction &IGF, Address context,
                                     Size size);

  void emitAsyncFunctionEntry(IRGenFunction &IGF,
                              const AsyncContextLayout &layout,
                              LinkEntity asyncFunction,
                              unsigned asyncContextIndex);

  StackAddress emitAllocCoroStaticFrame(IRGenFunction &IGF, toolchain::Value *size);
  void emitDeallocCoroStaticFrame(IRGenFunction &IGF, StackAddress frame);

  /// Yield the given values from the current continuation.
  ///
  /// \return an i1 indicating whether the caller wants to unwind this
  ///   coroutine instead of resuming it normally
  toolchain::Value *emitYield(IRGenFunction &IGF,
                         CanSILFunctionType coroutineType,
                         Explosion &yieldedValues);

  enum class AsyncFunctionArgumentIndex : unsigned {
    Context = 0,
  };

  void emitAsyncReturn(
      IRGenFunction &IGF, AsyncContextLayout &layout, CanSILFunctionType fnType,
      std::optional<ArrayRef<toolchain::Value *>> nativeResultArgs = std::nullopt);

  void emitAsyncReturn(IRGenFunction &IGF, AsyncContextLayout &layout,
                       SILType funcResultTypeInContext,
                       CanSILFunctionType fnType, Explosion &result,
                       Explosion &error);
  void emitYieldOnceCoroutineResult(IRGenFunction &IGF, Explosion &result,
                                    SILType funcResultType, SILType returnResultType);

  Address emitAutoDiffCreateLinearMapContextWithType(
      IRGenFunction &IGF, toolchain::Value *topLevelSubcontextMetatype);

  Address emitAutoDiffProjectTopLevelSubcontext(
      IRGenFunction &IGF, Address context);

  Address
  emitAutoDiffAllocateSubcontextWithType(IRGenFunction &IGF, Address context,
                                         toolchain::Value *subcontextMetatype);

  FunctionPointer getFunctionPointerForDispatchCall(IRGenModule &IGM,
                                                    const FunctionPointer &fn);
  void forwardAsyncCallResult(IRGenFunction &IGF, CanSILFunctionType fnType,
                              AsyncContextLayout &layout, toolchain::CallInst *call);

  /// Converts a value for direct error return.
  toolchain::Value *convertForDirectError(IRGenFunction &IGF, toolchain::Value *value,
                                     toolchain::Type *toTy, bool forExtraction);

  void buildDirectError(IRGenFunction &IGF,
                        const CombinedResultAndErrorType &combined,
                        const NativeConventionSchema &errorSchema,
                        SILType silErrorTy, Explosion &errorResult,
                        bool forAsync, Explosion &out);
} // end namespace irgen
} // end namespace language

#endif
