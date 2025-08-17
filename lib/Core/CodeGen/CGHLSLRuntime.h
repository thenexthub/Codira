//===----- CGHLSLRuntime.h - Interface to HLSL Runtimes -----*- C++ -*-===//
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
//
// This provides an abstract class for HLSL code generation.  Concrete
// subclasses of this implement code generation for specific HLSL
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGHLSLRUNTIME_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGHLSLRUNTIME_H

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/IR/IRBuilder.h"
#include "toolchain/IR/Intrinsics.h"
#include "toolchain/IR/IntrinsicsDirectX.h"
#include "toolchain/IR/IntrinsicsSPIRV.h"

#include "language/Core/Basic/Builtins.h"
#include "language/Core/Basic/HLSLRuntime.h"

#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Frontend/HLSL/HLSLResource.h"

#include <optional>
#include <vector>

// A function generator macro for picking the right intrinsic
// for the target backend
#define GENERATE_HLSL_INTRINSIC_FUNCTION(FunctionName, IntrinsicPostfix)       \
  toolchain::Intrinsic::ID get##FunctionName##Intrinsic() {                         \
    toolchain::Triple::ArchType Arch = getArch();                                   \
    switch (Arch) {                                                            \
    case toolchain::Triple::dxil:                                                   \
      return toolchain::Intrinsic::dx_##IntrinsicPostfix;                           \
    case toolchain::Triple::spirv:                                                  \
      return toolchain::Intrinsic::spv_##IntrinsicPostfix;                          \
    default:                                                                   \
      toolchain_unreachable("Intrinsic " #IntrinsicPostfix                          \
                       " not supported by target architecture");               \
    }                                                                          \
  }

using ResourceClass = toolchain::dxil::ResourceClass;

namespace toolchain {
class GlobalVariable;
class Function;
class StructType;
class Metadata;
} // namespace toolchain

namespace language::Core {
class NamedDecl;
class VarDecl;
class ParmVarDecl;
class InitListExpr;
class HLSLBufferDecl;
class HLSLVkBindingAttr;
class HLSLResourceBindingAttr;
class Type;
class RecordType;
class DeclContext;
class HLSLPackOffsetAttr;

class FunctionDecl;

namespace CodeGen {

class CodeGenModule;
class CodeGenFunction;

class CGHLSLRuntime {
public:
  //===----------------------------------------------------------------------===//
  // Start of reserved area for HLSL intrinsic getters.
  //===----------------------------------------------------------------------===//

