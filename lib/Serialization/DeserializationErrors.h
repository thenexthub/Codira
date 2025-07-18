//===--- DeserializationErrors.h - Problems in deserialization --*- C++ -*-===//
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

#ifndef LANGUAGE_SERIALIZATION_DESERIALIZATIONERRORS_H
#define LANGUAGE_SERIALIZATION_DESERIALIZATIONERRORS_H

#include "ModuleFile.h"
#include "ModuleFileSharedCore.h"
#include "ModuleFormat.h"
#include "language/AST/Identifier.h"
#include "language/AST/Module.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/PrettyStackTrace.h"

namespace language {
namespace serialization {

[[nodiscard]]
static inline std::unique_ptr<toolchain::ErrorInfoBase>
takeErrorInfo(toolchain::Error error) {
  std::unique_ptr<toolchain::ErrorInfoBase> result;
  toolchain::handleAllErrors(std::move(error),
                        [&](std::unique_ptr<toolchain::ErrorInfoBase> info) {
    result = std::move(info);
  });
  return result;
}

/// This error is generated by ModuleFile::diagnoseFatal(). All
/// FatalDeserializationError has already been added to the DiagnosticsEngine
/// upon creation.
struct FatalDeserializationError
    : public toolchain::ErrorInfo<FatalDeserializationError> {
  static char ID;
  std::string Message;
  FatalDeserializationError(std::string Message) : Message(Message) {}

  void log(toolchain::raw_ostream &OS) const override {
    OS << "Deserialization Error: " << Message;
  }
  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class XRefTracePath {
  class PathPiece {
  public:
    enum class Kind {
      Value,
      Type,
      Operator,
      OperatorFilter,
      Accessor,
      Extension,
      GenericParam,
      PrivateDiscriminator,
      OpaqueReturnType,
      Unknown
    };

  private:
    Kind kind;
    void *data;

    template <typename T>
    T getDataAs() const {
      return toolchain::PointerLikeTypeTraits<T>::getFromVoidPointer(data);
    }

  public:
    template <typename T>
    PathPiece(Kind K, T value)
      : kind(K),
        data(toolchain::PointerLikeTypeTraits<T>::getAsVoidPointer(value)) {}

    DeclBaseName getAsBaseName() const {
      switch (kind) {
      case Kind::Value:
      case Kind::Operator:
      case Kind::PrivateDiscriminator:
      case Kind::OpaqueReturnType:
        return getDataAs<DeclBaseName>();
      case Kind::Type:
      case Kind::OperatorFilter:
      case Kind::Accessor:
      case Kind::Extension:
      case Kind::GenericParam:
      case Kind::Unknown:
        return Identifier();
      }
      toolchain_unreachable("unhandled kind");
    }

    void print(raw_ostream &os) const {
      switch (kind) {
      case Kind::Value:
        os << getDataAs<DeclBaseName>();
        break;
      case Kind::Type:
        os << "with type " << getDataAs<Type>();
        break;
      case Kind::Extension:
        if (getDataAs<ModuleDecl *>()) {
          os << "in an extension in module '"
             << getDataAs<ModuleDecl *>()->getName()
             << "'";
        } else {
          os << "in an extension in any module";
        }
        break;
      case Kind::Operator:
        os << "operator " << getDataAs<Identifier>();
        break;
      case Kind::OperatorFilter:
        switch (getDataAs<uintptr_t>()) {
        case Infix:
          os << "(infix)";
          break;
        case Prefix:
          os << "(prefix)";
          break;
        case Postfix:
          os << "(postfix)";
          break;
        default:
          os << "(unknown operator filter)";
          break;
        }
        break;
      case Kind::Accessor:
        switch (getDataAs<uintptr_t>()) {
        case Get:
          os << "(getter)";
          break;
        case Set:
          os << "(setter)";
          break;
        case Address:
          os << "(addressor)";
          break;
        case MutableAddress:
          os << "(mutableAddressor)";
          break;
        case WillSet:
          os << "(willSet)";
          break;
        case DidSet:
          os << "(didSet)";
          break;
        case Read:
          os << "(read)";
          break;
        case Modify:
          os << "(modify)";
          break;
        default:
          os << "(unknown accessor kind)";
          break;
        }
        break;
      case Kind::GenericParam:
        os << "generic param #" << getDataAs<uintptr_t>();
        break;
      case Kind::PrivateDiscriminator:
        os << "(in " << getDataAs<Identifier>() << ")";
        break;
      case Kind::OpaqueReturnType:
        os << "opaque return type of " << getDataAs<DeclBaseName>();
        break;
      case Kind::Unknown:
        os << "unknown xref kind " << getDataAs<uintptr_t>();
        break;
      }
    }
  };

  ModuleDecl &baseM;
  SmallVector<PathPiece, 8> path;

public:
  explicit XRefTracePath(ModuleDecl &M) : baseM(M) {}

  void addValue(DeclBaseName name) {
    path.push_back({ PathPiece::Kind::Value, name });
  }

  void addType(Type ty) {
    path.push_back({ PathPiece::Kind::Type, ty });
  }

  void addOperator(Identifier name) {
    path.push_back({ PathPiece::Kind::Operator, name });
  }

  void addOperatorFilter(uint8_t fixity) {
    path.push_back({ PathPiece::Kind::OperatorFilter,
                     static_cast<uintptr_t>(fixity) });
  }

  void addAccessor(uint8_t kind) {
    path.push_back({ PathPiece::Kind::Accessor,
                     static_cast<uintptr_t>(kind) });
  }

  void addExtension(ModuleDecl *M) {
    path.push_back({ PathPiece::Kind::Extension, M });
  }

  void addGenericParam(uintptr_t index) {
    path.push_back({ PathPiece::Kind::GenericParam, index });
  }

  void addPrivateDiscriminator(Identifier name) {
    path.push_back({ PathPiece::Kind::PrivateDiscriminator, name });
  }

  void addUnknown(uintptr_t kind) {
    path.push_back({ PathPiece::Kind::Unknown, kind });
  }
  
  void addOpaqueReturnType(Identifier name) {
    path.push_back({ PathPiece::Kind::OpaqueReturnType, name });
  }

  DeclBaseName getLastName() const {
    for (auto &piece : toolchain::reverse(path)) {
      DeclBaseName result = piece.getAsBaseName();
      if (!result.empty())
        return result;
    }
    return DeclBaseName();
  }

  void removeLast() {
    path.pop_back();
  }

  void print(raw_ostream &os, StringRef leading = "") const {
    os << "Cross-reference to module '" << baseM.getName() << "'\n";
    for (auto &piece : path) {
      os << leading << "... ";
      piece.print(os);
      os << "\n";
    }
  }
};

class DeclDeserializationError : public toolchain::ErrorInfoBase {
  static const char ID;
  void anchor() override;

public:
  enum Flag : unsigned {
    DesignatedInitializer = 1 << 0,
    NeedsFieldOffsetVectorEntry = 1 << 1,
  };
  using Flags = OptionSet<Flag>;

protected:
  DeclName name;
  Flags flags;
  uint8_t numTableEntries = 0;

public:
  DeclName getName() const {
    return name;
  }

  bool isDesignatedInitializer() const {
    return flags.contains(Flag::DesignatedInitializer);
  }
  unsigned getNumberOfTableEntries() const {
    return numTableEntries;
  }
  bool needsFieldOffsetVectorEntry() const {
    return flags.contains(Flag::NeedsFieldOffsetVectorEntry);
  }

  bool isA(const void *const ClassID) const override {
    return ClassID == classID() || ErrorInfoBase::isA(ClassID);
  }

  static const void *classID() { return &ID; }
};

class XRefError : public toolchain::ErrorInfo<XRefError, DeclDeserializationError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

  XRefTracePath path;
  const char *message;
public:
  template <size_t N>
  XRefError(const char (&message)[N], XRefTracePath path, DeclName name)
      : path(path), message(message) {
    this->name = name;
  }

  void log(raw_ostream &OS) const override {
    OS << message << " (" << name << ")\n";
    path.print(OS);
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class XRefNonLoadedModuleError :
    public toolchain::ErrorInfo<XRefNonLoadedModuleError, DeclDeserializationError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  explicit XRefNonLoadedModuleError(Identifier name) {
    this->name = name;
  }

  void log(raw_ostream &OS) const override {
    OS << "module '" << name << "' was not loaded";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

/// Project issue affecting modules. Usually a change in the context between
/// the time a module was built and when it was imported.
class ModularizationError : public toolchain::ErrorInfo<ModularizationError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  enum class Kind {
    DeclMoved,
    DeclKindChanged,
    DeclNotFound
  };

private:
  DeclName name;
  bool declIsType;
  Kind errorKind;

  /// Module targeted by the reference that should define the decl.
  const ModuleDecl *expectedModule;

  /// Binary module file with the outgoing reference.
  const ModuleFile *referenceModule;

  /// Module where the compiler did find the decl, if any.
  const ModuleDecl *foundModule;

  XRefTracePath path;

  /// Expected vs found type if the mismatch caused a decl to be rejected.
  std::optional<std::pair<Type, Type>> mismatchingTypes;

public:
  explicit ModularizationError(DeclName name, bool declIsType, Kind errorKind,
                               const ModuleDecl *expectedModule,
                               const ModuleFile *referenceModule,
                               const ModuleDecl *foundModule,
                               XRefTracePath path,
                               std::optional<std::pair<Type, Type>> mismatchingTypes):
    name(name), declIsType(declIsType), errorKind(errorKind),
    expectedModule(expectedModule),
    referenceModule(referenceModule),
    foundModule(foundModule), path(path),
    mismatchingTypes(mismatchingTypes) {}

  void diagnose(const ModuleFile *MF,
                DiagnosticBehavior limit = DiagnosticBehavior::Fatal) const;

  void log(raw_ostream &OS) const override {
    OS << "modularization issue on '" << name << "', reference from '";
    OS << referenceModule->getName() << "' not resolvable: ";
    switch (errorKind) {
      case Kind::DeclMoved:
        OS << "expected in '" << expectedModule->getName() << "' but found in '";
        OS << foundModule->getName() << "'";
        break;
      case Kind::DeclKindChanged:
        OS << "decl details changed between what was imported from '";
        OS << expectedModule->getName() << "' and what is now imported from '";
        OS << foundModule->getName() << "'";
        break;
      case Kind::DeclNotFound:
        OS << "not found, expected in '" << expectedModule->getName() << "'";
        break;
    }
    OS << "\n";
    path.print(OS);
  }

  SourceLoc getSourceLoc() const;

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }

  bool isA(const void *const ClassID) const override {
    return ClassID == classID() || ErrorInfoBase::isA(ClassID);
  }

  static const void *classID() { return &ID; }
};

class OverrideError : public toolchain::ErrorInfo<OverrideError,
                                             DeclDeserializationError> {
private:
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  explicit OverrideError(DeclName name,
                         Flags flags={}, unsigned numTableEntries=0) {
    this->name = name;
    this->flags = flags;
    this->numTableEntries = numTableEntries;
  }

  void log(raw_ostream &OS) const override {
    OS << "could not find '" << name << "' in parent class";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

// Service class for errors with an underlying cause.
class ErrorWithUnderlyingReason {
  std::unique_ptr<toolchain::ErrorInfoBase> underlyingReason;

public:
  explicit ErrorWithUnderlyingReason (std::unique_ptr<toolchain::ErrorInfoBase> reason) :
    underlyingReason(std::move(reason)) {}

  template <typename UnderlyingErrorT>
  bool underlyingReasonIsA() const {
    if (!underlyingReason)
      return false;
    return underlyingReason->isA<UnderlyingErrorT>();
  }

  void log(raw_ostream &OS) const {
    if (underlyingReason) {
      OS << "\nCaused by: ";
      underlyingReason->log(OS);
    }
  }

  // Returns \c true if the error was diagnosed.
  template <typename HandlerT>
  bool diagnoseUnderlyingReason(HandlerT &&handler) {
    if (underlyingReason &&
        toolchain::ErrorHandlerTraits<HandlerT>::appliesTo(*underlyingReason)) {
      auto error = toolchain::ErrorHandlerTraits<HandlerT>::apply(
                                               std::forward<HandlerT>(handler),
                                               std::move(underlyingReason));
      if (!error) {
        // The underlying reason was diagnosed.
        return true;
      } else {
        underlyingReason = takeErrorInfo(std::move(error));
        return false;
      }
    }
    return false;
  }
};

class TypeError : public toolchain::ErrorInfo<TypeError, DeclDeserializationError>,
                  public ErrorWithUnderlyingReason {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  explicit TypeError(DeclName name, std::unique_ptr<ErrorInfoBase> reason,
                     Flags flags={}, unsigned numTableEntries=0)
      : ErrorWithUnderlyingReason(std::move(reason)) {
    this->name = name;
    this->flags = flags;
    this->numTableEntries = numTableEntries;
  }

  void diagnose(const ModuleFile *MF) const;

  void log(raw_ostream &OS) const override {
    OS << "Could not deserialize type for '" << name << "'";
    ErrorWithUnderlyingReason::log(OS);
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class ExtensionError : public toolchain::ErrorInfo<ExtensionError>,
                       public ErrorWithUnderlyingReason {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  explicit ExtensionError(std::unique_ptr<ErrorInfoBase> reason)
      : ErrorWithUnderlyingReason(std::move(reason)) {}

  void diagnose(const ModuleFile *MF) const;

  void log(raw_ostream &OS) const override {
    OS << "could not deserialize extension";
    ErrorWithUnderlyingReason::log(OS);
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class SILEntityError : public toolchain::ErrorInfo<SILEntityError>,
                       public ErrorWithUnderlyingReason {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

  StringRef name;
public:
  SILEntityError(StringRef name, std::unique_ptr<ErrorInfoBase> reason)
      : ErrorWithUnderlyingReason(std::move(reason)), name(name) {}

  void log(raw_ostream &OS) const override {
    OS << "could not deserialize SIL entity '" << name << "'";
    ErrorWithUnderlyingReason::log(OS);
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class SILFunctionTypeMismatch : public toolchain::ErrorInfo<SILFunctionTypeMismatch> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

  StringRef name;
  std::string descLHS, descRHS;
public:
  SILFunctionTypeMismatch(StringRef name, std::string descLHS,
                          std::string descRHS)
      : name(name), descLHS(descLHS), descRHS(descRHS) {}

  void log(raw_ostream &OS) const override {
    OS << "SILFunction type mismatch for '" << name << "': '";
    OS << descLHS << "' != '" <<  descRHS << "'\n";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

// Decl was not deserialized because its attributes did not match the filter.
//
// \sa getDeclChecked
class DeclAttributesDidNotMatch : public toolchain::ErrorInfo<DeclAttributesDidNotMatch> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  DeclAttributesDidNotMatch() {}

  void log(raw_ostream &OS) const override {
    OS << "Decl attributes did not match filter";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class InvalidRecordKindError :
    public toolchain::ErrorInfo<InvalidRecordKindError, DeclDeserializationError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

  unsigned recordKind;
  const char *extraText;

public:
  explicit InvalidRecordKindError(unsigned kind,
                                  const char *extraText = nullptr) {
    this->recordKind = kind;
    this->extraText = extraText;
  }

  void log(raw_ostream &OS) const override {
    OS << "don't know how to deserialize record with code " << recordKind;
    if (recordKind >= decls_block::SILGenName_DECL_ATTR)
      OS << " (attribute kind "
         << recordKind - decls_block::SILGenName_DECL_ATTR << ")";
    if (extraText)
      OS << ": " << extraText;
    OS << "; this may be a compiler bug, or a file on disk may have changed "
          "during compilation";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

// Decl was not deserialized because it's an internal detail maked as unsafe
// at serialization.
class UnsafeDeserializationError : public toolchain::ErrorInfo<UnsafeDeserializationError, DeclDeserializationError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  UnsafeDeserializationError(Identifier name) {
    this->name = name;
  }

  void log(raw_ostream &OS) const override {
    OS << "Decl '" << name << "' is unsafe to deserialize";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class InvalidEnumValueError
    : public toolchain::ErrorInfo<InvalidEnumValueError, DeclDeserializationError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

  unsigned enumValue;
  const char *enumDescription;

public:
  explicit InvalidEnumValueError(unsigned enumValue,
                                 const char *enumDescription) {
    this->enumValue = enumValue;
    this->enumDescription = enumDescription;
  }

  void log(raw_ostream &OS) const override {
    OS << "invalid value " << enumValue << " for enumeration "
       << enumDescription;
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

// Cross-reference to a conformance was not found where it was expected.
class ConformanceXRefError : public toolchain::ErrorInfo<ConformanceXRefError,
                                                    DeclDeserializationError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

  DeclName protoName;
  const ModuleDecl *expectedModule;

public:
  ConformanceXRefError(Identifier name, Identifier protoName,
                       const ModuleDecl *expectedModule):
                     protoName(protoName), expectedModule(expectedModule) {
    this->name = name;
  }

  void diagnose(const ModuleFile *MF,
                DiagnosticBehavior limit = DiagnosticBehavior::Fatal) const;

  void log(raw_ostream &OS) const override {
    OS << "Conformance of '" << name << "' to '" << protoName
       << "' not found in module '" << expectedModule->getName() << "'";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class InavalidAvailabilityDomainError
    : public toolchain::ErrorInfo<InavalidAvailabilityDomainError> {
  friend ErrorInfo;
  static const char ID;
  void anchor() override;

public:
  InavalidAvailabilityDomainError() {}

  void log(raw_ostream &OS) const override {
    OS << "Invalid availability domain";
  }

  std::error_code convertToErrorCode() const override {
    return toolchain::inconvertibleErrorCode();
  }
};

class PrettyStackTraceModuleFile : public toolchain::PrettyStackTraceEntry {
  const char *Action;
  const ModuleFile &MF;
public:
  explicit PrettyStackTraceModuleFile(const char *action, ModuleFile &module)
      : Action(action), MF(module) {}
  explicit PrettyStackTraceModuleFile(ModuleFile &module)
      : PrettyStackTraceModuleFile("While reading from", module) {}

  void print(raw_ostream &os) const override {
    os << Action << " ";
    MF.outputDiagnosticInfo(os);
    os << "\n";
  }
};

class PrettyStackTraceModuleFileCore : public toolchain::PrettyStackTraceEntry {
  const ModuleFileSharedCore &MF;
public:
  explicit PrettyStackTraceModuleFileCore(ModuleFileSharedCore &module)
      : MF(module) {}

  void print(raw_ostream &os) const override {
    os << "While reading from ";
    MF.outputDiagnosticInfo(os);
    os << "\n";
  }
};

} // end namespace serialization
} // end namespace language

#endif
