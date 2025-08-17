//===--- CodeGenModule.h - Per-Module state for LLVM CodeGen ----*- C++ -*-===//
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
// This is the internal per-translation-unit state used for toolchain translation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_CODEGENMODULE_H
#define LANGUAGE_CORE_LIB_CODEGEN_CODEGENMODULE_H

#include "CGVTables.h"
#include "CodeGenTypeCache.h"
#include "CodeGenTypes.h"
#include "SanitizerMetadata.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/AST/DeclOpenMP.h"
#include "language/Core/AST/GlobalDecl.h"
#include "language/Core/AST/Mangle.h"
#include "language/Core/Basic/ABI.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/NoSanitizeList.h"
#include "language/Core/Basic/ProfileList.h"
#include "language/Core/Basic/StackExhaustionHandler.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/XRayLists.h"
#include "language/Core/Lex/PreprocessorOptions.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/IR/Module.h"
#include "toolchain/IR/ValueHandle.h"
#include "toolchain/Transforms/Utils/SanitizerStats.h"
#include <optional>

namespace toolchain {
class Module;
class Constant;
class ConstantInt;
class Function;
class GlobalValue;
class DataLayout;
class FunctionType;
class LLVMContext;
class IndexedInstrProfReader;

namespace vfs {
class FileSystem;
}
}

namespace language::Core {
class ASTContext;
class AtomicType;
class FunctionDecl;
class IdentifierInfo;
class ObjCImplementationDecl;
class ObjCEncodeExpr;
class BlockExpr;
class CharUnits;
class Decl;
class Expr;
class Stmt;
class StringLiteral;
class NamedDecl;
class PointerAuthSchema;
class ValueDecl;
class VarDecl;
class LangOptions;
class CodeGenOptions;
class HeaderSearchOptions;
class DiagnosticsEngine;
class AnnotateAttr;
class CXXDestructorDecl;
class Module;
class CoverageSourceInfo;
class InitSegAttr;

namespace CodeGen {

class CodeGenFunction;
class CodeGenTBAA;
class CGCXXABI;
class CGDebugInfo;
class CGObjCRuntime;
class CGOpenCLRuntime;
class CGOpenMPRuntime;
class CGCUDARuntime;
class CGHLSLRuntime;
class CoverageMappingModuleGen;
class TargetCodeGenInfo;

enum ForDefinition_t : bool {
  NotForDefinition = false,
  ForDefinition = true
};

/// The Counter with an optional additional Counter for
/// branches. `Skipped` counter can be calculated with `Executed` and
/// a common Counter (like `Parent`) as `(Parent-Executed)`.
///
/// In SingleByte mode, Counters are binary. Subtraction is not
/// applicable (but addition is capable). In this case, both
/// `Executed` and `Skipped` counters are required.  `Skipped` is
/// `None` by default. It is allocated in the coverage mapping.
///
/// There might be cases that `Parent` could be induced with
/// `(Executed+Skipped)`. This is not always applicable.
class CounterPair {
public:
  /// Optional value.
  class ValueOpt {
  private:
    static constexpr uint32_t None = (1u << 31); /// None is allocated.
    static constexpr uint32_t Mask = None - 1;

    uint32_t Val;

  public:
    ValueOpt() : Val(None) {}

    ValueOpt(unsigned InitVal) {
      assert(!(InitVal & ~Mask));
      Val = InitVal;
    }

    bool hasValue() const { return !(Val & None); }

    operator uint32_t() const { return Val; }
  };

  ValueOpt Executed;
  ValueOpt Skipped; /// May be None.

  /// Initialized with Skipped=None.
  CounterPair(unsigned Val) : Executed(Val) {}

  // FIXME: Should work with {None, None}
  CounterPair() : Executed(0) {}
};

struct OrderGlobalInitsOrStermFinalizers {
  unsigned int priority;
  unsigned int lex_order;
  OrderGlobalInitsOrStermFinalizers(unsigned int p, unsigned int l)
      : priority(p), lex_order(l) {}

  bool operator==(const OrderGlobalInitsOrStermFinalizers &RHS) const {
    return priority == RHS.priority && lex_order == RHS.lex_order;
  }

  bool operator<(const OrderGlobalInitsOrStermFinalizers &RHS) const {
    return std::tie(priority, lex_order) <
           std::tie(RHS.priority, RHS.lex_order);
  }
};

struct ObjCEntrypoints {
  ObjCEntrypoints() { memset(this, 0, sizeof(*this)); }

  /// void objc_alloc(id);
  toolchain::FunctionCallee objc_alloc;

  /// void objc_allocWithZone(id);
  toolchain::FunctionCallee objc_allocWithZone;

  /// void objc_alloc_init(id);
  toolchain::FunctionCallee objc_alloc_init;

  /// void objc_autoreleasePoolPop(void*);
  toolchain::FunctionCallee objc_autoreleasePoolPop;

  /// void objc_autoreleasePoolPop(void*);
  /// Note this method is used when we are using exception handling
  toolchain::FunctionCallee objc_autoreleasePoolPopInvoke;

  /// void *objc_autoreleasePoolPush(void);
  toolchain::Function *objc_autoreleasePoolPush;

  /// id objc_autorelease(id);
  toolchain::Function *objc_autorelease;

  /// id objc_autorelease(id);
  /// Note this is the runtime method not the intrinsic.
  toolchain::FunctionCallee objc_autoreleaseRuntimeFunction;

  /// id objc_autoreleaseReturnValue(id);
  toolchain::Function *objc_autoreleaseReturnValue;

  /// void objc_copyWeak(id *dest, id *src);
  toolchain::Function *objc_copyWeak;

  /// void objc_destroyWeak(id*);
  toolchain::Function *objc_destroyWeak;

  /// id objc_initWeak(id*, id);
  toolchain::Function *objc_initWeak;

  /// id objc_loadWeak(id*);
  toolchain::Function *objc_loadWeak;

  /// id objc_loadWeakRetained(id*);
  toolchain::Function *objc_loadWeakRetained;

  /// void objc_moveWeak(id *dest, id *src);
  toolchain::Function *objc_moveWeak;

  /// id objc_retain(id);
  toolchain::Function *objc_retain;

  /// id objc_retain(id);
  /// Note this is the runtime method not the intrinsic.
  toolchain::FunctionCallee objc_retainRuntimeFunction;

  /// id objc_retainAutorelease(id);
  toolchain::Function *objc_retainAutorelease;

  /// id objc_retainAutoreleaseReturnValue(id);
  toolchain::Function *objc_retainAutoreleaseReturnValue;

  /// id objc_retainAutoreleasedReturnValue(id);
  toolchain::Function *objc_retainAutoreleasedReturnValue;

  /// id objc_retainBlock(id);
  toolchain::Function *objc_retainBlock;

  /// void objc_release(id);
  toolchain::Function *objc_release;

  /// void objc_release(id);
  /// Note this is the runtime method not the intrinsic.
  toolchain::FunctionCallee objc_releaseRuntimeFunction;

  /// void objc_storeStrong(id*, id);
  toolchain::Function *objc_storeStrong;

  /// id objc_storeWeak(id*, id);
  toolchain::Function *objc_storeWeak;

  /// id objc_unsafeClaimAutoreleasedReturnValue(id);
  toolchain::Function *objc_unsafeClaimAutoreleasedReturnValue;

  /// A void(void) inline asm to use to mark that the return value of
  /// a call will be immediately retain.
  toolchain::InlineAsm *retainAutoreleasedReturnValueMarker;

  /// void clang.arc.use(...);
  toolchain::Function *clang_arc_use;

  /// void clang.arc.noop.use(...);
  toolchain::Function *clang_arc_noop_use;
};

/// This class records statistics on instrumentation based profiling.
class InstrProfStats {
  uint32_t VisitedInMainFile = 0;
  uint32_t MissingInMainFile = 0;
  uint32_t Visited = 0;
  uint32_t Missing = 0;
  uint32_t Mismatched = 0;

public:
  InstrProfStats() = default;
  /// Record that we've visited a function and whether or not that function was
  /// in the main source file.
  void addVisited(bool MainFile) {
    if (MainFile)
      ++VisitedInMainFile;
    ++Visited;
  }
  /// Record that a function we've visited has no profile data.
  void addMissing(bool MainFile) {
    if (MainFile)
      ++MissingInMainFile;
    ++Missing;
  }
  /// Record that a function we've visited has mismatched profile data.
  void addMismatched(bool MainFile) { ++Mismatched; }
  /// Whether or not the stats we've gathered indicate any potential problems.
  bool hasDiagnostics() { return Missing || Mismatched; }
  /// Report potential problems we've found to \c Diags.
  void reportDiagnostics(DiagnosticsEngine &Diags, StringRef MainFile);
};

/// A pair of helper functions for a __block variable.
class BlockByrefHelpers : public toolchain::FoldingSetNode {
  // MSVC requires this type to be complete in order to process this
  // header.
public:
  toolchain::Constant *CopyHelper;
  toolchain::Constant *DisposeHelper;

  /// The alignment of the field.  This is important because
  /// different offsets to the field within the byref struct need to
  /// have different helper functions.
  CharUnits Alignment;

  BlockByrefHelpers(CharUnits alignment)
      : CopyHelper(nullptr), DisposeHelper(nullptr), Alignment(alignment) {}
  BlockByrefHelpers(const BlockByrefHelpers &) = default;
  virtual ~BlockByrefHelpers();

  void Profile(toolchain::FoldingSetNodeID &id) const {
    id.AddInteger(Alignment.getQuantity());
    profileImpl(id);
  }
  virtual void profileImpl(toolchain::FoldingSetNodeID &id) const = 0;

  virtual bool needsCopy() const { return true; }
  virtual void emitCopy(CodeGenFunction &CGF, Address dest, Address src) = 0;

  virtual bool needsDispose() const { return true; }
  virtual void emitDispose(CodeGenFunction &CGF, Address field) = 0;
};

/// This class organizes the cross-function state that is used while generating
/// LLVM code.
class CodeGenModule : public CodeGenTypeCache {
  CodeGenModule(const CodeGenModule &) = delete;
  void operator=(const CodeGenModule &) = delete;

public:
  struct Structor {
    Structor()
        : Priority(0), LexOrder(~0u), Initializer(nullptr),
          AssociatedData(nullptr) {}
    Structor(int Priority, unsigned LexOrder, toolchain::Constant *Initializer,
             toolchain::Constant *AssociatedData)
        : Priority(Priority), LexOrder(LexOrder), Initializer(Initializer),
          AssociatedData(AssociatedData) {}
    int Priority;
    unsigned LexOrder;
    toolchain::Constant *Initializer;
    toolchain::Constant *AssociatedData;
  };

