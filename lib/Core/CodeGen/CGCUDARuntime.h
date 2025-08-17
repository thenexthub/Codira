//===----- CGCUDARuntime.h - Interface to CUDA Runtimes ---------*- C++ -*-===//
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
// This provides an abstract class for CUDA code generation.  Concrete
// subclasses of this implement code generation for specific CUDA
// runtime libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGCUDARUNTIME_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGCUDARUNTIME_H

#include "language/Core/AST/GlobalDecl.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Frontend/Offloading/Utility.h"
#include "toolchain/IR/GlobalValue.h"

namespace toolchain {
class CallBase;
class Function;
class GlobalVariable;
}

namespace language::Core {

class CUDAKernelCallExpr;
class NamedDecl;
class VarDecl;

namespace CodeGen {

class CodeGenFunction;
class CodeGenModule;
class FunctionArgList;
class ReturnValueSlot;
class RValue;

class CGCUDARuntime {
protected:
  CodeGenModule &CGM;

public:
  // Global variable properties that must be passed to CUDA runtime.
  class DeviceVarFlags {
  public:
    enum DeviceVarKind {
      Variable, // Variable
      Surface,  // Builtin surface
      Texture,  // Builtin texture
    };

  private:
    LLVM_PREFERRED_TYPE(DeviceVarKind)
    unsigned Kind : 2;
    LLVM_PREFERRED_TYPE(bool)
    unsigned Extern : 1;
    LLVM_PREFERRED_TYPE(bool)
    unsigned Constant : 1;   // Constant variable.
    LLVM_PREFERRED_TYPE(bool)
    unsigned Managed : 1;    // Managed variable.
    LLVM_PREFERRED_TYPE(bool)
    unsigned Normalized : 1; // Normalized texture.
    int SurfTexType;         // Type of surface/texutre.

  public:
    DeviceVarFlags(DeviceVarKind K, bool E, bool C, bool M, bool N, int T)
        : Kind(K), Extern(E), Constant(C), Managed(M), Normalized(N),
          SurfTexType(T) {}

    DeviceVarKind getKind() const { return static_cast<DeviceVarKind>(Kind); }
    bool isExtern() const { return Extern; }
    bool isConstant() const { return Constant; }
    bool isManaged() const { return Managed; }
    bool isNormalized() const { return Normalized; }
    int getSurfTexType() const { return SurfTexType; }
  };

  CGCUDARuntime(CodeGenModule &CGM) : CGM(CGM) {}
  virtual ~CGCUDARuntime();

  virtual RValue
  EmitCUDAKernelCallExpr(CodeGenFunction &CGF, const CUDAKernelCallExpr *E,
                         ReturnValueSlot ReturnValue,
                         toolchain::CallBase **CallOrInvoke = nullptr);

  /// Emits a kernel launch stub.
  virtual void emitDeviceStub(CodeGenFunction &CGF, FunctionArgList &Args) = 0;

  /// Check whether a variable is a device variable and register it if true.
  virtual void handleVarRegistration(const VarDecl *VD,
                                     toolchain::GlobalVariable &Var) = 0;

  /// Finalize generated LLVM module. Returns a module constructor function
  /// to be added or a null pointer.
  virtual toolchain::Function *finalizeModule() = 0;

  /// Returns function or variable name on device side even if the current
  /// compilation is for host.
  virtual std::string getDeviceSideName(const NamedDecl *ND) = 0;

  /// Get kernel handle by stub function.
  virtual toolchain::GlobalValue *getKernelHandle(toolchain::Function *Stub,
                                             GlobalDecl GD) = 0;

  /// Get kernel stub by kernel handle.
  virtual toolchain::Function *getKernelStub(toolchain::GlobalValue *Handle) = 0;

  /// Adjust linkage of shadow variables in host compilation.
  virtual void
  internalizeDeviceSideVar(const VarDecl *D,
                           toolchain::GlobalValue::LinkageTypes &Linkage) = 0;
};

/// Creates an instance of a CUDA runtime class.
CGCUDARuntime *CreateNVCUDARuntime(CodeGenModule &CGM);

}
}

#endif
