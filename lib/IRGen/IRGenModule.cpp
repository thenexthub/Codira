//===--- IRGenModule.cpp - Codira Global LLVM IR Generation ----------------===//
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
//  This file implements IR generation for global declarations in Codira.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTContext.h"
#include "language/AST/AvailabilityRange.h"
#include "language/AST/DiagnosticsIRGen.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/IRGenOptions.h"
#include "language/AST/IRGenRequests.h"
#include "language/AST/Module.h"
#include "language/AST/ModuleDependencies.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/ToolchainExtras.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/Demangling/ManglingMacros.h"
#include "language/IRGen/IRGenPublic.h"
#include "language/IRGen/Linking.h"
#include "language/Runtime/Config.h"
#include "language/Runtime/RuntimeFnWrappersGen.h"
#include "language/Strings.h"
#include "language/Subsystems.h"
#include "clang/AST/ASTContext.h"
#include "clang/Basic/CharInfo.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CodeGenABITypes.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/CodeGen/CodiraCallingConv.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/Frontend/Debug/Options.h"
#include "toolchain/IR/Attributes.h"
#include "toolchain/IR/Constants.h"
#include "toolchain/IR/DataLayout.h"
#include "toolchain/IR/DerivedTypes.h"
#include "toolchain/IR/IRBuilder.h"
#include "toolchain/IR/Intrinsics.h"
#include "toolchain/IR/MDBuilder.h"
#include "toolchain/IR/Module.h"
#include "toolchain/IR/Type.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/MD5.h"
#include "toolchain/Support/ModRef.h"

#include "Callee.h"
#include "ConformanceDescription.h"
#include "GenDecl.h"
#include "GenEnum.h"
#include "GenMeta.h"
#include "GenPointerAuth.h"
#include "GenIntegerLiteral.h"
#include "GenType.h"
#include "IRGenModule.h"
#include "IRGenDebugInfo.h"
#include "ProtocolInfo.h"
#include "StructLayout.h"

#include <initializer_list>

using namespace language;
using namespace irgen;
using toolchain::Attribute;

const unsigned DefaultAS = 0;

/// A helper for creating LLVM struct types.
static toolchain::StructType *createStructType(IRGenModule &IGM,
                                          StringRef name,
                                  std::initializer_list<toolchain::Type*> types,
                                          bool packed = false) {
  return toolchain::StructType::create(IGM.getLLVMContext(),
                                  ArrayRef<toolchain::Type*>(types.begin(),
                                                        types.size()),
                                  name, packed);
}

toolchain::StructType *IRGenModule::createTransientStructType(
    StringRef name, std::initializer_list<toolchain::Type *> types, bool packed) {
  return createStructType(*this, name, types, packed);
}

static clang::CodeGenerator *createClangCodeGenerator(ASTContext &Context,
                                                      toolchain::LLVMContext &LLVMContext,
                                                      const IRGenOptions &Opts,
                                                      StringRef ModuleName,
                                                      StringRef PD) {
  auto Loader = Context.getClangModuleLoader();
  auto *Importer = static_cast<ClangImporter*>(&*Loader);
  assert(Importer && "No clang module loader!");
  auto &ClangContext = Importer->getClangASTContext();

  auto &CGTI = Importer->getTargetInfo();
  auto &CGO = Importer->getCodeGenOpts();
  CGO.OptimizationLevel = Opts.shouldOptimize() ? 3 : 0;

  CGO.DebugTypeExtRefs = !Opts.DisableClangModuleSkeletonCUs;
  CGO.DiscardValueNames = !Opts.shouldProvideValueNames();
  switch (Opts.DebugInfoLevel) {
  case IRGenDebugInfoLevel::None:
    CGO.setDebugInfo(toolchain::codegenoptions::DebugInfoKind::NoDebugInfo);
    break;
  case IRGenDebugInfoLevel::LineTables:
    CGO.setDebugInfo(toolchain::codegenoptions::DebugInfoKind::DebugLineTablesOnly);
    break;
  case IRGenDebugInfoLevel::ASTTypes:
  case IRGenDebugInfoLevel::DwarfTypes:
    CGO.setDebugInfo(toolchain::codegenoptions::DebugInfoKind::FullDebugInfo);
    break;
  }
  switch (Opts.DebugInfoFormat) {
  case IRGenDebugInfoFormat::None:
    break;
  case IRGenDebugInfoFormat::DWARF:
    CGO.DebugCompilationDir = Opts.DebugCompilationDir;
    CGO.DwarfVersion = Opts.DWARFVersion;
    CGO.DwarfDebugFlags =
        Opts.getDebugFlags(PD, Context.LangOpts.EnableCXXInterop,
                           Context.LangOpts.hasFeature(Feature::Embedded));
    break;
  case IRGenDebugInfoFormat::CodeView:
    CGO.EmitCodeView = true;
    CGO.DebugCompilationDir = Opts.DebugCompilationDir;
    // This actually contains the debug flags for codeview.
    CGO.DwarfDebugFlags =
        Opts.getDebugFlags(PD, Context.LangOpts.EnableCXXInterop,
                           Context.LangOpts.hasFeature(Feature::Embedded));
    break;
  }
  if (!Opts.TrapFuncName.empty()) {
    CGO.TrapFuncName = Opts.TrapFuncName;
  }

  // We don't need to perform coverage mapping for any Clang decls we've
  // synthesized, as they have no user-written code. This is also needed to
  // avoid a Clang crash when attempting to emit coverage for decls without
  // source locations (rdar://100172217).
  CGO.CoverageMapping = false;

  auto &VFS = Importer->getClangInstance().getVirtualFileSystem();
  auto &HSI = Importer->getClangPreprocessor()
                  .getHeaderSearchInfo()
                  .getHeaderSearchOpts();
  auto &PPO = Importer->getClangPreprocessor().getPreprocessorOpts();
  auto *ClangCodeGen = clang::CreateLLVMCodeGen(ClangContext.getDiagnostics(),
                                                ModuleName, &VFS, HSI, PPO, CGO,
                                                LLVMContext);
  ClangCodeGen->Initialize(ClangContext, CGTI);
  return ClangCodeGen;
}

#ifndef NDEBUG
static ValueDecl *lookupSimple(ModuleDecl *module, ArrayRef<StringRef> declPath) {
  DeclContext *dc = module;
  for (;; declPath = declPath.drop_front()) {
    SmallVector<ValueDecl*, 1> results;
    module->lookupMember(results, dc, module->getASTContext().getIdentifier(declPath.front()), Identifier());
    if (results.size() != 1) return nullptr;
    if (declPath.size() == 1) return results.front();
    dc = dyn_cast<DeclContext>(results.front());
    if (!dc) return nullptr;
  }
}

static void checkPointerAuthWitnessDiscriminator(IRGenModule &IGM, ArrayRef<StringRef> declPath, uint16_t expected) {
  auto &schema = IGM.getOptions().PointerAuth.ProtocolWitnesses;
  if (!schema.isEnabled()) return;

  auto decl = lookupSimple(IGM.getCodiraModule(), declPath);
  assert(decl && "decl not found");
  auto discriminator = PointerAuthInfo::getOtherDiscriminator(IGM, schema, SILDeclRef(decl));
  assert(discriminator->getZExtValue() == expected && "discriminator value doesn't match");
}

static void checkPointerAuthAssociatedTypeDiscriminator(IRGenModule &IGM, ArrayRef<StringRef> declPath, uint16_t expected) {
  auto &schema = IGM.getOptions().PointerAuth.ProtocolAssociatedTypeAccessFunctions;
  if (!schema.isEnabled()) return;

  auto decl = cast<AssociatedTypeDecl>(lookupSimple(IGM.getCodiraModule(), declPath));
  auto discriminator = PointerAuthInfo::getOtherDiscriminator(IGM, schema, decl);
  assert(discriminator->getZExtValue() == expected && "discriminator value doesn't match");
}

static void sanityCheckStdlib(IRGenModule &IGM) {
  if (!IGM.getCodiraModule()->isStdlibModule()) return;

  // Only run the soundness check when we're building the real stdlib.
  if (!lookupSimple(IGM.getCodiraModule(), { "String" })) return;

  if (!IGM.ObjCInterop) return;

  checkPointerAuthAssociatedTypeDiscriminator(IGM, { "_ObjectiveCBridgeable", "_ObjectiveCType" }, SpecialPointerAuthDiscriminators::ObjectiveCTypeDiscriminator);
  checkPointerAuthWitnessDiscriminator(IGM, { "_ObjectiveCBridgeable", "_bridgeToObjectiveC" }, SpecialPointerAuthDiscriminators::bridgeToObjectiveCDiscriminator);
  checkPointerAuthWitnessDiscriminator(IGM, { "_ObjectiveCBridgeable", "_forceBridgeFromObjectiveC" }, SpecialPointerAuthDiscriminators::forceBridgeFromObjectiveCDiscriminator);
  checkPointerAuthWitnessDiscriminator(IGM, { "_ObjectiveCBridgeable", "_conditionallyBridgeFromObjectiveC" }, SpecialPointerAuthDiscriminators::conditionallyBridgeFromObjectiveCDiscriminator);
}
#endif

static bool getIsCoroCCSupported(const clang::TargetInfo &targetInfo,
                                 const IRGenOptions &IRGenOpts) {
  // TODO: CoroutineAccessors: The one caller of this function should just be
  //                           rewritten as follows (once clang::CC_CoroAsync is
  //                           defined).
  //
  // clangASTContext.getTargetInfo().checkCallingConvention(clang::CC_CoroAsync)
  if (targetInfo.getTriple().isAArch64() && IRGenOpts.UseCoroCCArm64) {
    return true;
  }
  if (targetInfo.getTriple().getArch() == toolchain::Triple::x86_64 &&
      IRGenOpts.UseCoroCCX8664) {
    return true;
  }

  return false;
}