  typedef std::vector<Structor> CtorList;

private:
  ASTContext &Context;
  const LangOptions &LangOpts;
  IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS; // Only used for debug info.
  const HeaderSearchOptions &HeaderSearchOpts; // Only used for debug info.
  const PreprocessorOptions &PreprocessorOpts; // Only used for debug info.
  const CodeGenOptions &CodeGenOpts;
  unsigned NumAutoVarInit = 0;
  toolchain::Module &TheModule;
  DiagnosticsEngine &Diags;
  const TargetInfo &Target;
  std::unique_ptr<CGCXXABI> ABI;
  toolchain::LLVMContext &VMContext;
  std::string ModuleNameHash;
  bool CXX20ModuleInits = false;
  std::unique_ptr<CodeGenTBAA> TBAA;

  mutable std::unique_ptr<TargetCodeGenInfo> TheTargetCodeGenInfo;

  // This should not be moved earlier, since its initialization depends on some
  // of the previous reference members being already initialized and also checks
  // if TheTargetCodeGenInfo is NULL
  std::unique_ptr<CodeGenTypes> Types;

  /// Holds information about C++ vtables.
  CodeGenVTables VTables;

  std::unique_ptr<CGObjCRuntime> ObjCRuntime;
  std::unique_ptr<CGOpenCLRuntime> OpenCLRuntime;
  std::unique_ptr<CGOpenMPRuntime> OpenMPRuntime;
  std::unique_ptr<CGCUDARuntime> CUDARuntime;
  std::unique_ptr<CGHLSLRuntime> HLSLRuntime;
  std::unique_ptr<CGDebugInfo> DebugInfo;
  std::unique_ptr<ObjCEntrypoints> ObjCData;
  toolchain::MDNode *NoObjCARCExceptionsMetadata = nullptr;
  std::unique_ptr<toolchain::IndexedInstrProfReader> PGOReader;
  InstrProfStats PGOStats;
  std::unique_ptr<toolchain::SanitizerStatReport> SanStats;
  StackExhaustionHandler StackHandler;

  // A set of references that have only been seen via a weakref so far. This is
  // used to remove the weak of the reference if we ever see a direct reference
  // or a definition.
  toolchain::SmallPtrSet<toolchain::GlobalValue*, 10> WeakRefReferences;

  /// This contains all the decls which have definitions but/ which are deferred
  /// for emission and therefore should only be output if they are actually
  /// used. If a decl is in this, then it is known to have not been referenced
  /// yet.
  toolchain::DenseMap<StringRef, GlobalDecl> DeferredDecls;

  toolchain::StringSet<toolchain::BumpPtrAllocator> DeferredResolversToEmit;

  /// This is a list of deferred decls which we have seen that *are* actually
  /// referenced. These get code generated when the module is done.
  std::vector<GlobalDecl> DeferredDeclsToEmit;
  void addDeferredDeclToEmit(GlobalDecl GD) {
    DeferredDeclsToEmit.emplace_back(GD);
    addEmittedDeferredDecl(GD);
  }

  /// Decls that were DeferredDecls and have now been emitted.
  toolchain::DenseMap<toolchain::StringRef, GlobalDecl> EmittedDeferredDecls;

  void addEmittedDeferredDecl(GlobalDecl GD) {
    // Reemission is only needed in incremental mode.
    if (!Context.getLangOpts().IncrementalExtensions)
      return;

    // Assume a linkage by default that does not need reemission.
    auto L = toolchain::GlobalValue::ExternalLinkage;
    if (toolchain::isa<FunctionDecl>(GD.getDecl()))
      L = getFunctionLinkage(GD);
    else if (auto *VD = toolchain::dyn_cast<VarDecl>(GD.getDecl()))
      L = getLLVMLinkageVarDefinition(VD);

    if (toolchain::GlobalValue::isInternalLinkage(L) ||
        toolchain::GlobalValue::isLinkOnceLinkage(L) ||
        toolchain::GlobalValue::isWeakLinkage(L)) {
      EmittedDeferredDecls[getMangledName(GD)] = GD;
    }
  }

  /// List of alias we have emitted. Used to make sure that what they point to
  /// is defined once we get to the end of the of the translation unit.
  std::vector<GlobalDecl> Aliases;

  /// List of multiversion functions to be emitted. This list is processed in
  /// conjunction with other deferred symbols and is used to ensure that
  /// multiversion function resolvers and ifuncs are defined and emitted.
  std::vector<GlobalDecl> MultiVersionFuncs;

  toolchain::MapVector<StringRef, toolchain::TrackingVH<toolchain::Constant>> Replacements;

  /// List of global values to be replaced with something else. Used when we
  /// want to replace a GlobalValue but can't identify it by its mangled name
  /// anymore (because the name is already taken).
  toolchain::SmallVector<std::pair<toolchain::GlobalValue *, toolchain::Constant *>, 8>
    GlobalValReplacements;

  /// Variables for which we've emitted globals containing their constant
  /// values along with the corresponding globals, for opportunistic reuse.
  toolchain::DenseMap<const VarDecl*, toolchain::GlobalVariable*> InitializerConstants;

  /// Set of global decls for which we already diagnosed mangled name conflict.
  /// Required to not issue a warning (on a mangling conflict) multiple times
  /// for the same decl.
  toolchain::DenseSet<GlobalDecl> DiagnosedConflictingDefinitions;

  /// A queue of (optional) vtables to consider emitting.
  std::vector<const CXXRecordDecl*> DeferredVTables;

  /// A queue of (optional) vtables that may be emitted opportunistically.
  std::vector<const CXXRecordDecl *> OpportunisticVTables;

  /// List of global values which are required to be present in the object file;
  /// bitcast to i8*. This is used for forcing visibility of symbols which may
  /// otherwise be optimized out.
  std::vector<toolchain::WeakTrackingVH> LLVMUsed;
  std::vector<toolchain::WeakTrackingVH> LLVMCompilerUsed;

  /// Store the list of global constructors and their respective priorities to
  /// be emitted when the translation unit is complete.
  CtorList GlobalCtors;

  /// Store the list of global destructors and their respective priorities to be
  /// emitted when the translation unit is complete.
  CtorList GlobalDtors;

  /// An ordered map of canonical GlobalDecls to their mangled names.
  toolchain::MapVector<GlobalDecl, StringRef> MangledDeclNames;
  toolchain::StringMap<GlobalDecl, toolchain::BumpPtrAllocator> Manglings;

  /// Global annotations.
  std::vector<toolchain::Constant*> Annotations;

  // Store deferred function annotations so they can be emitted at the end with
  // most up to date ValueDecl that will have all the inherited annotations.
  toolchain::MapVector<StringRef, const ValueDecl *> DeferredAnnotations;

  /// Map used to get unique annotation strings.
  toolchain::StringMap<toolchain::Constant*> AnnotationStrings;

  /// Used for uniquing of annotation arguments.
  toolchain::DenseMap<unsigned, toolchain::Constant *> AnnotationArgs;

  toolchain::StringMap<toolchain::GlobalVariable *> CFConstantStringMap;

  toolchain::DenseMap<toolchain::Constant *, toolchain::GlobalVariable *> ConstantStringMap;
  toolchain::DenseMap<const UnnamedGlobalConstantDecl *, toolchain::GlobalVariable *>
      UnnamedGlobalConstantDeclMap;
  toolchain::DenseMap<const Decl*, toolchain::Constant *> StaticLocalDeclMap;
  toolchain::DenseMap<const Decl*, toolchain::GlobalVariable*> StaticLocalDeclGuardMap;
  toolchain::DenseMap<const Expr*, toolchain::Constant *> MaterializedGlobalTemporaryMap;

  toolchain::DenseMap<QualType, toolchain::Constant *> AtomicSetterHelperFnMap;
  toolchain::DenseMap<QualType, toolchain::Constant *> AtomicGetterHelperFnMap;

  /// Map used to get unique type descriptor constants for sanitizers.
  toolchain::DenseMap<QualType, toolchain::Constant *> TypeDescriptorMap;

  /// Map used to track internal linkage functions declared within
  /// extern "C" regions.
  typedef toolchain::MapVector<IdentifierInfo *,
                          toolchain::GlobalValue *> StaticExternCMap;
  StaticExternCMap StaticExternCValues;

  /// thread_local variables defined or used in this TU.
  std::vector<const VarDecl *> CXXThreadLocals;

  /// thread_local variables with initializers that need to run
  /// before any thread_local variable in this TU is odr-used.
  std::vector<toolchain::Function *> CXXThreadLocalInits;
  std::vector<const VarDecl *> CXXThreadLocalInitVars;

  /// Global variables with initializers that need to run before main.
  std::vector<toolchain::Function *> CXXGlobalInits;

  /// When a C++ decl with an initializer is deferred, null is
  /// appended to CXXGlobalInits, and the index of that null is placed
  /// here so that the initializer will be performed in the correct
  /// order. Once the decl is emitted, the index is replaced with ~0U to ensure
  /// that we don't re-emit the initializer.
  toolchain::DenseMap<const Decl*, unsigned> DelayedCXXInitPosition;

  typedef std::pair<OrderGlobalInitsOrStermFinalizers, toolchain::Function *>
      GlobalInitData;

  // When a tail call is performed on an "undefined" symbol, on PPC without pc
  // relative feature, the tail call is not allowed. In "EmitCall" for such
  // tail calls, the "undefined" symbols may be forward declarations, their
  // definitions are provided in the module after the callsites. For such tail
  // calls, diagnose message should not be emitted.
  toolchain::SmallSetVector<std::pair<const FunctionDecl *, SourceLocation>, 4>
      MustTailCallUndefinedGlobals;

  struct GlobalInitPriorityCmp {
    bool operator()(const GlobalInitData &LHS,
                    const GlobalInitData &RHS) const {
      return LHS.first.priority < RHS.first.priority;
    }
  };

  /// Global variables with initializers whose order of initialization is set by
  /// init_priority attribute.
  SmallVector<GlobalInitData, 8> PrioritizedCXXGlobalInits;

  /// Global destructor functions and arguments that need to run on termination.
  /// When UseSinitAndSterm is set, it instead contains sterm finalizer
  /// functions, which also run on unloading a shared library.
  typedef std::tuple<toolchain::FunctionType *, toolchain::WeakTrackingVH,
                     toolchain::Constant *>
      CXXGlobalDtorsOrStermFinalizer_t;
  SmallVector<CXXGlobalDtorsOrStermFinalizer_t, 8>
      CXXGlobalDtorsOrStermFinalizers;

  typedef std::pair<OrderGlobalInitsOrStermFinalizers, toolchain::Function *>
      StermFinalizerData;

  struct StermFinalizerPriorityCmp {
    bool operator()(const StermFinalizerData &LHS,
                    const StermFinalizerData &RHS) const {
      return LHS.first.priority < RHS.first.priority;
    }
  };

  /// Global variables with sterm finalizers whose order of initialization is
  /// set by init_priority attribute.
  SmallVector<StermFinalizerData, 8> PrioritizedCXXStermFinalizers;

