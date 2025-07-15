//===--- PrintCodiraToClangCoreScaffold.cpp - Print core decls ---*- C++ -*-===//
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

#include "PrintCodiraToClangCoreScaffold.h"
#include "ClangSyntaxPrinter.h"
#include "PrimitiveTypeMapping.h"
#include "CodiraToClangInteropContext.h"
#include "language/ABI/MetadataValues.h"
#include "language/AST/Decl.h"
#include "language/AST/Type.h"
#include "language/Basic/Assertions.h"
#include "language/IRGen/IRABIDetailsProvider.h"
#include "language/IRGen/Linking.h"
#include "clang/Basic/AddressSpaces.h"
#include "clang/Basic/TargetInfo.h"
#include "toolchain/ADT/STLExtras.h"

using namespace language;

static void printKnownStruct(const ASTContext &astContext,
    PrimitiveTypeMapping &typeMapping, raw_ostream &os, StringRef name,
    const IRABIDetailsProvider::TypeRecordABIRepresentation &typeRecord) {
  assert(typeRecord.getMembers().size() > 1);
  os << "struct " << name << " {\n";
  for (const auto &ty : toolchain::enumerate(typeRecord.getMembers())) {
    os << "  ";
    ClangSyntaxPrinter(astContext, os).printKnownCType(ty.value(), typeMapping);
    os << " _" << ty.index() << ";\n";
  }
  os << "};\n";
}

static void printKnownTypedef(const ASTContext &astContext,
    PrimitiveTypeMapping &typeMapping, raw_ostream &os, StringRef name,
    const IRABIDetailsProvider::TypeRecordABIRepresentation &typeRecord) {
  assert(typeRecord.getMembers().size() == 1);
  os << "typedef ";
  ClangSyntaxPrinter(astContext, os).printKnownCType(typeRecord.getMembers()[0],
                                         typeMapping);
  os << " " << name << ";\n";
}

static void printKnownType(const ASTContext &astContext,
    PrimitiveTypeMapping &typeMapping, raw_ostream &os, StringRef name,
    const IRABIDetailsProvider::TypeRecordABIRepresentation &typeRecord) {
  if (typeRecord.getMembers().size() == 1)
    return printKnownTypedef(astContext, typeMapping, os, name, typeRecord);
  printKnownStruct(astContext, typeMapping, os, name, typeRecord);
}

static void
printValueWitnessTableFunctionType(raw_ostream &os, StringRef prefix,
                                   StringRef name, StringRef returnType,
                                   std::string paramTypes,
                                   uint16_t ptrauthDisc) {
  os << "using " << prefix << name << "Ty = " << returnType
     << "(* __ptrauth_language_value_witness_function_pointer(" << ptrauthDisc
     << "))(" << paramTypes << ") LANGUAGE_NOEXCEPT_FUNCTION_PTR;\n";
}

static std::string makeParams(const char *arg) { return arg; }

template <class... T>
static std::string makeParams(const char *arg, const T... args) {
  return std::string(arg) + ", " + makeParams(args...);
}

