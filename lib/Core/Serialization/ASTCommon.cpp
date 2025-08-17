//===--- ASTCommon.cpp - Common stuff for ASTReader/ASTWriter----*- C++ -*-===//
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
//  This file defines common functions that both ASTReader and ASTWriter use.
//
//===----------------------------------------------------------------------===//

#include "ASTCommon.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/DeclObjC.h"
#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Serialization/ASTDeserializationListener.h"
#include "toolchain/Support/DJB.h"

using namespace language::Core;

// Give ASTDeserializationListener's VTable a home.
ASTDeserializationListener::~ASTDeserializationListener() { }

serialization::TypeIdx
serialization::TypeIdxFromBuiltin(const BuiltinType *BT) {
  unsigned ID = 0;
  switch (BT->getKind()) {
  case BuiltinType::Void:
    ID = PREDEF_TYPE_VOID_ID;
    break;
  case BuiltinType::Bool:
    ID = PREDEF_TYPE_BOOL_ID;
    break;
  case BuiltinType::Char_U:
    ID = PREDEF_TYPE_CHAR_U_ID;
    break;
  case BuiltinType::UChar:
    ID = PREDEF_TYPE_UCHAR_ID;
    break;
  case BuiltinType::UShort:
    ID = PREDEF_TYPE_USHORT_ID;
    break;
  case BuiltinType::UInt:
    ID = PREDEF_TYPE_UINT_ID;
    break;
  case BuiltinType::ULong:
    ID = PREDEF_TYPE_ULONG_ID;
    break;
  case BuiltinType::ULongLong:
    ID = PREDEF_TYPE_ULONGLONG_ID;
    break;
  case BuiltinType::UInt128:
    ID = PREDEF_TYPE_UINT128_ID;
    break;
  case BuiltinType::Char_S:
    ID = PREDEF_TYPE_CHAR_S_ID;
    break;
  case BuiltinType::SChar:
    ID = PREDEF_TYPE_SCHAR_ID;
    break;
  case BuiltinType::WChar_S:
  case BuiltinType::WChar_U:
    ID = PREDEF_TYPE_WCHAR_ID;
    break;
  case BuiltinType::Short:
    ID = PREDEF_TYPE_SHORT_ID;
    break;
  case BuiltinType::Int:
    ID = PREDEF_TYPE_INT_ID;
    break;
  case BuiltinType::Long:
    ID = PREDEF_TYPE_LONG_ID;
    break;
  case BuiltinType::LongLong:
    ID = PREDEF_TYPE_LONGLONG_ID;
    break;
  case BuiltinType::Int128:
    ID = PREDEF_TYPE_INT128_ID;
    break;
  case BuiltinType::Half:
    ID = PREDEF_TYPE_HALF_ID;
    break;
  case BuiltinType::Float:
    ID = PREDEF_TYPE_FLOAT_ID;
    break;
  case BuiltinType::Double:
    ID = PREDEF_TYPE_DOUBLE_ID;
    break;
  case BuiltinType::LongDouble:
    ID = PREDEF_TYPE_LONGDOUBLE_ID;
    break;
  case BuiltinType::ShortAccum:
    ID = PREDEF_TYPE_SHORT_ACCUM_ID;
    break;
  case BuiltinType::Accum:
    ID = PREDEF_TYPE_ACCUM_ID;
    break;
  case BuiltinType::LongAccum:
    ID = PREDEF_TYPE_LONG_ACCUM_ID;
    break;
  case BuiltinType::UShortAccum:
    ID = PREDEF_TYPE_USHORT_ACCUM_ID;
    break;
  case BuiltinType::UAccum:
    ID = PREDEF_TYPE_UACCUM_ID;
    break;
  case BuiltinType::ULongAccum:
    ID = PREDEF_TYPE_ULONG_ACCUM_ID;
    break;
  case BuiltinType::ShortFract:
    ID = PREDEF_TYPE_SHORT_FRACT_ID;
    break;
  case BuiltinType::Fract:
    ID = PREDEF_TYPE_FRACT_ID;
    break;
  case BuiltinType::LongFract:
    ID = PREDEF_TYPE_LONG_FRACT_ID;
    break;
  case BuiltinType::UShortFract:
    ID = PREDEF_TYPE_USHORT_FRACT_ID;
    break;
  case BuiltinType::UFract:
    ID = PREDEF_TYPE_UFRACT_ID;
    break;
  case BuiltinType::ULongFract:
    ID = PREDEF_TYPE_ULONG_FRACT_ID;
    break;
  case BuiltinType::SatShortAccum:
    ID = PREDEF_TYPE_SAT_SHORT_ACCUM_ID;
    break;
  case BuiltinType::SatAccum:
    ID = PREDEF_TYPE_SAT_ACCUM_ID;
    break;
  case BuiltinType::SatLongAccum:
    ID = PREDEF_TYPE_SAT_LONG_ACCUM_ID;
    break;
  case BuiltinType::SatUShortAccum:
    ID = PREDEF_TYPE_SAT_USHORT_ACCUM_ID;
    break;
  case BuiltinType::SatUAccum:
    ID = PREDEF_TYPE_SAT_UACCUM_ID;
    break;
  case BuiltinType::SatULongAccum:
    ID = PREDEF_TYPE_SAT_ULONG_ACCUM_ID;
    break;
  case BuiltinType::SatShortFract:
    ID = PREDEF_TYPE_SAT_SHORT_FRACT_ID;
    break;
  case BuiltinType::SatFract:
    ID = PREDEF_TYPE_SAT_FRACT_ID;
    break;
  case BuiltinType::SatLongFract:
    ID = PREDEF_TYPE_SAT_LONG_FRACT_ID;
    break;
  case BuiltinType::SatUShortFract:
    ID = PREDEF_TYPE_SAT_USHORT_FRACT_ID;
    break;
  case BuiltinType::SatUFract:
    ID = PREDEF_TYPE_SAT_UFRACT_ID;
    break;
  case BuiltinType::SatULongFract:
    ID = PREDEF_TYPE_SAT_ULONG_FRACT_ID;
    break;
  case BuiltinType::Float16:
    ID = PREDEF_TYPE_FLOAT16_ID;
    break;
  case BuiltinType::Float128:
    ID = PREDEF_TYPE_FLOAT128_ID;
    break;
  case BuiltinType::Ibm128:
    ID = PREDEF_TYPE_IBM128_ID;
    break;
  case BuiltinType::NullPtr:
    ID = PREDEF_TYPE_NULLPTR_ID;
    break;
  case BuiltinType::Char8:
    ID = PREDEF_TYPE_CHAR8_ID;
    break;
  case BuiltinType::Char16:
    ID = PREDEF_TYPE_CHAR16_ID;
    break;
  case BuiltinType::Char32:
    ID = PREDEF_TYPE_CHAR32_ID;
    break;
  case BuiltinType::Overload:
    ID = PREDEF_TYPE_OVERLOAD_ID;
    break;
  case BuiltinType::UnresolvedTemplate:
    ID = PREDEF_TYPE_UNRESOLVED_TEMPLATE;
    break;
  case BuiltinType::BoundMember:
    ID = PREDEF_TYPE_BOUND_MEMBER;
    break;
  case BuiltinType::PseudoObject:
    ID = PREDEF_TYPE_PSEUDO_OBJECT;
    break;
  case BuiltinType::Dependent:
    ID = PREDEF_TYPE_DEPENDENT_ID;
    break;
  case BuiltinType::UnknownAny:
    ID = PREDEF_TYPE_UNKNOWN_ANY;
    break;
  case BuiltinType::ARCUnbridgedCast:
    ID = PREDEF_TYPE_ARC_UNBRIDGED_CAST;
    break;
  case BuiltinType::ObjCId:
    ID = PREDEF_TYPE_OBJC_ID;
    break;
  case BuiltinType::ObjCClass:
    ID = PREDEF_TYPE_OBJC_CLASS;
    break;
  case BuiltinType::ObjCSel:
    ID = PREDEF_TYPE_OBJC_SEL;
    break;
#define IMAGE_TYPE(ImgType, Id, SingletonId, Access, Suffix) \
  case BuiltinType::Id: \
    ID = PREDEF_TYPE_##Id##_ID; \
    break;
#include "language/Core/Basic/OpenCLImageTypes.def"
#define EXT_OPAQUE_TYPE(ExtType, Id, Ext) \
  case BuiltinType::Id: \
    ID = PREDEF_TYPE_##Id##_ID; \
    break;
#include "language/Core/Basic/OpenCLExtensionTypes.def"
  case BuiltinType::OCLSampler:
    ID = PREDEF_TYPE_SAMPLER_ID;
    break;
  case BuiltinType::OCLEvent:
    ID = PREDEF_TYPE_EVENT_ID;
    break;
  case BuiltinType::OCLClkEvent:
    ID = PREDEF_TYPE_CLK_EVENT_ID;
    break;
  case BuiltinType::OCLQueue:
    ID = PREDEF_TYPE_QUEUE_ID;
    break;
  case BuiltinType::OCLReserveID:
    ID = PREDEF_TYPE_RESERVE_ID_ID;
    break;
#define SVE_TYPE(Name, Id, SingletonId) \
  case BuiltinType::Id: \
    ID = PREDEF_TYPE_##Id##_ID; \
    break;
#include "language/Core/Basic/AArch64ACLETypes.def"
#define PPC_VECTOR_TYPE(Name, Id, Size) \
  case BuiltinType::Id: \
    ID = PREDEF_TYPE_##Id##_ID; \
    break;
#include "language/Core/Basic/PPCTypes.def"
#define RVV_TYPE(Name, Id, SingletonId)                                        \
  case BuiltinType::Id:                                                        \
    ID = PREDEF_TYPE_##Id##_ID;                                                \
    break;
#include "language/Core/Basic/RISCVVTypes.def"
#define WASM_TYPE(Name, Id, SingletonId)                                       \
  case BuiltinType::Id:                                                        \
    ID = PREDEF_TYPE_##Id##_ID;                                                \
    break;
#include "language/Core/Basic/WebAssemblyReferenceTypes.def"
#define AMDGPU_TYPE(Name, Id, SingletonId, Width, Align)                       \
  case BuiltinType::Id:                                                        \
    ID = PREDEF_TYPE_##Id##_ID;                                                \
    break;
#include "language/Core/Basic/AMDGPUTypes.def"
#define HLSL_INTANGIBLE_TYPE(Name, Id, SingletonId)                            \
  case BuiltinType::Id:                                                        \
    ID = PREDEF_TYPE_##Id##_ID;                                                \
    break;
#include "language/Core/Basic/HLSLIntangibleTypes.def"
  case BuiltinType::BuiltinFn:
    ID = PREDEF_TYPE_BUILTIN_FN;
    break;
  case BuiltinType::IncompleteMatrixIdx:
    ID = PREDEF_TYPE_INCOMPLETE_MATRIX_IDX;
    break;
  case BuiltinType::ArraySection:
    ID = PREDEF_TYPE_ARRAY_SECTION;
    break;
  case BuiltinType::OMPArrayShaping:
    ID = PREDEF_TYPE_OMP_ARRAY_SHAPING;
    break;
  case BuiltinType::OMPIterator:
    ID = PREDEF_TYPE_OMP_ITERATOR;
    break;
  case BuiltinType::BFloat16:
    ID = PREDEF_TYPE_BFLOAT16_ID;
    break;
  }

  return TypeIdx(0, ID);
}

