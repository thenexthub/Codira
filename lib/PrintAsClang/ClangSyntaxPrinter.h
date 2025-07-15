//===--- ClangSyntaxPrinter.h - Printer for C and C++ code ------*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_CLANGSYNTAXPRINTER_H
#define LANGUAGE_PRINTASCLANG_CLANGSYNTAXPRINTER_H

#include "language/AST/ASTContext.h"
#include "language/AST/ASTMangler.h"
#include "language/AST/Type.h"
#include "language/Basic/Toolchain.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/IRGen/GenericRequirement.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {

class CanGenericSignature;
class GenericTypeParamType;
class ModuleDecl;
class NominalTypeDecl;
class PrimitiveTypeMapping;

namespace cxx_synthesis {

/// Return the name of the namespace for things exported from Codira stdlib
StringRef getCxxCodiraNamespaceName();

/// Return the name of the implementation namespace that is used to hide
/// declarations from the namespace that corresponds to the imported Codira
/// module in C++.
StringRef getCxxImplNamespaceName();

/// Return the name of the C++ class inside of `language::_impl`
/// namespace that holds an opaque value, like a resilient struct.
StringRef getCxxOpaqueStorageClassName();

} // end namespace cxx_synthesis

class ClangSyntaxPrinter {
public:
  enum class LeadingTrivia { None, Comma };

  ClangSyntaxPrinter(const ASTContext &Ctx, raw_ostream &os) : os(os), mangler(Ctx) {}

  /// Print a given identifier. If the identifer conflicts with a keyword, add a
  /// trailing underscore.
  void printIdentifier(StringRef name) const;

  /// Print the base name of the given declaration.
  void printBaseName(const ValueDecl *decl) const;

  /// Print the C-style prefix for the given module name, that's used for
  /// C type names inside the module.
  void printModuleNameCPrefix(const ModuleDecl &mod);

  /// Print the optional namespace qualifiers for the given module reference if
  /// it's not the same as the current context.
  void
  printModuleNamespaceQualifiersIfNeeded(const ModuleDecl *referencedModule,
                                         const ModuleDecl *currentContext);

  /// Print out additional C++ `template` and `requires` clauses that
  /// are required to emit a member definition outside  a C++ class that is
  /// generated for the given Codira type declaration.
  ///
  /// \returns true if nothing was printed.
  ///
  /// Examples:
  ///    1) For Codira's `String` type, it will print nothing.
  ///    2) For Codira's `Array<T>` type, it will print `template<class
  ///    T_0_0>\nrequires language::isUsableInGenericContext<T_0_0>\n`
  bool printNominalTypeOutsideMemberDeclTemplateSpecifiers(
      const NominalTypeDecl *typeDecl);

  /// Print out additional C++ `static_assert` clauses that
  /// are required to emit a generic member definition outside a C++ class that
  /// is generated for the given Codira type declaration.
  ///
  /// \returns true if nothing was printed.
  ///
  /// Examples:
  ///    1) For Codira's `String` type, it will print nothing.
  ///    2) For Codira's `Array<T>` type, it will print
  ///    `static_assert(language::isUsableInGenericContext<T_0_0>);\n`
  bool printNominalTypeOutsideMemberDeclInnerStaticAssert(
      const NominalTypeDecl *typeDecl);

  // Returns true when no qualifiers were printed.
  bool printNestedTypeNamespaceQualifiers(const ValueDecl *D,
                                          bool forC = false) const;

  /// Print out the C++ class access qualifier for the given Codira  type
  /// declaration.
  ///
  /// Examples:
  ///    1) For Codira's `String` type, it will print `String
  ///    2) For Codira's `Array<T>` type, it will print `Array<T_0_0>
  ///    3) For Codira's `Array<T>.Index` type, it will print
  ///    `Array<T_0_0>::Index` 4) For Codira's `String` type in another module,
  ///    it will print `Codira::String`
  void printNominalTypeReference(const NominalTypeDecl *typeDecl,
                                 const ModuleDecl *moduleContext);

  /// Print out the C++ record qualifier for the given C++ record.
  void printClangTypeReference(const clang::Decl *typeDecl);

  /// Print out the C++ class access qualifier for the given Codira  type
  /// declaration.
  ///
  /// Examples:
  ///    1) For Codira's `String` type, it will print `String::`.
  ///    2) For Codira's `Array<T>` type, it will print `Array<T_0_0>::`
  ///    3) For Codira's `Array<T>.Index` type, it will print
  ///    `Array<T_0_0>::Index::` 4) For Codira's `String` type in another module,
  ///    it will print `Codira::String::`
  void printNominalTypeQualifier(const NominalTypeDecl *typeDecl,
                                 const ModuleDecl *moduleContext);

  enum class NamespaceTrivia { None, AttributeCodiraPrivate };

  void printModuleNamespaceStart(const ModuleDecl &moduleContext) const;

