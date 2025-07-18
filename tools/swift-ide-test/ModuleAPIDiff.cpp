//===--- ModuleAPIDiff.cpp - Codira Module API I/O -------------------------===//
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

#include "ModuleAPIDiff.h"
#include "language/AST/ASTVisitor.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/GenericSignature.h"
#include "language/Basic/SourceManager.h"
#include "language/Driver/FrontendUtil.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/PrintingDiagnosticConsumer.h"
#include "language/Sema/IDETypeChecking.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/Support/YAMLTraits.h"
#include <memory>
#include <optional>

using namespace language;

/*

Machine-Readable Representation of API and ABI of a Codira Module (SMA)
======================================================================

SMA stands for Codira Module API/ABI.

This format is designed to accurately represent API (and, in future, ABI) of a
Codira module.  Design constraints are as follows:

- the format should allow comparing API and ABI of two different modules
  produced by two different compilers;

- the comparison procedure should be as simple as possible (ideally, applying
  diff(1) to the files should produce reasonable results; next preference is a
  simple, generic, tree diffing algorithm that is not actually aware of the
  data model);

- a comparison that is aware of the data model should be able to automatically
  judge if two modules are ABI- or ABI-compatible;

- the format should be machine-readable;

- the format should be human-readable as long as it does not hurt other goals;

- the format should be able to sustain significant changes (within reason) to
  any aspects of the Codira language, including syntax changes, type system
  changes, AST representation changes within the compiler etc.

- the immediate goal is to capture the module API, but we would like to be able
  to extend this format to carry ABI information in future.

If you find this format useful for any other purpose (for example, generating
documentation), it is completely incidental; we recommend not to rely on this
facility for other purposes.

module ::=
    Name: <identifier>
    ExportedModules: [ <submodule-name>* ]  (default: none)
    <nested-decls>

nested-decls ::=
    NestedDecls:                                (all unordered, default: none)
        Structs: [ <struct-decl>* ]
        Enums: [ <enum-decl>* ]
        Classes: [ <class-decl>* ]
        Protocols: [ <protocol-decl>* ]
        Typealiases: [ <typealias-decl>* ]
        AssociatedTypes: [ <associated-type-decl>* ]
        Vars: [ <var-decl>* ]
        Lets: [ <let-decl>* ]
        Functions: [ <fn-decl>* ]
        Initializers: [ <init-decl>* ]
        Deinitializers: [ <deinit-decl>* ]

generic-signature ::=
    GenericSignature:
        GenericParams:                  (ordered)
            - Name: <identifier>
        ConformanceRequirements:
            - Type: <type-name>
              ProtocolOrClass: <type-name>
        SameTypeRequirements:
            - FirstType: <type-name>
              SecondType: <type-name>

struct-decl ::=
    Name: <identifier>
    <generic-signature>?
    ConformsToProtocols: [ <type-name>* ]       (default: none)
    <decl-attributes>?
    <nested-decls>

enum-case-decl ::=
    Name: <identifier>
    ArgumentType: <type-name>                   (default: none)
    RawValue: <string>                          (default: none)
    <decl-attributes>?

enum-decl ::=
    Name: <identifier>
    <generic-signature>?
    RawType: <type-name>?                       (default: none)
    ConformsToProtocols: [ <type-name>* ]       (default: none)
    <decl-attributes>?
    Cases: [ <enum-case-decl>* ]                (default: none)
    <nested-decls>

class-decl ::=
    Name: <identifier>
    <generic-signature>?
    Superclass: <type-name>?                    (default: none)
    ConformsToProtocols: [ <type-name>* ]       (default: none)
    <decl-attributes>?
    <nested-decls>

protocol-decl ::=
    Name: <identifier>
    ConformsToProtocols: [ <type-name>* ]       (default: none)
    <decl-attributes>?
    <nested-decls>

typealias-decl ::=
    Name: <identifier>
    Type: <type-name>
    <decl-attributes>?

associated-type-decl ::=
    Name: <identifier>
    Superclass: <type-name>?                    (default: none)
    DefaultDefinition: <type-name>?             (default: none)
    <decl-attributes>?

var-decl ::=
    Name: <identifier>
    Type: <type-name>
    IsSettable: <bool>
    IsStored: <bool>
    <decl-attributes>?

let-decl ::=
    Name: <identifier>
    Type: <type-name>
    <decl-attributes>?

fn-param ::=
    Name: <identifier>
    Type: <type-name>
    IsInout: <bool>             (default: false)
    <decl-attributes>?

fn-decl ::=
    IsStatic: <bool>            (default: false)
    Name: <function-name>
    <generic-signature>?
    Params:
        [ [ <fn-param>* ]* ]
    ResultType: <type-name>
    <decl-attributes>?

init-decl ::=
    InitializerKind: (Designated|Convenience)
    InitializerFailability: (None|Optional|ImplicitlyUnwrappedOptional) (default: None)
    <generic-signature>?
    Params:
        [ <fn-param>* ]
    IsTrappingStub: <bool>      (default: false)
    <decl-attributes>?

deinit-decl ::=
    Name: deinit (always 'deinit'; introduced so that the decl is non-empty)
    <decl-attributes>?

identifier ::= <string>

// A sequence of dot-separated identifiers.
submodule-name ::= <string>

type-name ::= <string>

decl-attributes ::=
    Attributes:                         (default: false)
        IsClassProtocol: <bool>
        IsDynamic: <bool>
        IsFinal: <bool>
        IsLazy: <bool>
        IsMutating: <bool>
        IsObjC: <bool>
        IsOptional: <bool>
        IsRequired: <bool>
        IsRequiresStoredPropertyInits: <bool>
        IsTransparent: <bool>
        IsWeak: <bool>

*/

