//===--- PrintClangFunction.h - Printer for C/C++ functions -----*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_PRINTCLANGFUNCTION_H
#define LANGUAGE_PRINTASCLANG_PRINTCLANGFUNCTION_H

#include "OutputLanguageMode.h"
#include "language/AST/Type.h"
#include "language/Basic/Toolchain.h"
#include "language/ClangImporter/ClangImporter.h"
#include "language/IRGen/GenericRequirement.h"
#include "language/IRGen/IRABIDetailsProvider.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>

namespace language {

class AbstractFunctionDecl;
class AccessorDecl;
class AnyFunctionType;
class FuncDecl;
class GenericTypeParamType;
class ModuleDecl;
class NominalTypeDecl;
class LoweredFunctionSignature;
class ParamDecl;
class ParameterList;
class PrimitiveTypeMapping;
class CodiraToClangInteropContext;
class DeclAndTypePrinter;

struct ClangRepresentation {
  enum Kind { representable, objcxxonly, unsupported };

  ClangRepresentation(Kind kind) : kind(kind) {}

  /// Returns true if the given Codira node is unsupported in Clang in any
  /// language mode.
  bool isUnsupported() const { return kind == unsupported; }

  /// Returns true if the given Codira node is only supported in
  /// Objective C++ mode.
  bool isObjCxxOnly() const { return kind == objcxxonly; }

  const ClangRepresentation &merge(ClangRepresentation other) {
    if (other.kind == unsupported)
      kind = unsupported;
    else if (kind == representable)
      kind = other.kind;
    return *this;
  }

private:
  Kind kind;
};

/// Responsible for printing a Codira function decl or type in C or C++ mode, to
/// be included in a Codira module's generated clang header.
class DeclAndTypeClangFunctionPrinter {
public:
  DeclAndTypeClangFunctionPrinter(raw_ostream &os, raw_ostream &cPrologueOS,
                                  PrimitiveTypeMapping &typeMapping,
                                  CodiraToClangInteropContext &interopContext,
                                  DeclAndTypePrinter &declPrinter)
      : os(os), cPrologueOS(cPrologueOS), typeMapping(typeMapping),
        interopContext(interopContext), declPrinter(declPrinter) {}

  /// What kind of function signature should be emitted for the given Codira
  /// function.
  enum class FunctionSignatureKind {
    /// Emit a signature for the C function prototype.
    CFunctionProto,
    /// Emit a signature for the inline C++ function thunk.
    CxxInlineThunk
  };

  /// Optional modifiers that can be applied to function signature.
  struct FunctionSignatureModifiers {
    /// Additional qualifier to add before the function's name.
    const NominalTypeDecl *qualifierContext = nullptr;
    bool isStatic = false;
    bool isInline = false;
    bool isConst = false;
    bool isNoexcept = false;
    bool hasSymbolUSR = true;
    /// Specific declaration that should be used to emit the symbol's
    /// USR instead of the original function declaration.
    const ValueDecl *symbolUSROverride = nullptr;

    FunctionSignatureModifiers() {}
  };

  /// Print the C function declaration or the C++ function thunk that
  /// corresponds to the given function declaration.
  ///
  /// \return value describing in which Clang language mode the function is
  /// supported, if any.
  ClangRepresentation printFunctionSignature(
      const AbstractFunctionDecl *FD, const LoweredFunctionSignature &signature,
      StringRef name, Type resultTy, FunctionSignatureKind kind,
      FunctionSignatureModifiers modifiers = {});

  /// Print the body of the inline C++ function thunk that calls the underlying
  /// Codira function.
  void printCxxThunkBody(
      const AbstractFunctionDecl *FD, const LoweredFunctionSignature &signature,
      StringRef languageSymbolName, const NominalTypeDecl *typeDeclContext,
      const ModuleDecl *moduleContext, Type resultTy,
      const ParameterList *params, bool hasThrows = false,
      const AnyFunctionType *funcType = nullptr, bool isStaticMethod = false,
      std::optional<IRABIDetailsProvider::MethodDispatchInfo> dispatchInfo =
          std::nullopt);

