//===--- CGDebugInfo.h - DebugInfo for LLVM CodeGen -------------*- C++ -*-===//
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
// This is the source-level debug info generator for toolchain translation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CGDEBUGINFO_H
#define LANGUAGE_CORE_LIB_CODEGEN_CGDEBUGINFO_H

#include "CGBuilder.h"
#include "SanitizerHandler.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExternalASTSource.h"
#include "language/Core/AST/PrettyPrinter.h"
#include "language/Core/AST/Type.h"
#include "language/Core/AST/TypeOrdering.h"
#include "language/Core/Basic/ASTSourceDescriptor.h"
#include "language/Core/Basic/CodeGenOptions.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"
#include "toolchain/IR/DIBuilder.h"
#include "toolchain/IR/DebugInfo.h"
#include "toolchain/IR/ValueHandle.h"
#include "toolchain/Support/Allocator.h"
#include <map>
#include <optional>
#include <string>

namespace toolchain {
class MDNode;
}

namespace language::Core {
class ClassTemplateSpecializationDecl;
class GlobalDecl;
class Module;
class ModuleMap;
class ObjCInterfaceDecl;
class UsingDecl;
class VarDecl;
enum class DynamicInitKind : unsigned;

namespace CodeGen {
class CodeGenModule;
class CodeGenFunction;
class CGBlockInfo;

/// This class gathers all debug information during compilation and is
/// responsible for emitting to toolchain globals or pass directly to the
/// backend.
class CGDebugInfo {
  friend class ApplyDebugLocation;
  friend class SaveAndRestoreLocation;
  friend class ApplyAtomGroup;

  CodeGenModule &CGM;
  const toolchain::codegenoptions::DebugInfoKind DebugKind;
  bool DebugTypeExtRefs;
  toolchain::DIBuilder DBuilder;
  toolchain::DICompileUnit *TheCU = nullptr;
  ModuleMap *ClangModuleMap = nullptr;
  ASTSourceDescriptor PCHDescriptor;
  SourceLocation CurLoc;
  toolchain::MDNode *CurInlinedAt = nullptr;
  toolchain::DIType *VTablePtrType = nullptr;
  toolchain::DIType *ClassTy = nullptr;
  toolchain::DICompositeType *ObjTy = nullptr;
  toolchain::DIType *SelTy = nullptr;
#define IMAGE_TYPE(ImgType, Id, SingletonId, Access, Suffix)                   \
  toolchain::DIType *SingletonId = nullptr;
#include "language/Core/Basic/OpenCLImageTypes.def"
  toolchain::DIType *OCLSamplerDITy = nullptr;
  toolchain::DIType *OCLEventDITy = nullptr;
  toolchain::DIType *OCLClkEventDITy = nullptr;
  toolchain::DIType *OCLQueueDITy = nullptr;
  toolchain::DIType *OCLNDRangeDITy = nullptr;
  toolchain::DIType *OCLReserveIDDITy = nullptr;
#define EXT_OPAQUE_TYPE(ExtType, Id, Ext) \
  toolchain::DIType *Id##Ty = nullptr;
#include "language/Core/Basic/OpenCLExtensionTypes.def"
#define WASM_TYPE(Name, Id, SingletonId) toolchain::DIType *SingletonId = nullptr;
#include "language/Core/Basic/WebAssemblyReferenceTypes.def"
#define AMDGPU_TYPE(Name, Id, SingletonId, Width, Align)                       \
  toolchain::DIType *SingletonId = nullptr;
#include "language/Core/Basic/AMDGPUTypes.def"
#define HLSL_INTANGIBLE_TYPE(Name, Id, SingletonId)                            \
  toolchain::DIType *SingletonId = nullptr;
#include "language/Core/Basic/HLSLIntangibleTypes.def"

  /// Cache of previously constructed Types.
  toolchain::DenseMap<const void *, toolchain::TrackingMDRef> TypeCache;

  /// Cache that maps VLA types to size expressions for that type,
  /// represented by instantiated Metadata nodes.
  toolchain::SmallDenseMap<QualType, toolchain::Metadata *> SizeExprCache;

  /// Callbacks to use when printing names and types.
  class PrintingCallbacks final : public language::Core::PrintingCallbacks {
    const CGDebugInfo &Self;

  public:
    PrintingCallbacks(const CGDebugInfo &Self) : Self(Self) {}
    std::string remapPath(StringRef Path) const override {
      return Self.remapDIPath(Path);
    }
  };
  PrintingCallbacks PrintCB = {*this};

  struct ObjCInterfaceCacheEntry {
    const ObjCInterfaceType *Type;
    toolchain::DIType *Decl;
    toolchain::DIFile *Unit;
    ObjCInterfaceCacheEntry(const ObjCInterfaceType *Type, toolchain::DIType *Decl,
                            toolchain::DIFile *Unit)
        : Type(Type), Decl(Decl), Unit(Unit) {}
  };

  /// Cache of previously constructed interfaces which may change.
  toolchain::SmallVector<ObjCInterfaceCacheEntry, 32> ObjCInterfaceCache;

  /// Cache of forward declarations for methods belonging to the interface.
  /// The extra bit on the DISubprogram specifies whether a method is
  /// "objc_direct".
  toolchain::DenseMap<const ObjCInterfaceDecl *,
                 std::vector<toolchain::PointerIntPair<toolchain::DISubprogram *, 1>>>
      ObjCMethodCache;

  /// Cache of references to clang modules and precompiled headers.
  toolchain::DenseMap<const Module *, toolchain::TrackingMDRef> ModuleCache;

  /// List of interfaces we want to keep even if orphaned.
  std::vector<void *> RetainedTypes;

