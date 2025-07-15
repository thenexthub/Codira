//===--- PrintClangValueType.cpp - Printer for C/C++ value types *- C++ -*-===//
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

#include "PrintClangValueType.h"
#include "ClangSyntaxPrinter.h"
#include "DeclAndTypePrinter.h"
#include "OutputLanguageMode.h"
#include "PrimitiveTypeMapping.h"
#include "CodiraToClangInteropContext.h"
#include "language/AST/Decl.h"
#include "language/AST/CodiraNameTranslation.h"
#include "language/AST/Type.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/IRGen/IRABIDetailsProvider.h"
#include "language/IRGen/Linking.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

/// Print out the C type name of a struct/enum declaration.
static void printCTypeName(raw_ostream &os, const NominalTypeDecl *type) {
  ClangSyntaxPrinter printer(type->getASTContext(), os);
  printer.printModuleNameCPrefix(*type->getParentModule());
  if (!ClangSyntaxPrinter(type->getASTContext(), os)
           .printNestedTypeNamespaceQualifiers(type, /*forC=*/true))
    os << "_";
  printer.printBaseName(type);
}

/// Print out the C++ type name of a struct/enum declaration.
static void printCxxTypeName(raw_ostream &os, const NominalTypeDecl *type,
                             const ModuleDecl *moduleContext) {
  ClangSyntaxPrinter(type->getASTContext(), os).printPrimaryCxxTypeName(type, moduleContext);
}

void ClangValueTypePrinter::printCxxImplClassName(raw_ostream &os,
                                                  const NominalTypeDecl *type) {
  os << "_impl_";
  ClangSyntaxPrinter(type->getASTContext(), os).printBaseName(type);
}

void ClangValueTypePrinter::printMetadataAccessAsVariable(
    const ASTContext &Context,
    raw_ostream &os, StringRef metadataFuncName,
    ArrayRef<GenericRequirement> genericRequirements, int indent,
    StringRef varName) {
  ClangSyntaxPrinter printer(Context, os);
  os << std::string(indent, ' ') << "auto " << varName << " = "
     << cxx_synthesis::getCxxImplNamespaceName() << "::";
  printer.printCodiraTypeMetadataAccessFunctionCall(metadataFuncName,
                                                   genericRequirements);
  os << ";\n";
}

void ClangValueTypePrinter::printValueWitnessTableAccessAsVariable(
    const ASTContext &Context,
    raw_ostream &os, StringRef metadataFuncName,
    ArrayRef<GenericRequirement> genericRequirements, int indent,
    StringRef metadataVarName, StringRef vwTableVarName) {
  ClangSyntaxPrinter printer(Context, os);
  printMetadataAccessAsVariable(Context, os, metadataFuncName, genericRequirements,
                                indent, metadataVarName);
  printer.printValueWitnessTableAccessSequenceFromTypeMetadata(
      metadataVarName, vwTableVarName, indent);
}

static void
printCValueTypeStorageStruct(raw_ostream &os, const NominalTypeDecl *typeDecl,
                             IRABIDetailsProvider::SizeAndAlignment layout) {
  os << "struct ";
  printCTypeName(os, typeDecl);
  os << " {\n";
  os << "  _Alignas(" << layout.alignment << ") ";
  os << "char _storage[" << layout.size << "];\n";
  os << "};\n\n";
}

void ClangValueTypePrinter::forwardDeclType(
    raw_ostream &os, const NominalTypeDecl *typeDecl,
    DeclAndTypePrinter &declAndTypePrinter) {
  ClangSyntaxPrinter(typeDecl->getASTContext(), os).printParentNamespaceForNestedTypes(
      typeDecl, [&](raw_ostream &) {
        if (typeDecl->isGeneric()) {
          auto genericSignature =
              typeDecl->getGenericSignature().getCanonicalSignature();
          ClangSyntaxPrinter(typeDecl->getASTContext(), os).printGenericSignature(genericSignature);
        }

        os << "class";
        declAndTypePrinter.printAvailability(os, typeDecl);
        ClangSyntaxPrinter(typeDecl->getASTContext(), os).printSymbolUSRAttribute(typeDecl);
        os << ' ';
        ClangSyntaxPrinter(typeDecl->getASTContext(), os).printBaseName(typeDecl);
        os << ";\n";
      });
  printTypePrecedingGenericTraits(os, typeDecl, typeDecl->getModuleContext());
}

