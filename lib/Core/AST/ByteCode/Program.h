//===--- Program.h - Bytecode for the constexpr VM --------------*- C++ -*-===//
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
// Defines a program which organises and links multiple bytecode functions.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_INTERP_PROGRAM_H
#define LANGUAGE_CORE_AST_INTERP_PROGRAM_H

#include "Function.h"
#include "Pointer.h"
#include "PrimType.h"
#include "Record.h"
#include "Source.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/Allocator.h"
#include <vector>

namespace language::Core {
class RecordDecl;
class Expr;
class FunctionDecl;
class StringLiteral;
class VarDecl;

namespace interp {
class Context;

/// The program contains and links the bytecode for all functions.
class Program final {
public:
  Program(Context &Ctx) : Ctx(Ctx) {}

  ~Program() {
    // Manually destroy all the blocks. They are almost all harmless,
    // but primitive arrays might have an InitMap* heap allocated and
    // that needs to be freed.
    for (Global *G : Globals)
      if (Block *B = G->block(); B->isInitialized())
        B->invokeDtor();

    // Records might actually allocate memory themselves, but they
    // are allocated using a BumpPtrAllocator. Call their desctructors
    // here manually so they are properly freeing their resources.
    for (auto RecordPair : Records) {
      if (Record *R = RecordPair.second)
        R->~Record();
    }
  }

  /// Marshals a native pointer to an ID for embedding in bytecode.
  unsigned getOrCreateNativePointer(const void *Ptr);

  /// Returns the value of a marshalled native pointer.
  const void *getNativePointer(unsigned Idx);

  /// Emits a string literal among global data.
  unsigned createGlobalString(const StringLiteral *S,
                              const Expr *Base = nullptr);

  /// Returns a pointer to a global.
  Pointer getPtrGlobal(unsigned Idx) const;

  /// Returns the value of a global.
  Block *getGlobal(unsigned Idx) {
    assert(Idx < Globals.size());
    return Globals[Idx]->block();
  }

  /// Finds a global's index.
  std::optional<unsigned> getGlobal(const ValueDecl *VD);
  std::optional<unsigned> getGlobal(const Expr *E);

  /// Returns or creates a global an creates an index to it.
  std::optional<unsigned> getOrCreateGlobal(const ValueDecl *VD,
                                            const Expr *Init = nullptr);

  /// Returns or creates a dummy value for unknown declarations.
  unsigned getOrCreateDummy(const DeclTy &D);

  /// Creates a global and returns its index.
  std::optional<unsigned> createGlobal(const ValueDecl *VD, const Expr *Init);

  /// Creates a global from a lifetime-extended temporary.
  std::optional<unsigned> createGlobal(const Expr *E);

  /// Creates a new function from a code range.
  template <typename... Ts>
  Function *createFunction(const FunctionDecl *Def, Ts &&...Args) {
    Def = Def->getCanonicalDecl();
    auto *Func = new Function(*this, Def, std::forward<Ts>(Args)...);
    Funcs.insert({Def, std::unique_ptr<Function>(Func)});
    return Func;
  }
  /// Creates an anonymous function.
  template <typename... Ts> Function *createFunction(Ts &&...Args) {
    auto *Func = new Function(*this, std::forward<Ts>(Args)...);
    AnonFuncs.emplace_back(Func);
    return Func;
  }

  /// Returns a function.
  Function *getFunction(const FunctionDecl *F);

  /// Returns a record or creates one if it does not exist.
  Record *getOrCreateRecord(const RecordDecl *RD);

  /// Creates a descriptor for a primitive type.
  Descriptor *createDescriptor(const DeclTy &D, PrimType T,
                               const Type *SourceTy = nullptr,
                               Descriptor::MetadataSize MDSize = std::nullopt,
                               bool IsConst = false, bool IsTemporary = false,
                               bool IsMutable = false,
                               bool IsVolatile = false) {
    return allocateDescriptor(D, SourceTy, T, MDSize, IsConst, IsTemporary,
                              IsMutable, IsVolatile);
  }

  /// Creates a descriptor for a composite type.
  Descriptor *createDescriptor(const DeclTy &D, const Type *Ty,
                               Descriptor::MetadataSize MDSize = std::nullopt,
                               bool IsConst = false, bool IsTemporary = false,
                               bool IsMutable = false, bool IsVolatile = false,
                               const Expr *Init = nullptr);