  /// Cache of forward declared types to RAUW at the end of compilation.
  std::vector<std::pair<const TagType *, toolchain::TrackingMDRef>> ReplaceMap;

  /// Cache of replaceable forward declarations (functions and
  /// variables) to RAUW at the end of compilation.
  std::vector<std::pair<const DeclaratorDecl *, toolchain::TrackingMDRef>>
      FwdDeclReplaceMap;

  /// Keep track of our current nested lexical block.
  std::vector<toolchain::TypedTrackingMDRef<toolchain::DIScope>> LexicalBlockStack;
  toolchain::DenseMap<const Decl *, toolchain::TrackingMDRef> RegionMap;
  /// Keep track of LexicalBlockStack counter at the beginning of a
  /// function. This is used to pop unbalanced regions at the end of a
  /// function.
  std::vector<unsigned> FnBeginRegionCount;

  /// This is a storage for names that are constructed on demand. For
  /// example, C++ destructors, C++ operators etc..
  toolchain::BumpPtrAllocator DebugInfoNames;

  toolchain::DenseMap<const char *, toolchain::TrackingMDRef> DIFileCache;
  toolchain::DenseMap<const FunctionDecl *, toolchain::TrackingMDRef> SPCache;
  /// Cache declarations relevant to DW_TAG_imported_declarations (C++
  /// using declarations and global alias variables) that aren't covered
  /// by other more specific caches.
  toolchain::DenseMap<const Decl *, toolchain::TrackingMDRef> DeclCache;
  toolchain::DenseMap<const Decl *, toolchain::TrackingMDRef> ImportedDeclCache;
  toolchain::DenseMap<const NamespaceDecl *, toolchain::TrackingMDRef> NamespaceCache;
  toolchain::DenseMap<const NamespaceAliasDecl *, toolchain::TrackingMDRef>
      NamespaceAliasCache;
  toolchain::DenseMap<const Decl *, toolchain::TypedTrackingMDRef<toolchain::DIDerivedType>>
      StaticDataMemberCache;

  using ParamDecl2StmtTy = toolchain::DenseMap<const ParmVarDecl *, const Stmt *>;
  using Param2DILocTy =
      toolchain::DenseMap<const ParmVarDecl *, toolchain::DILocalVariable *>;

  /// The key is coroutine real parameters, value is coroutine move parameters.
  ParamDecl2StmtTy CoroutineParameterMappings;
  /// The key is coroutine real parameters, value is DIVariable in LLVM IR.
  Param2DILocTy ParamDbgMappings;

  /// Key Instructions bookkeeping.
  /// Source atoms are identified by a {AtomGroup, InlinedAt} pair, meaning
  /// AtomGroup numbers can be repeated across different functions.
  struct {
    uint64_t NextAtom = 1;
    uint64_t HighestEmittedAtom = 0;
    uint64_t CurrentAtom = 0;
  } KeyInstructionsInfo;

private:
  /// Helper functions for getOrCreateType.
  /// @{
  /// Currently the checksum of an interface includes the number of
  /// ivars and property accessors.
  toolchain::DIType *CreateType(const BuiltinType *Ty);
  toolchain::DIType *CreateType(const ComplexType *Ty);
  toolchain::DIType *CreateType(const BitIntType *Ty);
  toolchain::DIType *CreateQualifiedType(QualType Ty, toolchain::DIFile *Fg);
  toolchain::DIType *CreateQualifiedType(const FunctionProtoType *Ty,
                                    toolchain::DIFile *Fg);
  toolchain::DIType *CreateType(const TypedefType *Ty, toolchain::DIFile *Fg);
  toolchain::DIType *CreateType(const TemplateSpecializationType *Ty,
                           toolchain::DIFile *Fg);
  toolchain::DIType *CreateType(const ObjCObjectPointerType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const PointerType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const BlockPointerType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const FunctionType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const HLSLAttributedResourceType *Ty,
                           toolchain::DIFile *F);
  toolchain::DIType *CreateType(const HLSLInlineSpirvType *Ty, toolchain::DIFile *F);
  /// Get structure or union type.
  toolchain::DIType *CreateType(const RecordType *Tyg);

