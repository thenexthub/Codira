//===--- ClangAdapter.cpp - Interfaces with Clang entities ----------------===//
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
// This file provides convenient and canonical interfaces with Clang entities,
// serving as both a useful place to put utility functions and a canonical
// interface that can abstract nitty gritty Clang internal details.
//
//===----------------------------------------------------------------------===//

#include "CFTypeInfo.h"
#include "ClangAdapter.h"
#include "ImportName.h"
#include "ImporterImpl.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/Lex/Lexer.h"
#include "language/Core/Sema/Lookup.h"
#include "language/Core/Sema/Sema.h"

using namespace language;
using namespace importer;

/// Get a bit vector indicating which arguments are non-null for a
/// given function or method.
SmallBitVector
importer::getNonNullArgs(const language::Core::Decl *decl,
                         ArrayRef<const language::Core::ParmVarDecl *> params) {
  SmallBitVector result;
  if (!decl)
    return result;

  for (const auto *nonnull : decl->specific_attrs<language::Core::NonNullAttr>()) {
    if (!nonnull->args_size()) {
      // Easy case: all pointer arguments are non-null.
      if (result.empty())
        result.resize(params.size(), true);
      else
        result.set(0, params.size());

      return result;
    }

    // Mark each of the listed parameters as non-null.
    if (result.empty())
      result.resize(params.size(), false);

    for (auto paramIdx : nonnull->args()) {
      unsigned idx = paramIdx.getASTIndex();
      if (idx < result.size())
        result.set(idx);
    }
  }

  return result;
}

std::optional<const language::Core::Decl *>
importer::getDefinitionForClangTypeDecl(const language::Core::Decl *D) {
  if (auto OID = dyn_cast<language::Core::ObjCInterfaceDecl>(D))
    return OID->getDefinition();

  if (auto TD = dyn_cast<language::Core::TagDecl>(D))
    return TD->getDefinition();

  if (auto OPD = dyn_cast<language::Core::ObjCProtocolDecl>(D))
    return OPD->getDefinition();

  return std::nullopt;
}

static bool isInLocalScope(const language::Core::Decl *D) {
  const language::Core::DeclContext *LDC = D->getLexicalDeclContext();
  while (true) {
    if (LDC->isFunctionOrMethod())
      return true;
    if (!isa<language::Core::TagDecl>(LDC))
      return false;
    if (const auto *CRD = dyn_cast<language::Core::CXXRecordDecl>(LDC))
      if (CRD->isLambda())
        return true;
    LDC = LDC->getLexicalParent();
  }
  return false;
}

const language::Core::Decl *
importer::getFirstNonLocalDecl(const language::Core::Decl *D) {
  D = D->getCanonicalDecl();
  auto iter = toolchain::find_if(D->redecls(), [](const language::Core::Decl *next) -> bool {
    return !isInLocalScope(next);
  });
  if (iter == D->redecls_end())
    return nullptr;
  return *iter;
}

std::optional<language::Core::Module *>
importer::getClangSubmoduleForDecl(const language::Core::Decl *D,
                                   bool allowForwardDeclaration) {
  const language::Core::Decl *actual = nullptr;

  // Put an Objective-C class into the module that contains the @interface
  // definition, not just some @class forward declaration.
  if (auto maybeDefinition = getDefinitionForClangTypeDecl(D)) {
    actual = maybeDefinition.value();
    if (!actual && !allowForwardDeclaration)
      return std::nullopt;
  }

  if (!actual)
    actual = getFirstNonLocalDecl(D);

  return actual->getImportedOwningModule();
}

/// Retrieve the instance type of the given Clang declaration context.
language::Core::QualType
importer::getClangDeclContextType(const language::Core::DeclContext *dc) {
  auto &ctx = dc->getParentASTContext();
  if (auto objcClass = dyn_cast<language::Core::ObjCInterfaceDecl>(dc))
    return ctx.getObjCObjectPointerType(ctx.getObjCInterfaceType(objcClass));

  if (auto objcCategory = dyn_cast<language::Core::ObjCCategoryDecl>(dc)) {
    if (objcCategory->isInvalidDecl())
      return language::Core::QualType();

    return ctx.getObjCObjectPointerType(
        ctx.getObjCInterfaceType(objcCategory->getClassInterface()));
  }

  if (auto constProto = dyn_cast<language::Core::ObjCProtocolDecl>(dc)) {
    auto proto = const_cast<language::Core::ObjCProtocolDecl *>(constProto);
    auto type = ctx.getObjCObjectType(ctx.ObjCBuiltinIdTy, {}, {proto}, false);
    return ctx.getObjCObjectPointerType(type);
  }

  if (auto tag = dyn_cast<language::Core::TagDecl>(dc)) {
    return ctx.getTagDeclType(tag);
  }

  return language::Core::QualType();
}

