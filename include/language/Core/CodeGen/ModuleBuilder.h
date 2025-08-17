//===--- CodeGen/ModuleBuilder.h - Build LLVM from ASTs ---------*- C++ -*-===//
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
//  This file defines the ModuleBuilder interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_CODEGEN_MODULEBUILDER_H
#define LANGUAGE_CORE_CODEGEN_MODULEBUILDER_H

#include "language/Core/AST/ASTConsumer.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"

namespace toolchain {
  class Constant;
  class LLVMContext;
  class Module;
  class StringRef;

  namespace vfs {
  class FileSystem;
  }
}

// Prefix of the name of the artificial inline frame.
inline constexpr toolchain::StringRef ClangTrapPrefix = "__clang_trap_msg";

namespace language::Core {
  class CodeGenOptions;
  class CoverageSourceInfo;
  class Decl;
  class DiagnosticsEngine;
  class GlobalDecl;
  class HeaderSearchOptions;
  class LangOptions;
  class PreprocessorOptions;

namespace CodeGen {
  class CodeGenModule;
  class CGDebugInfo;
}

/// The primary public interface to the Clang code generator.
///
/// This is not really an abstract interface.
class CodeGenerator : public ASTConsumer {
  virtual void anchor();

public:
  /// Return an opaque reference to the CodeGenModule object, which can
  /// be used in various secondary APIs.  It is valid as long as the
  /// CodeGenerator exists.
  CodeGen::CodeGenModule &CGM();

  /// Return the module that this code generator is building into.
  ///
  /// This may return null after HandleTranslationUnit is called;
  /// this signifies that there was an error generating code.  A
  /// diagnostic will have been generated in this case, and the module
  /// will be deleted.
  ///
  /// It will also return null if the module is released.
  toolchain::Module *GetModule();

  /// Release ownership of the module to the caller.
  ///
  /// It is illegal to call methods other than GetModule on the
  /// CodeGenerator after releasing its module.
  toolchain::Module *ReleaseModule();

  /// Return debug info code generator.
  CodeGen::CGDebugInfo *getCGDebugInfo();

  /// Given a mangled name, return a declaration which mangles that way
  /// which has been added to this code generator via a Handle method.
  ///
  /// This may return null if there was no matching declaration.
  const Decl *GetDeclForMangledName(toolchain::StringRef MangledName);

  /// Given a global declaration, return a mangled name for this declaration
  /// which has been added to this code generator via a Handle method.
  toolchain::StringRef GetMangledName(GlobalDecl GD);

  /// Return the LLVM address of the given global entity.
  ///
  /// \param isForDefinition If true, the caller intends to define the
  ///   entity; the object returned will be an toolchain::GlobalValue of
  ///   some sort.  If false, the caller just intends to use the entity;
  ///   the object returned may be any sort of constant value, and the
  ///   code generator will schedule the entity for emission if a
  ///   definition has been registered with this code generator.
  toolchain::Constant *GetAddrOfGlobal(GlobalDecl decl, bool isForDefinition);

  /// Create a new \c toolchain::Module after calling HandleTranslationUnit. This
  /// enable codegen in interactive processing environments.
  toolchain::Module* StartModule(toolchain::StringRef ModuleName, toolchain::LLVMContext &C);
};

/// CreateLLVMCodeGen - Create a CodeGenerator instance.
/// It is the responsibility of the caller to call delete on
/// the allocated CodeGenerator instance.
CodeGenerator *CreateLLVMCodeGen(DiagnosticsEngine &Diags,
                                 toolchain::StringRef ModuleName,
                                 IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS,
                                 const HeaderSearchOptions &HeaderSearchOpts,
                                 const PreprocessorOptions &PreprocessorOpts,
                                 const CodeGenOptions &CGO,
                                 toolchain::LLVMContext &C,
                                 CoverageSourceInfo *CoverageInfo = nullptr);

} // end namespace language::Core

#endif
