//===--- ImportType.cpp - Import Clang Types ------------------------------===//
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
// This file implements support for importing Clang types as Codira types.
//
//===----------------------------------------------------------------------===//

#include "CFTypeInfo.h"
#include "ImportEnumInfo.h"
#include "ImporterImpl.h"
#include "CodiraDeclSynthesizer.h"
#include "language/ABI/MetadataValues.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/DefaultArgumentKind.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsClangImporter.h"
#include "language/AST/ExistentialLayout.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/GenericParamList.h"
#include "language/AST/GenericSignature.h"
#include "language/AST/Module.h"
#include "language/AST/NameLookup.h"
#include "language/AST/ParameterList.h"
#include "language/AST/PrettyStackTrace.h"
#include "language/AST/Type.h"
#include "language/AST/TypeVisitor.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "language/ClangImporter/ClangImporterRequests.h"
#include "language/ClangImporter/ClangModule.h"
#include "language/Parse/Token.h"
#include "language/Strings.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjCCommon.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/TypeVisitor.h"
#include "clang/Basic/Builtins.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Sema.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/Support/Compiler.h"

using namespace language;
using namespace importer;

// XXX: This is to resolve the build dependency with Clang. Remove it once these
// types actually land in Clang.
namespace clang {
class DynamicRangePointerType;
class ValueTerminatedType;
}

/// Given that a type is the result of a special typedef import, was
/// it originally a CF pointer?
static bool isImportedCFPointer(clang::QualType clangType, Type type) {
  return (clangType->isPointerType() &&
          (type->is<ClassType>() || type->isClassExistentialType()));
}

bool ClangImporter::Implementation::isOverAligned(const clang::TypeDecl *decl) {
  auto type = getClangASTContext().getTypeDeclType(decl);
  return isOverAligned(type);
}

bool ClangImporter::Implementation::isOverAligned(clang::QualType type) {
  auto align = getClangASTContext().getTypeAlignInChars(type);
  return align > clang::CharUnits::fromQuantity(MaximumAlignment);
}

namespace {
  /// Various types that we want to do something interesting to after
  /// importing them.
  struct ImportHint {
    enum ImportHintKind {
      /// There is nothing special about the source type.
      None,

      /// The source type is 'void'.
      Void,

      /// The source type is 'BOOL' or 'Boolean' -- a type mapped to Codira's
      /// 'Bool'.
      Boolean,

      /// The source type is 'NSUInteger'.
      NSUInteger,

      /// The source type is 'va_list'.
      VAList,

      /// The source type is an Objective-C class type bridged to a Codira
      /// type.
      ObjCBridged,

      /// The source type is an Objective-C object pointer type.
      ObjCPointer,

      /// The source type is a CF object pointer type.
      CFPointer,

      /// The source type is a block pointer type.
      Block,

      /// The source type is a function pointer type.
      CFunctionPointer,

      /// The source type is any other pointer type.
      OtherPointer,
    };

    ImportHintKind Kind;

    /// The type to which the imported type is bridged.
    Type BridgedType;

    /// Allow conversion from an import hint to an import hint kind,
    /// which is useful for switches and comparisons.
    operator ImportHintKind() const { return Kind; }

    ImportHint(ImportHintKind kind) : Kind(kind) {
      assert(kind != ObjCBridged &&
             "Bridged entry point requires a bridged type");
    }

    ImportHint(ImportHintKind kind, Type bridgedType)
        : Kind(kind), BridgedType(bridgedType) {
      assert(kind == ImportHint::ObjCBridged && "Wrong kind for bridged type");
    }
  };

  bool canImportAsOptional(ImportHint hint) {
    // See also ClangImporter.cpp's canImportAsOptional.
    switch (hint) {
    case ImportHint::None:
    case ImportHint::Boolean:
    case ImportHint::NSUInteger:
    case ImportHint::Void:
      return false;

    case ImportHint::Block:
    case ImportHint::CFPointer:
    case ImportHint::ObjCBridged:
    case ImportHint::ObjCPointer:
    case ImportHint::CFunctionPointer:
    case ImportHint::OtherPointer:
    case ImportHint::VAList:
      return true;
    }

    toolchain_unreachable("Invalid ImportHint.");
  }

  struct ImportResult {
    Type AbstractType;
    ImportHint Hint;

    /*implicit*/ ImportResult(Type type = Type(),
                              ImportHint hint = ImportHint::None)
      : AbstractType(type), Hint(hint) {}

    /*implicit*/ ImportResult(TypeBase *type,
                              ImportHint hint = ImportHint::None)
      : AbstractType(type), Hint(hint) {}

    explicit operator bool() const { return (bool) AbstractType; }
  };

  static ImportResult importFunctionPointerLikeType(const clang::Type &type,
                                                    const Type &pointeeType) {
    auto funcTy = pointeeType->castTo<FunctionType>();
    return {FunctionType::get(
                funcTy->getParams(), funcTy->getResult(),
                funcTy->getExtInfo()
                    .intoBuilder()
                    .withRepresentation(
                        AnyFunctionType::Representation::CFunctionPointer)
                    .withClangFunctionType(&type)
                    .build()),
            type.isReferenceType() ? ImportHint::None
                                   : ImportHint::CFunctionPointer};
  }

  static ImportResult importOverAlignedFunctionPointerLikeType(
      const clang::Type &type, ClangImporter::Implementation &Impl) {
    auto opaquePointer = Impl.CodiraContext.getOpaquePointerDecl();
    if (!opaquePointer) {
      return Type();
    }
    return {opaquePointer->getDeclaredInterfaceType(),
            type.isReferenceType() ? ImportHint::None
                                    : ImportHint::OtherPointer};
  }

  static bool
  isDirectUseOfForeignReferenceType(clang::QualType clangPointeeType,
                                    Type languagePointeeType) {
     return languagePointeeType && languagePointeeType->isForeignReferenceType() &&
               !clangPointeeType->isPointerType();
   }