// SMA data model is defined in 'language::sma' namespace.
//
// Never 'use namespace language::sma'.
//
// It is fine to shadow names from the 'language' namespace in 'language::sma'.
//
// Don't use any AST types in 'language::sma'.  Only use simple types like
// 'std::string', 'std::vector', 'std::map' etc.

/// Define a type 'language::sma::TYPE_NAME' that is a "strong typedef" for
/// 'std::string'.
#define DEFINE_SMA_STRING_STRONG_TYPEDEF(TYPE_NAME, STRING_MEMBER_NAME)        \
  namespace language {                                                            \
  namespace sma {                                                              \
  struct TYPE_NAME {                                                           \
    std::string STRING_MEMBER_NAME;                                            \
  };                                                                           \
  } /* namespace sma */                                                        \
  } /* namespace language */                                                      \
                                                                               \
  namespace toolchain {                                                             \
  namespace yaml {                                                             \
  template <> struct ScalarTraits<::language::sma::TYPE_NAME> {                   \
    static void output(const ::language::sma::TYPE_NAME &Val, void *Context,      \
                       toolchain::raw_ostream &Out) {                               \
      ScalarTraits<std::string>::output(Val.STRING_MEMBER_NAME, Context, Out); \
    }                                                                          \
    static StringRef input(StringRef Scalar, void *Context,                    \
                           ::language::sma::TYPE_NAME &Val) {                     \
      return ScalarTraits<std::string>::input(Scalar, Context,                 \
                                              Val.STRING_MEMBER_NAME);         \
    }                                                                          \
    static QuotingType mustQuote(StringRef S) {                                \
      return ScalarTraits<std::string>::mustQuote(S);                          \
    }                                                                          \
  };                                                                           \
  } /* namespace yaml */                                                       \
  } /* namespace toolchain */

DEFINE_SMA_STRING_STRONG_TYPEDEF(Identifier, Name)
DEFINE_SMA_STRING_STRONG_TYPEDEF(FunctionName, Name)
DEFINE_SMA_STRING_STRONG_TYPEDEF(TypeName, Name)
DEFINE_SMA_STRING_STRONG_TYPEDEF(SubmoduleName, Name)

#undef DEFINE_SMA_STRING_STRONG_TYPEDEF