static void addCppExtensionsToStdlibType(const NominalTypeDecl *typeDecl,
                                         raw_ostream &os,
                                         ClangSyntaxPrinter &printer,
                                         raw_ostream &cPrologueOS) {
  if (typeDecl == typeDecl->getASTContext().getStringDecl()) {
    // Perform String -> NSString conversion using
    // _bridgeToObjectiveCImpl.
    // FIXME: This is an extension, we should
    // just expose the method to C once extensions are
    // supported.
    // FIXME: This C passing should not be here, remove it.
    cPrologueOS << "struct language_interop_stub_Codira_String {\n"
                   "#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu\n"
                   "uint64_t _1;\n"
                   "void * _Nullable _2;\n"
                   "#elif UINTPTR_MAX == 0xFFFFFFFF\n"
                   "uint32_t _1;\n"
                   "uint32_t _2;\n"
                   "uint32_t _3;\n"
                   "#endif\n"
                   "};\n";
    cPrologueOS
        << "static LANGUAGE_INLINE_THUNK struct language_interop_stub_Codira_String "
           "language_interop_passDirect_Codira_String(const char * "
           "_Nonnull value) {\n"
           "struct language_interop_stub_Codira_String result;\n"
           "#if UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu\n"
           "memcpy(&result._1, value, 8);\n"
           "memcpy(&result._2, value + 8, 8);\n"
           "#elif UINTPTR_MAX == 0xFFFFFFFF\n"
           "memcpy(&result._1, value, 4);\n"
           "memcpy(&result._2, value + 4, 4);\n"
           "memcpy(&result._3, value + 8, 4);\n"
           "#endif\n"
           "return result;\n"
           "}\n";
    cPrologueOS << "LANGUAGE_EXTERN void *_Nonnull "
                   "$sSS10FoundationE19_bridgeToObjectiveCSo8NSStringCyF(language_interop_stub_"
                   "Codira_String) LANGUAGE_NOEXCEPT LANGUAGE_CALL;\n";
    cPrologueOS << "LANGUAGE_EXTERN language_interop_stub_Codira_String "
                   "$sSS10FoundationE36_"
                   "unconditionallyBridgeFromObjectiveCySSSo8NSStringCSgFZ("
                   "void * _Nullable) LANGUAGE_NOEXCEPT LANGUAGE_CALL;\n";
    cPrologueOS << "LANGUAGE_EXTERN language_interop_stub_Codira_String "
                   "$sSS7cStringSSSPys4Int8VG_tcfC("
                   "const char * _Nonnull) LANGUAGE_NOEXCEPT LANGUAGE_CALL;\n";
    printer.printObjCBlock([&](raw_ostream &os) {
      os << "  ";
      ClangSyntaxPrinter(typeDecl->getASTContext(), os).printInlineForThunk();
      os << "operator NSString * _Nonnull () const noexcept {\n";
      os << "    return (__bridge_transfer NSString "
            "*)(_impl::$sSS10FoundationE19_bridgeToObjectiveCSo8NSStringCyF(_impl::language_interop_"
            "passDirect_Codira_String(_getOpaquePointer())));\n";
      os << "  }\n";
      os << "static ";
      ClangSyntaxPrinter(typeDecl->getASTContext(), os).printInlineForThunk();
      os << "String init(NSString * _Nonnull nsString) noexcept {\n";
      os << "    auto result = _make();\n";
      os << "    auto res = "
            "_impl::$sSS10FoundationE36_"
            "unconditionallyBridgeFromObjectiveCySSSo8NSStringCSgFZ((__bridge "
            "void *)nsString);\n";
      os << "    memcpy(result._getOpaquePointer(), &res, sizeof(res));\n";
      os << "    return result;\n";
      os << "  }\n";
    });
    // Add additional methods for the `String` declaration.
    printer.printDefine("LANGUAGE_CXX_INTEROP_STRING_MIXIN");
    printer.printIncludeForShimHeader("_CodiraStdlibCxxOverlay.h");
  } else if (typeDecl == typeDecl->getASTContext().getOptionalDecl()) {
    // Add additional methods for the `Optional` declaration.
    printer.printDefine("LANGUAGE_CXX_INTEROP_OPTIONAL_MIXIN");
    printer.printIncludeForShimHeader("_CodiraStdlibCxxOverlay.h");
  }
}