  /// Print a C++ namespace declaration with the give name and body.
  void printNamespace(toolchain::function_ref<void(raw_ostream &OS)> namePrinter,
                      toolchain::function_ref<void(raw_ostream &OS)> bodyPrinter,
                      NamespaceTrivia trivia = NamespaceTrivia::None,
                      const ModuleDecl *moduleContext = nullptr) const;

  void printNamespace(StringRef name,
                      toolchain::function_ref<void(raw_ostream &OS)> bodyPrinter,
                      NamespaceTrivia trivia = NamespaceTrivia::None) const;

  /// Prints the C++ namespaces of the outer types for a nested type.
  /// E.g., for struct Foo { struct Bar {...} } it will print
  /// namespace __FooNested { ..body.. } // namespace __FooNested
  void printParentNamespaceForNestedTypes(
      const ValueDecl *D, toolchain::function_ref<void(raw_ostream &OS)> bodyPrinter,
      NamespaceTrivia trivia = NamespaceTrivia::None) const;

  /// Print an extern C block with given body.
  void
  printExternC(toolchain::function_ref<void(raw_ostream &OS)> bodyPrinter) const;

  /// Print an #ifdef __OBJC__ block.
  void
  printObjCBlock(toolchain::function_ref<void(raw_ostream &OS)> bodyPrinter) const;

  /// Print the `language::_impl::` namespace qualifier.
  void printCodiraImplQualifier() const;

  /// Where nullability information should be printed.
  enum class NullabilityPrintKind {
    Before,
    After,
    ContextSensitive,
  };

  void printInlineForThunk() const;
  void printInlineForHelperFunction() const;

  void printNullability(
      std::optional<OptionalTypeKind> kind,
      NullabilityPrintKind printKind = NullabilityPrintKind::After) const;

  /// Returns true if \p name matches a keyword in any Clang language mode.
  static bool isClangKeyword(StringRef name);
  static bool isClangKeyword(Identifier name);

  /// Print the call expression to the Codira type metadata access function.
  void printCodiraTypeMetadataAccessFunctionCall(
      StringRef name, ArrayRef<GenericRequirement> requirements);

  /// Print the set of statements to access the value witness table pointer
  /// ('vwTable') from the given type metadata variable.
  void printValueWitnessTableAccessSequenceFromTypeMetadata(
      StringRef metadataVariable, StringRef vwTableVariable, int indent);

  /// Print the metadata accessor function for the given type declaration.
  void printCTypeMetadataTypeFunction(
      const TypeDecl *typeDecl, StringRef typeMetadataFuncName,
      toolchain::ArrayRef<GenericRequirement> genericRequirements);

  /// Print the name of the generic type param type in C++.
  void printGenericTypeParamTypeName(const GenericTypeParamType *gtpt);

  /// Print the Codira generic signature as C++ template declaration alongside
  /// its requirements.
  void printGenericSignature(GenericSignature signature);

  /// Print the `static_assert` statements used for legacy type-checking for
  /// generics in C++14/C++17 mode.
  void
  printGenericSignatureInnerStaticAsserts(GenericSignature signature);

  /// Print the C++ template parameters that should be passed for a given
  /// generic signature.
  void printGenericSignatureParams(GenericSignature signature);

  /// Print the call to the C++ type traits that computes the underlying type /
  /// witness table pointer value that are passed to Codira for the given generic
  /// requirement.
  void
  printGenericRequirementInstantiantion(const GenericRequirement &requirement);

  /// Print the list of calls to C++ type traits that compute the generic
  /// pointer values to pass to Codira.
  void printGenericRequirementsInstantiantions(
      ArrayRef<GenericRequirement> requirements,
      LeadingTrivia leadingTrivia = LeadingTrivia::None);

  // Print the C++ type name that corresponds to the primary user facing C++
  // class for the given nominal type.
  void printPrimaryCxxTypeName(const NominalTypeDecl *type,
                               const ModuleDecl *moduleContext);

  // Print the #include sequence for the specified C++ interop shim header.
  void printIncludeForShimHeader(StringRef headerName);

  // Print the #define for the given macro.
  void printDefine(StringRef macroName);

  // Print the ignored Clang diagnostic preprocessor directives around the given
  // source.
  void printIgnoredDiagnosticBlock(StringRef diagName,
                                   toolchain::function_ref<void()> bodyPrinter);

  void printIgnoredCxx17ExtensionDiagnosticBlock(
      toolchain::function_ref<void()> bodyPrinter);

  /// Print the macro that applies Clang's `external_source_symbol` attribute
  /// on the generated declaration.
  void printSymbolUSRAttribute(const ValueDecl *D) const;

  /// Print the given **known** type as a C type.
  void printKnownCType(Type t, PrimitiveTypeMapping &typeMapping) const;

  /// Print the nominal type's Codira mangled name as a typedef from a char to
  /// the mangled name, and a static constexpr variable declaration, whose type
  /// is the aforementioned typedef, and whose name is known to the debugger.
  void printCodiraMangledNameForDebugger(const NominalTypeDecl *typeDecl);

protected:
  raw_ostream &os;
  language::Mangle::ASTMangler mangler;
};

} // end namespace language

#endif