static void printValueWitnessTable(raw_ostream &os) {
  std::string members;
  toolchain::raw_string_ostream membersOS(members);

  // C++ only supports noexcept on `using` function types in C++17.
  os << "#if __cplusplus > 201402L\n";
  os << "#  define LANGUAGE_NOEXCEPT_FUNCTION_PTR noexcept\n";
  os << "#else\n";
  os << "#  define LANGUAGE_NOEXCEPT_FUNCTION_PTR\n";
  os << "#endif\n\n";

#define WANT_ONLY_REQUIRED_VALUE_WITNESSES
#define DATA_VALUE_WITNESS(lowerId, upperId, type)                             \
  membersOS << "  " << type << " " << #lowerId << ";\n";
#define FUNCTION_VALUE_WITNESS(lowerId, upperId, returnType, paramTypes)       \
  printValueWitnessTableFunctionType(                                          \
      os, "ValueWitness", #upperId, returnType, makeParams paramTypes,         \
      SpecialPointerAuthDiscriminators::upperId);                              \
  membersOS << "  ValueWitness" << #upperId << "Ty _Nonnull " << #lowerId      \
            << ";\n";
#define MUTABLE_VALUE_TYPE "void * _Nonnull"
#define IMMUTABLE_VALUE_TYPE "const void * _Nonnull"
#define MUTABLE_BUFFER_TYPE "void * _Nonnull"
#define IMMUTABLE_BUFFER_TYPE "const void * _Nonnull"
#define TYPE_TYPE "void * _Nonnull"
#define SIZE_TYPE "size_t"
#define INT_TYPE "int"
#define UINT_TYPE "unsigned"
#define VOID_TYPE "void"
#include "language/ABI/ValueWitness.def"

  membersOS << "\n  constexpr size_t getAlignment() const { return (flags & "
            << TargetValueWitnessFlags<uint64_t>::AlignmentMask << ") + 1; }\n";
  os << "\nstruct ValueWitnessTable {\n" << membersOS.str() << "};\n\n";
  membersOS.str().clear();

#define WANT_ONLY_ENUM_VALUE_WITNESSES
#define DATA_VALUE_WITNESS(lowerId, upperId, type)                             \
  membersOS << "  " << type << " " << #lowerId << ";\n";
#define FUNCTION_VALUE_WITNESS(lowerId, upperId, returnType, paramTypes)       \
  printValueWitnessTableFunctionType(                                          \
      os, "EnumValueWitness", #upperId, returnType, makeParams paramTypes,     \
      SpecialPointerAuthDiscriminators::upperId);                              \
  membersOS << "  EnumValueWitness" << #upperId << "Ty _Nonnull " << #lowerId  \
            << ";\n";
#define MUTABLE_VALUE_TYPE "void * _Nonnull"
#define IMMUTABLE_VALUE_TYPE "const void * _Nonnull"
#define MUTABLE_BUFFER_TYPE "void * _Nonnull"
#define IMMUTABLE_BUFFER_TYPE "const void * _Nonnull"
#define TYPE_TYPE "void * _Nonnull"
#define SIZE_TYPE "size_t"
#define INT_TYPE "int"
#define UINT_TYPE "unsigned"
#define VOID_TYPE "void"
#include "language/ABI/ValueWitness.def"

  os << "\nstruct EnumValueWitnessTable {\n"
     << "  ValueWitnessTable vwTable;\n"
     << membersOS.str() << "};\n\n";

  os << "#undef LANGUAGE_NOEXCEPT_FUNCTION_PTR\n\n";
}

static void printTypeMetadataResponseType(CodiraToClangInteropContext &ctx,
                                          PrimitiveTypeMapping &typeMapping,
                                          raw_ostream &os) {
  os << "// Codira type metadata response type.\n";
  // Print out the type metadata structure.
  auto funcSig = ctx.getIrABIDetails().getTypeMetadataAccessFunctionSignature();
  printKnownType(ctx.getASTContext(), typeMapping, os, "MetadataResponseTy", funcSig.returnType);
  assert(funcSig.parameterTypes.size() == 1);
  os << "// Codira type metadata request type.\n";
  printKnownType(ctx.getASTContext(), typeMapping, os, "MetadataRequestTy",
                 funcSig.parameterTypes[0]);
}