  /// Print the Codira method as C++ method declaration/definition, including
  /// constructors.
  void printCxxMethod(
      DeclAndTypePrinter &declAndTypePrinter,
      const NominalTypeDecl *typeDeclContext, const AbstractFunctionDecl *FD,
      const LoweredFunctionSignature &signature, StringRef languageSymbolName,
      Type resultTy, bool isStatic, bool isDefinition,
      std::optional<IRABIDetailsProvider::MethodDispatchInfo> dispatchInfo);

  /// Print the C++ getter/setter method signature.
  void printCxxPropertyAccessorMethod(
      DeclAndTypePrinter &declAndTypePrinter,
      const NominalTypeDecl *typeDeclContext, const AccessorDecl *accessor,
      const LoweredFunctionSignature &signature, StringRef languageSymbolName,
      Type resultTy, bool isStatic, bool isDefinition,
      std::optional<IRABIDetailsProvider::MethodDispatchInfo> dispatchInfo);

  /// Print the C++ subscript method.
  void printCxxSubscriptAccessorMethod(
      DeclAndTypePrinter &declAndTypePrinter,
      const NominalTypeDecl *typeDeclContext, const AccessorDecl *accessor,
      const LoweredFunctionSignature &signature, StringRef languageSymbolName,
      Type resultTy, bool isDefinition,
      std::optional<IRABIDetailsProvider::MethodDispatchInfo> dispatchInfo);

  /// Print Codira type as C/C++ type, as the return type of a C/C++ function.
  ClangRepresentation printClangFunctionReturnType(
      raw_ostream &stream, Type ty, OptionalTypeKind optKind,
      ModuleDecl *moduleContext, OutputLanguageMode outputLang);

  static void printGenericReturnSequence(
      raw_ostream &os, const GenericTypeParamType *gtpt,
      toolchain::function_ref<void(StringRef)> invocationPrinter,
      std::optional<StringRef> initializeWithTakeFromValue = std::nullopt);

  using PrinterTy =
      toolchain::function_ref<void(toolchain::MapVector<Type, std::string> &)>;

  /// Print generated C++ helper function
  void printCustomCxxFunction(const SmallVector<Type> &neededTypes,
                              bool NeedsReturnTypes,
                              PrinterTy retTypeAndNamePrinter,
                              PrinterTy paramPrinter, bool isConstFunc,
                              PrinterTy bodyPrinter, ValueDecl *valueDecl,
                              ModuleDecl *emittedModule,
                              raw_ostream &outOfLineOS);

  static ClangRepresentation
  getTypeRepresentation(PrimitiveTypeMapping &typeMapping,
                        CodiraToClangInteropContext &interopContext,
                        DeclAndTypePrinter &declPrinter,
                        const ModuleDecl *emittedModule, Type ty);

  /// Prints the name of the type including generic arguments.
  void printTypeName(Type ty, const ModuleDecl *moduleContext);

private:
  void printCxxToCFunctionParameterUse(Type type, StringRef name,
                                       const ModuleDecl *moduleContext,
                                       bool isInOut, bool isIndirect,
                                       std::string directTypeEncoding,
                                       bool isSelf);

  // Print out the full type specifier that refers to the
  // _impl::_impl_<typename> C++ class for the given Codira type.
  void printTypeImplTypeSpecifier(Type type, const ModuleDecl *moduleContext);

  bool hasKnownOptionalNullableCxxMapping(Type type);

  raw_ostream &os;
  raw_ostream &cPrologueOS;
  PrimitiveTypeMapping &typeMapping;
  CodiraToClangInteropContext &interopContext;
  DeclAndTypePrinter &declPrinter;
};

} // end namespace language

#endif