namespace language {
namespace sma {

using std::optional;

#define SMA_FOR_EVERY_DECL_ATTRIBUTE(MACRO) \
  MACRO(IsDynamic) \
  MACRO(IsFinal) \
  MACRO(IsLazy) \
  MACRO(IsMutating) \
  MACRO(IsObjC) \
  MACRO(IsOptional) \
  MACRO(IsRequired) \
  MACRO(IsRequiresStoredPropertyInits) \
  MACRO(IsTransparent) \
  MACRO(IsWeak)

struct DeclAttributes {
#define DEFINE_MEMBER(NAME) bool NAME = false;
  SMA_FOR_EVERY_DECL_ATTRIBUTE(DEFINE_MEMBER)
#undef DEFINE_MEMBER
};

bool operator==(const DeclAttributes &LHS, const DeclAttributes &RHS) {
  return
#define PROCESS_MEMBER(NAME) LHS.NAME == RHS.NAME &&
      SMA_FOR_EVERY_DECL_ATTRIBUTE(PROCESS_MEMBER)
#undef PROCESS_MEMBER
      // Add an identity element for && over bools.
      true;
}

struct StructDecl;
struct EnumDecl;
struct ClassDecl;
struct ProtocolDecl;
struct TypealiasDecl;
struct AssociatedTypeDecl;
struct VarDecl;
struct LetDecl;
struct FuncDecl;
struct InitDecl;
struct DeinitDecl;

/// A container for declarations contained in some declaration context.
struct NestedDecls {
  std::vector<std::shared_ptr<StructDecl>> Structs;
  std::vector<std::shared_ptr<EnumDecl>> Enums;
  std::vector<std::shared_ptr<ClassDecl>> Classes;
  std::vector<std::shared_ptr<ProtocolDecl>> Protocols;
  std::vector<std::shared_ptr<TypealiasDecl>> Typealiases;
  std::vector<std::shared_ptr<AssociatedTypeDecl>> AssociatedTypes;
  std::vector<std::shared_ptr<VarDecl>> Vars;
  std::vector<std::shared_ptr<LetDecl>> Lets;
  std::vector<std::shared_ptr<FuncDecl>> Functions;
  std::vector<std::shared_ptr<InitDecl>> Initializers;
  std::vector<std::shared_ptr<DeinitDecl>> Deinitializers;

  bool isEmpty() const {
    return Structs.empty() && Enums.empty() && Classes.empty() &&
           Protocols.empty() && Typealiases.empty() &&
           AssociatedTypes.empty() && Vars.empty() && Lets.empty() &&
           Functions.empty() && Initializers.empty() && Deinitializers.empty();
  }
};

bool operator==(const NestedDecls &LHS, const NestedDecls &RHS) {
  // Only empty instances compare equal.
  return LHS.isEmpty() && RHS.isEmpty();
}

struct GenericParam {
  Identifier Name;
};

struct ConformanceRequirement {
  TypeName Type;
  TypeName Protocol;
};

struct SameTypeRequirement {
  TypeName FirstType;
  TypeName SecondType;
};

struct GenericSignature {
  std::vector<GenericParam> GenericParams;
  std::vector<ConformanceRequirement> ConformanceRequirements;
  std::vector<SameTypeRequirement> SameTypeRequirements;
};

struct Module {
  Identifier Name;
  std::vector<SubmoduleName> ExportedModules;
  NestedDecls Decls;
};

struct StructDecl {
  Identifier Name;
  std::optional<GenericSignature> TheGenericSignature;
  std::vector<TypeName> ConformsToProtocols;
  DeclAttributes Attributes;
  NestedDecls Decls;
};

struct EnumCaseDecl {
  Identifier Name;
  TypeName ArgumentType;
  std::string RawValue;
  DeclAttributes Attributes;
};

struct EnumDecl {
  Identifier Name;
  std::optional<GenericSignature> TheGenericSignature;
  std::optional<TypeName> RawType;
  std::vector<TypeName> ConformsToProtocols;
  DeclAttributes Attributes;
  std::vector<std::shared_ptr<EnumCaseDecl>> Cases;
  NestedDecls Decls;
};

struct ClassDecl {
  Identifier Name;
  std::optional<GenericSignature> TheGenericSignature;
  std::optional<TypeName> Superclass;
  std::vector<TypeName> ConformsToProtocols;
  DeclAttributes Attributes;
  NestedDecls Decls;
};

struct ProtocolDecl {
  Identifier Name;
  std::vector<TypeName> ConformsToProtocols;
  DeclAttributes Attributes;
  NestedDecls Decls;
};

struct TypealiasDecl {
  Identifier Name;
  TypeName Type;
  DeclAttributes Attributes;
};

struct AssociatedTypeDecl {
  Identifier Name;
  std::optional<TypeName> DefaultDefinition;
  DeclAttributes Attributes;
};

struct VarDecl {
  Identifier Name;
  TypeName Type;
  bool IsSettable = false;
  bool IsStored = false;
  DeclAttributes Attributes;
};

struct LetDecl {
  Identifier Name;
  TypeName Type;
  DeclAttributes Attributes;
};

struct FuncParam {
  Identifier Name;
  TypeName Type;
  bool IsInout = false;
  DeclAttributes Attributes;
};

struct FuncDecl {
  FunctionName Name;
  bool IsStatic = false;
  std::optional<GenericSignature> TheGenericSignature;
  std::vector<std::vector<FuncParam>> Params;
  TypeName ResultType;
  DeclAttributes Attributes;
};

enum class InitializerKind {
  Designated,
  Convenience,
};

enum class InitializerFailability {
  None, // default
  Optional,
  ImplicitlyUnwrappedOptional,
};

struct InitDecl {
  InitializerKind TheInitializerKind;
  InitializerFailability TheInitializerFailability;
  std::optional<GenericSignature> TheGenericSignature;
  std::vector<FuncParam> Params;
  bool IsTrappingStub = false;
  DeclAttributes Attributes;
};

struct DeinitDecl {
  Identifier Name;
  DeclAttributes Attributes;
};

} // namespace sma
} // namespace language