  /// The complete set of modules that has been imported.
  toolchain::SetVector<language::Core::Module *> ImportedModules;

  /// The set of modules for which the module initializers
  /// have been emitted.
  toolchain::SmallPtrSet<language::Core::Module *, 16> EmittedModuleInitializers;

  /// A vector of metadata strings for linker options.
  SmallVector<toolchain::MDNode *, 16> LinkerOptionsMetadata;

  /// A vector of metadata strings for dependent libraries for ELF.
  SmallVector<toolchain::MDNode *, 16> ELFDependentLibraries;

  /// @name Cache for Objective-C runtime types
  /// @{

  /// Cached reference to the class for constant strings. This value has type
  /// int * but is actually an Obj-C class pointer.
  toolchain::WeakTrackingVH CFConstantStringClassRef;

  /// The type used to describe the state of a fast enumeration in
  /// Objective-C's for..in loop.
  QualType ObjCFastEnumerationStateType;

  /// @}

  /// Lazily create the Objective-C runtime
  void createObjCRuntime();

  void createOpenCLRuntime();
  void createOpenMPRuntime();
  void createCUDARuntime();
  void createHLSLRuntime();

  bool isTriviallyRecursive(const FunctionDecl *F);
  bool shouldEmitFunction(GlobalDecl GD);
  // Whether a global variable should be emitted by CUDA/HIP host/device
  // related attributes.
  bool shouldEmitCUDAGlobalVar(const VarDecl *VD) const;
  bool shouldOpportunisticallyEmitVTables();
  /// Map used to be sure we don't emit the same CompoundLiteral twice.
  toolchain::DenseMap<const CompoundLiteralExpr *, toolchain::GlobalVariable *>
      EmittedCompoundLiterals;

  /// Map of the global blocks we've emitted, so that we don't have to re-emit
  /// them if the constexpr evaluator gets aggressive.
  toolchain::DenseMap<const BlockExpr *, toolchain::Constant *> EmittedGlobalBlocks;

  /// @name Cache for Blocks Runtime Globals
  /// @{

  toolchain::Constant *NSConcreteGlobalBlock = nullptr;
  toolchain::Constant *NSConcreteStackBlock = nullptr;

  toolchain::FunctionCallee BlockObjectAssign = nullptr;
  toolchain::FunctionCallee BlockObjectDispose = nullptr;

  toolchain::Type *BlockDescriptorType = nullptr;
  toolchain::Type *GenericBlockLiteralType = nullptr;

  struct {
    int GlobalUniqueCount;
  } Block;

  GlobalDecl initializedGlobalDecl;

  /// @}

  /// void @toolchain.lifetime.start(i64 %size, i8* nocapture <ptr>)
  toolchain::Function *LifetimeStartFn = nullptr;

  /// void @toolchain.lifetime.end(i64 %size, i8* nocapture <ptr>)
  toolchain::Function *LifetimeEndFn = nullptr;

  /// void @toolchain.fake.use(...)
  toolchain::Function *FakeUseFn = nullptr;

  std::unique_ptr<SanitizerMetadata> SanitizerMD;

  toolchain::MapVector<const Decl *, bool> DeferredEmptyCoverageMappingDecls;

  std::unique_ptr<CoverageMappingModuleGen> CoverageMapping;

  /// Mapping from canonical types to their metadata identifiers. We need to
  /// maintain this mapping because identifiers may be formed from distinct
  /// MDNodes.
  typedef toolchain::DenseMap<QualType, toolchain::Metadata *> MetadataTypeMap;
  MetadataTypeMap MetadataIdMap;
  MetadataTypeMap VirtualMetadataIdMap;
  MetadataTypeMap GeneralizedMetadataIdMap;

  // Helps squashing blocks of TopLevelStmtDecl into a single toolchain::Function
  // when used with -fincremental-extensions.
  std::pair<std::unique_ptr<CodeGenFunction>, const TopLevelStmtDecl *>
      GlobalTopLevelStmtBlockInFlight;

  toolchain::DenseMap<GlobalDecl, uint16_t> PtrAuthDiscriminatorHashes;

  toolchain::DenseMap<const CXXRecordDecl *, std::optional<PointerAuthQualifier>>
      VTablePtrAuthInfos;
  std::optional<PointerAuthQualifier>
  computeVTPointerAuthentication(const CXXRecordDecl *ThisClass);

  AtomicOptions AtomicOpts;

  // A set of functions which should be hot-patched; see
  // -fms-hotpatch-functions-file (and -list). This will nearly always be empty.
  // The list is sorted for binary-searching.
  std::vector<std::string> MSHotPatchFunctions;

public:
  CodeGenModule(ASTContext &C, IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS,
                const HeaderSearchOptions &headersearchopts,
                const PreprocessorOptions &ppopts,
                const CodeGenOptions &CodeGenOpts, toolchain::Module &M,
                DiagnosticsEngine &Diags,
                CoverageSourceInfo *CoverageInfo = nullptr);

  ~CodeGenModule();

  void clear();

  /// Finalize LLVM code generation.
  void Release();

  /// Get the current Atomic options.
  AtomicOptions getAtomicOpts() { return AtomicOpts; }

  /// Set the current Atomic options.
  void setAtomicOpts(AtomicOptions AO) { AtomicOpts = AO; }

  /// Return true if we should emit location information for expressions.
  bool getExpressionLocationsEnabled() const;

  /// Return a reference to the configured Objective-C runtime.
  CGObjCRuntime &getObjCRuntime() {
    if (!ObjCRuntime) createObjCRuntime();
    return *ObjCRuntime;
  }

  /// Return true iff an Objective-C runtime has been configured.
  bool hasObjCRuntime() { return !!ObjCRuntime; }

  const std::string &getModuleNameHash() const { return ModuleNameHash; }

  /// Return a reference to the configured OpenCL runtime.
  CGOpenCLRuntime &getOpenCLRuntime() {
    assert(OpenCLRuntime != nullptr);
    return *OpenCLRuntime;
  }

  /// Return a reference to the configured OpenMP runtime.
  CGOpenMPRuntime &getOpenMPRuntime() {
    assert(OpenMPRuntime != nullptr);
    return *OpenMPRuntime;
  }

  /// Return a reference to the configured CUDA runtime.
  CGCUDARuntime &getCUDARuntime() {
    assert(CUDARuntime != nullptr);
    return *CUDARuntime;
  }

  /// Return a reference to the configured HLSL runtime.
  CGHLSLRuntime &getHLSLRuntime() {
    assert(HLSLRuntime != nullptr);
    return *HLSLRuntime;
  }

  ObjCEntrypoints &getObjCEntrypoints() const {
    assert(ObjCData != nullptr);
    return *ObjCData;
  }

  // Version checking functions, used to implement ObjC's @available:
  // i32 @__isOSVersionAtLeast(i32, i32, i32)
  toolchain::FunctionCallee IsOSVersionAtLeastFn = nullptr;
  // i32 @__isPlatformVersionAtLeast(i32, i32, i32, i32)
  toolchain::FunctionCallee IsPlatformVersionAtLeastFn = nullptr;

  InstrProfStats &getPGOStats() { return PGOStats; }
  toolchain::IndexedInstrProfReader *getPGOReader() const { return PGOReader.get(); }

  CoverageMappingModuleGen *getCoverageMapping() const {
    return CoverageMapping.get();
  }

  toolchain::Constant *getStaticLocalDeclAddress(const VarDecl *D) {
    return StaticLocalDeclMap[D];
  }
  void setStaticLocalDeclAddress(const VarDecl *D,
                                 toolchain::Constant *C) {
    StaticLocalDeclMap[D] = C;
  }

  toolchain::Constant *
  getOrCreateStaticVarDecl(const VarDecl &D,
                           toolchain::GlobalValue::LinkageTypes Linkage);

  toolchain::GlobalVariable *getStaticLocalDeclGuardAddress(const VarDecl *D) {
    return StaticLocalDeclGuardMap[D];
  }
  void setStaticLocalDeclGuardAddress(const VarDecl *D,
                                      toolchain::GlobalVariable *C) {
    StaticLocalDeclGuardMap[D] = C;
  }

  Address createUnnamedGlobalFrom(const VarDecl &D, toolchain::Constant *Constant,
                                  CharUnits Align);

  bool lookupRepresentativeDecl(StringRef MangledName,
                                GlobalDecl &Result) const;

  toolchain::Constant *getAtomicSetterHelperFnMap(QualType Ty) {
    return AtomicSetterHelperFnMap[Ty];
  }
  void setAtomicSetterHelperFnMap(QualType Ty,
                            toolchain::Constant *Fn) {
    AtomicSetterHelperFnMap[Ty] = Fn;
  }

  toolchain::Constant *getAtomicGetterHelperFnMap(QualType Ty) {
    return AtomicGetterHelperFnMap[Ty];
  }
  void setAtomicGetterHelperFnMap(QualType Ty,
                            toolchain::Constant *Fn) {
    AtomicGetterHelperFnMap[Ty] = Fn;
  }

  toolchain::Constant *getTypeDescriptorFromMap(QualType Ty) {
    return TypeDescriptorMap[Ty];
  }
  void setTypeDescriptorInMap(QualType Ty, toolchain::Constant *C) {
    TypeDescriptorMap[Ty] = C;
  }

  CGDebugInfo *getModuleDebugInfo() { return DebugInfo.get(); }

  toolchain::MDNode *getNoObjCARCExceptionsMetadata() {
    if (!NoObjCARCExceptionsMetadata)
      NoObjCARCExceptionsMetadata = toolchain::MDNode::get(getLLVMContext(), {});
    return NoObjCARCExceptionsMetadata;
  }

  ASTContext &getContext() const { return Context; }
  const LangOptions &getLangOpts() const { return LangOpts; }
  const IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &getFileSystem() const {
    return FS;
  }
  const HeaderSearchOptions &getHeaderSearchOpts()
    const { return HeaderSearchOpts; }
  const PreprocessorOptions &getPreprocessorOpts()
    const { return PreprocessorOpts; }
  const CodeGenOptions &getCodeGenOpts() const { return CodeGenOpts; }
  toolchain::Module &getModule() const { return TheModule; }
  DiagnosticsEngine &getDiags() const { return Diags; }
  const toolchain::DataLayout &getDataLayout() const {
    return TheModule.getDataLayout();
  }
  const TargetInfo &getTarget() const { return Target; }
  const toolchain::Triple &getTriple() const { return Target.getTriple(); }
  bool supportsCOMDAT() const;
  void maybeSetTrivialComdat(const Decl &D, toolchain::GlobalObject &GO);

  const ABIInfo &getABIInfo();
  CGCXXABI &getCXXABI() const { return *ABI; }
  toolchain::LLVMContext &getLLVMContext() { return VMContext; }