/// Determine whether this is the name of a collection with a single
/// element type.
static bool isCollectionName(StringRef typeName) {
  auto lastWord = camel_case::getLastWord(typeName);
  return lastWord == "Array" || lastWord == "Set";
}

/// Retrieve the name of the given Clang type for use when omitting
/// needless words.
OmissionTypeName importer::getClangTypeNameForOmission(language::Core::ASTContext &ctx,
                                                       language::Core::QualType type) {
  if (type.isNull())
    return OmissionTypeName();

  // Dig through the type, looking for a typedef-name and stripping
  // references along the way.
  StringRef lastTypedefName;
  do {
    // The name of a typedef-name.
    auto typePtr = type.getTypePtr();
    if (auto typedefType = dyn_cast<language::Core::TypedefType>(typePtr)) {
      auto name = typedefType->getDecl()->getName();

      // Objective-C selector type.
      if (ctx.hasSameUnqualifiedType(type, ctx.getObjCSelType()) &&
          name == "SEL")
        return "Selector";

      // Objective-C "id" type.
      if (type->isObjCIdType() && name == "id")
        return "Object";

      // Objective-C "Class" type.
      if (type->isObjCClassType() && name == "Class")
        return "Class";

      // Objective-C "BOOL" type.
      if (name == "BOOL")
        return OmissionTypeName("Bool", OmissionTypeFlags::Boolean);

      // If this is an imported CF type, use that name.
      StringRef CFName = getCFTypeName(typedefType->getDecl());
      if (!CFName.empty())
        return CFName;

      // If we have NS(U)Integer or CGFloat, return it.
      if (name == "NSInteger" || name == "NSUInteger" || name == "CGFloat")
        return name;

      // If it's a collection name and of pointer type, call it an
      // array of the pointee type.
      if (isCollectionName(name)) {
        if (auto ptrType = type->getAs<language::Core::PointerType>()) {
          return OmissionTypeName(
              name, std::nullopt,
              getClangTypeNameForOmission(ctx, ptrType->getPointeeType()).Name);
        }
      }

      // Otherwise, desugar one level...
      lastTypedefName = name;
      type = typedefType->getDecl()->getUnderlyingType();
      continue;
    }

    // For array types, convert the element type and treat this an as array.
    if (auto arrayType = dyn_cast<language::Core::ArrayType>(typePtr)) {
      return OmissionTypeName(
          "Array", std::nullopt,
          getClangTypeNameForOmission(ctx, arrayType->getElementType()).Name);
    }

    // Look through reference types.
    if (auto refType = dyn_cast<language::Core::ReferenceType>(typePtr)) {
      type = refType->getPointeeTypeAsWritten();
      continue;
    }

    // Look through pointer types.
    if (auto ptrType = dyn_cast<language::Core::PointerType>(typePtr)) {
      type = ptrType->getPointeeType();
      continue;
    }

    // Try to desugar one level...
    language::Core::QualType desugared = type.getSingleStepDesugaredType(ctx);
    if (desugared.getTypePtr() == type.getTypePtr())
      break;

    type = desugared;
  } while (true);

  // Objective-C object pointers.
  if (auto objcObjectPtr = type->getAs<language::Core::ObjCObjectPointerType>()) {
    auto objcClass = objcObjectPtr->getInterfaceDecl();

    // For id<Proto> or NSObject<Proto>, retrieve the name of "Proto".
    if (objcObjectPtr->getNumProtocols() == 1 &&
        (!objcClass || objcClass->getName() == "NSObject"))
      return (*objcObjectPtr->qual_begin())->getName();

    // If there is a class, use it.
    if (objcClass) {
      // If this isn't the name of an Objective-C collection, we're done.
      auto className = objcClass->getName();
      if (!isCollectionName(className))
        return className;

      // If we don't have type parameters, use the prefix of the type
      // name as the collection element type.
      if (objcClass && !objcClass->getTypeParamList()) {
        unsigned lastWordSize = camel_case::getLastWord(className).size();
        StringRef elementName =
            className.substr(0, className.size() - lastWordSize);
        return OmissionTypeName(className, std::nullopt, elementName);
      }

      // If we don't have type arguments, the collection element type
      // is "Object".
      auto typeArgs = objcObjectPtr->getTypeArgs();
      if (typeArgs.empty())
        return OmissionTypeName(className, std::nullopt, "Object");

      return OmissionTypeName(
          className, std::nullopt,
          getClangTypeNameForOmission(ctx, typeArgs[0]).Name);
    }

    // Objective-C "id" type.
    if (objcObjectPtr->isObjCIdType())
      return "Object";

    // Objective-C "Class" type.
    if (objcObjectPtr->isObjCClassType())
      return "Class";

    return StringRef();
  }

  // Handle builtin types by importing them and getting the Codira name.
  if (auto builtinTy = type->getAs<language::Core::BuiltinType>()) {
    // Names of integer types.
    static const char *intTypeNames[] = {"UInt8", "UInt16", "UInt32", "UInt64",
                                         "UInt128"};

    /// Retrieve the name for an integer type based on its size.
    auto getIntTypeName = [&](bool isSigned) -> StringRef {
      switch (ctx.getTypeSize(builtinTy)) {
      case 8:
        return StringRef(intTypeNames[0]).substr(isSigned ? 1 : 0);
      case 16:
        return StringRef(intTypeNames[1]).substr(isSigned ? 1 : 0);
      case 32:
        return StringRef(intTypeNames[2]).substr(isSigned ? 1 : 0);
      case 64:
        return StringRef(intTypeNames[3]).substr(isSigned ? 1 : 0);
      case 128:
        return StringRef(intTypeNames[4]).substr(isSigned ? 1 : 0);
      default:
        toolchain_unreachable("bad integer type size");
      }
    };

    switch (builtinTy->getKind()) {
    case language::Core::BuiltinType::Void:
      return "Void";

    case language::Core::BuiltinType::Bool:
      return OmissionTypeName("Bool", OmissionTypeFlags::Boolean);

    case language::Core::BuiltinType::Float:
      return "Float";

    case language::Core::BuiltinType::Double:
      return "Double";

    case language::Core::BuiltinType::Char8:
      return "UInt8";

    case language::Core::BuiltinType::Char16:
      return "UInt16";

    case language::Core::BuiltinType::Char32:
      return "UnicodeScalar";

    case language::Core::BuiltinType::Char_U:
    case language::Core::BuiltinType::UChar:
    case language::Core::BuiltinType::UShort:
    case language::Core::BuiltinType::UInt:
    case language::Core::BuiltinType::ULong:
    case language::Core::BuiltinType::ULongLong:
    case language::Core::BuiltinType::UInt128:
    case language::Core::BuiltinType::WChar_U:
      return getIntTypeName(false);

    case language::Core::BuiltinType::Char_S:
    case language::Core::BuiltinType::SChar:
    case language::Core::BuiltinType::Short:
    case language::Core::BuiltinType::Int:
    case language::Core::BuiltinType::Long:
    case language::Core::BuiltinType::LongLong:
    case language::Core::BuiltinType::Int128:
    case language::Core::BuiltinType::WChar_S:
      return getIntTypeName(true);

    // Types that cannot be mapped into Codira, and probably won't ever be.
    case language::Core::BuiltinType::Dependent:
    case language::Core::BuiltinType::ARCUnbridgedCast:
    case language::Core::BuiltinType::BoundMember:
    case language::Core::BuiltinType::BuiltinFn:
    case language::Core::BuiltinType::IncompleteMatrixIdx:
    case language::Core::BuiltinType::Overload:
    case language::Core::BuiltinType::PseudoObject:
    case language::Core::BuiltinType::UnknownAny:
    case language::Core::BuiltinType::UnresolvedTemplate:
      return OmissionTypeName();

    // FIXME: Types that can be mapped, but aren't yet.
    case language::Core::BuiltinType::ShortAccum:
    case language::Core::BuiltinType::Accum:
    case language::Core::BuiltinType::LongAccum:
    case language::Core::BuiltinType::UShortAccum:
    case language::Core::BuiltinType::UAccum:
    case language::Core::BuiltinType::ULongAccum:
    case language::Core::BuiltinType::ShortFract:
    case language::Core::BuiltinType::Fract:
    case language::Core::BuiltinType::LongFract:
    case language::Core::BuiltinType::UShortFract:
    case language::Core::BuiltinType::UFract:
    case language::Core::BuiltinType::ULongFract:
    case language::Core::BuiltinType::SatShortAccum:
    case language::Core::BuiltinType::SatAccum:
    case language::Core::BuiltinType::SatLongAccum:
    case language::Core::BuiltinType::SatUShortAccum:
    case language::Core::BuiltinType::SatUAccum:
    case language::Core::BuiltinType::SatULongAccum:
    case language::Core::BuiltinType::SatShortFract:
    case language::Core::BuiltinType::SatFract:
    case language::Core::BuiltinType::SatLongFract:
    case language::Core::BuiltinType::SatUShortFract:
    case language::Core::BuiltinType::SatUFract:
    case language::Core::BuiltinType::SatULongFract:
    case language::Core::BuiltinType::Half:
    case language::Core::BuiltinType::LongDouble:
    case language::Core::BuiltinType::BFloat16:
    case language::Core::BuiltinType::Float16:
    case language::Core::BuiltinType::Float128:
    case language::Core::BuiltinType::NullPtr:
    case language::Core::BuiltinType::Ibm128:
      return OmissionTypeName();

    // Objective-C types that aren't mapped directly; rather, pointers to
    // these types will be mapped.
    case language::Core::BuiltinType::ObjCClass:
    case language::Core::BuiltinType::ObjCId:
    case language::Core::BuiltinType::ObjCSel:
      return OmissionTypeName();

    // OpenMP types that don't have Codira equivalents.
    case language::Core::BuiltinType::ArraySection:
    case language::Core::BuiltinType::OMPArrayShaping:
    case language::Core::BuiltinType::OMPIterator:
      return OmissionTypeName();

    // OpenCL builtin types that don't have Codira equivalents.
    case language::Core::BuiltinType::OCLClkEvent:
    case language::Core::BuiltinType::OCLEvent:
    case language::Core::BuiltinType::OCLSampler:
    case language::Core::BuiltinType::OCLQueue:
    case language::Core::BuiltinType::OCLReserveID:
#define IMAGE_TYPE(Name, Id, ...) case language::Core::BuiltinType::Id:
#include "language/Core/Basic/OpenCLImageTypes.def"
#define EXT_OPAQUE_TYPE(Name, Id, ...) case language::Core::BuiltinType::Id:
#include "language/Core/Basic/OpenCLExtensionTypes.def"
      return OmissionTypeName();

    // ARM SVE builtin types that don't have Codira equivalents.
#define SVE_TYPE(Name, Id, ...) case language::Core::BuiltinType::Id:
#include "language/Core/Basic/AArch64SVEACLETypes.def"
      return OmissionTypeName();

    // PPC MMA builtin types that don't have Codira equivalents.
#define PPC_VECTOR_TYPE(Name, Id, ...) case language::Core::BuiltinType::Id:
#include "language/Core/Basic/PPCTypes.def"
      return OmissionTypeName();

    // RISC-V V builtin types that don't have Codira equivalents.
#define RVV_TYPE(Name, Id, ...) case language::Core::BuiltinType::Id:
#include "language/Core/Basic/RISCVVTypes.def"
      return OmissionTypeName();

    // WASM builtin types that don't have Codira equivalents.
#define WASM_TYPE(Name, Id, ...) case language::Core::BuiltinType::Id:
#include "language/Core/Basic/WebAssemblyReferenceTypes.def"
      return OmissionTypeName();

    // AMDGPU builtins that don't have Codira equivalents.
#define AMDGPU_TYPE(Name, Id, ...) case language::Core::BuiltinType::Id:
#include "language/Core/Basic/AMDGPUTypes.def"
      return OmissionTypeName();
    }
  }

  // Tag types.
  if (auto tagType = type->getAs<language::Core::TagType>()) {
    if (tagType->getDecl()->getName().empty())
      return lastTypedefName;

    return tagType->getDecl()->getName();
  }

  // Block pointers.
  if (type->getAs<language::Core::BlockPointerType>())
    return OmissionTypeName("Block", OmissionTypeFlags::Function);

  // Function pointers.
  if (type->isFunctionType())
    return OmissionTypeName("Function", OmissionTypeFlags::Function);

  return StringRef();
}