namespace toolchain {
namespace yaml {
template <typename T> struct MappingTraits<std::shared_ptr<T>> {
  static void mapping(IO &io, std::shared_ptr<T> &Ptr) {
    MappingTraits<T>::mapping(io, *Ptr.get());
  }
};
} // namespace yaml
} // namespace toolchain

TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(::language::sma::TypeName)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(::language::sma::SubmoduleName)

TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::StructDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::EnumCaseDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::EnumDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::ClassDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::ProtocolDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::TypealiasDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::AssociatedTypeDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::VarDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::LetDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(::language::sma::FuncParam)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::vector<::language::sma::FuncParam>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::FuncDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::InitDecl>)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(std::shared_ptr<::language::sma::DeinitDecl>)

TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(::language::sma::GenericParam)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(::language::sma::ConformanceRequirement)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(::language::sma::SameTypeRequirement)

namespace toolchain {
namespace yaml {
template <> struct MappingTraits<::language::sma::DeclAttributes> {
  static void mapping(IO &io, ::language::sma::DeclAttributes &DA) {
#define SERIALIZE_MEMBER(NAME) io.mapOptional(#NAME, DA.NAME, false);
    SMA_FOR_EVERY_DECL_ATTRIBUTE(SERIALIZE_MEMBER)
#undef SERIALIZE_MEMBER
  }
};

template <> struct MappingTraits<::language::sma::NestedDecls> {
  // Defined out of line to break circular dependency.
  static void mapping(IO &io, ::language::sma::NestedDecls &ND);
};

template <> struct MappingTraits<::language::sma::GenericParam> {
  static void mapping(IO &io, ::language::sma::GenericParam &ND) {
    io.mapRequired("Name", ND.Name);
  }
};

template <> struct MappingTraits<::language::sma::ConformanceRequirement> {
  static void mapping(IO &io, ::language::sma::ConformanceRequirement &CR) {
    io.mapRequired("Type", CR.Type);
    io.mapRequired("Protocol", CR.Protocol);
  }
};

template <> struct MappingTraits<::language::sma::SameTypeRequirement> {
  static void mapping(IO &io, ::language::sma::SameTypeRequirement &STR) {
    io.mapRequired("FirstType", STR.FirstType);
    io.mapRequired("SecondType", STR.SecondType);
  }
};

template <> struct MappingTraits<::language::sma::GenericSignature> {
  static void mapping(IO &io, ::language::sma::GenericSignature &GS) {
    io.mapOptional("GenericParams", GS.GenericParams);
    io.mapOptional("ConformanceRequirements", GS.ConformanceRequirements);
    io.mapOptional("SameTypeRequirements", GS.SameTypeRequirements);
  }
};

template <> struct MappingTraits<::language::sma::Module> {
  static void mapping(IO &io, ::language::sma::Module &M) {
    io.mapRequired("Name", M.Name);
    io.mapOptional("ExportedModules", M.ExportedModules);
    io.mapOptional("NestedDecls", M.Decls, ::language::sma::NestedDecls());
  }
};

template <> struct MappingTraits<::language::sma::StructDecl> {
  static void mapping(IO &io, ::language::sma::StructDecl &SD) {
    io.mapRequired("Name", SD.Name);
    io.mapOptional("GenericSignature", SD.TheGenericSignature);
    io.mapOptional("ConformsToProtocols", SD.ConformsToProtocols);
    io.mapOptional("Attributes", SD.Attributes, ::language::sma::DeclAttributes());
    io.mapOptional("NestedDecls", SD.Decls, ::language::sma::NestedDecls());
  }
};

template <> struct MappingTraits<::language::sma::EnumCaseDecl> {
  static void mapping(IO &io, ::language::sma::EnumCaseDecl &ECD) {
    io.mapRequired("Name", ECD.Name);
    io.mapOptional("ArgumentType", ECD.ArgumentType);
    io.mapOptional("RawValue", ECD.RawValue);
    io.mapOptional("Attributes", ECD.Attributes,
                   ::language::sma::DeclAttributes());
  }
};

template <> struct MappingTraits<::language::sma::EnumDecl> {
  static void mapping(IO &io, ::language::sma::EnumDecl &ED) {
    io.mapRequired("Name", ED.Name);
    io.mapOptional("GenericSignature", ED.TheGenericSignature);
    io.mapOptional("RawType", ED.RawType);
    io.mapOptional("ConformsToProtocols", ED.ConformsToProtocols);
    io.mapOptional("Attributes", ED.Attributes, ::language::sma::DeclAttributes());
    io.mapOptional("Cases", ED.Cases);
    io.mapOptional("NestedDecls", ED.Decls, ::language::sma::NestedDecls());
  }
};

template <> struct MappingTraits<::language::sma::ClassDecl> {
  static void mapping(IO &io, ::language::sma::ClassDecl &CD) {
    io.mapRequired("Name", CD.Name);
    io.mapOptional("GenericSignature", CD.TheGenericSignature);
    io.mapOptional("Superclass", CD.Superclass);
    io.mapOptional("ConformsToProtocols", CD.ConformsToProtocols);
    io.mapOptional("Attributes", CD.Attributes, ::language::sma::DeclAttributes());
    io.mapOptional("NestedDecls", CD.Decls, ::language::sma::NestedDecls());
  }
};

template <> struct MappingTraits<::language::sma::ProtocolDecl> {
  static void mapping(IO &io, ::language::sma::ProtocolDecl &PD) {
    io.mapRequired("Name", PD.Name);
    io.mapOptional("ConformsToProtocols", PD.ConformsToProtocols);
    io.mapOptional("Attributes", PD.Attributes, ::language::sma::DeclAttributes());
    io.mapOptional("NestedDecls", PD.Decls, ::language::sma::NestedDecls());
  }
};

template <> struct MappingTraits<::language::sma::TypealiasDecl> {
  static void mapping(IO &io, ::language::sma::TypealiasDecl &TD) {
    io.mapRequired("Name", TD.Name);
    io.mapRequired("Type", TD.Type);
    io.mapOptional("Attributes", TD.Attributes, ::language::sma::DeclAttributes());
  }
};

template <> struct MappingTraits<::language::sma::AssociatedTypeDecl> {
  static void mapping(IO &io, ::language::sma::AssociatedTypeDecl &ATD) {
    io.mapRequired("Name", ATD.Name);
    io.mapOptional("DefaultDefinition", ATD.DefaultDefinition);
    io.mapOptional("Attributes", ATD.Attributes,
                   ::language::sma::DeclAttributes());
  }
};

template <> struct MappingTraits<::language::sma::VarDecl> {
  static void mapping(IO &io, ::language::sma::VarDecl &VD) {
    io.mapRequired("Name", VD.Name);
    io.mapRequired("Type", VD.Type);
    io.mapOptional("IsSettable", VD.IsSettable, false);
    io.mapOptional("IsStored", VD.IsStored, false);
    io.mapOptional("Attributes", VD.Attributes, ::language::sma::DeclAttributes());
  }
};

template <> struct MappingTraits<::language::sma::LetDecl> {
  static void mapping(IO &io, ::language::sma::LetDecl &LD) {
    io.mapRequired("Name", LD.Name);
    io.mapRequired("Type", LD.Type);
    io.mapOptional("Attributes", LD.Attributes, ::language::sma::DeclAttributes());
  }
};

template <> struct MappingTraits<::language::sma::FuncParam> {
  static void mapping(IO &io, ::language::sma::FuncParam &FP) {
    io.mapRequired("Name", FP.Name);
    io.mapRequired("Type", FP.Type);
    io.mapOptional("IsInout", FP.IsInout, false);
    io.mapOptional("Attributes", FP.Attributes, ::language::sma::DeclAttributes());
  }
};

template <> struct MappingTraits<::language::sma::FuncDecl> {
  static void mapping(IO &io, ::language::sma::FuncDecl &FD) {
    io.mapRequired("Name", FD.Name);
    io.mapOptional("IsStatic", FD.IsStatic, false);
    io.mapOptional("GenericSignature", FD.TheGenericSignature);
    io.mapRequired("Params", FD.Params);
    io.mapRequired("ResultType", FD.ResultType);
    io.mapOptional("Attributes", FD.Attributes, ::language::sma::DeclAttributes());
  }
};

template <> struct ScalarEnumerationTraits<::language::sma::InitializerKind> {
  static void enumeration(IO &io, ::language::sma::InitializerKind &Value) {
    io.enumCase(Value, "Designated", ::language::sma::InitializerKind::Designated);
    io.enumCase(Value, "Convenience",
                ::language::sma::InitializerKind::Convenience);
  }
};

template <>
struct ScalarEnumerationTraits<::language::sma::InitializerFailability> {
  static void enumeration(IO &io, ::language::sma::InitializerFailability &Value) {
    io.enumCase(Value, "None", ::language::sma::InitializerFailability::None);
    io.enumCase(Value, "Optional",
                ::language::sma::InitializerFailability::Optional);
    io.enumCase(
        Value, "ImplicitlyUnwrappedOptional",
        ::language::sma::InitializerFailability::ImplicitlyUnwrappedOptional);
  }
};

template <> struct MappingTraits<::language::sma::InitDecl> {
  static void mapping(IO &io, ::language::sma::InitDecl &ID) {
    io.mapRequired("InitializerKind", ID.TheInitializerKind);
    io.mapRequired("InitializerFailability", ID.TheInitializerFailability);
    io.mapOptional("GenericSignature", ID.TheGenericSignature);
    io.mapRequired("Params", ID.Params);
    io.mapOptional("IsTrappingStub", ID.IsTrappingStub);
    io.mapOptional("Attributes", ID.Attributes, ::language::sma::DeclAttributes());
  }
};

template <> struct MappingTraits<::language::sma::DeinitDecl> {
  static void mapping(IO &io, ::language::sma::DeinitDecl &DD) {
    io.mapRequired("Name", DD.Name);
    io.mapOptional("Attributes", DD.Attributes, ::language::sma::DeclAttributes());
  }
};

void MappingTraits<::language::sma::NestedDecls>::mapping(
    IO &io, ::language::sma::NestedDecls &ND) {
  io.mapOptional("Structs", ND.Structs);
  io.mapOptional("Enums", ND.Enums);
  io.mapOptional("Classes", ND.Classes);
  io.mapOptional("Protocols", ND.Protocols);
  io.mapOptional("Typealiases", ND.Typealiases);
  io.mapOptional("AssociatedTypes", ND.AssociatedTypes);
  io.mapOptional("Vars", ND.Vars);
  io.mapOptional("Lets", ND.Lets);
  io.mapOptional("Functions", ND.Functions);
  io.mapOptional("Initializers", ND.Initializers);
  io.mapOptional("Deinitializers", ND.Deinitializers);
}

} // namespace yaml
} // namespace toolchain