  /// Create definition for the specified 'Ty'.
  ///
  /// \returns A pair of 'toolchain::DIType's. The first is the definition
  /// of the 'Ty'. The second is the type specified by the preferred_name
  /// attribute on 'Ty', which can be a nullptr if no such attribute
  /// exists.
  std::pair<toolchain::DIType *, toolchain::DIType *>
  CreateTypeDefinition(const RecordType *Ty);
  toolchain::DICompositeType *CreateLimitedType(const RecordType *Ty);
  void CollectContainingType(const CXXRecordDecl *RD,
                             toolchain::DICompositeType *CT);
  /// Get Objective-C interface type.
  toolchain::DIType *CreateType(const ObjCInterfaceType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateTypeDefinition(const ObjCInterfaceType *Ty,
                                     toolchain::DIFile *F);
  /// Get Objective-C object type.
  toolchain::DIType *CreateType(const ObjCObjectType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const ObjCTypeParamType *Ty, toolchain::DIFile *Unit);

  toolchain::DIType *CreateType(const VectorType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const ConstantMatrixType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const ArrayType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const LValueReferenceType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const RValueReferenceType *Ty, toolchain::DIFile *Unit);
  toolchain::DIType *CreateType(const MemberPointerType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const AtomicType *Ty, toolchain::DIFile *F);
  toolchain::DIType *CreateType(const PipeType *Ty, toolchain::DIFile *F);
  /// Get enumeration type.
  toolchain::DIType *CreateEnumType(const EnumType *Ty);
  toolchain::DIType *CreateTypeDefinition(const EnumType *Ty);
  /// Look up the completed type for a self pointer in the TypeCache and
  /// create a copy of it with the ObjectPointer and Artificial flags
  /// set. If the type is not cached, a new one is created. This should
  /// never happen though, since creating a type for the implicit self
  /// argument implies that we already parsed the interface definition
  /// and the ivar declarations in the implementation.
  toolchain::DIType *CreateSelfType(const QualType &QualTy, toolchain::DIType *Ty);
  /// @}

  /// Get the type from the cache or return null type if it doesn't
  /// exist.
  toolchain::DIType *getTypeOrNull(const QualType);
  /// Return the debug type for a C++ method.
  /// \arg CXXMethodDecl is of FunctionType. This function type is
  /// not updated to include implicit \c this pointer. Use this routine
  /// to get a method type which includes \c this pointer.
  toolchain::DISubroutineType *getOrCreateMethodType(const CXXMethodDecl *Method,
                                                toolchain::DIFile *F);

  toolchain::DISubroutineType *
  getOrCreateMethodTypeForDestructor(const CXXMethodDecl *Method,
                                     toolchain::DIFile *F, QualType FNType);

  toolchain::DISubroutineType *
  getOrCreateInstanceMethodType(QualType ThisPtr, const FunctionProtoType *Func,
                                toolchain::DIFile *Unit, bool SkipFirst = false);
  toolchain::DISubroutineType *
  getOrCreateFunctionType(const Decl *D, QualType FnType, toolchain::DIFile *F);
  /// \return debug info descriptor for vtable.
  toolchain::DIType *getOrCreateVTablePtrType(toolchain::DIFile *F);

  /// \return namespace descriptor for the given namespace decl.
  toolchain::DINamespace *getOrCreateNamespace(const NamespaceDecl *N);
  toolchain::DIType *CreatePointerLikeType(toolchain::dwarf::Tag Tag, const Type *Ty,
                                      QualType PointeeTy, toolchain::DIFile *F);
  toolchain::DIType *getOrCreateStructPtrType(StringRef Name, toolchain::DIType *&Cache);

  /// A helper function to create a subprogram for a single member
  /// function GlobalDecl.
  toolchain::DISubprogram *CreateCXXMemberFunction(const CXXMethodDecl *Method,
                                              toolchain::DIFile *F,
                                              toolchain::DIType *RecordTy);

  /// A helper function to collect debug info for C++ member
  /// functions. This is used while creating debug info entry for a
  /// Record.
  void CollectCXXMemberFunctions(const CXXRecordDecl *Decl, toolchain::DIFile *F,
                                 SmallVectorImpl<toolchain::Metadata *> &E,
                                 toolchain::DIType *T);

  /// A helper function to collect debug info for C++ base
  /// classes. This is used while creating debug info entry for a
  /// Record.
  void CollectCXXBases(const CXXRecordDecl *Decl, toolchain::DIFile *F,
                       SmallVectorImpl<toolchain::Metadata *> &EltTys,
                       toolchain::DIType *RecordTy);

  /// Helper function for CollectCXXBases.
  /// Adds debug info entries for types in Bases that are not in SeenTypes.
  void CollectCXXBasesAux(
      const CXXRecordDecl *RD, toolchain::DIFile *Unit,
      SmallVectorImpl<toolchain::Metadata *> &EltTys, toolchain::DIType *RecordTy,
      const CXXRecordDecl::base_class_const_range &Bases,
      toolchain::DenseSet<CanonicalDeclPtr<const CXXRecordDecl>> &SeenTypes,
      toolchain::DINode::DIFlags StartingFlags);

  /// Helper function that returns the toolchain::DIType that the
  /// PreferredNameAttr attribute on \ref RD refers to. If no such
  /// attribute exists, returns nullptr.
  toolchain::DIType *GetPreferredNameType(const CXXRecordDecl *RD,
                                     toolchain::DIFile *Unit);

  struct TemplateArgs {
    const TemplateParameterList *TList;
    toolchain::ArrayRef<TemplateArgument> Args;
  };
  /// A helper function to collect template parameters.
  toolchain::DINodeArray CollectTemplateParams(std::optional<TemplateArgs> Args,
                                          toolchain::DIFile *Unit);
  /// A helper function to collect debug info for function template
  /// parameters.
  toolchain::DINodeArray CollectFunctionTemplateParams(const FunctionDecl *FD,
                                                  toolchain::DIFile *Unit);

  /// A helper function to collect debug info for function template
  /// parameters.
  toolchain::DINodeArray CollectVarTemplateParams(const VarDecl *VD,
                                             toolchain::DIFile *Unit);

  std::optional<TemplateArgs> GetTemplateArgs(const VarDecl *) const;
  std::optional<TemplateArgs> GetTemplateArgs(const RecordDecl *) const;
  std::optional<TemplateArgs> GetTemplateArgs(const FunctionDecl *) const;

  /// A helper function to collect debug info for template
  /// parameters.
  toolchain::DINodeArray CollectCXXTemplateParams(const RecordDecl *TS,
                                             toolchain::DIFile *F);

  /// A helper function to collect debug info for btf_decl_tag annotations.
  toolchain::DINodeArray CollectBTFDeclTagAnnotations(const Decl *D);

  toolchain::DIType *createFieldType(StringRef name, QualType type,
                                SourceLocation loc, AccessSpecifier AS,
                                uint64_t offsetInBits, uint32_t AlignInBits,
                                toolchain::DIFile *tunit, toolchain::DIScope *scope,
                                const RecordDecl *RD = nullptr,
                                toolchain::DINodeArray Annotations = nullptr);

  toolchain::DIType *createFieldType(StringRef name, QualType type,
                                SourceLocation loc, AccessSpecifier AS,
                                uint64_t offsetInBits, toolchain::DIFile *tunit,
                                toolchain::DIScope *scope,
                                const RecordDecl *RD = nullptr) {
    return createFieldType(name, type, loc, AS, offsetInBits, 0, tunit, scope,
                           RD);
  }

  /// Create new bit field member.
  toolchain::DIDerivedType *createBitFieldType(const FieldDecl *BitFieldDecl,
                                          toolchain::DIScope *RecordTy,
                                          const RecordDecl *RD);

  /// Create an anonnymous zero-size separator for bit-field-decl if needed on
  /// the target.
  toolchain::DIDerivedType *createBitFieldSeparatorIfNeeded(
      const FieldDecl *BitFieldDecl, const toolchain::DIDerivedType *BitFieldDI,
      toolchain::ArrayRef<toolchain::Metadata *> PreviousFieldsDI, const RecordDecl *RD);

  /// A cache that maps names of artificial inlined functions to subprograms.
  toolchain::StringMap<toolchain::DISubprogram *> InlinedSubprogramMap;

  /// A function that returns the subprogram corresponding to the artificial
  /// inlined function for traps.
  toolchain::DISubprogram *createInlinedSubprogram(StringRef FuncName,
                                              toolchain::DIFile *FileScope);

  /// Helpers for collecting fields of a record.
  /// @{
  void CollectRecordLambdaFields(const CXXRecordDecl *CXXDecl,
                                 SmallVectorImpl<toolchain::Metadata *> &E,
                                 toolchain::DIType *RecordTy);
  toolchain::DIDerivedType *CreateRecordStaticField(const VarDecl *Var,
                                               toolchain::DIType *RecordTy,
                                               const RecordDecl *RD);
  void CollectRecordNormalField(const FieldDecl *Field, uint64_t OffsetInBits,
                                toolchain::DIFile *F,
                                SmallVectorImpl<toolchain::Metadata *> &E,
                                toolchain::DIType *RecordTy, const RecordDecl *RD);
  void CollectRecordNestedType(const TypeDecl *RD,
                               SmallVectorImpl<toolchain::Metadata *> &E);
  void CollectRecordFields(const RecordDecl *Decl, toolchain::DIFile *F,
                           SmallVectorImpl<toolchain::Metadata *> &E,
                           toolchain::DICompositeType *RecordTy);

  /// If the C++ class has vtable info then insert appropriate debug
  /// info entry in EltTys vector.
  void CollectVTableInfo(const CXXRecordDecl *Decl, toolchain::DIFile *F,
                         SmallVectorImpl<toolchain::Metadata *> &EltTys);
  /// @}

  /// Create a new lexical block node and push it on the stack.
  void CreateLexicalBlock(SourceLocation Loc);

  /// If target-specific LLVM \p AddressSpace directly maps to target-specific
  /// DWARF address space, appends extended dereferencing mechanism to complex
  /// expression \p Expr. Otherwise, does nothing.
  ///
  /// Extended dereferencing mechanism is has the following format:
  ///     DW_OP_constu <DWARF Address Space> DW_OP_swap DW_OP_xderef
  void AppendAddressSpaceXDeref(unsigned AddressSpace,
                                SmallVectorImpl<uint64_t> &Expr) const;

  /// A helper function to collect debug info for the default elements of a
  /// block.
  ///
  /// \returns The next available field offset after the default elements.
  uint64_t collectDefaultElementTypesForBlockPointer(
      const BlockPointerType *Ty, toolchain::DIFile *Unit,
      toolchain::DIDerivedType *DescTy, unsigned LineNo,
      SmallVectorImpl<toolchain::Metadata *> &EltTys);

  /// A helper function to collect debug info for the default fields of a
  /// block.
  void collectDefaultFieldsForBlockLiteralDeclare(
      const CGBlockInfo &Block, const ASTContext &Context, SourceLocation Loc,
      const toolchain::StructLayout &BlockLayout, toolchain::DIFile *Unit,
      SmallVectorImpl<toolchain::Metadata *> &Fields);

public:
  CGDebugInfo(CodeGenModule &CGM);
  ~CGDebugInfo();

  void finalize();

  /// Remap a given path with the current debug prefix map
  std::string remapDIPath(StringRef) const;

  /// Register VLA size expression debug node with the qualified type.
  void registerVLASizeExpression(QualType Ty, toolchain::Metadata *SizeExpr) {
    SizeExprCache[Ty] = SizeExpr;
  }

  /// Module debugging: Support for building PCMs.
  /// @{
  /// Set the main CU's DwoId field to \p Signature.
  void setDwoId(uint64_t Signature);

  /// When generating debug information for a clang module or
  /// precompiled header, this module map will be used to determine
  /// the module of origin of each Decl.
  void setModuleMap(ModuleMap &MMap) { ClangModuleMap = &MMap; }

  /// When generating debug information for a clang module or
  /// precompiled header, this module map will be used to determine
  /// the module of origin of each Decl.
  void setPCHDescriptor(ASTSourceDescriptor PCH) { PCHDescriptor = PCH; }
  /// @}

  /// Update the current source location. If \arg loc is invalid it is
  /// ignored.
  void setLocation(SourceLocation Loc);

  /// Return the current source location. This does not necessarily correspond
  /// to the IRBuilder's current DebugLoc.
  SourceLocation getLocation() const { return CurLoc; }

  /// Update the current inline scope. All subsequent calls to \p EmitLocation
  /// will create a location with this inlinedAt field.
  void setInlinedAt(toolchain::MDNode *InlinedAt) { CurInlinedAt = InlinedAt; }

  /// \return the current inline scope.
  toolchain::MDNode *getInlinedAt() const { return CurInlinedAt; }

  // Converts a SourceLocation to a DebugLoc
  toolchain::DebugLoc SourceLocToDebugLoc(SourceLocation Loc);

  /// Emit metadata to indicate a change in line/column information in
  /// the source file. If the location is invalid, the previous
  /// location will be reused.
  void EmitLocation(CGBuilderTy &Builder, SourceLocation Loc);

  QualType getFunctionType(const FunctionDecl *FD, QualType RetTy,
                           const SmallVectorImpl<const VarDecl *> &Args);

  /// Emit a call to toolchain.dbg.function.start to indicate
  /// start of a new function.
  /// \param Loc       The location of the function header.
  /// \param ScopeLoc  The location of the function body.
  void emitFunctionStart(GlobalDecl GD, SourceLocation Loc,
                         SourceLocation ScopeLoc, QualType FnType,
                         toolchain::Function *Fn, bool CurFnIsThunk);

  /// Start a new scope for an inlined function.
  void EmitInlineFunctionStart(CGBuilderTy &Builder, GlobalDecl GD);
  /// End an inlined function scope.
  void EmitInlineFunctionEnd(CGBuilderTy &Builder);

  /// Emit debug info for a function declaration.
  /// \p Fn is set only when a declaration for a debug call site gets created.
  void EmitFunctionDecl(GlobalDecl GD, SourceLocation Loc,
                        QualType FnType, toolchain::Function *Fn = nullptr);

  /// Emit debug info for an extern function being called.
  /// This is needed for call site debug info.
  void EmitFuncDeclForCallSite(toolchain::CallBase *CallOrInvoke,
                               QualType CalleeType,
                               const FunctionDecl *CalleeDecl);

  /// Constructs the debug code for exiting a function.
  void EmitFunctionEnd(CGBuilderTy &Builder, toolchain::Function *Fn);

  /// Emit metadata to indicate the beginning of a new lexical block
  /// and push the block onto the stack.
  void EmitLexicalBlockStart(CGBuilderTy &Builder, SourceLocation Loc);

  /// Emit metadata to indicate the end of a new lexical block and pop
  /// the current block.
  void EmitLexicalBlockEnd(CGBuilderTy &Builder, SourceLocation Loc);

  /// Emit call to \c toolchain.dbg.declare for an automatic variable
  /// declaration.
  /// Returns a pointer to the DILocalVariable associated with the
  /// toolchain.dbg.declare, or nullptr otherwise.
  toolchain::DILocalVariable *
  EmitDeclareOfAutoVariable(const VarDecl *Decl, toolchain::Value *AI,
                            CGBuilderTy &Builder,
                            const bool UsePointerValue = false);

  /// Emit call to \c toolchain.dbg.label for an label.
  void EmitLabel(const LabelDecl *D, CGBuilderTy &Builder);

  /// Emit call to \c toolchain.dbg.declare for an imported variable
  /// declaration in a block.
  void EmitDeclareOfBlockDeclRefVariable(
      const VarDecl *variable, toolchain::Value *storage, CGBuilderTy &Builder,
      const CGBlockInfo &blockInfo, toolchain::Instruction *InsertPoint = nullptr);

  /// Emit call to \c toolchain.dbg.declare for an argument variable
  /// declaration.
  toolchain::DILocalVariable *
  EmitDeclareOfArgVariable(const VarDecl *Decl, toolchain::Value *AI, unsigned ArgNo,
                           CGBuilderTy &Builder, bool UsePointerValue = false);

  /// Emit call to \c toolchain.dbg.declare for the block-literal argument
  /// to a block invocation function.
  void EmitDeclareOfBlockLiteralArgVariable(const CGBlockInfo &block,
                                            StringRef Name, unsigned ArgNo,
                                            toolchain::AllocaInst *LocalAddr,
                                            CGBuilderTy &Builder);

  /// Emit information about a global variable.
  void EmitGlobalVariable(toolchain::GlobalVariable *GV, const VarDecl *Decl);

  /// Emit a constant global variable's debug info.
  void EmitGlobalVariable(const ValueDecl *VD, const APValue &Init);

  /// Emit information about an external variable.
  void EmitExternalVariable(toolchain::GlobalVariable *GV, const VarDecl *Decl);

  /// Emit a pseudo variable and debug info for an intermediate value if it does
  /// not correspond to a variable in the source code, so that a profiler can
  /// track more accurate usage of certain instructions of interest.
  void EmitPseudoVariable(CGBuilderTy &Builder, toolchain::Instruction *Value,
                          QualType Ty);

  /// Emit information about global variable alias.
  void EmitGlobalAlias(const toolchain::GlobalValue *GV, const GlobalDecl Decl);

  /// Emit C++ using directive.
  void EmitUsingDirective(const UsingDirectiveDecl &UD);

  /// Emit the type explicitly casted to.
  void EmitExplicitCastType(QualType Ty);

  /// Emit the type even if it might not be used.
  void EmitAndRetainType(QualType Ty);

  /// Emit a shadow decl brought in by a using or using-enum
  void EmitUsingShadowDecl(const UsingShadowDecl &USD);

  /// Emit C++ using declaration.
  void EmitUsingDecl(const UsingDecl &UD);

  /// Emit C++ using-enum declaration.
  void EmitUsingEnumDecl(const UsingEnumDecl &UD);

  /// Emit an @import declaration.
  void EmitImportDecl(const ImportDecl &ID);

  /// DebugInfo isn't attached to string literals by default. While certain
  /// aspects of debuginfo aren't useful for string literals (like a name), it's
  /// nice to be able to symbolize the line and column information. This is
  /// especially useful for sanitizers, as it allows symbolization of
  /// heap-buffer-overflows on constant strings.
  void AddStringLiteralDebugInfo(toolchain::GlobalVariable *GV,
                                 const StringLiteral *S);

  /// Emit C++ namespace alias.
  toolchain::DIImportedEntity *EmitNamespaceAlias(const NamespaceAliasDecl &NA);

  /// Emit record type's standalone debug info.
  toolchain::DIType *getOrCreateRecordType(QualType Ty, SourceLocation L);

  /// Emit an Objective-C interface type standalone debug info.
  toolchain::DIType *getOrCreateInterfaceType(QualType Ty, SourceLocation Loc);

  /// Emit standalone debug info for a type.
  toolchain::DIType *getOrCreateStandaloneType(QualType Ty, SourceLocation Loc);

  /// Add heapallocsite metadata for MSAllocator calls.
  void addHeapAllocSiteMetadata(toolchain::CallBase *CallSite, QualType AllocatedTy,
                                SourceLocation Loc);

  void completeType(const EnumDecl *ED);
  void completeType(const RecordDecl *RD);
  void completeRequiredType(const RecordDecl *RD);
  void completeClassData(const RecordDecl *RD);
  void completeClass(const RecordDecl *RD);

  void completeTemplateDefinition(const ClassTemplateSpecializationDecl &SD);
  void completeUnusedClass(const CXXRecordDecl &D);

  /// Create debug info for a macro defined by a #define directive or a macro
  /// undefined by a #undef directive.
  toolchain::DIMacro *CreateMacro(toolchain::DIMacroFile *Parent, unsigned MType,
                             SourceLocation LineLoc, StringRef Name,
                             StringRef Value);

  /// Create debug info for a file referenced by an #include directive.
  toolchain::DIMacroFile *CreateTempMacroFile(toolchain::DIMacroFile *Parent,
                                         SourceLocation LineLoc,
                                         SourceLocation FileLoc);

  Param2DILocTy &getParamDbgMappings() { return ParamDbgMappings; }
  ParamDecl2StmtTy &getCoroutineParameterMappings() {
    return CoroutineParameterMappings;
  }

  /// Create a debug location from `TrapLocation` that adds an artificial inline
  /// frame where the frame name is
  ///
  /// * `<Prefix>:<Category>:<FailureMsg>`
  ///
  /// `<Prefix>` is "__clang_trap_msg".
  ///
  /// This is used to store failure reasons for traps.
  toolchain::DILocation *CreateTrapFailureMessageFor(toolchain::DebugLoc TrapLocation,
                                                StringRef Category,
                                                StringRef FailureMsg);
  /// Create a debug location from `Location` that adds an artificial inline
  /// frame where the frame name is FuncName
  ///
  /// This is used to indiciate instructions that come from compiler
  /// instrumentation.
  toolchain::DILocation *CreateSyntheticInlineAt(toolchain::DebugLoc Location,
                                            StringRef FuncName);

  /// Reset internal state.
  void completeFunction();

  /// Add \p KeyInstruction and an optional \p Backup instruction to the
  /// current atom group, created using ApplyAtomGroup.
  void addInstToCurrentSourceAtom(toolchain::Instruction *KeyInstruction,
                                  toolchain::Value *Backup);

  /// Add \p KeyInstruction and an optional \p Backup instruction to the atom
  /// group \p Atom.
  void addInstToSpecificSourceAtom(toolchain::Instruction *KeyInstruction,
                                   toolchain::Value *Backup, uint64_t Atom);

  /// Emit symbol for debugger that holds the pointer to the vtable.
  void emitVTableSymbol(toolchain::GlobalVariable *VTable, const CXXRecordDecl *RD);

private:
  /// Amend \p I's DebugLoc with \p Group (its source atom group) and \p
  /// Rank (lower nonzero rank is higher precedence). Does nothing if \p I
  /// has no DebugLoc, and chooses the atom group in which the instruction
  /// has the highest precedence if it's already in one.
  void addInstSourceAtomMetadata(toolchain::Instruction *I, uint64_t Group,
                                 uint8_t Rank);

  /// Emit call to toolchain.dbg.declare for a variable declaration.
  /// Returns a pointer to the DILocalVariable associated with the
  /// toolchain.dbg.declare, or nullptr otherwise.
  toolchain::DILocalVariable *EmitDeclare(const VarDecl *decl, toolchain::Value *AI,
                                     std::optional<unsigned> ArgNo,
                                     CGBuilderTy &Builder,
                                     const bool UsePointerValue = false);

  /// Emit call to toolchain.dbg.declare for a binding declaration.
  /// Returns a pointer to the DILocalVariable associated with the
  /// toolchain.dbg.declare, or nullptr otherwise.
  toolchain::DILocalVariable *EmitDeclare(const BindingDecl *decl, toolchain::Value *AI,
                                     std::optional<unsigned> ArgNo,
                                     CGBuilderTy &Builder,
                                     const bool UsePointerValue = false);

  struct BlockByRefType {
    /// The wrapper struct used inside the __block_literal struct.
    toolchain::DIType *BlockByRefWrapper;
    /// The type as it appears in the source code.
    toolchain::DIType *WrappedType;
  };

  bool HasReconstitutableArgs(ArrayRef<TemplateArgument> Args) const;
  std::string GetName(const Decl *, bool Qualified = false) const;

  /// Build up structure info for the byref.  See \a BuildByRefType.
  BlockByRefType EmitTypeForVarWithBlocksAttr(const VarDecl *VD,
                                              uint64_t *OffSet);

  /// Get context info for the DeclContext of \p Decl.
  toolchain::DIScope *getDeclContextDescriptor(const Decl *D);
  /// Get context info for a given DeclContext \p Decl.
  toolchain::DIScope *getContextDescriptor(const Decl *Context,
                                      toolchain::DIScope *Default);

  toolchain::DIScope *getCurrentContextDescriptor(const Decl *Decl);

  /// Create a forward decl for a RecordType in a given context.
  toolchain::DICompositeType *getOrCreateRecordFwdDecl(const RecordType *,
                                                  toolchain::DIScope *);

  /// Return current directory name.
  StringRef getCurrentDirname();

  /// Create new compile unit.
  void CreateCompileUnit();

  /// Compute the file checksum debug info for input file ID.
  std::optional<toolchain::DIFile::ChecksumKind>
  computeChecksum(FileID FID, SmallString<64> &Checksum) const;

  /// Get the source of the given file ID.
  std::optional<StringRef> getSource(const SourceManager &SM, FileID FID);

  /// Convenience function to get the file debug info descriptor for the input
  /// location.
  toolchain::DIFile *getOrCreateFile(SourceLocation Loc);

  /// Create a file debug info descriptor for a source file.
  toolchain::DIFile *
  createFile(StringRef FileName,
             std::optional<toolchain::DIFile::ChecksumInfo<StringRef>> CSInfo,
             std::optional<StringRef> Source);

  /// Get the type from the cache or create a new type if necessary.
  toolchain::DIType *getOrCreateType(QualType Ty, toolchain::DIFile *Fg);

  /// Get a reference to a clang module.  If \p CreateSkeletonCU is true,
  /// this also creates a split dwarf skeleton compile unit.
  toolchain::DIModule *getOrCreateModuleRef(ASTSourceDescriptor Mod,
                                       bool CreateSkeletonCU);

  /// DebugTypeExtRefs: If \p D originated in a clang module, return it.
  toolchain::DIModule *getParentModuleOrNull(const Decl *D);

  /// Get the type from the cache or create a new partial type if
  /// necessary.
  toolchain::DICompositeType *getOrCreateLimitedType(const RecordType *Ty);

  /// Create type metadata for a source language type.
  toolchain::DIType *CreateTypeNode(QualType Ty, toolchain::DIFile *Fg);

  /// Create new member and increase Offset by FType's size.
  toolchain::DIType *CreateMemberType(toolchain::DIFile *Unit, QualType FType,
                                 StringRef Name, uint64_t *Offset);

  /// Retrieve the DIDescriptor, if any, for the canonical form of this
  /// declaration.
  toolchain::DINode *getDeclarationOrDefinition(const Decl *D);

  /// \return debug info descriptor to describe method
  /// declaration for the given method definition.
  toolchain::DISubprogram *getFunctionDeclaration(const Decl *D);

  /// \return          debug info descriptor to the describe method declaration
  ///                  for the given method definition.
  /// \param FnType    For Objective-C methods, their type.
  /// \param LineNo    The declaration's line number.
  /// \param Flags     The DIFlags for the method declaration.
  /// \param SPFlags   The subprogram-spcific flags for the method declaration.
  toolchain::DISubprogram *
  getObjCMethodDeclaration(const Decl *D, toolchain::DISubroutineType *FnType,
                           unsigned LineNo, toolchain::DINode::DIFlags Flags,
                           toolchain::DISubprogram::DISPFlags SPFlags);

  /// \return debug info descriptor to describe in-class static data
  /// member declaration for the given out-of-class definition.  If D
  /// is an out-of-class definition of a static data member of a
  /// class, find its corresponding in-class declaration.
  toolchain::DIDerivedType *
  getOrCreateStaticDataMemberDeclarationOrNull(const VarDecl *D);

  /// Helper that either creates a forward declaration or a stub.
  toolchain::DISubprogram *getFunctionFwdDeclOrStub(GlobalDecl GD, bool Stub);

  /// Create a subprogram describing the forward declaration
  /// represented in the given FunctionDecl wrapped in a GlobalDecl.
  toolchain::DISubprogram *getFunctionForwardDeclaration(GlobalDecl GD);

  /// Create a DISubprogram describing the function
  /// represented in the given FunctionDecl wrapped in a GlobalDecl.
  toolchain::DISubprogram *getFunctionStub(GlobalDecl GD);

  /// Create a global variable describing the forward declaration
  /// represented in the given VarDecl.
  toolchain::DIGlobalVariable *
  getGlobalVariableForwardDeclaration(const VarDecl *VD);

  /// Return a global variable that represents one of the collection of global
  /// variables created for an anonmyous union.
  ///
  /// Recursively collect all of the member fields of a global
  /// anonymous decl and create static variables for them. The first
  /// time this is called it needs to be on a union and then from
  /// there we can have additional unnamed fields.
  toolchain::DIGlobalVariableExpression *
  CollectAnonRecordDecls(const RecordDecl *RD, toolchain::DIFile *Unit,
                         unsigned LineNo, StringRef LinkageName,
                         toolchain::GlobalVariable *Var, toolchain::DIScope *DContext);


  /// Return flags which enable debug info emission for call sites, provided
  /// that it is supported and enabled.
  toolchain::DINode::DIFlags getCallSiteRelatedAttrs() const;

  /// Get the printing policy for producing names for debug info.
  PrintingPolicy getPrintingPolicy() const;

  /// Get function name for the given FunctionDecl. If the name is
  /// constructed on demand (e.g., C++ destructor) then the name is
  /// stored on the side.
  StringRef getFunctionName(const FunctionDecl *FD);

  /// Returns the unmangled name of an Objective-C method.
  /// This is the display name for the debugging info.
  StringRef getObjCMethodName(const ObjCMethodDecl *FD);

  /// Return selector name. This is used for debugging
  /// info.
  StringRef getSelectorName(Selector S);

  /// Get class name including template argument list.
  StringRef getClassName(const RecordDecl *RD);

  /// Get the vtable name for the given class.
  StringRef getVTableName(const CXXRecordDecl *Decl);

  /// Get the name to use in the debug info for a dynamic initializer or atexit
  /// stub function.
  StringRef getDynamicInitializerName(const VarDecl *VD,
                                      DynamicInitKind StubKind,
                                      toolchain::Function *InitFn);

  /// Get line number for the location. If location is invalid
  /// then use current location.
  unsigned getLineNumber(SourceLocation Loc);

  /// Get column number for the location. If location is
  /// invalid then use current location.
  /// \param Force  Assume DebugColumnInfo option is true.
  unsigned getColumnNumber(SourceLocation Loc, bool Force = false);

  /// Collect various properties of a FunctionDecl.
  /// \param GD  A GlobalDecl whose getDecl() must return a FunctionDecl.
  void collectFunctionDeclProps(GlobalDecl GD, toolchain::DIFile *Unit,
                                StringRef &Name, StringRef &LinkageName,
                                toolchain::DIScope *&FDContext,
                                toolchain::DINodeArray &TParamsArray,
                                toolchain::DINode::DIFlags &Flags);

  /// Collect various properties of a VarDecl.
  void collectVarDeclProps(const VarDecl *VD, toolchain::DIFile *&Unit,
                           unsigned &LineNo, QualType &T, StringRef &Name,
                           StringRef &LinkageName,
                           toolchain::MDTuple *&TemplateParameters,
                           toolchain::DIScope *&VDContext);

  /// Create a DIExpression representing the constant corresponding
  /// to the specified 'Val'. Returns nullptr on failure.
  toolchain::DIExpression *createConstantValueExpression(const language::Core::ValueDecl *VD,
                                                    const APValue &Val);

  /// Allocate a copy of \p A using the DebugInfoNames allocator
  /// and return a reference to it. If multiple arguments are given the strings
  /// are concatenated.
  StringRef internString(StringRef A, StringRef B = StringRef()) {
    char *Data = DebugInfoNames.Allocate<char>(A.size() + B.size());
    if (!A.empty())
      std::memcpy(Data, A.data(), A.size());
    if (!B.empty())
      std::memcpy(Data + A.size(), B.data(), B.size());
    return StringRef(Data, A.size() + B.size());
  }
};

/// A scoped helper to set the current debug location to the specified
/// location or preferred location of the specified Expr.
class ApplyDebugLocation {
private:
  void init(SourceLocation TemporaryLocation, bool DefaultToEmpty = false);
  ApplyDebugLocation(CodeGenFunction &CGF, bool DefaultToEmpty,
                     SourceLocation TemporaryLocation);