void ClangValueTypePrinter::printValueTypeDecl(
    const NominalTypeDecl *typeDecl, toolchain::function_ref<void(void)> bodyPrinter,
    DeclAndTypePrinter &declAndTypePrinter) {
  // FIXME: Add support for generic structs.
  std::optional<IRABIDetailsProvider::SizeAndAlignment> typeSizeAlign;
  GenericSignature genericSignature;
  auto printGenericSignature = [&](raw_ostream &os) {
    if (!genericSignature)
      return;
    ClangSyntaxPrinter(Context, os).printGenericSignature(genericSignature);
  };
  auto printGenericParamRefs = [&](raw_ostream &os) {
    if (!genericSignature)
      return;
    ClangSyntaxPrinter(Context, os).printGenericSignatureParams(genericSignature);
  };
  if (typeDecl->isGeneric()) {
    genericSignature = typeDecl->getGenericSignature();
    assert(cxx_translation::isExposableToCxx(genericSignature));

    // FIXME: Can we make some better layout than opaque layout for generic
    // types.
  } else if (!typeDecl->isResilient()) {
    typeSizeAlign =
        interopContext.getIrABIDetails().getTypeSizeAlignment(typeDecl);
    // typeSizeAlign can be null if this is not a fixed-layout type,
    // e.g. it has resilient fields.
    if (typeSizeAlign && typeSizeAlign->size == 0) {
      // FIXME: How to represent 0 sized structs?
      declAndTypePrinter.getCxxDeclEmissionScope()
          .additionalUnrepresentableDeclarations.push_back(typeDecl);
      return;
    }
  }
  bool isOpaqueLayout = !typeSizeAlign.has_value();

  auto typeMetadataFunc = irgen::LinkEntity::forTypeMetadataAccessFunction(
      typeDecl->getDeclaredType()->getCanonicalType());
  std::string typeMetadataFuncName = typeMetadataFunc.mangleAsString(typeDecl->getASTContext());
  auto typeMetadataFuncGenericParams =
      interopContext.getIrABIDetails()
          .getTypeMetadataAccessFunctionGenericRequirementParameters(
              const_cast<NominalTypeDecl *>(typeDecl));

  ClangSyntaxPrinter printer(Context, os);
  printer.printParentNamespaceForNestedTypes(typeDecl, [&](raw_ostream &os) {
    // Print out a forward declaration of the "hidden" _impl class.
    printer.printNamespace(
        cxx_synthesis::getCxxImplNamespaceName(), [&](raw_ostream &os) {
          printGenericSignature(os);
          os << "class";
          declAndTypePrinter.printAvailability(os, typeDecl);
          os << ' ';
          printCxxImplClassName(os, typeDecl);
          os << ";\n\n";

          // Print out special functions, like functions that
          // access type metadata.
          printer.printCTypeMetadataTypeFunction(typeDecl, typeMetadataFuncName,
                                                 typeMetadataFuncGenericParams);
          // Print out global variables for resilient enum
          // cases
          if (isa<EnumDecl>(typeDecl) && isOpaqueLayout) {
            auto elementTagMapping =
                interopContext.getIrABIDetails().getEnumTagMapping(
                    cast<EnumDecl>(typeDecl));
            os << "// Tags for resilient enum ";
            os << typeDecl->getName().str() << '\n';
            os << "extern \"C\" {\n";
            for (const auto &pair : elementTagMapping) {
              os << "extern unsigned " << pair.second.globalVariableName
                 << ";\n";
            }
            os << "}\n";
          }
        });

    auto printEnumVWTableVariable = [&](StringRef metadataName = "metadata",
                                        StringRef vwTableName = "vwTable",
                                        StringRef enumVWTableName =
                                            "enumVWTable") {
      ClangValueTypePrinter::printValueWitnessTableAccessAsVariable(Context,
          os, typeMetadataFuncName, typeMetadataFuncGenericParams);
      os << "    const auto *" << enumVWTableName << " = reinterpret_cast<";
      ClangSyntaxPrinter(Context, os).printCodiraImplQualifier();
      os << "EnumValueWitnessTable";
      os << " *>(" << vwTableName << ");\n";
    };

    // Print out the C++ class itself.
    printGenericSignature(os);
    os << "class";
    declAndTypePrinter.printAvailability(os, typeDecl);
    ClangSyntaxPrinter(Context, os).printSymbolUSRAttribute(typeDecl);
    os << ' ';
    ClangSyntaxPrinter(Context, os).printBaseName(typeDecl);
    os << " final {\n";
    os << "public:\n";
    if (genericSignature)
      ClangSyntaxPrinter(Context, os).printGenericSignatureInnerStaticAsserts(
          genericSignature);

    // Print out the destructor.
    os << "  ";
    printer.printInlineForThunk();
    os << '~';
    printer.printBaseName(typeDecl);
    os << "() noexcept {\n";
    ClangValueTypePrinter::printValueWitnessTableAccessAsVariable(Context,
        os, typeMetadataFuncName, typeMetadataFuncGenericParams);
    os << "    vwTable->destroy(_getOpaquePointer(), metadata._0);\n";
    os << "  }\n";

    // copy constructor.
    os << "  ";
    printer.printInlineForThunk();
    printer.printBaseName(typeDecl);
    os << "(const ";
    printer.printBaseName(typeDecl);
    os << " &other) noexcept {\n";
    ClangValueTypePrinter::printValueWitnessTableAccessAsVariable(Context,
        os, typeMetadataFuncName, typeMetadataFuncGenericParams);
    if (isOpaqueLayout) {
      os << "    _storage = ";
      printer.printCodiraImplQualifier();
      os << cxx_synthesis::getCxxOpaqueStorageClassName()
         << "(vwTable->size, vwTable->getAlignment());\n";
    }
    os << "    vwTable->initializeWithCopy(_getOpaquePointer(), "
          "const_cast<char "
          "*>(other._getOpaquePointer()), metadata._0);\n";
    os << "  }\n";

    // copy assignment.
    os << "  ";
    printer.printInlineForThunk();
    printer.printBaseName(typeDecl);
    os << " &operator =(const ";
    printer.printBaseName(typeDecl);
    os << " &other) noexcept {\n";
    ClangValueTypePrinter::printValueWitnessTableAccessAsVariable(Context,
        os, typeMetadataFuncName, typeMetadataFuncGenericParams);
    os << "    vwTable->assignWithCopy(_getOpaquePointer(), const_cast<char "
          "*>(other._getOpaquePointer()), metadata._0);\n";
    os << "  return *this;\n";
    os << "  }\n";

    // FIXME: implement the move assignment.
    // FIXME: implement the move constructor.

    bodyPrinter();
    if (typeDecl->isStdlibDecl())
      addCppExtensionsToStdlibType(typeDecl, os, printer, cPrologueOS);

    os << "private:\n";

    // Print out private default constructor.
    os << "  ";
    printer.printInlineForThunk();
    printer.printBaseName(typeDecl);
    if (isOpaqueLayout) {
      os << "(";
      printer.printCodiraImplQualifier();
      os << "ValueWitnessTable * _Nonnull vwTable) noexcept : "
            "_storage(vwTable->size, "
            "vwTable->getAlignment()) {}\n";
    } else {
      os << "() noexcept {}\n";
    }
    // Print out '_make' function which returns an unitialized instance for
    // passing to Codira.
    os << "  static ";
    printer.printInlineForThunk();
    printer.printBaseName(typeDecl);
    os << " _make() noexcept {";
    if (isOpaqueLayout) {
      os << "\n";
      ClangValueTypePrinter::printValueWitnessTableAccessAsVariable(Context,
          os, typeMetadataFuncName, typeMetadataFuncGenericParams);
      os << "    return ";
      printer.printBaseName(typeDecl);
      os << "(vwTable);\n  }\n";
    } else {
      os << " return ";
      printer.printBaseName(typeDecl);
      os << "(); }\n";
    }
    // Print out the private accessors to the underlying Codira value storage.
    os << "  ";
    printer.printInlineForThunk();
    os << "const char * _Nonnull _getOpaquePointer() const noexcept { return "
          "_storage";
    if (isOpaqueLayout)
      os << ".getOpaquePointer()";
    os << "; }\n";
    os << "  ";
    printer.printInlineForThunk();
    os << "char * _Nonnull _getOpaquePointer() noexcept { return _storage";
    if (isOpaqueLayout)
      os << ".getOpaquePointer()";
    os << "; }\n";
    os << "\n";
    // Print out helper function for enums
    if (isa<EnumDecl>(typeDecl)) {
      os << "  ";
      printer.printInlineForThunk();
      os << "char * _Nonnull _destructiveProjectEnumData() noexcept {\n";
      printEnumVWTableVariable();
      os << "    enumVWTable->destructiveProjectEnumData(_getOpaquePointer(), "
            "metadata._0);\n";
      os << "    return _getOpaquePointer();\n";
      os << "  }\n";
      os << "  ";
      printer.printInlineForThunk();
      os << "void _destructiveInjectEnumTag(unsigned tag) noexcept {\n";
      printEnumVWTableVariable();
      os << "    enumVWTable->destructiveInjectEnumTag(_getOpaquePointer(), "
            "tag, "
            "metadata._0);\n";
      os << "  }\n";
      os << "  ";
      printer.printInlineForThunk();
      os << "unsigned _getEnumTag() const noexcept {\n";
      printEnumVWTableVariable();
      os << "    return enumVWTable->getEnumTag(_getOpaquePointer(), "
            "metadata._0);\n";
      os << "  }\n";
    }
    // Print out the storage for the value type.
    os << "  ";
    if (isOpaqueLayout) {
      printer.printCodiraImplQualifier();
      os << cxx_synthesis::getCxxOpaqueStorageClassName() << " _storage;\n";
    } else {
      os << "alignas(" << typeSizeAlign->alignment << ") ";
      os << "char _storage[" << typeSizeAlign->size << "];\n";
    }
    // Wrap up the value type.
    os << "  friend class " << cxx_synthesis::getCxxImplNamespaceName() << "::";
    printCxxImplClassName(os, typeDecl);
    printGenericParamRefs(os);
    os << ";\n";

    printer.printCodiraMangledNameForDebugger(typeDecl);

    os << "};\n";
    os << '\n';

    const auto *moduleContext = typeDecl->getModuleContext();
    // Print out the "hidden" _impl class.
    printer.printNamespace(
        cxx_synthesis::getCxxImplNamespaceName(), [&](raw_ostream &os) {
          printGenericSignature(os);
          os << "class";
          declAndTypePrinter.printAvailability(os, typeDecl);
          os << ' ';
          printCxxImplClassName(os, typeDecl);
          os << " {\n";
          os << "public:\n";
          if (genericSignature)
            ClangSyntaxPrinter(Context, os).printGenericSignatureInnerStaticAsserts(
                genericSignature);

          os << "  static ";
          ClangSyntaxPrinter(Context, os).printInlineForThunk();
          os << "char * _Nonnull getOpaquePointer(";
          printCxxTypeName(os, typeDecl, moduleContext);
          printGenericParamRefs(os);
          os << " &object) { return object._getOpaquePointer(); }\n";

          os << "  static ";
          ClangSyntaxPrinter(Context, os).printInlineForThunk();
          os << "const char * _Nonnull getOpaquePointer(const ";
          printCxxTypeName(os, typeDecl, moduleContext);
          printGenericParamRefs(os);
          os << " &object) { return object._getOpaquePointer(); }\n";

          os << "  template<class T>\n";
          os << "  static ";
          ClangSyntaxPrinter(Context, os).printInlineForHelperFunction();
          printCxxTypeName(os, typeDecl, moduleContext);
          printGenericParamRefs(os);
          os << " returnNewValue(T callable) {\n";
          os << "    auto result = ";
          printCxxTypeName(os, typeDecl, moduleContext);
          printGenericParamRefs(os);
          os << "::_make();\n";
          os << "    callable(result._getOpaquePointer());\n";
          os << "    return result;\n";
          os << "  }\n";
          // Print out helper function for initializeWithTake
          os << "  static ";
          ClangSyntaxPrinter(Context, os).printInlineForThunk();
          os << "void initializeWithTake(char * _Nonnull "
                "destStorage, char * _Nonnull srcStorage) {\n";
          ClangValueTypePrinter::printValueWitnessTableAccessAsVariable(
              Context, os, typeMetadataFuncName, typeMetadataFuncGenericParams);
          os << "    vwTable->initializeWithTake(destStorage, srcStorage, "
                "metadata._0);\n";
          os << "  }\n";
          os << "};\n";
        });
  });

  if (!isOpaqueLayout)
    printCValueTypeStorageStruct(cPrologueOS, typeDecl, *typeSizeAlign);

  printTypeGenericTraits(
      os, typeDecl, typeMetadataFuncName, typeMetadataFuncGenericParams,
      typeDecl->getModuleContext(), declAndTypePrinter, isOpaqueLayout);
}