namespace {

class SMAModelGenerator : public DeclVisitor<SMAModelGenerator> {
  sma::NestedDecls Result;

public:
  void visit(const Decl *D) {
    DeclVisitor<SMAModelGenerator>::visit(const_cast<Decl *>(D));
  }

  sma::NestedDecls &&takeNestedDecls() { return std::move(Result); }

  sma::Identifier convertToIdentifier(Identifier I) const {
    return sma::Identifier{I.str().str()};
  }

  sma::TypeName convertToTypeName(Type T) const {
    PrintOptions Options;
    Options.PreferTypeRepr = true;

    sma::TypeName ResultTN;
    toolchain::raw_string_ostream OS(ResultTN.Name);
    T.print(OS, Options);
    return ResultTN;
  }

  std::optional<sma::TypeName> convertToOptionalTypeName(Type T) const {
    if (!T)
      return std::nullopt;
    return convertToTypeName(T);
  }

  std::optional<sma::GenericSignature>
  convertToGenericSignature(GenericSignature GS) {
    if (!GS)
      return std::nullopt;
    sma::GenericSignature ResultGS;
    for (auto *GTPT : GS.getGenericParams()) {
      sma::GenericParam ResultGP;
      ResultGP.Name = convertToIdentifier(GTPT->getName());
      ResultGS.GenericParams.emplace_back(std::move(ResultGP));
    }
    for (auto &Req : GS.getRequirements()) {
      switch (Req.getKind()) {
      case RequirementKind::Superclass:
      case RequirementKind::Conformance:
        ResultGS.ConformanceRequirements.emplace_back(
            sma::ConformanceRequirement{
                convertToTypeName(Req.getFirstType()),
                convertToTypeName(Req.getSecondType())});
        break;
      case RequirementKind::Layout:
      case RequirementKind::SameShape:
        // FIXME
        assert(false && "Not implemented");
        break;
      case RequirementKind::SameType:
        ResultGS.SameTypeRequirements.emplace_back(
            sma::SameTypeRequirement{convertToTypeName(Req.getFirstType()),
                                     convertToTypeName(Req.getSecondType())});
        break;
      }
    }
    return ResultGS;
  }