  bool shouldUseTBAA() const { return TBAA != nullptr; }

  const TargetCodeGenInfo &getTargetCodeGenInfo();

  CodeGenTypes &getTypes() { return *Types; }

  CodeGenVTables &getVTables() { return VTables; }

  ItaniumVTableContext &getItaniumVTableContext() {
    return VTables.getItaniumVTableContext();
  }

  const ItaniumVTableContext &getItaniumVTableContext() const {
    return VTables.getItaniumVTableContext();
  }

  MicrosoftVTableContext &getMicrosoftVTableContext() {
    return VTables.getMicrosoftVTableContext();
  }

  CtorList &getGlobalCtors() { return GlobalCtors; }
  CtorList &getGlobalDtors() { return GlobalDtors; }

  /// getTBAATypeInfo - Get metadata used to describe accesses to objects of
  /// the given type.
  toolchain::MDNode *getTBAATypeInfo(QualType QTy);

  /// getTBAAAccessInfo - Get TBAA information that describes an access to
  /// an object of the given type.
  TBAAAccessInfo getTBAAAccessInfo(QualType AccessType);

  /// getTBAAVTablePtrAccessInfo - Get the TBAA information that describes an
  /// access to a virtual table pointer.
  TBAAAccessInfo getTBAAVTablePtrAccessInfo(toolchain::Type *VTablePtrType);

  toolchain::MDNode *getTBAAStructInfo(QualType QTy);

  /// getTBAABaseTypeInfo - Get metadata that describes the given base access
  /// type. Return null if the type is not suitable for use in TBAA access tags.
  toolchain::MDNode *getTBAABaseTypeInfo(QualType QTy);

  /// getTBAAAccessTagInfo - Get TBAA tag for a given memory access.
  toolchain::MDNode *getTBAAAccessTagInfo(TBAAAccessInfo Info);

  /// mergeTBAAInfoForCast - Get merged TBAA information for the purposes of
  /// type casts.
  TBAAAccessInfo mergeTBAAInfoForCast(TBAAAccessInfo SourceInfo,
                                      TBAAAccessInfo TargetInfo);

  /// mergeTBAAInfoForConditionalOperator - Get merged TBAA information for the
  /// purposes of conditional operator.
  TBAAAccessInfo mergeTBAAInfoForConditionalOperator(TBAAAccessInfo InfoA,
                                                     TBAAAccessInfo InfoB);

  /// mergeTBAAInfoForMemoryTransfer - Get merged TBAA information for the
  /// purposes of memory transfer calls.
  TBAAAccessInfo mergeTBAAInfoForMemoryTransfer(TBAAAccessInfo DestInfo,
                                                TBAAAccessInfo SrcInfo);

  /// getTBAAInfoForSubobject - Get TBAA information for an access with a given
  /// base lvalue.
  TBAAAccessInfo getTBAAInfoForSubobject(LValue Base, QualType AccessType) {
    if (Base.getTBAAInfo().isMayAlias())
      return TBAAAccessInfo::getMayAliasInfo();
    return getTBAAAccessInfo(AccessType);
  }

  bool isPaddedAtomicType(QualType type);
  bool isPaddedAtomicType(const AtomicType *type);

  /// DecorateInstructionWithTBAA - Decorate the instruction with a TBAA tag.
  void DecorateInstructionWithTBAA(toolchain::Instruction *Inst,
                                   TBAAAccessInfo TBAAInfo);

  /// Adds !invariant.barrier !tag to instruction
  void DecorateInstructionWithInvariantGroup(toolchain::Instruction *I,
                                             const CXXRecordDecl *RD);

  /// Emit the given number of characters as a value of type size_t.
  toolchain::ConstantInt *getSize(CharUnits numChars);

  /// Set the visibility for the given LLVM GlobalValue.
  void setGlobalVisibility(toolchain::GlobalValue *GV, const NamedDecl *D) const;

  void setDSOLocal(toolchain::GlobalValue *GV) const;

  bool shouldMapVisibilityToDLLExport(const NamedDecl *D) const {
    return getLangOpts().hasDefaultVisibilityExportMapping() && D &&
           (D->getLinkageAndVisibility().getVisibility() ==
            DefaultVisibility) &&
           (getLangOpts().isAllDefaultVisibilityExportMapping() ||
            (getLangOpts().isExplicitDefaultVisibilityExportMapping() &&
             D->getLinkageAndVisibility().isVisibilityExplicit()));
  }
  void setDLLImportDLLExport(toolchain::GlobalValue *GV, GlobalDecl D) const;
  void setDLLImportDLLExport(toolchain::GlobalValue *GV, const NamedDecl *D) const;
  /// Set visibility, dllimport/dllexport and dso_local.
  /// This must be called after dllimport/dllexport is set.
  void setGVProperties(toolchain::GlobalValue *GV, GlobalDecl GD) const;
  void setGVProperties(toolchain::GlobalValue *GV, const NamedDecl *D) const;

  void setGVPropertiesAux(toolchain::GlobalValue *GV, const NamedDecl *D) const;

  /// Set the TLS mode for the given LLVM GlobalValue for the thread-local
  /// variable declaration D.
  void setTLSMode(toolchain::GlobalValue *GV, const VarDecl &D) const;

  /// Get LLVM TLS mode from CodeGenOptions.
  toolchain::GlobalVariable::ThreadLocalMode GetDefaultLLVMTLSModel() const;

  static toolchain::GlobalValue::VisibilityTypes GetLLVMVisibility(Visibility V) {
    switch (V) {
    case DefaultVisibility:   return toolchain::GlobalValue::DefaultVisibility;
    case HiddenVisibility:    return toolchain::GlobalValue::HiddenVisibility;
    case ProtectedVisibility: return toolchain::GlobalValue::ProtectedVisibility;
    }
    toolchain_unreachable("unknown visibility!");
  }

  toolchain::Constant *GetAddrOfGlobal(GlobalDecl GD,
                                  ForDefinition_t IsForDefinition
                                    = NotForDefinition);

  /// Will return a global variable of the given type. If a variable with a
  /// different type already exists then a new  variable with the right type
  /// will be created and all uses of the old variable will be replaced with a
  /// bitcast to the new variable.
  toolchain::GlobalVariable *
  CreateOrReplaceCXXRuntimeVariable(StringRef Name, toolchain::Type *Ty,
                                    toolchain::GlobalValue::LinkageTypes Linkage,
                                    toolchain::Align Alignment);

  toolchain::Function *CreateGlobalInitOrCleanUpFunction(
      toolchain::FunctionType *ty, const Twine &name, const CGFunctionInfo &FI,
      SourceLocation Loc = SourceLocation(), bool TLS = false,
      toolchain::GlobalVariable::LinkageTypes Linkage =
          toolchain::GlobalVariable::InternalLinkage);

  /// Return the AST address space of the underlying global variable for D, as
  /// determined by its declaration. Normally this is the same as the address
  /// space of D's type, but in CUDA, address spaces are associated with
  /// declarations, not types. If D is nullptr, return the default address
  /// space for global variable.
  ///
  /// For languages without explicit address spaces, if D has default address
  /// space, target-specific global or constant address space may be returned.
  LangAS GetGlobalVarAddressSpace(const VarDecl *D);

  /// Return the AST address space of constant literal, which is used to emit
  /// the constant literal as global variable in LLVM IR.
  /// Note: This is not necessarily the address space of the constant literal
  /// in AST. For address space agnostic language, e.g. C++, constant literal
  /// in AST is always in default address space.
  LangAS GetGlobalConstantAddressSpace() const;

  /// Return the toolchain::Constant for the address of the given global variable.
  /// If Ty is non-null and if the global doesn't exist, then it will be created
  /// with the specified type instead of whatever the normal requested type
  /// would be. If IsForDefinition is true, it is guaranteed that an actual
  /// global with type Ty will be returned, not conversion of a variable with
  /// the same mangled name but some other type.
  toolchain::Constant *GetAddrOfGlobalVar(const VarDecl *D,
                                     toolchain::Type *Ty = nullptr,
                                     ForDefinition_t IsForDefinition
                                       = NotForDefinition);

  /// Return the address of the given function. If Ty is non-null, then this
  /// function will use the specified type if it has to create it.
  toolchain::Constant *GetAddrOfFunction(GlobalDecl GD, toolchain::Type *Ty = nullptr,
                                    bool ForVTable = false,
                                    bool DontDefer = false,
                                    ForDefinition_t IsForDefinition
                                      = NotForDefinition);

  // Return the function body address of the given function.
  toolchain::Constant *GetFunctionStart(const ValueDecl *Decl);

  /// Return a function pointer for a reference to the given function.
  /// This correctly handles weak references, but does not apply a
  /// pointer signature.
  toolchain::Constant *getRawFunctionPointer(GlobalDecl GD,
                                        toolchain::Type *Ty = nullptr);

  /// Return the ABI-correct function pointer value for a reference
  /// to the given function.  This will apply a pointer signature if
  /// necessary, caching the result for the given function.
  toolchain::Constant *getFunctionPointer(GlobalDecl GD, toolchain::Type *Ty = nullptr);

  /// Return the ABI-correct function pointer value for a reference
  /// to the given function.  This will apply a pointer signature if
  /// necessary.
  toolchain::Constant *getFunctionPointer(toolchain::Constant *Pointer,
                                     QualType FunctionType);

  toolchain::Constant *getMemberFunctionPointer(const FunctionDecl *FD,
                                           toolchain::Type *Ty = nullptr);

  toolchain::Constant *getMemberFunctionPointer(toolchain::Constant *Pointer,
                                           QualType FT);

  CGPointerAuthInfo getFunctionPointerAuthInfo(QualType T);

  CGPointerAuthInfo getMemberFunctionPointerAuthInfo(QualType FT);

  CGPointerAuthInfo getPointerAuthInfoForPointeeType(QualType type);

  CGPointerAuthInfo getPointerAuthInfoForType(QualType type);

  bool shouldSignPointer(const PointerAuthSchema &Schema);
  toolchain::Constant *getConstantSignedPointer(toolchain::Constant *Pointer,
                                           const PointerAuthSchema &Schema,
                                           toolchain::Constant *StorageAddress,
                                           GlobalDecl SchemaDecl,
                                           QualType SchemaType);

  toolchain::Constant *
  getConstantSignedPointer(toolchain::Constant *Pointer, unsigned Key,
                           toolchain::Constant *StorageAddress,
                           toolchain::ConstantInt *OtherDiscriminator);

  toolchain::ConstantInt *
  getPointerAuthOtherDiscriminator(const PointerAuthSchema &Schema,
                                   GlobalDecl SchemaDecl, QualType SchemaType);

  uint16_t getPointerAuthDeclDiscriminator(GlobalDecl GD);
  std::optional<CGPointerAuthInfo>
  getVTablePointerAuthInfo(CodeGenFunction *Context,
                           const CXXRecordDecl *Record,
                           toolchain::Value *StorageAddress);