static language::Core::CodiraNewTypeAttr *
retrieveNewTypeAttr(const language::Core::TypedefNameDecl *decl) {
  // Retrieve the attribute.
  auto attr = decl->getAttr<language::Core::CodiraNewTypeAttr>();
  if (!attr)
    return nullptr;

  // FIXME: CFErrorDomain is marked as CF_EXTENSIBLE_STRING_ENUM, but it turned
  // out to be more disruptive than not to leave it that way.
  auto name = decl->getName();
  if (name == "CFErrorDomain")
    return nullptr;

  return attr;
}

language::Core::CodiraNewTypeAttr *
importer::getCodiraNewtypeAttr(const language::Core::TypedefNameDecl *decl,
                              ImportNameVersion version) {
  // Newtype was introduced in Codira 3
  if (version <= ImportNameVersion::language2())
    return nullptr;
  return retrieveNewTypeAttr(decl);
}

// If this decl is associated with a language_newtype typedef, return it, otherwise
// null
language::Core::TypedefNameDecl *importer::findCodiraNewtype(const language::Core::NamedDecl *decl,
                                                   language::Core::Sema &clangSema,
                                                   ImportNameVersion version) {
  // Newtype was introduced in Codira 3
  if (version <= ImportNameVersion::language2())
    return nullptr;

  auto varDecl = dyn_cast<language::Core::VarDecl>(decl);
  if (!varDecl)
    return nullptr;

  if (auto typedefTy = varDecl->getType()->getAs<language::Core::TypedefType>())
    if (retrieveNewTypeAttr(typedefTy->getDecl()))
      return typedefTy->getDecl();

  // Special case: "extern NSString * fooNotification" adopts
  // NSNotificationName type, and is a member of NSNotificationName
  if (isNSNotificationGlobal(decl)) {
    language::Core::IdentifierInfo *notificationName =
        &clangSema.getASTContext().Idents.get("NSNotificationName");
    language::Core::LookupResult lookupResult(clangSema, notificationName,
                                     language::Core::SourceLocation(),
                                     language::Core::Sema::LookupOrdinaryName);
    if (!clangSema.LookupQualifiedName(
            lookupResult,
            /*LookupCtx*/ clangSema.getASTContext().getTranslationUnitDecl()))
      return nullptr;
    auto nsDecl = lookupResult.getAsSingle<language::Core::TypedefNameDecl>();
    if (!nsDecl)
      return nullptr;

    // Make sure it also has a newtype decl on it
    if (retrieveNewTypeAttr(nsDecl))
      return nsDecl;

    return nullptr;
  }

  return nullptr;
}