  std::vector<sma::TypeName> collectProtocolConformances(NominalTypeDecl *NTD) {
    const auto AllProtocols = NTD->getAllProtocols();
    std::vector<sma::TypeName> Result;
    Result.reserve(AllProtocols.size());
    for (const auto *PD : AllProtocols) {
      Result.emplace_back(convertToTypeName(PD->getDeclaredInterfaceType()));
    }
    return Result;
  }

  void visitDecl(Decl *D) {
    // FIXME: maybe don't have a default case
  }

  void visitStructDecl(StructDecl *SD) {
    auto ResultSD = std::make_shared<sma::StructDecl>();
    ResultSD->Name = convertToIdentifier(SD->getName());
    ResultSD->TheGenericSignature =
        convertToGenericSignature(SD->getGenericSignature());
    ResultSD->ConformsToProtocols = collectProtocolConformances(SD);
    // FIXME
    // ResultSD->Attributes = ?;
    // ResultSD->Decls = ?;
    Result.Structs.emplace_back(std::move(ResultSD));
  }

  void visitEnumDecl(EnumDecl *ED) {
    auto ResultED = std::make_shared<sma::EnumDecl>();
    ResultED->Name = convertToIdentifier(ED->getName());
    ResultED->TheGenericSignature =
        convertToGenericSignature(ED->getGenericSignature());
    ResultED->RawType = convertToOptionalTypeName(ED->getRawType());
    ResultED->ConformsToProtocols = collectProtocolConformances(ED);
    // FIXME
    // ResultED->Attributes = ?;
    // ResultED->Decls = ?;
    Result.Enums.emplace_back(std::move(ResultED));
  }

