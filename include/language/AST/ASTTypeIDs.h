//===--- ASTTypeIDs.h - AST Type Ids ----------------------------*- C++ -*-===//
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
//  This file defines TypeID support for AST types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_ASTTYPEIDS_H
#define LANGUAGE_AST_ASTTYPEIDS_H

#include "language/Basic/Toolchain.h"
#include "language/Basic/TypeID.h"
#include <optional>

namespace language {

class AbstractFunctionDecl;
class ActorIsolation;
class ApplyExpr;
enum class BodyInitKind;
struct BodyInitKindAndExpr;
class BraceStmt;
class ClosureExpr;
class IDEInspectionCallbacksFactory;
class ConstructorDecl;
class CustomAttr;
class Decl;
class EnumDecl;
class Fingerprint;
class FuncDecl;
class GenericParamList;
class GenericSignature;
class GenericTypeParamType;
class InfixOperatorDecl;
class IsSingleValueStmtResult;
class IterableDeclContext;
class ModuleDecl;
struct ImplicitImportList;
class NamedPattern;
class NominalTypeDecl;
class OperatorDecl;
class OpaqueTypeDecl;
class PatternBindingEntry;
class ParamDecl;
enum class ParamSpecifier : uint8_t;
class PostfixOperatorDecl;
class PrecedenceGroupDecl;
class PrefixOperatorDecl;
struct PropertyWrapperAuxiliaryVariables;
class PropertyWrapperInitializerInfo;
struct PropertyWrapperTypeInfo;
enum class CtorInitializerKind;
struct PropertyWrapperLValueness;
struct PropertyWrapperMutability;
class ProtocolConformance;
class ProtocolDecl;
class Requirement;
enum class ResilienceExpansion : unsigned;
struct FragileFunctionKind;
enum class PolymorphicEffectKind : uint8_t;
class PolymorphicEffectRequirementList;
class SourceFile;
struct TangentPropertyInfo;
class Type;
class TypeAliasDecl;
struct TypePair;
struct TypeWitnessAndDecl;
class ValueDecl;
class VarDecl;
class Witness;
enum class AncestryFlags : uint8_t;
enum class ImplicitMemberAction : uint8_t;
struct FingerprintAndMembers;
class Identifier;
class BodyAndFingerprint;
struct ConstValueTypeInfo;

// Define the AST type zone (zone 1)
#define LANGUAGE_TYPEID_ZONE AST
#define LANGUAGE_TYPEID_HEADER "language/AST/ASTTypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"

} // end namespace language

#endif // LANGUAGE_AST_ASTTYPEIDS_H