bool importer::isNSString(const language::Core::Type *type) {
  if (auto ptrType = type->getAs<language::Core::ObjCObjectPointerType>())
    if (auto interfaceType = ptrType->getInterfaceType())
      if (interfaceType->getDecl()->getName() == "NSString")
        return true;
  return false;
}

bool importer::isNSString(language::Core::QualType qt) {
  return qt.getTypePtrOrNull() && isNSString(qt.getTypePtrOrNull());
}

bool importer::isNSNotificationName(language::Core::QualType type) {
  if (auto *typealias = type->getAs<language::Core::TypedefType>()) {
    return typealias->getDecl()->getName() == "NSNotificationName";
  }
  return false;
}

bool importer::isNSNotificationGlobal(const language::Core::NamedDecl *decl) {
  // Looking for: extern NSString *fooNotification;

  // Must be extern global variable
  auto vDecl = dyn_cast<language::Core::VarDecl>(decl);
  if (!vDecl || !vDecl->hasExternalFormalLinkage())
    return false;

  // No explicit language_name
  if (decl->getAttr<language::Core::CodiraNameAttr>())
    return false;

  // Must end in Notification
  if (!vDecl->getDeclName().isIdentifier())
    return false;
  if (stripNotification(vDecl->getName()).empty())
    return false;

  // Must be NSString *
  if (!isNSString(vDecl->getType()))
    return false;

  // We're a match!
  return true;
}