  void visitClassDecl(ClassDecl *CD) {
    auto ResultCD = std::make_shared<sma::ClassDecl>();
    ResultCD->Name = convertToIdentifier(CD->getName());
    ResultCD->TheGenericSignature =
        convertToGenericSignature(CD->getGenericSignature());
    ResultCD->Superclass = convertToOptionalTypeName(CD->getSuperclass());
    ResultCD->ConformsToProtocols = collectProtocolConformances(CD);
    // FIXME
    // ResultCD->Attributes = ?;
    // ResultCD->Decls = ?;
    Result.Classes.emplace_back(std::move(ResultCD));
  }

  void visitProtocolDecl(ProtocolDecl *PD) {
    auto ResultPD = std::make_shared<sma::ProtocolDecl>();
    ResultPD->Name = convertToIdentifier(PD->getName());
    ResultPD->ConformsToProtocols = collectProtocolConformances(PD);
    // FIXME
    // ResultPD->Attributes = ?;
    // ResultPD->Decls = ?;
    Result.Protocols.emplace_back(std::move(ResultPD));
  }

  void visitTypeAliasDecl(TypeAliasDecl *TAD) {
    auto ResultTD = std::make_shared<sma::TypealiasDecl>();
    ResultTD->Name = convertToIdentifier(TAD->getName());
    ResultTD->Type = convertToTypeName(TAD->getUnderlyingType());
    // FIXME
    // ResultTD->Attributes = ?;
    Result.Typealiases.emplace_back(std::move(ResultTD));
  }