IRGenModule::IRGenModule(IRGenerator &irgen,
                         std::unique_ptr<toolchain::TargetMachine> &&target,
                         SourceFile *SF, StringRef ModuleName,
                         StringRef OutputFilename,
                         StringRef MainInputFilenameForDebugInfo,
                         StringRef PrivateDiscriminator)
    : LLVMContext(new toolchain::LLVMContext()), IRGen(irgen),
      Context(irgen.SIL.getASTContext()),
      // The LLVMContext (and the IGM itself) will get deleted by the IGMDeleter
      // as long as the IGM is registered with the IRGenerator.
      ClangCodeGen(createClangCodeGenerator(Context, *LLVMContext, irgen.Opts,
                                            ModuleName, PrivateDiscriminator)),
      Module(*ClangCodeGen->GetModule()),
      DataLayout(irgen.getClangDataLayoutString()),
      Triple(irgen.getEffectiveClangTriple()),
      VariantTriple(irgen.getEffectiveClangVariantTriple()),
      TargetMachine(std::move(target)),
      silConv(irgen.SIL), OutputFilename(OutputFilename),
      MainInputFilenameForDebugInfo(MainInputFilenameForDebugInfo),
      TargetInfo(CodiraTargetInfo::get(*this)), DebugInfo(nullptr),
      ModuleHash(nullptr), ObjCInterop(Context.LangOpts.EnableObjCInterop),
      UseDarwinPreStableABIBit(Context.LangOpts.UseDarwinPreStableABIBit),
      Types(*new TypeConverter(*this)) {
  irgen.addGenModule(SF, this);

  auto &opts = irgen.Opts;

  EnableValueNames = opts.shouldProvideValueNames();
  
  VoidTy = toolchain::Type::getVoidTy(getLLVMContext());
  PtrTy = toolchain::PointerType::getUnqual(getLLVMContext());
  Int1Ty = toolchain::Type::getInt1Ty(getLLVMContext());
  Int8Ty = toolchain::Type::getInt8Ty(getLLVMContext());
  Int16Ty = toolchain::Type::getInt16Ty(getLLVMContext());
  Int32Ty = toolchain::Type::getInt32Ty(getLLVMContext());
  Int32PtrTy = Int32Ty->getPointerTo();
  Int64Ty = toolchain::Type::getInt64Ty(getLLVMContext());
  Int8PtrTy = PtrTy;
  Int8PtrPtrTy = Int8PtrTy->getPointerTo(0);
  SizeTy = DataLayout.getIntPtrType(getLLVMContext(), /*addrspace*/ 0);

  // For the relative address type, we want to use the int32 bit type
  // on most architectures, e.g. x86_64, because it produces valid
  // fixups/relocations. The exception is 16-bit architectures,
  // so we shorten the relative address type there.
  if (SizeTy->getBitWidth()<32) {
    RelativeAddressTy = SizeTy;
  } else {
    RelativeAddressTy = Int32Ty;
  }

  RelativeAddressPtrTy = RelativeAddressTy->getPointerTo();

  FloatTy = toolchain::Type::getFloatTy(getLLVMContext());
  DoubleTy = toolchain::Type::getDoubleTy(getLLVMContext());

  auto CI = static_cast<ClangImporter*>(&*Context.getClangModuleLoader());
  assert(CI && "no clang module loader");
  auto &clangASTContext = CI->getClangASTContext();

  ObjCBoolTy = Int1Ty;
  if (clangASTContext.getTargetInfo().useSignedCharForObjCBool())
    ObjCBoolTy = Int8Ty;

  RefCountedStructTy =
    toolchain::StructType::create(getLLVMContext(), "language.refcounted");
  RefCountedPtrTy = RefCountedStructTy->getPointerTo(/*addrspace*/ 0);
  RefCountedNull = toolchain::ConstantPointerNull::get(RefCountedPtrTy);

  // For now, references storage types are just pointers.
#define CHECKED_REF_STORAGE(Name, name, ...)                                   \
  Name##ReferenceStructTy =                                                    \
      createStructType(*this, "language." #name, {RefCountedPtrTy});              \
  Name##ReferencePtrTy = Name##ReferenceStructTy->getPointerTo(0);
#include "language/AST/ReferenceStorage.def"

  // A type metadata record is the structure pointed to by the canonical
  // address point of a type metadata.  This is at least one word, and
  // potentially more than that, past the start of the actual global
  // structure.
  TypeMetadataStructTy = createStructType(*this, "language.type", {
    MetadataKindTy          // MetadataKind Kind;
  });
  TypeMetadataPtrTy = TypeMetadataStructTy->getPointerTo(DefaultAS);
  TypeMetadataPtrPtrTy = TypeMetadataPtrTy->getPointerTo(DefaultAS);

  TypeMetadataResponseTy = createStructType(*this, "language.metadata_response", {
    TypeMetadataPtrTy,
    SizeTy
  });

  OffsetPairTy = toolchain::StructType::get(getLLVMContext(), { SizeTy, SizeTy });

  // The TypeLayout structure, including all possible trailing components.
  FullTypeLayoutTy = createStructType(*this, "language.full_type_layout", {
    SizeTy, // size
    SizeTy, // flags
    SizeTy, // alignment
    SizeTy  // extra inhabitant flags (optional)
  });

  TypeLayoutTy = createStructType(*this, "language.type_layout", {
    SizeTy, // size
    SizeTy, // stride
    Int32Ty, // flags
    Int32Ty // extra inhabitant count
  });

  // A protocol descriptor describes a protocol. It is not type metadata in
  // and of itself, but is referenced in the structure of existential type
  // metadata records.
  ProtocolDescriptorStructTy = createStructType(*this, "language.protocol", {
    Int8PtrTy,              // objc isa
    Int8PtrTy,              // name
    Int8PtrTy,              // inherited protocols
    Int8PtrTy,              // required objc instance methods
    Int8PtrTy,              // required objc class methods
    Int8PtrTy,              // optional objc instance methods
    Int8PtrTy,              // optional objc class methods
    Int8PtrTy,              // objc properties
    Int32Ty,                // size
    Int32Ty,                // flags
    Int32Ty,                // total requirement count
    Int32Ty,                // requirements array
    RelativeAddressTy,      // superclass
    RelativeAddressTy       // associated type names
  });
  
  ProtocolDescriptorPtrTy = ProtocolDescriptorStructTy->getPointerTo();

  ProtocolRequirementStructTy =
      createStructType(*this, "language.protocol_requirement", {
    Int32Ty,                // flags
    RelativeAddressTy,      // default implementation
  });
  
  // A tuple type metadata record has a couple extra fields.
  auto tupleElementTy = createStructType(*this, "language.tuple_element_type", {
    TypeMetadataPtrTy,      // Metadata *Type;
    Int32Ty                 // int32_t Offset;
  });
  TupleTypeMetadataTy = createStructType(
      *this, "language.tuple_type",
      {
          TypeMetadataStructTy,                   // (base)
          SizeTy,                                 // size_t NumElements;
          Int8PtrTy,                              // const char *Labels;
          toolchain::ArrayType::get(tupleElementTy, 0) // Element Elements[];
      });
  TupleTypeMetadataPtrTy = TupleTypeMetadataTy->getPointerTo();
  // A full type metadata record is basically just an adjustment to the
  // address point of a type metadata.  Resilience may cause
  // additional data to be laid out prior to this address point.
  static_assert(MetadataAdjustmentIndex::ValueType == 2,
                "Adjustment index must be synchronized with this layout");
  FullTypeMetadataStructTy = createStructType(*this, "language.full_type", {
    Int8PtrTy,
    WitnessTablePtrTy,
    TypeMetadataStructTy
  });
  FullTypeMetadataPtrTy = FullTypeMetadataStructTy->getPointerTo(DefaultAS);

  FullForeignTypeMetadataStructTy = createStructType(*this, "language.full_foreign_type", {
    WitnessTablePtrTy,
    TypeMetadataStructTy
  });

  DeallocatingDtorTy = toolchain::FunctionType::get(VoidTy, RefCountedPtrTy, false);
  toolchain::Type *dtorPtrTy = DeallocatingDtorTy->getPointerTo();

  FullExistentialTypeMetadataStructTy = createStructType(*this, "language.full_existential_type", {
    WitnessTablePtrTy,
    TypeMetadataStructTy
  });

  // A full heap metadata is basically just an additional small prefix
  // on a full metadata, used for metadata corresponding to heap
  // allocations.
  static_assert(MetadataAdjustmentIndex::Class == 3,
                "Adjustment index must be synchronized with this layout");
  FullHeapMetadataStructTy =
                  createStructType(*this, "language.full_heapmetadata", {
    Int8PtrTy,
    dtorPtrTy,
    WitnessTablePtrTy,
    TypeMetadataStructTy
  });
  FullHeapMetadataPtrTy = FullHeapMetadataStructTy->getPointerTo(DefaultAS);

  // A full box metadata is non-type heap metadata for a heap allocation of a
  // single value. The box tracks the offset to the value inside the box.
  FullBoxMetadataStructTy =
                  createStructType(*this, "language.full_boxmetadata", {
    dtorPtrTy,
    WitnessTablePtrTy,
    TypeMetadataStructTy,
    Int32Ty,
    CaptureDescriptorPtrTy,
  });
  FullBoxMetadataPtrTy = FullBoxMetadataStructTy->getPointerTo(DefaultAS);

  // This must match struct HeapObject in the runtime.
  toolchain::Type *refCountedElts[] = {TypeMetadataPtrTy, IntPtrTy};
  RefCountedStructTy->setBody(refCountedElts);
  RefCountedStructSize =
    Size(DataLayout.getStructLayout(RefCountedStructTy)->getSizeInBytes());

  PtrSize = Size(DataLayout.getPointerSize(DefaultAS));

  FunctionPairTy = createStructType(*this, "language.function", {
    FunctionPtrTy,
    RefCountedPtrTy,
  });

  OpaqueTy = toolchain::StructType::create(getLLVMContext(), "language.opaque");
  OpaquePtrTy = OpaqueTy->getPointerTo(DefaultAS);
  NoEscapeFunctionPairTy = createStructType(*this, "language.noescape.function", {
    FunctionPtrTy,
    OpaquePtrTy,
  });

  ProtocolRecordTy =
    createStructType(*this, "language.protocolref", {
      RelativeAddressTy
    });
  ProtocolRecordPtrTy = ProtocolRecordTy->getPointerTo();

  ProtocolConformanceDescriptorTy
    = createStructType(*this, "language.protocol_conformance_descriptor", {
      RelativeAddressTy,
      RelativeAddressTy,
      RelativeAddressTy,
      Int32Ty
    });
  ProtocolConformanceDescriptorPtrTy
    = ProtocolConformanceDescriptorTy->getPointerTo(DefaultAS);

  TypeContextDescriptorTy
    = toolchain::StructType::create(getLLVMContext(), "language.type_descriptor");
  TypeContextDescriptorPtrTy
    = TypeContextDescriptorTy->getPointerTo(DefaultAS);

  ClassContextDescriptorTy =
        toolchain::StructType::get(getLLVMContext(), {
    Int32Ty, // context flags
    Int32Ty, // parent
    Int32Ty, // name
    Int32Ty, // kind
    Int32Ty, // accessor function
    Int32Ty, // num fields
    Int32Ty, // field offset vector
    Int32Ty, // is_reflectable flag
    Int32Ty, // (Generics Descriptor) argument offset
    Int32Ty, // (Generics Descriptor) num params
    Int32Ty, // (Generics Descriptor) num requirements
    Int32Ty, // (Generics Descriptor) num key arguments
    Int32Ty, // (Generics Descriptor) num extra arguments
    Int32Ty, // (VTable Descriptor) offset
    Int32Ty, // (VTable Descriptor) size
    Int32Ty, // (Methods Descriptor) accessor
    Int32Ty, // (Methods Descriptor) flags
  }, /*packed=*/true);

  MethodDescriptorStructTy
    = createStructType(*this, "language.method_descriptor", {
      Int32Ty,
      RelativeAddressTy,
    });

  MethodOverrideDescriptorStructTy
    = createStructType(*this, "language.method_override_descriptor", {
      RelativeAddressTy,
      RelativeAddressTy,
      RelativeAddressTy
    });

  MethodDefaultOverrideDescriptorStructTy = createStructType(
      *this, "language.method_default_override_descriptor",
      {RelativeAddressTy, RelativeAddressTy, RelativeAddressTy});

  TypeMetadataRecordTy
    = createStructType(*this, "language.type_metadata_record", {
      RelativeAddressTy
    });
  TypeMetadataRecordPtrTy
    = TypeMetadataRecordTy->getPointerTo(DefaultAS);

  FieldDescriptorTy
    = toolchain::StructType::create(getLLVMContext(), "language.field_descriptor");
  FieldDescriptorPtrTy = FieldDescriptorTy->getPointerTo(DefaultAS);
  FieldDescriptorPtrPtrTy = FieldDescriptorPtrTy->getPointerTo(DefaultAS);

  FixedBufferTy = nullptr;
  for (unsigned i = 0; i != MaxNumValueWitnesses; ++i)
    ValueWitnessTys[i] = nullptr;

  ObjCPtrTy = toolchain::StructType::create(getLLVMContext(), "objc_object")
                ->getPointerTo(DefaultAS);
  BridgeObjectPtrTy = toolchain::StructType::create(getLLVMContext(), "language.bridge")
                ->getPointerTo(DefaultAS);

  ObjCClassStructTy = toolchain::StructType::create(getLLVMContext(), "objc_class");
  ObjCClassPtrTy = ObjCClassStructTy->getPointerTo(DefaultAS);
  toolchain::Type *objcClassElts[] = {
    ObjCClassPtrTy,
    ObjCClassPtrTy,
    OpaquePtrTy,
    OpaquePtrTy,
    IntPtrTy
  };
  ObjCClassStructTy->setBody(objcClassElts);

  ObjCSuperStructTy = toolchain::StructType::create(getLLVMContext(), "objc_super");
  ObjCSuperPtrTy = ObjCSuperStructTy->getPointerTo(DefaultAS);
  toolchain::Type *objcSuperElts[] = {
    ObjCPtrTy,
    ObjCClassPtrTy
  };
  ObjCSuperStructTy->setBody(objcSuperElts);
  
  ObjCBlockStructTy = toolchain::StructType::create(getLLVMContext(), "objc_block");
  ObjCBlockPtrTy = ObjCBlockStructTy->getPointerTo(DefaultAS);
  toolchain::Type *objcBlockElts[] = {
    ObjCClassPtrTy, // isa
    Int32Ty,        // flags
    Int32Ty,        // reserved
    FunctionPtrTy,  // invoke function pointer
    Int8PtrTy,      // TODO: block descriptor pointer.
                    // We will probably need a struct type for that at some
                    // point too.
  };
  ObjCBlockStructTy->setBody(objcBlockElts);

  // Class _Nullable callback(Class _Nonnull cls, void * _Nullable arg);
  toolchain::Type *params[] = { ObjCClassPtrTy, Int8PtrTy };
  ObjCUpdateCallbackTy = toolchain::FunctionType::get(ObjCClassPtrTy, params, false);

  // The full class stub structure, including a word before the address point.
  ObjCFullResilientClassStubTy = createStructType(*this, "objc_full_class_stub", {
    SizeTy, // zero padding to appease the linker
    SizeTy, // isa pointer -- always 1
    ObjCUpdateCallbackTy->getPointerTo() // the update callback
  });

  // What we actually export.
  ObjCResilientClassStubTy = createStructType(*this, "objc_class_stub", {
    SizeTy, // isa pointer -- always 1
    ObjCUpdateCallbackTy->getPointerTo() // the update callback
  });

  auto ErrorStructTy = toolchain::StructType::create(getLLVMContext(), "language.error");
  // ErrorStruct is currently opaque to the compiler.
  ErrorPtrTy = ErrorStructTy->getPointerTo(DefaultAS);
  
  toolchain::Type *openedErrorTriple[] = {
    OpaquePtrTy,
    TypeMetadataPtrTy,
    WitnessTablePtrTy,
  };
  OpenedErrorTripleTy = toolchain::StructType::get(getLLVMContext(),
                                              openedErrorTriple,
                                              /*packed*/ false);
  OpenedErrorTriplePtrTy = OpenedErrorTripleTy->getPointerTo(DefaultAS);

  WitnessTablePtrPtrTy = WitnessTablePtrTy->getPointerTo(DefaultAS);
  
  // todo
  OpaqueTypeDescriptorTy = TypeContextDescriptorTy;
  OpaqueTypeDescriptorPtrTy = OpaqueTypeDescriptorTy->getPointerTo();

  InvariantMetadataID = getLLVMContext().getMDKindID("invariant.load");
  InvariantNode = toolchain::MDNode::get(getLLVMContext(), {});
  DereferenceableID = getLLVMContext().getMDKindID("dereferenceable");
  
  C_CC = getOptions().PlatformCCallingConvention;
  // TODO: use "tinycc" on platforms that support it
  DefaultCC = LANGUAGE_DEFAULT_TOOLCHAIN_CC;

  bool isCodiraCCSupported =
    clangASTContext.getTargetInfo().checkCallingConvention(clang::CC_Codira)
    == clang::TargetInfo::CCCR_OK;
  if (isCodiraCCSupported) {
    CodiraCC = toolchain::CallingConv::Codira;
  } else {
    CodiraCC = DefaultCC;
  }

  bool isAsyncCCSupported =
    clangASTContext.getTargetInfo().checkCallingConvention(clang::CC_CodiraAsync)
    == clang::TargetInfo::CCCR_OK;
  if (isAsyncCCSupported) {
    CodiraAsyncCC = toolchain::CallingConv::CodiraTail;
    AsyncTailCallKind = toolchain::CallInst::TCK_MustTail;
  } else {
    CodiraAsyncCC = CodiraCC;
    AsyncTailCallKind = toolchain::CallInst::TCK_Tail;
  }

  bool isCoroCCSupported =
      getIsCoroCCSupported(clangASTContext.getTargetInfo(), opts);
  if (isCoroCCSupported) {
    CodiraCoroCC = toolchain::CallingConv::CodiraCoro;
  } else {
    CodiraCoroCC = CodiraCC;
  }

  if (opts.DebugInfoLevel > IRGenDebugInfoLevel::None)
    DebugInfo = IRGenDebugInfo::createIRGenDebugInfo(IRGen.Opts, *CI, *this,
                                                     Module,
                                                 MainInputFilenameForDebugInfo,
                                                     PrivateDiscriminator);

  if (auto loader = Context.getClangModuleLoader()) {
    ClangASTContext =
        &static_cast<ClangImporter *>(loader)->getClangASTContext();
  }

  if (ClangASTContext) {
    auto atomicBoolTy = ClangASTContext->getAtomicType(ClangASTContext->BoolTy);
    AtomicBoolSize = Size(ClangASTContext->getTypeSize(atomicBoolTy));
    AtomicBoolAlign = Alignment(ClangASTContext->getTypeSize(atomicBoolTy));
  }
  if (TargetInfo.OutputObjectFormat == toolchain::Triple::Wasm) {
    // On WebAssembly, tail optional arguments are not allowed because Wasm
    // requires callee and caller signature to be the same. So LLVM adds dummy
    // arguments for `languageself` and `languageerror`. If there is `languageself` but
    // is no `languageerror` in a languagecc function or invocation, then LLVM adds
    // dummy `languageerror` parameter or argument. To count up how many dummy
    // arguments should be added, we need to mark it as `languageerror` even though
    // it's not in register.
    ShouldUseCodiraError = true;
  } else if (!isCodiraCCSupported) {
    ShouldUseCodiraError = false;
  } else {
    ShouldUseCodiraError =
        clang::CodeGen::languagecall::isCodiraErrorLoweredInRegister(
            ClangCodeGen->CGM());
  }

#ifndef NDEBUG
  sanityCheckStdlib(*this);
#endif

  DynamicReplacementsTy =
      toolchain::StructType::get(getLLVMContext(), {Int8PtrPtrTy, Int8PtrTy});
  DynamicReplacementsPtrTy = DynamicReplacementsTy->getPointerTo(DefaultAS);

  DynamicReplacementLinkEntryTy =
      toolchain::StructType::create(getLLVMContext(), "language.dyn_repl_link_entry");
  DynamicReplacementLinkEntryPtrTy =
      DynamicReplacementLinkEntryTy->getPointerTo(DefaultAS);
  toolchain::Type *linkEntryFields[] = {
    Int8PtrTy, // function pointer.
    DynamicReplacementLinkEntryPtrTy // next.
  };
  DynamicReplacementLinkEntryTy->setBody(linkEntryFields);

  DynamicReplacementKeyTy = createStructType(*this, "language.dyn_repl_key",
                                             {RelativeAddressTy, Int32Ty});

  AccessibleFunctionRecordTy =
      createStructType(*this, "language.accessible_function",
                       {RelativeAddressTy, RelativeAddressTy, RelativeAddressTy,
                        RelativeAddressTy, Int32Ty});

  AsyncFunctionPointerTy = createStructType(*this, "language.async_func_pointer",
                                            {RelativeAddressTy, Int32Ty}, true);
  CodiraContextTy = toolchain::StructType::create(getLLVMContext(), "language.context");
  CodiraContextPtrTy = CodiraContextTy->getPointerTo(DefaultAS);

  // This must match the definition of class AsyncTask in language/ABI/Task.h.
  CodiraTaskTy = createStructType(*this, "language.task", {
    RefCountedStructTy,   // object header
    Int8PtrTy, Int8PtrTy, // Job.SchedulerPrivate
    Int32Ty,              // Job.Flags
    Int32Ty,              // Job.ID
    Int8PtrTy, Int8PtrTy, // Reserved
    FunctionPtrTy,        // Job.RunJob/Job.ResumeTask
    CodiraContextPtrTy,    // Task.ResumeContext
  });

  AsyncFunctionPointerPtrTy = AsyncFunctionPointerTy->getPointerTo(DefaultAS);
  CodiraTaskPtrTy = CodiraTaskTy->getPointerTo(DefaultAS);
  CodiraAsyncLetPtrTy = Int8PtrTy; // we pass it opaquely (AsyncLet*)
  CodiraTaskOptionRecordTy =
    toolchain::StructType::create(getLLVMContext(), "language.task_option");
  CodiraTaskOptionRecordPtrTy = CodiraTaskOptionRecordTy->getPointerTo(DefaultAS);
  CodiraTaskOptionRecordTy->setBody({
    SizeTy,                     // Flags
    CodiraTaskOptionRecordPtrTy, // Parent
  });
  CodiraTaskGroupPtrTy = Int8PtrTy; // we pass it opaquely (TaskGroup*)
  CodiraTaskGroupTaskOptionRecordTy = createStructType(
      *this, "language.task_group_task_option", {
    CodiraTaskOptionRecordTy,    // Base option record
    CodiraTaskGroupPtrTy,        // Task group
  });
  CodiraResultTypeInfoTaskOptionRecordTy = createStructType(
      *this, "language.result_type_info_task_option", {
    CodiraTaskOptionRecordTy,    // Base option record
    SizeTy,
    SizeTy,
    Int8PtrTy,
    Int8PtrTy,
    Int8PtrTy,
  });
  ExecutorFirstTy = SizeTy;
  ExecutorSecondTy = SizeTy;
  CodiraExecutorTy = createStructType(*this, "language.executor", {
    ExecutorFirstTy,      // identity
    ExecutorSecondTy,     // implementation
  });
  CodiraInitialSerialExecutorTaskOptionRecordTy =
      createStructType(*this, "language.serial_executor_task_option", {
    CodiraTaskOptionRecordTy, // Base option record
    CodiraExecutorTy,         // Executor
  });
  CodiraInitialTaskExecutorUnownedPreferenceTaskOptionRecordTy =
      createStructType(*this, "language.task_executor_task_option", {
    CodiraTaskOptionRecordTy, // Base option record
    CodiraExecutorTy,         // Executor
  });
  CodiraInitialTaskExecutorOwnedPreferenceTaskOptionRecordTy =
      createStructType(*this, "language.task_executor_owned_task_option", {
    CodiraTaskOptionRecordTy, // Base option record
    CodiraExecutorTy,         // Executor
  });
  CodiraInitialTaskNameTaskOptionRecordTy =
      createStructType(*this, "language.task_name_task_option", {
    CodiraTaskOptionRecordTy, // Base option record
    Int8PtrTy,               // Task name string (char*)
  });
  CodiraJobTy = createStructType(*this, "language.job", {
    RefCountedStructTy,   // object header
    Int8PtrTy, Int8PtrTy, // SchedulerPrivate
    Int32Ty,              // flags
    Int32Ty,              // ID
    Int8PtrTy, Int8PtrTy, // Reserved
    FunctionPtrTy,        // RunJob/ResumeTask
  });
  CodiraJobPtrTy = CodiraJobTy->getPointerTo(DefaultAS);

  // using TaskContinuationFunction =
  //   LANGUAGE_CC(language) void (LANGUAGE_ASYNC_CONTEXT AsyncContext *);
  TaskContinuationFunctionTy = toolchain::FunctionType::get(
      VoidTy, {CodiraContextPtrTy}, /*isVarArg*/ false);
  TaskContinuationFunctionPtrTy = TaskContinuationFunctionTy->getPointerTo();

  CodiraContextTy->setBody({
    CodiraContextPtrTy,             // Parent
    TaskContinuationFunctionPtrTy, // ResumeParent
  });

  AsyncTaskAndContextTy = createStructType(
      *this, "language.async_task_and_context",
      { CodiraTaskPtrTy, CodiraContextPtrTy });

  if (Context.LangOpts.isConcurrencyModelTaskToThread()) {
    ContinuationAsyncContextTy = createStructType(
        *this, "language.continuation_context",
        {CodiraContextTy,       // AsyncContext header
         SizeTy,               // flags
         SizeTy,               // await synchronization
         ErrorPtrTy,           // error result pointer
         OpaquePtrTy,          // normal result address
         CodiraExecutorTy,      // resume to executor
         SizeTy                // pointer to condition variable
         });
  } else {
    ContinuationAsyncContextTy = createStructType(
        *this, "language.continuation_context",
        {CodiraContextTy,       // AsyncContext header
         SizeTy,               // flags
         SizeTy,               // await synchronization
         ErrorPtrTy,           // error result pointer
         OpaquePtrTy,          // normal result address
         CodiraExecutorTy       // resume to executor
         });
  }
  ContinuationAsyncContextPtrTy =
    ContinuationAsyncContextTy->getPointerTo(DefaultAS);

  ClassMetadataBaseOffsetTy = toolchain::StructType::get(
      getLLVMContext(), {
                            SizeTy,  // Immediate members offset
                            Int32Ty, // Negative size in words
                            Int32Ty  // Positive size in words
                        });

  DifferentiabilityWitnessTy = createStructType(
      *this, "language.differentiability_witness", {Int8PtrTy, Int8PtrTy});

  CoroFunctionPointerTy = createStructType(*this, "language.coro_func_pointer",
                                           {RelativeAddressTy, Int32Ty}, true);
  CoroFunctionPointerPtrTy = CoroFunctionPointerTy->getPointerTo();
  CoroAllocationTy = PtrTy;
  CoroAllocateFnTy =
      toolchain::FunctionType::get(CoroAllocationTy, SizeTy, /*isVarArg*/ false);
  CoroDeallocateFnTy =
      toolchain::FunctionType::get(VoidTy, CoroAllocationTy, /*isVarArg*/ false);
  CoroAllocatorFlagsTy = Int32Ty;
  // language/ABI/Coro.h : CoroAllocator
  CoroAllocatorTy = createStructType(*this, "language.coro_allocator",
                                     {
                                         Int32Ty, // CoroAllocator.Flags
                                         CoroAllocateFnTy->getPointerTo(),
                                         CoroDeallocateFnTy->getPointerTo(),
                                     });
  CoroAllocatorPtrTy = CoroAllocatorTy->getPointerTo();
}

IRGenModule::~IRGenModule() {
  destroyMetadataLayoutMap();
  destroyPointerAuthCaches();
  delete &Types;
}

// Explicitly listing these constants is an unfortunate compromise for
// making the database file much more compact.
//
// They have to be non-local because otherwise we'll get warnings when
// a particular x-macro expansion doesn't use one.
namespace RuntimeConstants {
  const auto ReadNone = toolchain::MemoryEffects::none();
  const auto ReadOnly = toolchain::MemoryEffects::readOnly();
  const auto ArgMemOnly = toolchain::MemoryEffects::argMemOnly();
  const auto ArgMemReadOnly = toolchain::MemoryEffects::argMemOnly(toolchain::ModRefInfo::Ref);
  const auto NoReturn = toolchain::Attribute::NoReturn;
  const auto NoUnwind = toolchain::Attribute::NoUnwind;
  const auto ZExt = toolchain::Attribute::ZExt;
  const auto FirstParamReturned = toolchain::Attribute::Returned;
  const auto WillReturn = toolchain::Attribute::WillReturn;

  RuntimeAvailability AlwaysAvailable(ASTContext &Context) {
    return RuntimeAvailability::AlwaysAvailable;
  }

  bool
  isDeploymentAvailabilityContainedIn(ASTContext &Context,
                                      AvailabilityRange featureAvailability) {
    auto deploymentAvailability =
        AvailabilityRange::forDeploymentTarget(Context);
    return deploymentAvailability.isContainedIn(featureAvailability);
  }

  RuntimeAvailability OpaqueTypeAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getOpaqueTypeAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability
  GetTypesInAbstractMetadataStateAvailability(ASTContext &context) {
    auto featureAvailability =
        context.getTypesInAbstractMetadataStateAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability DynamicReplacementAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getCodira51Availability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::AvailableByCompatibilityLibrary;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability
  CompareTypeContextDescriptorsAvailability(ASTContext &Context) {
    auto featureAvailability =
        Context.getCompareTypeContextDescriptorsAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability
  CompareProtocolConformanceDescriptorsAvailability(ASTContext &Context) {
    auto featureAvailability =
        Context.getCompareProtocolConformanceDescriptorsAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability
  GetCanonicalSpecializedMetadataAvailability(ASTContext &context) {
    auto featureAvailability =
        context.getIntermodulePrespecializedGenericMetadataAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability
  GetCanonicalPrespecializedGenericMetadataAvailability(ASTContext &context) {
    auto featureAvailability =
        context.getPrespecializedGenericMetadataAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability ConcurrencyAvailability(ASTContext &context) {
    auto featureAvailability = context.getConcurrencyAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability TaskExecutorAvailability(ASTContext &context) {
    auto featureAvailability = context.getTaskExecutorAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability ConcurrencyDiscardingTaskGroupAvailability(ASTContext &context) {
    auto featureAvailability =
        context.getConcurrencyDiscardingTaskGroupAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability DifferentiationAvailability(ASTContext &context) {
    auto featureAvailability = context.getDifferentiationAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability TypedThrowsAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getTypedThrowsAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability IsolatedDeinitAvailability(ASTContext &context) {
    auto featureAvailability = context.getIsolatedDeinitAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability
  MultiPayloadEnumTagSinglePayloadAvailability(ASTContext &context) {
    auto featureAvailability = context.getMultiPayloadEnumTagSinglePayload();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability
  ObjCIsUniquelyReferencedAvailability(ASTContext &context) {
    auto featureAvailability =
        context.getObjCIsUniquelyReferencedAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability SignedConformsToProtocolAvailability(ASTContext &context) {
    auto featureAvailability =
        context.getSignedConformsToProtocolAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability SignedDescriptorAvailability(ASTContext &context) {
    auto featureAvailability =
        context.getSignedDescriptorAvailability();
    if (!isDeploymentAvailabilityContainedIn(context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability TaskRunInlineAvailability(ASTContext &context) {
    if (context.LangOpts.isConcurrencyModelTaskToThread()) {
      return RuntimeAvailability::AlwaysAvailable;
    }
    // language_task_run_inline is only available under task-to-thread execution
    // model.
    return RuntimeAvailability::ConditionallyAvailable;
  }

  RuntimeAvailability ParameterizedExistentialAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getParameterizedExistentialAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability ClearSensitiveAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getClearSensitiveAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability InitRawStructMetadataAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getInitRawStructMetadataAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability InitRawStructMetadata2Availability(ASTContext &Context) {
    auto featureAvailability = Context.getInitRawStructMetadata2Availability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability ValueGenericTypeAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getValueGenericTypeAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

  RuntimeAvailability CoroutineAccessorsAvailability(ASTContext &Context) {
    auto featureAvailability = Context.getCoroutineAccessorsAvailability();
    if (!isDeploymentAvailabilityContainedIn(Context, featureAvailability)) {
      return RuntimeAvailability::ConditionallyAvailable;
    }
    return RuntimeAvailability::AlwaysAvailable;
  }

} // namespace RuntimeConstants

// We don't use enough attributes to justify generalizing the
// RuntimeFunctions.def FUNCTION macro. Instead, special case the one attribute
// associated with the return type not the function type.
static bool isReturnAttribute(toolchain::Attribute::AttrKind Attr) {
  return Attr == toolchain::Attribute::ZExt;
}
// Similar to the 'return' attribute we assume that the 'returned' attributed is
// associated with the first function parameter.
static bool isReturnedAttribute(toolchain::Attribute::AttrKind Attr) {
  return Attr == toolchain::Attribute::Returned;
}

static toolchain::MemoryEffects mergeMemoryEffects(ArrayRef<toolchain::MemoryEffects> effects) {
    if (effects.empty())
      return toolchain::MemoryEffects::unknown();
    toolchain::MemoryEffects mergedEffects = toolchain::MemoryEffects::none();
    for (auto effect : effects)
        mergedEffects |= effect;
    return mergedEffects;
}

toolchain::FunctionType *language::getRuntimeFnType(toolchain::Module &Module,
                                           toolchain::ArrayRef<toolchain::Type*> retTypes,
                                           toolchain::ArrayRef<toolchain::Type*> argTypes) {
  toolchain::Type *retTy;
  if (retTypes.size() == 1)
    retTy = *retTypes.begin();
  else
    retTy = toolchain::StructType::get(Module.getContext(),
                                  {retTypes.begin(), retTypes.end()},
                                  /*packed*/ false);
  return toolchain::FunctionType::get(retTy, {argTypes.begin(), argTypes.end()},
                                 /*isVararg*/ false);
}

toolchain::Constant *language::getRuntimeFn(
    toolchain::Module &Module, toolchain::Constant *&cache, const char *ModuleName,
    const char *FunctionName, toolchain::CallingConv::ID cc,
    RuntimeAvailability availability, toolchain::ArrayRef<toolchain::Type *> retTypes,
    toolchain::ArrayRef<toolchain::Type *> argTypes, ArrayRef<Attribute::AttrKind> attrs,
    ArrayRef<toolchain::MemoryEffects> memEffects, IRGenModule *IGM) {

  if (cache)
    return cache;

  bool isWeakLinked = false;
  std::string name(FunctionName);

  switch (availability) {
  case RuntimeAvailability::AlwaysAvailable:
    // Nothing to do.
    break;
  case RuntimeAvailability::ConditionallyAvailable: {
    isWeakLinked = true;
    break;
  }
  case RuntimeAvailability::AvailableByCompatibilityLibrary: {
    name.append("50");
    break;
  }
  }

  toolchain::Type *retTy;
  if (retTypes.size() == 1)
    retTy = *retTypes.begin();
  else
    retTy = toolchain::StructType::get(Module.getContext(),
                                  {retTypes.begin(), retTypes.end()},
                                  /*packed*/ false);
  auto fnTy = toolchain::FunctionType::get(retTy,
                                      {argTypes.begin(), argTypes.end()},
                                      /*isVararg*/ false);

  auto addr = Module.getOrInsertFunction(name.c_str(), fnTy).getCallee();
  auto fnptr = addr;
  // Strip off any bitcast we might have due to this function being declared of
  // a different type previously.
  if (auto bitcast = dyn_cast<toolchain::BitCastInst>(fnptr))
    fnptr = cast<toolchain::Constant>(bitcast->getOperand(0));
  cache = cast<toolchain::Constant>(addr);

  // Add any function attributes and set the calling convention.
  if (auto fn = dyn_cast<toolchain::Function>(fnptr)) {
    fn->setCallingConv(cc);

    bool IsExternal =
        fn->getLinkage() == toolchain::GlobalValue::AvailableExternallyLinkage ||
        (fn->getLinkage() == toolchain::GlobalValue::ExternalLinkage &&
         fn->isDeclaration());

    if (IGM && useDllStorage(IGM->Triple) && IsExternal) {
      bool bIsImported = true;
      language::ASTContext &Context = IGM->Context;
      if (IGM->getCodiraModule()->getPublicModuleName(true).str() == ModuleName)
        bIsImported = false;
      else if (ModuleDecl *MD = Context.getModuleByName(ModuleName))
        bIsImported = !MD->isStaticLibrary();
      else if (strcmp(ModuleName, "BlocksRuntime") == 0)
        bIsImported =
            !static_cast<ClangImporter *>(Context.getClangModuleLoader())
                ->getCodeGenOpts().StaticClosure;

      if (bIsImported)
        fn->setDLLStorageClass(toolchain::GlobalValue::DLLImportStorageClass);
    }

    // Windows does not allow multiple definitions of weak symbols.
    if (IsExternal && isWeakLinked &&
        !toolchain::Triple(Module.getTargetTriple()).isOSWindows())
      fn->setLinkage(toolchain::GlobalValue::ExternalWeakLinkage);

    toolchain::AttrBuilder buildFnAttr(Module.getContext());
    toolchain::AttrBuilder buildRetAttr(Module.getContext());
    toolchain::AttrBuilder buildFirstParamAttr(Module.getContext());

    for (auto Attr : attrs) {
      if (isReturnAttribute(Attr))
        buildRetAttr.addAttribute(Attr);
      else if (isReturnedAttribute(Attr))
        buildFirstParamAttr.addAttribute(Attr);
      else
        buildFnAttr.addAttribute(Attr);
    }

    toolchain::MemoryEffects mergedEffects = mergeMemoryEffects(memEffects);
    if (mergedEffects != toolchain::MemoryEffects::unknown()) {
      buildFnAttr.addAttribute(toolchain::Attribute::getWithMemoryEffects(
          Module.getContext(), mergedEffects));
    }

    fn->addFnAttrs(buildFnAttr);
    fn->addRetAttrs(buildRetAttr);
    fn->addParamAttrs(0, buildFirstParamAttr);

    // Add languageself and languageerror attributes only when language_willThrow
    // language_willThrow is defined in RuntimeFunctions.def, but due to the
    // DSL limitation, arguments attributes are not set.
    // On the other hand, caller of `language_willThrow` assumes that it's attributed
    // with `languageself` and `languageerror`.
    // This mismatch of attributes would be issue when lowering to WebAssembly.
    // While lowering, LLVM counts how many dummy params are necessary to match
    // callee and caller signature. So we need to add them correctly.
    if (name == "language_willThrow") {
      assert(IGM && "IGM is required for language_willThrow.");
      fn->addParamAttr(0, Attribute::AttrKind::CodiraSelf);
      if (IGM->ShouldUseCodiraError) {
        fn->addParamAttr(1, Attribute::AttrKind::CodiraError);
      }
    }
  }

  return cache;
}

toolchain::Constant *IRGenModule::getDeletedAsyncMethodErrorAsyncFunctionPointer() {
  return getAddrOfLLVMVariableOrGOTEquivalent(
      LinkEntity::forKnownAsyncFunctionPointer("language_deletedAsyncMethodError")).getValue();
}

toolchain::Constant *IRGenModule::
    getDeletedCalleeAllocatedCoroutineMethodErrorCoroFunctionPointer() {
  return getAddrOfLLVMVariableOrGOTEquivalent(
             LinkEntity::forKnownCoroFunctionPointer(
                 "language_deletedCalleeAllocatedCoroutineMethodError"))
      .getValue();
}

static bool isReturnAttribute(toolchain::Attribute::AttrKind Attr);
static bool isReturnedAttribute(toolchain::Attribute::AttrKind Attr);

#ifdef CHECK_RUNTIME_EFFECT_ANALYSIS
void IRGenModule::registerRuntimeEffect(ArrayRef<RuntimeEffect> effect,
                                         const char *funcName) {
  for (RuntimeEffect rt : effect) {
    effectOfRuntimeFuncs = RuntimeEffect((unsigned)effectOfRuntimeFuncs |
                                         (unsigned)rt);
    emittedRuntimeFuncs.push_back(funcName);
  }
}
#else
#define registerRuntimeEffect(...)
#endif

#define QUOTE(...) __VA_ARGS__
#define STR(X)     #X

#define FUNCTION(ID, MODULE, NAME, CC, AVAILABILITY, RETURNS, ARGS, ATTRS,     \
                 EFFECT, MEMEFFECTS)                                           \
  FUNCTION_IMPL(ID, MODULE, NAME, CC, AVAILABILITY, QUOTE(RETURNS),            \
                QUOTE(ARGS), QUOTE(ATTRS), QUOTE(EFFECT), QUOTE(MEMEFFECTS))

#define RETURNS(...) { __VA_ARGS__ }
#define ARGS(...) { __VA_ARGS__ }
#define NO_ARGS {}
#define ATTRS(...) { __VA_ARGS__ }
#define NO_ATTRS {}
#define EFFECT(...) { __VA_ARGS__ }
#define UNKNOWN_MEMEFFECTS                                                     \
  {}
#define MEMEFFECTS(...)                                                        \
  { __VA_ARGS__ }

#define FUNCTION_IMPL(ID, MODULE, NAME, CC, AVAILABILITY, RETURNS, ARGS, ATTRS,\
                      EFFECT, MEMEFFECTS)                                      \
  toolchain::Constant *IRGenModule::get##ID##Fn() {                                 \
    using namespace RuntimeConstants;                                          \
    registerRuntimeEffect(EFFECT, #NAME);                                      \
    return getRuntimeFn(Module, ID##Fn, #MODULE, #NAME, CC,                    \
                        AVAILABILITY(this->Context), RETURNS, ARGS, ATTRS,     \
                        MEMEFFECTS, this);                                     \
  }                                                                            \
  FunctionPointer IRGenModule::get##ID##FunctionPointer() {                    \
    using namespace RuntimeConstants;                                          \
    auto fn = get##ID##Fn();                                                   \
    auto fnTy = get##ID##FnType();                                             \
    toolchain::AttributeList attrs;                                                 \
    SmallVector<toolchain::Attribute::AttrKind, 8> theAttrs(ATTRS);                 \
    for (auto Attr : theAttrs) {                                               \
      if (isReturnAttribute(Attr))                                             \
        attrs = attrs.addRetAttribute(getLLVMContext(), Attr);                 \
      else if (isReturnedAttribute(Attr))                                      \
        attrs = attrs.addParamAttribute(getLLVMContext(), 0, Attr);            \
      else                                                                     \
        attrs = attrs.addFnAttribute(getLLVMContext(), Attr);                  \
    }                                                                          \
    toolchain::MemoryEffects effects = mergeMemoryEffects(MEMEFFECTS);              \
    if (effects != toolchain::MemoryEffects::unknown()) {                           \
      attrs = attrs.addFnAttribute(                                            \
          getLLVMContext(),                                                    \
          toolchain::Attribute::getWithMemoryEffects(getLLVMContext(), effects));   \
    }                                                                          \
    auto sig = Signature(fnTy, attrs, CC);                                     \
    return FunctionPointer::forDirect(FunctionPointer::Kind::Function, fn,     \
                                      nullptr, sig);                           \
  }                                                                            \
  toolchain::FunctionType *IRGenModule::get##ID##FnType() {                         \
    return getRuntimeFnType(Module, RETURNS, ARGS);                            \
  }

#include "language/Runtime/RuntimeFunctions.def"

std::pair<toolchain::GlobalVariable *, toolchain::Constant *>
IRGenModule::createStringConstant(StringRef Str, bool willBeRelativelyAddressed,
                                  StringRef sectionName, StringRef name) {
  // If not, create it.  This implicitly adds a trailing null.
  auto init = toolchain::ConstantDataArray::getString(getLLVMContext(), Str);
  auto global = new toolchain::GlobalVariable(Module, init->getType(), true,
                                         toolchain::GlobalValue::PrivateLinkage,
                                         init, name);
  // FIXME: ld64 crashes resolving relative references to coalesceable symbols.
  // rdar://problem/22674524
  // If we intend to relatively address this string, don't mark it with
  // unnamed_addr to prevent it from going into the cstrings section and getting
  // coalesced.
  if (!willBeRelativelyAddressed)
    global->setUnnamedAddr(toolchain::GlobalValue::UnnamedAddr::Global);

  if (!sectionName.empty())
    global->setSection(sectionName);

  // Drill down to make an i8*.
  auto zero = toolchain::ConstantInt::get(SizeTy, 0);
  toolchain::Constant *indices[] = { zero, zero };
  auto address = toolchain::ConstantExpr::getInBoundsGetElementPtr(
    global->getValueType(), global, indices);

  return { global, address };
}

#define KNOWN_METADATA_ACCESSOR(NAME, SYM)                                     \
  toolchain::Constant *IRGenModule::get##NAME() {                                   \
    if (NAME)                                                                  \
      return NAME;                                                             \
    NAME = Module.getOrInsertGlobal(SYM, FullExistentialTypeMetadataStructTy); \
    if (!getCodiraModule()->isStdlibModule() &&                                 \
        !Context.getStdlibModule(true)->isStaticLibrary())                     \
      ApplyIRLinkage(IRLinkage::ExternalImport)                                \
          .to(cast<toolchain::GlobalVariable>(NAME));                               \
    return NAME;                                                               \
  }

KNOWN_METADATA_ACCESSOR(EmptyTupleMetadata,
                        MANGLE_AS_STRING(METADATA_SYM(EMPTY_TUPLE_MANGLING)))
KNOWN_METADATA_ACCESSOR(AnyExistentialMetadata,
                        MANGLE_AS_STRING(METADATA_SYM(ANY_MANGLING)))
KNOWN_METADATA_ACCESSOR(AnyObjectExistentialMetadata,
                        MANGLE_AS_STRING(METADATA_SYM(ANYOBJECT_MANGLING)))

#undef KNOWN_METADATA_ACCESSOR

toolchain::Constant *IRGenModule::getObjCEmptyCachePtr() {
  if (ObjCEmptyCachePtr)
    return ObjCEmptyCachePtr;

  if (ObjCInterop) {
    // struct objc_cache _objc_empty_cache;
    ObjCEmptyCachePtr = Module.getOrInsertGlobal("_objc_empty_cache", OpaqueTy);
    ApplyIRLinkage(IRLinkage::ExternalImport)
        .to(cast<toolchain::GlobalVariable>(ObjCEmptyCachePtr));
  } else {
    // FIXME: Remove even the null value per rdar://problem/18801263
    ObjCEmptyCachePtr = toolchain::ConstantPointerNull::get(OpaquePtrTy);
  }
  return ObjCEmptyCachePtr;
}

toolchain::Constant *IRGenModule::getObjCEmptyVTablePtr() {
  // IMP _objc_empty_vtable;

  // On recent Darwin platforms, this symbol is defined at
  // runtime as an absolute symbol with the value of null. Older ObjCs
  // didn't guarantee _objc_empty_vtable to be nil, but Codira doesn't
  // deploy far enough back for that to be a concern.

  // FIXME: When !ObjCInterop, we should remove even the null value per
  // rdar://problem/18801263

  if (!ObjCEmptyVTablePtr)
    ObjCEmptyVTablePtr = toolchain::ConstantPointerNull::get(OpaquePtrTy);

  return ObjCEmptyVTablePtr;
}

Address IRGenModule::getAddrOfObjCISAMask() {
  // This symbol is only exported by the runtime if the platform uses
  // isa masking.
  assert(TargetInfo.hasISAMasking());
  if (!ObjCISAMaskPtr) {
    ObjCISAMaskPtr = Module.getOrInsertGlobal("language_isaMask", IntPtrTy);
    ApplyIRLinkage(IRLinkage::ExternalImport)
        .to(cast<toolchain::GlobalVariable>(ObjCISAMaskPtr));
  }
  return Address(ObjCISAMaskPtr, IntPtrTy, getPointerAlignment());
}

toolchain::Constant *
IRGenModule::getAddrOfAccessibleFunctionRecord(SILFunction *accessibleFn) {
  auto entity = LinkEntity::forAccessibleFunctionRecord(accessibleFn);
  return getAddrOfLLVMVariable(entity, ConstantInit(), DebugTypeInfo());
}

ModuleDecl *IRGenModule::getCodiraModule() const {
  return IRGen.SIL.getCodiraModule();
}

AvailabilityRange IRGenModule::getAvailabilityRange() const {
  return AvailabilityRange::forDeploymentTarget(Context);
}

Lowering::TypeConverter &IRGenModule::getSILTypes() const {
  return IRGen.SIL.Types;
}

clang::CodeGen::CodeGenModule &IRGenModule::getClangCGM() const {
  return ClangCodeGen->CGM();
}

toolchain::Module *IRGenModule::getModule() const {
  return ClangCodeGen->GetModule();
}

bool IRGenModule::IsWellKnownBuiltinOrStructralType(CanType T) const {
  if (T == Context.TheEmptyTupleType || T == Context.TheNativeObjectType ||
      T == Context.TheBridgeObjectType || T == Context.TheRawPointerType ||
      T == Context.getAnyObjectType())
    return true;

  if (auto IntTy = dyn_cast_or_null<BuiltinIntegerType>(T)) {
    auto Width = IntTy->getWidth();
    if (Width.isPointerWidth())
      return true;
    if (!Width.isFixedWidth())
      return false;
    switch (Width.getFixedWidth()) {
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
      return true;
    default:
      break;
    }
  }

  return false;
}

GeneratedModule IRGenModule::intoGeneratedModule() && {
  return GeneratedModule{
      std::move(LLVMContext),
      std::unique_ptr<toolchain::Module>{ClangCodeGen->ReleaseModule()},
      std::move(TargetMachine), std::move(RemarkStream)};
}

bool IRGenerator::canEmitWitnessTableLazily(SILWitnessTable *wt) {
  if (Opts.UseJIT)
    return false;

  // witness tables are always emitted lazily in embedded language.
  if (SIL.getASTContext().LangOpts.hasFeature(Feature::Embedded))
    return true;

  // Regardless of the access level, if the witness table is shared it means
  // we can safely not emit it. Every other module which needs it will generate
  // its own shared copy of it.
  if (wt->getLinkage() == SILLinkage::Shared)
    return true;

  // Check if this type is set to be explicitly externally visible
  NominalTypeDecl *ConformingTy = wt->getConformingNominal();
  if (PrimaryIGM->getSILModule().isExternallyVisibleDecl(ConformingTy))
    return false;

  switch (wt->getConformingNominal()->getEffectiveAccess()) {
    case AccessLevel::Private:
    case AccessLevel::FilePrivate:
      return true;

    case AccessLevel::Internal:
      return PrimaryIGM->getSILModule().isWholeModule();

    default:
      return false;
  }
  toolchain_unreachable("switch does not handle all cases");
}

void IRGenerator::addLazyWitnessTable(const ProtocolConformance *Conf) {
  // In Embedded Codira, only class-bound wtables are allowed.
  if (SIL.getASTContext().LangOpts.hasFeature(Feature::Embedded)) {
    assert(Conf->getProtocol()->requiresClass());
  }

  if (auto *wt = SIL.lookUpWitnessTable(Conf)) {
    // Add it to the queue if it hasn't already been put there.
    if (canEmitWitnessTableLazily(wt) &&
        LazilyEmittedWitnessTables.insert(wt).second) {
      assert(!FinishedEmittingLazyDefinitions);
      LazyWitnessTables.push_back(wt);
    }
  }
}

void IRGenerator::addClassForEagerInitialization(ClassDecl *ClassDecl) {
  if (!ClassDecl->getAttrs().hasAttribute<StaticInitializeObjCMetadataAttr>())
    return;

  assert(!ClassDecl->isGenericContext());
  assert(!ClassDecl->hasClangNode());

  ClassesForEagerInitialization.push_back(ClassDecl);
}

void IRGenerator::addBackDeployedObjCActorInitialization(ClassDecl *ClassDecl) {
  if (!ClassDecl->isActor())
    return;

  if (!ClassDecl->isObjC())
    return;

  // If we are not back-deploying concurrency, there's nothing to do.
  ASTContext &ctx = ClassDecl->getASTContext();
  auto deploymentAvailability = AvailabilityRange::forDeploymentTarget(ctx);
  if (deploymentAvailability.isContainedIn(ctx.getConcurrencyAvailability()))
    return;

  ObjCActorsNeedingSuperclassSwizzle.push_back(ClassDecl);
}

toolchain::AttributeList IRGenModule::getAllocAttrs() {
  if (AllocAttrs.isEmpty()) {
    AllocAttrs = toolchain::AttributeList().addRetAttribute(
        getLLVMContext(), toolchain::Attribute::NoAlias);
    AllocAttrs =
        AllocAttrs.addFnAttribute(getLLVMContext(), toolchain::Attribute::NoUnwind);
  }
  return AllocAttrs;
}

/// Disable thumb-mode until debugger support is there.
bool language::irgen::shouldRemoveTargetFeature(StringRef feature) {
  return feature == "+thumb-mode";
}

void IRGenModule::setHasNoFramePointer(toolchain::AttrBuilder &Attrs) {
  Attrs.addAttribute("frame-pointer", "non-leaf");
}

void IRGenModule::setHasNoFramePointer(toolchain::Function *F) {
  toolchain::AttrBuilder b(getLLVMContext());
  setHasNoFramePointer(b);
  F->addFnAttrs(b);
}

void IRGenModule::setMustHaveFramePointer(toolchain::Function *F) {
  toolchain::AttrBuilder b(getLLVMContext());
  b.addAttribute("frame-pointer", "all");
  F->addFnAttrs(b);
}

/// Construct initial function attributes from options.
void IRGenModule::constructInitialFnAttributes(
    toolchain::AttrBuilder &Attrs, OptimizationMode FuncOptMode,
    StackProtectorMode stackProtector) {
  // Add the default attributes for the Clang configuration.
  clang::CodeGen::addDefaultFunctionDefinitionAttributes(getClangCGM(), Attrs);

  // Add/remove MinSize based on the appropriate setting.
  if (FuncOptMode == OptimizationMode::NotSet)
    FuncOptMode = IRGen.Opts.OptMode;
  if (FuncOptMode == OptimizationMode::ForSize) {
    Attrs.addAttribute(toolchain::Attribute::OptimizeForSize);
    Attrs.addAttribute(toolchain::Attribute::MinSize);
  } else {
    Attrs.removeAttribute(toolchain::Attribute::MinSize);
    Attrs.removeAttribute(toolchain::Attribute::OptimizeForSize);
  }
  if (stackProtector == StackProtectorMode::StackProtector) {
    Attrs.addAttribute(toolchain::Attribute::StackProtectReq);
    Attrs.addAttribute("stack-protector-buffer-size", toolchain::utostr(8));
  }

  // Mark as 'nounwind' to avoid referencing exception personality symbols, this
  // is okay even with C++ interop on because the landinpads are trapping.
  if (Context.LangOpts.hasFeature(Feature::Embedded)) {
    Attrs.addAttribute(toolchain::Attribute::NoUnwind);
  }
}

toolchain::AttributeList IRGenModule::constructInitialAttributes() {
  toolchain::AttrBuilder b(getLLVMContext());
  constructInitialFnAttributes(b);
  return toolchain::AttributeList().addFnAttributes(getLLVMContext(), b);
}

toolchain::ConstantInt *IRGenModule::getInt32(uint32_t value) {
  return toolchain::ConstantInt::get(Int32Ty, value);
}

toolchain::ConstantInt *IRGenModule::getSize(Size size) {
  return toolchain::ConstantInt::get(SizeTy, size.getValue());
}

toolchain::Constant *IRGenModule::getOpaquePtr(toolchain::Constant *ptr) {
  return toolchain::ConstantExpr::getBitCast(ptr, Int8PtrTy);
}

static void appendEncodedName(raw_ostream &os, StringRef name) {
  if (clang::isValidAsciiIdentifier(name)) {
    os << "_" << name;
  } else {
    for (auto c : name)
      os.write_hex(static_cast<uint8_t>(c));
  }
}

static void appendEncodedName(toolchain::SmallVectorImpl<char> &buf,
                              StringRef name) {
  toolchain::raw_svector_ostream os{buf};
  appendEncodedName(os, name);
}

StringRef
language::irgen::encodeForceLoadSymbolName(toolchain::SmallVectorImpl<char> &buf,
                                        StringRef name) {
  toolchain::raw_svector_ostream os{buf};
  os << "_language_FORCE_LOAD_$";
  appendEncodedName(os, name);
  return os.str();
}

toolchain::SmallString<32> getTargetDependentLibraryOption(const toolchain::Triple &T,
                                                      LinkLibrary library) {
  toolchain::SmallString<32> buffer;
  StringRef name = library.getName();

  if (T.isWindowsMSVCEnvironment() || T.isWindowsItaniumEnvironment()) {
    bool quote = name.contains(' ');

    buffer += "/DEFAULTLIB:";
    if (quote)
      buffer += '"';
    if (library.isStaticLibrary() && !name.starts_with_insensitive("lib"))
      buffer += "lib";
    buffer += name;
    if (!name.ends_with_insensitive(".lib"))
      buffer += ".lib";
    if (quote)
      buffer += '"';
  } else if (T.isPS4()) {
    bool quote = name.contains(' ');

    buffer += "\01";
    if (quote)
      buffer += '"';
    buffer += name;
    if (quote)
      buffer += '"';
  } else {
    buffer += "-l";
    buffer += name;
  }

  return buffer;
}

void IRGenModule::addLinkLibrary(const LinkLibrary &linkLib) {
  // The debugger gets the autolink information directly from
  // the LinkLibraries of the module, so there's no reason to
  // emit it into the IR of debugger expressions.
  if (Context.LangOpts.DebuggerSupport)
    return;

  if (Context.LangOpts.hasFeature(Feature::Embedded))
    return;

  // '-disable-autolinking' means we will not auto-link
  // any loaded library at all.
  if (!IRGen.Opts.DisableAllAutolinking) {
    switch (linkLib.getKind()) {
    case LibraryKind::Library: {
      auto &libraries = IRGen.Opts.DisableAutolinkLibraries;
      if (toolchain::find(libraries, linkLib.getName()) != libraries.end())
	return;
      AutolinkEntries.emplace_back(linkLib);
      break;
    }
    case LibraryKind::Framework: {
      // 'disable-autolink-frameworks' means we will not auto-link
      // any loaded framework.
      if (!IRGen.Opts.DisableFrameworkAutolinking) {
	auto &frameworks = IRGen.Opts.DisableAutolinkFrameworks;
	if (toolchain::find(frameworks, linkLib.getName()) != frameworks.end())
	  return;
	AutolinkEntries.emplace_back(linkLib);
      }
      break;
    }
    }
  }

  if (!IRGen.Opts.DisableForceLoadSymbols && linkLib.shouldForceLoad()) {
    toolchain::SmallString<64> buf;
    encodeForceLoadSymbolName(buf, linkLib.getName());
    auto ForceImportThunk = cast<toolchain::Function>(
        Module.getOrInsertFunction(buf, toolchain::FunctionType::get(VoidTy, false))
            .getCallee());

    const IRLinkage IRL =
        toolchain::Triple(Module.getTargetTriple()).isOSBinFormatCOFF()
            ? IRLinkage::ExternalImport
            : IRLinkage::ExternalWeakImport;
    ApplyIRLinkage(IRL).to(cast<toolchain::GlobalValue>(ForceImportThunk));

    buf += "_$";
    appendEncodedName(buf, IRGen.Opts.ModuleName);

    if (!Module.getGlobalVariable(buf.str())) {
      auto ref = new toolchain::GlobalVariable(Module, ForceImportThunk->getType(),
                                          /*isConstant=*/true,
                                          toolchain::GlobalValue::WeakODRLinkage,
                                          ForceImportThunk, buf.str());
      ApplyIRLinkage(IRLinkage::InternalWeakODR).to(ref);
      auto casted = toolchain::ConstantExpr::getBitCast(ref, Int8PtrTy);
      LLVMUsed.push_back(casted);
    }
  }
}

void IRGenModule::addLinkLibraries() {
  auto registerLinkLibrary = [this](const LinkLibrary &ll) {
    this->addLinkLibrary(ll);
  };

  getCodiraModule()->collectLinkLibraries(
    [registerLinkLibrary](LinkLibrary linkLib) {
      registerLinkLibrary(linkLib);
    });

  if (ObjCInterop)
    registerLinkLibrary(
        LinkLibrary{"objc", LibraryKind::Library, /*static=*/false});

  // If C++ interop is enabled, add -lc++ on Darwin and -lstdc++ on linux.
  // Also link with C++ bridging utility module (Cxx) and C++ stdlib overlay
  // (std) if available.
  if (Context.LangOpts.EnableCXXInterop) {
    bool hasStaticCxx = false;
    bool hasStaticCxxStdlib = false;
    if (const auto *M = Context.getModuleByName(CXX_MODULE_NAME))
      hasStaticCxx = M->isStaticLibrary();
    if (Context.LangOpts.Target.getOS() == toolchain::Triple::Win32)
      if (const auto *M = Context.getModuleByName("CxxStdlib"))
        hasStaticCxxStdlib = M->isStaticLibrary();
    dependencies::registerCxxInteropLibraries(Context.LangOpts.Target,
                                              getCodiraModule()->getName().str(),
                                              hasStaticCxx,
                                              hasStaticCxxStdlib,
                                              Context.LangOpts.CXXStdlib,
                                              registerLinkLibrary);
  }

  // FIXME: It'd be better to have the driver invocation or build system that
  // executes the linker introduce these compatibility libraries, since at
  // that point we know whether we're building an executable, which is the only
  // place where the compatibility libraries take effect. For the benefit of
  // build systems that build Codira code, but don't use Codira to drive
  // the linker, we can also use autolinking to pull in the compatibility
  // libraries. This may however cause the library to get pulled in in
  // situations where it isn't useful, such as for dylibs, though this is
  // harmless aside from code size.
  if (!IRGen.Opts.UseJIT && !Context.LangOpts.hasFeature(Feature::Embedded))
    dependencies::registerBackDeployLibraries(IRGen.Opts, registerLinkLibrary);
}

static bool replaceModuleFlagsEntry(toolchain::LLVMContext &Ctx,
                                    toolchain::Module &Module, StringRef EntryName,
                                    toolchain::Module::ModFlagBehavior Behavior,
                                    toolchain::Metadata *Val) {
  auto *ModuleFlags = Module.getModuleFlagsMetadata();

  for (unsigned I = 0, E = ModuleFlags->getNumOperands(); I != E; ++I) {
    toolchain::MDNode *Op = ModuleFlags->getOperand(I);
    toolchain::MDString *ID = cast<toolchain::MDString>(Op->getOperand(1));

    if (ID->getString() == EntryName) {

      // Create the new entry.
      toolchain::Type *Int32Ty = toolchain::Type::getInt32Ty(Ctx);
      toolchain::Metadata *Ops[3] = {toolchain::ConstantAsMetadata::get(
                                    toolchain::ConstantInt::get(Int32Ty, Behavior)),
                                toolchain::MDString::get(Ctx, EntryName), Val};

      ModuleFlags->setOperand(I, toolchain::MDNode::get(Ctx, Ops));
      return true;
    }
  }
  toolchain_unreachable("Could not replace old linker options entry?");
}

static bool
doesTargetAutolinkUsingAutolinkExtract(const CodiraTargetInfo &TargetInfo,
                                       const toolchain::Triple &Triple) {
  if (TargetInfo.OutputObjectFormat == toolchain::Triple::ELF && !Triple.isPS4())
    return true;

  if (TargetInfo.OutputObjectFormat == toolchain::Triple::Wasm)
    return true;

  if (Triple.isOSCygMing())
    return true;

  return false;
}

namespace {

struct AutolinkKind {
  enum ValueTy {
    LLVMLinkerOptions,
    LLVMDependentLibraries,
    CodiraAutoLinkExtract,
  };

  ValueTy Value;

  AutolinkKind(ValueTy value) : Value(value) {}
  AutolinkKind(const AutolinkKind &kind) : Value(kind.Value) {}

  StringRef getSectionNameMetadata();

  template <typename Vector, typename Set>
  void collectEntriesFromLibraries(toolchain::SetVector<toolchain::MDNode *, Vector, Set> &Entries,
                                   ArrayRef<LinkLibrary> AutolinkEntries,
                                   IRGenModule &IGM);

  template <typename Vector, typename Set>
  void writeEntries(toolchain::SetVector<toolchain::MDNode *, Vector, Set> Entries,
                    toolchain::NamedMDNode *Metadata, IRGenModule &IGM);

  static AutolinkKind create(const CodiraTargetInfo &TargetInfo,
                             toolchain::Triple Triple, IRGenLLVMLTOKind LLVMLTOKind);
};

} // anonymous namespace

StringRef AutolinkKind::getSectionNameMetadata() {
  // FIXME: This constant should be vended by LLVM somewhere.
  switch (Value) {
  case AutolinkKind::LLVMDependentLibraries:
    return "toolchain.dependent-libraries";
  case AutolinkKind::LLVMLinkerOptions:
  case AutolinkKind::CodiraAutoLinkExtract:
    return "toolchain.linker.options";
  }

  toolchain_unreachable("Unhandled AutolinkKind in switch.");
}

template <typename Vector, typename Set>
void AutolinkKind::collectEntriesFromLibraries(
    toolchain::SetVector<toolchain::MDNode *, Vector, Set> &Entries,
    ArrayRef<LinkLibrary> AutolinkEntries, IRGenModule &IGM) {
  toolchain::LLVMContext &ctx = IGM.getLLVMContext();

  switch (Value) {
  case AutolinkKind::LLVMLinkerOptions: {
    // On platforms that support autolinking, continue to use the metadata.
    for (LinkLibrary linkLib : AutolinkEntries) {
      switch (linkLib.getKind()) {
      case LibraryKind::Library: {
        toolchain::SmallString<32> opt =
            getTargetDependentLibraryOption(IGM.Triple, linkLib);
        Entries.insert(toolchain::MDNode::get(ctx, toolchain::MDString::get(ctx, opt)));
        continue;
      }
      case LibraryKind::Framework: {
        toolchain::Metadata *args[] = {toolchain::MDString::get(ctx, "-framework"),
                                  toolchain::MDString::get(ctx, linkLib.getName())};
        Entries.insert(toolchain::MDNode::get(ctx, args));
        continue;
      }
      }
      toolchain_unreachable("Unhandled LibraryKind in switch.");
    }
    return;
  }
  case AutolinkKind::CodiraAutoLinkExtract: {
    // On platforms that support autolinking, continue to use the metadata.
    for (LinkLibrary linkLib : AutolinkEntries) {
      switch (linkLib.getKind()) {
      case LibraryKind::Library: {
        toolchain::SmallString<32> opt =
            getTargetDependentLibraryOption(IGM.Triple, linkLib);
        Entries.insert(toolchain::MDNode::get(ctx, toolchain::MDString::get(ctx, opt)));
        continue;
      }
      case LibraryKind::Framework:
        // Frameworks are not supported with Codira Autolink Extract.
        continue;
      }
      toolchain_unreachable("Unhandled LibraryKind in switch.");
    }
    return;
  }
  case AutolinkKind::LLVMDependentLibraries: {
    for (LinkLibrary linkLib : AutolinkEntries) {
      switch (linkLib.getKind()) {
      case LibraryKind::Library: {
        Entries.insert(toolchain::MDNode::get(
            ctx, toolchain::MDString::get(ctx, linkLib.getName())));
        continue;
      }
      case LibraryKind::Framework: {
        toolchain_unreachable(
            "toolchain.dependent-libraries doesn't support framework dependency");
      }
      }
      toolchain_unreachable("Unhandled LibraryKind in switch.");
    }
    return;
  }
  }
  toolchain_unreachable("Unhandled AutolinkKind in switch.");
}

template <typename Vector, typename Set>
void AutolinkKind::writeEntries(toolchain::SetVector<toolchain::MDNode *, Vector, Set> Entries,
                                toolchain::NamedMDNode *Metadata, IRGenModule &IGM) {
  switch (Value) {
  case AutolinkKind::LLVMLinkerOptions:
  case AutolinkKind::LLVMDependentLibraries: {
    // On platforms that support autolinking, continue to use the metadata.
    Metadata->clearOperands();
    for (auto *Entry : Entries)
      Metadata->addOperand(Entry);
    return;
  }
  case AutolinkKind::CodiraAutoLinkExtract: {
    // Merge the entries into null-separated string.
    toolchain::SmallString<64> EntriesString;
    for (auto EntryNode : Entries) {
      const auto *MD = cast<toolchain::MDNode>(EntryNode);
      for (auto &Entry : MD->operands()) {
        const toolchain::MDString *MS = cast<toolchain::MDString>(Entry);
        EntriesString += MS->getString();
        EntriesString += '\0';
      }
    }
    auto EntriesConstant = toolchain::ConstantDataArray::getString(
        IGM.getLLVMContext(), EntriesString, /*AddNull=*/false);
    auto var =
        new toolchain::GlobalVariable(*IGM.getModule(), EntriesConstant->getType(),
                                 true, toolchain::GlobalValue::PrivateLinkage,
                                 EntriesConstant, "_language1_autolink_entries");
    var->setSection(".code1_autolink_entries");
    var->setAlignment(toolchain::MaybeAlign(IGM.getPointerAlignment().getValue()));

    disableAddressSanitizer(IGM, var);
    IGM.addUsedGlobal(var);
    return;
  }
  }
  toolchain_unreachable("Unhandled AutolinkKind in switch.");
}

AutolinkKind AutolinkKind::create(const CodiraTargetInfo &TargetInfo,
                                  toolchain::Triple Triple,
                                  IRGenLLVMLTOKind LLVMLTOKind) {
  // When performing LTO, we always use lld that supports auto linking
  // mechanism with ELF. So embed dependent libraries names in
  // "toolchain.dependent-libraries" instead of "toolchain.linker.options".
  if (TargetInfo.OutputObjectFormat == toolchain::Triple::ELF &&
      LLVMLTOKind != IRGenLLVMLTOKind::None) {
    return AutolinkKind::LLVMDependentLibraries;
  }

  if (doesTargetAutolinkUsingAutolinkExtract(TargetInfo, Triple)) {
    return AutolinkKind::CodiraAutoLinkExtract;
  }

  return AutolinkKind::LLVMLinkerOptions;
}

static toolchain::GlobalObject *createForceImportThunk(IRGenModule &IGM) {
  toolchain::SmallString<64> buf;
  encodeForceLoadSymbolName(buf, IGM.IRGen.Opts.ForceLoadSymbolName);
  if (IGM.Triple.isOSBinFormatMachO()) {
    // On Mach-O targets, emit a common force-load symbol that resolves to
    // a global variable so it will be coalesced at static link time into a
    // single external symbol.
    //
    // Looks like C's tentative definitions are good for something after all.
    auto ForceImportThunk =
        new toolchain::GlobalVariable(IGM.Module,
                                 IGM.Int1Ty,
                                 /*isConstant=*/false,
                                 toolchain::GlobalValue::CommonLinkage,
                                 toolchain::Constant::getNullValue(IGM.Int1Ty),
                                 buf.str());
    ApplyIRLinkage(IRLinkage::ExternalCommon).to(ForceImportThunk);
    IGM.addUsedGlobal(ForceImportThunk);
    return ForceImportThunk;
  } else {
    // On all other targets, emit an external symbol that resolves to a
    // function definition. On Windows, linkonce_odr is basically a no-op and
    // the COMDAT we set by applying linkage gives us the desired coalescing
    // behavior.
    auto ForceImportThunk =
        toolchain::Function::Create(toolchain::FunctionType::get(IGM.VoidTy, false),
                               toolchain::GlobalValue::LinkOnceODRLinkage, buf,
                               &IGM.Module);
    ForceImportThunk->setAttributes(IGM.constructInitialAttributes());
    ApplyIRLinkage(IGM.IRGen.Opts.InternalizeSymbols
                      ? IRLinkage::Internal
                      : IRLinkage::ExternalExport).to(ForceImportThunk);
    if (IGM.Triple.supportsCOMDAT())
      if (auto *GO = cast<toolchain::GlobalObject>(ForceImportThunk))
        GO->setComdat(IGM.Module.getOrInsertComdat(ForceImportThunk->getName()));

    auto BB = toolchain::BasicBlock::Create(IGM.getLLVMContext(), "", ForceImportThunk);
    toolchain::IRBuilder<> IRB(BB);
    IRB.CreateRetVoid();
    IGM.addUsedGlobal(ForceImportThunk);
    return ForceImportThunk;
  }
}

void IRGenModule::emitAutolinkInfo() {
  if (getSILModule().getOptions().StopOptimizationAfterSerialization) {
    // We're asked to emit an empty IR module
    return;
  }

  auto Autolink =
      AutolinkKind::create(TargetInfo, Triple, IRGen.Opts.LLVMLTOKind);

  StringRef AutolinkSectionName = Autolink.getSectionNameMetadata();

  auto *Metadata = Module.getOrInsertNamedMetadata(AutolinkSectionName);
  language::SmallSetVector<toolchain::MDNode *, 4> Entries;

  // Collect the linker options already in the module (from ClangCodeGen).
  for (auto Entry : Metadata->operands()) {
    Entries.insert(Entry);
  }

  Autolink.collectEntriesFromLibraries(Entries, AutolinkEntries, *this);

  Autolink.writeEntries(Entries, Metadata, *this);

  if (!IRGen.Opts.ForceLoadSymbolName.empty()) {
    (void) createForceImportThunk(*this);
  }
}

void emitCodiraVersionNumberIntoModule(toolchain::Module *Module) {
  // LLVM's object-file emission collects a fixed set of keys for the
  // image info.
  // Using "Objective-C Garbage Collection" as the key here is a hack,
  // but LLVM's object-file emission isn't general enough to collect
  // arbitrary keys to put in the image info.

  const char *ObjectiveCGarbageCollection = "Objective-C Garbage Collection";
  uint8_t Major, Minor;
  std::tie(Major, Minor) = version::getCodiraNumericVersion();
  uint32_t Value =
      (Major << 24) | (Minor << 16) | (IRGenModule::languageVersion << 8);
  auto &toolchainContext = Module->getContext();
  if (Module->getModuleFlag(ObjectiveCGarbageCollection)) {
    bool FoundOldEntry = replaceModuleFlagsEntry(
        toolchainContext, *Module, ObjectiveCGarbageCollection,
        toolchain::Module::Override,
        toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(
            toolchain::Type::getInt32Ty(toolchainContext), Value)));

    (void)FoundOldEntry;
    assert(FoundOldEntry && "Could not replace old module flag entry?");
  } else
    Module->addModuleFlag(toolchain::Module::Override, ObjectiveCGarbageCollection,
                          Value);
}

void IRGenModule::cleanupClangCodeGenMetadata() {
  // Remove toolchain.ident that ClangCodeGen might have left in the module.
  auto *LLVMIdent = Module.getNamedMetadata("toolchain.ident");
  if (LLVMIdent)
    Module.eraseNamedMetadata(LLVMIdent);
  emitCodiraVersionNumberIntoModule(&Module);
}

bool IRGenModule::finalize() {
  const char *ModuleHashVarName = "toolchain.code_module_hash";
  if (IRGen.Opts.OutputKind == IRGenOutputKind::ObjectFile &&
      !Module.getGlobalVariable(ModuleHashVarName) &&
      !getSILModule().getOptions().StopOptimizationAfterSerialization) {
    // Create a global variable into which we will store the hash of the
    // module (used for incremental compilation).
    // We have to create the variable now (before we emit the global lists).
    // But we want to calculate the hash later because later we can do it
    // multi-threaded.
    toolchain::MD5::MD5Result zero{};
    ArrayRef<uint8_t> ZeroArr(reinterpret_cast<uint8_t *>(&zero), sizeof(zero));
    auto *ZeroConst = toolchain::ConstantDataArray::get(Module.getContext(), ZeroArr);
    ModuleHash = new toolchain::GlobalVariable(Module, ZeroConst->getType(), true,
                                          toolchain::GlobalValue::PrivateLinkage,
                                          ZeroConst, ModuleHashVarName);
    switch (TargetInfo.OutputObjectFormat) {
    case toolchain::Triple::MachO:
      // On Darwin the linker ignores the __LLVM segment.
      ModuleHash->setSection("__LLVM,__language_modhash");
      break;
    case toolchain::Triple::ELF:
    case toolchain::Triple::Wasm:
      ModuleHash->setSection(".code_modhash");
      break;
    case toolchain::Triple::COFF:
      ModuleHash->setSection(".sw5hash");
      break;
    default:
      toolchain_unreachable("Don't know how to emit the module hash for the selected"
                       "object format.");
    }
    addUsedGlobal(ModuleHash);
  }
  emitLazyPrivateDefinitions();

  // Finalize Codira debug info before running Clang codegen, because it may
  // delete the toolchain module.
  if (DebugInfo)
    DebugInfo->finalize();

  // Finalize clang IR-generation.
  finalizeClangCodeGen();

  // If that failed, report failure up and skip the final clean-up.
  if (!ClangCodeGen->GetModule())
    return false;

  emitCodiraAsyncExtendedFrameInfoWeakRef();
  emitAutolinkInfo();
  emitGlobalLists();
  emitUsedConditionals();
  cleanupClangCodeGenMetadata();

  // Clean up DSOLocal & DLLImport attributes, they cannot be applied together.
  // The imported declarations are marked as DSO local by default.
  for (auto &GV : Module.globals())
    if (GV.hasDLLImportStorageClass())
      GV.setDSOLocal(false);

  for (auto &F : Module.functions())
    if (F.hasDLLImportStorageClass())
      F.setDSOLocal(false);

  if (getSILModule().getOptions().StopOptimizationAfterSerialization) {
    // We're asked to emit an empty IR module, check that that's actually true
    if (Module.global_size() != 0 || Module.size() != 0) {
      toolchain::errs() << Module;
      toolchain::report_fatal_error("Module is not empty");
    }
  }

  return true;
}

/// Emit lazy definitions that have to be emitted in this specific
/// IRGenModule.
void IRGenModule::emitLazyPrivateDefinitions() {
  emitLazyObjCProtocolDefinitions();
}

toolchain::MDNode *IRGenModule::createProfileWeights(uint64_t TrueCount,
                                                uint64_t FalseCount) const {
  uint64_t MaxWeight = std::max(TrueCount, FalseCount);
  uint64_t Scale = (MaxWeight > UINT32_MAX) ? UINT32_MAX : 1;
  uint32_t ScaledTrueCount = (TrueCount / Scale) + 1;
  uint32_t ScaledFalseCount = (FalseCount / Scale) + 1;
  toolchain::MDBuilder MDHelper(getLLVMContext());
  return MDHelper.createBranchWeights(ScaledTrueCount, ScaledFalseCount);
}

void IRGenModule::unimplemented(SourceLoc loc, StringRef message) {
  Context.Diags.diagnose(loc, diag::irgen_unimplemented, message);
}

void IRGenModule::fatal_unimplemented(SourceLoc loc, StringRef message) {
  Context.Diags.diagnose(loc, diag::irgen_unimplemented, message);
  toolchain::report_fatal_error(toolchain::Twine("unimplemented IRGen feature! ") +
                             message);
}

void IRGenModule::error(SourceLoc loc, const Twine &message) {
  SmallVector<char, 128> buffer;
  Context.Diags.diagnose(loc, diag::irgen_failure,
                         message.toStringRef(buffer));
}

// In embedded language features are available independent of deployment and
// runtime targets because the runtime library is always statically linked
// to the program.

#define FEATURE(N, V)                                                          \
  bool IRGenModule::is##N##FeatureAvailable(const ASTContext &context) {       \
    if (Context.LangOpts.hasFeature(Feature::Embedded))                        \
      return true;                                                             \
    auto deploymentAvailability =                                              \
        AvailabilityRange::forDeploymentTarget(context);                       \
    auto runtimeAvailability = AvailabilityRange::forRuntimeTarget(context);   \
    return deploymentAvailability.isContainedIn(                               \
               context.get##N##Availability()) &&                              \
           runtimeAvailability.isContainedIn(                                  \
               context.get##N##RuntimeAvailability());                         \
  }

#include "language/AST/FeatureAvailability.def"

bool IRGenModule::shouldPrespecializeGenericMetadata() {
  auto canPrespecializeTarget =
      (Triple.isOSDarwin() || Triple.isOSWindows() ||
       (Triple.isOSLinux() && !(Triple.isARM() && Triple.isArch32Bit())));
  if (canPrespecializeTarget && getCodiraModule()->isStdlibModule())
    return IRGen.Opts.PrespecializeGenericMetadata;

  auto &context = getCodiraModule()->getASTContext();
  auto deploymentAvailability = AvailabilityRange::forDeploymentTarget(context);
  return IRGen.Opts.PrespecializeGenericMetadata &&
         deploymentAvailability.isContainedIn(
             context.getPrespecializedGenericMetadataAvailability()) &&
         canPrespecializeTarget;
}

bool IRGenModule::canUseObjCSymbolicReferences() {
  if (!IRGen.Opts.EnableObjectiveCProtocolSymbolicReferences)
    return false;
  return isObjCSymbolicReferencesFeatureAvailable(
    getCodiraModule()->getASTContext()
  );
}

bool IRGenModule::canMakeStaticObjectReadOnly(SILType objectType) {
  if (getOptions().DisableReadonlyStaticObjects)
    return false;

  // TODO: Support constant static arrays on other platforms, too.
  // See also the comment in GlobalObjects.cpp.
  if (!Triple.isOSDarwin() && !Context.LangOpts.hasFeature(Feature::Embedded))
    return false;

  auto *clDecl = objectType.getClassOrBoundGenericClass();
  if (!clDecl)
    return false;

  // Currently only arrays can be put into a read-only data section.
  // "Regular" classes have dynamically initialized metadata, which needs to be
  // stored into the isa field at runtime.
  if (clDecl->getNameStr() != "_ContiguousArrayStorage")
    return false;

  if (!isStaticReadOnlyArraysFeatureAvailable())
    return false;

  if (!getStaticArrayStorageDecl())
    return false;

  return true;
}

ClassDecl *IRGenModule::getStaticArrayStorageDecl() {
  SmallVector<ValueDecl *, 1> results;
  Context.lookupInCodiraModule("__StaticArrayStorage", results);

  if (results.size() != 1)
    return nullptr;

  return dyn_cast<ClassDecl>(results[0]);
}

void IRGenerator::addGenModule(SourceFile *SF, IRGenModule *IGM) {
  assert(GenModules.count(SF) == 0);
  GenModules[SF] = IGM;
  if (!PrimaryIGM) {
    PrimaryIGM = IGM;
  }
  Queue.push_back(IGM);
}

IRGenModule *IRGenerator::getGenModule(SourceFile *SF) {
  // If we're emitting for a single module, or a single file, we always use the
  // primary IGM.
  if (GenModules.size() == 1)
    return getPrimaryIGM();

 IRGenModule *IGM = GenModules[SF];
 assert(IGM);
 return IGM;
}

IRGenModule *IRGenerator::getGenModule(DeclContext *ctxt) {
  if (GenModules.size() == 1 || !ctxt) {
    return getPrimaryIGM();
  }
  SourceFile *SF = ctxt->getOutermostParentSourceFile();
  if (!SF) {
    return getPrimaryIGM();
  }

  IRGenModule *IGM = GenModules[SF];
  assert(IGM);
  return IGM;
}

IRGenModule *IRGenerator::getGenModule(SILFunction *f) {
  if (GenModules.size() == 1) {
    return getPrimaryIGM();
  }

  auto found = DefaultIGMForFunction.find(f);
  if (found != DefaultIGMForFunction.end())
    return found->second;

  if (auto *dc = f->getDeclContext())
    return getGenModule(dc);

  return getPrimaryIGM();
}

uint32_t language::irgen::getCodiraABIVersion() {
  return IRGenModule::languageVersion;
}

toolchain::Triple IRGenerator::getEffectiveClangTriple() {
  auto CI = static_cast<ClangImporter *>(
      &*SIL.getASTContext().getClangModuleLoader());
  assert(CI && "no clang module loader");
  return toolchain::Triple(CI->getTargetInfo().getTargetOpts().Triple);
}

toolchain::Triple IRGenerator::getEffectiveClangVariantTriple() {
  auto CI = static_cast<ClangImporter *>(
      &*SIL.getASTContext().getClangModuleLoader());
  assert(CI && "no clang module loader");
  return toolchain::Triple(CI->getTargetInfo().getTargetOpts().DarwinTargetVariantTriple);
}

const toolchain::StringRef IRGenerator::getClangDataLayoutString() {
  return static_cast<ClangImporter *>(
             SIL.getASTContext().getClangModuleLoader())
      ->getTargetInfo()
      .getDataLayoutString();
}

TypeExpansionContext IRGenModule::getMaximalTypeExpansionContext() const {
  return TypeExpansionContext::maximal(getSILModule().getAssociatedContext(),
                                       getSILModule().isWholeModule());
}

const TypeLayoutEntry
&IRGenModule::getTypeLayoutEntry(SILType T, bool useStructLayouts) {
  return Types.getTypeLayoutEntry(T, useStructLayouts);
}


void IRGenModule::emitCodiraAsyncExtendedFrameInfoWeakRef() {
  if (!hasCodiraAsyncFunctionDef || extendedFramePointerFlagsWeakRef)
    return;
  if (IRGen.Opts.CodiraAsyncFramePointer !=
      CodiraAsyncFramePointerKind::Auto)
    return;
  if (isConcurrencyAvailable())
    return;

  // Emit a weak reference to the `language_async_extendedFramePointerFlags` symbol
  // needed by Codira async functions.
  auto symbolName = "language_async_extendedFramePointerFlags";
  if ((extendedFramePointerFlagsWeakRef = Module.getGlobalVariable(symbolName)))
    return;
  extendedFramePointerFlagsWeakRef = new toolchain::GlobalVariable(Module, Int8PtrTy, false,
                                         toolchain::GlobalValue::ExternalWeakLinkage, nullptr,
                                         symbolName);

  // The weak imported extendedFramePointerFlagsWeakRef gets optimized out
  // before being added back as a strong import.
  // Declarations can't be added to the used list, so we create a little
  // global that can't be used from the program, but can be in the used list to
  // avoid optimizations.
  toolchain::GlobalVariable *usage = new toolchain::GlobalVariable(
      Module, extendedFramePointerFlagsWeakRef->getType(), false,
      toolchain::GlobalValue::LinkOnceODRLinkage,
      static_cast<toolchain::GlobalVariable *>(extendedFramePointerFlagsWeakRef),
      "_language_async_extendedFramePointerFlagsUser");
  usage->setVisibility(toolchain::GlobalValue::HiddenVisibility);
  addUsedGlobal(usage);
}

bool IRGenModule::isConcurrencyAvailable() {
  auto &ctx = getCodiraModule()->getASTContext();
  auto deploymentAvailability = AvailabilityRange::forDeploymentTarget(ctx);
  return deploymentAvailability.isContainedIn(ctx.getConcurrencyAvailability());
}

/// Pretend the other files that drivers/build systems expect exist by
/// creating empty files. Used by UseSingleModuleLLVMEmission when
/// num-threads > 0.
bool language::writeEmptyOutputFilesFor(
  const ASTContext &Context,
  std::vector<std::string>& ParallelOutputFilenames,
  const IRGenOptions &IRGenOpts) {
  for (auto fileName : ParallelOutputFilenames) {
    // The first output file, was use for genuine output.
    if (fileName == ParallelOutputFilenames[0])
      continue;

    std::unique_ptr<toolchain::LLVMContext> toolchainContext(new toolchain::LLVMContext());
    std::unique_ptr<clang::CodeGenerator> clangCodeGen(
      createClangCodeGenerator(const_cast<ASTContext&>(Context),
                               *toolchainContext, IRGenOpts, fileName, ""));
    auto *toolchainModule = clangCodeGen->GetModule();

    auto *clangImporter = static_cast<ClangImporter *>(
      Context.getClangModuleLoader());
    toolchainModule->setTargetTriple(
      clangImporter->getTargetInfo().getTargetOpts().Triple);

    // Add LLVM module flags.
    auto &clangASTContext = clangImporter->getClangASTContext();
    clangCodeGen->HandleTranslationUnit(
        const_cast<clang::ASTContext &>(clangASTContext));

    emitCodiraVersionNumberIntoModule(toolchainModule);

    language::performLLVM(IRGenOpts, const_cast<ASTContext&>(Context),
                       toolchainModule, fileName);
  }
  return false;
}