unsigned serialization::ComputeHash(Selector Sel) {
  unsigned N = Sel.getNumArgs();
  if (N == 0)
    ++N;
  unsigned R = 5381;
  for (unsigned I = 0; I != N; ++I)
    if (const IdentifierInfo *II = Sel.getIdentifierInfoForSlot(I))
      R = toolchain::djbHash(II->getName(), R);
  return R;
}

const DeclContext *
serialization::getDefinitiveDeclContext(const DeclContext *DC) {
  switch (DC->getDeclKind()) {
  // These entities may have multiple definitions.
  case Decl::TranslationUnit:
  case Decl::ExternCContext:
  case Decl::Namespace:
  case Decl::LinkageSpec:
  case Decl::Export:
    return nullptr;

  // C/C++ tag types can only be defined in one place.
  case Decl::Enum:
  case Decl::Record:
    if (const TagDecl *Def = cast<TagDecl>(DC)->getDefinition())
      return Def;
    return nullptr;

  // FIXME: These can be defined in one place... except special member
  // functions and out-of-line definitions.
  case Decl::CXXRecord:
  case Decl::ClassTemplateSpecialization:
  case Decl::ClassTemplatePartialSpecialization:
    return nullptr;

  // Each function, method, and block declaration is its own DeclContext.
  case Decl::Function:
  case Decl::CXXMethod:
  case Decl::CXXConstructor:
  case Decl::CXXDestructor:
  case Decl::CXXConversion:
  case Decl::ObjCMethod:
  case Decl::Block:
  case Decl::OutlinedFunction:
  case Decl::Captured:
    // Objective C categories, category implementations, and class
    // implementations can only be defined in one place.
  case Decl::ObjCCategory:
  case Decl::ObjCCategoryImpl:
  case Decl::ObjCImplementation:
    return DC;

  case Decl::ObjCProtocol:
    if (const ObjCProtocolDecl *Def
          = cast<ObjCProtocolDecl>(DC)->getDefinition())
      return Def;
    return nullptr;

  // FIXME: These are defined in one place, but properties in class extensions
  // end up being back-patched into the main interface. See
  // SemaObjC::HandlePropertyInClassExtension for the offending code.
  case Decl::ObjCInterface:
    return nullptr;

  default:
    toolchain_unreachable("Unhandled DeclContext in AST reader");
  }

  toolchain_unreachable("Unhandled decl kind");
}