  void *Allocate(size_t Size, unsigned Align = 8) const {
    return Allocator.Allocate(Size, Align);
  }
  template <typename T> T *Allocate(size_t Num = 1) const {
    return static_cast<T *>(Allocate(Num * sizeof(T), alignof(T)));
  }
  void Deallocate(void *Ptr) const {}

  /// Context to manage declaration lifetimes.
  class DeclScope {
  public:
    DeclScope(Program &P) : P(P), PrevDecl(P.CurrentDeclaration) {
      ++P.LastDeclaration;
      P.CurrentDeclaration = P.LastDeclaration;
    }
    ~DeclScope() { P.CurrentDeclaration = PrevDecl; }

  private:
    Program &P;
    unsigned PrevDecl;
  };

  /// Returns the current declaration ID.
  std::optional<unsigned> getCurrentDecl() const {
    if (CurrentDeclaration == NoDeclaration)
      return std::nullopt;
    return CurrentDeclaration;
  }

private:
  friend class DeclScope;

  std::optional<unsigned> createGlobal(const DeclTy &D, QualType Ty,
                                       bool IsStatic, bool IsExtern,
                                       bool IsWeak, const Expr *Init = nullptr);

  /// Reference to the VM context.
  Context &Ctx;
  /// Mapping from decls to cached bytecode functions.
  toolchain::DenseMap<const FunctionDecl *, std::unique_ptr<Function>> Funcs;
  /// List of anonymous functions.
  std::vector<std::unique_ptr<Function>> AnonFuncs;

  /// Function relocation locations.
  toolchain::DenseMap<const FunctionDecl *, std::vector<unsigned>> Relocs;

  /// Native pointers referenced by bytecode.
  std::vector<const void *> NativePointers;
  /// Cached native pointer indices.
  toolchain::DenseMap<const void *, unsigned> NativePointerIndices;

  /// Custom allocator for global storage.
  using PoolAllocTy = toolchain::BumpPtrAllocator;

  /// Descriptor + storage for a global object.
  ///
  /// Global objects never go out of scope, thus they do not track pointers.
  class Global {
  public:
    /// Create a global descriptor for string literals.
    template <typename... Tys>
    Global(Tys... Args) : B(std::forward<Tys>(Args)...) {}

    /// Allocates the global in the pool, reserving storate for data.
    void *operator new(size_t Meta, PoolAllocTy &Alloc, size_t Data) {
      return Alloc.Allocate(Meta + Data, alignof(void *));
    }

    /// Return a pointer to the data.
    std::byte *data() { return B.data(); }
    /// Return a pointer to the block.
    Block *block() { return &B; }
    const Block *block() const { return &B; }

  private:
    /// Required metadata - does not actually track pointers.
    Block B;
  };

  /// Allocator for globals.
  mutable PoolAllocTy Allocator;

  /// Global objects.
  std::vector<Global *> Globals;
  /// Cached global indices.
  toolchain::DenseMap<const void *, unsigned> GlobalIndices;

  /// Mapping from decls to record metadata.
  toolchain::DenseMap<const RecordDecl *, Record *> Records;

  /// Dummy parameter to generate pointers from.
  toolchain::DenseMap<const void *, unsigned> DummyVariables;

  /// Creates a new descriptor.
  template <typename... Ts> Descriptor *allocateDescriptor(Ts &&...Args) {
    return new (Allocator) Descriptor(std::forward<Ts>(Args)...);
  }

  /// No declaration ID.
  static constexpr unsigned NoDeclaration = ~0u;
  /// Last declaration ID.
  unsigned LastDeclaration = 0;
  /// Current declaration ID.
  unsigned CurrentDeclaration = NoDeclaration;

public:
  /// Dumps the disassembled bytecode to \c toolchain::errs().
  void dump() const;
  void dump(toolchain::raw_ostream &OS) const;
};

} // namespace interp
} // namespace language::Core

inline void *operator new(size_t Bytes, const language::Core::interp::Program &C,
                          size_t Alignment = 8) {
  return C.Allocate(Bytes, Alignment);
}

inline void operator delete(void *Ptr, const language::Core::interp::Program &C,
                            size_t) {
  C.Deallocate(Ptr);
}
inline void *operator new[](size_t Bytes, const language::Core::interp::Program &C,
                            size_t Alignment = 8) {
  return C.Allocate(Bytes, Alignment);
}

#endif