void ClangValueTypePrinter::printParameterCxxToCUseScaffold(
    const ModuleDecl *moduleContext, toolchain::function_ref<void()> typePrinter,
    toolchain::function_ref<void()> cxxParamPrinter, bool forceSelf) {
  // A Codira value type is passed to its underlying Codira function
  if (forceSelf) {
    os << "_getOpaquePointer()";
  } else {
    // FIXME: can we propagate the _impl request here?
    typePrinter();
    os << "::getOpaquePointer(";
    cxxParamPrinter();
    os << ')';
  }
}

void ClangValueTypePrinter::printValueTypeReturnType(
    const NominalTypeDecl *type, OutputLanguageMode outputLang,
    TypeUseKind typeUse, const ModuleDecl *moduleContext) {
  assert(isa<StructDecl>(type) || isa<EnumDecl>(type));
  assert(outputLang == OutputLanguageMode::Cxx);
  // FIXME: make a type use.
  if (outputLang == OutputLanguageMode::Cxx) {
    if (typeUse == TypeUseKind::CxxTypeName)
      printCxxTypeName(os, type, moduleContext);
    else {
      assert(typeUse == TypeUseKind::CxxImplTypeName);
      ClangSyntaxPrinter(Context, os).printBaseName(type->getModuleContext());
      os << "::";
      if (!ClangSyntaxPrinter(Context, os).printNestedTypeNamespaceQualifiers(type))
        os << "::";
      os << cxx_synthesis::getCxxImplNamespaceName() << "::";
      printCxxImplClassName(os, type);
    }
  }
}