bool serialization::isRedeclarableDeclKind(unsigned Kind) {
  switch (static_cast<Decl::Kind>(Kind)) {
  case Decl::TranslationUnit:
  case Decl::ExternCContext:
    // Special case of a "merged" declaration.
    return true;

  case Decl::Namespace:
  case Decl::NamespaceAlias:
  case Decl::Typedef:
  case Decl::TypeAlias:
  case Decl::Enum:
  case Decl::Record:
  case Decl::CXXRecord:
  case Decl::ClassTemplateSpecialization:
  case Decl::ClassTemplatePartialSpecialization:
  case Decl::VarTemplateSpecialization:
  case Decl::VarTemplatePartialSpecialization:
  case Decl::Function:
  case Decl::CXXDeductionGuide:
  case Decl::CXXMethod:
  case Decl::CXXConstructor:
  case Decl::CXXDestructor:
  case Decl::CXXConversion:
  case Decl::UsingShadow:
  case Decl::ConstructorUsingShadow:
  case Decl::Var:
  case Decl::FunctionTemplate:
  case Decl::ClassTemplate:
  case Decl::VarTemplate:
  case Decl::TypeAliasTemplate:
  case Decl::ObjCProtocol:
  case Decl::ObjCInterface:
  case Decl::Empty:
    return true;

  // Never redeclarable.
  case Decl::UsingDirective:
  case Decl::Label:
  case Decl::UnresolvedUsingTypename:
  case Decl::TemplateTypeParm:
  case Decl::EnumConstant:
  case Decl::UnresolvedUsingValue:
  case Decl::IndirectField:
  case Decl::Field:
  case Decl::MSProperty:
  case Decl::MSGuid:
  case Decl::UnnamedGlobalConstant:
  case Decl::TemplateParamObject:
  case Decl::ObjCIvar:
  case Decl::ObjCAtDefsField:
  case Decl::NonTypeTemplateParm:
  case Decl::TemplateTemplateParm:
  case Decl::Using:
  case Decl::UsingEnum:
  case Decl::UsingPack:
  case Decl::ObjCMethod:
  case Decl::ObjCCategory:
  case Decl::ObjCCategoryImpl:
  case Decl::ObjCImplementation:
  case Decl::ObjCProperty:
  case Decl::ObjCCompatibleAlias:
  case Decl::LinkageSpec:
  case Decl::Export:
  case Decl::ObjCPropertyImpl:
  case Decl::PragmaComment:
  case Decl::PragmaDetectMismatch:
  case Decl::FileScopeAsm:
  case Decl::TopLevelStmt:
  case Decl::AccessSpec:
  case Decl::Friend:
  case Decl::FriendTemplate:
  case Decl::StaticAssert:
  case Decl::Block:
  case Decl::OutlinedFunction:
  case Decl::Captured:
  case Decl::Import:
  case Decl::OMPThreadPrivate:
  case Decl::OMPAllocate:
  case Decl::OMPRequires:
  case Decl::OMPCapturedExpr:
  case Decl::OMPDeclareReduction:
  case Decl::OMPDeclareMapper:
  case Decl::BuiltinTemplate:
  case Decl::Decomposition:
  case Decl::Binding:
  case Decl::Concept:
  case Decl::ImplicitConceptSpecialization:
  case Decl::LifetimeExtendedTemporary:
  case Decl::RequiresExprBody:
  case Decl::UnresolvedUsingIfExists:
  case Decl::HLSLBuffer:
  case Decl::HLSLRootSignature:
  case Decl::OpenACCDeclare:
  case Decl::OpenACCRoutine:
    return false;

  // These indirectly derive from Redeclarable<T> but are not actually
  // redeclarable.
  case Decl::ImplicitParam:
  case Decl::ParmVar:
  case Decl::ObjCTypeParam:
    return false;
  }

  toolchain_unreachable("Unhandled declaration kind");
}