bool importer::hasNativeCodiraDecl(const language::Core::Decl *decl) {
  if (auto *attr = decl->getAttr<language::Core::ExternalSourceSymbolAttr>())
    if (attr->getGeneratedDeclaration() && attr->getLanguage() == "Codira")
      return true;
  return false;
}

/// Translate the "nullability" notion from API notes into an optional type
/// kind.
OptionalTypeKind importer::translateNullability(
    language::Core::NullabilityKind kind, bool stripNonResultOptionality) {
  if (stripNonResultOptionality &&
      kind != language::Core::NullabilityKind::NullableResult)
    return OptionalTypeKind::OTK_None;

  switch (kind) {
  case language::Core::NullabilityKind::NonNull:
    return OptionalTypeKind::OTK_None;

  case language::Core::NullabilityKind::Nullable:
  case language::Core::NullabilityKind::NullableResult:
    return OptionalTypeKind::OTK_Optional;

  case language::Core::NullabilityKind::Unspecified:
    return OptionalTypeKind::OTK_ImplicitlyUnwrappedOptional;
  }

  toolchain_unreachable("Invalid NullabilityKind.");
  return OptionalTypeKind::OTK_Optional;
}

bool importer::isRequiredInitializer(const language::Core::ObjCMethodDecl *method) {
  // FIXME: No way to express this in Objective-C.
  return false;
}