  toolchain::DebugLoc OriginalLocation;
  CodeGenFunction *CGF;

public:
  /// Set the location to the (valid) TemporaryLocation.
  ApplyDebugLocation(CodeGenFunction &CGF, SourceLocation TemporaryLocation);
  ApplyDebugLocation(CodeGenFunction &CGF, const Expr *E);
  ApplyDebugLocation(CodeGenFunction &CGF, toolchain::DebugLoc Loc);
  ApplyDebugLocation(ApplyDebugLocation &&Other) : CGF(Other.CGF) {
    Other.CGF = nullptr;
  }

  // Define copy assignment operator.
  ApplyDebugLocation &operator=(ApplyDebugLocation &&Other) {
    if (this != &Other) {
      CGF = Other.CGF;
      Other.CGF = nullptr;
    }
    return *this;
  }

  ~ApplyDebugLocation();

  /// Apply TemporaryLocation if it is valid. Otherwise switch
  /// to an artificial debug location that has a valid scope, but no
  /// line information.
  ///
  /// Artificial locations are useful when emitting compiler-generated
  /// helper functions that have no source location associated with
  /// them. The DWARF specification allows the compiler to use the
  /// special line number 0 to indicate code that can not be
  /// attributed to any source location. Note that passing an empty
  /// SourceLocation to CGDebugInfo::setLocation() will result in the
  /// last valid location being reused.
  static ApplyDebugLocation CreateArtificial(CodeGenFunction &CGF) {
    return ApplyDebugLocation(CGF, false, SourceLocation());
  }
  /// Apply TemporaryLocation if it is valid. Otherwise switch
  /// to an artificial debug location that has a valid scope, but no
  /// line information.
  static ApplyDebugLocation
  CreateDefaultArtificial(CodeGenFunction &CGF,
                          SourceLocation TemporaryLocation) {
    return ApplyDebugLocation(CGF, false, TemporaryLocation);
  }