void ClangValueTypePrinter::printValueTypeReturnScaffold(
    const NominalTypeDecl *type, const ModuleDecl *moduleContext,
    toolchain::function_ref<void()> typePrinter,
    toolchain::function_ref<void(StringRef)> bodyPrinter) {
  assert(isa<StructDecl>(type) || isa<EnumDecl>(type));
  os << "  return ";
  typePrinter();
  os << "::returnNewValue([&](char * _Nonnull result) "
        "LANGUAGE_INLINE_THUNK_ATTRIBUTES {\n    ";
  bodyPrinter("result");
  os << ";\n";
  os << "  });\n";
}

void ClangValueTypePrinter::printClangTypeCodiraGenericTraits(
    raw_ostream &os, const TypeDecl *typeDecl, const ModuleDecl *moduleContext,
    DeclAndTypePrinter &declAndTypePrinter) {
  assert(typeDecl->hasClangNode());
  // Do not reference unspecialized templates.
  if (isa<clang::ClassTemplateDecl>(typeDecl->getClangDecl()))
    return;
  auto typeMetadataFunc = irgen::LinkEntity::forTypeMetadataAccessFunction(
      typeDecl->getDeclaredInterfaceType()->getCanonicalType());
  std::string typeMetadataFuncName = typeMetadataFunc.mangleAsString(typeDecl->getASTContext());
  printTypeGenericTraits(os, typeDecl, typeMetadataFuncName,
                         /*typeMetadataFuncRequirements=*/{}, moduleContext,
                         declAndTypePrinter);
}