/// Check if this method is declared in the context that conforms to
/// NSAccessibility.
static bool isAccessibilityConformingContext(const language::Core::DeclContext *ctx) {
  const language::Core::ObjCProtocolList *protocols = nullptr;

  if (auto protocol = dyn_cast<language::Core::ObjCProtocolDecl>(ctx)) {
    if (protocol->getName() == "NSAccessibility")
      return true;
    return false;
  } else if (auto interface = dyn_cast<language::Core::ObjCInterfaceDecl>(ctx))
    protocols = &interface->getReferencedProtocols();
  else if (auto category = dyn_cast<language::Core::ObjCCategoryDecl>(ctx))
    protocols = &category->getReferencedProtocols();
  else
    return false;

  for (auto pi : *protocols) {
    if (pi->getName() == "NSAccessibility")
      return true;
  }
  return false;
}

bool
importer::shouldImportPropertyAsAccessors(const language::Core::ObjCPropertyDecl *prop) {
  if (prop->hasAttr<language::Core::CodiraImportPropertyAsAccessorsAttr>())
    return true;

  // Check if the property is one of the specially handled accessibility APIs.
  //
  // These appear as both properties and methods in ObjC and should be
  // imported as methods into Codira, as a sort of least-common-denominator
  // compromise.
  if (!prop->getName().starts_with("accessibility"))
    return false;
  if (isAccessibilityConformingContext(prop->getDeclContext()))
    return true;

  return false;
}

bool importer::isInitMethod(const language::Core::ObjCMethodDecl *method) {
  // init methods are always instance methods.
  if (!method->isInstanceMethod())
    return false;

  // init methods must be classified as such by Clang.
  if (method->getMethodFamily() != language::Core::OMF_init)
    return false;

  // Codira restriction: init methods must start with the word "init".
  auto selector = method->getSelector();
  return camel_case::getFirstWord(selector.getNameForSlot(0)) == "init";
}

bool importer::isObjCId(const language::Core::Decl *decl) {
  auto typedefDecl = dyn_cast<language::Core::TypedefNameDecl>(decl);
  if (!typedefDecl)
    return false;

  if (!typedefDecl->getDeclContext()->getRedeclContext()->isTranslationUnit())
    return false;

  return typedefDecl->getName() == "id";
}

bool importer::isUnavailableInCodira(
    const language::Core::Decl *decl,
    const PlatformAvailability *platformAvailability,
    bool enableObjCInterop) {
  // 'id' is always unavailable in Codira.
  if (enableObjCInterop && isObjCId(decl))
    return true;

  if (decl->isUnavailable())
    return true;

  for (auto *attr : decl->specific_attrs<language::Core::AvailabilityAttr>()) {
    if (attr->getPlatform()->getName() == "language")
      return true;

    if (!platformAvailability)
      continue;

    if (!platformAvailability->isPlatformRelevant(
            attr->getPlatform()->getName())) {
      continue;
    }


    toolchain::VersionTuple version = attr->getDeprecated();
    if (version.empty())
      continue;
    if (platformAvailability->treatDeprecatedAsUnavailable(
            decl, version, /*isAsync=*/false)) {
      return true;
    }
  }

  return false;
}

OptionalTypeKind importer::getParamOptionality(const language::Core::ParmVarDecl *param,
                                               bool knownNonNull) {
  // If nullability is available on the type, use it.
  language::Core::QualType paramTy = param->getType();
  if (auto nullability = paramTy->getNullability()) {
    return translateNullability(*nullability);
  }

  // If it's known non-null, use that.
  if (knownNonNull || param->hasAttr<language::Core::NonNullAttr>())
    return OTK_None;

  // Check for the 'static' annotation on C arrays.
  if (const auto *DT = dyn_cast<language::Core::DecayedType>(paramTy))
    if (const auto *AT = DT->getOriginalType()->getAsArrayTypeUnsafe())
      if (AT->getSizeModifier() == language::Core::ArraySizeModifier::Static)
        return OTK_None;

  // Default to implicitly unwrapped optionals.
  return OTK_ImplicitlyUnwrappedOptional;
}