  GENERATE_HLSL_INTRINSIC_FUNCTION(All, all)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Any, any)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Cross, cross)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Degrees, degrees)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Frac, frac)
  GENERATE_HLSL_INTRINSIC_FUNCTION(FlattenedThreadIdInGroup,
                                   flattened_thread_id_in_group)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Lerp, lerp)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Normalize, normalize)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Rsqrt, rsqrt)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Saturate, saturate)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Sign, sign)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Step, step)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Radians, radians)
  GENERATE_HLSL_INTRINSIC_FUNCTION(ThreadId, thread_id)
  GENERATE_HLSL_INTRINSIC_FUNCTION(GroupThreadId, thread_id_in_group)
  GENERATE_HLSL_INTRINSIC_FUNCTION(GroupId, group_id)
  GENERATE_HLSL_INTRINSIC_FUNCTION(FDot, fdot)
  GENERATE_HLSL_INTRINSIC_FUNCTION(SDot, sdot)
  GENERATE_HLSL_INTRINSIC_FUNCTION(UDot, udot)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Dot4AddI8Packed, dot4add_i8packed)
  GENERATE_HLSL_INTRINSIC_FUNCTION(Dot4AddU8Packed, dot4add_u8packed)
  GENERATE_HLSL_INTRINSIC_FUNCTION(WaveActiveAllTrue, wave_all)
  GENERATE_HLSL_INTRINSIC_FUNCTION(WaveActiveAnyTrue, wave_any)
  GENERATE_HLSL_INTRINSIC_FUNCTION(WaveActiveCountBits, wave_active_countbits)
  GENERATE_HLSL_INTRINSIC_FUNCTION(WaveIsFirstLane, wave_is_first_lane)
  GENERATE_HLSL_INTRINSIC_FUNCTION(WaveGetLaneCount, wave_get_lane_count)
  GENERATE_HLSL_INTRINSIC_FUNCTION(WaveReadLaneAt, wave_readlane)
  GENERATE_HLSL_INTRINSIC_FUNCTION(FirstBitUHigh, firstbituhigh)
  GENERATE_HLSL_INTRINSIC_FUNCTION(FirstBitSHigh, firstbitshigh)
  GENERATE_HLSL_INTRINSIC_FUNCTION(FirstBitLow, firstbitlow)
  GENERATE_HLSL_INTRINSIC_FUNCTION(NClamp, nclamp)
  GENERATE_HLSL_INTRINSIC_FUNCTION(SClamp, sclamp)
  GENERATE_HLSL_INTRINSIC_FUNCTION(UClamp, uclamp)

  GENERATE_HLSL_INTRINSIC_FUNCTION(CreateResourceGetPointer,
                                   resource_getpointer)
  GENERATE_HLSL_INTRINSIC_FUNCTION(CreateHandleFromBinding,
                                   resource_handlefrombinding)
  GENERATE_HLSL_INTRINSIC_FUNCTION(CreateHandleFromImplicitBinding,
                                   resource_handlefromimplicitbinding)
  GENERATE_HLSL_INTRINSIC_FUNCTION(BufferUpdateCounter, resource_updatecounter)
  GENERATE_HLSL_INTRINSIC_FUNCTION(GroupMemoryBarrierWithGroupSync,
                                   group_memory_barrier_with_group_sync)

  //===----------------------------------------------------------------------===//
  // End of reserved area for HLSL intrinsic getters.
  //===----------------------------------------------------------------------===//

protected:
  CodeGenModule &CGM;

  toolchain::Value *emitInputSemantic(toolchain::IRBuilder<> &B, const ParmVarDecl &D,
                                 toolchain::Type *Ty);

public:
  CGHLSLRuntime(CodeGenModule &CGM) : CGM(CGM) {}
  virtual ~CGHLSLRuntime() {}

  toolchain::Type *
  convertHLSLSpecificType(const Type *T,
                          SmallVector<int32_t> *Packoffsets = nullptr);

  void generateGlobalCtorDtorCalls();

  void addBuffer(const HLSLBufferDecl *D);
  void finishCodeGen();

  void setHLSLEntryAttributes(const FunctionDecl *FD, toolchain::Function *Fn);

  void emitEntryFunction(const FunctionDecl *FD, toolchain::Function *Fn);
  void setHLSLFunctionAttributes(const FunctionDecl *FD, toolchain::Function *Fn);
  void handleGlobalVarDefinition(const VarDecl *VD, toolchain::GlobalVariable *Var);

  toolchain::Instruction *getConvergenceToken(toolchain::BasicBlock &BB);

  toolchain::TargetExtType *
  getHLSLBufferLayoutType(const RecordType *LayoutStructTy);
  void addHLSLBufferLayoutType(const RecordType *LayoutStructTy,
                               toolchain::TargetExtType *LayoutTy);
  void emitInitListOpaqueValues(CodeGenFunction &CGF, InitListExpr *E);

private:
  void emitBufferGlobalsAndMetadata(const HLSLBufferDecl *BufDecl,
                                    toolchain::GlobalVariable *BufGV);
  void initializeBufferFromBinding(const HLSLBufferDecl *BufDecl,
                                   toolchain::GlobalVariable *GV,
                                   HLSLVkBindingAttr *VkBinding);
  void initializeBufferFromBinding(const HLSLBufferDecl *BufDecl,
                                   toolchain::GlobalVariable *GV,
                                   HLSLResourceBindingAttr *RBA);
  toolchain::Triple::ArchType getArch();

  toolchain::DenseMap<const language::Core::RecordType *, toolchain::TargetExtType *> LayoutTypes;
};

} // namespace CodeGen
} // namespace language::Core

#endif
