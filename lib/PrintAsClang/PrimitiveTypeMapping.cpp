//===--- PrimitiveTypeMapping.cpp - Mapping primitive types -----*- C++ -*-===//
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

#include "PrimitiveTypeMapping.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/AST/Module.h"
#include "language/Basic/Assertions.h"
#include "language/ClangImporter/ClangImporter.h"

using namespace language;

void PrimitiveTypeMapping::initialize(ASTContext &ctx) {
  assert(mappedTypeNames.empty() && "expected empty type map");
#define MAP(LANGUAGE_NAME, CLANG_REPR, NEEDS_NULLABILITY)                         \
  mappedTypeNames[{ctx.StdlibModuleName, ctx.getIdentifier(#LANGUAGE_NAME)}] = {  \
      CLANG_REPR, std::optional<StringRef>(CLANG_REPR),                        \
      std::optional<StringRef>(CLANG_REPR), NEEDS_NULLABILITY}
#define MAP_C(LANGUAGE_NAME, OBJC_REPR, C_REPR, NEEDS_NULLABILITY)                \
  mappedTypeNames[{ctx.StdlibModuleName, ctx.getIdentifier(#LANGUAGE_NAME)}] = {  \
      OBJC_REPR, std::optional<StringRef>(C_REPR),                             \
      std::optional<StringRef>(C_REPR), NEEDS_NULLABILITY}
#define MAP_CXX(LANGUAGE_NAME, OBJC_REPR, C_REPR, CXX_REPR, NEEDS_NULLABILITY)    \
  mappedTypeNames[{ctx.StdlibModuleName, ctx.getIdentifier(#LANGUAGE_NAME)}] = {  \
      OBJC_REPR, std::optional<StringRef>(C_REPR),                             \
      std::optional<StringRef>(CXX_REPR), NEEDS_NULLABILITY}

  MAP(CBool, "bool", false);

  MAP(CChar, "char", false);
  MAP(CWideChar, "wchar_t", false);
  MAP(CChar8, "char8_t", false);
  MAP(CChar16, "char16_t", false);
  MAP(CChar32, "char32_t", false);

  MAP(CSignedChar, "signed char", false);
  MAP(CShort, "short", false);
  MAP(CInt, "int", false);
  MAP(CLong, "long", false);
  MAP(CLongLong, "long long", false);

  MAP(CUnsignedChar, "unsigned char", false);
  MAP(CUnsignedShort, "unsigned short", false);
  MAP(CUnsignedInt, "unsigned int", false);
  MAP(CUnsignedLong, "unsigned long", false);
  MAP(CUnsignedLongLong, "unsigned long long", false);

  MAP(CFloat, "float", false);
  MAP(CDouble, "double", false);

  MAP(Int8, "int8_t", false);
  MAP(Int16, "int16_t", false);
  MAP(Int32, "int32_t", false);
  MAP(Int64, "int64_t", false);
  MAP(UInt8, "uint8_t", false);
  MAP(UInt16, "uint16_t", false);
  MAP(UInt32, "uint32_t", false);
  MAP(UInt64, "uint64_t", false);

  MAP(Float, "float", false);
  MAP(Double, "double", false);
  MAP(Float32, "float", false);
  MAP(Float64, "double", false);

  MAP_CXX(Int, "NSInteger", "ptrdiff_t", "language::Int", false);
  MAP_CXX(UInt, "NSUInteger", "size_t", "language::UInt", false);
  MAP_C(Bool, "BOOL", "bool", false);

  MAP(OpaquePointer, "void *", true);
  MAP(UnsafeRawPointer, "void const *", true);
  MAP(UnsafeMutableRawPointer, "void *", true);

  Identifier ID_ObjectiveC = ctx.Id_ObjectiveC;
  mappedTypeNames[{ID_ObjectiveC, ctx.getIdentifier("ObjCBool")}] = {
      "BOOL", std::nullopt, std::nullopt, false};
  mappedTypeNames[{ID_ObjectiveC, ctx.getIdentifier("Selector")}] = {
      "SEL", std::nullopt, std::nullopt, true};
  mappedTypeNames[{ID_ObjectiveC, ctx.getIdentifier(language::getCodiraName(
                                      KnownFoundationEntity::NSZone))}] = {
      "struct _NSZone *", std::nullopt, std::nullopt, true};

  mappedTypeNames[{ctx.Id_Darwin, ctx.getIdentifier("DarwinBoolean")}] = {
      "Boolean", std::nullopt, std::nullopt, false};

  mappedTypeNames[{ctx.Id_CoreGraphics, ctx.Id_CGFloat}] = {
      "CGFloat", std::nullopt, std::nullopt, false};

  mappedTypeNames[{ctx.Id_CoreFoundation, ctx.Id_CGFloat}] = {
      "CGFloat", std::nullopt, std::nullopt, false};

  // Use typedefs we set up for SIMD vector types.
#define MAP_SIMD_TYPE(BASENAME, _, __)                                         \
  StringRef simd2##BASENAME = "language_" #BASENAME "2";                          \
  mappedTypeNames[{ctx.Id_simd, ctx.getIdentifier(#BASENAME "2")}] = {         \
      simd2##BASENAME, simd2##BASENAME, simd2##BASENAME, false};               \
  StringRef simd3##BASENAME = "language_" #BASENAME "3";                          \
  mappedTypeNames[{ctx.Id_simd, ctx.getIdentifier(#BASENAME "3")}] = {         \
      simd3##BASENAME, simd3##BASENAME, simd3##BASENAME, false};               \
  StringRef simd4##BASENAME = "language_" #BASENAME "4";                          \
  mappedTypeNames[{ctx.Id_simd, ctx.getIdentifier(#BASENAME "4")}] = {         \
      simd4##BASENAME, simd4##BASENAME, simd4##BASENAME, false};
#include "language/ClangImporter/SIMDMappedTypes.def"
  static_assert(LANGUAGE_MAX_IMPORTED_SIMD_ELEMENTS == 4,
                "must add or remove special name mappings if max number of "
                "SIMD elements is changed");
}

PrimitiveTypeMapping::FullClangTypeInfo *
PrimitiveTypeMapping::getMappedTypeInfoOrNull(const TypeDecl *typeDecl) {
  if (mappedTypeNames.empty())
    initialize(typeDecl->getASTContext());

  Identifier moduleName = typeDecl->getModuleContext()->getName();
  Identifier name = typeDecl->getName();
  auto iter = mappedTypeNames.find({moduleName, name});
  if (iter == mappedTypeNames.end())
    return nullptr;
  return &iter->second;
}

std::optional<PrimitiveTypeMapping::ClangTypeInfo>
PrimitiveTypeMapping::getKnownObjCTypeInfo(const TypeDecl *typeDecl) {
  if (auto *typeInfo = getMappedTypeInfoOrNull(typeDecl))
    return ClangTypeInfo{typeInfo->objcName, typeInfo->canBeNullable};
  return std::nullopt;
}

std::optional<PrimitiveTypeMapping::ClangTypeInfo>
PrimitiveTypeMapping::getKnownCTypeInfo(const TypeDecl *typeDecl) {
  if (auto *typeInfo = getMappedTypeInfoOrNull(typeDecl)) {
    if (typeInfo->cName)
      return ClangTypeInfo{*typeInfo->cName, typeInfo->canBeNullable};
  }
  return std::nullopt;
}

std::optional<PrimitiveTypeMapping::ClangTypeInfo>
PrimitiveTypeMapping::getKnownCxxTypeInfo(const TypeDecl *typeDecl) {
  if (auto *typeInfo = getMappedTypeInfoOrNull(typeDecl)) {
    if (typeInfo->cxxName)
      return ClangTypeInfo{*typeInfo->cxxName, typeInfo->canBeNullable};
  }
  return std::nullopt;
}