  std::optional<PointerAuthQualifier>
  getVTablePointerAuthentication(const CXXRecordDecl *thisClass);

  CGPointerAuthInfo EmitPointerAuthInfo(const RecordDecl *RD);

  // Return whether RTTI information should be emitted for this target.
  bool shouldEmitRTTI(bool ForEH = false) {
    return (ForEH || getLangOpts().RTTI) &&
           (!getLangOpts().isTargetDevice() || !getTriple().isGPU());
  }

  /// Get the address of the RTTI descriptor for the given type.
  toolchain::Constant *GetAddrOfRTTIDescriptor(QualType Ty, bool ForEH = false);

  /// Get the address of a GUID.
  ConstantAddress GetAddrOfMSGuidDecl(const MSGuidDecl *GD);

  /// Get the address of a UnnamedGlobalConstant
  ConstantAddress
  GetAddrOfUnnamedGlobalConstantDecl(const UnnamedGlobalConstantDecl *GCD);

  /// Get the address of a template parameter object.
  ConstantAddress
  GetAddrOfTemplateParamObject(const TemplateParamObjectDecl *TPO);

  /// Get the address of the thunk for the given global decl.
  toolchain::Constant *GetAddrOfThunk(StringRef Name, toolchain::Type *FnTy,
                                 GlobalDecl GD);

  /// Get a reference to the target of VD.
  ConstantAddress GetWeakRefReference(const ValueDecl *VD);

  /// Returns the assumed alignment of an opaque pointer to the given class.
  CharUnits getClassPointerAlignment(const CXXRecordDecl *CD);

  /// Returns the minimum object size for an object of the given class type
  /// (or a class derived from it).
  CharUnits getMinimumClassObjectSize(const CXXRecordDecl *CD);

  /// Returns the minimum object size for an object of the given type.
  CharUnits getMinimumObjectSize(QualType Ty) {
    if (CXXRecordDecl *RD = Ty->getAsCXXRecordDecl())
      return getMinimumClassObjectSize(RD);
    return getContext().getTypeSizeInChars(Ty);
  }

  /// Returns the assumed alignment of a virtual base of a class.
  CharUnits getVBaseAlignment(CharUnits DerivedAlign,
                              const CXXRecordDecl *Derived,
                              const CXXRecordDecl *VBase);

  /// Given a class pointer with an actual known alignment, and the
  /// expected alignment of an object at a dynamic offset w.r.t that
  /// pointer, return the alignment to assume at the offset.
  CharUnits getDynamicOffsetAlignment(CharUnits ActualAlign,
                                      const CXXRecordDecl *Class,
                                      CharUnits ExpectedTargetAlign);

  CharUnits
  computeNonVirtualBaseClassOffset(const CXXRecordDecl *DerivedClass,
                                   CastExpr::path_const_iterator Start,
                                   CastExpr::path_const_iterator End);

  /// Returns the offset from a derived class to  a class. Returns null if the
  /// offset is 0.
  toolchain::Constant *
  GetNonVirtualBaseClassOffset(const CXXRecordDecl *ClassDecl,
                               CastExpr::path_const_iterator PathBegin,
                               CastExpr::path_const_iterator PathEnd);

  toolchain::FoldingSet<BlockByrefHelpers> ByrefHelpersCache;

  /// Fetches the global unique block count.
  int getUniqueBlockCount() { return ++Block.GlobalUniqueCount; }

  /// Fetches the type of a generic block descriptor.
  toolchain::Type *getBlockDescriptorType();

  /// The type of a generic block literal.
  toolchain::Type *getGenericBlockLiteralType();

  /// Gets the address of a block which requires no captures.
  toolchain::Constant *GetAddrOfGlobalBlock(const BlockExpr *BE, StringRef Name);

  /// Returns the address of a block which requires no caputres, or null if
  /// we've yet to emit the block for BE.
  toolchain::Constant *getAddrOfGlobalBlockIfEmitted(const BlockExpr *BE) {
    return EmittedGlobalBlocks.lookup(BE);
  }

  /// Notes that BE's global block is available via Addr. Asserts that BE
  /// isn't already emitted.
  void setAddrOfGlobalBlock(const BlockExpr *BE, toolchain::Constant *Addr);

  /// Return a pointer to a constant CFString object for the given string.
  ConstantAddress GetAddrOfConstantCFString(const StringLiteral *Literal);

  /// Return a constant array for the given string.
  toolchain::Constant *GetConstantArrayFromStringLiteral(const StringLiteral *E);

  /// Return a pointer to a constant array for the given string literal.
  ConstantAddress
  GetAddrOfConstantStringFromLiteral(const StringLiteral *S,
                                     StringRef Name = ".str");

  /// Return a pointer to a constant array for the given ObjCEncodeExpr node.
  ConstantAddress
  GetAddrOfConstantStringFromObjCEncode(const ObjCEncodeExpr *);

  /// Returns a pointer to a character array containing the literal and a
  /// terminating '\0' character. The result has pointer to array type.
  ///
  /// \param GlobalName If provided, the name to use for the global (if one is
  /// created).
  ConstantAddress
  GetAddrOfConstantCString(const std::string &Str,
                           const char *GlobalName = nullptr);

  /// Returns a pointer to a constant global variable for the given file-scope
  /// compound literal expression.
  ConstantAddress GetAddrOfConstantCompoundLiteral(const CompoundLiteralExpr*E);

  /// If it's been emitted already, returns the GlobalVariable corresponding to
  /// a compound literal. Otherwise, returns null.
  toolchain::GlobalVariable *
  getAddrOfConstantCompoundLiteralIfEmitted(const CompoundLiteralExpr *E);

  /// Notes that CLE's GlobalVariable is GV. Asserts that CLE isn't already
  /// emitted.
  void setAddrOfConstantCompoundLiteral(const CompoundLiteralExpr *CLE,
                                        toolchain::GlobalVariable *GV);

  /// Returns a pointer to a global variable representing a temporary
  /// with static or thread storage duration.
  ConstantAddress GetAddrOfGlobalTemporary(const MaterializeTemporaryExpr *E,
                                           const Expr *Inner);

  /// Retrieve the record type that describes the state of an
  /// Objective-C fast enumeration loop (for..in).
  QualType getObjCFastEnumerationStateType();

  // Produce code for this constructor/destructor. This method doesn't try
  // to apply any ABI rules about which other constructors/destructors
  // are needed or if they are alias to each other.
  toolchain::Function *codegenCXXStructor(GlobalDecl GD);

  /// Return the address of the constructor/destructor of the given type.
  toolchain::Constant *
  getAddrOfCXXStructor(GlobalDecl GD, const CGFunctionInfo *FnInfo = nullptr,
                       toolchain::FunctionType *FnType = nullptr,
                       bool DontDefer = false,
                       ForDefinition_t IsForDefinition = NotForDefinition) {
    return cast<toolchain::Constant>(getAddrAndTypeOfCXXStructor(GD, FnInfo, FnType,
                                                            DontDefer,
                                                            IsForDefinition)
                                    .getCallee());
  }

  toolchain::FunctionCallee getAddrAndTypeOfCXXStructor(
      GlobalDecl GD, const CGFunctionInfo *FnInfo = nullptr,
      toolchain::FunctionType *FnType = nullptr, bool DontDefer = false,
      ForDefinition_t IsForDefinition = NotForDefinition);

  /// Given a builtin id for a function like "__builtin_fabsf", return a
  /// Function* for "fabsf".
  toolchain::Constant *getBuiltinLibFunction(const FunctionDecl *FD,
                                        unsigned BuiltinID);

  toolchain::Function *getIntrinsic(unsigned IID, ArrayRef<toolchain::Type *> Tys = {});

  void AddCXXGlobalInit(toolchain::Function *F) { CXXGlobalInits.push_back(F); }

  /// Emit code for a single top level declaration.
  void EmitTopLevelDecl(Decl *D);

  /// Stored a deferred empty coverage mapping for an unused
  /// and thus uninstrumented top level declaration.
  void AddDeferredUnusedCoverageMapping(Decl *D);

  /// Remove the deferred empty coverage mapping as this
  /// declaration is actually instrumented.
  void ClearUnusedCoverageMapping(const Decl *D);

  /// Emit all the deferred coverage mappings
  /// for the uninstrumented functions.
  void EmitDeferredUnusedCoverageMappings();

  /// Emit an alias for "main" if it has no arguments (needed for wasm).
  void EmitMainVoidAlias();

  /// Tell the consumer that this variable has been instantiated.
  void HandleCXXStaticMemberVarInstantiation(VarDecl *VD);

  /// If the declaration has internal linkage but is inside an
  /// extern "C" linkage specification, prepare to emit an alias for it
  /// to the expected name.
  template<typename SomeDecl>
  void MaybeHandleStaticInExternC(const SomeDecl *D, toolchain::GlobalValue *GV);

  /// Add a global to a list to be added to the toolchain.used metadata.
  void addUsedGlobal(toolchain::GlobalValue *GV);

  /// Add a global to a list to be added to the toolchain.compiler.used metadata.
  void addCompilerUsedGlobal(toolchain::GlobalValue *GV);

  /// Add a global to a list to be added to the toolchain.compiler.used metadata.
  void addUsedOrCompilerUsedGlobal(toolchain::GlobalValue *GV);

  /// Add a destructor and object to add to the C++ global destructor function.
  void AddCXXDtorEntry(toolchain::FunctionCallee DtorFn, toolchain::Constant *Object) {
    CXXGlobalDtorsOrStermFinalizers.emplace_back(DtorFn.getFunctionType(),
                                                 DtorFn.getCallee(), Object);
  }

  /// Add an sterm finalizer to the C++ global cleanup function.
  void AddCXXStermFinalizerEntry(toolchain::FunctionCallee DtorFn) {
    CXXGlobalDtorsOrStermFinalizers.emplace_back(DtorFn.getFunctionType(),
                                                 DtorFn.getCallee(), nullptr);
  }

  /// Add an sterm finalizer to its own toolchain.global_dtors entry.
  void AddCXXStermFinalizerToGlobalDtor(toolchain::Function *StermFinalizer,
                                        int Priority) {
    AddGlobalDtor(StermFinalizer, Priority);
  }

  void AddCXXPrioritizedStermFinalizerEntry(toolchain::Function *StermFinalizer,
                                            int Priority) {
    OrderGlobalInitsOrStermFinalizers Key(Priority,
                                          PrioritizedCXXStermFinalizers.size());
    PrioritizedCXXStermFinalizers.push_back(
        std::make_pair(Key, StermFinalizer));
  }