bool serialization::needsAnonymousDeclarationNumber(const NamedDecl *D) {
  // Friend declarations in dependent contexts aren't anonymous in the usual
  // sense, but they cannot be found by name lookup in their semantic context
  // (or indeed in any context), so we treat them as anonymous.
  //
  // This doesn't apply to friend tag decls; Sema makes those available to name
  // lookup in the surrounding context.
  if (D->getFriendObjectKind() &&
      D->getLexicalDeclContext()->isDependentContext() && !isa<TagDecl>(D)) {
    // For function templates and class templates, the template is numbered and
    // not its pattern.
    if (auto *FD = dyn_cast<FunctionDecl>(D))
      return !FD->getDescribedFunctionTemplate();
    if (auto *RD = dyn_cast<CXXRecordDecl>(D))
      return !RD->getDescribedClassTemplate();
    return true;
  }

  // At block scope, we number everything that we need to deduplicate, since we
  // can't just use name matching to keep things lined up.
  // FIXME: This is only necessary for an inline function or a template or
  // similar.
  if (D->getLexicalDeclContext()->isFunctionOrMethod()) {
    if (auto *VD = dyn_cast<VarDecl>(D))
      return VD->isStaticLocal();
    // FIXME: What about CapturedDecls (and declarations nested within them)?
    return isa<TagDecl, BlockDecl>(D);
  }

  // Otherwise, we only care about anonymous class members / block-scope decls.
  // FIXME: We need to handle blocks within inline / templated variables too.
  if (D->getDeclName())
    return false;
  if (!isa<RecordDecl, ObjCInterfaceDecl>(D->getLexicalDeclContext()))
    return false;
  return isa<TagDecl, FieldDecl>(D);
}