  void visitAssociatedTypeDecl(AssociatedTypeDecl *ATD) {
    auto ResultATD = std::make_shared<sma::AssociatedTypeDecl>();
    ResultATD->Name = convertToIdentifier(ATD->getName());
    ResultATD->DefaultDefinition =
        convertToOptionalTypeName(ATD->getDefaultDefinitionType());
    // FIXME
    // ResultATD->Attributes = ?;
    Result.AssociatedTypes.emplace_back(std::move(ResultATD));
  }

  // FIXME
  // VarDecl
  // LetDecl

  void visitFuncDecl(FuncDecl *FD) {
    auto ResultFD = std::make_shared<sma::FuncDecl>();
    // FIXME
    Result.Functions.emplace_back(std::move(ResultFD));
  }

  void visitConstructorDecl(ConstructorDecl *CD) {
    auto ResultID = std::make_shared<sma::InitDecl>();
    // FIXME
    Result.Initializers.emplace_back(std::move(ResultID));
  }

  void visitDestructorDecl(DestructorDecl *DD) {
    auto ResultDD = std::make_shared<sma::DeinitDecl>();
    ResultDD->Name.Name = "deinit";
    // FIXME
    // ResultDD->Attributes = ?;
    Result.Deinitializers.emplace_back(std::move(ResultDD));
  }
};

std::shared_ptr<sma::Module> createSMAModel(ModuleDecl *M) {
  SmallVector<Decl *, 1> Decls;
  language::getTopLevelDeclsForDisplay(M, Decls);

  SMAModelGenerator Generator;
  for (auto *D : Decls) {
    Generator.visit(D);
  }

  auto ResultM = std::make_shared<sma::Module>();
  ResultM->Name = Generator.convertToIdentifier(M->getName());

  // FIXME:
  // ResultM->ExportedModules = ?;

  ResultM->Decls = Generator.takeNestedDecls();

  return ResultM;
}

} // unnamed namespace

int language::doGenerateModuleAPIDescription(StringRef DriverPath,
                                          StringRef MainExecutablePath,
                                          ArrayRef<std::string> Args) {
  std::vector<const char *> CStringArgs;
  for (auto &S : Args) {
    CStringArgs.push_back(S.c_str());
  }

  PrintingDiagnosticConsumer PDC;
  SourceManager SM;
  DiagnosticEngine Diags(SM);
  Diags.addConsumer(PDC);

  CompilerInvocation Invocation;
  bool HadError = driver::getSingleFrontendInvocationFromDriverArguments(
      MainExecutablePath, CStringArgs, Diags,
      [&](ArrayRef<const char *> FrontendArgs) {
        return Invocation.parseArgs(FrontendArgs, Diags);
      });

  if (HadError) {
    toolchain::errs() << "error: unable to create a CompilerInvocation\n";
    return 1;
  }

  Invocation.setMainExecutablePath(MainExecutablePath);

  CompilerInstance CI;
  CI.addDiagnosticConsumer(&PDC);
  std::string InstanceSetupError;
  if (CI.setup(Invocation, InstanceSetupError)) {
    toolchain::errs() << InstanceSetupError << '\n';
    return 1;
  }
  CI.performSema();

  PrintOptions Options = PrintOptions::printDeclarations();

  ModuleDecl *M = CI.getMainModule();
  M->getMainSourceFile().print(toolchain::outs(), Options);

  auto SMAModel = createSMAModel(M);
  toolchain::yaml::Output YOut(toolchain::outs());
  YOut << *SMAModel.get();

  return 0;
}