  class CodiraTypeConverter :
    public clang::TypeVisitor<CodiraTypeConverter, ImportResult>
  {
    ClangImporter::Implementation &Impl;
    toolchain::function_ref<void(Diagnostic &&)> addImportDiagnostic;
    bool AllowNSUIntegerAsInt;
    Bridgeability Bridging;
    const clang::FunctionType *CompletionHandlerType;
    std::optional<unsigned> CompletionHandlerErrorParamIndex;

  public:
    CodiraTypeConverter(ClangImporter::Implementation &impl,
                       toolchain::function_ref<void(Diagnostic &&)> addDiag,
                       bool allowNSUIntegerAsInt, Bridgeability bridging,
                       const clang::FunctionType *completionHandlerType,
                       std::optional<unsigned> completionHandlerErrorParamIndex)
        : Impl(impl), addImportDiagnostic(addDiag),
          AllowNSUIntegerAsInt(allowNSUIntegerAsInt), Bridging(bridging),
          CompletionHandlerType(completionHandlerType),
          CompletionHandlerErrorParamIndex(completionHandlerErrorParamIndex) {}

    using TypeVisitor::Visit;
    ImportResult Visit(clang::QualType type) {
      PrettyStackTraceClangType trace(Impl.getClangASTContext(),
                                      "importing a clang type",
                                      type.getTypePtr());

      auto IR = Visit(type.getTypePtr());
      return IR;
    }

    ImportResult VisitType(const Type*) = delete;

    // TODO(https://github.com/apple/language/issues/56206): Add support for dependent types.
#define DEPENDENT_TYPE(Class, Base)                                            \
  ImportResult Visit##Class##Type(const clang::Class##Type *) { return Impl.CodiraContext.getAnyExistentialType(); }
#define TYPE(Class, Base)
#include "clang/AST/TypeNodes.inc"

    // Given a loaded type like CInt, look through the type alias sugar that the
    // stdlib uses to show the underlying type.  We want to import the signature
    // of the exit(3) libc function as "fn exit(Int32)", not as
    // "fn exit(CInt)".
    static Type unwrapCType(Type T) {
      // Handle missing or invalid stdlib declarations
      if (!T || T->hasError())
        return Type();
      if (auto *NAT = dyn_cast<TypeAliasType>(T.getPointer()))
        return NAT->getSinglyDesugaredType();
      return T;
    }

    ImportResult VisitBuiltinType(const clang::BuiltinType *type) {
      switch (type->getKind()) {
      case clang::BuiltinType::Void:
        return { Type(), ImportHint::Void };

#define MAP_BUILTIN_TYPE(CLANG_BUILTIN_KIND, LANGUAGE_TYPE_NAME)             \
      case clang::BuiltinType::CLANG_BUILTIN_KIND:                        \
        return unwrapCType(Impl.getNamedCodiraType(Impl.getStdlibModule(), \
                                        #LANGUAGE_TYPE_NAME));
#define MAP_BUILTIN_CCHAR_TYPE(CLANG_BUILTIN_KIND, LANGUAGE_TYPE_NAME)       \
      case clang::BuiltinType::CLANG_BUILTIN_KIND:                        \
        return Impl.getNamedCodiraType(Impl.getStdlibModule(), #LANGUAGE_TYPE_NAME);
#include "language/ClangImporter/BuiltinMappedTypes.def"

      // Types that cannot be mapped into Codira, and probably won't ever be.
      case clang::BuiltinType::Dependent:
      case clang::BuiltinType::ARCUnbridgedCast:
      case clang::BuiltinType::BoundMember:
      case clang::BuiltinType::BuiltinFn:
      case clang::BuiltinType::IncompleteMatrixIdx:
      case clang::BuiltinType::Overload:
      case clang::BuiltinType::PseudoObject:
      case clang::BuiltinType::UnknownAny:
      case clang::BuiltinType::UnresolvedTemplate:
        return Type();

      // FIXME: Types that can be mapped, but aren't yet.
      case clang::BuiltinType::ShortAccum:
      case clang::BuiltinType::Accum:
      case clang::BuiltinType::LongAccum:
      case clang::BuiltinType::UShortAccum:
      case clang::BuiltinType::UAccum:
      case clang::BuiltinType::ULongAccum:
      case clang::BuiltinType::ShortFract:
      case clang::BuiltinType::Fract:
      case clang::BuiltinType::LongFract:
      case clang::BuiltinType::UShortFract:
      case clang::BuiltinType::UFract:
      case clang::BuiltinType::ULongFract:
      case clang::BuiltinType::SatShortAccum:
      case clang::BuiltinType::SatAccum:
      case clang::BuiltinType::SatLongAccum:
      case clang::BuiltinType::SatUShortAccum:
      case clang::BuiltinType::SatUAccum:
      case clang::BuiltinType::SatULongAccum:
      case clang::BuiltinType::SatShortFract:
      case clang::BuiltinType::SatFract:
      case clang::BuiltinType::SatLongFract:
      case clang::BuiltinType::SatUShortFract:
      case clang::BuiltinType::SatUFract:
      case clang::BuiltinType::SatULongFract:
      case clang::BuiltinType::BFloat16:
      case clang::BuiltinType::Float128:
      case clang::BuiltinType::NullPtr:
      case clang::BuiltinType::Ibm128:
        return Type();

      // Objective-C types that aren't mapped directly; rather, pointers to
      // these types will be mapped.
      case clang::BuiltinType::ObjCClass:
      case clang::BuiltinType::ObjCId:
      case clang::BuiltinType::ObjCSel:
        return Type();

      // OpenMP types that don't have Codira equivalents.
      case clang::BuiltinType::ArraySection:
      case clang::BuiltinType::OMPArrayShaping:
      case clang::BuiltinType::OMPIterator:
        return Type();

      // OpenCL builtin types that don't have Codira equivalents.
      case clang::BuiltinType::OCLClkEvent:
      case clang::BuiltinType::OCLEvent:
      case clang::BuiltinType::OCLSampler:
      case clang::BuiltinType::OCLQueue:
      case clang::BuiltinType::OCLReserveID:
#define IMAGE_TYPE(Name, Id, ...) case clang::BuiltinType::Id:
#include "clang/Basic/OpenCLImageTypes.def"
#define EXT_OPAQUE_TYPE(Name, Id, ...) case clang::BuiltinType::Id:
#include "clang/Basic/OpenCLExtensionTypes.def"
        return Type();

      // ARM SVE builtin types that don't have Codira equivalents.
#define SVE_TYPE(Name, Id, ...) case clang::BuiltinType::Id:
#include "clang/Basic/AArch64SVEACLETypes.def"
        return Type();

      // PPC SVE builtin types that don't have Codira equivalents.
#define PPC_VECTOR_TYPE(Name, Id, Size) case clang::BuiltinType::Id:
#include "clang/Basic/PPCTypes.def"
        return Type();

      // RISC-V V builtin types that don't have Codira equivalents.
#define RVV_TYPE(Name, Id, Size) case clang::BuiltinType::Id:
#include "clang/Basic/RISCVVTypes.def"
        return Type();

#define WASM_TYPE(Name, Id, Size) case clang::BuiltinType::Id:
#include "clang/Basic/WebAssemblyReferenceTypes.def"
        return Type();

      // AMDGPU builtin types that don't have Codira equivalents.
#define AMDGPU_TYPE(Name, Id, SingletonId) case clang::BuiltinType::Id:
#include "clang/Basic/AMDGPUTypes.def"
        return Type();

      }

      toolchain_unreachable("Invalid BuiltinType.");
    }

    ImportResult VisitBitIntType(const clang::BitIntType *type) {
      Impl.addImportDiagnostic(type, Diagnostic(diag::unsupported_builtin_type,
                                                type->getTypeClassName()),
                               clang::SourceLocation());
      // BitInt is not supported in Codira.
      return Type();
    }

    ImportResult VisitPipeType(const clang::PipeType *type) {
      Impl.addImportDiagnostic(type, Diagnostic(diag::unsupported_builtin_type,
                                                type->getTypeClassName()),
                               clang::SourceLocation());
      // OpenCL types are not supported in Codira.
      return Type();
    }

    ImportResult VisitMatrixType(const clang::MatrixType *ty) {
      Impl.addImportDiagnostic(ty, Diagnostic(diag::unsupported_builtin_type,
                                              ty->getTypeClassName()),
                               clang::SourceLocation());
      // Matrix types are not supported in Codira.
      return Type();
    }

    ImportResult VisitComplexType(const clang::ComplexType *type) {
      Impl.addImportDiagnostic(type, Diagnostic(diag::unsupported_builtin_type,
                                                type->getTypeClassName()),
                               clang::SourceLocation());
      // FIXME: Implement once Complex is in the library.
      return Type();
    }

    ImportResult VisitAtomicType(const clang::AtomicType *type) {
      Impl.addImportDiagnostic(type, Diagnostic(diag::unsupported_builtin_type,
                                                type->getTypeClassName()),
                               clang::SourceLocation());
      // FIXME: handle pointers and fields of atomic type
      return Type();
    }

    ImportResult
    VisitCountAttributedType(const clang::CountAttributedType *type) {
      return Visit(type->desugar());
    }

    ImportResult VisitDynamicRangePointerType(
        const clang::DynamicRangePointerType *type) {
      // DynamicRangePointerType is a clang type representing a pointer with
      // an "ended_by" type attribute for -fbounds-safety.
      return Visit(type->desugar());
    }

    ImportResult VisitValueTerminatedType(
        const clang::ValueTerminatedType *type) {
      // ValueTerminatedType is a clang type representing a pointer with
      // a "terminated_by" type attribute for -fbounds-safety.
      return Visit(type->desugar());
    }

    ImportResult VisitMemberPointerType(const clang::MemberPointerType *type) {
      return Type();
    }

    ImportResult VisitPointerType(const clang::PointerType *type) {
      auto pointeeQualType = type->getPointeeType();
      auto quals = pointeeQualType.getQualifiers();

      // Special case for NSZone*, which has its own Codira wrapper.
      if (const clang::RecordType *pointee =
            pointeeQualType->getAsStructureType()) {
        if (pointee && !pointee->getDecl()->isCompleteDefinition() &&
            pointee->getDecl()->getName() == "_NSZone") {
          Identifier Id_ObjectiveC = Impl.CodiraContext.Id_ObjectiveC;
          ModuleDecl *objCModule = Impl.CodiraContext.getLoadedModule(Id_ObjectiveC);
          Type wrapperTy = Impl.getNamedCodiraType(
                             objCModule,
                             language::getCodiraName(
                               KnownFoundationEntity::NSZone));
          if (wrapperTy)
            return {wrapperTy, ImportHint::OtherPointer};
        }
      }

      // Import 'void*' as 'UnsafeMutableRawPointer' and 'const void*' as
      // 'UnsafeRawPointer'. This is Codira's version of an untyped pointer. Note
      // that 'Unsafe[Mutable]Pointer<T>' implicitly converts to
      // 'Unsafe[Mutable]RawPointer' for interoperability.
      if (pointeeQualType->isVoidType()) {
        auto pointerTypeDecl =
          (quals.hasConst()
           ? Impl.CodiraContext.getUnsafeRawPointerDecl()
           : Impl.CodiraContext.getUnsafeMutableRawPointerDecl());
        if (!pointerTypeDecl)
          return Type();
        return {pointerTypeDecl->getDeclaredInterfaceType(),
                ImportHint::OtherPointer};
      }

      if (pointeeQualType->isDependentType())
        return Type();

      // FIXME: remove workaround once Unsafe*Pointer supports
      //        nonescapable pointees.
      if (evaluateOrDefault(
              Impl.CodiraContext.evaluator,
              ClangTypeEscapability({pointeeQualType.getTypePtr(), nullptr}),
              CxxEscapability::Unknown) == CxxEscapability::NonEscapable) {
        addImportDiagnostic(Diagnostic(diag::ptr_to_nonescapable,
                                       pointeeQualType.getTypePtr()));
        return Type();
      }

      // All other C pointers to concrete types map to
      // UnsafeMutablePointer<T> or OpaquePointer.

      // With pointer conversions enabled, map to the normal pointer types
      // without special hints.
      Type pointeeType = Impl.importTypeIgnoreIUO(
          pointeeQualType, ImportTypeKind::Value, addImportDiagnostic,
          AllowNSUIntegerAsInt, Bridgeability::None, ImportTypeAttrs(),
          OTK_ImplicitlyUnwrappedOptional, /*resugarNSErrorPointer=*/true);

      // If this is imported as a reference type, ignore the innermost pointer.
      // (`T *` becomes `T`, but `T **` becomes `UnsafeMutablePointer<T>`.)
      if (isDirectUseOfForeignReferenceType(pointeeQualType, pointeeType))
        return {pointeeType, ImportHint::OtherPointer};

      // If the pointed-to type is unrepresentable in Codira, or its C
      // alignment is greater than the maximum Codira alignment, import as
      // OpaquePointer.
      if (!pointeeType || Impl.isOverAligned(pointeeQualType)) {
        return importOverAlignedFunctionPointerLikeType(*type, Impl);
      }

      if (pointeeQualType->isFunctionType()) {
        return importFunctionPointerLikeType(*type, pointeeType);
      }

      // FIXME: this is a workaround for rdar://128013193
      if (pointeeType && pointeeType->getAnyNominal() &&
          pointeeType->getAnyNominal()
              ->getAttrs()
              .hasAttribute<MoveOnlyAttr>() &&
          Impl.CodiraContext.LangOpts.CxxInteropUseOpaquePointerForMoveOnly) {
        auto opaquePointerDecl = Impl.CodiraContext.getOpaquePointerDecl();
        if (!opaquePointerDecl)
          return Type();
        return {opaquePointerDecl->getDeclaredInterfaceType(),
                ImportHint::OtherPointer};
      }

      PointerTypeKind pointerKind;
      if (quals.hasConst()) {
        pointerKind = PTK_UnsafePointer;
      } else {
        switch (quals.getObjCLifetime()) {
        case clang::Qualifiers::OCL_Autoreleasing:
        case clang::Qualifiers::OCL_ExplicitNone:
          // Mutable pointers with __autoreleasing or __unsafe_unretained
          // ownership map to AutoreleasingUnsafeMutablePointer<T>.
          pointerKind = PTK_AutoreleasingUnsafeMutablePointer;

          // FIXME: We have tests using a non-Apple stdlib that nevertheless
          // exercise ObjC interop. Fail softly for those tests.
          if (!Impl.CodiraContext.getAutoreleasingUnsafeMutablePointerDecl())
            return Type();
          break;

        case clang::Qualifiers::OCL_Weak:
          // FIXME: We should refuse to import this.
          TOOLCHAIN_FALLTHROUGH;

        case clang::Qualifiers::OCL_None:
        case clang::Qualifiers::OCL_Strong:
          // All other mutable pointers map to UnsafeMutablePointer.
          pointerKind = PTK_UnsafeMutablePointer;
        }
      }
      if (auto wrapped = pointeeType->wrapInPointer(pointerKind)) {
        return {wrapped, ImportHint::OtherPointer};
      } else {
        addImportDiagnostic(Diagnostic(diag::bridged_pointer_type_not_found,
                                       pointerKind));
        return Type();
      }
    }

    ImportResult VisitBlockPointerType(const clang::BlockPointerType *type) {
      // Block pointer types are mapped to function types.
      Type pointeeType = Visit(type->getPointeeType()).AbstractType;
      if (!pointeeType)
        return Type();
      FunctionType *fTy = pointeeType->castTo<FunctionType>();

      auto extInfo =
          fTy->getExtInfo()
              .intoBuilder()
              .withRepresentation(FunctionType::Representation::Block)
              .withClangFunctionType(type)
              .build();
      auto funcTy =
          FunctionType::get(fTy->getParams(), fTy->getResult(), extInfo);
      return { funcTy, ImportHint::Block };
    }

    ImportResult VisitReferenceType(const clang::ReferenceType *type) {
      auto pointeeQualType = type->getPointeeType();
      auto quals = pointeeQualType.getQualifiers();
      Type pointeeType =
          Impl.importTypeIgnoreIUO(pointeeQualType, ImportTypeKind::Value,
                                   addImportDiagnostic,
                                   AllowNSUIntegerAsInt, Bridgeability::None,
                                   ImportTypeAttrs());
      if (!pointeeType)
        return Type();

      if (isDirectUseOfForeignReferenceType(pointeeQualType, pointeeType))
        return {pointeeType, ImportHint::None};

      if (pointeeQualType->isFunctionType()) {
        return importFunctionPointerLikeType(*type, pointeeType);
      }

      // Currently, we can't generate thunks for references to dependent types
      // because there's no way to cast without a copy (without writing the SIL
      // manually).
      if (pointeeQualType->isDependentType())
        return Type();

      if (Impl.isOverAligned(pointeeQualType)) {
        return importOverAlignedFunctionPointerLikeType(*type, Impl);
      }

      PointerTypeKind pointerKind;
      if (quals.hasConst()) {
        pointerKind = PTK_UnsafePointer;
      } else {
        pointerKind = PTK_UnsafeMutablePointer;
      }

      auto pointerType = pointeeType->wrapInPointer(pointerKind);
      if (!pointerType)
        addImportDiagnostic(Diagnostic(diag::bridged_pointer_type_not_found,
                                       pointerKind));
      return {pointerType, ImportHint::None};
    }

    ImportResult VisitMemberPointer(const clang::MemberPointerType *type) {
      // FIXME: Member function pointers can be mapped to curried functions,
      // but only when we can express the notion of a function that does
      // not capture anything from its enclosing context.
      return Type();
    }

    ImportResult VisitArrayType(const clang::ArrayType *type) {
      // FIXME: Array types will need to be mapped differently depending on
      // context.
      return Type();
    }

    ImportResult VisitConstantArrayType(const clang::ConstantArrayType *type) {
      // FIXME: Map to a real fixed-size Codira array type when we have those.
      // Importing as a tuple at least fills the right amount of space, and
      // we can cheese static-offset "indexing" using .$n operations.

      Type elementType = Impl.importTypeIgnoreIUO(
          type->getElementType(), ImportTypeKind::Value, addImportDiagnostic,
          AllowNSUIntegerAsInt, Bridgeability::None, ImportTypeAttrs());
      if (!elementType)
        return Type();

      auto size = type->getSize().getZExtValue();

      // An array of size N is imported as an N-element tuple which
      // takes very long to compile. We chose 4096 as the upper limit because
      // we don't want to break arrays of size PATH_MAX.
      if (size > 4096)
        return Type();

      if (size == 1)
        return elementType;

      SmallVector<TupleTypeElt, 8> elts{static_cast<size_t>(size), elementType};
      return TupleType::get(elts, elementType->getASTContext());
    }

    ImportResult VisitVectorType(const clang::VectorType *type) {
      // Get the imported element type and count.
      Type element = Impl.importTypeIgnoreIUO(
        type->getElementType(), ImportTypeKind::Abstract, addImportDiagnostic,
        false /* No NSUIntegerAsInt */, Bridgeability::None, ImportTypeAttrs(),
        OptionalTypeKind::OTK_None);
      if (!element) { return Type(); }
      unsigned count = type->getNumElements();
      // Import vector-of-one as the element type.
      if (count == 1) { return element; }
      // Imported element type needs to conform to SIMDScalar.
      auto nominal = element->getAnyNominal();
      auto simdscalar = Impl.CodiraContext.getProtocol(KnownProtocolKind::SIMDScalar);
      SmallVector<ProtocolConformance *, 2> conformances;
      if (simdscalar && nominal->lookupConformance(simdscalar, conformances)) {
        // Element type conforms to SIMDScalar. Get the SIMDn generic type
        // if it exists.
        SmallString<8> name("SIMD");
        name.append(std::to_string(count));
        if (auto vector = Impl.getNamedCodiraType(Impl.getStdlibModule(), name)) {
          if (auto unbound = vector->getAs<UnboundGenericType>()) {
            // All checks passed: the imported element type is SIMDScalar,
            // and a generic SIMDn type exists with n == count. Construct the
            // bound generic type and return that.
            return BoundGenericType::get(
              cast<NominalTypeDecl>(unbound->getDecl()), Type(), { element }
            );
          }
        }
      }
      return Type();
    }

    ImportResult VisitFunctionProtoType(const clang::FunctionProtoType *type) {
      // C-style variadic functions cannot be called from Codira.
      if (type->isVariadic())
        return Type();

      // Import the result type.
      auto resultTy = Impl.importTypeIgnoreIUO(
          type->getReturnType(), ImportTypeKind::Result, addImportDiagnostic,
          AllowNSUIntegerAsInt, Bridging, ImportTypeAttrs(), OTK_Optional);
      if (!resultTy)
        return Type();

      SmallVector<FunctionType::Param, 4> params;
      for (auto param = type->param_type_begin(),
             paramEnd = type->param_type_end();
           param != paramEnd; ++param) {
        // Determine whether we have a result parameter of a completion
        // handler that can also express a thrown error.
        ImportTypeKind paramImportKind = ImportTypeKind::Parameter;
        unsigned paramIdx = param - type->param_type_begin();
        if (CompletionHandlerType &&
            Impl.getClangASTContext().hasSameType(
                CompletionHandlerType, type) &&
            paramIdx != CompletionHandlerErrorParamIndex) {
          paramImportKind = ImportTypeKind::CompletionHandlerResultParameter;
        }

        auto paramQualType = *param;
        if (paramQualType->isReferenceType() &&
            paramQualType->getPointeeType().isConstQualified())
          paramQualType = paramQualType->getPointeeType();

        // Mark any `sending` parameters if need be.
        ImportTypeAttrs paramAttributes;
        if (Impl.CodiraContext.LangOpts.hasFeature(Feature::SendingArgsAndResults)) {
          getConcurrencyAttrs(Impl.CodiraContext, ImportTypeKind::Parameter,
                              paramAttributes, paramQualType);
        }

        auto languageParamTy = Impl.importTypeIgnoreIUO(
            paramQualType, paramImportKind, addImportDiagnostic,
            AllowNSUIntegerAsInt, Bridging, ImportTypeAttrs(), OTK_Optional);
        if (!languageParamTy)
          return Type();

        ParameterTypeFlags flags;
        flags = flags.withSending(
            paramAttributes.contains(ImportTypeAttr::Sending));

        // FIXME(https://github.com/apple/language/issues/45134): If we were walking TypeLocs, we could actually get parameter names.
        // The probably doesn't matter outside of a FuncDecl, which we'll have
        // to special-case, but it's an interesting bit of data loss.
        params.emplace_back(languageParamTy, Identifier(), flags);
      }

      // Mark any `sending` result types if need be.
      auto extInfo = FunctionType::ExtInfo();
      ImportTypeAttrs resultAttributes;
      if (Impl.CodiraContext.LangOpts.hasFeature(Feature::SendingArgsAndResults)) {
        getConcurrencyAttrs(Impl.CodiraContext, ImportTypeKind::Result,
                            resultAttributes, type->getReturnType());

        const bool sending = resultAttributes.contains(ImportTypeAttr::Sending);
        extInfo = FunctionType::ExtInfo()
                           .intoBuilder()
                           .withSendingResult(sending)
                           .build();
      }

      // Form the function type.
      return FunctionType::get(params, resultTy, extInfo);
    }

    ImportResult
    VisitFunctionNoProtoType(const clang::FunctionNoProtoType *type) {
      // Import functions without prototypes as functions with no parameters.
      auto resultTy = Impl.importTypeIgnoreIUO(
          type->getReturnType(), ImportTypeKind::Result, addImportDiagnostic,
          AllowNSUIntegerAsInt, Bridging, ImportTypeAttrs(), OTK_Optional);
      if (!resultTy)
        return Type();

      // FIXME: Verify ExtInfo state is correct, not working by accident.
      FunctionType::ExtInfo info;
      return FunctionType::get({}, resultTy, info);
    }

    ImportResult VisitParenType(const clang::ParenType *type) {
      auto inner = Visit(type->getInnerType());
      if (!inner)
        return Type();

      return { inner.AbstractType, inner.Hint };
    }

    /// Imports the type defined by \p objcTypeParamDecl.
    ///
    /// If the type parameter is not imported for some reason, returns \c None.
    /// This is different from a failure; it means the caller should try
    /// importing the underlying type instead.
    std::optional<ImportResult>
    importObjCTypeParamDecl(const clang::ObjCTypeParamDecl *objcTypeParamDecl) {
      // Pull the corresponding generic type parameter from the imported class.
      const auto *typeParamContext = objcTypeParamDecl->getDeclContext();
      GenericSignature genericSig;
      if (auto *category =
            dyn_cast<clang::ObjCCategoryDecl>(typeParamContext)) {
        auto ext = cast_or_null<ExtensionDecl>(
            Impl.importDecl(category, Impl.CurrentVersion));
        if (!ext)
          return ImportResult();
        genericSig = ext->getGenericSignature();
      } else if (auto *interface =
          dyn_cast<clang::ObjCInterfaceDecl>(typeParamContext)) {
        auto cls = castIgnoringCompatibilityAlias<ClassDecl>(
            Impl.importDecl(interface, Impl.CurrentVersion));
        if (!cls)
          return ImportResult();
        genericSig = cls->getGenericSignature();
      }
      unsigned index = objcTypeParamDecl->getIndex();
      // Pull the generic param decl out of the imported class.
      if (!genericSig) {
        // The ObjC type param didn't get imported, possibly because it was
        // suppressed. Treat it as a typedef.
        return std::nullopt;
      }
      if (index > genericSig.getGenericParams().size()) {
        return ImportResult();
      }

      return ImportResult(genericSig.getGenericParams()[index],
                          ImportHint::ObjCPointer);
    }

    ImportResult VisitObjCTypeParamType(const clang::ObjCTypeParamType *type) {
      // FIXME: This drops any added protocols on the floor, which is the whole
      // point of ObjCTypeParamType. Fixing this might be source-breaking,
      // though. rdar://problem/29763975
      if (auto result = importObjCTypeParamDecl(type->getDecl()))
        return result.value();
      // Fall back to importing the desugared type, which uses the parameter's
      // bound. This isn't perfect but it's better than dropping the type.
      return Visit(type->getLocallyUnqualifiedSingleStepDesugaredType());
    }

    ImportResult VisitTypedefType(const clang::TypedefType *type) {
      // If the underlying declaration is an Objective-C type parameter,
      // pull the corresponding generic type parameter from the imported class.
      if (auto *objcTypeParamDecl =
            dyn_cast<clang::ObjCTypeParamDecl>(type->getDecl())) {
        if (auto result = importObjCTypeParamDecl(objcTypeParamDecl))
          return result.value();
        return Visit(type->getLocallyUnqualifiedSingleStepDesugaredType());
      }

      // Import the underlying declaration.
      auto decl = dyn_cast_or_null<TypeDecl>(
          Impl.importDecl(type->getDecl(), Impl.CurrentVersion));

      // If that fails, fall back on importing the underlying type.
      if (!decl) return Visit(type->desugar());

      Type mappedType = decl->getDeclaredInterfaceType();

      if (getCodiraNewtypeAttr(type->getDecl(), Impl.CurrentVersion)) {
        auto underlying = Visit(type->getDecl()->getUnderlyingType());
        switch (underlying.Hint) {
        case ImportHint::None:
        case ImportHint::Void:
        case ImportHint::Block:
        case ImportHint::CFPointer:
        case ImportHint::ObjCPointer:
        case ImportHint::CFunctionPointer:
        case ImportHint::OtherPointer:
        case ImportHint::VAList:
          return {mappedType, underlying.Hint};

        case ImportHint::Boolean:
        case ImportHint::NSUInteger:
          // Toss out the special rules for these types; we still want to
          // import as a wrapper.
          return {mappedType, ImportHint::None};

        case ImportHint::ObjCBridged:
          // If the underlying type was bridged, the wrapper type is
          // only useful in bridged cases. Exit early.
          return { underlying.AbstractType,
                   ImportHint(ImportHint::ObjCBridged, mappedType) };
        }
      }

      // For certain special typedefs, we don't want to use the imported type.
      if (auto specialKind = Impl.getSpecialTypedefKind(type->getDecl())) {
        switch (specialKind.value()) {
        case MappedTypeNameKind::DoNothing:
        case MappedTypeNameKind::DefineAndUse:
          break;
        case MappedTypeNameKind::DefineOnly:
          if (auto typealias = dyn_cast<TypeAliasDecl>(decl))
            mappedType = typealias->getDeclaredInterfaceType()
              ->getDesugaredType();
          break;
        }

        static const toolchain::StringLiteral vaListNames[] = {
          "va_list", "__gnuc_va_list", "__va_list"
        };

        ImportHint hint = ImportHint::None;
        if (type->getDecl()->getName() == "BOOL") {
          hint = ImportHint::Boolean;
        } else if (type->getDecl()->getName() == "Boolean") {
          // FIXME: Darwin only?
          hint = ImportHint::Boolean;
        } else if (type->getDecl()->getName() == "NSUInteger") {
          hint = ImportHint::NSUInteger;
        } else if (toolchain::is_contained(vaListNames,
                                      type->getDecl()->getName())) {
          hint = ImportHint::VAList;
        } else if (isImportedCFPointer(type->desugar(), mappedType)) {
          hint = ImportHint::CFPointer;
        } else if (mappedType->isAnyExistentialType()) { // id, Class
          hint = ImportHint::ObjCPointer;
        } else if (type->isPointerType() || type->isBlockPointerType()) {
          hint = ImportHint::OtherPointer;
        }
        // Any other interesting mapped types should be hinted here.
        return { mappedType, hint };
      }

      // Otherwise, recurse on the underlying type.  We need to recompute
      // the hint, and if the typedef uses different bridgeability than the
      // context then we may also need to bypass the typedef.
      auto underlyingResult = Visit(type->desugar());

      // If we used different bridgeability than this typedef normally
      // would because we're in a non-bridgeable context, and therefore
      // the underlying type is different from the mapping of the typedef,
      // use the underlying type.
      if (Bridging != getTypedefBridgeability(type->getDecl()) &&
          !underlyingResult.AbstractType->isEqual(mappedType)) {
        return underlyingResult;
      }

#ifndef NDEBUG
      switch (underlyingResult.Hint) {
      case ImportHint::Block:
      case ImportHint::ObjCBridged:
        // Bridging is fine for Objective-C and blocks.
        break;
      case ImportHint::NSUInteger:
        // NSUInteger might be imported as Int rather than UInt depending
        // on where the import lives.
        if (underlyingResult.AbstractType->isInt())
          break;
        TOOLCHAIN_FALLTHROUGH;
      default:
        if (!underlyingResult.AbstractType->isEqual(mappedType)) {
          underlyingResult.AbstractType->dump(toolchain::errs());
          mappedType->dump(toolchain::errs());
        }
        assert(underlyingResult.AbstractType->isEqual(mappedType) &&
               "typedef without special typedef kind was mapped "
               "differently from its underlying type?");
      }
#endif

      // If the imported typealias is unavailable, return the underlying type.
      if (decl->isUnavailable())
        return underlyingResult;

      return { mappedType, underlyingResult.Hint };
    }

    // TODO: add custom visitors for these types.
#define MAYBE_SUGAR_TYPE(KIND)                                                 \
    ImportResult Visit##KIND##Type(const clang::KIND##Type *type) {            \
      if (type->isSugared())                                                   \
        return Visit(type->desugar());                                         \
      return Type();                                                           \
    }
    MAYBE_SUGAR_TYPE(TypeOfExpr)
    MAYBE_SUGAR_TYPE(TypeOf)
    MAYBE_SUGAR_TYPE(Decltype)
    MAYBE_SUGAR_TYPE(UnaryTransform)
    MAYBE_SUGAR_TYPE(TemplateSpecialization)
    MAYBE_SUGAR_TYPE(Auto)
    MAYBE_SUGAR_TYPE(DeducedTemplateSpecialization)

    // These types are ALWAYS sugared.
#define SUGAR_TYPE(KIND)                                                       \
    ImportResult Visit##KIND##Type(const clang::KIND##Type *type) {            \
      return Visit(type->desugar());                                           \
    }
    SUGAR_TYPE(MacroQualified)
    SUGAR_TYPE(Attributed)
    SUGAR_TYPE(Adjusted)
    SUGAR_TYPE(SubstTemplateTypeParm)
    SUGAR_TYPE(Elaborated)
    SUGAR_TYPE(Using)
    SUGAR_TYPE(BTFTagAttributed)

    ImportResult VisitDecayedType(const clang::DecayedType *type) {
      clang::ASTContext &clangCtx = Impl.getClangASTContext();
      if (clangCtx.hasSameType(type->getOriginalType(),
                               clangCtx.getBuiltinVaListType())) {
        return {Impl.getNamedCodiraType(Impl.getStdlibModule(),
                                       "CVaListPointer"),
                ImportHint::VAList};
      }
      return Visit(type->desugar());
    }

    ImportResult VisitRecordType(const clang::RecordType *type) {
      auto decl = dyn_cast_or_null<TypeDecl>(
          Impl.importDecl(type->getDecl(), Impl.CurrentVersion));
      if (!decl)
        return nullptr;

      return decl->getDeclaredInterfaceType();
    }

    ImportResult VisitEnumType(const clang::EnumType *type) {
      auto clangDecl = type->getDecl()->getDefinition();
      if (!clangDecl) {
        // FIXME: If the enum has a fixed underlying type, can we use that
        // instead? Or import it opaquely somehow?
        return nullptr;
      }
      switch (Impl.getEnumKind(clangDecl)) {
      case EnumKind::Constants: {
        // Map anonymous enums with no fixed underlying type to Int /if/
        // they fit in an Int32. If not, this mapping isn't guaranteed to be
        // consistent for all platforms we care about.
        if (!clangDecl->isFixed() && clangDecl->isFreeStanding() &&
            clangDecl->getNumPositiveBits() < 32 &&
            clangDecl->getNumNegativeBits() <= 32)
          return Impl.getNamedCodiraType(Impl.getStdlibModule(), "Int");

        // Import the underlying integer type.
        return Visit(clangDecl->getIntegerType());
      }
      case EnumKind::NonFrozenEnum:
      case EnumKind::FrozenEnum:
      case EnumKind::Unknown:
      case EnumKind::Options: {
        auto decl = dyn_cast_or_null<TypeDecl>(
            Impl.importDecl(clangDecl, Impl.CurrentVersion));
        if (!decl)
          return nullptr;

        return decl->getDeclaredInterfaceType();
      }
      }

      toolchain_unreachable("Invalid EnumKind.");
    }

    ImportResult VisitObjCObjectType(const clang::ObjCObjectType *type) {
      // We only handle pointers to objects.
      return nullptr;
    }

    /// Map the Clang language_bridge attribute to a specific type.
    Type mapCodiraBridgeAttr(const clang::NamedDecl *clangDecl) {
      // Check whether there is a language_bridge attribute.
      if (Impl.DisableCodiraBridgeAttr)
        return Type();
      auto bridgeAttr = clangDecl->getAttr<clang::CodiraBridgeAttr>();
      if (!bridgeAttr) return Type();

      // Determine the module and Codira declaration names.
      StringRef moduleName;
      StringRef name = bridgeAttr->getCodiraType();
      auto dotPos = name.find('.');
      if (dotPos == StringRef::npos) {
        // Determine the module name from the Clang declaration.
        if (auto module = clangDecl->getImportedOwningModule())
          moduleName = module->getTopLevelModuleName();
        else
          moduleName = clangDecl->getASTContext().getLangOpts().CurrentModule;
      } else {
        // The string is ModuleName.TypeName.
        moduleName = name.substr(0, dotPos);
        name = name.substr(dotPos + 1);
      }

      return Impl.getNamedCodiraType(moduleName, name);
    }

    ImportResult
    VisitObjCObjectPointerType(const clang::ObjCObjectPointerType *type) {
      Type importedType = Impl.CodiraContext.getAnyObjectType();

      if (!type->qual_empty()) {
        for (auto cp = type->qual_begin(), end = type->qual_end(); cp != end;
             ++cp) {
          if (!(*cp)->hasDefinition())
            Impl.addImportDiagnostic(
                type, Diagnostic(diag::incomplete_protocol, *cp),
                clang::SourceLocation());
        }
      }

      // If this object pointer refers to an Objective-C class (possibly
      // qualified),
      if (auto objcClass = type->getInterfaceDecl()) {
        auto imported = castIgnoringCompatibilityAlias<ClassDecl>(
            Impl.importDecl(objcClass, Impl.CurrentVersion));
        if (!imported && !objcClass->hasDefinition())
          Impl.addImportDiagnostic(
              type, Diagnostic(diag::incomplete_interface, objcClass),
              clang::SourceLocation());

        if (!imported)
          return nullptr;

        // If the objc type has any generic args, convert them and bind them to
        // the imported class type.
        if (imported->getGenericParams()) {
          auto *dc = imported->getDeclContext();
          Type parentTy;
          // Check if this is a nested type and if so,
          // fetch the parent type.
          if (dc->isTypeContext()) {
            parentTy = dc->getDeclaredInterfaceType();
            if (parentTy->is<ErrorType>())
              return Type();
          }

          unsigned typeParamCount = imported->getGenericParams()->size();
          auto typeArgs = type->getObjectType()->getTypeArgs();
          assert(typeArgs.empty() || typeArgs.size() == typeParamCount);
          SmallVector<Type, 2> importedTypeArgs;
          importedTypeArgs.reserve(typeParamCount);
          if (!typeArgs.empty()) {
            for (auto typeArg : typeArgs) {
              Type importedTypeArg = Visit(typeArg).AbstractType;
              if (!importedTypeArg)
                return nullptr;
              importedTypeArgs.push_back(importedTypeArg);
            }
          } else {
            auto *genericEnv = imported->getGenericEnvironment();

            for (auto typeParam : imported->getGenericParams()->getParams()) {
              Type importedTypeArg = genericEnv->mapTypeIntoContext(
                  typeParam->getDeclaredInterfaceType())
                  ->castTo<ArchetypeType>()
                  ->getExistentialType();
              importedTypeArgs.push_back(importedTypeArg);
            }
          }
          assert(importedTypeArgs.size() == typeParamCount);
          importedType = BoundGenericClassType::get(
            imported, parentTy, importedTypeArgs);
        } else {
          importedType = imported->getDeclaredInterfaceType();
        }

        if (!type->qual_empty()) {
          // As a special case, turn 'NSObject <NSCopying>' into
          // 'id <NSObject, NSCopying>', which can be imported more usefully.
          Type nsObjectTy = Impl.getNSObjectType();
          if (!nsObjectTy) {
            // Input is malformed
            return {};
          }
          if (nsObjectTy && importedType->isEqual(nsObjectTy)) {
            // Skip if there is no NSObject protocol.
            auto nsObjectProtoType =
                Impl.getNSObjectProtocolType();
            if (nsObjectProtoType) {
              auto *nsObjectProto = nsObjectProtoType->getAnyNominal();
              if (!nsObjectProto) {
                // Input is malformed
                return {};
              }

              SmallVector<clang::ObjCProtocolDecl *, 4> protocols{
                type->qual_begin(), type->qual_end()
              };
              auto *clangProto =
                  cast<clang::ObjCProtocolDecl>(nsObjectProto->getClangDecl());
              protocols.push_back(
                  const_cast<clang::ObjCProtocolDecl *>(clangProto));

              clang::ASTContext &clangCtx = Impl.getClangASTContext();
              clang::QualType protosOnlyType =
                  clangCtx.getObjCObjectType(clangCtx.ObjCBuiltinIdTy,
                                             /*type args*/{},
                                             protocols,
                                             /*kindof*/false);
              return Visit(clangCtx.getObjCObjectPointerType(protosOnlyType));
            }
          }
        }

        // Determine whether this Objective-C class type is bridged to
        // a Codira type. Hardcode "NSString" since it's referenced from
        // the ObjectiveC module (a level below Foundation).
        Type bridgedType;
        if (auto objcClassDef = objcClass->getDefinition())
          bridgedType = mapCodiraBridgeAttr(objcClassDef);
        else if (objcClass->getName() == "NSString")
          bridgedType = Impl.CodiraContext.getStringType();

        if (bridgedType) {
          // Gather the type arguments.
          SmallVector<Type, 2> importedTypeArgs;
          ArrayRef<clang::QualType> typeArgs = type->getTypeArgs();
          SmallVector<clang::QualType, 2> typeArgsScratch;

          // If we have an unspecialized form of a parameterized
          // Objective-C class type, fill in the defaults.
          if (typeArgs.empty()) {
            if (auto objcGenericParams = objcClass->getTypeParamList()) {
              objcGenericParams->gatherDefaultTypeArgs(typeArgsScratch);
              typeArgs = typeArgsScratch;
            }
          }

          // Convert the type arguments.
          for (auto typeArg : typeArgs) {
            Type importedTypeArg = Impl.importTypeIgnoreIUO(
                typeArg, ImportTypeKind::ObjCCollectionElement,
                addImportDiagnostic, AllowNSUIntegerAsInt, Bridging,
                ImportTypeAttrs(), OTK_None);
            if (!importedTypeArg) {
              importedTypeArgs.clear();
              break;
            }

            importedTypeArgs.push_back(importedTypeArg);
          }

          // If we have an unbound generic bridged type, get the arguments.
          if (auto unboundType = bridgedType->getAs<UnboundGenericType>()) {
            auto unboundDecl = unboundType->getDecl();
            auto bridgedSig = unboundDecl->getGenericSignature();
            assert(bridgedSig && "Bridged signature");
            unsigned numExpectedTypeArgs = bridgedSig.getGenericParams().size();
            if (importedTypeArgs.size() != numExpectedTypeArgs)
              return Type();

            // The first type argument for Dictionary or Set needs
            // to be Hashable. If something isn't Hashable, fall back
            // to AnyHashable as a key type.
            if (unboundType->isDictionary() || unboundType->isSet()) {
              auto &keyType = importedTypeArgs[0];
              if (!Impl.matchesHashableBound(keyType) ||
                  // Dictionary and Array conditionally conform to Hashable,
                  // but the conformance doesn't necessarily apply with the
                  // imported versions of their type arguments.
                  // FIXME: Import their non-Hashable type parameters as
                  // AnyHashable in this context.
                  keyType->isDictionary() || keyType->isArray()) {
                keyType = Impl.CodiraContext.getAnyHashableType();
              }
            }

            // Form the specialized type.
            if (unboundType->isArray()) {
              // Type sugar for arrays.
              assert(importedTypeArgs.size() == 1);
              bridgedType = ArraySliceType::get(importedTypeArgs[0]);
            } else if (unboundType->isDictionary()) {
              // Type sugar for dictionaries.
              assert(importedTypeArgs.size() == 2);
              bridgedType = DictionaryType::get(importedTypeArgs[0],
                                                importedTypeArgs[1]);
            } else {
              // Everything else.
              bridgedType =
                  BoundGenericType::get(cast<NominalTypeDecl>(unboundDecl),
                                        Type(), importedTypeArgs);
            }
          }

          if (bridgedType->isConstraintType())
            bridgedType = ExistentialType::get(bridgedType);

          return { importedType,
                   ImportHint(ImportHint::ObjCBridged, bridgedType) };
        }
      }

      if (!type->qual_empty()) {
        SmallVector<Type, 4> members;
        if (!importedType->isAnyObject())
          members.push_back(importedType);

        for (auto cp = type->qual_begin(), cpEnd = type->qual_end();
             cp != cpEnd; ++cp) {
          auto proto = castIgnoringCompatibilityAlias<ProtocolDecl>(
            Impl.importDecl(*cp, Impl.CurrentVersion));
          if (!proto)
            return Type();

          members.push_back(proto->getDeclaredInterfaceType());
        }

        importedType = ExistentialType::get(
            ProtocolCompositionType::get(Impl.CodiraContext, members,
                                         /*Inverses=*/{},
                                         /*HasExplicitAnyObject=*/false));
      }

      // Class or Class<P> maps to an existential metatype.
      if (type->isObjCClassType() ||
          type->isObjCQualifiedClassType()) {
        importedType = ExistentialMetatypeType::get(importedType);
        return { importedType, ImportHint::ObjCPointer };
      }

      // Beyond here, we're using AnyObject.

      // id maps to Any in bridgeable contexts, AnyObject otherwise.
      if (type->isObjCIdType()) {
        return { Impl.CodiraContext.getAnyObjectType(),
                 ImportHint(ImportHint::ObjCBridged,
                            Impl.CodiraContext.getAnyExistentialType())};
      }

      return { importedType, ImportHint::ObjCPointer };
    }

    ImportResult VisitPackIndexingType(const clang::PackIndexingType *type) {
      return Type();
    }
  };
} // end anonymous namespace

/// True if we're converting a function parameter, property type, or
/// function result type, and can thus safely apply representation
/// conversions for bridged types.
static bool canBridgeTypes(ImportTypeKind importKind) {
  switch (importKind) {
  case ImportTypeKind::Abstract:
  case ImportTypeKind::Value:
  case ImportTypeKind::Variable:
  case ImportTypeKind::AuditedVariable:
  case ImportTypeKind::Enum:
  case ImportTypeKind::RecordField:
    return false;
  case ImportTypeKind::Result:
  case ImportTypeKind::AuditedResult:
  case ImportTypeKind::Parameter:
  case ImportTypeKind::CompletionHandlerParameter:
  case ImportTypeKind::CompletionHandlerResultParameter:
  case ImportTypeKind::Property:
  case ImportTypeKind::PropertyWithReferenceSemantics:
  case ImportTypeKind::ObjCCollectionElement:
  case ImportTypeKind::Typedef:
    return true;
  }

  toolchain_unreachable("Invalid ImportTypeKind.");
}

/// True if the type has known CoreFoundation reference counting semantics.
static bool isCFAudited(ImportTypeKind importKind) {
  switch (importKind) {
  case ImportTypeKind::Abstract:
  case ImportTypeKind::Typedef:
  case ImportTypeKind::Value:
  case ImportTypeKind::ObjCCollectionElement:
  case ImportTypeKind::Variable:
  case ImportTypeKind::Result:
  case ImportTypeKind::Enum:
  case ImportTypeKind::RecordField:
    return false;
  case ImportTypeKind::AuditedVariable:
  case ImportTypeKind::AuditedResult:
  case ImportTypeKind::Parameter:
  case ImportTypeKind::CompletionHandlerParameter:
  case ImportTypeKind::CompletionHandlerResultParameter:
  case ImportTypeKind::Property:
  case ImportTypeKind::PropertyWithReferenceSemantics:
    return true;
  }

  toolchain_unreachable("Invalid ImportTypeKind.");
}

/// Turn T into Unmanaged<T>.
static Type getUnmanagedType(ClangImporter::Implementation &impl,
                             Type payloadType) {
  NominalTypeDecl *unmanagedDecl = impl.CodiraContext.getUnmanagedDecl();
  if (!unmanagedDecl || unmanagedDecl->getGenericParams()->size() != 1)
    return payloadType;

  Type unmanagedClassType = BoundGenericType::get(unmanagedDecl,
                                                  /*parent*/ Type(),
                                                  payloadType);
  return unmanagedClassType;
}

/// Determine whether type is 'NSString.
static bool isNSString(Type type) {
  if (auto classType = type->getAs<ClassType>()) {
    if (auto clangDecl = classType->getDecl()->getClangDecl()) {
      if (auto objcClass = dyn_cast<clang::ObjCInterfaceDecl>(clangDecl)) {
        return objcClass->getName() == "NSString";
      }
    }
  }

  return false;
}

static Type maybeImportNSErrorOutParameter(ClangImporter::Implementation &impl,
                                           Type importedType,
                                           bool resugarNSErrorPointer) {
  PointerTypeKind PTK;
  auto elementType = importedType->getAnyPointerElementType(PTK);
  if (!elementType || PTK != PTK_AutoreleasingUnsafeMutablePointer)
    return Type();

  auto elementObj = elementType->getOptionalObjectType();
  if (!elementObj)
    return Type();

  auto elementClass = elementObj->getClassOrBoundGenericClass();
  if (!elementClass)
    return Type();

  // FIXME: Avoid string comparison by caching this identifier.
  if (elementClass->getName().str() !=
        language::getCodiraName(KnownFoundationEntity::NSError))
    return Type();

  if (!impl.canImportFoundationModule() ||
      !elementClass->getModuleContext()->isFoundationModule())
    return Type();

  ModuleDecl *foundationModule = impl.tryLoadFoundationModule();
  if (resugarNSErrorPointer)
    return impl.getNamedCodiraType(
      foundationModule,
        language::getCodiraName(
          KnownFoundationEntity::NSErrorPointer));

  // The imported type is AUMP<NSError?>, but the typealias is AUMP<NSError?>?
  // so we have to manually make them match. We also want to assume this in
  // general for error out-parameters even if they weren't marked nullable in C.
  // Or at least we do for source-compatibility reasons...
  return OptionalType::get(importedType);
}

static Type maybeImportCFOutParameter(ClangImporter::Implementation &impl,
                                      Type importedType,
                                      ImportTypeAttrs attrs,
                                      toolchain::function_ref<void(Diagnostic &&)> addImportDiagnostic) {
  PointerTypeKind PTK;
  auto elementType = importedType->getAnyPointerElementType(PTK);
  if (!elementType || PTK != PTK_UnsafeMutablePointer)
    return Type();

  auto insideOptionalType = elementType->getOptionalObjectType();
  bool isOptional = (bool) insideOptionalType;
  if (!insideOptionalType)
    insideOptionalType = elementType;

  auto boundGenericTy = insideOptionalType->getAs<BoundGenericType>();
  if (!boundGenericTy)
    return Type();

  if (!boundGenericTy->isUnmanaged())
    return Type();

  assert(boundGenericTy->getGenericArgs().size() == 1 &&
         "signature of Unmanaged has changed");

  auto resultTy = boundGenericTy->getGenericArgs().front();
  if (isOptional)
    resultTy = OptionalType::get(resultTy);

  PointerTypeKind pointerKind;
  if (attrs.contains(ImportTypeAttr::CFRetainedOutParameter))
    pointerKind = PTK_UnsafeMutablePointer;
  else
    pointerKind = PTK_AutoreleasingUnsafeMutablePointer;

  resultTy = resultTy->wrapInPointer(pointerKind);
  if (!resultTy)
    addImportDiagnostic(Diagnostic(diag::bridged_pointer_type_not_found,
                                   pointerKind));
  return resultTy;
}

static ImportedType adjustTypeForConcreteImport(
    ClangImporter::Implementation &impl,
    ImportResult importResult, ImportTypeKind importKind,
    bool allowNSUIntegerAsInt, Bridgeability bridging,
    toolchain::function_ref<void(Diagnostic &&)> addImportDiagnostic,
    ImportTypeAttrs attrs, OptionalTypeKind optKind,
    bool resugarNSErrorPointer,
    clang::Qualifiers::ObjCLifetime objCLifetime) {
  Type importedType = importResult.AbstractType;
  ImportHint hint = importResult.Hint;

  if (importKind == ImportTypeKind::Abstract) {
    return {importedType, false};
  }

  // If we completely failed to import the type, give up now.
  // Special-case for 'void' which is valid in result positions.
  if (!importedType && hint != ImportHint::Void)
    return {Type(), false};

  switch (hint) {
  case ImportHint::None:
    break;

  case ImportHint::ObjCPointer:
  case ImportHint::CFunctionPointer:
    break;

  case ImportHint::Void:
    // 'void' can only be imported as a function result type.
    if (importKind != ImportTypeKind::AuditedResult &&
        importKind != ImportTypeKind::Result) {
      return {Type(), false};
    }
    importedType = impl.CodiraContext.getVoidType();
    if (!importedType) {
      addImportDiagnostic(Diagnostic(diag::bridged_type_not_found_in_module,
                                     "Void", "Codira"));
      return {Type(), false};
    }
    break;

  case ImportHint::ObjCBridged:
    // Import NSString * globals as non-optional String.
    if (isNSString(importedType)) {
      if (importKind == ImportTypeKind::Variable ||
          importKind == ImportTypeKind::AuditedVariable) {
        importedType = hint.BridgedType;
        optKind = OTK_None;
        break;
      }
    }

    // If we have a bridged Objective-C type and we are allowed to
    // bridge, do so.
    if (canBridgeTypes(importKind) &&
        importKind != ImportTypeKind::PropertyWithReferenceSemantics &&
        !(importKind == ImportTypeKind::RecordField &&
          objCLifetime <= clang::Qualifiers::OCL_ExplicitNone) &&
        !(importKind == ImportTypeKind::Typedef &&
          bridging == Bridgeability::None)) {
      // id and Any can be bridged without Foundation. There would be
      // bootstrapping issues with the ObjectiveC module otherwise.
      if (hint.BridgedType->isAny()
          || impl.canImportFoundationModule()
          || impl.ImportForwardDeclarations) {

        // Set the bridged type if it wasn't done already.
        if (!importedType->isEqual(hint.BridgedType))
          importedType = hint.BridgedType;
      }
    }
    break;

  case ImportHint::Block: {
    // CodiraTypeConverter turns block pointers into @convention(block) types.
    // In some contexts, we bridge them to use the Codira function type
    // representation. This includes typedefs of block types, which use the
    // Codira function type representation.
    // FIXME: Do not bridge on RecordFields to keep previous behaviour for
    // the time being.
    if (!canBridgeTypes(importKind) ||
        importKind == ImportTypeKind::RecordField)
      break;

    // Determine the function type representation we need.
    //
    // For Objective-C collection arguments, we cannot bridge from a block
    // to a Codira function type, so force the block representation. Normally
    // the mapped type will have a block representation (making this a no-op),
    // but in cases where the Clang type was written as a typedef of a
    // block type, that typedef will have a Codira function type
    // representation. This code will then break down the imported type
    // alias and produce a function type with block representation.
    auto requiredFunctionTypeRepr = FunctionTypeRepresentation::Codira;
    if (importKind == ImportTypeKind::ObjCCollectionElement) {
      requiredFunctionTypeRepr = FunctionTypeRepresentation::Block;
    }

    auto fTy = importedType->castTo<FunctionType>();
    FunctionType::ExtInfo einfo = fTy->getExtInfo();
    if (einfo.getRepresentation() != requiredFunctionTypeRepr) {
      const clang::Type *clangType = nullptr;
      if (shouldStoreClangType(requiredFunctionTypeRepr))
        clangType = fTy->getASTContext().getClangFunctionType(
            fTy->getParams(), fTy->getResult(), requiredFunctionTypeRepr);
      einfo = einfo.intoBuilder()
                  .withRepresentation(requiredFunctionTypeRepr)
                  .withClangFunctionType(clangType)
                  .build();
      importedType = fTy->withExtInfo(einfo);
    }
    break;
  }

  case ImportHint::Boolean:
    // Turn BOOL and DarwinBoolean into Bool in contexts that can bridge types
    // losslessly.
    if (bridging == Bridgeability::Full && canBridgeTypes(importKind))
      importedType = impl.CodiraContext.getBoolType();
    break;

  case ImportHint::NSUInteger:
    // When NSUInteger is used as an enum's underlying type or if it does not
    // come from a system module, make sure it stays unsigned.
    if (importKind == ImportTypeKind::Enum || !allowNSUIntegerAsInt)
      importedType = impl.CodiraContext.getUIntType();
    break;

  case ImportHint::CFPointer:
    // Wrap CF pointers up as unmanaged types, unless this is an audited
    // context.
    if (!isCFAudited(importKind)) {
      Type underlyingType = importedType->getCodiraNewtypeUnderlyingType();
      if (!underlyingType)
        underlyingType = importedType;
      importedType = getUnmanagedType(impl, underlyingType);
    }
    break;

  case ImportHint::VAList:
    // Treat va_list specially: null-unspecified va_list parameters should be
    // assumed to be non-optional. (Most people don't even think of va_list as a
    // pointer, and it's not a portable assumption anyway.)
    if (importKind == ImportTypeKind::Parameter &&
        optKind == OTK_ImplicitlyUnwrappedOptional) {
      optKind = OTK_None;
    }
    break;

  case ImportHint::OtherPointer:
    // Remove 'Unmanaged' from audited CF out-parameters.
    if (attrs.contains(ImportTypeAttr::CFRetainedOutParameter) ||
        attrs.contains(ImportTypeAttr::CFUnretainedOutParameter)) {
      if (Type outParamTy =
              maybeImportCFOutParameter(impl, importedType, attrs,
                                        addImportDiagnostic)) {
        importedType = outParamTy;
        break;
      }
    } else if (importKind == ImportTypeKind::Parameter) {
      // Special-case AutoreleasingUnsafeMutablePointer<NSError?> parameters.
      if (Type result = maybeImportNSErrorOutParameter(impl, importedType,
                                                       resugarNSErrorPointer)) {
        importedType = result;
        optKind = OTK_None;
        break;
      }
    }

    break;
  }

  assert(importedType);

  if (importKind == ImportTypeKind::RecordField &&
      !importedType->isForeignReferenceType()) {
    switch (objCLifetime) {
      // Wrap retainable struct fields in Unmanaged.
      case clang::Qualifiers::OCL_None:
      case clang::Qualifiers::OCL_ExplicitNone:
        // FIXME: This should apply to blocks as well, but Unmanaged is constrained
        // to AnyObject.
        if (importedType->isAnyClassReferenceType()) {
          importedType = getUnmanagedType(impl, importedType);
        }
        break;
      // FIXME: Eventually we might get C++-like support for strong pointers in
      // structs, at which point we should really be checking the lifetime
      // qualifiers.
      case clang::Qualifiers::OCL_Strong:
        if (!impl.CodiraContext.LangOpts.EnableCXXInterop) {
          return {Type(), false};
        }
        break;
      case clang::Qualifiers::OCL_Weak:
        return {Type(), false};
      case clang::Qualifiers::OCL_Autoreleasing:
        toolchain_unreachable("invalid Objective-C lifetime");
    }
  }

  // Apply attrs.
  importedType =
      impl.applyImportTypeAttrs(attrs, importedType, addImportDiagnostic);

  // Wrap class, class protocol, function, and metatype types in an
  // optional type.
  bool isIUO = false;
  if (importKind != ImportTypeKind::Typedef && optKind != OTK_None &&
      canImportAsOptional(hint)) {
    isIUO = optKind == OTK_ImplicitlyUnwrappedOptional;
    importedType = OptionalType::get(importedType);
  }

  return {importedType, isIUO};
}

void language::findCodiraAttributes(
    clang::QualType type,
    toolchain::function_ref<void(const clang::CodiraAttrAttr *)> callback) {
  std::function<clang::QualType(clang::QualType)> skipUnrelatedSugar =
      [&](clang::QualType type) -> clang::QualType {
    if (auto *MQT = dyn_cast<clang::MacroQualifiedType>(type))
      return MQT->isSugared() ? skipUnrelatedSugar(MQT->desugar()) : type;

    if (auto *ET = dyn_cast<clang::ElaboratedType>(type))
      return ET->isSugared() ? skipUnrelatedSugar(ET->desugar()) : type;

    return type;
  };

  type = skipUnrelatedSugar(type);

  // Consider only immediate attributes, don't look through the typerefs
  // because they are imported separately.
  while (const auto *AT = dyn_cast<clang::AttributedType>(type)) {
    if (auto languageAttr =
            dyn_cast_or_null<clang::CodiraAttrAttr>(AT->getAttr())) {
      callback(languageAttr);
    }
    type = skipUnrelatedSugar(AT->getEquivalentType());
  }
}

void language::getConcurrencyAttrs(ASTContext &CodiraContext,
                                ImportTypeKind importKind,
                                ImportTypeAttrs &attrs, clang::QualType type) {
  bool isMainActor = false;
  bool isSendable = false;
  bool isNonSendable = false;
  bool isSending = false;

  // Consider only immediate attributes, don't look through the typerefs
  // because they are imported separately.
  findCodiraAttributes(type, [&](const clang::CodiraAttrAttr *attr) {
    if (isMainActorAttr(attr)) {
      isMainActor = true;
      isSendable = true; // MainActor implies Sendable
    } else if (attr->getAttribute() == "@Sendable") {
      isSendable = true;
    } else if (attr->getAttribute() == "@_nonSendable") {
      isNonSendable = true;
    } else if (attr->getAttribute() == "sending") {
      isSending = true;
    }
  });

  if (isMainActor)
    attrs |= ImportTypeAttr::MainActor;
  if (isSendable)
    attrs |= ImportTypeAttr::Sendable;
  if (isNonSendable) {
    attrs -= ImportTypeAttr::Sendable;
    attrs -= ImportTypeAttr::DefaultsToSendable;
  }
  if (isSending)
    attrs |= ImportTypeAttr::Sending;
}

ImportedType ClangImporter::Implementation::importType(
    clang::QualType type, ImportTypeKind importKind,
    toolchain::function_ref<void(Diagnostic &&)> addImportDiagnosticFn,
    bool allowNSUIntegerAsInt, Bridgeability bridging, ImportTypeAttrs attrs,
    OptionalTypeKind optionality, bool resugarNSErrorPointer,
    std::optional<unsigned> completionHandlerErrorParamIndex) {
  if (type.isNull())
    return {Type(), false};

  // The "built-in" Objective-C types id, Class, and SEL can actually be (and
  // are) defined within the library. Clang tracks the redefinition types
  // separately, so it can provide fallbacks in certain cases. For Codira, we
  // map the redefinition types back to the equivalent of the built-in types.
  // This bans some trickery that the redefinition types enable, but is a more
  // sound model overall.
  auto &clangContext = getClangASTContext();
  if (clangContext.getLangOpts().ObjC) {
    if (clangContext.hasSameUnqualifiedType(
          type, clangContext.getObjCIdRedefinitionType()) &&
        !clangContext.hasSameUnqualifiedType(
           clangContext.getObjCIdType(),
           clangContext.getObjCIdRedefinitionType()))
      type = clangContext.getObjCIdType();
    else if (clangContext.hasSameUnqualifiedType(
                type, clangContext.getObjCClassRedefinitionType()) &&
             !clangContext.hasSameUnqualifiedType(
                clangContext.getObjCClassType(),
                clangContext.getObjCClassRedefinitionType()))
      type = clangContext.getObjCClassType();
    else if (clangContext.hasSameUnqualifiedType(
               type, clangContext.getObjCSelRedefinitionType()) &&
             !clangContext.hasSameUnqualifiedType(
                clangContext.getObjCSelType(),
                clangContext.getObjCSelRedefinitionType()))
      type = clangContext.getObjCSelType();
  }

  // If nullability is provided as part of the type, that overrides
  // optionality provided externally.
  if (auto nullability = type->getNullability()) {
    bool stripNonResultOptionality =
        importKind == ImportTypeKind::CompletionHandlerResultParameter;
    
    optionality = translateNullability(*nullability, stripNonResultOptionality);
  }

  getConcurrencyAttrs(CodiraContext, importKind, attrs, type);

  // If this is a completion handler parameter, record the function type whose
  // parameters will act as the results of the completion handler.
  const clang::FunctionType *completionHandlerType = nullptr;
  if (completionHandlerErrorParamIndex) {
    if (auto blockPtrType = type->getAs<clang::BlockPointerType>()) {
      completionHandlerType =
          blockPtrType->getPointeeType()->castAs<clang::FunctionType>();

      type = clang::QualType(blockPtrType, 0);
    }
  }

  clang::Qualifiers::ObjCLifetime objCLifetime = type.getObjCLifetime();

  // Perform abstract conversion, ignoring how the type is actually used.
  CodiraTypeConverter converter(
      *this, addImportDiagnosticFn, allowNSUIntegerAsInt, bridging,
      completionHandlerType, completionHandlerErrorParamIndex);
  auto importResult = converter.Visit(type);

  // Now fix up the type based on how we're concretely using it.
  auto adjustedType = adjustTypeForConcreteImport(
      *this, importResult, importKind, allowNSUIntegerAsInt, bridging,
      addImportDiagnosticFn, attrs, optionality, resugarNSErrorPointer,
      objCLifetime);

  return adjustedType;
}

Type ClangImporter::Implementation::importTypeIgnoreIUO(
    clang::QualType type, ImportTypeKind importKind,
    toolchain::function_ref<void(Diagnostic &&)> addImportDiagnosticFn,
    bool allowNSUIntegerAsInt, Bridgeability bridging, ImportTypeAttrs attrs,
    OptionalTypeKind optionality, bool resugarNSErrorPointer) {

  auto importedType = importType(
      type, importKind, addImportDiagnosticFn, allowNSUIntegerAsInt, bridging,
      attrs, optionality, resugarNSErrorPointer, std::nullopt);

  return importedType.getType();
}

bool ClangImporter::Implementation::shouldImportGlobalAsLet(
       clang::QualType type)
{
  // Const variables should be imported as 'let'.
  if (type.isConstQualified()) {
    return true;
  }
  // Globals of type NSString * should be imported as 'let'.
  if (isNSString(type))
    return true;

  return false;
}

/// Returns true if \p name contains the substring "Unsigned" or "unsigned".
static bool nameContainsUnsigned(StringRef name) {
  size_t pos = name.find("nsigned");
  if (pos == StringRef::npos || pos == 0)
    return false;
  --pos;
  return (name[pos] == 'u') || (name[pos] == 'U');
}

bool ClangImporter::Implementation::shouldAllowNSUIntegerAsInt(
    bool isFromSystemModule, const clang::NamedDecl *decl) {
  if (isFromSystemModule)
    if (auto identInfo = decl->getIdentifier())
      return !nameContainsUnsigned(identInfo->getName());
  return false;
}

ImportedType ClangImporter::Implementation::importPropertyType(
    const clang::ObjCPropertyDecl *decl, bool isFromSystemModule) {
  const auto assignOrUnsafeUnretained =
      clang::ObjCPropertyAttribute::kind_assign |
      clang::ObjCPropertyAttribute::kind_unsafe_unretained;

  ImportTypeKind importKind;
  // HACK: Certain decls are always imported using bridged types,
  // because that's what a standalone method would do.
  if (shouldImportPropertyAsAccessors(decl)) {
    importKind = ImportTypeKind::Property;
  } else {
    switch (decl->getSetterKind()) {
    case clang::ObjCPropertyDecl::Assign:
      // If it's readonly, this might just be returned as a default.
      if (decl->isReadOnly() &&
          (decl->getPropertyAttributes() & assignOrUnsafeUnretained) == 0) {
        importKind = ImportTypeKind::Property;
      } else {
        importKind = ImportTypeKind::PropertyWithReferenceSemantics;
      }
      break;
    case clang::ObjCPropertyDecl::Retain:
    case clang::ObjCPropertyDecl::Copy:
      importKind = ImportTypeKind::Property;
      break;
    case clang::ObjCPropertyDecl::Weak:
      importKind = ImportTypeKind::PropertyWithReferenceSemantics;
      break;
    }
  }

  OptionalTypeKind optionality = OTK_ImplicitlyUnwrappedOptional;
  return importType(decl->getType(), importKind,
                    ImportDiagnosticAdder(*this, decl, decl->getLocation()),
                    shouldAllowNSUIntegerAsInt(isFromSystemModule, decl),
                    Bridgeability::Full, getImportTypeAttrs(decl),
                    optionality);
}

namespace {

class GetSendableType :
    private TypeVisitor<GetSendableType, std::pair<Type, bool>> {
  ASTContext &ctx;

public:
  GetSendableType(ASTContext &ctx) : ctx(ctx) {}

  /// The result of a conversion. Contains the converted type and a \c bool that
  /// is \c true if the operation found something to change, or \c false
  /// otherwise.
  using Result = std::pair<Type, /*found=*/bool>;

  /// Returns a modified version of \p type that has been made explicitly
  /// \c Sendable by adding an \c \@Sendable attribute to a function type
  /// or forming a protocol composition with \c & \c Sendable.
  Result convert(Type type) { return visit(type); }

private:
  /// Decide how to represent the given type in a protocol composition. This
  /// is specialized for \c ProtocolCompositionType to avoid nesting
  /// compositions.
  ///
  /// \param members The types to include in the composition.
  /// \return \c true if the composition should include \c AnyObject, \c false
  ///         otherwise.
  bool getAsComposition(ProtocolCompositionType *ty,
                        SmallVectorImpl<Type> &members,
                        InvertibleProtocolSet &inverses) {
    toolchain::append_range(members, ty->getMembers());
    inverses.insertAll(ty->getInverses());
    return ty->hasExplicitAnyObject();
  }

  /// Decide how to represent the given type in a protocol composition. This
  /// is specialized for \c ProtocolCompositionType to avoid nesting
  /// compositions.
  ///
  /// \param members The types to include in the composition.
  /// \return \c true if the composition should include \c AnyObject, \c false
  ///         otherwise.
  bool getAsComposition(TypeBase *ty,
                        SmallVectorImpl<Type> &members,
                        InvertibleProtocolSet &inverses) {
    members.push_back(ty);
    return false;
  }

  // MARK: Visitor Actions

  /// Visitor action: Replace this type with a protocol composition that
  /// includes \c Sendable.
  template <typename Ty> Result compose(Ty *ty) {
    SmallVector<Type, 8> members;
    InvertibleProtocolSet inverses;
    bool explicitAnyObject = getAsComposition(ty, members, inverses);

    auto proto = ctx.getProtocol(KnownProtocolKind::Sendable);
    members.push_back(proto->getDeclaredInterfaceType());

    auto composition =
        ProtocolCompositionType::get(ctx, members, inverses, explicitAnyObject);

    // If we started from a protocol or a composition we should already
    // be in an existential context. Otherwise we'd have to wrap a new
    // composition into an existential.
    if (isa<ProtocolType>(ty) || isa<ProtocolCompositionType>(ty))
      return {composition, true};

    return {ExistentialType::get(composition), true};
  }

  /// Visitor action: Recurse into the children of this type and try to add
  /// \c Sendable to them.
  Result recurse(Type ty) {
    bool anyFound = false;

    Type newTy = ty.transformRec([&](TypeBase *childTy) -> std::optional<Type> {
      // We want to visit the first level of children.
      if (childTy == ty.getPointer())
        return std::nullopt;

      auto result = this->visit(childTy);
      anyFound |= result.second;
      return result.first;
    });

    return { newTy, anyFound };
  }

  /// Visitor action: Ignore this type; do not modify it and do not recurse into
  /// it to find other types to modify.
  Result pass(Type ty, bool found = false) {
    return { ty, found };
  }

  // Macros to define visitors based on these actions.
#define VISIT(CLASS, ACT)  Result visit##CLASS(CLASS *ty) { return ACT(ty); }
#define NEVER_VISIT(CLASS) Result visit##CLASS(CLASS *ty) { \
    toolchain_unreachable("can't have " #CLASS " in imported clang type"); \
    return pass(ty); \
  }

  // MARK: Visitors

  friend TypeVisitor<GetSendableType, Result>;

  Result visitErrorType(ErrorType *ty) {
    // Pass, but suppress diagnostic about not finding anything `Sendable`.
    return pass(ty, /*found=*/true);
  }

  NEVER_VISIT(UnresolvedType)
  NEVER_VISIT(PlaceholderType)
  NEVER_VISIT(BuiltinType)
  NEVER_VISIT(BuiltinTupleType)
  NEVER_VISIT(IntegerType)

  VISIT(TupleType, recurse)

  NEVER_VISIT(ReferenceStorageType)

  VISIT(EnumType, pass)
  VISIT(StructType, pass)
  VISIT(ClassType, compose)
  VISIT(ProtocolType, compose)

  Result visitBoundGenericType(BoundGenericType *ty) {
    assert(!isa<BoundGenericClassType>(ty) && "classes handled elsewhere");

    // These types are produced during bridging and have conditional
    // conformances to Sendable depending on their generic parameters, so we
    // want to make their generic parameters `Sendable`.
    // FIXME: `Unmanaged` ought to be here too, but `AnyObject & Sendable`
    //        doesn't satisfy the generic parameter's `AnyObject` requirement.
    //        (rdar://90946615)
    if (ty->isOptional() || ty->isArray() || ty->isSet() ||
        ty->isDictionary())
      return recurse(ty);

    // Other non-class generic types (e.g. pointers) cannot be made Sendable.
    return pass(ty);
  }

  VISIT(BoundGenericClassType, compose)
  NEVER_VISIT(UnboundGenericType)

  VISIT(AnyMetatypeType, recurse)

  VISIT(ModuleType, pass)
  VISIT(DynamicSelfType, pass)

  // Ignore attributes placed on generic parameter references and
  // other substitutable types.
  VISIT(SubstitutableType, pass)
  NEVER_VISIT(DependentMemberType)

  Result visitAnyFunctionType(AnyFunctionType *ty) {
    auto newFn = applyToFunctionType(ty, [](ASTExtInfo extInfo) {
      return extInfo.withSendable();
    });
    return { newFn, true };
  }

  NEVER_VISIT(SILFunctionType)
  NEVER_VISIT(SILBlockStorageType)
  NEVER_VISIT(SILBoxType)
  NEVER_VISIT(SILPackType)
  NEVER_VISIT(SILTokenType)
  NEVER_VISIT(SILMoveOnlyWrappedType)

  VISIT(ProtocolCompositionType, compose)

  // ProtocolCompositionType doesn't handle ParameterizedProtocolType
  // correctly, but we currently never import anything with it, so forbid it
  // until we find we need it.
  NEVER_VISIT(ParameterizedProtocolType)

  VISIT(ExistentialType, recurse)
  NEVER_VISIT(LValueType)
  VISIT(InOutType, recurse)

  NEVER_VISIT(PackType)
  NEVER_VISIT(PackExpansionType)
  NEVER_VISIT(PackElementType)
  NEVER_VISIT(TypeVariableType)
  NEVER_VISIT(ErrorUnionType)

  VISIT(SugarType, recurse)

  Result visitTypeAliasType(TypeAliasType *ty) {
    // Try converting the underlying type.
    Type underlying = ty->getSinglyDesugaredType();
    auto result = visit(underlying);

    // If nothing that could be made Sendable was found in the underlying type,
    // keep the sugar.
    if (!result.second)
      return pass(ty);

    // If something Sendable-capable *was* found but the operation was a no-op,
    // keep the sugar but indicate that we did find something to avoid a
    // diagnostic.
    if (result.first->getCanonicalType() == underlying->getCanonicalType())
      return pass(ty, /*found=*/true);

    // We found something and it did change the type. Desugar to the converted
    // underlying type.
    return result;
  }
};

} // anonymous namespace

ImportTypeAttrs language::getImportTypeAttrs(const clang::Decl *D, bool isParam) {
  ImportTypeAttrs attrs;

  bool sendableRequested = false;
  bool sendableDisqualified = false;

  if (D->hasAttrs()) {
    for (auto attr : D->getAttrs()) {
      // Map __attribute__((noescape)) to @noescape.
      if (isParam && isa<clang::NoEscapeAttr>(attr)) {
        attrs |= ImportTypeAttr::NoEscape;
        continue;
      }

      if (isa<clang::CFReturnsRetainedAttr>(attr)) {
        attrs |= ImportTypeAttr::CFRetainedOutParameter;
        continue;
      }

      if (isa<clang::CFReturnsNotRetainedAttr>(attr)) {
        attrs |= ImportTypeAttr::CFUnretainedOutParameter;
        continue;
      }

      auto languageAttr = dyn_cast<clang::CodiraAttrAttr>(attr);
      if (!languageAttr)
        continue;

      // Map the main-actor attribute.
      if (isParam && isMainActorAttr(languageAttr)) {
        attrs |= ImportTypeAttr::MainActor;
        sendableDisqualified = true;
        continue;
      }

      // Map @Sendable.
      if (languageAttr->getAttribute() == "@Sendable") {
        sendableRequested = true;
        continue;
      }

      // Map @_nonSendable.
      if (languageAttr->getAttribute() == "@_nonSendable") {
        sendableDisqualified = true;
        continue;
      }
    }
  }

  if (!sendableDisqualified && sendableRequested) {
    attrs |= ImportTypeAttr::Sendable;
  }

  return attrs;
}

Type ClangImporter::Implementation::
applyImportTypeAttrs(ImportTypeAttrs attrs, Type type,
                     toolchain::function_ref<void(Diagnostic &&)> addDiag) {
  if (attrs.contains(ImportTypeAttr::NoEscape)) {
    type = applyToFunctionType(type, [](ASTExtInfo extInfo) {
      return extInfo.withNoEscape();
    });
  }

  if (attrs.contains(ImportTypeAttr::MainActor)) {
    if (Type mainActor = CodiraContext.getMainActorType()) {
      type = applyToFunctionType(type, [&](ASTExtInfo extInfo) {
        return extInfo.withGlobalActor(mainActor);
      });
    } else {
      // If we can't use @MainActor, fall back to at least using @Sendable.
      attrs |= ImportTypeAttr::Sendable;
    }
  }

  if (attrs.contains(ImportTypeAttr::Sendable) ||
      attrs.contains(ImportTypeAttr::DefaultsToSendable)) {
    bool changed;
    std::tie(type, changed) = GetSendableType(CodiraContext).convert(type);

    // Diagnose if we couldn't find a place to add `Sendable` to the type.
    if (!changed) {
      addDiag(Diagnostic(diag::clang_ignored_sendable_attr, type));
    }
  }

  return type;
}

ImportedType ClangImporter::Implementation::importFunctionReturnType(
    DeclContext *dc, const clang::FunctionDecl *clangDecl,
    bool allowNSUIntegerAsInt) {

  // Hardcode handling of certain result types for builtins.
  if (auto builtinID = clangDecl->getBuiltinID()) {
    switch (getClangASTContext().BuiltinInfo.getTypeString(builtinID)[0]) {
    case 'z': // size_t
    case 'Y': // ptrdiff_t
      return {CodiraContext.getIntType(), false};
    default:
      break;
    }
  }

  // CF function results can be managed if they are audited or
  // the ownership convention is explicitly declared.
  assert(clangDecl && "expected to have a decl to import");
  bool isAuditedResult =
    (clangDecl->hasAttr<clang::CFAuditedTransferAttr>() ||
     clangDecl->hasAttr<clang::CFReturnsRetainedAttr>() ||
     clangDecl->hasAttr<clang::CFReturnsNotRetainedAttr>());

  // C++ operators +=, -=, *=, /= may return a reference to self. This is not
  // idiomatic in Codira, let's drop these return values.
  clang::OverloadedOperatorKind op = clangDecl->getOverloadedOperator();
  if ((op == clang::OverloadedOperatorKind::OO_PlusEqual ||
       op == clang::OverloadedOperatorKind::OO_MinusEqual ||
       op == clang::OverloadedOperatorKind::OO_StarEqual ||
       op == clang::OverloadedOperatorKind::OO_SlashEqual) &&
      clangDecl->getReturnType()->isReferenceType()) {
    auto voidTy = CodiraContext.getVoidType();
    if (!voidTy)
      addImportDiagnostic(clangDecl,
                          Diagnostic(diag::bridged_type_not_found_in_module,
                                     "Void", "Codira"),
                          clangDecl->getLocation());
    return {voidTy, false};
  }

  // Fix up optionality.
  OptionalTypeKind OptionalityOfReturn;
  if (clangDecl->hasAttr<clang::ReturnsNonNullAttr>()) {
    OptionalityOfReturn = OTK_None;
  } else {
    OptionalityOfReturn = OTK_ImplicitlyUnwrappedOptional;
  }

  clang::QualType returnType = desugarIfElaborated(clangDecl->getReturnType());
  returnType = desugarIfBoundsAttributed(returnType);
  // In C interop mode, the return type of library builtin functions
  // like 'memcpy' from headers like 'string.h' drops
  // any nullability specifiers from their return type, and preserves it on the
  // declared return type. Thus, in C mode the imported return type of such
  // functions is always optional. However, in C++ interop mode, the return type
  // of builtin functions can preseve the nullability specifiers on their return
  // type, and thus the imported return type of such functions can be
  // non-optional, if the type is annotated with
  // `_Nonnull`. The difference between these two modes can break cross-module
  // Codira serialization, as Codira will no longer be able to resolve an x-ref
  // such as 'memcpy' from a Codira module that uses C interop, within a Codira
  // context that uses C++ interop. In order to avoid the x-ref resolution
  // failure, normalize the return type's nullability for builtin functions in
  // C++ interop mode, to match the imported type in C interop mode.
  auto builtinContext = clang::Builtin::Context();
  if (CodiraContext.LangOpts.EnableCXXInterop && clangDecl->getBuiltinID() &&
      !builtinContext.isTSBuiltin(clangDecl->getBuiltinID()) &&
      builtinContext.isPredefinedLibFunction(
          clangDecl->getBuiltinID()) &&
      builtinContext.getHeaderName(clangDecl->getBuiltinID()) ==
          StringRef("string.h")) {
    if (const auto ART = dyn_cast<clang::AttributedType>(returnType)) {
      if (ART->getImmediateNullability())
        clang::AttributedType::stripOuterNullability(returnType);
    }
  }

  // Specialized templates need to match the args/result exactly (i.e.,
  // ptr -> ptr not ptr -> Optional<ptr>).
  if (returnType->isPointerType() && clangDecl->getPrimaryTemplate() &&
      clangDecl
          ->getPrimaryTemplate()
          ->getAsFunction()
          ->getReturnType()
          ->isTemplateTypeParmType())
    OptionalityOfReturn = OTK_None;

  ImportedType optionSetEnum = importer::findOptionSetEnum(returnType, *this);
  if (optionSetEnum)
    return optionSetEnum;

  // Import the underlying result type.
  if (clangDecl) {
    if (auto recordType = returnType->getAsCXXRecordDecl()) {
      if (auto *vd = evaluateOrDefault(
              CodiraContext.evaluator,
              // `importerImpl` is set to nullptr here to avoid diagnostics
              // during this CxxRecordSemantics evaluation.
              CxxRecordAsCodiraType({recordType, CodiraContext, nullptr}),
              nullptr)) {
        if (auto *cd = dyn_cast<ClassDecl>(vd)) {
          Type t = ClassType::get(cd, Type(), CodiraContext);
          return ImportedType(t, /*implicitlyUnwraps=*/false);
        }
      }
    }
  }

  // Import the result type.
  return importType(
      returnType,
      (isAuditedResult ? ImportTypeKind::AuditedResult
                       : ImportTypeKind::Result),
      ImportDiagnosticAdder(*this, clangDecl, clangDecl->getLocation()),
      allowNSUIntegerAsInt, Bridgeability::Full, getImportTypeAttrs(clangDecl),
      OptionalityOfReturn, true, std::nullopt);
}

static Type
findGenericTypeInGenericDecls(ClangImporter::Implementation &impl,
                              const clang::TemplateTypeParmType *templateParam,
                              ArrayRef<GenericTypeParamDecl *> genericParams,
                              ImportTypeAttrs attrs,
                              toolchain::function_ref<void(Diagnostic &&)> addDiag) {
  StringRef name = templateParam->getIdentifier()->getName();
  auto genericParamIter =
      toolchain::find_if(genericParams, [name](GenericTypeParamDecl *generic) {
        return generic->getName().str() == name;
      });
  // TODO: once we support generics in class types, replace this with
  // "return nullptr". Once support for template classes, this will need to
  // be updated, though. I'm leaving the assert here to make it easier to
  // find.
  assert(genericParamIter != genericParams.end() &&
         "Could not find generic param type in generic params.");
  auto *genericParamDecl = *genericParamIter;
  auto metatype =
      cast<MetatypeType>(genericParamDecl->getInterfaceType().getPointer());
  return impl.applyImportTypeAttrs(attrs, metatype->getMetatypeInstanceType(),
                                   addDiag);
}

ImportedType ClangImporter::Implementation::importFunctionParamsAndReturnType(
    DeclContext *dc, const clang::FunctionDecl *clangDecl,
    ArrayRef<const clang::ParmVarDecl *> params, bool isVariadic,
    bool isFromSystemModule, DeclName name, ParameterList *&parameterList,
    ArrayRef<GenericTypeParamDecl *> genericParams) {

  bool allowNSUIntegerAsInt =
      shouldAllowNSUIntegerAsInt(isFromSystemModule, clangDecl);

  // Only eagerly import the return type if it's not too expensive (the current
  // heuristic for that is if it's not a record type).
  ImportDiagnosticAdder addDiag(*this, clangDecl,
                                clangDecl->getSourceRange().getBegin());
  clang::QualType returnType = desugarIfElaborated(clangDecl->getReturnType());
  returnType = desugarIfBoundsAttributed(returnType);

  ImportedType importedType = importer::findOptionSetEnum(returnType, *this);

  if (auto templateType =
          dyn_cast<clang::TemplateTypeParmType>(returnType)) {
    importedType = {findGenericTypeInGenericDecls(
                        *this, templateType, genericParams,
                        getImportTypeAttrs(clangDecl), addDiag),
                    false};
  } else if ((isa<clang::PointerType>(returnType) ||
          isa<clang::ReferenceType>(returnType)) &&
         isa<clang::TemplateTypeParmType>(returnType->getPointeeType())) {
    auto pointeeType = returnType->getPointeeType();
    auto templateParamType = cast<clang::TemplateTypeParmType>(pointeeType);
    PointerTypeKind pointerKind = pointeeType.getQualifiers().hasConst()
                                      ? PTK_UnsafePointer
                                      : PTK_UnsafeMutablePointer;
    auto genericType =
        findGenericTypeInGenericDecls(*this, templateParamType, genericParams,
                                      getImportTypeAttrs(clangDecl), addDiag);
    auto genericPointerType = genericType->wrapInPointer(pointerKind);
    if (!genericPointerType)
      addDiag(Diagnostic(diag::bridged_pointer_type_not_found, pointerKind));
    importedType = {genericPointerType, false};
  } else if (!(isa<clang::RecordType>(returnType) ||
               isa<clang::TemplateSpecializationType>(returnType)) ||
             // TODO: we currently don't lazily load operator return types, but
             // this should be trivial to add.
             clangDecl->isOverloadedOperator() ||
             // Dependant types are trivially mapped as Any.
             returnType->isDependentType()) {
    // If importedType is already initialized, it means we found the enum that
    // was supposed to be used (instead of the typedef type).
    if (!importedType) {
      importedType =
          importFunctionReturnType(dc, clangDecl, allowNSUIntegerAsInt);
      if (!importedType) {
        addDiag(Diagnostic(diag::return_type_not_imported));
        return {Type(), false};
      }
    }
  }

  Type languageResultTy = importedType.getType();
  ArrayRef<Identifier> argNames = name.getArgumentNames();
  parameterList = importFunctionParameterList(dc, clangDecl, params, isVariadic,
                                              allowNSUIntegerAsInt, argNames,
                                              genericParams, languageResultTy);
  if (!parameterList)
    return {Type(), false};

  if (clangDecl->isNoReturn())
    languageResultTy = CodiraContext.getNeverType();

  return {languageResultTy, importedType.isImplicitlyUnwrapped()};
}

static bool isParameterContextGlobalActorIsolated(DeclContext *dc,
                                                  const clang::Decl *parent) {
  if (getActorIsolationOfContext(dc).isGlobalActor())
    return true;

  if (!parent->hasAttrs())
    return false;

  for (const auto *attr : parent->getAttrs()) {
    if (auto languageAttr = dyn_cast<clang::CodiraAttrAttr>(attr)) {
      if (isMainActorAttr(languageAttr))
        return true;
    }
  }

  return false;
}

static bool isSendableInferenceOnCompletionHandlerParameterAllowed(
    DeclContext *dc, const clang::Decl *parent) {
  auto &C = dc->getASTContext();
  if (!C.LangOpts.hasFeature(Feature::SendableCompletionHandlers))
    return false;

  return !isParameterContextGlobalActorIsolated(dc, parent);
}

std::optional<ClangImporter::Implementation::ImportParameterTypeResult>
ClangImporter::Implementation::importParameterType(
    DeclContext *dc, const clang::Decl *parent, const clang::ParmVarDecl *param,
    OptionalTypeKind optionalityOfParam, bool allowNSUIntegerAsInt,
    bool isNSDictionarySubscriptGetter, bool paramIsError,
    bool paramIsCompletionHandler,
    std::optional<unsigned> completionHandlerErrorParamIndex,
    ArrayRef<GenericTypeParamDecl *> genericParams,
    toolchain::function_ref<void(Diagnostic &&)> addImportDiagnosticFn) {
  auto paramTy = desugarIfElaborated(param->getType());
  paramTy = desugarIfBoundsAttributed(paramTy);

  // If this type has a _Nullable/_Nonnull attribute, drop it, since we already
  // have that information in optionalityOfParam.
  if (auto attributedTy = dyn_cast<clang::AttributedType>(paramTy)) {
    if (attributedTy->getImmediateNullability())
      clang::AttributedType::stripOuterNullability(paramTy);
  }

  ImportTypeKind importKind = paramIsCompletionHandler
                                  ? ImportTypeKind::CompletionHandlerParameter
                                  : ImportTypeKind::Parameter;

  // Import the parameter type into Codira.
  auto attrs = getImportTypeAttrs(param, /*isParam=*/true);
  Type languageParamTy;
  bool isInOut = false;
  bool isConsuming = false;
  bool isParamTypeImplicitlyUnwrapped = false;

  if (paramIsCompletionHandler &&
      isSendableInferenceOnCompletionHandlerParameterAllowed(dc, parent)) {
    attrs |= ImportTypeAttr::DefaultsToSendable;
  }

  if (auto optionSetEnum = importer::findOptionSetEnum(paramTy, *this)) {
    languageParamTy = optionSetEnum.getType();
  } else if (isa<clang::PointerType>(paramTy) &&
             isa<clang::TemplateTypeParmType>(paramTy->getPointeeType())) {
    auto pointeeType = paramTy->getPointeeType();
    auto templateParamType = cast<clang::TemplateTypeParmType>(pointeeType);
    PointerTypeKind pointerKind = pointeeType.getQualifiers().hasConst()
                                      ? PTK_UnsafePointer
                                      : PTK_UnsafeMutablePointer;
    auto genericType = findGenericTypeInGenericDecls(
        *this, templateParamType, genericParams, attrs, addImportDiagnosticFn);
    languageParamTy = genericType->wrapInPointer(pointerKind);
    if (!languageParamTy) {
      addImportDiagnosticFn(Diagnostic(diag::bridged_pointer_type_not_found,
                                       pointerKind));
      return std::nullopt;
    }
    switch (optionalityOfParam) {
    case OTK_Optional:
      languageParamTy = OptionalType::get(languageParamTy);
      break;
    case OTK_ImplicitlyUnwrappedOptional:
      languageParamTy = OptionalType::get(languageParamTy);
      isParamTypeImplicitlyUnwrapped = true;
      break;
    case OTK_None:
      break;
    }
  } else if (isa<clang::ReferenceType>(paramTy) &&
             isa<clang::TemplateTypeParmType>(paramTy->getPointeeType())) {
    // We don't support universal reference, bail.
    if (paramTy->isRValueReferenceType()) {
      addImportDiagnosticFn(Diagnostic(diag::rvalue_ref_params_not_imported));
      return std::nullopt;
    }

    auto templateParamType =
        cast<clang::TemplateTypeParmType>(paramTy->getPointeeType());
    languageParamTy = findGenericTypeInGenericDecls(
        *this, templateParamType, genericParams, attrs, addImportDiagnosticFn);
    if (!paramTy->getPointeeType().isConstQualified())
      isInOut = true;
  } else if (auto *templateParamType =
                 dyn_cast<clang::TemplateTypeParmType>(paramTy)) {
    languageParamTy = findGenericTypeInGenericDecls(
        *this, templateParamType, genericParams, attrs, addImportDiagnosticFn);
  }

  if (!languageParamTy) {
    // C++ reference types are brought in as direct
    // types most commonly.
    if (auto refPointeeType =
            getCxxReferencePointeeTypeOrNone(paramTy.getTypePtr())) {
      // We don't support reference type to a dependent type, just bail.
      if ((*refPointeeType)->isDependentType()) {
        return std::nullopt;
      }

      bool isRvalueRef = paramTy->isRValueReferenceType();
      // A C++ parameter of type `const <type> &` or `<type> &` becomes `<type>`
      // or `inout <type>`. Moreover, `const <type> &&` or `<type> &&`
      // becomes `<type>` or `consuming <type>`. Note that SILGen will use the
      // indirect parameter convention for such a type.
      paramTy = *refPointeeType;
      if (!paramTy.isConstQualified()) {
        if (isRvalueRef)
          isConsuming = true;
        else
          isInOut = true;
      }
    }
  }

  // Special case for NSDictionary's subscript.
  if (isNSDictionarySubscriptGetter && paramTy->isObjCIdType()) {
    // Not using `getImportTypeAttrs()` is unprincipled but OK for this hack.
    auto nsCopying = CodiraContext.getNSCopyingType();
    if (!nsCopying)
      return std::nullopt;

    languageParamTy = ExistentialType::get(nsCopying);
    if (optionalityOfParam != OTK_None)
      languageParamTy = OptionalType::get(languageParamTy);

    isParamTypeImplicitlyUnwrapped =
        optionalityOfParam == OTK_ImplicitlyUnwrappedOptional;
  }

  if (!languageParamTy) {
    if (auto recordType = paramTy->getAsCXXRecordDecl()) {

      if (auto *vd = evaluateOrDefault(
              CodiraContext.evaluator,
              // `importerImpl` is set to nullptr here to avoid diagnostics
              // during this CxxRecordSemantics evaluation.
              CxxRecordAsCodiraType({recordType, CodiraContext, nullptr}),
              nullptr)) {

        if (auto *cd = dyn_cast<ClassDecl>(vd)) {

          languageParamTy = ClassType::get(cd, Type(), CodiraContext);
        }
      }
    }
  }

  if (!languageParamTy) {
    // If this is the throws error parameter, we don't need to convert any
    // NSError** arguments to the sugared NSErrorPointer typealias form,
    // because all that is done with it is retrieving the canonical
    // type. Avoiding the sugar breaks a loop in Foundation caused by method
    // on NSString that has an error parameter. FIXME: This is a work-around
    // for the specific case when the throws conversion works, but is not
    // sufficient if it fails. (The correct, overarching fix is ClangImporter
    // being lazier.)
    auto importedType = importType(paramTy, importKind, addImportDiagnosticFn,
                                   allowNSUIntegerAsInt, Bridgeability::Full,
                                   attrs, optionalityOfParam,
                                   /*resugarNSErrorPointer=*/!paramIsError,
                                   completionHandlerErrorParamIndex);
    if (!importedType)
      return std::nullopt;

    isParamTypeImplicitlyUnwrapped = importedType.isImplicitlyUnwrapped();
    languageParamTy = importedType.getType();
  }

  // `isInOut` is set above if we stripped off a mutable `&` before importing
  // the type. Normally, we want to use an `inout` parameter in this situation.
  // However, if the parameter belongs to a foreign reference type *and* the
  // reference we stripped out was directly to that type (rather than to a
  // pointer to that type), the foreign reference type should "eat" the
  // indirection of the `&`, so we *don't* want to use an `inout` parameter.
  if (isInOut && isDirectUseOfForeignReferenceType(paramTy, languageParamTy))
    isInOut = false;

  return ImportParameterTypeResult{languageParamTy, isInOut, isConsuming,
                                   isParamTypeImplicitlyUnwrapped};
}

bool ClangImporter::Implementation::isDefaultArgSafeToImport(
    const clang::ParmVarDecl *param) {
  // If the argument is explicitly marked as import_unsafe, import its default
  // expression regardless of the safety heuristics.
  if (hasUnsafeAPIAttr(param))
    return true;

  auto functionDecl = cast<clang::FunctionDecl>(param->getDeclContext());

  if (param->hasUninstantiatedDefaultArg() &&
      !functionDecl->getTemplateInstantiationPattern(
          /*ForDefinition*/ false))
    // HACK: Clang will crash while trying to instantiate this default arg.
    return false;

  clang::CXXDefaultArgExpr *defaultArgExpr = nullptr;
  // Try to instantiate the default expression.
  auto defaultArgExprResult = getClangSema().BuildCXXDefaultArgExpr(
      clang::SourceLocation(), const_cast<clang::FunctionDecl *>(functionDecl),
      const_cast<clang::ParmVarDecl *>(param));
  // If the default expression can't be instantiated, bail.
  if (!defaultArgExprResult.isUsable())
    return false;
  else
    defaultArgExpr = cast<clang::CXXDefaultArgExpr>(defaultArgExprResult.get());

  // If the type of this parameter is a view type, do not import the
  // default expression, since we cannot guarantee the lifetime of the
  // pointee value.
  if (auto paramRecordDecl = param->getType()->getAsCXXRecordDecl()) {
    if (isViewType(paramRecordDecl) &&
        !paramRecordDecl->hasUserDeclaredCopyConstructor())
      return false;
  }
  // If the parameter is a const reference, check if the expression
  // creates temporaries. Since we import const T& as T in Codira, the
  // value of the default expression will get copied in Codira unlike C++,
  // which might be unexpected and unsafe.
  if (param->getType()->isReferenceType() &&
      param->getType()->getPointeeType().isConstQualified()) {
    if (isa<clang::MaterializeTemporaryExpr>(defaultArgExpr->getExpr()))
      return false;
  }
  return true;
}

static ParamDecl *getParameterInfo(ClangImporter::Implementation *impl,
                                   const clang::ParmVarDecl *param,
                                   const Identifier &name,
                                   const language::Type &languageParamTy,
                                   const bool isInOut, const bool isConsuming,
                                   const bool isParamTypeImplicitlyUnwrapped) {
  // Figure out the name for this parameter.
  Identifier bodyName = impl->importFullName(param, impl->CurrentVersion)
                            .getBaseIdentifier(impl->CodiraContext);

  // It doesn't actually matter which DeclContext we use, so just use the
  // imported header unit.
  auto paramInfo = impl->createDeclWithClangNode<ParamDecl>(
      param, AccessLevel::Private, SourceLoc(), SourceLoc(), name,
      impl->importSourceLoc(param->getLocation()), bodyName,
      impl->ImportedHeaderUnit);

  auto &ASTContext = paramInfo->getASTContext();
  // If SendingArgsAndResults are enabled and we have a sending argument,
  // set that the param was sending.
  if (ASTContext.LangOpts.hasFeature(Feature::SendingArgsAndResults)) {
    if (auto *attr = param->getAttr<clang::CodiraAttrAttr>()) {
      if (attr->getAttribute() == "sending") {
        paramInfo->setSending();
      }
    }
  }

  // C++ types taking a reference might return a reference/pointer to a
  // subobject of the referenced storage. In those cases we need to prevent the
  // Codira compiler to pass in a temporary copy to prevent dangling.
  if (ASTContext.LangOpts.hasFeature(Feature::AddressableParameters) &&
      !param->getType().isNull() && param->getType()->isReferenceType()) {
    paramInfo->setAddressable();
  }

  // Parameters of type const T& imported as T, make sure we borrow from them
  // when they have lifetime annotations.
  bool isBorrowing = (param->getAttr<clang::LifetimeBoundAttr>() ||
                      param->getAttr<clang::LifetimeCaptureByAttr>()) &&
                     param->getType()->isReferenceType();
  // Foreign references are already references so they don't need to be passed
  // as inout.
  paramInfo->setSpecifier(
      isConsuming ? ParamSpecifier::Consuming
                  : (isInOut ? ParamSpecifier::InOut
                             : (isBorrowing ? ParamSpecifier::Borrowing
                                            : ParamSpecifier::Default)));
  paramInfo->setInterfaceType(languageParamTy);
  impl->recordImplicitUnwrapForDecl(paramInfo, isParamTypeImplicitlyUnwrapped);

  // Import the default expression for this parameter if possible.
  // Codira doesn't support default values of inout parameters.
  // TODO: support default arguments of constructors
  // (https://github.com/apple/language/issues/70124)
  // TODO: support params with template parameters
  if (param->hasDefaultArg() && !isInOut &&
      !isa<clang::CXXConstructorDecl>(param->getDeclContext()) &&
      impl->isDefaultArgSafeToImport(param) &&
      !param->isTemplated()) {
    CodiraDeclSynthesizer synthesizer(*impl);
    if (CallExpr *defaultArgExpr = synthesizer.makeDefaultArgument(
            param, languageParamTy, paramInfo->getParameterNameLoc())) {
      paramInfo->setDefaultArgumentKind(DefaultArgumentKind::Normal);
      paramInfo->setTypeCheckedDefaultExpr(defaultArgExpr);
      paramInfo->setDefaultValueStringRepresentation("cxxDefaultArg");
    }
  }

  return paramInfo;
}

ParameterList *ClangImporter::Implementation::importFunctionParameterList(
    DeclContext *dc, const clang::FunctionDecl *clangDecl,
    ArrayRef<const clang::ParmVarDecl *> params, bool isVariadic,
    bool allowNSUIntegerAsInt, ArrayRef<Identifier> argNames,
    ArrayRef<GenericTypeParamDecl *> genericParams, Type resultType) {
  // Import the parameters.
  SmallVector<ParamDecl *, 4> parameters;
  unsigned index = 0;
  SmallBitVector nonNullArgs = getNonNullArgs(clangDecl, params);

  for (auto param : params) {
    auto paramTy = param->getType();
    if (paramTy->isVoidType()) {
      ++index;
      continue;
    }

    bool knownNonNull = !nonNullArgs.empty() && nonNullArgs[index];

    // Check nullability of the parameter.
    OptionalTypeKind optionalityOfParam =
        getParamOptionality(param, knownNonNull);

    ImportDiagnosticAdder paramAddDiag(*this, clangDecl, param->getLocation());

    auto languageParamTyOpt = importParameterType(
        dc, clangDecl, param, optionalityOfParam, allowNSUIntegerAsInt,
        /*isNSDictionarySubscriptGetter=*/false,
        /*paramIsError=*/false,
        /*paramIsCompletionHandler=*/false,
        /*completionHandlerErrorParamIndex=*/std::nullopt, genericParams,
        paramAddDiag);
    if (!languageParamTyOpt) {
      addImportDiagnostic(param,
                          Diagnostic(diag::parameter_type_not_imported, param),
                          param->getSourceRange().getBegin());
      return nullptr;
    }
    auto languageParamTy = languageParamTyOpt->languageTy;
    bool isInOut = languageParamTyOpt->isInOut;
    bool isConsuming = languageParamTyOpt->isConsuming;
    bool isParamTypeImplicitlyUnwrapped =
        languageParamTyOpt->isParamTypeImplicitlyUnwrapped;

    // Retrieve the argument name.
    Identifier name;
    if (index < argNames.size())
      name = argNames[index];

    auto paramInfo =
        getParameterInfo(this, param, name, languageParamTy, isInOut, isConsuming,
                         isParamTypeImplicitlyUnwrapped);
    parameters.push_back(paramInfo);
    ++index;
  }

  auto genericParamTypeUsedInSignature =
      [params, resultType](GenericTypeParamDecl *genericParam,
                           bool shouldCheckResultType) -> bool {
    auto paramDecl = genericParam->getClangDecl();
    auto templateTypeParam = cast<clang::TemplateTypeParmDecl>(paramDecl);
    // TODO(https://github.com/apple/language/issues/56206): This won't work when we support importing dependent types.
    // We'll have to change this logic to traverse the type tree of the imported
    // Codira type (basically whatever ends up in the parameters variable).
    // Check if genericParam's corresponding clang template type is used by
    // the clang function's parameters.
    for (auto param : params) {
      if (hasSameUnderlyingType(param->getType().getTypePtr(),
                                templateTypeParam)) {
        return true;
      }
    }

    // Check if genericParam is used as a type parameter in the result type.
    return shouldCheckResultType &&
        resultType.findIf([genericParam](Type typePart) -> bool {
          return typePart->isEqual(genericParam->getDeclaredInterfaceType());
        });
  };
  
  // Make sure all generic parameters are accounted for in the function signature.
  for (auto genericParam : genericParams) {
    bool shouldCheckResultType = resultType && resultType->hasTypeParameter();
    if (genericParamTypeUsedInSignature(genericParam, shouldCheckResultType))
      continue;
    
    // If this generic parameter is not used in the function signature,
    // add a new parameter that accepts a metatype corresponding to that
    // generic parameter.
    Identifier name = genericParam->getName();
    auto param = new (CodiraContext)
        ParamDecl(SourceLoc(), SourceLoc(), name, SourceLoc(),
                  name, dc);
    auto metatype =
      cast<MetatypeType>(genericParam->getInterfaceType().getPointer());
    param->setInterfaceType(metatype);
    param->setSpecifier(ParamSpecifier::Default);
    parameters.push_back(param);
  }

  // Append an additional argument to represent varargs.
  if (isVariadic) {
    auto paramTy = VariadicSequenceType::get(
        CodiraContext.getAnyExistentialType());
    auto name = CodiraContext.getIdentifier("varargs");
    auto param = new (CodiraContext) ParamDecl(SourceLoc(), SourceLoc(),
                                              Identifier(), SourceLoc(),
                                              name,
                                              ImportedHeaderUnit);
    param->setSpecifier(ParamSpecifier::Default);
    param->setInterfaceType(paramTy);

    param->setVariadic();
    parameters.push_back(param);
  }

  // Form the parameter list.
  // Estimate locations for the begin and end of parameter list.
  auto begin = clangDecl->getLocation();
  auto end = begin;
  if (!params.empty()) {
    begin = params.front()->getBeginLoc();
    end = params.back()->getEndLoc();
  }
  return ParameterList::create(CodiraContext, importSourceLoc(begin), parameters,
                               importSourceLoc(end));
}

static bool isObjCMethodResultAudited(const clang::Decl *decl) {
  if (!decl)
    return false;
  return (decl->hasAttr<clang::CFReturnsRetainedAttr>() ||
          decl->hasAttr<clang::CFReturnsNotRetainedAttr>() ||
          decl->hasAttr<clang::ObjCReturnsInnerPointerAttr>());
}

ArgumentAttrs ClangImporter::Implementation::inferDefaultArgument(
    clang::QualType type, OptionalTypeKind clangOptionality,
    DeclBaseName baseName, StringRef argumentLabel, bool isFirstParameter,
    bool isLastParameter, NameImporter &nameImporter) {
  auto baseNameStr = baseName.userFacingName();

  // Don't introduce a default argument for the first parameter of setters.
  if (isFirstParameter && camel_case::getFirstWord(baseNameStr) == "set")
    return DefaultArgumentKind::None;

  if (auto elaboratedTy = type->getAs<clang::ElaboratedType>())
    type = elaboratedTy->desugar();

  // Some nullable parameters default to 'nil'.
  if (clangOptionality == OTK_Optional) {
    // Nullable trailing closure parameters default to 'nil'.
    if (isLastParameter &&
        (type->isFunctionPointerType() || type->isBlockPointerType()))
      return DefaultArgumentKind::NilLiteral;

    // NSZone parameters default to 'nil'.
    if (auto ptrType = type->getAs<clang::PointerType>()) {
      if (auto recType
            = ptrType->getPointeeType()->getAs<clang::RecordType>()) {
        if (recType->isStructureOrClassType() &&
            recType->getDecl()->getName() == "_NSZone")
          return DefaultArgumentKind::NilLiteral;
      }
    }
  }

  // Option sets default to "[]" if they have "Options" in their name.
  if (const clang::EnumType *enumTy = type->getAs<clang::EnumType>()) {
    const clang::EnumDecl *enumDef = enumTy->getDecl()->getDefinition();
    if (enumDef && nameImporter.getEnumKind(enumDef) == EnumKind::Options) {
      auto enumName = enumDef->getName();
      for (auto word : toolchain::reverse(camel_case::getWords(enumName))) {
        if (camel_case::sameWordIgnoreFirstCase(word, "options"))
          return DefaultArgumentKind::EmptyArray;
      }
    }
  } else if (const clang::TypedefType *typedefType =
                 type->getAs<clang::TypedefType>()) {
    // Get the AvailabilityAttr that would be set from CF/NS_OPTIONS
    auto typedefDecl = typedefType->getDecl();
    if (importer::isUnavailableInCodira(typedefDecl, nullptr, true)) {
      // If we've taken this branch it means we have an enum type, and it is
      // likely an integer or NSInteger that is being used by NS/CF_OPTIONS to
      // behave like a C enum in the presence of C++.
      auto enumName = typedefDecl->getName();
      ArgumentAttrs argumentAttrs(DefaultArgumentKind::None, true, enumName);
      for (auto word : toolchain::reverse(camel_case::getWords(enumName))) {
        if (camel_case::sameWordIgnoreFirstCase(word, "options")) {
          argumentAttrs.argumentKind = DefaultArgumentKind::EmptyArray;
          return argumentAttrs;
        }
      }
      if (isCFOptionsMacro(typedefDecl, nameImporter.getClangPreprocessor()))
        return argumentAttrs;
    }
  }

  // NSDictionary arguments default to [:] (or nil, if nullable) if "options",
  // "attributes", or "userInfo" occur in the argument label or (if there is no
  // argument label) at the end of the base name.
  if (auto objcPtrTy = type->getAs<clang::ObjCObjectPointerType>()) {
    if (auto objcClass = objcPtrTy->getInterfaceDecl()) {
      if (objcClass->getName() == "NSDictionary") {
        StringRef searchStr = argumentLabel;
        if (searchStr.empty() && !baseNameStr.empty())
          searchStr = baseNameStr;

        auto emptyDictionaryKind = DefaultArgumentKind::EmptyDictionary;
        if (clangOptionality == OTK_Optional)
          emptyDictionaryKind = DefaultArgumentKind::NilLiteral;

        bool sawInfo = false;
        for (auto word : toolchain::reverse(camel_case::getWords(searchStr))) {
          if (camel_case::sameWordIgnoreFirstCase(word, "options"))
            return emptyDictionaryKind;

          if (camel_case::sameWordIgnoreFirstCase(word, "attributes"))
            return emptyDictionaryKind;

          if (camel_case::sameWordIgnoreFirstCase(word, "info")) {
            sawInfo = true;
            continue;
          }

          if (sawInfo && camel_case::sameWordIgnoreFirstCase(word, "user"))
            return emptyDictionaryKind;

          if (argumentLabel.empty())
            break;

          sawInfo = false;
        }
      }
    }
  }

  return DefaultArgumentKind::None;
}

/// Adjust the result type of a throwing function based on the
/// imported error information.
static ImportedType
adjustResultTypeForThrowingFunction(ForeignErrorConvention::Info errorInfo,
                                    ImportedType importedType) {
  switch (errorInfo.TheKind) {
  case ForeignErrorConvention::ZeroResult:
  case ForeignErrorConvention::NonZeroResult:
    // Check for a bad override.
    if (importedType.getType()->isVoid())
      return {Type(), false};
    return {TupleType::getEmpty(importedType.getType()->getASTContext()),
            false};

  case ForeignErrorConvention::NilResult:
    if (Type unwrappedTy = importedType.getType()->getOptionalObjectType())
      return {unwrappedTy, false};
    // Check for a bad override.
    if (importedType.getType()->isVoid())
      return {Type(), false};
    // It's possible an Objective-C method overrides the base method to never
    // fail, and marks the method _Nonnull to indicate that. Codira can't
    // represent that, but it shouldn't fall over either.
    return importedType;

  case ForeignErrorConvention::ZeroPreservedResult:
    // Check for a bad override.
    if (importedType.getType()->isVoid())
      return {Type(), false};
    return importedType;

  case ForeignErrorConvention::NonNilError:
    return importedType;
  }

  toolchain_unreachable("Invalid ForeignErrorConvention.");
}

/// Produce the foreign error convention from the imported error info,
/// error parameter type, and original result type.
static ForeignErrorConvention
getForeignErrorInfo(ForeignErrorConvention::Info errorInfo,
                    CanType errorParamTy, CanType origResultTy) {
  assert(errorParamTy && "not fully initialized!");
  using FEC = ForeignErrorConvention;
  auto ParamIndex = errorInfo.ErrorParameterIndex;
  auto IsOwned = (FEC::IsOwned_t) errorInfo.ErrorIsOwned;
  auto ReplaceParamWithVoid = errorInfo.ErrorParameterIsReplaced
                                ? FEC::IsReplaced
                                : FEC::IsNotReplaced;
  switch (errorInfo.TheKind) {
  case FEC::ZeroResult:
    return FEC::getZeroResult(ParamIndex, IsOwned, ReplaceParamWithVoid,
                              errorParamTy, origResultTy);
  case FEC::NonZeroResult:
    return FEC::getNonZeroResult(ParamIndex, IsOwned, ReplaceParamWithVoid,
                                 errorParamTy, origResultTy);
  case FEC::ZeroPreservedResult:
    return FEC::getZeroPreservedResult(ParamIndex, IsOwned,
                                       ReplaceParamWithVoid, errorParamTy);
  case FEC::NilResult:
    return FEC::getNilResult(ParamIndex, IsOwned, ReplaceParamWithVoid,
                             errorParamTy);
  case FEC::NonNilError:
    return FEC::getNonNilError(ParamIndex, IsOwned, ReplaceParamWithVoid,
                               errorParamTy);
  }
  toolchain_unreachable("bad error convention");
}

// 'toDC' must be a subclass or a type conforming to the protocol
// 'fromDC'.
static Type mapGenericArgs(const DeclContext *fromDC,
                           const DeclContext *toDC, Type type) {
  if (fromDC == toDC)
    return type;

  auto subs = toDC->getDeclaredInterfaceType()->getContextSubstitutionMap(fromDC);
  return type.subst(subs);
}

/// Decompose the type of a completion handler parameter in a function
/// imported as 'async' and produce the result type of the 'async' function.
static Type decomposeCompletionHandlerType(
   Type paramTy, ForeignAsyncConvention::Info info) {
  auto fnType = paramTy->lookThroughAllOptionalTypes()->getAs<FunctionType>();
  if (!fnType)
    return Type();

  SmallVector<TupleTypeElt, 2> resultTypeElts;
  auto params = fnType->getParams();
  for (unsigned paramIdx : indices(params)) {
    const auto &param = params[paramIdx];
    if (param.isInOut() || param.isVariadic())
      return Type();

    // If there are error-related parameters to the completion handler, they are
    // not part of the result type of the asynchronous function.
    if (info.completionHandlerErrorParamIndex() &&
        paramIdx == *info.completionHandlerErrorParamIndex())
      continue;
    if (info.completionHandlerFlagParamIndex() &&
        paramIdx == *info.completionHandlerFlagParamIndex())
      continue;

    resultTypeElts.push_back(param.getPlainType());
  }

  switch (resultTypeElts.size()) {
  case 0:
    return paramTy->getASTContext().getVoidType();

  case 1:
    return resultTypeElts.front().getType();

  default:
    return TupleType::get(resultTypeElts, paramTy->getASTContext());
  }
}

ImportedType ClangImporter::Implementation::importEffectfulPropertyType(
                                        const clang::ObjCMethodDecl *decl,
                                        DeclContext *dc,
                                        importer::ImportedName name,
                                        bool isFromSystemModule) {
  // here we expect a method that is being imported as an effectful property.
  // thus, we currently require async info.
  if (!name.getAsyncInfo())
    return ImportedType();

  // a variadic method doesn't make sense here
  if (decl->isVariadic())
    return ImportedType();

  // Our strategy here is to determine what the return type of the method would
  // be, had we imported it as a method.

  std::optional<ForeignAsyncConvention> asyncConvention;
  std::optional<ForeignErrorConvention> errorConvention;

  const auto kind = SpecialMethodKind::Regular;

  // Import the parameter list and result type.
  ParameterList *bodyParams = nullptr;
  auto methodReturnType = importMethodParamsAndReturnType(
      dc, decl, decl->parameters(), false,
      isFromSystemModule, &bodyParams, name,
      asyncConvention, errorConvention, kind);

  // was there a problem during import?
  if (!methodReturnType)
    return ImportedType();

  assert(bodyParams);

  // getter mustn't have any parameters!
  if (bodyParams->size() != 0) {
    return ImportedType();
  }

  // We expect that the method, after import, will have only an async convention
  if (!asyncConvention || errorConvention)
    return ImportedType();

  return methodReturnType;
}

ImportedType ClangImporter::Implementation::importMethodParamsAndReturnType(
    const DeclContext *dc, const clang::ObjCMethodDecl *clangDecl,
    ArrayRef<const clang::ParmVarDecl *> params, bool isVariadic,
    bool isFromSystemModule, ParameterList **bodyParams,
    ImportedName importedName,
    std::optional<ForeignAsyncConvention> &asyncConvention,
    std::optional<ForeignErrorConvention> &foreignErrorInfo,
    SpecialMethodKind kind) {

  // Cannot import variadic types unless specially handled before calling this
  // function.
  if (isVariadic || clangDecl->sel_param_end() != clangDecl->param_end())
    return {Type(), false};

  // Clang doesn't provide pragmas for auditing the CF behavior of
  // ObjC methods, but it does have attributes for declaring
  // return-type management:
  //   - cf_returns_retained and cf_returns_not_retained are obvious
  //   - objc_returns_inner_pointer is sometimes used on methods
  //     returning CF types as a workaround for ARC not managing CF
  //     objects
  ImportTypeKind resultKind;
  if (isObjCMethodResultAudited(clangDecl))
    resultKind = ImportTypeKind::AuditedResult;
  else
    resultKind = ImportTypeKind::Result;

  // The member was defined in 'origDC', but is being imported into 'dc'.
  // 'dc' must be a subclass or a type conforming to protocol.
  DeclContext *origDC = importDeclContextOf(clangDecl,
                                            clangDecl->getDeclContext());
  assert(origDC);

  // Import the result type.
  CanType origCodiraResultTy;
  std::optional<ForeignErrorConvention::Info> errorInfo =
      importedName.getErrorInfo();
  auto asyncInfo = importedName.getAsyncInfo();
  bool isAsync = asyncInfo.has_value();
  if (!isAsync)
    asyncInfo = importedName.getAsyncAlternateInfo();

  OptionalTypeKind OptionalityOfReturn;
  if (clangDecl->hasAttr<clang::ReturnsNonNullAttr>()) {
    OptionalityOfReturn = OTK_None;
  } else {
    OptionalityOfReturn = OTK_ImplicitlyUnwrappedOptional;
  }

  bool allowNSUIntegerAsIntInResult = isFromSystemModule;
  if (allowNSUIntegerAsIntInResult) {
    clang::Selector sel = clangDecl->getSelector();
    StringRef name = sel.getNameForSlot(0);
    if (!name.empty()) {
      allowNSUIntegerAsIntInResult = !nameContainsUnsigned(name);
    }
  }

  ImportDiagnosticAdder addImportDiag(*this, clangDecl,
                                      clangDecl->getLocation());
  clang::QualType resultType = desugarIfElaborated(clangDecl->getReturnType());
  ImportedType importedType = importer::findOptionSetEnum(resultType, *this);
  if (!importedType)
    importedType = importType(resultType, resultKind, addImportDiag,
                              allowNSUIntegerAsIntInResult, Bridgeability::Full,
                              getImportTypeAttrs(clangDecl),
                              OptionalityOfReturn);

  // Adjust the result type for a throwing function.
  if (importedType.getType() && errorInfo) {

    // Get the original unbridged result type.
    auto origImportedType =
        importType(resultType, resultKind, addImportDiag,
                   allowNSUIntegerAsIntInResult, Bridgeability::None,
                   getImportTypeAttrs(clangDecl), OptionalityOfReturn);
    origCodiraResultTy = origImportedType.getType()->getCanonicalType();

    importedType =
        adjustResultTypeForThrowingFunction(*errorInfo, importedType);
  }

  auto languageResultTy = importedType.getType();

  if (languageResultTy &&
      clangDecl->getMethodFamily() == clang::OMF_performSelector) {
    // performSelector methods that return 'id' should be imported into Codira
    // as returning Unmanaged<AnyObject>.
    Type nonOptionalTy = languageResultTy->getOptionalObjectType();
    bool resultIsOptional = (bool) nonOptionalTy;
    if (!nonOptionalTy)
      nonOptionalTy = languageResultTy;

    // Undo 'Any' bridging.
    if (nonOptionalTy->isAny())
      nonOptionalTy = CodiraContext.getAnyObjectType();

    if (nonOptionalTy->isAnyClassReferenceType()) {
      languageResultTy = getUnmanagedType(*this, nonOptionalTy);
      if (resultIsOptional)
        languageResultTy = OptionalType::get(languageResultTy);
    }
  }

  if (!languageResultTy) {
    addImportDiagnostic(clangDecl, Diagnostic(diag::return_type_not_imported),
                        clangDecl->getSourceRange().getBegin());
    return {Type(), false};
  }

  languageResultTy = mapGenericArgs(origDC, dc, languageResultTy);

  CanType errorParamType;
  CanType completionHandlerType;

  SmallBitVector nonNullArgs = getNonNullArgs(clangDecl, params);

  // Import the parameters.
  SmallVector<ParamDecl*, 4> languageParams;

  auto addEmptyTupleParameter = [&](Identifier argName) {
    // It doesn't actually matter which DeclContext we use, so just
    // use the imported header unit.
    auto type = TupleType::getEmpty(CodiraContext);
    auto var = new (CodiraContext) ParamDecl(SourceLoc(),
                                            SourceLoc(), argName,
                                            SourceLoc(), argName,
                                            ImportedHeaderUnit);
    var->setSpecifier(ParamSpecifier::Default);
    var->setInterfaceType(type);
    languageParams.push_back(var);
  };

  auto argNames = importedName.getDeclName().getArgumentNames();
  unsigned nameIndex = 0;
  for (size_t paramIndex = 0, e = params.size(); paramIndex != e;
       ++paramIndex) {
    auto param = params[paramIndex];
    auto paramTy = param->getType();
    auto paramIsError = errorInfo && paramIndex == errorInfo->ErrorParameterIndex;
    if (paramTy->isVoidType()) {
      assert(!paramIsError);
      ++nameIndex;
      continue;
    }

    bool paramIsCompletionHandler =
        asyncInfo && paramIndex == asyncInfo->completionHandlerParamIndex();
    // Figure out if this is a completion handler parameter whose error
    // parameter is used to indicate throwing.
    std::optional<unsigned> completionHandlerErrorParamIndex;
    if (isAsync && paramIsCompletionHandler) {
      completionHandlerErrorParamIndex =
          asyncInfo->completionHandlerErrorParamIndex();
    }

    // Check nullability of the parameter.
    bool knownNonNull = !nonNullArgs.empty() && nonNullArgs[paramIndex];
    OptionalTypeKind optionalityOfParam =
        getParamOptionality(param, knownNonNull);

    bool allowNSUIntegerAsIntInParam = isFromSystemModule;
    if (allowNSUIntegerAsIntInParam) {
      StringRef name;
      clang::Selector sel = clangDecl->getSelector();
      if (nameIndex < sel.getNumArgs())
        name = sel.getNameForSlot(nameIndex);
      if (name.empty() && nameIndex == 0)
        name = sel.getNameForSlot(0);
      if (!name.empty())
        allowNSUIntegerAsIntInParam = !nameContainsUnsigned(name);
    }

    ImportDiagnosticAdder paramAddDiag(*this, clangDecl, param->getLocation());

    auto languageParamTyOpt = importParameterType(
        origDC, clangDecl, param, optionalityOfParam,
        allowNSUIntegerAsIntInParam,
        kind == SpecialMethodKind::NSDictionarySubscriptGetter, paramIsError,
        paramIsCompletionHandler, completionHandlerErrorParamIndex,
        ArrayRef<GenericTypeParamDecl *>(), paramAddDiag);
    if (!languageParamTyOpt) {
      addImportDiagnostic(param,
                          Diagnostic(diag::parameter_type_not_imported, param),
                          param->getSourceRange().getBegin());
      return {Type(), false};
    }
    auto languageParamTy = languageParamTyOpt->languageTy;
    bool isInOut = languageParamTyOpt->isInOut;
    bool isConsuming = languageParamTyOpt->isConsuming;
    bool isParamTypeImplicitlyUnwrapped =
        languageParamTyOpt->isParamTypeImplicitlyUnwrapped;

    languageParamTy = mapGenericArgs(origDC, dc, languageParamTy);

    // If this is the error parameter, remember it, but don't build it
    // into the parameter type.
    if (paramIsError) {
      errorParamType = languageParamTy->getCanonicalType();

      // ...unless we're supposed to replace it with ().
      if (errorInfo->ErrorParameterIsReplaced) {
        addEmptyTupleParameter(argNames[nameIndex]);
        ++nameIndex;
      }
      continue;
    }

    // If this is a completion handler, figure out it's effect on the result
    // type but don't build it into the parameter type.
    if (isAsync && paramIsCompletionHandler) {
      if (Type replacedCodiraResultTy =
              decomposeCompletionHandlerType(languageParamTy, *asyncInfo)) {
        languageResultTy = replacedCodiraResultTy;

        ImportTypeAttrs attrs;
        // This is required because a parameter type of a sync variant and
        // a completion type of an async variant have to match exactly.
        if (isSendableInferenceOnCompletionHandlerParameterAllowed(origDC,
                                                                   clangDecl)) {
          attrs |= ImportTypeAttr::DefaultsToSendable;
        }

        // Import the original completion handler type without adjustments.
        Type origCodiraParamTy =
            importType(paramTy, ImportTypeKind::CompletionHandlerParameter,
                       paramAddDiag, allowNSUIntegerAsIntInParam,
                       Bridgeability::Full, attrs,
                       optionalityOfParam,
                       /*resugarNSErrorPointer=*/!paramIsError, std::nullopt)
                .getType();
        completionHandlerType =
            mapGenericArgs(origDC, dc, origCodiraParamTy)->getCanonicalType();
        continue;
      }

      toolchain_unreachable("async info computed incorrectly?");
    }

    // Figure out the name for this argument, which comes from the method name.
    Identifier name;
    if (nameIndex < argNames.size()) {
      name = argNames[nameIndex];
    }
    ++nameIndex;

    // Set up the parameter info
    auto paramInfo =
        getParameterInfo(this, param, name, languageParamTy, isInOut, isConsuming,
                         isParamTypeImplicitlyUnwrapped);

    // Determine whether we have a default argument.
    if (kind == SpecialMethodKind::Regular ||
        kind == SpecialMethodKind::Constructor) {
      bool isLastParameter =
          (paramIndex == params.size() - 1) ||
          (paramIndex == params.size() - 2 && errorInfo &&
           errorInfo->ErrorParameterIndex == params.size() - 1);

      auto defaultArg = inferDefaultArgument(
          param->getType(), optionalityOfParam,
          importedName.getDeclName().getBaseName(), name.str(), paramIndex == 0,
          isLastParameter, getNameImporter());
      if (defaultArg != DefaultArgumentKind::None)
        paramInfo->setDefaultArgumentKind(defaultArg);
    }
    languageParams.push_back(paramInfo);
  }

  // If we have a constructor with no parameters and a name with an
  // argument name, synthesize a Void parameter with that name.
  if (kind == SpecialMethodKind::Constructor && params.empty() &&
      argNames.size() == 1) {
    addEmptyTupleParameter(argNames[0]);
  }

  if (importedName.hasCustomName() && argNames.size() != languageParams.size()) {
    // Note carefully: we're emitting a warning in the /Clang/ buffer.
    if (clangDecl->getLocation().isValid()) {
      HeaderLoc methodLoc(clangDecl->getLocation());
      diagnose(methodLoc, diag::invalid_language_name_method,
               languageParams.size() < argNames.size(),
               languageParams.size(), argNames.size());
      ModuleDecl *parentModule = dc->getParentModule();
      if (parentModule != ImportedHeaderUnit->getParentModule()) {
        diagnose(methodLoc, diag::unresolvable_clang_decl_is_a_framework_bug,
                 parentModule->getName().str());
      }
    }
    return {Type(), false};
  }


  // Form the parameter list.
  *bodyParams = ParameterList::create(CodiraContext, languageParams);

  if (clangDecl->hasAttr<clang::NoReturnAttr>()) {
    origCodiraResultTy = CodiraContext.getNeverType()->getCanonicalType();
    languageResultTy = CodiraContext.getNeverType();
  }

  if (isAsync) {
    asyncConvention = ForeignAsyncConvention(
        completionHandlerType, asyncInfo->completionHandlerParamIndex(),
        asyncInfo->completionHandlerErrorParamIndex(),
        asyncInfo->completionHandlerFlagParamIndex(),
        asyncInfo->completionHandlerFlagIsErrorOnZero());
  }

  if (errorInfo) {
    foreignErrorInfo = getForeignErrorInfo(*errorInfo, errorParamType,
                                           origCodiraResultTy);
  }

  return {languageResultTy,
          importedType.isImplicitlyUnwrapped()};
}

ImportedType ClangImporter::Implementation::importAccessorParamsAndReturnType(
    const DeclContext *dc, const clang::ObjCPropertyDecl *property,
    const clang::ObjCMethodDecl *clangDecl, bool isFromSystemModule,
    ImportedName functionName, language::ParameterList **params) {
  // Note: We're using a pointer instead of a reference here to make it clear
  // at the call site that this is an out-parameter.
  assert(params && "'params' is a required out-parameter");

  // Determine if the method is a property getter or setter.
  bool isGetter;
  if (clangDecl->parameters().empty())
    isGetter = true;
  else if (clangDecl->parameters().size() == 1)
    isGetter = false;
  else
    toolchain_unreachable("not a valid accessor");

  // The member was defined in 'origDC', but is being imported into 'dc'.
  // 'dc' must be a subclass or a type conforming to protocol.
  // FIXME: Duplicated from importMethodParamsAndReturnType.
  DeclContext *origDC = importDeclContextOf(property,
                                            property->getDeclContext());
  if (!origDC)
    return {Type(), false};

  auto fieldType =
      desugarIfElaborated(isGetter ? clangDecl->getReturnType()
                                   : clangDecl->getParamDecl(0)->getType());
  // Import the property type, independent of what kind of accessor this is.
  ImportedType importedType = importer::findOptionSetEnum(fieldType, *this);
  if (!importedType)
    importedType = importPropertyType(property, isFromSystemModule);
  if (!importedType)
    return {Type(), false};

  auto propertyTy = mapGenericArgs(origDC, dc, importedType.getType());
  bool isIUO = importedType.isImplicitlyUnwrapped();

  // Now build up the resulting FunctionType and parameters.
  Type resultTy;
  if (isGetter) {
    *params = ParameterList::createEmpty(CodiraContext);
    resultTy = propertyTy;
  } else {
    const clang::ParmVarDecl *param = clangDecl->parameters().front();
    ImportedName fullBodyName = importFullName(param, CurrentVersion);
    Identifier bodyName = fullBodyName.getBaseIdentifier(CodiraContext);
    SourceLoc nameLoc = importSourceLoc(param->getLocation());
    Identifier argLabel = functionName.getDeclName().getArgumentNames().front();
    auto paramInfo
      = createDeclWithClangNode<ParamDecl>(param, AccessLevel::Private,
                                           /*let loc*/SourceLoc(),
                                           /*label loc*/SourceLoc(),
                                           argLabel, nameLoc, bodyName,
                                           /*dummy DC*/ImportedHeaderUnit);
    paramInfo->setSpecifier(ParamSpecifier::Default);
    paramInfo->setInterfaceType(propertyTy);

    *params = ParameterList::create(CodiraContext, paramInfo);
    resultTy = CodiraContext.getVoidType();
    if (!resultTy)
      addImportDiagnostic(clangDecl,
                          Diagnostic(diag::bridged_type_not_found_in_module,
                                     "Void", "Codira"),
                          clangDecl->getLocation());
    isIUO = false;
  }

  return {resultTy, isIUO};
}


ModuleDecl *ClangImporter::Implementation::getStdlibModule() {
  return CodiraContext.getStdlibModule(true);
}

ModuleDecl *ClangImporter::Implementation::getNamedModule(StringRef name) {
  return CodiraContext.getLoadedModule(CodiraContext.getIdentifier(name));
}

static ModuleDecl *tryLoadModule(ASTContext &C,
                             Identifier moduleName,
                             bool importForwardDeclarations,
                             toolchain::DenseMap<Identifier, ModuleDecl *>
                               &checkedModules) {
  // If we've already done this check, return the cached result.
  auto [known, inserted] = checkedModules.try_emplace(moduleName, nullptr);
  if (!inserted)
    return known->second;

  ModuleDecl *module;

  // If we're synthesizing forward declarations, we don't want to pull in
  // the module too eagerly.
  if (importForwardDeclarations)
    module = C.getLoadedModule(moduleName);
  else
    module = C.getModuleByIdentifier(moduleName);

  known->getSecond() = module;
  return module;
}

ModuleDecl *ClangImporter::Implementation::tryLoadFoundationModule() {
  return tryLoadModule(CodiraContext, CodiraContext.Id_Foundation,
                       ImportForwardDeclarations, checkedModules);
}

bool ClangImporter::Implementation::canImportFoundationModule() {
  ImportPath::Module::Builder builder(CodiraContext.Id_Foundation);
  auto modulePath = builder.get();
  return CodiraContext.canImportModule(modulePath, SourceLoc());
}

Type ClangImporter::Implementation::getNamedCodiraType(ModuleDecl *module,
                                                      StringRef name) {
  return CodiraContext.getNamedCodiraType(module, name);
}

Type ClangImporter::Implementation::getNamedCodiraType(StringRef moduleName,
                                                      StringRef name) {
  // Try to load the module.
  auto module = tryLoadModule(CodiraContext,
                              CodiraContext.getIdentifier(moduleName),
                              ImportForwardDeclarations, checkedModules);
  if (!module) return Type();

  return getNamedCodiraType(module, name);
}

Decl *ClangImporter::Implementation::importDeclByName(StringRef name) {
  auto &sema = Instance->getSema();

  // Map the name. If we can't represent the Codira name in Clang, bail out now.
  auto clangName = &getClangASTContext().Idents.get(name);

  // Perform name lookup into the global scope.
  // FIXME: Map source locations over.
  clang::LookupResult lookupResult(sema, clangName, clang::SourceLocation(),
                                   clang::Sema::LookupOrdinaryName);
  lookupResult.setAllowHidden(true);
  if (!sema.LookupName(lookupResult, /*Scope=*/sema.TUScope)) {
    return nullptr;
  }

  for (auto decl : lookupResult) {
    if (auto languageDecl =
            importDecl(decl->getUnderlyingDecl(), CurrentVersion)) {
      return languageDecl;
    }
  }

  return nullptr;
}

Type ClangImporter::Implementation::getNSObjectType() {
  if (NSObjectTy)
    return NSObjectTy;

  if (auto decl = dyn_cast_or_null<ClassDecl>(importDeclByName("NSObject"))) {
    NSObjectTy = decl->getDeclaredInterfaceType();
    return NSObjectTy;
  }

  return Type();
}

bool ClangImporter::Implementation::matchesHashableBound(Type type) {
  Type NSObjectType = getNSObjectType();
  if (!NSObjectType)
    return false;

  // Match generic parameters against their bounds.
  if (auto *genericTy = type->getAs<GenericTypeParamType>()) {
    if (auto *generic = genericTy->getDecl()) {
      auto genericSig =
        generic->getDeclContext()->getGenericSignatureOfContext();
      if (genericSig && genericSig->getRequiredProtocols(type).empty()) {
        type = genericSig->getSuperclassBound(type);
        if (!type)
          return false;
      }
    }
  }

  // Existentials cannot match the Hashable bound.
  if (type->isAnyExistentialType())
    return false;

  // Class type that inherits from NSObject.
  if (NSObjectType->isExactSuperclassOf(type))
    return true;

  // Struct or enum type must have been bridged.
  // TODO: Check that the bridged type is Hashable?
  if (type->getStructOrBoundGenericStruct() ||
      type->getEnumOrBoundGenericEnum()) {
    auto nominal = type->getAnyNominal();
    auto hashable = CodiraContext.getProtocol(KnownProtocolKind::Hashable);
    SmallVector<ProtocolConformance *, 2> conformances;
    return hashable &&
      nominal->lookupConformance(hashable, conformances);
  }

  return false;
}

static Type getNamedProtocolType(ClangImporter::Implementation &impl,
                                 StringRef name) {
  auto &sema = impl.getClangSema();
  auto clangName = &sema.getASTContext().Idents.get(name);
  assert(clangName);

  // Perform name lookup into the global scope.
  clang::LookupResult lookupResult(sema, clangName, clang::SourceLocation(),
                                   clang::Sema::LookupObjCProtocolName);
  lookupResult.setAllowHidden(true);
  if (!sema.LookupName(lookupResult, /*Scope=*/sema.TUScope))
    return Type();

  for (auto decl : lookupResult) {
    if (auto languageDecl =
            impl.importDecl(decl->getUnderlyingDecl(), impl.CurrentVersion)) {
      if (auto protoDecl =
              dynCastIgnoringCompatibilityAlias<ProtocolDecl>(languageDecl)) {
        return protoDecl->getDeclaredInterfaceType();
      }
    }
  }

  return Type();
}

Type ClangImporter::Implementation::getNSObjectProtocolType() {
  return getNamedProtocolType(*this, "NSObject");
}