  /// Create or return a runtime function declaration with the specified type
  /// and name. If \p AssumeConvergent is true, the call will have the
  /// convergent attribute added.
  ///
  /// For new code, please use the overload that takes a QualType; it sets
  /// function attributes more accurately.
  toolchain::FunctionCallee
  CreateRuntimeFunction(toolchain::FunctionType *Ty, StringRef Name,
                        toolchain::AttributeList ExtraAttrs = toolchain::AttributeList(),
                        bool Local = false, bool AssumeConvergent = false);

  /// Create or return a runtime function declaration with the specified type
  /// and name. If \p AssumeConvergent is true, the call will have the
  /// convergent attribute added.
  toolchain::FunctionCallee
  CreateRuntimeFunction(QualType ReturnTy, ArrayRef<QualType> ArgTys,
                        StringRef Name,
                        toolchain::AttributeList ExtraAttrs = toolchain::AttributeList(),
                        bool Local = false, bool AssumeConvergent = false);

  /// Create a new runtime global variable with the specified type and name.
  toolchain::Constant *CreateRuntimeVariable(toolchain::Type *Ty,
                                        StringRef Name);

  ///@name Custom Blocks Runtime Interfaces
  ///@{

  toolchain::Constant *getNSConcreteGlobalBlock();
  toolchain::Constant *getNSConcreteStackBlock();
  toolchain::FunctionCallee getBlockObjectAssign();
  toolchain::FunctionCallee getBlockObjectDispose();

  ///@}

  toolchain::Function *getLLVMLifetimeStartFn();
  toolchain::Function *getLLVMLifetimeEndFn();
  toolchain::Function *getLLVMFakeUseFn();

  // Make sure that this type is translated.
  void UpdateCompletedType(const TagDecl *TD);

  toolchain::Constant *getMemberPointerConstant(const UnaryOperator *e);

  /// Emit type info if type of an expression is a variably modified
  /// type. Also emit proper debug info for cast types.
  void EmitExplicitCastExprType(const ExplicitCastExpr *E,
                                CodeGenFunction *CGF = nullptr);

  /// Return the result of value-initializing the given type, i.e. a null
  /// expression of the given type.  This is usually, but not always, an LLVM
  /// null constant.
  toolchain::Constant *EmitNullConstant(QualType T);

  /// Return a null constant appropriate for zero-initializing a base class with
  /// the given type. This is usually, but not always, an LLVM null constant.
  toolchain::Constant *EmitNullConstantForBase(const CXXRecordDecl *Record);

  /// Emit a general error that something can't be done.
  void Error(SourceLocation loc, StringRef error);

  /// Print out an error that codegen doesn't support the specified stmt yet.
  void ErrorUnsupported(const Stmt *S, const char *Type);

  /// Print out an error that codegen doesn't support the specified decl yet.
  void ErrorUnsupported(const Decl *D, const char *Type);

  /// Run some code with "sufficient" stack space. (Currently, at least 256K is
  /// guaranteed). Produces a warning if we're low on stack space and allocates
  /// more in that case. Use this in code that may recurse deeply to avoid stack
  /// overflow.
  void runWithSufficientStackSpace(SourceLocation Loc,
                                   toolchain::function_ref<void()> Fn);

  /// Set the attributes on the LLVM function for the given decl and function
  /// info. This applies attributes necessary for handling the ABI as well as
  /// user specified attributes like section.
  void SetInternalFunctionAttributes(GlobalDecl GD, toolchain::Function *F,
                                     const CGFunctionInfo &FI);

  /// Set the LLVM function attributes (sext, zext, etc).
  void SetLLVMFunctionAttributes(GlobalDecl GD, const CGFunctionInfo &Info,
                                 toolchain::Function *F, bool IsThunk);

  /// Set the LLVM function attributes which only apply to a function
  /// definition.
  void SetLLVMFunctionAttributesForDefinition(const Decl *D, toolchain::Function *F);

  /// Set the LLVM function attributes that represent floating point
  /// environment.
  void setLLVMFunctionFEnvAttributes(const FunctionDecl *D, toolchain::Function *F);

  /// Return true iff the given type uses 'sret' when used as a return type.
  bool ReturnTypeUsesSRet(const CGFunctionInfo &FI);

  /// Return true iff the given type has `inreg` set.
  bool ReturnTypeHasInReg(const CGFunctionInfo &FI);

  /// Return true iff the given type uses an argument slot when 'sret' is used
  /// as a return type.
  bool ReturnSlotInterferesWithArgs(const CGFunctionInfo &FI);

  /// Return true iff the given type uses 'fpret' when used as a return type.
  bool ReturnTypeUsesFPRet(QualType ResultType);

  /// Return true iff the given type uses 'fp2ret' when used as a return type.
  bool ReturnTypeUsesFP2Ret(QualType ResultType);

  /// Get the LLVM attributes and calling convention to use for a particular
  /// function type.
  ///
  /// \param Name - The function name.
  /// \param Info - The function type information.
  /// \param CalleeInfo - The callee information these attributes are being
  /// constructed for. If valid, the attributes applied to this decl may
  /// contribute to the function attributes and calling convention.
  /// \param Attrs [out] - On return, the attribute list to use.
  /// \param CallingConv [out] - On return, the LLVM calling convention to use.
  void ConstructAttributeList(StringRef Name, const CGFunctionInfo &Info,
                              CGCalleeInfo CalleeInfo,
                              toolchain::AttributeList &Attrs, unsigned &CallingConv,
                              bool AttrOnCallSite, bool IsThunk);

  /// Adjust Memory attribute to ensure that the BE gets the right attribute
  // in order to generate the library call or the intrinsic for the function
  // name 'Name'.
  void AdjustMemoryAttribute(StringRef Name, CGCalleeInfo CalleeInfo,
                             toolchain::AttributeList &Attrs);

  /// Like the overload taking a `Function &`, but intended specifically
  /// for frontends that want to build on Clang's target-configuration logic.
  void addDefaultFunctionDefinitionAttributes(toolchain::AttrBuilder &attrs);

  StringRef getMangledName(GlobalDecl GD);
  StringRef getBlockMangledName(GlobalDecl GD, const BlockDecl *BD);
  const GlobalDecl getMangledNameDecl(StringRef);

  void EmitTentativeDefinition(const VarDecl *D);

  void EmitExternalDeclaration(const DeclaratorDecl *D);

  void EmitVTable(CXXRecordDecl *Class);

  void RefreshTypeCacheForClass(const CXXRecordDecl *Class);

  /// Appends Opts to the "toolchain.linker.options" metadata value.
  void AppendLinkerOptions(StringRef Opts);

  /// Appends a detect mismatch command to the linker options.
  void AddDetectMismatch(StringRef Name, StringRef Value);

  /// Appends a dependent lib to the appropriate metadata value.
  void AddDependentLib(StringRef Lib);


  toolchain::GlobalVariable::LinkageTypes getFunctionLinkage(GlobalDecl GD);

  void setFunctionLinkage(GlobalDecl GD, toolchain::Function *F) {
    F->setLinkage(getFunctionLinkage(GD));
  }

  /// Return the appropriate linkage for the vtable, VTT, and type information
  /// of the given class.
  toolchain::GlobalVariable::LinkageTypes getVTableLinkage(const CXXRecordDecl *RD);

  /// Return the store size, in character units, of the given LLVM type.
  CharUnits GetTargetTypeStoreSize(toolchain::Type *Ty) const;

  /// Returns LLVM linkage for a declarator.
  toolchain::GlobalValue::LinkageTypes
  getLLVMLinkageForDeclarator(const DeclaratorDecl *D, GVALinkage Linkage);

  /// Returns LLVM linkage for a declarator.
  toolchain::GlobalValue::LinkageTypes
  getLLVMLinkageVarDefinition(const VarDecl *VD);

  /// Emit all the global annotations.
  void EmitGlobalAnnotations();

  /// Emit an annotation string.
  toolchain::Constant *EmitAnnotationString(StringRef Str);

  /// Emit the annotation's translation unit.
  toolchain::Constant *EmitAnnotationUnit(SourceLocation Loc);

  /// Emit the annotation line number.
  toolchain::Constant *EmitAnnotationLineNo(SourceLocation L);

  /// Emit additional args of the annotation.
  toolchain::Constant *EmitAnnotationArgs(const AnnotateAttr *Attr);

  /// Generate the toolchain::ConstantStruct which contains the annotation
  /// information for a given GlobalValue. The annotation struct is
  /// {i8 *, i8 *, i8 *, i32}. The first field is a constant expression, the
  /// GlobalValue being annotated. The second field is the constant string
  /// created from the AnnotateAttr's annotation. The third field is a constant
  /// string containing the name of the translation unit. The fourth field is
  /// the line number in the file of the annotated value declaration.
  toolchain::Constant *EmitAnnotateAttr(toolchain::GlobalValue *GV,
                                   const AnnotateAttr *AA,
                                   SourceLocation L);

  /// Add global annotations that are set on D, for the global GV. Those
  /// annotations are emitted during finalization of the LLVM code.
  void AddGlobalAnnotations(const ValueDecl *D, toolchain::GlobalValue *GV);

  bool isInNoSanitizeList(SanitizerMask Kind, toolchain::Function *Fn,
                          SourceLocation Loc) const;

  bool isInNoSanitizeList(SanitizerMask Kind, toolchain::GlobalVariable *GV,
                          SourceLocation Loc, QualType Ty,
                          StringRef Category = StringRef()) const;

  /// Imbue XRay attributes to a function, applying the always/never attribute
  /// lists in the process. Returns true if we did imbue attributes this way,
  /// false otherwise.
  bool imbueXRayAttrs(toolchain::Function *Fn, SourceLocation Loc,
                      StringRef Category = StringRef()) const;

  /// \returns true if \p Fn at \p Loc should be excluded from profile
  /// instrumentation by the SCL passed by \p -fprofile-list.
  ProfileList::ExclusionType
  isFunctionBlockedByProfileList(toolchain::Function *Fn, SourceLocation Loc) const;

  /// \returns true if \p Fn at \p Loc should be excluded from profile
  /// instrumentation.
  ProfileList::ExclusionType
  isFunctionBlockedFromProfileInstr(toolchain::Function *Fn,
                                    SourceLocation Loc) const;

  SanitizerMetadata *getSanitizerMetadata() {
    return SanitizerMD.get();
  }

  void addDeferredVTable(const CXXRecordDecl *RD) {
    DeferredVTables.push_back(RD);
  }

  /// Emit code for a single global function or var decl. Forward declarations
  /// are emitted lazily.
  void EmitGlobal(GlobalDecl D);

  bool TryEmitBaseDestructorAsAlias(const CXXDestructorDecl *D);

  toolchain::GlobalValue *GetGlobalValue(StringRef Ref);

  /// Set attributes which are common to any form of a global definition (alias,
  /// Objective-C method, function, global variable).
  ///
  /// NOTE: This should only be called for definitions.
  void SetCommonAttributes(GlobalDecl GD, toolchain::GlobalValue *GV);