void ClangValueTypePrinter::printTypePrecedingGenericTraits(
    raw_ostream &os, const NominalTypeDecl *typeDecl,
    const ModuleDecl *moduleContext) {
  assert(!typeDecl->hasClangNode());
  ClangSyntaxPrinter printer(typeDecl->getASTContext(), os);
  // FIXME: avoid popping out of the module's namespace here.
  os << "} // end namespace \n\n";
  os << "namespace language LANGUAGE_PRIVATE_ATTR {\n";

  os << "#pragma clang diagnostic push\n";
  os << "#pragma clang diagnostic ignored \"-Wc++17-extensions\"\n";

  if (printer.printNominalTypeOutsideMemberDeclTemplateSpecifiers(typeDecl))
    os << "template<>\n";
  os << "inline const constexpr bool isUsableInGenericContext<";
  printer.printNominalTypeReference(typeDecl,
                                    /*moduleContext=*/nullptr);
  os << "> = ";
  if (typeDecl->isGeneric()) {
    auto signature = typeDecl->getGenericSignature().getCanonicalSignature();
    toolchain::interleave(
        signature.getInnermostGenericParams(), os,
        [&](const GenericTypeParamType *genericParamType) {
          os << "isUsableInGenericContext<";
          printer.printGenericTypeParamTypeName(genericParamType);
          os << '>';
        },
        " && ");
  } else
    os << "true";
  os << ";\n";

  os << "#pragma clang diagnostic pop\n";
  os << "} // namespace language\n";
  os << "\n";
  printer.printModuleNamespaceStart(*moduleContext);
}