  /// Set the IRBuilder to not attach debug locations.  Note that
  /// passing an empty SourceLocation to \a CGDebugInfo::setLocation()
  /// will result in the last valid location being reused.  Note that
  /// all instructions that do not have a location at the beginning of
  /// a function are counted towards to function prologue.
  static ApplyDebugLocation CreateEmpty(CodeGenFunction &CGF) {
    return ApplyDebugLocation(CGF, true, SourceLocation());
  }
};

/// A scoped helper to set the current debug location to an inlined location.
class ApplyInlineDebugLocation {
  SourceLocation SavedLocation;
  CodeGenFunction *CGF;

public:
  /// Set up the CodeGenFunction's DebugInfo to produce inline locations for the
  /// function \p InlinedFn. The current debug location becomes the inlined call
  /// site of the inlined function.
  ApplyInlineDebugLocation(CodeGenFunction &CGF, GlobalDecl InlinedFn);
  /// Restore everything back to the original state.
  ~ApplyInlineDebugLocation();
};

class SanitizerDebugLocation {
  CodeGenFunction *CGF;
  ApplyDebugLocation Apply;

public:
  SanitizerDebugLocation(CodeGenFunction *CGF,
                         ArrayRef<SanitizerKind::SanitizerOrdinal> Ordinals,
                         SanitizerHandler Handler);
  ~SanitizerDebugLocation();
};

} // namespace CodeGen
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_CODEGEN_CGDEBUGINFO_H