  void addReplacement(StringRef Name, toolchain::Constant *C);

  void addGlobalValReplacement(toolchain::GlobalValue *GV, toolchain::Constant *C);

  /// Emit a code for threadprivate directive.
  /// \param D Threadprivate declaration.
  void EmitOMPThreadPrivateDecl(const OMPThreadPrivateDecl *D);

  /// Emit a code for declare reduction construct.
  void EmitOMPDeclareReduction(const OMPDeclareReductionDecl *D,
                               CodeGenFunction *CGF = nullptr);

  /// Emit a code for declare mapper construct.
  void EmitOMPDeclareMapper(const OMPDeclareMapperDecl *D,
                            CodeGenFunction *CGF = nullptr);

  // Emit code for the OpenACC Declare declaration.
  void EmitOpenACCDeclare(const OpenACCDeclareDecl *D,
                          CodeGenFunction *CGF = nullptr);
  // Emit code for the OpenACC Routine declaration.
  void EmitOpenACCRoutine(const OpenACCRoutineDecl *D,
                          CodeGenFunction *CGF = nullptr);

  /// Emit a code for requires directive.
  /// \param D Requires declaration
  void EmitOMPRequiresDecl(const OMPRequiresDecl *D);

  /// Emit a code for the allocate directive.
  /// \param D The allocate declaration
  void EmitOMPAllocateDecl(const OMPAllocateDecl *D);

  /// Return the alignment specified in an allocate directive, if present.
  std::optional<CharUnits> getOMPAllocateAlignment(const VarDecl *VD);

  /// Returns whether the given record has hidden LTO visibility and therefore
  /// may participate in (single-module) CFI and whole-program vtable
  /// optimization.
  bool HasHiddenLTOVisibility(const CXXRecordDecl *RD);

  /// Returns whether the given record has public LTO visibility (regardless of
  /// -lto-whole-program-visibility) and therefore may not participate in
  /// (single-module) CFI and whole-program vtable optimization.
  bool AlwaysHasLTOVisibilityPublic(const CXXRecordDecl *RD);

  /// Returns the vcall visibility of the given type. This is the scope in which
  /// a virtual function call could be made which ends up being dispatched to a
  /// member function of this class. This scope can be wider than the visibility
  /// of the class itself when the class has a more-visible dynamic base class.
  /// The client should pass in an empty Visited set, which is used to prevent
  /// redundant recursive processing.
  toolchain::GlobalObject::VCallVisibility
  GetVCallVisibilityLevel(const CXXRecordDecl *RD,
                          toolchain::DenseSet<const CXXRecordDecl *> &Visited);

  /// Emit type metadata for the given vtable using the given layout.
  void EmitVTableTypeMetadata(const CXXRecordDecl *RD,
                              toolchain::GlobalVariable *VTable,
                              const VTableLayout &VTLayout);

  toolchain::Type *getVTableComponentType() const;

  /// Generate a cross-DSO type identifier for MD.
  toolchain::ConstantInt *CreateCrossDsoCfiTypeId(toolchain::Metadata *MD);

  /// Generate a KCFI type identifier for T.
  toolchain::ConstantInt *CreateKCFITypeId(QualType T, StringRef Salt);

  /// Create a metadata identifier for the given type. This may either be an
  /// MDString (for external identifiers) or a distinct unnamed MDNode (for
  /// internal identifiers).
  toolchain::Metadata *CreateMetadataIdentifierForType(QualType T);

  /// Create a metadata identifier that is intended to be used to check virtual
  /// calls via a member function pointer.
  toolchain::Metadata *CreateMetadataIdentifierForVirtualMemPtrType(QualType T);

  /// Create a metadata identifier for the generalization of the given type.
  /// This may either be an MDString (for external identifiers) or a distinct
  /// unnamed MDNode (for internal identifiers).
  toolchain::Metadata *CreateMetadataIdentifierGeneralized(QualType T);

  /// Create and attach type metadata to the given function.
  void createFunctionTypeMetadataForIcall(const FunctionDecl *FD,
                                          toolchain::Function *F);

  /// Set type metadata to the given function.
  void setKCFIType(const FunctionDecl *FD, toolchain::Function *F);

  /// Emit KCFI type identifier constants and remove unused identifiers.
  void finalizeKCFITypes();

  /// Whether this function's return type has no side effects, and thus may
  /// be trivially discarded if it is unused.
  bool MayDropFunctionReturn(const ASTContext &Context,
                             QualType ReturnType) const;

  /// Returns whether this module needs the "all-vtables" type identifier.
  bool NeedAllVtablesTypeId() const;

  /// Create and attach type metadata for the given vtable.
  void AddVTableTypeMetadata(toolchain::GlobalVariable *VTable, CharUnits Offset,
                             const CXXRecordDecl *RD);

  /// Return a vector of most-base classes for RD. This is used to implement
  /// control flow integrity checks for member function pointers.
  ///
  /// A most-base class of a class C is defined as a recursive base class of C,
  /// including C itself, that does not have any bases.
  SmallVector<const CXXRecordDecl *, 0>
  getMostBaseClasses(const CXXRecordDecl *RD);

  /// Get the declaration of std::terminate for the platform.
  toolchain::FunctionCallee getTerminateFn();

  toolchain::SanitizerStatReport &getSanStats();

  toolchain::Value *
  createOpenCLIntToSamplerConversion(const Expr *E, CodeGenFunction &CGF);

  /// OpenCL v1.2 s5.6.4.6 allows the compiler to store kernel argument
  /// information in the program executable. The argument information stored
  /// includes the argument name, its type, the address and access qualifiers
  /// used. This helper can be used to generate metadata for source code kernel
  /// function as well as generated implicitly kernels. If a kernel is generated
  /// implicitly null value has to be passed to the last two parameters,
  /// otherwise all parameters must have valid non-null values.
  /// \param FN is a pointer to IR function being generated.
  /// \param FD is a pointer to function declaration if any.
  /// \param CGF is a pointer to CodeGenFunction that generates this function.
  void GenKernelArgMetadata(toolchain::Function *FN,
                            const FunctionDecl *FD = nullptr,
                            CodeGenFunction *CGF = nullptr);

  /// Get target specific null pointer.
  /// \param T is the LLVM type of the null pointer.
  /// \param QT is the clang QualType of the null pointer.
  toolchain::Constant *getNullPointer(toolchain::PointerType *T, QualType QT);

  CharUnits getNaturalTypeAlignment(QualType T,
                                    LValueBaseInfo *BaseInfo = nullptr,
                                    TBAAAccessInfo *TBAAInfo = nullptr,
                                    bool forPointeeType = false);
  CharUnits getNaturalPointeeTypeAlignment(QualType T,
                                           LValueBaseInfo *BaseInfo = nullptr,
                                           TBAAAccessInfo *TBAAInfo = nullptr);
  bool stopAutoInit();

  /// Print the postfix for externalized static variable or kernels for single
  /// source offloading languages CUDA and HIP. The unique postfix is created
  /// using either the CUID argument, or the file's UniqueID and active macros.
  /// The fallback method without a CUID requires that the offloading toolchain
  /// does not define separate macros via the -cc1 options.
  void printPostfixForExternalizedDecl(toolchain::raw_ostream &OS,
                                       const Decl *D) const;

  /// Move some lazily-emitted states to the NewBuilder. This is especially
  /// essential for the incremental parsing environment like Clang Interpreter,
  /// because we'll lose all important information after each repl.
  void moveLazyEmissionStates(CodeGenModule *NewBuilder);

  /// Emit the IR encoding to attach the CUDA launch bounds attribute to \p F.
  /// If \p MaxThreadsVal is not nullptr, the max threads value is stored in it,
  /// if a valid one was found.
  void handleCUDALaunchBoundsAttr(toolchain::Function *F,
                                  const CUDALaunchBoundsAttr *A,
                                  int32_t *MaxThreadsVal = nullptr,
                                  int32_t *MinBlocksVal = nullptr,
                                  int32_t *MaxClusterRankVal = nullptr);

  /// Emit the IR encoding to attach the AMD GPU flat-work-group-size attribute
  /// to \p F. Alternatively, the work group size can be taken from a \p
  /// ReqdWGS. If \p MinThreadsVal is not nullptr, the min threads value is
  /// stored in it, if a valid one was found. If \p MaxThreadsVal is not
  /// nullptr, the max threads value is stored in it, if a valid one was found.
  void handleAMDGPUFlatWorkGroupSizeAttr(
      toolchain::Function *F, const AMDGPUFlatWorkGroupSizeAttr *A,
      const ReqdWorkGroupSizeAttr *ReqdWGS = nullptr,
      int32_t *MinThreadsVal = nullptr, int32_t *MaxThreadsVal = nullptr);

  /// Emit the IR encoding to attach the AMD GPU waves-per-eu attribute to \p F.
  void handleAMDGPUWavesPerEUAttr(toolchain::Function *F,
                                  const AMDGPUWavesPerEUAttr *A);

  toolchain::Constant *
  GetOrCreateLLVMGlobal(StringRef MangledName, toolchain::Type *Ty, LangAS AddrSpace,
                        const VarDecl *D,
                        ForDefinition_t IsForDefinition = NotForDefinition);

  // FIXME: Hardcoding priority here is gross.
  void AddGlobalCtor(toolchain::Function *Ctor, int Priority = 65535,
                     unsigned LexOrder = ~0U,
                     toolchain::Constant *AssociatedData = nullptr);
  void AddGlobalDtor(toolchain::Function *Dtor, int Priority = 65535,
                     bool IsDtorAttrFunc = false);

  // Return whether structured convergence intrinsics should be generated for
  // this target.
  bool shouldEmitConvergenceTokens() const {
    // TODO: this should probably become unconditional once the controlled
    // convergence becomes the norm.
    return getTriple().isSPIRVLogical();
  }

  void addUndefinedGlobalForTailCall(
      std::pair<const FunctionDecl *, SourceLocation> Global) {
    MustTailCallUndefinedGlobals.insert(Global);
  }