void printPrimitiveGenericTypeTraits(raw_ostream &os, ASTContext &astContext,
                                     PrimitiveTypeMapping &typeMapping,
                                     bool isCForwardDefinition) {
  Type supportedPrimitiveTypes[] = {
      astContext.getBoolType(),

      // Primitive integer, C integer and Int/UInt mappings.
      astContext.getInt8Type(), astContext.getUInt8Type(),
      astContext.getInt16Type(), astContext.getUInt16Type(),
      astContext.getInt32Type(), astContext.getUInt32Type(),
      astContext.getInt64Type(), astContext.getUInt64Type(),

      // Primitive floating point type mappings.
      astContext.getFloatType(), astContext.getDoubleType(),

      // Pointer types.
      // FIXME: support raw pointers?
      astContext.getOpaquePointerType(),

      astContext.getIntType(), astContext.getUIntType()};

  auto primTypesArray = toolchain::ArrayRef(supportedPrimitiveTypes);

  // Ensure that `long` and `unsigned long` are treated as valid
  // generic Codira types (`Int` and `UInt`) on platforms
  // that do define `Int`/`ptrdiff_t` as `long` and don't define `int64_t` to be
  // `long`.
  auto &clangTI =
      astContext.getClangModuleLoader()->getClangASTContext().getTargetInfo();
  bool isCodiraIntLong =
      clangTI.getPtrDiffType(clang::LangAS::Default) == clang::TransferrableTargetInfo::SignedLong;
  bool isInt64Long =
      clangTI.getInt64Type() == clang::TransferrableTargetInfo::SignedLong;
  if (!(isCodiraIntLong && !isInt64Long))
    primTypesArray = primTypesArray.drop_back(2);

  // We do not have metadata for primitive types in Embedded Codira.
  // As a result, the following features are not supported with primitive types in this mode:
  // - Dynamic casts
  // - Static self
  // - Generic requirement parameters
  // - Metadata source parameter
  bool embedded = astContext.LangOpts.hasFeature(Feature::Embedded);

  for (Type type : primTypesArray) {
    auto typeInfo = *typeMapping.getKnownCxxTypeInfo(
        type->getNominalOrBoundGenericNominal());

    if (!isCForwardDefinition) {
      os << "template<>\n";
      os << "inline const constexpr bool isUsableInGenericContext<"
         << typeInfo.name << "> = true;\n\n";
    }

    if (embedded)
      continue;

    auto typeMetadataFunc = irgen::LinkEntity::forTypeMetadata(
        type->getCanonicalType(), irgen::TypeMetadataAddress::AddressPoint);
    std::string typeMetadataVarName = typeMetadataFunc.mangleAsString(type->getASTContext());

    if (isCForwardDefinition) {
      os << "// type metadata address for " << type.getString() << ".\n";
      os << "LANGUAGE_IMPORT_STDLIB_SYMBOL extern size_t " << typeMetadataVarName
         << ";\n";
      continue;
    }

    os << "template<>\nstruct TypeMetadataTrait<" << typeInfo.name << "> {\n"
       << "  static ";
    ClangSyntaxPrinter(astContext, os).printInlineForThunk();
    os << "void * _Nonnull getTypeMetadata() {\n"
       << "    return &" << cxx_synthesis::getCxxImplNamespaceName()
       << "::" << typeMetadataVarName << ";\n"
       << "  }\n};\n\n";
  }
}

void language::printCodiraToClangCoreScaffold(CodiraToClangInteropContext &ctx,
                                          ASTContext &astContext,
                                          PrimitiveTypeMapping &typeMapping,
                                          raw_ostream &os) {
  ClangSyntaxPrinter printer(astContext, os);
  printer.printNamespace(
      cxx_synthesis::getCxxCodiraNamespaceName(),
      [&](raw_ostream &) {
        printer.printNamespace(
            cxx_synthesis::getCxxImplNamespaceName(), [&](raw_ostream &) {
              printer.printExternC([&](raw_ostream &os) {
                printTypeMetadataResponseType(ctx, typeMapping, os);
                os << "\n";
                printValueWitnessTable(os);
                os << "\n";
                printPrimitiveGenericTypeTraits(os, astContext, typeMapping,
                                                /*isCForwardDefinition=*/true);
              });
              os << "\n";
            });
        os << "\n";
        // C++ only supports inline variables from C++17.
        ClangSyntaxPrinter(astContext, os).printIgnoredCxx17ExtensionDiagnosticBlock([&]() {
          printPrimitiveGenericTypeTraits(os, astContext, typeMapping,
                                          /*isCForwardDefinition=*/false);
        });
      },
      ClangSyntaxPrinter::NamespaceTrivia::AttributeCodiraPrivate);
}