void ClangValueTypePrinter::printTypeGenericTraits(
    raw_ostream &os, const TypeDecl *typeDecl, StringRef typeMetadataFuncName,
    ArrayRef<GenericRequirement> typeMetadataFuncRequirements,
    const ModuleDecl *moduleContext, DeclAndTypePrinter &declAndTypePrinter,
    bool isOpaqueLayout) {
  auto *NTD = dyn_cast<NominalTypeDecl>(typeDecl);
  ClangSyntaxPrinter printer(typeDecl->getASTContext(), os);
  if (typeDecl->hasClangNode()) {
    /// Print a reference to the type metadata function for a C++ type.
    printer.printParentNamespaceForNestedTypes(typeDecl, [&](raw_ostream &os) {
      printer.printNamespace(
          cxx_synthesis::getCxxImplNamespaceName(), [&](raw_ostream &os) {
            ClangSyntaxPrinter(typeDecl->getASTContext(), os)
                .printCTypeMetadataTypeFunction(typeDecl, typeMetadataFuncName,
                                                typeMetadataFuncRequirements);
          });
    });
  }

  // FIXME: avoid popping out of the module's namespace here.
  os << "} // end namespace \n\n";
  os << "namespace language LANGUAGE_PRIVATE_ATTR {\n";
  auto classDecl = dyn_cast<ClassDecl>(typeDecl);
  bool addPointer =
      typeDecl->isObjC() || (classDecl && classDecl->isForeignReferenceType());

  os << "#pragma clang diagnostic push\n";
  os << "#pragma clang diagnostic ignored \"-Wc++17-extensions\"\n";
  if (typeDecl->hasClangNode()) {
    // FIXME: share the code.
    os << "template<>\n";
    os << "inline const constexpr bool isUsableInGenericContext<";
    printer.printClangTypeReference(typeDecl->getClangDecl());
    if (addPointer)
      os << "*";
    os << "> = true;\n";
  }
  if (!NTD || printer.printNominalTypeOutsideMemberDeclTemplateSpecifiers(NTD))
    os << "template<>\n";
  os << "struct";
  declAndTypePrinter.printAvailability(os, typeDecl);
  os << " TypeMetadataTrait<";
  if (typeDecl->hasClangNode()) {
    printer.printClangTypeReference(typeDecl->getClangDecl());
    if (addPointer)
      os << "*";
  } else {
    assert(NTD);
    printer.printNominalTypeReference(NTD,
                                      /*moduleContext=*/nullptr);
  }
  os << "> {\n";
  os << "  static ";
  ClangSyntaxPrinter(typeDecl->getASTContext(), os).printInlineForHelperFunction();
  os << "void * _Nonnull getTypeMetadata() {\n";
  os << "    return ";
  if (typeDecl->hasClangNode())
    printer.printBaseName(moduleContext);
  else
    printer.printBaseName(typeDecl->getModuleContext());
  os << "::";
  if (!printer.printNestedTypeNamespaceQualifiers(typeDecl))
    os << "::";
  os << cxx_synthesis::getCxxImplNamespaceName() << "::";
  ClangSyntaxPrinter(typeDecl->getASTContext(), os).printCodiraTypeMetadataAccessFunctionCall(
      typeMetadataFuncName, typeMetadataFuncRequirements);
  os << "._0;\n";
  os << "  }\n};\n";

  os << "namespace " << cxx_synthesis::getCxxImplNamespaceName() << "{\n";

  if (typeDecl->hasClangNode() && !typeDecl->isObjC()) {
    os << "template<>\n";
    os << "inline const constexpr bool isCodiraBridgedCxxRecord<";
    printer.printClangTypeReference(typeDecl->getClangDecl());
    os << "> = true;\n";
  }

  if (!isa<ClassDecl>(typeDecl) && !typeDecl->hasClangNode()) {
    assert(NTD && "not a nominal type?");
    if (printer.printNominalTypeOutsideMemberDeclTemplateSpecifiers(NTD))
      os << "template<>\n";
    os << "inline const constexpr bool isValueType<";
    printer.printBaseName(typeDecl->getModuleContext());
    os << "::";
    printer.printNominalTypeReference(NTD, moduleContext);
    os << "> = true;\n";
  }
  if (isOpaqueLayout) {
    assert(NTD && "not a nominal type?");
    assert(!isa<ClassDecl>(typeDecl) && !typeDecl->hasClangNode());
    if (printer.printNominalTypeOutsideMemberDeclTemplateSpecifiers(NTD))
      os << "template<>\n";
    os << "inline const constexpr bool isOpaqueLayout<";
    printer.printNominalTypeReference(NTD,
                                      /*moduleContext=*/nullptr);
    os << "> = true;\n";
  }

  if (!typeDecl->hasClangNode()) {
    assert(NTD);
    if (printer.printNominalTypeOutsideMemberDeclTemplateSpecifiers(NTD))
      os << "template<>\n";
    os << "struct";
    declAndTypePrinter.printAvailability(os, typeDecl);
    os << " implClassFor<";
    printer.printBaseName(typeDecl->getModuleContext());
    os << "::";
    printer.printNominalTypeReference(NTD, moduleContext);
    os << "> { using type = ";
    printer.printBaseName(typeDecl->getModuleContext());
    os << "::";
    if (!printer.printNestedTypeNamespaceQualifiers(typeDecl))
      os << "::";
    os << cxx_synthesis::getCxxImplNamespaceName() << "::";
    printCxxImplClassName(os, NTD);
    if (NTD->isGeneric())
      printer.printGenericSignatureParams(
          NTD->getGenericSignature().getCanonicalSignature());
    os << "; };\n";
  }
  os << "} // namespace\n";
  os << "#pragma clang diagnostic pop\n";
  os << "} // namespace language\n";
  os << "\n";
  printer.printModuleNamespaceStart(*moduleContext);
}