  bool shouldZeroInitPadding() const {
    // In C23 (N3096) $6.7.10:
    // """
    // If any object is initialized with an empty iniitializer, then it is
    // subject to default initialization:
    //  - if it is an aggregate, every member is initialized (recursively)
    //  according to these rules, and any padding is initialized to zero bits;
    //  - if it is a union, the first named member is initialized (recursively)
    //  according to these rules, and any padding is initialized to zero bits.
    //
    // If the aggregate or union contains elements or members that are
    // aggregates or unions, these rules apply recursively to the subaggregates
    // or contained unions.
    //
    // If there are fewer initializers in a brace-enclosed list than there are
    // elements or members of an aggregate, or fewer characters in a string
    // literal used to initialize an array of known size than there are elements
    // in the array, the remainder of the aggregate is subject to default
    // initialization.
    // """
    //
    // From my understanding, the standard is ambiguous in the following two
    // areas:
    // 1. For a union type with empty initializer, if the first named member is
    // not the largest member, then the bytes comes after the first named member
    // but before padding are left unspecified. An example is:
    //    union U { int a; long long b;};
    //    union U u = {};  // The first 4 bytes are 0, but 4-8 bytes are left
    //    unspecified.
    //
    // 2. It only mentions padding for empty initializer, but doesn't mention
    // padding for a non empty initialization list. And if the aggregation or
    // union contains elements or members that are aggregates or unions, and
    // some are non empty initializers, while others are empty initiailizers,
    // the padding initialization is unclear. An example is:
    //    struct S1 { int a; long long b; };
    //    struct S2 { char c; struct S1 s1; };
    //    // The values for paddings between s2.c and s2.s1.a, between s2.s1.a
    //    and s2.s1.b are unclear.
    //    struct S2 s2 = { 'c' };
    //
    // Here we choose to zero initiailize left bytes of a union type. Because
    // projects like the Linux kernel are relying on this behavior. If we don't
    // explicitly zero initialize them, the undef values can be optimized to
    // return gabage data. We also choose to zero initialize paddings for
    // aggregates and unions, no matter they are initialized by empty
    // initializers or non empty initializers. This can provide a consistent
    // behavior. So projects like the Linux kernel can rely on it.
    return !getLangOpts().CPlusPlus;
  }

  // Helper to get the alignment for a variable.
  unsigned getVtableGlobalVarAlignment(const VarDecl *D = nullptr) {
    LangAS AS = GetGlobalVarAddressSpace(D);
    unsigned PAlign = getItaniumVTableContext().isRelativeLayout()
                          ? 32
                          : getTarget().getPointerAlign(AS);
    return PAlign;
  }

private:
  bool shouldDropDLLAttribute(const Decl *D, const toolchain::GlobalValue *GV) const;

  toolchain::Constant *GetOrCreateLLVMFunction(
      StringRef MangledName, toolchain::Type *Ty, GlobalDecl D, bool ForVTable,
      bool DontDefer = false, bool IsThunk = false,
      toolchain::AttributeList ExtraAttrs = toolchain::AttributeList(),
      ForDefinition_t IsForDefinition = NotForDefinition);

  // Adds a declaration to the list of multi version functions if not present.
  void AddDeferredMultiVersionResolverToEmit(GlobalDecl GD);

  // References to multiversion functions are resolved through an implicitly
  // defined resolver function. This function is responsible for creating
  // the resolver symbol for the provided declaration. The value returned
  // will be for an ifunc (toolchain::GlobalIFunc) if the current target supports
  // that feature and for a regular function (toolchain::GlobalValue) otherwise.
  toolchain::Constant *GetOrCreateMultiVersionResolver(GlobalDecl GD);

  // In scenarios where a function is not known to be a multiversion function
  // until a later declaration, it is sometimes necessary to change the
  // previously created mangled name to align with requirements of whatever
  // multiversion function kind the function is now known to be. This function
  // is responsible for performing such mangled name updates.
  void UpdateMultiVersionNames(GlobalDecl GD, const FunctionDecl *FD,
                               StringRef &CurName);

  bool GetCPUAndFeaturesAttributes(GlobalDecl GD,
                                   toolchain::AttrBuilder &AttrBuilder,
                                   bool SetTargetFeatures = true);
  void setNonAliasAttributes(GlobalDecl GD, toolchain::GlobalObject *GO);

  /// Set function attributes for a function declaration.
  void SetFunctionAttributes(GlobalDecl GD, toolchain::Function *F,
                             bool IsIncompleteFunction, bool IsThunk);

  void EmitGlobalDefinition(GlobalDecl D, toolchain::GlobalValue *GV = nullptr);

  void EmitGlobalFunctionDefinition(GlobalDecl GD, toolchain::GlobalValue *GV);
  void EmitMultiVersionFunctionDefinition(GlobalDecl GD, toolchain::GlobalValue *GV);

  void EmitGlobalVarDefinition(const VarDecl *D, bool IsTentative = false);
  void EmitAliasDefinition(GlobalDecl GD);
  void emitIFuncDefinition(GlobalDecl GD);
  void emitCPUDispatchDefinition(GlobalDecl GD);
  void EmitObjCPropertyImplementations(const ObjCImplementationDecl *D);
  void EmitObjCIvarInitializations(ObjCImplementationDecl *D);

  // C++ related functions.

  void EmitDeclContext(const DeclContext *DC);
  void EmitLinkageSpec(const LinkageSpecDecl *D);
  void EmitTopLevelStmt(const TopLevelStmtDecl *D);

  /// Emit the function that initializes C++ thread_local variables.
  void EmitCXXThreadLocalInitFunc();

  /// Emit the function that initializes global variables for a C++ Module.
  void EmitCXXModuleInitFunc(language::Core::Module *Primary);

  /// Emit the function that initializes C++ globals.
  void EmitCXXGlobalInitFunc();

  /// Emit the function that performs cleanup associated with C++ globals.
  void EmitCXXGlobalCleanUpFunc();

  /// Emit the function that initializes the specified global (if PerformInit is
  /// true) and registers its destructor.
  void EmitCXXGlobalVarDeclInitFunc(const VarDecl *D,
                                    toolchain::GlobalVariable *Addr,
                                    bool PerformInit);

  void EmitPointerToInitFunc(const VarDecl *VD, toolchain::GlobalVariable *Addr,
                             toolchain::Function *InitFunc, InitSegAttr *ISA);

  /// EmitCtorList - Generates a global array of functions and priorities using
  /// the given list and name. This array will have appending linkage and is
  /// suitable for use as a LLVM constructor or destructor array. Clears Fns.
  void EmitCtorList(CtorList &Fns, const char *GlobalName);

  /// Emit any needed decls for which code generation was deferred.
  void EmitDeferred();

  /// Try to emit external vtables as available_externally if they have emitted
  /// all inlined virtual functions.  It runs after EmitDeferred() and therefore
  /// is not allowed to create new references to things that need to be emitted
  /// lazily.
  void EmitVTablesOpportunistically();

  /// Call replaceAllUsesWith on all pairs in Replacements.
  void applyReplacements();

  /// Call replaceAllUsesWith on all pairs in GlobalValReplacements.
  void applyGlobalValReplacements();

  void checkAliases();

  std::map<int, toolchain::TinyPtrVector<toolchain::Function *>> DtorsUsingAtExit;

  /// Register functions annotated with __attribute__((destructor)) using
  /// __cxa_atexit, if it is available, or atexit otherwise.
  void registerGlobalDtorsWithAtExit();

  // When using sinit and sterm functions, unregister
  // __attribute__((destructor)) annotated functions which were previously
  // registered by the atexit subroutine using unatexit.
  void unregisterGlobalDtorsWithUnAtExit();

  /// Emit deferred multiversion function resolvers and associated variants.
  void emitMultiVersionFunctions();

  /// Emit any vtables which we deferred and still have a use for.
  void EmitDeferredVTables();

  /// Emit a dummy function that reference a CoreFoundation symbol when
  /// @available is used on Darwin.
  void emitAtAvailableLinkGuard();

  /// Emit the toolchain.used and toolchain.compiler.used metadata.
  void emitLLVMUsed();

  /// For C++20 Itanium ABI, emit the initializers for the module.
  void EmitModuleInitializers(language::Core::Module *Primary);

  /// Emit the link options introduced by imported modules.
  void EmitModuleLinkOptions();

  /// Helper function for EmitStaticExternCAliases() to redirect ifuncs that
  /// have a resolver name that matches 'Elem' to instead resolve to the name of
  /// 'CppFunc'. This redirection is necessary in cases where 'Elem' has a name
  /// that will be emitted as an alias of the name bound to 'CppFunc'; ifuncs
  /// may not reference aliases. Redirection is only performed if 'Elem' is only
  /// used by ifuncs in which case, 'Elem' is destroyed. 'true' is returned if
  /// redirection is successful, and 'false' is returned otherwise.
  bool CheckAndReplaceExternCIFuncs(toolchain::GlobalValue *Elem,
                                    toolchain::GlobalValue *CppFunc);

  /// Emit aliases for internal-linkage declarations inside "C" language
  /// linkage specifications, giving them the "expected" name where possible.
  void EmitStaticExternCAliases();

  void EmitDeclMetadata();

  /// Emit the Clang version as toolchain.ident metadata.
  void EmitVersionIdentMetadata();

  /// Emit the Clang commandline as toolchain.commandline metadata.
  void EmitCommandLineMetadata();

  /// Emit the module flag metadata used to pass options controlling the
  /// the backend to LLVM.
  void EmitBackendOptionsMetadata(const CodeGenOptions &CodeGenOpts);

  /// Emits OpenCL specific Metadata e.g. OpenCL version.
  void EmitOpenCLMetadata();

  /// Emit the toolchain.gcov metadata used to tell LLVM where to emit the .gcno and
  /// .gcda files in a way that persists in .bc files.
  void EmitCoverageFile();

  /// Given a sycl_kernel_entry_point attributed function, emit the
  /// corresponding SYCL kernel caller offload entry point function.
  void EmitSYCLKernelCaller(const FunctionDecl *KernelEntryPointFn,
                            ASTContext &Ctx);

  /// Determine whether the definition must be emitted; if this returns \c
  /// false, the definition can be emitted lazily if it's used.
  bool MustBeEmitted(const ValueDecl *D);

  /// Determine whether the definition can be emitted eagerly, or should be
  /// delayed until the end of the translation unit. This is relevant for
  /// definitions whose linkage can change, e.g. implicit function instantions
  /// which may later be explicitly instantiated.
  bool MayBeEmittedEagerly(const ValueDecl *D);

  /// Check whether we can use a "simpler", more core exceptions personality
  /// function.
  void SimplifyPersonality();

  /// Helper function for getDefaultFunctionAttributes. Builds a set of function
  /// attributes which can be simply added to a function.
  void getTrivialDefaultFunctionAttributes(StringRef Name, bool HasOptnone,
                                           bool AttrOnCallSite,
                                           toolchain::AttrBuilder &FuncAttrs);

  /// Helper function for ConstructAttributeList and
  /// addDefaultFunctionDefinitionAttributes.  Builds a set of function
  /// attributes to add to a function with the given properties.
  void getDefaultFunctionAttributes(StringRef Name, bool HasOptnone,
                                    bool AttrOnCallSite,
                                    toolchain::AttrBuilder &FuncAttrs);

  toolchain::Metadata *CreateMetadataIdentifierImpl(QualType T, MetadataTypeMap &Map,
                                               StringRef Suffix);
};

}  // end namespace CodeGen
}  // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_CODEGEN_CODEGENMODULE_H
