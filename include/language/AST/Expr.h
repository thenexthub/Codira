//===--- Expr.h - Codira Language Expression ASTs ----------------*- C++ -*-===//
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
// This file defines the Expr class and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_EXPR_H
#define LANGUAGE_AST_EXPR_H

#include "language/AST/ArgumentList.h"
#include "language/AST/Attr.h"
#include "language/AST/AvailabilityRange.h"
#include "language/AST/CaptureInfo.h"
#include "language/AST/ConcreteDeclRef.h"
#include "language/AST/Decl.h"
#include "language/AST/DeclContext.h"
#include "language/AST/DeclNameLoc.h"
#include "language/AST/FreestandingMacroExpansion.h"
#include "language/AST/FunctionRefInfo.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/AST/ThrownErrorDestination.h"
#include "language/AST/TypeAlignments.h"
#include "language/Basic/Debug.h"
#include "language/Basic/InlineBitfield.h"
#include "toolchain/Support/TrailingObjects.h"
#include <optional>
#include <utility>

namespace toolchain {
  struct fltSemantics;
}

namespace language {
  enum class AccessKind : unsigned char;
  class ArchetypeType;
  class ASTContext;
  class AvailabilitySpec;
  class DeclRefTypeRepr;
  class Type;
  class TypeRepr;
  class ValueDecl;
  class Decl;
  class DeclRefExpr;
  class ExistentialArchetypeType;
  class ParamDecl;
  class Pattern;
  class SubscriptDecl;
  class Stmt;
  class BraceStmt;
  class ASTWalker;
  class Initializer;
  class VarDecl;
  class OpaqueValueExpr;
  class FuncDecl;
  class ConstructorDecl;
  class TypeDecl;
  class PatternBindingDecl;
  class ParameterList;
  class EnumElementDecl;
  class CallExpr;
  class KeyPathExpr;
  class CaptureListExpr;
  class ThenStmt;
  class ReturnStmt;

enum class ExprKind : uint8_t {
#define EXPR(Id, Parent) Id,
#define LAST_EXPR(Id) Last_Expr = Id,
#define EXPR_RANGE(Id, FirstId, LastId) \
  First_##Id##Expr = FirstId, Last_##Id##Expr = LastId,
#include "language/AST/ExprNodes.def"
};
enum : unsigned { NumExprKindBits =
  countBitsUsed(static_cast<unsigned>(ExprKind::Last_Expr)) };
  
/// Discriminates certain kinds of checked cast that have specialized diagnostic
/// and/or code generation peephole behavior.
///
/// This enumeration should not have any semantic effect on the behavior of a
/// well-typed program, since the runtime can perform all casts that are
/// statically accepted.
enum class CheckedCastKind : unsigned {
  /// The kind has not been determined yet.
  Unresolved,

  /// Valid resolved kinds start here.
  First_Resolved,

  /// The requested cast is an implicit conversion, so this is a coercion.
  Coercion = First_Resolved,
  /// A checked cast with no known specific behavior.
  ValueCast,
  // A downcast from an array type to another array type.
  ArrayDowncast,
  // A downcast from a dictionary type to another dictionary type.
  DictionaryDowncast,
  // A downcast from a set type to another set type.
  SetDowncast,
  /// A bridging conversion that always succeeds.
  BridgingCoercion,

  Last_CheckedCastKind = BridgingCoercion,
};

/// What are the high-level semantics of this access?
enum class AccessSemantics : uint8_t {
  /// On a storage reference, this is a direct access to the underlying
  /// physical storage, bypassing any observers.  The declaration must be 
  /// a variable with storage.
  ///
  /// On a function reference, this is a non-polymorphic access to a
  /// particular implementation.
  DirectToStorage,

  /// On a storage reference, this is a direct access to the concrete
  /// implementation of this storage, bypassing any possibility of override.
  DirectToImplementation,

  /// This is an ordinary access to a declaration, using whatever
  /// polymorphism is expected.
  Ordinary,

  /// This is an access to the underlying storage through a distributed thunk.
  ///
  /// The declaration must be a 'distributed' computed property used outside
  /// of its actor isolation context.
  DistributedThunk,
};

/// Expr - Base class for all expressions in language.
class alignas(8) Expr : public ASTAllocated<Expr> {
  Expr(const Expr&) = delete;
  void operator=(const Expr&) = delete;

protected:
  // clang-format off
  union { uint64_t OpaqueBits;

  LANGUAGE_INLINE_BITFIELD_BASE(Expr, bitmax(NumExprKindBits,8)+1,
    /// The subclass of Expr that this is.
    Kind : bitmax(NumExprKindBits,8),
    /// Whether the Expr represents something directly written in source or
    /// it was implicitly generated by the type-checker.
    Implicit : 1
  );

  LANGUAGE_INLINE_BITFIELD_FULL(CollectionExpr, Expr, 64-NumExprBits,
    /// True if the type of this collection expr was inferred by the collection
    /// fallback type, like [Any].
    IsTypeDefaulted : 1,
    /// Number of comma source locations.
    NumCommas : 32 - 1 - NumExprBits,
    /// Number of entries in the collection. If this is a DictionaryExpr,
    /// each entry is a Tuple with the key and value pair.
    NumSubExprs : 32
  );

  LANGUAGE_INLINE_BITFIELD_EMPTY(LiteralExpr, Expr);
  LANGUAGE_INLINE_BITFIELD_EMPTY(IdentityExpr, Expr);
  LANGUAGE_INLINE_BITFIELD(LookupExpr, Expr, 1+1+1,
    IsSuper : 1,
    IsImplicitlyAsync : 1,
    IsImplicitlyThrows : 1
  );
  LANGUAGE_INLINE_BITFIELD_EMPTY(DynamicLookupExpr, LookupExpr);

  LANGUAGE_INLINE_BITFIELD_EMPTY(ParenExpr, IdentityExpr);

  LANGUAGE_INLINE_BITFIELD(NumberLiteralExpr, LiteralExpr, 1+1,
    IsNegative : 1,
    IsExplicitConversion : 1
  );

  LANGUAGE_INLINE_BITFIELD(StringLiteralExpr, LiteralExpr, 3+1+1,
    Encoding : 3,
    IsSingleUnicodeScalar : 1,
    IsSingleExtendedGraphemeCluster : 1
  );

  LANGUAGE_INLINE_BITFIELD_FULL(InterpolatedStringLiteralExpr, LiteralExpr, 32+20,
    : NumPadBits,
    InterpolationCount : 20,
    LiteralCapacity : 32
  );

  LANGUAGE_INLINE_BITFIELD(DeclRefExpr, Expr, 2+3+1+1,
    Semantics : 2, // an AccessSemantics
    FunctionRefInfo : 3,
    IsImplicitlyAsync : 1,
    IsImplicitlyThrows : 1
  );

  LANGUAGE_INLINE_BITFIELD(UnresolvedDeclRefExpr, Expr, 2+3,
    DeclRefKind : 2,
    FunctionRefInfo : 3
  );

  LANGUAGE_INLINE_BITFIELD(MemberRefExpr, LookupExpr, 2,
    Semantics : 2 // an AccessSemantics
  );

  LANGUAGE_INLINE_BITFIELD_FULL(TupleElementExpr, Expr, 32,
    : NumPadBits,
    FieldNo : 32
  );

  LANGUAGE_INLINE_BITFIELD_FULL(TupleExpr, Expr, 1+1+32,
    /// Whether this tuple has any labels.
    HasElementNames : 1,

    /// Whether this tuple has label locations.
    HasElementNameLocations : 1,

    : NumPadBits,
    NumElements : 32
  );

  LANGUAGE_INLINE_BITFIELD(UnresolvedDotExpr, Expr, 3,
    FunctionRefInfo : 3
  );

  LANGUAGE_INLINE_BITFIELD_FULL(SubscriptExpr, LookupExpr, 2,
    Semantics : 2 // an AccessSemantics
  );

  LANGUAGE_INLINE_BITFIELD_EMPTY(DynamicSubscriptExpr, DynamicLookupExpr);

  LANGUAGE_INLINE_BITFIELD_FULL(UnresolvedMemberExpr, Expr, 3,
    FunctionRefInfo : 3
  );

  LANGUAGE_INLINE_BITFIELD(OverloadSetRefExpr, Expr, 3,
    FunctionRefInfo : 3
  );

  LANGUAGE_INLINE_BITFIELD(BooleanLiteralExpr, LiteralExpr, 1,
    Value : 1
  );

  LANGUAGE_INLINE_BITFIELD(MagicIdentifierLiteralExpr, LiteralExpr, 3+1,
    Kind : 3,
    StringEncoding : 1
  );

  LANGUAGE_INLINE_BITFIELD_FULL(ObjectLiteralExpr, LiteralExpr, 3,
    LitKind : 3
  );

  LANGUAGE_INLINE_BITFIELD(AbstractClosureExpr, Expr, (16-NumExprBits)+16,
    : 16 - NumExprBits, // Align and leave room for subclasses
    Discriminator : 16
  );

  LANGUAGE_INLINE_BITFIELD(AutoClosureExpr, AbstractClosureExpr, 2,
    /// If the autoclosure was built for a curry thunk, the thunk kind is
    /// stored here.
    Kind : 2
  );

  LANGUAGE_INLINE_BITFIELD(ClosureExpr, AbstractClosureExpr, 1+1+1+1+1+1+1+1+1,
    /// True if closure parameters were synthesized from anonymous closure
    /// variables.
    HasAnonymousClosureVars : 1,

    /// True if "self" can be captured implicitly without requiring "self."
    /// on each member reference.
    ImplicitSelfCapture : 1,

    /// True if this closure parameter should implicitly inherit the actor
    /// context from where it was formed.
    InheritActorContext : 1,
    /// The kind for inheritance - none or always at the moment.
    InheritActorContextKind : 1,

    /// True if this closure's actor isolation behavior was determined by an
    /// \c \@preconcurrency declaration.
    IsolatedByPreconcurrency : 1,

    /// True if this is a closure literal that is passed to a sending parameter.
    IsPassedToSendingParameter : 1,

    /// True if we're in the common case where the GlobalActorAttributeRequest
    /// request returned a pair of null pointers.
    NoGlobalActorAttribute : 1,

    /// Indicates whether this closure literal would require dynamic actor 
    /// isolation checks when it either specifies or inherits isolation
    /// and was passed as an argument to a function that is not fully 
    /// concurrency checked.
    RequiresDynamicIsolationChecking : 1,

    /// Whether this closure was type-checked as an argument to a macro. This
    /// is only populated after type-checking, and only exists for diagnostic
    /// logic. Do not add more uses of this.
    IsMacroArgument : 1
  );

  LANGUAGE_INLINE_BITFIELD_FULL(BindOptionalExpr, Expr, 16,
    : NumPadBits,
    Depth : 16
  );

  LANGUAGE_INLINE_BITFIELD_EMPTY(ImplicitConversionExpr, Expr);

  LANGUAGE_INLINE_BITFIELD_FULL(DestructureTupleExpr, ImplicitConversionExpr, 16,
    /// The number of elements in the tuple type being destructured.
    NumElements : 16
  );

  LANGUAGE_INLINE_BITFIELD(ForceValueExpr, Expr, 1,
    ForcedIUO : 1
  );

  LANGUAGE_INLINE_BITFIELD(InOutToPointerExpr, ImplicitConversionExpr, 1,
    IsNonAccessing : 1
  );

  LANGUAGE_INLINE_BITFIELD(ArrayToPointerExpr, ImplicitConversionExpr, 1,
    IsNonAccessing : 1
  );

  LANGUAGE_INLINE_BITFIELD_FULL(ErasureExpr, ImplicitConversionExpr, 32+20,
    : NumPadBits,
    NumConformances : 32,
    NumArgumentConversions : 20
  );

  LANGUAGE_INLINE_BITFIELD_FULL(UnresolvedSpecializeExpr, Expr, 32,
    : NumPadBits,
    NumUnresolvedParams : 32
  );

  LANGUAGE_INLINE_BITFIELD_FULL(CaptureListExpr, Expr, 32,
    : NumPadBits,
    NumCaptures : 32
  );

  LANGUAGE_INLINE_BITFIELD(ApplyExpr, Expr, 1+1+1+1+1,
    ThrowsIsSet : 1,
    ImplicitlyAsync : 1,
    ImplicitlyThrows : 1,
    NoAsync : 1,
    ShouldApplyDistributedThunk : 1
  );

  LANGUAGE_INLINE_BITFIELD_EMPTY(CallExpr, ApplyExpr);

  enum { NumCheckedCastKindBits = 4 };
  LANGUAGE_INLINE_BITFIELD(CheckedCastExpr, Expr, NumCheckedCastKindBits,
    CastKind : NumCheckedCastKindBits
  );
  static_assert(unsigned(CheckedCastKind::Last_CheckedCastKind)
                  < (1 << NumCheckedCastKindBits),
                "unable to fit a CheckedCastKind in the given number of bits");

  LANGUAGE_INLINE_BITFIELD_EMPTY(CollectionUpcastConversionExpr, Expr);

  LANGUAGE_INLINE_BITFIELD(ObjCSelectorExpr, Expr, 2,
    /// The selector kind.
    SelectorKind : 2
  );

  LANGUAGE_INLINE_BITFIELD(KeyPathExpr, Expr, 1,
    /// Whether this is an ObjC stringified keypath.
    IsObjC : 1
  );

  LANGUAGE_INLINE_BITFIELD_FULL(SequenceExpr, Expr, 32,
    : NumPadBits,
    NumElements : 32
  );

  LANGUAGE_INLINE_BITFIELD(OpaqueValueExpr, Expr, 1,
    IsPlaceholder : 1
  );

  LANGUAGE_INLINE_BITFIELD_FULL(TypeJoinExpr, Expr, 32,
    : NumPadBits,
    NumElements : 32
  );

  } Bits;
  // clang-format on

private:
  /// Ty - This is the type of the expression.
  Type Ty;
 
protected:
  Expr(ExprKind Kind, bool Implicit, Type Ty = Type()) : Ty(Ty) {
    Bits.OpaqueBits = 0;
    Bits.Expr.Kind = unsigned(Kind);
    Bits.Expr.Implicit = Implicit;
  }

public:
  /// Return the kind of this expression.
  ExprKind getKind() const { return ExprKind(Bits.Expr.Kind); }

  /// Retrieve the name of the given expression kind.
  ///
  /// This name should only be used for debugging dumps and other
  /// developer aids, and should never be part of a diagnostic or exposed
  /// to the user of the compiler in any way.
  static StringRef getKindName(ExprKind K);

  /// getType - Return the type of this expression.
  Type getType() const { return Ty; }

  /// setType - Sets the type of this expression.
  void setType(Type T);

  /// Return the source range of the expression.
  SourceRange getSourceRange() const;
  
  /// getStartLoc - Return the location of the start of the expression.
  SourceLoc getStartLoc() const;

  /// Retrieve the location of the last token of the expression.
  SourceLoc getEndLoc() const;
  
  /// getLoc - Return the caret location of this expression.
  SourceLoc getLoc() const;

#define LANGUAGE_FORWARD_SOURCE_LOCS_TO(SUBEXPR) \
  SourceLoc getStartLoc() const { return (SUBEXPR)->getStartLoc(); } \
  SourceLoc getEndLoc() const { return (SUBEXPR)->getEndLoc(); } \
  SourceLoc getLoc() const { return (SUBEXPR)->getLoc(); } \
  SourceRange getSourceRange() const { return (SUBEXPR)->getSourceRange(); }

  SourceLoc TrailingSemiLoc;

  /// getSemanticsProvidingExpr - Find the smallest subexpression
  /// which obeys the property that evaluating it is exactly
  /// equivalent to evaluating this expression.
  ///
  /// Looks through parentheses.  Would not look through something
  /// like '(foo(), x:bar(), baz()).x'.
  Expr *getSemanticsProvidingExpr();

  const Expr *getSemanticsProvidingExpr() const {
    return const_cast<Expr *>(this)->getSemanticsProvidingExpr();
  }

  /// getValueProvidingExpr - Find the smallest subexpression which is
  /// responsible for generating the value of this expression.
  /// Evaluating the result is not necessarily equivalent to
  /// evaluating this expression because of potential missing
  /// side-effects (which may influence the returned value).
  Expr *getValueProvidingExpr();

  const Expr *getValueProvidingExpr() const {
    return const_cast<Expr *>(this)->getValueProvidingExpr();
  }

  /// Checks whether this expression is always hoisted above a binary operator
  /// when it appears on the left-hand side, e.g 'try x + y' becomes
  /// 'try (x + y)'. If so, returns the sub-expression, \c nullptr otherwise.
  ///
  /// Note that such expressions may not appear on the right-hand side of a
  /// binary operator, except for assignment and ternaries.
  Expr *getAlwaysLeftFoldedSubExpr() const;

  /// Checks whether this expression is always hoisted above a binary operator
  /// when it appears on the left-hand side, e.g 'try x + y' becomes
  /// 'try (x + y)'.
  ///
  /// Note that such expressions may not appear on the right-hand side of a
  /// binary operator, except for assignment and ternaries.
  bool isAlwaysLeftFolded() const { return bool(getAlwaysLeftFoldedSubExpr()); }

  /// Find the original expression value, looking through various
  /// implicit conversions.
  const Expr *findOriginalValue() const;

  /// Find the original type of a value, looking through various implicit
  /// conversions.
  Type findOriginalType() const;

  /// If this is a reference to an operator written as a member of a type (or
  /// extension thereof), return the underlying operator reference.
  DeclRefExpr *getMemberOperatorRef();

  /// This recursively walks the AST rooted at this expression.
  Expr *walk(ASTWalker &walker);
  Expr *walk(ASTWalker &&walker) { return walk(walker); }

  /// Enumerate each immediate child expression of this node, invoking the
  /// specific functor on it.  This ignores statements and other non-expression
  /// children.
  void forEachImmediateChildExpr(toolchain::function_ref<Expr *(Expr *)> callback);

  /// Enumerate each expr node within this expression subtree, invoking the
  /// specific functor on it.  This ignores statements and other non-expression
  /// children, and if there is a closure within the expression, this does not
  /// walk into the body of it (unless it is single-expression).
  void forEachChildExpr(toolchain::function_ref<Expr *(Expr *)> callback);

  /// Determine whether this expression refers to a type by name.
  ///
  /// This distinguishes static references to types, like Int, from metatype
  /// values, "someTy: Any.Type".
  bool isTypeReference(
      toolchain::function_ref<Type(Expr *)> getType = [](Expr *E) -> Type {
        return E->getType();
      },
      toolchain::function_ref<Decl *(Expr *)> getDecl = [](Expr *E) -> Decl * {
        return nullptr;
      }) const;

  /// Determine whether this expression refers to a statically-derived metatype.
  ///
  /// This implies `isTypeReference`, but also requires that the referenced type
  /// is not an archetype or dependent type.
  bool isStaticallyDerivedMetatype(
      toolchain::function_ref<Type(Expr *)> getType = [](Expr *E) -> Type {
        return E->getType();
      },
      toolchain::function_ref<bool(Expr *)> isTypeReference =
          [](Expr *E) { return E->isTypeReference(); }) const;

  /// isImplicit - Determines whether this expression was implicitly-generated,
  /// rather than explicitly written in the AST.
  bool isImplicit() const {
    return Bits.Expr.Implicit;
  }
  void setImplicit(bool Implicit = true);

  /// Retrieves the declaration that is being referenced by this
  /// expression, if any.
  ConcreteDeclRef getReferencedDecl(bool stopAtParenExpr = false) const;

  /// Determine whether this expression is 'super', possibly converted to
  /// a base class.
  bool isSuperExpr() const;

  /// Returns whether the semantically meaningful content of this expression is
  /// an inout expression.
  ///
  /// FIXME(Remove InOutType): This should eventually sub-in for
  /// 'E->getType()->is<InOutType>()' in all cases.
  bool isSemanticallyInOutExpr() const {
    return getSemanticsProvidingExpr()->getKind() == ExprKind::InOut;
  }

  bool printConstExprValue(toolchain::raw_ostream *OS, toolchain::function_ref<bool(Expr*)> additionalCheck) const;
  bool isSemanticallyConstExpr(toolchain::function_ref<bool(Expr*)> additionalCheck = nullptr) const;

  /// Returns false if this expression needs to be wrapped in parens when
  /// used inside of a any postfix expression, true otherwise.
  ///
  /// \param appendingPostfixOperator if the expression being
  /// appended is a postfix operator like '!' or '?'.
  bool canAppendPostfixExpression(bool appendingPostfixOperator = false) const;

  /// Returns true if this is an infix operator of some sort, including
  /// a builtin operator.
  bool isInfixOperator() const;

  /// Returns true if this is a reference to the implicit self of function.
  bool isSelfExprOf(const AbstractFunctionDecl *AFD,
                    bool sameBase = false) const;

  /// If the expression has an argument list, returns it. Otherwise, returns
  /// \c nullptr.
  ArgumentList *getArgs() const;

  /// If the expression has a DeclNameLoc, returns it. Otherwise, returns
  /// an nullp DeclNameLoc.
  DeclNameLoc getNameLoc() const;

  /// Produce a mapping from each subexpression to its parent
  /// expression, with the provided expression serving as the root of
  /// the parent map.
  toolchain::DenseMap<Expr *, Expr *> getParentMap();

  /// Whether this expression is a valid parent for a given TypeExpr.
  bool isValidParentOfTypeExpr(Expr *typeExpr) const;

  LANGUAGE_DEBUG_DUMP;
  void dump(raw_ostream &OS, unsigned Indent = 0) const;
  void dump(raw_ostream &OS, toolchain::function_ref<Type(Expr *)> getType,
            toolchain::function_ref<Type(TypeRepr *)> getTypeOfTypeRepr,
            toolchain::function_ref<Type(KeyPathExpr *E, unsigned index)>
                getTypeOfKeyPathComponent,
            unsigned Indent = 0) const;

  void print(ASTPrinter &Printer, const PrintOptions &Opts) const;
};

/// ErrorExpr - Represents a semantically erroneous subexpression in the AST,
/// typically this will have an ErrorType.
class ErrorExpr : public Expr {
  SourceRange Range;
  Expr *OriginalExpr;
public:
  ErrorExpr(SourceRange Range, Type Ty = Type(), Expr *OriginalExpr = nullptr)
    : Expr(ExprKind::Error, /*Implicit=*/true, Ty), Range(Range),
      OriginalExpr(OriginalExpr) {}

  SourceRange getSourceRange() const { return Range; }
  Expr *getOriginalExpr() const { return OriginalExpr; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Error;
  }
};

/// CodeCompletionExpr - Represents the code completion token in the AST, this
/// can help us preserve the context of the code completion position.
class CodeCompletionExpr : public Expr {
  Expr *Base;
  SourceLoc Loc;

public:
  CodeCompletionExpr(Expr *Base, SourceLoc Loc)
      : Expr(ExprKind::CodeCompletion, /*Implicit=*/true, Type()),
        Base(Base), Loc(Loc) {}

  CodeCompletionExpr(SourceLoc Loc)
      : CodeCompletionExpr(/*Base=*/nullptr, Loc) {}

  Expr *getBase() const { return Base; }
  void setBase(Expr *E) { Base = E; }

  SourceLoc getLoc() const { return Loc; }
  SourceLoc getStartLoc() const { return Base ? Base->getStartLoc() : Loc; }
  SourceLoc getEndLoc() const { return Loc; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::CodeCompletion;
  }
};

/// LiteralExpr - Common base class between the literals.
class LiteralExpr : public Expr {
  // Set by Sema:
  ConcreteDeclRef Initializer;

public:
  LiteralExpr(ExprKind Kind, bool Implicit) : Expr(Kind, Implicit) {}

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_LiteralExpr &&
           E->getKind() <= ExprKind::Last_LiteralExpr;
  }

  /// Retrieve the initializer that will be used to construct the
  /// literal from the result of the initializer.
  ///
  /// Only literals that have no builtin literal conformance will have
  /// this initializer, which will be called on the result of the builtin
  /// initializer.
  ConcreteDeclRef getInitializer() const { return Initializer; }

  /// Set the initializer that will be used to construct the literal.
  void setInitializer(ConcreteDeclRef initializer) {
    Initializer = initializer;
  }

  /// Get description string for the literal expression.
  StringRef getLiteralKindDescription() const;
};

/// BuiltinLiteralExpr - Common base class between all literals
/// that provides BuiltinInitializer
class BuiltinLiteralExpr : public LiteralExpr {
  // Set by Seam:
  ConcreteDeclRef BuiltinInitializer;

public:
  BuiltinLiteralExpr(ExprKind Kind, bool Implicit)
      : LiteralExpr(Kind, Implicit) {}

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_BuiltinLiteralExpr &&
           E->getKind() <= ExprKind::Last_BuiltinLiteralExpr;
  }

  /// Retrieve the builtin initializer that will be used to construct the
  /// literal.
  ///
  /// Any type-checked literal will have a builtin initializer, which is
  /// called first to form a concrete Codira type.
  ConcreteDeclRef getBuiltinInitializer() const { return BuiltinInitializer; }

  /// Set the builtin initializer that will be used to construct the
  /// literal.
  void setBuiltinInitializer(ConcreteDeclRef builtinInitializer);
};

/// The 'nil' literal.
///
class NilLiteralExpr : public LiteralExpr {
  SourceLoc Loc;

public:
  NilLiteralExpr(SourceLoc Loc, bool Implicit = false)
  : LiteralExpr(ExprKind::NilLiteral, Implicit), Loc(Loc) {
  }
  
  SourceRange getSourceRange() const {
    return Loc;
  }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::NilLiteral;
  }
};

/// Abstract base class for numeric literals, potentially with a sign.
class NumberLiteralExpr : public BuiltinLiteralExpr {
  /// The value of the literal as an ASTContext-owned string. Underscores must
  /// be stripped.
  StringRef Val;  // Use StringRef instead of APInt or APFloat, which leak.

protected:
  SourceLoc MinusLoc;
  SourceLoc DigitsLoc;
  
public:
  NumberLiteralExpr(ExprKind Kind, StringRef Val, SourceLoc DigitsLoc,
                    bool Implicit)
      : BuiltinLiteralExpr(Kind, Implicit), Val(Val), DigitsLoc(DigitsLoc) {
    Bits.NumberLiteralExpr.IsNegative = false;
    Bits.NumberLiteralExpr.IsExplicitConversion = false;
  }

  bool isNegative() const { return Bits.NumberLiteralExpr.IsNegative; }
  void setNegative(SourceLoc Loc) {
    MinusLoc = Loc;
    Bits.NumberLiteralExpr.IsNegative = true;
  }

  bool isExplicitConversion() const {
    return Bits.NumberLiteralExpr.IsExplicitConversion;
  }
  void setExplicitConversion(bool isExplicitConversion = true) {
    Bits.NumberLiteralExpr.IsExplicitConversion = isExplicitConversion;
  }

  StringRef getDigitsText() const { return Val; }

  SourceRange getSourceRange() const {
    if (isNegative())
      return { MinusLoc, DigitsLoc };
    else
      return DigitsLoc;
  }

  SourceLoc getMinusLoc() const {
    return MinusLoc;
  }

  SourceLoc getDigitsLoc() const {
    return DigitsLoc;
  }

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_NumberLiteralExpr
      && E->getKind() <= ExprKind::Last_NumberLiteralExpr;
  }
};

/// Integer literal with a '+' or '-' sign, like '+4' or '- 2'.
///
/// After semantic analysis assigns types, this is guaranteed to have
/// a BuiltinIntegerType or be a normal type and implicitly be
/// AnyBuiltinIntegerType.
class IntegerLiteralExpr : public NumberLiteralExpr {
public:
  IntegerLiteralExpr(StringRef Val, SourceLoc DigitsLoc, bool Implicit = false)
      : NumberLiteralExpr(ExprKind::IntegerLiteral,
                          Val, DigitsLoc, Implicit)
  {}

  /// Returns a new integer literal expression with the given value.
  /// \p C The AST context.
  /// \p value The integer value.
  /// \return An implicit integer literal expression which evaluates to the value.
  static IntegerLiteralExpr *
  createFromUnsigned(ASTContext &C, unsigned value, SourceLoc loc);

  /// Returns the value of the literal, appropriately constructed in the
  /// target type.
  APInt getValue() const;

  /// Returns the raw value of the literal without any truncation.
  APInt getRawValue() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::IntegerLiteral;
  }
};

/// FloatLiteralExpr - Floating point literal, like '4.0'.  After semantic
/// analysis assigns types, BuiltinTy is guaranteed to only have a
/// BuiltinFloatingPointType.
class FloatLiteralExpr : public NumberLiteralExpr {
  /// This is the type of the builtin literal.
  Type BuiltinTy;

public:
  FloatLiteralExpr(StringRef Val, SourceLoc Loc, bool Implicit = false)
    : NumberLiteralExpr(ExprKind::FloatLiteral, Val, Loc, Implicit)
  {}
  
  APFloat getValue() const;
  static APFloat getValue(StringRef Text, const toolchain::fltSemantics &Semantics,
                          bool Negative);
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::FloatLiteral;
  }

  Type getBuiltinType() const { return BuiltinTy; }
  void setBuiltinType(Type ty) { BuiltinTy = ty; }
};

/// A Boolean literal ('true' or 'false')
///
class BooleanLiteralExpr : public BuiltinLiteralExpr {
  SourceLoc Loc;

public:
  BooleanLiteralExpr(bool Value, SourceLoc Loc, bool Implicit = false)
      : BuiltinLiteralExpr(ExprKind::BooleanLiteral, Implicit), Loc(Loc) {
    Bits.BooleanLiteralExpr.Value = Value;
  }

  /// Retrieve the Boolean value of this literal.
  bool getValue() const { return Bits.BooleanLiteralExpr.Value; }

  SourceRange getSourceRange() const {
    return Loc;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::BooleanLiteral;
  }
};

/// StringLiteralExpr - String literal, like '"foo"'.
class StringLiteralExpr : public BuiltinLiteralExpr {
  StringRef Val;
  SourceRange Range;

public:
  /// The encoding that should be used for the string literal.
  enum Encoding : unsigned {
    /// A UTF-8 string.
    UTF8,

    /// A single UnicodeScalar, passed as an integer.
    OneUnicodeScalar
  };

  StringLiteralExpr(StringRef Val, SourceRange Range, bool Implicit = false);

  StringRef getValue() const { return Val; }
  SourceRange getSourceRange() const { return Range; }

  /// Determine the encoding that should be used for this string literal.
  Encoding getEncoding() const {
    return static_cast<Encoding>(Bits.StringLiteralExpr.Encoding);
  }

  /// Set the encoding that should be used for this string literal.
  void setEncoding(Encoding encoding) {
    Bits.StringLiteralExpr.Encoding = static_cast<unsigned>(encoding);
  }

  bool isSingleUnicodeScalar() const {
    return Bits.StringLiteralExpr.IsSingleUnicodeScalar;
  }

  bool isSingleExtendedGraphemeCluster() const {
    return Bits.StringLiteralExpr.IsSingleExtendedGraphemeCluster;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::StringLiteral;
  }
};

/// Runs a series of statements which use or modify \c SubExpr
/// before it is given to the rest of the expression.
///
/// \c Body should begin with a \c VarDecl; this defines the variable
/// \c TapExpr will initialize at the beginning and read a result
/// from at the end. \c TapExpr creates a separate scope, then
/// assigns the result of \c SubExpr to the variable and runs \c Body
/// in it, returning the value of the variable after the \c Body runs.
///
/// (The design here could be a bit cleaner, particularly where the VarDecl
/// is concerned.)
class TapExpr : public Expr {
  Expr *SubExpr;
  BraceStmt *Body;

public:
  TapExpr(Expr *SubExpr, BraceStmt *Body);

  Expr * getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr * se) { SubExpr = se; }

  /// The variable which will be accessed and possibly modified by
  /// the \c Body. This is the first \c ASTNode in the \c Body.
  VarDecl * getVar() const;

  BraceStmt * getBody() const { return Body; }
  void setBody(BraceStmt * b) { Body = b; }

  SourceLoc getLoc() const { return SubExpr ? SubExpr->getLoc() : SourceLoc(); }

  SourceRange getSourceRange() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Tap;
  }
};

/// InterpolatedStringLiteral - An interpolated string literal.
///
/// An interpolated string literal mixes expressions (which are evaluated and
/// converted into string form) within a string literal.
///
/// \code
/// "[\(min)..\(max)]"
/// \endcode
class InterpolatedStringLiteralExpr : public LiteralExpr {
  /// Points at the beginning quote.
  SourceLoc Loc;
  TapExpr *AppendingExpr;

  // Set by Sema:
  OpaqueValueExpr *interpolationExpr = nullptr;
  ConcreteDeclRef builderInit;
  Expr *interpolationCountExpr = nullptr;
  Expr *literalCapacityExpr = nullptr;

public:
  InterpolatedStringLiteralExpr(SourceLoc Loc,
                                unsigned LiteralCapacity,
                                unsigned InterpolationCount,
                                TapExpr *AppendingExpr)
      : LiteralExpr(ExprKind::InterpolatedStringLiteral, /*Implicit=*/false),
        Loc(Loc),
        AppendingExpr(AppendingExpr) {
    Bits.InterpolatedStringLiteralExpr.InterpolationCount = InterpolationCount;
    Bits.InterpolatedStringLiteralExpr.LiteralCapacity = LiteralCapacity;
  }

  // Sets the constructor for the interpolation type.
  void setBuilderInit(ConcreteDeclRef decl) { builderInit = decl; }
  ConcreteDeclRef getBuilderInit() const { return builderInit; }

  /// Sets the OpaqueValueExpr that is passed into AppendingExpr as the SubExpr
  /// that the tap operates on.
  void setInterpolationExpr(OpaqueValueExpr *expr) { interpolationExpr = expr; }
  OpaqueValueExpr *getInterpolationExpr() const { return interpolationExpr; }

  /// Store a builtin integer literal expr wrapping getInterpolationCount().
  /// This is an arg to builderInit.
  void setInterpolationCountExpr(Expr *expr) { interpolationCountExpr = expr; }
  Expr *getInterpolationCountExpr() const { return interpolationCountExpr; }

  /// Store a builtin integer literal expr wrapping getLiteralCapacity().
  /// This is an arg to builderInit.
  void setLiteralCapacityExpr(Expr *expr) { literalCapacityExpr = expr; }
  Expr *getLiteralCapacityExpr() const { return literalCapacityExpr; }

  /// Retrieve the value of the literalCapacity parameter to the
  /// initializer.
  unsigned getLiteralCapacity() const {
    return Bits.InterpolatedStringLiteralExpr.LiteralCapacity;
  }

  /// Retrieve the value of the interpolationCount parameter to the
  /// initializer.
  unsigned getInterpolationCount() const {
    return Bits.InterpolatedStringLiteralExpr.InterpolationCount;
  }

  /// A block containing expressions which call
  /// \c StringInterpolationProtocol methods to append segments to the
  /// string interpolation. The first node in \c Body should be an uninitialized
  /// \c VarDecl; the other statements should append to it.
  TapExpr * getAppendingExpr() const { return AppendingExpr; }
  void setAppendingExpr(TapExpr * AE) { AppendingExpr = AE; }
  
  SourceLoc getStartLoc() const {
    return Loc;
  }
  SourceLoc getEndLoc() const {
    // SourceLocs are token based, and the interpolated string is one string
    // token, so the range should be (Start == End).
    return Loc;
  }
  
  /// Call the \c callback with information about each segment in turn.
  void forEachSegment(ASTContext &Ctx,
                      toolchain::function_ref<void(bool, CallExpr *)> callback);
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::InterpolatedStringLiteral;
  }
};

/// The opaque kind of a RegexLiteralExpr feature, which should only be
/// interpreted by the compiler's regex parsing library.
class RegexLiteralPatternFeatureKind final {
  unsigned RawValue;

public:
  RegexLiteralPatternFeatureKind(unsigned rawValue) : RawValue(rawValue) {}

  unsigned getRawValue() const { return RawValue; }

  AvailabilityRange getAvailability(ASTContext &ctx) const;
  StringRef getDescription(ASTContext &ctx) const;

  friend toolchain::hash_code
  hash_value(const RegexLiteralPatternFeatureKind &kind) {
    return toolchain::hash_value(kind.getRawValue());
  }

  friend bool operator==(const RegexLiteralPatternFeatureKind &lhs,
                         const RegexLiteralPatternFeatureKind &rhs) {
    return lhs.getRawValue() == rhs.getRawValue();
  }

  friend bool operator!=(const RegexLiteralPatternFeatureKind &lhs,
                         const RegexLiteralPatternFeatureKind &rhs) {
    return !(lhs == rhs);
  }
};

/// A specific feature used in a RegexLiteralExpr, needed for availability
/// diagnostics.
class RegexLiteralPatternFeature final {
  RegexLiteralPatternFeatureKind Kind;
  CharSourceRange Range;

public:
  RegexLiteralPatternFeature(RegexLiteralPatternFeatureKind kind,
                             CharSourceRange range)
      : Kind(kind), Range(range) {}

  RegexLiteralPatternFeatureKind getKind() const { return Kind; }
  CharSourceRange getRange() const { return Range; }
};

/// A regular expression literal e.g '(a|c)*'.
class RegexLiteralExpr : public LiteralExpr {
  ASTContext *Ctx;
  SourceLoc Loc;
  StringRef ParsedRegexText;

  RegexLiteralExpr(ASTContext *ctx, SourceLoc loc, StringRef parsedRegexText,
                   bool isImplicit)
      : LiteralExpr(ExprKind::RegexLiteral, isImplicit), Ctx(ctx), Loc(loc),
        ParsedRegexText(parsedRegexText) {}

public:
  static RegexLiteralExpr *createParsed(ASTContext &ctx, SourceLoc loc,
                                        StringRef regexText);

  ASTContext &getASTContext() const { return *Ctx; }

  /// Retrieve the raw parsed regex text.
  StringRef getParsedRegexText() const { return ParsedRegexText; }

  /// Retrieve the regex pattern to emit.
  StringRef getRegexToEmit() const;

  /// Retrieve the computed type for the regex.
  Type getRegexType() const;

  /// Retrieve the version of the regex string.
  unsigned getVersion() const;

  /// Retrieve any features used in the regex pattern.
  ArrayRef<RegexLiteralPatternFeature> getPatternFeatures() const;

  SourceRange getSourceRange() const { return Loc; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::RegexLiteral;
  }
};

/// MagicIdentifierLiteralExpr - A magic identifier like #file which expands
/// out to a literal at SILGen time.
class MagicIdentifierLiteralExpr : public BuiltinLiteralExpr {
public:
  enum Kind : unsigned {
#define MAGIC_IDENTIFIER(NAME, STRING) NAME,
#include "language/AST/MagicIdentifierKinds.def"
  };

  static StringRef getKindString(MagicIdentifierLiteralExpr::Kind value) {
    switch (value) {
#define MAGIC_IDENTIFIER(NAME, STRING)                                         \
  case NAME:                                                                   \
    return STRING;
#include "language/AST/MagicIdentifierKinds.def"
    }

    toolchain_unreachable("Unhandled MagicIdentifierLiteralExpr in getKindString.");
  }

private:
  SourceLoc Loc;

public:
  MagicIdentifierLiteralExpr(Kind kind, SourceLoc loc, bool implicit = false)
      : BuiltinLiteralExpr(ExprKind::MagicIdentifierLiteral, implicit),
        Loc(loc) {
    Bits.MagicIdentifierLiteralExpr.Kind = static_cast<unsigned>(kind);
    Bits.MagicIdentifierLiteralExpr.StringEncoding
      = static_cast<unsigned>(StringLiteralExpr::UTF8);
  }

  Kind getKind() const {
    return static_cast<Kind>(Bits.MagicIdentifierLiteralExpr.Kind);
  }

  bool isString() const {
    switch (getKind()) {
#define MAGIC_STRING_IDENTIFIER(NAME, STRING)                                  \
    case NAME:                                                                 \
      return true;
#define MAGIC_IDENTIFIER(NAME, STRING)                                         \
    case NAME:                                                                 \
      return false;
#include "language/AST/MagicIdentifierKinds.def"
    }
    toolchain_unreachable("bad Kind");
  }

  SourceRange getSourceRange() const { return Loc; }

  // For a magic identifier that produces a string literal, retrieve the
  // encoding for that string literal.
  StringLiteralExpr::Encoding getStringEncoding() const {
    assert(isString() && "Magic identifier literal has non-string encoding");
    return static_cast<StringLiteralExpr::Encoding>(
             Bits.MagicIdentifierLiteralExpr.StringEncoding);
  }

  // For a magic identifier that produces a string literal, set the encoding
  // for the string literal.
  void setStringEncoding(StringLiteralExpr::Encoding encoding) {
    assert(isString() && "Magic identifier literal has non-string encoding");
    Bits.MagicIdentifierLiteralExpr.StringEncoding
      = static_cast<unsigned>(encoding);
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::MagicIdentifierLiteral;
  }
};

// ObjectLiteralExpr - An expression of the form
// '#colorLiteral(red: 1, blue: 0, green: 0, alpha: 1)' with a name and a list
// argument. The components of the list argument are meant to be themselves
// constant.
class ObjectLiteralExpr final : public LiteralExpr {
public:
  /// The kind of object literal.
  enum LiteralKind : unsigned {
#define POUND_OBJECT_LITERAL(Name, Desc, Proto) Name,
#include "language/AST/TokenKinds.def"
  };

  static StringRef
  getLiteralKindPlainName(ObjectLiteralExpr::LiteralKind kind);

private:
  ArgumentList *ArgList;
  SourceLoc PoundLoc;

  ObjectLiteralExpr(SourceLoc poundLoc, LiteralKind litKind, ArgumentList *args,
                    bool implicit);

public:
  /// Create a new object literal expression.
  static ObjectLiteralExpr *create(ASTContext &ctx, SourceLoc poundLoc,
                                   LiteralKind kind, ArgumentList *args,
                                   bool implicit);

  LiteralKind getLiteralKind() const {
    return static_cast<LiteralKind>(Bits.ObjectLiteralExpr.LitKind);
  }

  ArgumentList *getArgs() const { return ArgList; }
  void setArgs(ArgumentList *newArgs) { ArgList = newArgs; }

  SourceLoc getSourceLoc() const { return PoundLoc; }
  SourceRange getSourceRange() const {
    return SourceRange(PoundLoc, ArgList->getEndLoc());
  }

  /// Return the string form of the literal name.
  StringRef getLiteralKindRawName() const;

  StringRef getLiteralKindPlainName() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ObjectLiteral;
  }
};

/// DiscardAssignmentExpr - A '_' in the left-hand side of an assignment, which
/// discards the corresponding tuple element on the right-hand side.
class DiscardAssignmentExpr : public Expr {
  SourceLoc Loc;

public:
  DiscardAssignmentExpr(SourceLoc Loc, bool Implicit)
    : Expr(ExprKind::DiscardAssignment, Implicit), Loc(Loc) {}
  
  SourceRange getSourceRange() const { return Loc; }
  SourceLoc getLoc() const { return Loc; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DiscardAssignment;
  }
};

/// DeclRefExpr - A reference to a value, "x".
class DeclRefExpr : public Expr {
  /// The declaration pointer.
  ConcreteDeclRef D;
  DeclNameLoc Loc;
  ActorIsolation implicitActorHopTarget;

  /// Destination information for a thrown error, which includes any
  /// necessary conversions from the actual type thrown to the type that
  /// is expected by the enclosing context.
  ThrownErrorDestination ThrowDest;

public:
  DeclRefExpr(ConcreteDeclRef D, DeclNameLoc Loc, bool Implicit,
              AccessSemantics semantics = AccessSemantics::Ordinary,
              Type Ty = Type())
    : Expr(ExprKind::DeclRef, Implicit, Ty), D(D), Loc(Loc),
      implicitActorHopTarget(ActorIsolation::forUnspecified()) {
    Bits.DeclRefExpr.Semantics = (unsigned)semantics;
    Bits.DeclRefExpr.IsImplicitlyAsync = false;
    Bits.DeclRefExpr.IsImplicitlyThrows = false;
    setFunctionRefInfo(FunctionRefInfo::unapplied(Loc));
  }

  /// Retrieve the declaration to which this expression refers.
  ValueDecl *getDecl() const {
    return getDeclRef().getDecl();
  }

  /// Return true if this access is direct, meaning that it does not call the
  /// getter or setter.
  AccessSemantics getAccessSemantics() const {
    return (AccessSemantics) Bits.DeclRefExpr.Semantics;
  }

  /// Determine whether this reference needs to happen asynchronously, i.e.,
  /// guarded by hop_to_executor, and if so describe the target.
  std::optional<ActorIsolation> isImplicitlyAsync() const {
    if (!Bits.DeclRefExpr.IsImplicitlyAsync)
      return std::nullopt;

    return implicitActorHopTarget;
  }

  /// Note that this reference is implicitly async and set the target.
  void setImplicitlyAsync(ActorIsolation target) {
    Bits.DeclRefExpr.IsImplicitlyAsync = true;
    implicitActorHopTarget = target;
  }

  /// Determine whether this reference needs may implicitly throw.
  ///
  /// This is the case for non-throwing `distributed fn` declarations,
  /// which are cross-actor invoked, because such calls actually go over the
  /// transport/network, and may throw from this, rather than the function
  /// implementation itself..
  bool isImplicitlyThrows() const {
    return Bits.DeclRefExpr.IsImplicitlyThrows;
  }

  /// The error thrown from this access.
  ThrownErrorDestination throws() const { return ThrowDest; }

  void setThrows(ThrownErrorDestination throws) {
    assert(!ThrowDest);
    ThrowDest = throws;
  }

  /// Set whether this reference must account for a `throw` occurring for reasons
  /// other than the function implementation itself throwing, e.g. an
  /// `DistributedActorSystem` implementing a `distributed fn` call throwing a
  /// networking error.
  void setImplicitlyThrows(bool isImplicitlyThrows) {
    Bits.DeclRefExpr.IsImplicitlyThrows = isImplicitlyThrows;
  }

  /// Retrieve the concrete declaration reference.
  ConcreteDeclRef getDeclRef() const {
    return D;
  }

  SourceRange getSourceRange() const { return Loc.getSourceRange(); }
  SourceLoc getLoc() const { return Loc.getBaseNameLoc(); }
  DeclNameLoc getNameLoc() const { return Loc; }

  /// Retrieve the kind of function reference.
  FunctionRefInfo getFunctionRefInfo() const {
    return FunctionRefInfo::fromOpaque(Bits.DeclRefExpr.FunctionRefInfo);
  }

  /// Set the kind of function reference.
  void setFunctionRefInfo(FunctionRefInfo refKind) {
    Bits.DeclRefExpr.FunctionRefInfo = refKind.getOpaqueValue();
    ASSERT(refKind == getFunctionRefInfo() && "FunctionRefInfo truncated");
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DeclRef;
  }
};
  
/// A reference to 'super'. References to members of 'super' resolve to members
/// of a superclass of 'self'.
class SuperRefExpr : public Expr {
  VarDecl *Self;
  SourceLoc Loc;
  
public:
  SuperRefExpr(VarDecl *Self, SourceLoc Loc, bool Implicit,
               Type SuperTy = Type())
    : Expr(ExprKind::SuperRef, Implicit, SuperTy), Self(Self), Loc(Loc) {}
  
  VarDecl *getSelf() const { return Self; }
  void setSelf(VarDecl *self) { Self = self; }
  
  SourceLoc getSuperLoc() const { return Loc; }
  SourceRange getSourceRange() const { return Loc; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::SuperRef;
  }
};

/// A reference to a type in expression context.
///
/// The type of this expression is always \c MetatypeType.
class TypeExpr : public Expr {
  TypeRepr *Repr;
public:
  /// Create a \c TypeExpr from a parsed \c TypeRepr.
  TypeExpr(TypeRepr *Ty);

  /// Retrieves the corresponding instance type of the type referenced by this
  /// expression.
  ///
  /// If this node has no type, the resulting instance type is also the
  /// null \c Type(). If the type of this node is not a \c MetatypeType, the
  /// resulting instance type is \c ErrorType.
  Type getInstanceType() const;

public:
  /// Create an implicit \c TypeExpr.
  ///
  /// The given type is required to be non-null and must be not be
  /// a \c MetatypeType as this function will wrap the given type in one.
  ///
  /// FIXME: This behavior is bizarre.
  static TypeExpr *createImplicit(Type Ty, ASTContext &C);

  /// Create an implicit \c TypeExpr that has artificial
  /// location information attached.
  ///
  /// The given type is required to be non-null and must be not be
  /// a \c MetatypeType as this function will wrap the given type in one.
  ///
  /// FIXME: This behavior is bizarre.
  ///
  /// Due to limitations in the modeling of certain AST elements, implicit
  /// \c TypeExpr nodes are often the only source of location information the
  /// expression checker has when it comes time to diagnose an error.
  static TypeExpr *createImplicitHack(SourceLoc Loc, Type Ty, ASTContext &C);

  /// Create an implicit \c TypeExpr for a given \c TypeDecl at the specified location.
  ///
  /// The given type is required to be non-null and must be not be
  /// a \c MetatypeType as this function will wrap the given type in one.
  ///
  /// FIXME: This behavior is bizarre.
  ///
  /// Unlike the non-implicit case, the given location is not required to be
  /// valid.
  static TypeExpr *createImplicitForDecl(DeclNameLoc Loc, TypeDecl *D,
                                         DeclContext *DC, Type ty);

public:
  /// Create a \c TypeExpr for a given \c TypeDecl at the specified location.
  ///
  /// The given location must be valid. If it is not, you must use
  /// \c TypeExpr::createImplicitForDecl instead.
  static TypeExpr *createForDecl(DeclNameLoc Loc, TypeDecl *D, DeclContext *DC);

  /// Create a TypeExpr for a member TypeDecl of the given parent TypeDecl.
  static TypeExpr *createForMemberDecl(DeclNameLoc ParentNameLoc,
                                       TypeDecl *Parent,
                                       DeclNameLoc NameLoc,
                                       TypeDecl *Decl);

  /// Create a \c TypeExpr for a member \c TypeDecl of the given parent
  /// \c TypeRepr.
  static TypeExpr *createForMemberDecl(TypeRepr *ParentTR, DeclNameLoc NameLoc,
                                       TypeDecl *Decl);

  /// Create a \c TypeExpr from an \c DeclRefTypeRepr with the given arguments
  /// applied at the specified location.
  ///
  /// Returns nullptr if the reference cannot be formed, which is a hack due
  /// to limitations in how we model generic typealiases.
  static TypeExpr *createForSpecializedDecl(DeclRefTypeRepr *ParentTR,
                                            ArrayRef<TypeRepr *> Args,
                                            SourceRange AngleLocs,
                                            ASTContext &C);

  TypeRepr *getTypeRepr() const { return Repr; }
  // NOTE: TypeExpr::getType() returns the type of the expr node, which is the
  // metatype of what is stored as an operand type.
  
  SourceRange getSourceRange() const;
  // TODO: optimize getStartLoc() and getEndLoc() when TypeLoc allows it.

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Type;
  }
};

class TypeValueExpr : public Expr {
  TypeRepr *repr;
  Type paramType;

  /// Create a \c TypeValueExpr from a given type representation.
  TypeValueExpr(TypeRepr *repr) :
      Expr(ExprKind::TypeValue, /*implicit*/ false), repr(repr),
      paramType(nullptr) {}

public:
  /// Create a \c TypeValueExpr for a given \c TypeDecl.
  ///
  /// The given location must be valid.
  static TypeValueExpr *createForDecl(DeclNameLoc loc, TypeDecl *d,
                                      DeclContext *dc);

  GenericTypeParamDecl *getParamDecl() const;

  TypeRepr *getRepr() const {
    return repr;
  }

  /// Retrieves the corresponding parameter type of the value referenced by this
  /// expression.
  Type getParamType() const {
    return paramType;
  }

  /// Sets the corresponding parameter type of the value referenced by this
  /// expression.
  void setParamType(Type paramType) {
    this->paramType = paramType;
  }

  SourceRange getSourceRange() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::TypeValue;
  }
};

/// A reference to another initializer from within a constructor body,
/// either to a delegating initializer or to a super.init invocation.
/// For a reference type, this semantically references a different constructor
/// entry point, called the 'initializing constructor', from the 'allocating
/// constructor' entry point referenced by a 'new' expression.
class OtherConstructorDeclRefExpr : public Expr {
  ConcreteDeclRef Ctor;
  DeclNameLoc Loc;
  
public:
  OtherConstructorDeclRefExpr(ConcreteDeclRef Ctor, DeclNameLoc Loc,
                              bool Implicit, Type Ty = {})
    : Expr(ExprKind::OtherConstructorDeclRef, Implicit, Ty),
      Ctor(Ctor), Loc(Loc)
  {}
  
  ConstructorDecl *getDecl() const;
  ConcreteDeclRef getDeclRef() const { return Ctor; }

  SourceLoc getLoc() const { return Loc.getBaseNameLoc(); }
  DeclNameLoc getConstructorLoc() const { return Loc; }
  SourceRange getSourceRange() const { return Loc.getSourceRange(); }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::OtherConstructorDeclRef;
  }
};

/// OverloadSetRefExpr - A reference to an overloaded set of values with a
/// single name.
///
/// This is an abstract class that covers the various different kinds of
/// overload sets.
class OverloadSetRefExpr : public Expr {
  ArrayRef<ValueDecl*> Decls;

protected:
  OverloadSetRefExpr(ExprKind Kind, ArrayRef<ValueDecl*> decls,
                     FunctionRefInfo functionRefInfo, bool Implicit, Type Ty)
      : Expr(Kind, Implicit, Ty), Decls(decls) {
    setFunctionRefInfo(functionRefInfo);
  }

public:
  ArrayRef<ValueDecl*> getDecls() const { return Decls; }

  void setDecls(ArrayRef<ValueDecl *> domain) {
    Decls = domain;
  }

  /// getBaseType - Determine the type of the base object provided for the
  /// given overload set, which is only non-null when dealing with an overloaded
  /// member reference.
  Type getBaseType() const;

  /// hasBaseObject - Determine whether this overloaded expression has a
  /// concrete base object (which is not a metatype).
  bool hasBaseObject() const;

  /// Retrieve the kind of function reference.
  FunctionRefInfo getFunctionRefInfo() const {
    return FunctionRefInfo::fromOpaque(Bits.OverloadSetRefExpr.FunctionRefInfo);
  }

  /// Set the kind of function reference.
  void setFunctionRefInfo(FunctionRefInfo refKind) {
    Bits.OverloadSetRefExpr.FunctionRefInfo = refKind.getOpaqueValue();
    ASSERT(refKind == getFunctionRefInfo() && "FunctionRefInfo truncated");
  }

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_OverloadSetRefExpr &&
           E->getKind() <= ExprKind::Last_OverloadSetRefExpr;
  }
};

/// OverloadedDeclRefExpr - A reference to an overloaded name that should
/// eventually be resolved (by overload resolution) to a value reference.
class OverloadedDeclRefExpr final : public OverloadSetRefExpr {
  DeclNameLoc Loc;

public:
  OverloadedDeclRefExpr(ArrayRef<ValueDecl*> Decls, DeclNameLoc Loc,
                        FunctionRefInfo functionRefInfo,
                        bool Implicit, Type Ty = Type())
      : OverloadSetRefExpr(ExprKind::OverloadedDeclRef, Decls, functionRefInfo,
                           Implicit, Ty),
        Loc(Loc) {
  }
  
  DeclNameLoc getNameLoc() const { return Loc; }
  SourceLoc getLoc() const { return Loc.getBaseNameLoc(); }
  SourceRange getSourceRange() const { return Loc.getSourceRange(); }

  bool isForOperator() const { return getDecls().front()->isOperator(); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::OverloadedDeclRef;
  }
};

/// UnresolvedDeclRefExpr - This represents use of an undeclared identifier,
/// which may ultimately be a use of something that hasn't been defined yet, it
/// may be a use of something that got imported (which will be resolved during
/// sema), or may just be a use of an unknown identifier.
///
class UnresolvedDeclRefExpr : public Expr {
  DeclNameRef Name;
  DeclNameLoc Loc;

public:
  UnresolvedDeclRefExpr(DeclNameRef name, DeclRefKind refKind, DeclNameLoc loc)
      : Expr(ExprKind::UnresolvedDeclRef, /*Implicit=*/loc.isInvalid()),
        Name(name), Loc(loc) {
    Bits.UnresolvedDeclRefExpr.DeclRefKind = static_cast<unsigned>(refKind);
    setFunctionRefInfo(FunctionRefInfo::unapplied(loc));
  }

  static UnresolvedDeclRefExpr *createImplicit(
      ASTContext &C, DeclName name,
      DeclRefKind refKind = DeclRefKind::Ordinary) {
    return new (C) UnresolvedDeclRefExpr(DeclNameRef(name), refKind,
                                         DeclNameLoc());
  }

  static UnresolvedDeclRefExpr *createImplicit(
      ASTContext &C, DeclBaseName baseName, ArrayRef<Identifier> argLabels) {
    return UnresolvedDeclRefExpr::createImplicit(C,
        DeclName(C, baseName, argLabels));
  }

  static UnresolvedDeclRefExpr *createImplicit(
      ASTContext &C, DeclBaseName baseName, ParameterList *paramList) {
    return UnresolvedDeclRefExpr::createImplicit(C,
        DeclName(C, baseName, paramList));
  }

  bool hasName() const { return static_cast<bool>(Name); }
  DeclNameRef getName() const { return Name; }

  DeclRefKind getRefKind() const {
    return static_cast<DeclRefKind>(Bits.UnresolvedDeclRefExpr.DeclRefKind);
  }

  /// Retrieve the kind of function reference.
  FunctionRefInfo getFunctionRefInfo() const {
    return FunctionRefInfo::fromOpaque(
        Bits.UnresolvedDeclRefExpr.FunctionRefInfo);
  }

  /// Set the kind of function reference.
  void setFunctionRefInfo(FunctionRefInfo refKind) {
    Bits.UnresolvedDeclRefExpr.FunctionRefInfo = refKind.getOpaqueValue();
    ASSERT(refKind == getFunctionRefInfo() && "FunctionRefInfo truncated");
  }

  DeclNameLoc getNameLoc() const { return Loc; }

  SourceRange getSourceRange() const { return Loc.getSourceRange(); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnresolvedDeclRef;
  }
};

/// LookupExpr - This abstract class represents 'a.b', 'a[]', etc where we
/// are referring to a member of a type, such as a property, variable, etc.
class LookupExpr : public Expr {
  Expr *Base;
  ConcreteDeclRef Member;
  ActorIsolation implicitActorHopTarget;

protected:
  explicit LookupExpr(ExprKind Kind, Expr *base, ConcreteDeclRef member,
                          bool Implicit)
      : Expr(Kind, Implicit), Base(base), Member(member),
        implicitActorHopTarget(ActorIsolation::forUnspecified()) {
    Bits.LookupExpr.IsSuper = false;
    Bits.LookupExpr.IsImplicitlyAsync = false;
    Bits.LookupExpr.IsImplicitlyThrows = false;
    assert(Base);
  }

  /// Destination information for a thrown error, which includes any
  /// necessary conversions from the actual type thrown to the type that
  /// is expected by the enclosing context.
  ThrownErrorDestination ThrowDest;

public:
  /// Retrieve the base of the expression.
  Expr *getBase() const { return Base; }

  /// Replace the base of the expression.
  void setBase(Expr *E) { Base = E; }

  /// Retrieve the member to which this access refers.
  ConcreteDeclRef getMember() const { return Member; }

  /// Determine whether the operation has a known underlying declaration or not.
  bool hasDecl() const { return static_cast<bool>(Member); }

  /// Retrieve the declaration that this /// operation refers to.
  /// Only valid when \c hasDecl() is true.
  ConcreteDeclRef getDecl() const {
    assert(hasDecl() && "No subscript declaration known!");
    return getMember();
  }

  /// Determine whether this reference refers to the superclass's property.
  bool isSuper() const { return Bits.LookupExpr.IsSuper; }

  /// Set whether this reference refers to the superclass's property.
  void setIsSuper(bool isSuper) { Bits.LookupExpr.IsSuper = isSuper; }

  /// Determine whether this reference needs to happen asynchronously, i.e.,
  /// guarded by hop_to_executor, and if so describe the target.
  std::optional<ActorIsolation> isImplicitlyAsync() const {
    if (!Bits.LookupExpr.IsImplicitlyAsync)
      return std::nullopt;

    return implicitActorHopTarget;
  }

  /// Note that this reference is implicitly async and set the target.
  void setImplicitlyAsync(ActorIsolation target) {
    Bits.LookupExpr.IsImplicitlyAsync = true;
    implicitActorHopTarget = target;
  }

  /// The error thrown from this access.
  ThrownErrorDestination throws() const { return ThrowDest; }

  void setThrows(ThrownErrorDestination throws) {
    assert(!ThrowDest);
    ThrowDest = throws;
  }

  /// Determine whether this reference needs may implicitly throw.
  ///
  /// This is the case for non-throwing `distributed fn` declarations,
  /// which are cross-actor invoked, because such calls actually go over the
  /// transport/network, and may throw from this, rather than the function
  /// implementation itself..
  bool isImplicitlyThrows() const { return Bits.LookupExpr.IsImplicitlyThrows; }

  /// Set whether this reference must account for a `throw` occurring for reasons
  /// other than the function implementation itself throwing, e.g. an
  /// `DistributedActorSystem` implementing a `distributed fn` call throwing a
  /// networking error.
  void setImplicitlyThrows(bool isImplicitlyThrows) {
    Bits.LookupExpr.IsImplicitlyThrows = isImplicitlyThrows;
  }

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_LookupExpr &&
           E->getKind() <= ExprKind::Last_LookupExpr;
  }
};

/// MemberRefExpr - This represents 'a.b' where we are referring to a member
/// of a type, such as a property or variable.
///
/// Note that methods found via 'dot' syntax are expressed as DotSyntaxCallExpr
/// nodes, because 'a.f' is actually an application of 'a' (the implicit object
/// argument) to the function 'f'.
class MemberRefExpr : public LookupExpr {
  SourceLoc DotLoc;
  DeclNameLoc NameLoc;
  
public:
  MemberRefExpr(Expr *base, SourceLoc dotLoc, ConcreteDeclRef member,
                DeclNameLoc loc, bool Implicit,
                AccessSemantics semantics = AccessSemantics::Ordinary);
  SourceLoc getDotLoc() const { return DotLoc; }
  DeclNameLoc getNameLoc() const { return NameLoc; }
  
  /// Return true if this member access is direct, meaning that it
  /// does not call the getter or setter.
  AccessSemantics getAccessSemantics() const {
    return (AccessSemantics) Bits.MemberRefExpr.Semantics;
  }

  /// Informs IRGen to that this member should be applied via its distributed
  /// thunk, rather than invoking it directly.
  ///
  /// Only intended to be set on distributed get-only computed properties.
  void setAccessViaDistributedThunk() {
    Bits.MemberRefExpr.Semantics = (unsigned)AccessSemantics::DistributedThunk;
  }

  SourceLoc getLoc() const { return NameLoc.getBaseNameLoc(); }
  SourceLoc getStartLoc() const {
    SourceLoc BaseStartLoc = getBase()->getStartLoc();
    if (BaseStartLoc.isInvalid() || NameLoc.isInvalid()) {
      return NameLoc.getBaseNameLoc();
    } else {
      return BaseStartLoc;
    }
  }
  SourceLoc getEndLoc() const {
    return NameLoc.getSourceRange().End;
  }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::MemberRef;
  }
};
  
/// Common base for expressions that involve dynamic lookup, which
/// determines at runtime whether a particular method, property, or
/// subscript is available.
class DynamicLookupExpr : public LookupExpr {
protected:
  explicit DynamicLookupExpr(ExprKind kind, ConcreteDeclRef member, Expr *base)
    : LookupExpr(kind, base, member, /*Implicit=*/false) { }

public:
  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_DynamicLookupExpr &&
           E->getKind() <= ExprKind::Last_DynamicLookupExpr;
  }
};

/// A reference to a member of an object that was found via dynamic lookup.
///
/// A member found via dynamic lookup may not actually be available at runtime.
/// Therefore, a reference to that member always returns an optional instance.
/// Users can then propagate the optional (via ?) or assert that the member is
/// always available (via !). For example:
///
/// \code
/// class C {
///   fn @objc foo(i : Int) -> String { ... }
/// };
///
/// var x : AnyObject = <some value>
/// print(x.foo!(17)) // x.foo has type ((i : Int) -> String)?
/// \endcode
class DynamicMemberRefExpr : public DynamicLookupExpr {
  SourceLoc DotLoc;
  DeclNameLoc NameLoc;

public:
  DynamicMemberRefExpr(Expr *base, SourceLoc dotLoc,
                       ConcreteDeclRef member,
                       DeclNameLoc nameLoc)
    : DynamicLookupExpr(ExprKind::DynamicMemberRef, member, base),
      DotLoc(dotLoc), NameLoc(nameLoc) {
    }

  /// Retrieve the location of the member name.
  DeclNameLoc getNameLoc() const { return NameLoc; }

  /// Retrieve the location of the '.'.
  SourceLoc getDotLoc() const { return DotLoc; }

  SourceLoc getLoc() const { return NameLoc.getBaseNameLoc(); }

  SourceLoc getStartLoc() const {
    SourceLoc BaseStartLoc = getBase()->getStartLoc();
    if (BaseStartLoc.isInvalid() || NameLoc.isInvalid()) {
      return NameLoc.getBaseNameLoc();
    } else {
      return BaseStartLoc;
    }
  }
  SourceLoc getEndLoc() const { return NameLoc.getSourceRange().End; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DynamicMemberRef;
  }
};

/// A subscript on an object with dynamic lookup type.
///
/// A subscript found via dynamic lookup may not actually be available
/// at runtime.  Therefore, the result of performing the subscript
/// operation always returns an optional instance.Users can then
/// propagate the optional (via ?) or assert that the member is always
/// available (via !). For example:
///
/// \code
/// class C {
///   @objc subscript (i : Int) -> String {
///     get {
///       ...
///     }
///   }
/// };
///
/// var x : AnyObject = <some value>
/// print(x[27]! // x[27] has type String?
/// \endcode
class DynamicSubscriptExpr final : public DynamicLookupExpr {
  ArgumentList *ArgList;

  DynamicSubscriptExpr(Expr *base, ArgumentList *argList,
                       ConcreteDeclRef member, bool implicit);

public:
  static DynamicSubscriptExpr *create(ASTContext &ctx, Expr *base,
                                      ArgumentList *argList,
                                      ConcreteDeclRef member, bool implicit);

  ArgumentList *getArgs() const { return ArgList; }
  void setArgs(ArgumentList *newArgs) { ArgList = newArgs; }

  SourceLoc getLoc() const { return getArgs()->getStartLoc(); }

  SourceLoc getStartLoc() const { return getBase()->getStartLoc(); }
  SourceLoc getEndLoc() const { return getArgs()->getEndLoc(); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DynamicSubscript;
  }
};

/// UnresolvedMemberExpr - This represents '.foo', an unresolved reference to a
/// member, which is to be resolved with context sensitive type information into
/// bar.foo.  These always have unresolved type.
class UnresolvedMemberExpr final
    : public Expr {
  SourceLoc DotLoc;
  DeclNameLoc NameLoc;
  DeclNameRef Name;

public:
  UnresolvedMemberExpr(SourceLoc dotLoc, DeclNameLoc nameLoc, DeclNameRef name,
                       bool implicit)
    : Expr(ExprKind::UnresolvedMember, implicit), DotLoc(dotLoc),
      NameLoc(nameLoc), Name(name) {
    setFunctionRefInfo(FunctionRefInfo::unapplied(nameLoc));
  }

  DeclNameRef getName() const { return Name; }
  DeclNameLoc getNameLoc() const { return NameLoc; }
  SourceLoc getDotLoc() const { return DotLoc; }

  SourceLoc getLoc() const { return NameLoc.getBaseNameLoc(); }

  SourceLoc getStartLoc() const { return DotLoc; }
  SourceLoc getEndLoc() const { return NameLoc.getSourceRange().End; }

  /// Retrieve the kind of function reference.
  FunctionRefInfo getFunctionRefInfo() const {
    return FunctionRefInfo::fromOpaque(
        Bits.UnresolvedMemberExpr.FunctionRefInfo);
  }

  /// Set the kind of function reference.
  void setFunctionRefInfo(FunctionRefInfo refKind) {
    Bits.UnresolvedMemberExpr.FunctionRefInfo = refKind.getOpaqueValue();
    ASSERT(refKind == getFunctionRefInfo() && "FunctionRefInfo truncated");
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnresolvedMember;
  }
};

/// AnyTryExpr - An abstract superclass for 'try' and 'try!'.
///
/// These are like IdentityExpr in some ways, but they're a bit too
/// semantic differentiated to just always look through.
class AnyTryExpr : public Expr {
  Expr *SubExpr;
  SourceLoc TryLoc;

public:
  AnyTryExpr(ExprKind kind, SourceLoc tryLoc, Expr *sub,
             Type type, bool implicit)
    : Expr(kind, implicit, type), SubExpr(sub), TryLoc(tryLoc) {}

  SourceLoc getLoc() const { return SubExpr->getLoc(); }
  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *E) { SubExpr = E; }

  SourceLoc getTryLoc() const { return TryLoc; }

  SourceLoc getStartLoc() const { return TryLoc; }
  SourceLoc getEndLoc() const { return getSubExpr()->getEndLoc(); }

  static bool classof(const Expr *e) {
    return e->getKind() >= ExprKind::First_AnyTryExpr
        && e->getKind() <= ExprKind::Last_AnyTryExpr;
  }
};

/// TryExpr - A 'try' surrounding an expression, marking that the
/// expression contains code which might throw.
///
/// getSemanticsProvidingExpr() looks through this because it doesn't
/// provide the value and only very specific clients care where the
/// 'try' was written.
class TryExpr : public AnyTryExpr {
public:
  TryExpr(SourceLoc tryLoc, Expr *sub, Type type = Type(),
          bool implicit = false)
    : AnyTryExpr(ExprKind::Try, tryLoc, sub, type, implicit) {}

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::Try;
  }

  static TryExpr *createImplicit(ASTContext &ctx, SourceLoc tryLoc, Expr *sub,
                                 Type type = Type()) {
    return new (ctx) TryExpr(tryLoc, sub, type, /*implicit=*/true);
  }
};

/// ForceTryExpr - A 'try!' surrounding an expression, marking that
/// the expression contains code which might throw, but that the code
/// should dynamically assert if it does.
class ForceTryExpr final : public AnyTryExpr {
  SourceLoc ExclaimLoc;
  Type thrownError;

public:
  ForceTryExpr(SourceLoc tryLoc, Expr *sub, SourceLoc exclaimLoc,
               Type type = Type(), bool implicit = false)
    : AnyTryExpr(ExprKind::ForceTry, tryLoc, sub, type, implicit),
      ExclaimLoc(exclaimLoc) {}

  SourceLoc getExclaimLoc() const { return ExclaimLoc; }

  /// Retrieve the type of the error thrown from the subexpression.
  Type getThrownError() const { return thrownError; }

  /// Set the type of the error thrown from the subexpression.
  void setThrownError(Type type) {
    assert(!thrownError || thrownError->isEqual(type));
    thrownError = type;
  }

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::ForceTry;
  }
};

/// A 'try?' surrounding an expression, marking that the expression contains
/// code which might throw, and that the result should be injected into an
/// Optional. If the code does throw, \c nil is produced.
class OptionalTryExpr final : public AnyTryExpr {
  SourceLoc QuestionLoc;
  Type thrownError;

public:
  OptionalTryExpr(SourceLoc tryLoc, Expr *sub, SourceLoc questionLoc,
                  Type type = Type(), bool implicit = false)
    : AnyTryExpr(ExprKind::OptionalTry, tryLoc, sub, type, implicit),
      QuestionLoc(questionLoc) {}

  SourceLoc getQuestionLoc() const { return QuestionLoc; }

  /// Retrieve the type of the error thrown from the subexpression.
  Type getThrownError() const { return thrownError; }

  /// Set the type of the error thrown from the subexpression.
  void setThrownError(Type type) {
    assert(!thrownError || thrownError->isEqual(type));
    thrownError = type;
  }

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::OptionalTry;
  }
};

/// An expression node that does not affect the evaluation of its subexpression.
class IdentityExpr : public Expr {
  Expr *SubExpr;
public:
  IdentityExpr(ExprKind kind,
               Expr *subExpr, Type ty = Type(),
               bool implicit = false)
    : Expr(kind, implicit, ty), SubExpr(subExpr)
  {}
  
  SourceLoc getLoc() const { return SubExpr->getLoc(); }
  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *E) { SubExpr = E; }
  
  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_IdentityExpr
        && E->getKind() <= ExprKind::Last_IdentityExpr;
  }
};
  
/// The '.self' pseudo-property, which has no effect except to
/// satisfy the syntactic requirement that type values appear only as part of
/// a property chain.
class DotSelfExpr : public IdentityExpr {
  SourceLoc DotLoc;
  SourceLoc SelfLoc;
  
public:
  DotSelfExpr(Expr *subExpr, SourceLoc dot, SourceLoc self,
              Type ty = Type())
    : IdentityExpr(ExprKind::DotSelf, subExpr, ty),
      DotLoc(dot), SelfLoc(self)
  {}
  
  SourceLoc getDotLoc() const { return DotLoc; }
  SourceLoc getSelfLoc() const { return SelfLoc; }

  SourceLoc getStartLoc() const { return getSubExpr()->getStartLoc(); }
  SourceLoc getEndLoc() const { return SelfLoc; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DotSelf;
  }
};
  
/// A parenthesized expression like '(x+x)'.  Syntactically,
/// this is just a TupleExpr with exactly one element that has no label.
/// Semantically, however, it serves only as grouping parentheses and
/// does not form an expression of tuple type (unless the sub-expression
/// has tuple type, of course).
class ParenExpr : public IdentityExpr {
  SourceLoc LParenLoc, RParenLoc;
  
public:
  ParenExpr(SourceLoc lploc, Expr *subExpr, SourceLoc rploc, Type ty = Type())
      : IdentityExpr(ExprKind::Paren, subExpr, ty), LParenLoc(lploc),
        RParenLoc(rploc) {
    assert(lploc.isValid() == rploc.isValid() &&
           "Mismatched source location information");
  }

  SourceLoc getLParenLoc() const { return LParenLoc; }
  SourceLoc getRParenLoc() const { return RParenLoc; }

  // When the locations of the parens are invalid, ask our
  // subexpression for its source range instead.  This isn't a
  // hot path and so we don't both optimizing for it.

  SourceLoc getStartLoc() const {
    return (LParenLoc.isInvalid() ? getSubExpr()->getStartLoc() : LParenLoc);
  }
  SourceLoc getEndLoc() const {
    if (RParenLoc.isInvalid())
      return getSubExpr()->getEndLoc();
    return RParenLoc;
  }

  static bool classof(const Expr *E) { return E->getKind() == ExprKind::Paren; }
};

/// Represents the result of a chain of accesses or calls hanging off of an
/// \c UnresolvedMemberExpr at the root. This is only used during type checking
/// to give the result type of such a chain representation in the AST. This
/// expression type is always implicit.
class UnresolvedMemberChainResultExpr : public IdentityExpr {
  /// The base of this chain of member accesses.
  UnresolvedMemberExpr *ChainBase;
public:
  UnresolvedMemberChainResultExpr(Expr *subExpr, UnresolvedMemberExpr *base,
                                  Type ty = Type())
    : IdentityExpr(ExprKind::UnresolvedMemberChainResult, subExpr, ty,
                   /*isImplicit=*/true),
      ChainBase(base) {
    assert(base);
  }

  UnresolvedMemberExpr *getChainBase() const { return ChainBase; }

  LANGUAGE_FORWARD_SOURCE_LOCS_TO(getSubExpr())

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnresolvedMemberChainResult;
  }
};
  
/// AwaitExpr - An 'await' surrounding an expression, marking that the
/// expression contains code which is a coroutine that may block.
///
/// getSemanticsProvidingExpr() looks through this because it doesn't
/// provide the value and only very specific clients care where the
/// 'await' was written.
class AwaitExpr final : public IdentityExpr {
  SourceLoc AwaitLoc;
public:
  AwaitExpr(SourceLoc awaitLoc, Expr *sub, Type type = Type(),
            bool implicit = false)
    : IdentityExpr(ExprKind::Await, sub, type, implicit), AwaitLoc(awaitLoc) {
  }

  static AwaitExpr *createImplicit(ASTContext &ctx, SourceLoc awaitLoc, Expr *sub, Type type = Type()) {
    return new (ctx) AwaitExpr(awaitLoc, sub, type, /*implicit=*/true);
  }

  SourceLoc getLoc() const { return AwaitLoc; }
  
  SourceLoc getAwaitLoc() const { return AwaitLoc; }
  SourceLoc getStartLoc() const { return AwaitLoc; }
  SourceLoc getEndLoc() const { return getSubExpr()->getEndLoc(); }

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::Await;
  }
};

/// UnsafeExpr - An 'unsafe' surrounding an expression, marking that the
/// expression contains uses of unsafe declarations.
///
/// getSemanticsProvidingExpr() looks through this because it doesn't
/// provide the value and only very specific clients care where the
/// 'unsafe' was written.
class UnsafeExpr final : public IdentityExpr {
  SourceLoc UnsafeLoc;
public:
  UnsafeExpr(SourceLoc unsafeLoc, Expr *sub, Type type = Type(),
            bool implicit = false)
    : IdentityExpr(ExprKind::Unsafe, sub, type, implicit),
      UnsafeLoc(unsafeLoc) {
  }

  static UnsafeExpr *createImplicit(ASTContext &ctx, SourceLoc unsafeLoc, Expr *sub, Type type = Type()) {
    return new (ctx) UnsafeExpr(unsafeLoc, sub, type, /*implicit=*/true);
  }

  SourceLoc getLoc() const { return UnsafeLoc; }

  SourceLoc getUnsafeLoc() const { return UnsafeLoc; }
  SourceLoc getStartLoc() const { return UnsafeLoc; }
  SourceLoc getEndLoc() const { return getSubExpr()->getEndLoc(); }

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::Unsafe;
  }
};

/// ConsumeExpr - A 'consume' surrounding an lvalue expression marking the
/// lvalue as needing to be moved.
class ConsumeExpr final : public Expr {
  Expr *SubExpr;
  SourceLoc ConsumeLoc;

public:
  ConsumeExpr(SourceLoc consumeLoc, Expr *sub, Type type = Type(),
              bool implicit = false)
      : Expr(ExprKind::Consume, implicit, type), SubExpr(sub),
        ConsumeLoc(consumeLoc) {}

  static ConsumeExpr *createImplicit(ASTContext &ctx, SourceLoc moveLoc,
                                     Expr *sub, Type type = Type()) {
    return new (ctx) ConsumeExpr(moveLoc, sub, type, /*implicit=*/true);
  }

  SourceLoc getLoc() const { return ConsumeLoc; }

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *E) { SubExpr = E; }

  SourceLoc getStartLoc() const { return getLoc(); }
  SourceLoc getEndLoc() const { return getSubExpr()->getEndLoc(); }

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::Consume;
  }
};

/// CopyExpr - A 'copy' surrounding an lvalue expression marking the lvalue as
/// needing a semantic copy. Used to force a copy of a no implicit copy type.
class CopyExpr final : public Expr {
  Expr *SubExpr;
  SourceLoc CopyLoc;

public:
  CopyExpr(SourceLoc copyLoc, Expr *sub, Type type = Type(),
           bool implicit = false)
      : Expr(ExprKind::Copy, implicit, type), SubExpr(sub), CopyLoc(copyLoc) {}

  static CopyExpr *createImplicit(ASTContext &ctx, SourceLoc copyLoc, Expr *sub,
                                  Type type = Type()) {
    return new (ctx) CopyExpr(copyLoc, sub, type, /*implicit=*/true);
  }

  SourceLoc getLoc() const { return CopyLoc; }

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *E) { SubExpr = E; }

  SourceLoc getStartLoc() const { return CopyLoc; }
  SourceLoc getEndLoc() const { return getSubExpr()->getEndLoc(); }

  static bool classof(const Expr *e) { return e->getKind() == ExprKind::Copy; }
};

/// BorrowExpr - A 'borrow' surrounding an lvalue/accessor expression at an
/// apply site marking the lvalue/accessor as being borrowed when passed to the
/// callee.
///
/// getSemanticsProvidingExpr() looks through this because it doesn't
/// provide the value and only very specific clients care where the
/// 'borrow' was written.
class BorrowExpr final : public IdentityExpr {
  SourceLoc BorrowLoc;

public:
  BorrowExpr(SourceLoc borrowLoc, Expr *sub, Type type = Type(),
             bool implicit = false)
      : IdentityExpr(ExprKind::Borrow, sub, type, implicit),
        BorrowLoc(borrowLoc) {}

  static BorrowExpr *createImplicit(ASTContext &ctx, SourceLoc borrowLoc,
                                    Expr *sub, Type type = Type()) {
    return new (ctx) BorrowExpr(borrowLoc, sub, type, /*implicit=*/true);
  }

  SourceLoc getLoc() const { return BorrowLoc; }

  SourceLoc getStartLoc() const { return BorrowLoc; }
  SourceLoc getEndLoc() const { return getSubExpr()->getEndLoc(); }

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::Borrow;
  }
};

/// TupleExpr - Parenthesized expressions like '(a: x+x)' and '(x, y, 4)'. Note
/// that expressions like '(4)' are represented with a ParenExpr.
class TupleExpr final : public Expr,
    private toolchain::TrailingObjects<TupleExpr, Expr *, Identifier, SourceLoc> {
  friend TrailingObjects;

  SourceLoc LParenLoc;
  SourceLoc RParenLoc;

  size_t numTrailingObjects(OverloadToken<Expr *>) const {
    return getNumElements();
  }
  size_t numTrailingObjects(OverloadToken<Identifier>) const {
    return hasElementNames() ? getNumElements() : 0;
  }
  size_t numTrailingObjects(OverloadToken<SourceLoc>) const {
    return hasElementNames() ? getNumElements() : 0;
  }

  /// Retrieve the buffer containing the element names.
  MutableArrayRef<Identifier> getElementNamesBuffer() {
    if (!hasElementNames())
      return { };

    return { getTrailingObjects<Identifier>(), getNumElements() };
  }

  /// Retrieve the buffer containing the element name locations.
  MutableArrayRef<SourceLoc> getElementNameLocsBuffer() {
    if (!hasElementNameLocs())
      return { };
    
    return { getTrailingObjects<SourceLoc>(), getNumElements() };
  }

  TupleExpr(SourceLoc LParenLoc, SourceLoc RParenLoc,
            ArrayRef<Expr *> SubExprs, ArrayRef<Identifier> ElementNames,
            ArrayRef<SourceLoc> ElementNameLocs, bool Implicit, Type Ty);

public:
  /// Create a tuple.
  static TupleExpr *create(ASTContext &ctx,
                           SourceLoc LParenLoc, 
                           ArrayRef<Expr *> SubExprs,
                           ArrayRef<Identifier> ElementNames, 
                           ArrayRef<SourceLoc> ElementNameLocs,
                           SourceLoc RParenLoc, bool Implicit,
                           Type Ty = Type());

  /// Create an empty tuple.
  static TupleExpr *createEmpty(ASTContext &ctx, SourceLoc LParenLoc, 
                                SourceLoc RParenLoc, bool Implicit);

  /// Create an implicit tuple with no source information.
  static TupleExpr *createImplicit(ASTContext &ctx, ArrayRef<Expr *> SubExprs,
                                   ArrayRef<Identifier> ElementNames);

  SourceLoc getLParenLoc() const { return LParenLoc; }
  SourceLoc getRParenLoc() const { return RParenLoc; }

  SourceRange getSourceRange() const;

  /// Retrieve the elements of this tuple.
  MutableArrayRef<Expr*> getElements() {
    return { getTrailingObjects<Expr *>(), getNumElements() };
  }

  /// Retrieve the elements of this tuple.
  ArrayRef<Expr*> getElements() const {
    return { getTrailingObjects<Expr *>(), getNumElements() };
  }

  unsigned getNumElements() const { return Bits.TupleExpr.NumElements; }

  Expr *getElement(unsigned i) const {
    return getElements()[i];
  }
  void setElement(unsigned i, Expr *e) {
    getElements()[i] = e;
  }

  /// Whether this tuple has element names.
  bool hasElementNames() const { 
    return Bits.TupleExpr.HasElementNames;
  }

  /// Retrieve the element names for a tuple.
  ArrayRef<Identifier> getElementNames() const { 
    return const_cast<TupleExpr *>(this)->getElementNamesBuffer();
  }
  
  /// Retrieve the ith element name.
  Identifier getElementName(unsigned i) const {
    return hasElementNames() ? getElementNames()[i] : Identifier();
  }
  
  /// Whether this tuple has element name locations.
  bool hasElementNameLocs() const { 
    return Bits.TupleExpr.HasElementNameLocations;
  }

  /// Retrieve the locations of the element names for a tuple.
  ArrayRef<SourceLoc> getElementNameLocs() const {
    return const_cast<TupleExpr *>(this)->getElementNameLocsBuffer();
  }

  /// Retrieve the location of the ith label, if known.
  SourceLoc getElementNameLoc(unsigned i) const {
    if (hasElementNameLocs())
      return getElementNameLocs()[i];
    
    return SourceLoc();
  }

  static bool classof(const Expr *E) { return E->getKind() == ExprKind::Tuple; }
};

/// A collection literal expression.
///
/// The subexpression is represented as a TupleExpr or ParenExpr and
/// passed on to the appropriate semantics-providing conversion
/// operation.
class CollectionExpr : public Expr {
  SourceLoc LBracketLoc;
  SourceLoc RBracketLoc;
  ConcreteDeclRef Initializer;

  /// Retrieve the intrusive pointer storage from the subtype
  Expr *const *getTrailingObjectsPointer() const;
  Expr **getTrailingObjectsPointer() {
    const CollectionExpr *temp = this;
    return const_cast<Expr**>(temp->getTrailingObjectsPointer());
  }

  /// Retrieve the intrusive pointer storage from the subtype
  const SourceLoc *getTrailingSourceLocs() const;
  SourceLoc *getTrailingSourceLocs() {
    const CollectionExpr *temp = this;
    return const_cast<SourceLoc*>(temp->getTrailingSourceLocs());
  }

protected:
  CollectionExpr(ExprKind Kind, SourceLoc LBracketLoc,
                 ArrayRef<Expr*> Elements, ArrayRef<SourceLoc> CommaLocs,
                 SourceLoc RBracketLoc, Type Ty)
    : Expr(Kind, /*Implicit=*/false, Ty),
      LBracketLoc(LBracketLoc), RBracketLoc(RBracketLoc) {
    Bits.CollectionExpr.IsTypeDefaulted = false;
    Bits.CollectionExpr.NumSubExprs = Elements.size();
    Bits.CollectionExpr.NumCommas = CommaLocs.size();
    assert(Bits.CollectionExpr.NumCommas == CommaLocs.size() && "Truncation");
    std::uninitialized_copy(Elements.begin(), Elements.end(),
                            getTrailingObjectsPointer());
    std::uninitialized_copy(CommaLocs.begin(), CommaLocs.end(),
                            getTrailingSourceLocs());
  }

public:

  /// Retrieve the elements stored in the collection.
  ArrayRef<Expr *> getElements() const {
    return {getTrailingObjectsPointer(), static_cast<size_t>(Bits.CollectionExpr.NumSubExprs)};
  }
  MutableArrayRef<Expr *> getElements() {
    return {getTrailingObjectsPointer(), static_cast<size_t>(Bits.CollectionExpr.NumSubExprs)};
  }
  Expr *getElement(unsigned i) const { return getElements()[i]; }
  void setElement(unsigned i, Expr *E) { getElements()[i] = E; }
  unsigned getNumElements() const { return Bits.CollectionExpr.NumSubExprs; }

  /// Retrieve the comma source locations stored in the collection. Please note
  /// that trailing commas are currently allowed, and that invalid code may have
  /// stray or missing commas.
  MutableArrayRef<SourceLoc> getCommaLocs() {
    return {getTrailingSourceLocs(), static_cast<size_t>(Bits.CollectionExpr.NumCommas)};
  }
  ArrayRef<SourceLoc> getCommaLocs() const {
    return {getTrailingSourceLocs(), static_cast<size_t>(Bits.CollectionExpr.NumCommas)};
  }
  unsigned getNumCommas() const { return Bits.CollectionExpr.NumCommas; }

  bool isTypeDefaulted() const { return Bits.CollectionExpr.IsTypeDefaulted; }
  void setIsTypeDefaulted(bool value = true) {
    Bits.CollectionExpr.IsTypeDefaulted = value;
  }

  SourceLoc getLBracketLoc() const { return LBracketLoc; }
  SourceLoc getRBracketLoc() const { return RBracketLoc; }
  SourceRange getSourceRange() const {
    return SourceRange(LBracketLoc, RBracketLoc);
  }

  static bool classof(const Expr *e) {
    return e->getKind() >= ExprKind::First_CollectionExpr &&
           e->getKind() <= ExprKind::Last_CollectionExpr;
  }

  /// Retrieve the initializer that will be used to construct the 'array'
  /// literal from the result of the initializer.
  ConcreteDeclRef getInitializer() const { return Initializer; }

  /// Set the initializer that will be used to construct the 'array' literal.
  void setInitializer(ConcreteDeclRef initializer) {
    Initializer = initializer;
  }
};
 
/// An array literal expression [a, b, c].
class ArrayExpr final : public CollectionExpr,
    private toolchain::TrailingObjects<ArrayExpr, Expr*, SourceLoc> {
  friend TrailingObjects;
  friend CollectionExpr;

  size_t numTrailingObjects(OverloadToken<Expr *>) const {
    return getNumElements();
  }
  size_t numTrailingObjects(OverloadToken<SourceLoc>) const {
    return getNumCommas();
  }

  ArrayExpr(SourceLoc LBracketLoc, ArrayRef<Expr*> Elements,
            ArrayRef<SourceLoc> CommaLocs,
            SourceLoc RBracketLoc, Type Ty)
  : CollectionExpr(ExprKind::Array, LBracketLoc, Elements, CommaLocs,
                   RBracketLoc, Ty) { }
public:
  static ArrayExpr *create(ASTContext &C, SourceLoc LBracketLoc,
                           ArrayRef<Expr*> Elements,
                           ArrayRef<SourceLoc> CommaLocs,
                           SourceLoc RBracketLoc,
                           Type Ty = Type());

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::Array;
  }

  Type getElementType();
};

/// A dictionary literal expression [a : x, b : y, c : z].
class DictionaryExpr final : public CollectionExpr,
    private toolchain::TrailingObjects<DictionaryExpr, Expr*, SourceLoc> {
  friend TrailingObjects;
  friend CollectionExpr;

  size_t numTrailingObjects(OverloadToken<Expr *>) const {
    return getNumElements();
  }
  size_t numTrailingObjects(OverloadToken<SourceLoc>) const {
    return getNumCommas();
  }

  DictionaryExpr(SourceLoc LBracketLoc, ArrayRef<Expr*> Elements,
                 ArrayRef<SourceLoc> CommaLocs,
                 SourceLoc RBracketLoc, Type Ty)
    : CollectionExpr(ExprKind::Dictionary, LBracketLoc, Elements, CommaLocs,
                     RBracketLoc, Ty) { }
public:

  static DictionaryExpr *create(ASTContext &C, SourceLoc LBracketLoc,
                                ArrayRef<Expr*> Elements,
                                ArrayRef<SourceLoc> CommaLocs,
                                SourceLoc RBracketLoc,
                                Type Ty = Type());

  static bool classof(const Expr *e) {
    return e->getKind() == ExprKind::Dictionary;
  }

  Type getElementType();
};

/// Subscripting expressions like a[i] that refer to an element within a
/// container.
///
/// There is no built-in subscripting in the language. Rather, a fully
/// type-checked and well-formed subscript expression refers to a subscript
/// declaration, which provides a getter and (optionally) a setter that will
/// be used to perform reads/writes.
class SubscriptExpr final : public LookupExpr {
  ArgumentList *ArgList;

  SubscriptExpr(Expr *base, ArgumentList *argList, ConcreteDeclRef decl,
                bool implicit, AccessSemantics semantics);

public:
  /// Create a new subscript.
  static SubscriptExpr *
  create(ASTContext &ctx, Expr *base, ArgumentList *argList,
         ConcreteDeclRef decl = ConcreteDeclRef(), bool implicit = false,
         AccessSemantics semantics = AccessSemantics::Ordinary);

  ArgumentList *getArgs() const { return ArgList; }
  void setArgs(ArgumentList *newArgs) { ArgList = newArgs; }

  /// Determine whether this subscript reference should bypass the
  /// ordinary accessors.
  AccessSemantics getAccessSemantics() const {
    return (AccessSemantics) Bits.SubscriptExpr.Semantics;
  }

  SourceLoc getLoc() const { return getArgs()->getStartLoc(); }
  SourceLoc getStartLoc() const { return getBase()->getStartLoc(); }
  SourceLoc getEndLoc() const {
    auto end = getArgs()->getEndLoc();
    return end.isValid() ? end : getBase()->getEndLoc();
  }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Subscript;
  }
};

/// Subscripting expression that applies a keypath to a base.
class KeyPathApplicationExpr : public Expr {
  Expr *Base;
  Expr *KeyPath;
  SourceLoc LBracketLoc, RBracketLoc;
  
public:
  KeyPathApplicationExpr(Expr *base, SourceLoc lBracket, Expr *keyPath,
                         SourceLoc rBracket, Type ty, bool implicit)
    : Expr(ExprKind::KeyPathApplication, implicit, ty),
      Base(base), KeyPath(keyPath), LBracketLoc(lBracket), RBracketLoc(rBracket)
  {}
  
  SourceLoc getLoc() const { return LBracketLoc; }
  SourceLoc getStartLoc() const { return Base->getStartLoc(); }
  SourceLoc getEndLoc() const { return RBracketLoc; }
  
  Expr *getBase() const { return Base; }
  void setBase(Expr *E) { Base = E; }
  Expr *getKeyPath() const { return KeyPath; }
  void setKeyPath(Expr *E) { KeyPath = E; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::KeyPathApplication;
  }
};

/// A member access (foo.bar) on an expression with unresolved type.
class UnresolvedDotExpr : public Expr {
  Expr *SubExpr;
  SourceLoc DotLoc;
  DeclNameLoc NameLoc;
  DeclNameRef Name;
  ArrayRef<ValueDecl *> OuterAlternatives;

public:
  UnresolvedDotExpr(
      Expr *subexpr, SourceLoc dotloc, DeclNameRef name, DeclNameLoc nameloc,
      bool Implicit,
      ArrayRef<ValueDecl *> outerAlternatives = ArrayRef<ValueDecl *>())
      : Expr(ExprKind::UnresolvedDot, Implicit), SubExpr(subexpr),
        DotLoc(dotloc), NameLoc(nameloc), Name(name),
        OuterAlternatives(outerAlternatives) {
    setFunctionRefInfo(FunctionRefInfo::unapplied(nameloc));
  }

  static UnresolvedDotExpr *createImplicit(
      ASTContext &C, Expr *base, DeclName name) {
    return new (C) UnresolvedDotExpr(base, SourceLoc(),
                                     DeclNameRef(name), DeclNameLoc(),
                                     /*implicit=*/true);
  }

  static UnresolvedDotExpr *createImplicit(
      ASTContext &C, Expr *base,
      DeclBaseName baseName, ArrayRef<Identifier> argLabels) {
    return UnresolvedDotExpr::createImplicit(C, base,
                                             DeclName(C, baseName, argLabels));
  }

  static UnresolvedDotExpr *createImplicit(
      ASTContext &C, Expr *base,
      DeclBaseName baseName, ParameterList *paramList) {
    return UnresolvedDotExpr::createImplicit(C, base,
                                             DeclName(C, baseName, paramList));
  }

  SourceLoc getLoc() const {
    if (NameLoc.isValid())
      return NameLoc.getBaseNameLoc();
    else if (DotLoc.isValid())
      return DotLoc;
    else
      return SubExpr->getEndLoc();
  }

  SourceLoc getStartLoc() const {
    auto SubLoc = SubExpr->getStartLoc();
    if (SubLoc.isValid())
      return SubLoc;
    else if (DotLoc.isValid())
      return DotLoc;
    else
      return NameLoc.getSourceRange().Start;
  }
  SourceLoc getEndLoc() const {
    if (NameLoc.isValid())
      return NameLoc.getSourceRange().End;
    else if (DotLoc.isValid())
      return DotLoc;
    else
      return SubExpr->getEndLoc();
  }

  SourceLoc getDotLoc() const { return DotLoc; }
  Expr *getBase() const { return SubExpr; }
  void setBase(Expr *e) { SubExpr = e; }

  DeclNameRef getName() const { return Name; }
  DeclNameLoc getNameLoc() const { return NameLoc; }

  ArrayRef<ValueDecl *> getOuterAlternatives() const {
    return OuterAlternatives;
  }

  /// Retrieve the kind of function reference.
  FunctionRefInfo getFunctionRefInfo() const {
    return FunctionRefInfo::fromOpaque(Bits.UnresolvedDotExpr.FunctionRefInfo);
  }

  /// Set the kind of function reference.
  void setFunctionRefInfo(FunctionRefInfo refKind) {
    Bits.UnresolvedDotExpr.FunctionRefInfo = refKind.getOpaqueValue();
    ASSERT(refKind == getFunctionRefInfo() && "FunctionRefInfo truncated");
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnresolvedDot;
  }
};

/// TupleElementExpr - Refer to an element of a tuple,
/// e.g. "(1,field:2).field".
class TupleElementExpr : public Expr {
  Expr *SubExpr;
  SourceLoc NameLoc;
  SourceLoc DotLoc;

public:
  TupleElementExpr(Expr *SubExpr, SourceLoc DotLoc, unsigned FieldNo,
                   SourceLoc NameLoc, Type Ty)
    : Expr(ExprKind::TupleElement, /*Implicit=*/false, Ty), SubExpr(SubExpr),
      NameLoc(NameLoc), DotLoc(DotLoc) {
    Bits.TupleElementExpr.FieldNo = FieldNo;
  }

  SourceLoc getLoc() const { return NameLoc; }
  Expr *getBase() const { return SubExpr; }
  void setBase(Expr *e) { SubExpr = e; }

  unsigned getFieldNumber() const { return Bits.TupleElementExpr.FieldNo; }
  SourceLoc getNameLoc() const { return NameLoc; }  
  SourceLoc getDotLoc() const { return DotLoc; }
  
  SourceLoc getStartLoc() const { return getBase()->getStartLoc(); }
  SourceLoc getEndLoc() const { return getNameLoc(); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::TupleElement;
  }
};

/// Describes a monadic bind from T? to T.
///
/// In a ?-chain expression, this is the part that's spelled with a
/// postfix ?.
///
/// A BindOptionalExpr must always appear within a
/// OptionalEvaluationExpr.  If the operand of the BindOptionalExpr
/// evaluates to a missing value, the OptionalEvaluationExpr
/// immediately completes and produces a missing value in the result
/// type.
///
/// The depth of the BindOptionalExpr indicates which
/// OptionalEvaluationExpr is completed, in case the BindOptionalExpr
/// is contained within more than one such expression.
class BindOptionalExpr : public Expr {
  Expr *SubExpr;
  SourceLoc QuestionLoc;

public:
  BindOptionalExpr(Expr *subExpr, SourceLoc questionLoc,
                   unsigned depth, Type ty = Type())
    : Expr(ExprKind::BindOptional, /*Implicit=*/ questionLoc.isInvalid(), ty),
      SubExpr(subExpr), QuestionLoc(questionLoc) {
    Bits.BindOptionalExpr.Depth = depth;
    assert(Bits.BindOptionalExpr.Depth == depth && "bitfield truncation");
  }

  SourceRange getSourceRange() const {
    if (QuestionLoc.isInvalid())
      return SubExpr->getSourceRange();
    return SourceRange(SubExpr->getStartLoc(), QuestionLoc);
  }
  SourceLoc getStartLoc() const {
    return SubExpr->getStartLoc();
  }
  SourceLoc getEndLoc() const {
    return (QuestionLoc.isInvalid() ? SubExpr->getEndLoc() : QuestionLoc);
  }
  SourceLoc getLoc() const { 
    if (isImplicit())
      return SubExpr->getLoc();

    return getQuestionLoc(); 
  }
  SourceLoc getQuestionLoc() const { return QuestionLoc; }

  unsigned getDepth() const { return Bits.BindOptionalExpr.Depth; }
  void setDepth(unsigned depth) {
    Bits.BindOptionalExpr.Depth = depth;
  }

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *expr) { SubExpr = expr; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::BindOptional;
  }
};

/// Describes the outer limits of an operation containing
/// monadic binds of T? to T.
///
/// In a ?-chain expression, this is implicitly formed at the outer
/// limits of the chain.  For example, in (foo?.bar?().baz).fred,
/// this is nested immediately within the parens.
///
/// This expression will always have optional type.
class OptionalEvaluationExpr : public Expr {
  Expr *SubExpr;

public:
  OptionalEvaluationExpr(Expr *subExpr, Type ty = Type())
    : Expr(ExprKind::OptionalEvaluation, /*Implicit=*/ true, ty),
      SubExpr(subExpr) {}

  LANGUAGE_FORWARD_SOURCE_LOCS_TO(SubExpr)

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *expr) { SubExpr = expr; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::OptionalEvaluation;
  }
};

/// An expression that forces an optional to its underlying value.
///
/// \code
/// fn parseInt(s : String) -> Int? { ... }
///
/// var maybeInt = parseInt("5")     // returns an Int?
/// var forcedInt = parseInt("5")!   // returns an Int; fails on empty optional
/// \endcode
///
class ForceValueExpr : public Expr {
  Expr *SubExpr;
  SourceLoc ExclaimLoc;

public:
  ForceValueExpr(Expr *subExpr, SourceLoc exclaimLoc, bool forcedIUO = false)
    : Expr(ExprKind::ForceValue, /*Implicit=*/exclaimLoc.isInvalid(), Type()),
      SubExpr(subExpr), ExclaimLoc(exclaimLoc) {
    Bits.ForceValueExpr.ForcedIUO = forcedIUO;
  }

  SourceRange getSourceRange() const {
    if (ExclaimLoc.isInvalid())
      return SubExpr->getSourceRange();

    return SourceRange(SubExpr->getStartLoc(), ExclaimLoc);
  }
  SourceLoc getStartLoc() const {
    return SubExpr->getStartLoc();
  }
  SourceLoc getEndLoc() const {
    return (isImplicit() ? SubExpr->getEndLoc() : getExclaimLoc());
  }
  SourceLoc getLoc() const {
    if (!isImplicit())
      return getExclaimLoc();
    
    return SubExpr->getLoc();
  }
  SourceLoc getExclaimLoc() const { return ExclaimLoc; }

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *expr) { SubExpr = expr; }

  bool isForceOfImplicitlyUnwrappedOptional() const {
    return Bits.ForceValueExpr.ForcedIUO;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ForceValue;
  }
};

/// An expression that grants temporary escapability to a nonescaping
/// closure value.
///
/// This expression is formed by the type checker when a call to the
/// `withoutActuallyEscaping` declaration is made.
class MakeTemporarilyEscapableExpr : public Expr {
  Expr *NonescapingClosureValue;
  OpaqueValueExpr *EscapingClosureValue;
  Expr *SubExpr;
  SourceLoc NameLoc, LParenLoc, RParenLoc;
  Expr *OriginalExpr;

public:
  MakeTemporarilyEscapableExpr(SourceLoc NameLoc,
                               SourceLoc LParenLoc,
                               Expr *NonescapingClosureValue,
                               Expr *SubExpr,
                               SourceLoc RParenLoc,
                               OpaqueValueExpr *OpaqueValueForEscapingClosure,
                               Expr *OriginalExpr,
                               bool implicit = false)
    : Expr(ExprKind::MakeTemporarilyEscapable, implicit, Type()),
      NonescapingClosureValue(NonescapingClosureValue),
      EscapingClosureValue(OpaqueValueForEscapingClosure),
      SubExpr(SubExpr),
      NameLoc(NameLoc), LParenLoc(LParenLoc), RParenLoc(RParenLoc),
      OriginalExpr(OriginalExpr)
  {}
  
  SourceLoc getStartLoc() const {
    return NameLoc;
  }
  SourceLoc getEndLoc() const {
    return RParenLoc;
  }
  
  SourceLoc getLoc() const {
    return NameLoc;
  }
  
  /// Retrieve the opaque value representing the escapable copy of the
  /// closure.
  OpaqueValueExpr *getOpaqueValue() const { return EscapingClosureValue; }
  
  /// Retrieve the nonescaping closure expression.
  Expr *getNonescapingClosureValue() const {
    return NonescapingClosureValue;
  }
  void setNonescapingClosureValue(Expr *e) {
    NonescapingClosureValue = e;
  }
  
  /// Retrieve the subexpression that has access to the escapable copy of the
  /// closure.
  Expr *getSubExpr() const {
    return SubExpr;
  }
  void setSubExpr(Expr *e) {
    SubExpr = e;
  }

  /// Retrieve the original 'withoutActuallyEscaping(closure) { ... }'
  //  expression.
  Expr *getOriginalExpr() const {
    return OriginalExpr;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::MakeTemporarilyEscapable;
  }
};

/// An expression that opens up a value of protocol or protocol
/// composition type and gives a name to its dynamic type.
///
/// This expression is implicitly created by the type checker when
/// calling a method on a protocol. In the future, this may become an
/// actual operation within the language.
class OpenExistentialExpr : public Expr {
  Expr *ExistentialValue;
  OpaqueValueExpr *OpaqueValue;
  Expr *SubExpr;
  SourceLoc ExclaimLoc;

public:
  OpenExistentialExpr(Expr *existentialValue,
                      OpaqueValueExpr *opaqueValue,
                      Expr *subExpr,
                      Type subExprTy)
    : Expr(ExprKind::OpenExistential, /*Implicit=*/ true, subExprTy),
      ExistentialValue(existentialValue), OpaqueValue(opaqueValue), 
      SubExpr(subExpr) { }

  LANGUAGE_FORWARD_SOURCE_LOCS_TO(SubExpr)

  /// Retrieve the expression that is being evaluated using the
  /// archetype value.
  ///
  /// This subexpression (and no other) may refer to the archetype
  /// type or the opaque value that stores the archetype's value.
  Expr *getSubExpr() const { return SubExpr; }

  /// Set the subexpression that is being evaluated.
  void setSubExpr(Expr *expr) { SubExpr = expr; }

  /// Retrieve the existential value that is being opened.
  Expr *getExistentialValue() const { return ExistentialValue; }

  /// Set the existential val ue that is being opened.
  void setExistentialValue(Expr *expr) { ExistentialValue = expr; }

  /// Retrieve the opaque value representing the value (of archetype
  /// type) stored in the existential.
  OpaqueValueExpr *getOpaqueValue() const { return OpaqueValue; }

  /// Retrieve the opened archetype, which can only be referenced
  /// within this expression's subexpression.
  ExistentialArchetypeType *getOpenedArchetype() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::OpenExistential;
  }
};

/// ImplicitConversionExpr - An abstract class for expressions which
/// implicitly convert the value of an expression in some way.
class ImplicitConversionExpr : public Expr {
  Expr *SubExpr;

protected:
  ImplicitConversionExpr(ExprKind kind, Expr *subExpr, Type ty)
    : Expr(kind, /*Implicit=*/true, ty), SubExpr(subExpr) {}

public:
  LANGUAGE_FORWARD_SOURCE_LOCS_TO(SubExpr)

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *e) { SubExpr = e; }

  Expr *getSyntacticSubExpr() const {
    if (auto *ICE = dyn_cast<ImplicitConversionExpr>(SubExpr))
      return ICE->getSyntacticSubExpr();
    return SubExpr;
  }

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_ImplicitConversionExpr &&
           E->getKind() <= ExprKind::Last_ImplicitConversionExpr;
  }
};

/// The implicit conversion from a class metatype to AnyObject.
class ClassMetatypeToObjectExpr : public ImplicitConversionExpr {
public:
  ClassMetatypeToObjectExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::ClassMetatypeToObject, subExpr, ty) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ClassMetatypeToObject;
  }
};

/// The implicit conversion from a class existential metatype to AnyObject.
class ExistentialMetatypeToObjectExpr : public ImplicitConversionExpr {
public:
  ExistentialMetatypeToObjectExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::ExistentialMetatypeToObject, subExpr, ty) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ExistentialMetatypeToObject;
  }
};
  
/// The implicit conversion from a protocol value metatype to ObjC's Protocol
/// class type.
class ProtocolMetatypeToObjectExpr : public ImplicitConversionExpr {
public:
  ProtocolMetatypeToObjectExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::ProtocolMetatypeToObject, subExpr, ty) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ProtocolMetatypeToObject;
  }
};
  
/// InjectIntoOptionalExpr - The implicit conversion from T to T?.
class InjectIntoOptionalExpr : public ImplicitConversionExpr {
public:
  InjectIntoOptionalExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::InjectIntoOptional, subExpr, ty) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::InjectIntoOptional;
  }
};
  
/// Convert the address of an inout property to a pointer.
class InOutToPointerExpr : public ImplicitConversionExpr {
public:
  InOutToPointerExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(ExprKind::InOutToPointer, subExpr, ty) {
    Bits.InOutToPointerExpr.IsNonAccessing = false;
  }

  /// Is this conversion "non-accessing"?  That is, is it only using the
  /// pointer for its identity, as opposed to actually accessing the memory?
  bool isNonAccessing() const {
    return Bits.InOutToPointerExpr.IsNonAccessing;
  }
  void setNonAccessing(bool nonAccessing = true) {
    Bits.InOutToPointerExpr.IsNonAccessing = nonAccessing;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::InOutToPointer;
  }
};
  
/// Convert the address of an array to a pointer.
class ArrayToPointerExpr : public ImplicitConversionExpr {
public:
  ArrayToPointerExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(ExprKind::ArrayToPointer, subExpr, ty) {
    Bits.ArrayToPointerExpr.IsNonAccessing = false;
  }
  
  /// Is this conversion "non-accessing"?  That is, is it only using the
  /// pointer for its identity, as opposed to actually accessing the memory?
  bool isNonAccessing() const {
    return Bits.ArrayToPointerExpr.IsNonAccessing;
  }
  void setNonAccessing(bool nonAccessing = true) {
    Bits.ArrayToPointerExpr.IsNonAccessing = nonAccessing;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ArrayToPointer;
  }
};
  
/// Convert the a string to a pointer referencing its encoded representation.
class StringToPointerExpr : public ImplicitConversionExpr {
public:
  StringToPointerExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::StringToPointer, subExpr, ty) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::StringToPointer;
  }
};
  
/// Convert a pointer to a different kind of pointer.
class PointerToPointerExpr : public ImplicitConversionExpr {
public:
  PointerToPointerExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::PointerToPointer, subExpr, ty) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::PointerToPointer;
  }
};

/// Convert between a foreign object and its corresponding Objective-C object.
class ForeignObjectConversionExpr : public ImplicitConversionExpr {
public:
  ForeignObjectConversionExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::ForeignObjectConversion, subExpr, ty) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ForeignObjectConversion;
  }
};

/// Construct an unevaluated instance of the underlying metatype.
class UnevaluatedInstanceExpr : public ImplicitConversionExpr {
public:
  UnevaluatedInstanceExpr(Expr *subExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::UnevaluatedInstance, subExpr, ty) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnevaluatedInstance;
  }
};

class DifferentiableFunctionExpr : public ImplicitConversionExpr {
public:
  DifferentiableFunctionExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(ExprKind::DifferentiableFunction, subExpr, ty) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DifferentiableFunction;
  }
};

class LinearFunctionExpr : public ImplicitConversionExpr {
public:
  LinearFunctionExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(ExprKind::LinearFunction, subExpr, ty) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::LinearFunction;
  }
};

class DifferentiableFunctionExtractOriginalExpr
    : public ImplicitConversionExpr {
public:
  DifferentiableFunctionExtractOriginalExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(ExprKind::DifferentiableFunctionExtractOriginal,
                               subExpr, ty) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DifferentiableFunctionExtractOriginal;
  }
};

class LinearFunctionExtractOriginalExpr : public ImplicitConversionExpr {
public:
  LinearFunctionExtractOriginalExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(ExprKind::LinearFunctionExtractOriginal,
                               subExpr, ty) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::LinearFunctionExtractOriginal;
  }
};

class LinearToDifferentiableFunctionExpr : public ImplicitConversionExpr {
public:
  LinearToDifferentiableFunctionExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(
            ExprKind::LinearToDifferentiableFunction, subExpr, ty) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::LinearToDifferentiableFunction;
  }
};

/// Use an opaque type to abstract a value of the underlying concrete type,
/// possibly nested inside other types. E.g. can perform conversions "T --->
/// (opaque type)" and "S<T> ---> S<(opaque type)>".
class UnderlyingToOpaqueExpr : public ImplicitConversionExpr {
public:
  /// The substitutions to be applied to the opaque type declaration to
  /// produce the resulting type.
  const SubstitutionMap substitutions;

  UnderlyingToOpaqueExpr(Expr *subExpr, Type ty, SubstitutionMap substitutions)
    : ImplicitConversionExpr(ExprKind::UnderlyingToOpaque, subExpr, ty),
      substitutions(substitutions) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnderlyingToOpaque;
  }
};

/// DestructureTupleExpr - Destructure a tuple value produced by a source
/// expression, binding the elements to OpaqueValueExprs, then evaluate the
/// result expression written in terms of the OpaqueValueExprs.
class DestructureTupleExpr final : public ImplicitConversionExpr,
    private toolchain::TrailingObjects<DestructureTupleExpr, OpaqueValueExpr *> {
  friend TrailingObjects;

  size_t numTrailingObjects(OverloadToken<OpaqueValueExpr *>) const {
    return Bits.DestructureTupleExpr.NumElements;
  }

private:
  Expr *DstExpr;

  DestructureTupleExpr(ArrayRef<OpaqueValueExpr *> destructuredElements,
                       Expr *srcExpr, Expr *dstExpr, Type ty)
    : ImplicitConversionExpr(ExprKind::DestructureTuple, srcExpr, ty),
      DstExpr(dstExpr) {
    Bits.DestructureTupleExpr.NumElements = destructuredElements.size();
    std::uninitialized_copy(destructuredElements.begin(),
                            destructuredElements.end(),
                            getTrailingObjects<OpaqueValueExpr *>());
  }

public:
  /// Create a tuple destructuring. The type of srcExpr must be a tuple type,
  /// and the number of elements must equal the size of destructureElements.
  static DestructureTupleExpr *
  create(ASTContext &ctx,
         ArrayRef<OpaqueValueExpr *> destructuredElements,
         Expr *srcExpr, Expr *dstExpr, Type ty);

  ArrayRef<OpaqueValueExpr *> getDestructuredElements() const {
    return {getTrailingObjects<OpaqueValueExpr *>(),
            static_cast<size_t>(Bits.DestructureTupleExpr.NumElements)};
  }

  Expr *getResultExpr() const {
    return DstExpr;
  }

  void setResultExpr(Expr *dstExpr) {
    DstExpr = dstExpr;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DestructureTuple;
  }
};

/// LoadExpr - Turn an l-value into an r-value by performing a "load"
/// operation.  This operation may actually be a logical operation,
/// i.e. one implemented using a call to a potentially user-defined
/// function instead of a simple memory transaction.
class LoadExpr : public ImplicitConversionExpr {
public:
  LoadExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::Load, subExpr, type) { }

  static bool classof(const Expr *E) { return E->getKind() == ExprKind::Load; }
};

/// ABISafeConversion - models a type conversion on an l-value that has no
/// material affect on the ABI of the type, while *preserving* the l-valueness
/// of the type.
class ABISafeConversionExpr : public ImplicitConversionExpr {
public:
  ABISafeConversionExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::ABISafeConversion, subExpr, type) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ABISafeConversion;
  }
};

/// This is a conversion from an expression of UnresolvedType to an arbitrary
/// other type, and from an arbitrary type to UnresolvedType.  This node does
/// not appear in valid code, only in code involving diagnostics.
class UnresolvedTypeConversionExpr : public ImplicitConversionExpr {
public:
  UnresolvedTypeConversionExpr(Expr *subExpr, Type type)
  : ImplicitConversionExpr(ExprKind::UnresolvedTypeConversion, subExpr, type) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnresolvedTypeConversion;
  }
};

/// FunctionConversionExpr - Convert a function to another function type,
/// which might involve renaming the parameters or handling substitutions
/// of subtypes (in the return) or supertypes (in the input).
///
/// FIXME: This should be a CapturingExpr.
class FunctionConversionExpr : public ImplicitConversionExpr {
public:
  FunctionConversionExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::FunctionConversion, subExpr, type) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::FunctionConversion;
  }
};

/// Perform a function conversion from one function that to one that has a
/// covariant result type.
///
/// This conversion is technically unsafe; however, semantic analysis will
/// only introduce such a conversion in cases where other language features
/// (i.e., Self returns) enforce static safety. Additionally, this conversion
/// avoids changing the ABI of the function in question.
class CovariantFunctionConversionExpr : public ImplicitConversionExpr {
public:
  CovariantFunctionConversionExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::CovariantFunctionConversion, subExpr,
                             type) { }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::CovariantFunctionConversion;
  }
};

/// Perform a conversion from a superclass to a subclass for a call to
/// a method with a covariant result type.
///
/// This conversion is technically unsafe; however, semantic analysis will
/// only introduce such a conversion in cases where other language features
/// (i.e., Self returns) enforce static safety.
class CovariantReturnConversionExpr : public ImplicitConversionExpr {
public:
  CovariantReturnConversionExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::CovariantReturnConversion, subExpr,
                             type) { }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::CovariantReturnConversion;
  }
};

/// MetatypeConversionExpr - Convert a metatype to another metatype
/// using essentially a derived-to-base conversion.
class MetatypeConversionExpr : public ImplicitConversionExpr {
public:
  MetatypeConversionExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::MetatypeConversion, subExpr, type) {}
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::MetatypeConversion;
  }
};
  
/// CollectionUpcastConversionExpr - Convert a collection whose
/// elements have some type T to the same kind of collection whose
/// elements have type U, where U is a subtype of T.
class CollectionUpcastConversionExpr : public ImplicitConversionExpr {
public:
  struct ConversionPair {
    OpaqueValueExpr *OrigValue;
    Expr *Conversion;

    explicit operator bool() const { return OrigValue != nullptr; }
  };
private:
  ConversionPair KeyConversion;
  ConversionPair ValueConversion;
public:
  CollectionUpcastConversionExpr(Expr *subExpr, Type type,
                                 ConversionPair keyConversion,
                                 ConversionPair valueConversion)
    : ImplicitConversionExpr(
        ExprKind::CollectionUpcastConversion, subExpr, type),
      KeyConversion(keyConversion), ValueConversion(valueConversion) {
    assert((!KeyConversion || ValueConversion)
           && "key conversion without value conversion");
  }

  /// Returns the expression that should be used to perform a
  /// conversion of the collection's values; null if the conversion
  /// is formally trivial because the key type does not change.
  const ConversionPair &getKeyConversion() const {
    return KeyConversion;
  }
  void setKeyConversion(const ConversionPair &pair) {
    KeyConversion = pair;
  }

  /// Returns the expression that should be used to perform a
  /// conversion of the collection's values; null if the conversion
  /// is formally trivial because the value type does not change.
  const ConversionPair &getValueConversion() const {
    return ValueConversion;
  }
  void setValueConversion(const ConversionPair &pair) {
    ValueConversion = pair;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::CollectionUpcastConversion;
  }
};
  
/// ErasureExpr - Perform type erasure by converting a value to existential
/// type. For example:
///
/// \code
/// protocol Printable {}
/// struct Book {}
///
/// var printable: Printable = Book() // erases type
/// var printableType: Printable.Type = Book.self // erases metatype
/// \endcode
///
/// The type of the expression should always satisfy isAnyExistentialType().
///
/// The type of the sub-expression should always be either:
///   - a non-existential type of the appropriate kind or
///   - an existential type of the appropriate kind which is a subtype
///     of the result type.
///
/// "Appropriate kind" means e.g. a concrete/existential metatype if the
/// result is an existential metatype.
class ErasureExpr final : public ImplicitConversionExpr,
    private toolchain::TrailingObjects<ErasureExpr, ProtocolConformanceRef,
                                  CollectionUpcastConversionExpr::ConversionPair> {
  friend TrailingObjects;
  using ConversionPair = CollectionUpcastConversionExpr::ConversionPair;

  ErasureExpr(Expr *subExpr, Type type,
              ArrayRef<ProtocolConformanceRef> conformances,
              ArrayRef<ConversionPair> argConversions)
    : ImplicitConversionExpr(ExprKind::Erasure, subExpr, type) {
    Bits.ErasureExpr.NumConformances = conformances.size();
    std::uninitialized_copy(conformances.begin(), conformances.end(),
                            getTrailingObjects<ProtocolConformanceRef>());

    Bits.ErasureExpr.NumArgumentConversions = argConversions.size();
    std::uninitialized_copy(argConversions.begin(), argConversions.end(),
                            getTrailingObjects<ConversionPair>());

    assert(toolchain::all_of(conformances, [](ProtocolConformanceRef ref) {
      return !ref.isInvalid();
    }));
  }

public:
  static ErasureExpr *create(ASTContext &ctx, Expr *subExpr, Type type,
                             ArrayRef<ProtocolConformanceRef> conformances,
                             ArrayRef<ConversionPair> argConversions);

  size_t numTrailingObjects(OverloadToken<ProtocolConformanceRef>) const {
    return Bits.ErasureExpr.NumConformances;
  }
  size_t numTrailingObjects(OverloadToken<ConversionPair>) const {
    return Bits.ErasureExpr.NumArgumentConversions;
  }

  /// Retrieve the mapping specifying how the type of the subexpression
  /// maps to the resulting existential type. If the resulting existential
  /// type involves several different protocols, there will be mappings for each
  /// of those protocols, in the order in which the existential type expands
  /// its properties.
  ///
  /// The entries in this array may be null, indicating that the conformance
  /// to the corresponding protocol is trivial (because the source
  /// type is either an archetype or an existential type that conforms to
  /// that corresponding protocol).
  ArrayRef<ProtocolConformanceRef> getConformances() const {
    return {getTrailingObjects<ProtocolConformanceRef>(),
            static_cast<size_t>(Bits.ErasureExpr.NumConformances) };
  }

  /// Retrieve the conversion expressions mapping requirements from any
  /// parameterized existentials involved in this erasure.
  ///
  /// If the destination type is not a parameterized protocol type,
  /// this array will be empty
  ArrayRef<ConversionPair> getArgumentConversions() const {
    return {getTrailingObjects<ConversionPair>(),
            static_cast<size_t>(Bits.ErasureExpr.NumArgumentConversions) };
  }

  /// Retrieve the conversion expressions mapping requirements from any
  /// parameterized existentials involved in this erasure.
  ///
  /// If the destination type is not a parameterized protocol type,
  /// this array will be empty
  MutableArrayRef<ConversionPair> getArgumentConversions() {
    return {getTrailingObjects<ConversionPair>(),
            static_cast<size_t>(Bits.ErasureExpr.NumArgumentConversions) };
  }

  void setArgumentConversion(unsigned i, const ConversionPair &p) {
    getArgumentConversions()[i] = p;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Erasure;
  }
};

/// AnyHashableErasureExpr - Perform type erasure by converting a value
/// to AnyHashable type.
///
/// The type of the sub-expression should always be a type that implements
/// the Hashable protocol.
class AnyHashableErasureExpr : public ImplicitConversionExpr {
  ProtocolConformanceRef Conformance;

public:
  AnyHashableErasureExpr(Expr *subExpr, Type type,
                         ProtocolConformanceRef conformance)
    : ImplicitConversionExpr(ExprKind::AnyHashableErasure, subExpr, type),
      Conformance(conformance) {}

  /// Retrieve the mapping specifying how the type of the
  /// subexpression conforms to the Hashable protocol.
  ProtocolConformanceRef getConformance() const {
    return Conformance;
  }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::AnyHashableErasure;
  }
};

/// ConditionalBridgeFromObjCExpr - Bridge a value from a non-native
/// representation.
class ConditionalBridgeFromObjCExpr : public ImplicitConversionExpr {
  ConcreteDeclRef Conversion;

public:
  ConditionalBridgeFromObjCExpr(Expr *subExpr, Type type,
                                ConcreteDeclRef conversion)
    : ImplicitConversionExpr(ExprKind::ConditionalBridgeFromObjC, subExpr, type),
      Conversion(conversion) {
  }

  /// Retrieve the conversion function.
  ConcreteDeclRef getConversion() const {
    return Conversion;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ConditionalBridgeFromObjC;
  }
};

/// BridgeFromObjCExpr - Bridge a value from a non-native representation.
class BridgeFromObjCExpr : public ImplicitConversionExpr {
public:
  BridgeFromObjCExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::BridgeFromObjC, subExpr, type) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::BridgeFromObjC;
  }
};

/// BridgeToObjCExpr - Bridge a value to a non-native representation.
class BridgeToObjCExpr : public ImplicitConversionExpr {
public:
  BridgeToObjCExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::BridgeToObjC, subExpr, type) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::BridgeToObjC;
  }
};

/// ActorIsolationErasureExpr - A special kind of function conversion that
/// drops actor isolation.
class ActorIsolationErasureExpr : public ImplicitConversionExpr {
public:
  ActorIsolationErasureExpr(Expr *subExpr, Type type)
      : ImplicitConversionExpr(ExprKind::ActorIsolationErasure, subExpr, type) {
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ActorIsolationErasure;
  }
};

/// UnsafeCastExpr - A special kind of conversion that performs an unsafe
/// bitcast from one type to the other.
///
/// Note that this is an unsafe operation and type-checker is allowed to
/// use this only in a limited number of cases like: `any Sendable` -> `Any`
/// conversions in some positions, covariant conversions of function and
/// function result types.
class UnsafeCastExpr : public ImplicitConversionExpr {
public:
  UnsafeCastExpr(Expr *subExpr, Type type)
      : ImplicitConversionExpr(ExprKind::UnsafeCast, subExpr, type) {
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnsafeCast;
  }
};

/// Extracts the isolation of a dynamically isolated function value.
class ExtractFunctionIsolationExpr : public Expr {
  /// The function value expression from which to extract the
  /// isolation. The type of `fnExpr` must be an ``@isolated(any)`
  /// funciton.
  Expr *fnExpr;

  /// The source location of `.isolation`
  SourceLoc isolationLoc;

public:
  ExtractFunctionIsolationExpr(Expr *fnExpr, SourceLoc isolationLoc,
                               Type type, bool implicit = false)
      : Expr(ExprKind::ExtractFunctionIsolation, implicit, type),
        fnExpr(fnExpr), isolationLoc(isolationLoc) {}

  Expr *getFunctionExpr() const {
    return fnExpr;
  }

  void setFunctionExpr(Expr *fnExpr) {
    this->fnExpr = fnExpr;
  }

  SourceLoc getStartLoc() const {
    return fnExpr->getStartLoc();
  }

  SourceLoc getEndLoc() const {
    return isolationLoc;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ExtractFunctionIsolation;
  }
};

/// UnresolvedSpecializeExpr - Represents an explicit specialization using
/// a type parameter list (e.g. "Vector<Int>") that has not been resolved.
class UnresolvedSpecializeExpr final : public Expr,
    private toolchain::TrailingObjects<UnresolvedSpecializeExpr, TypeRepr *> {
  friend TrailingObjects;

  Expr *SubExpr;
  SourceLoc LAngleLoc;
  SourceLoc RAngleLoc;

  UnresolvedSpecializeExpr(Expr *SubExpr,
                           SourceLoc LAngleLoc,
                           ArrayRef<TypeRepr *> UnresolvedParams,
                           SourceLoc RAngleLoc)
    : Expr(ExprKind::UnresolvedSpecialize, /*Implicit=*/false),
      SubExpr(SubExpr), LAngleLoc(LAngleLoc), RAngleLoc(RAngleLoc) {
    Bits.UnresolvedSpecializeExpr.NumUnresolvedParams = UnresolvedParams.size();
    std::uninitialized_copy(UnresolvedParams.begin(), UnresolvedParams.end(),
                            getTrailingObjects<TypeRepr *>());
  }

public:
  static UnresolvedSpecializeExpr *
  create(ASTContext &ctx, Expr *SubExpr, SourceLoc LAngleLoc,
         ArrayRef<TypeRepr *> UnresolvedParams, SourceLoc RAngleLoc);
  
  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *e) { SubExpr = e; }
  
  /// Retrieve the list of type parameters. These parameters have not yet
  /// been bound to archetypes of the entity to be specialized.
  ArrayRef<TypeRepr *> getUnresolvedParams() const {
    return {getTrailingObjects<TypeRepr *>(),
            static_cast<size_t>(Bits.UnresolvedSpecializeExpr.NumUnresolvedParams)};
  }
  
  SourceLoc getLoc() const { return LAngleLoc; }
  SourceLoc getLAngleLoc() const { return LAngleLoc; }
  SourceLoc getRAngleLoc() const { return RAngleLoc; }

  SourceLoc getStartLoc() const { return SubExpr->getStartLoc(); }
  SourceLoc getEndLoc() const { return RAngleLoc; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnresolvedSpecialize;
  }
};

/// Describes an implicit conversion from a subclass to one of its
/// superclasses.
class DerivedToBaseExpr : public ImplicitConversionExpr {
public:
  DerivedToBaseExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::DerivedToBase, subExpr, type) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DerivedToBase;
  }
};

/// Describes an implicit conversion from a value of archetype type to
/// its concrete superclass.
class ArchetypeToSuperExpr : public ImplicitConversionExpr {
public:
  ArchetypeToSuperExpr(Expr *subExpr, Type type)
    : ImplicitConversionExpr(ExprKind::ArchetypeToSuper, subExpr, type) {}

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ArchetypeToSuper;
  }
};

/// An expression that models an implicit conversion from an uninhabited value
/// to any type. It cannot be evaluated.
class UnreachableExpr : public ImplicitConversionExpr {
  UnreachableExpr(Expr *subExpr, Type ty)
      : ImplicitConversionExpr(ExprKind::Unreachable, subExpr, ty) {}

public:
  static UnreachableExpr *create(ASTContext &ctx, Expr *subExpr, Type ty) {
    return new (ctx) UnreachableExpr(subExpr, ty);
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Unreachable;
  }
};

/// The builtin unary '&' operator, which converts the
/// given lvalue into an 'inout' argument value.
class InOutExpr : public Expr {
  Expr *SubExpr;
  SourceLoc OperLoc;

public:
  InOutExpr(SourceLoc operLoc, Expr *subExpr, Type baseType,
            bool isImplicit = false);

  SourceLoc getStartLoc() const { return OperLoc; }
  SourceLoc getEndLoc() const { return SubExpr->getEndLoc(); }
  SourceLoc getLoc() const { return OperLoc; }

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *e) { SubExpr = e; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::InOut;
  }
};

/// The not-yet-actually-surfaced '...' varargs expansion operator,
/// which splices an array into a sequence of variadic arguments.
class VarargExpansionExpr : public Expr {
  Expr *SubExpr;

  VarargExpansionExpr(Expr *subExpr, bool implicit, Type type = Type())
    : Expr(ExprKind::VarargExpansion, implicit, type), SubExpr(subExpr) {}

public:
  static VarargExpansionExpr *createParamExpansion(ASTContext &ctx, Expr *E);
  static VarargExpansionExpr *createArrayExpansion(ASTContext &ctx, ArrayExpr *AE);

  LANGUAGE_FORWARD_SOURCE_LOCS_TO(SubExpr)

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *subExpr) { SubExpr = subExpr; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::VarargExpansion;
  }
};

/// A pack element expression spelled with the contextual \c each
/// keyword applied to a pack reference expression.
///
/// \code
///  fn zip<T..., U...>(t: (each T)..., u: (each U)...) {
///    let zipped = (each t, each u)...
///  }
/// \endcode
///
/// Pack elements can only appear in the pattern expression of a
/// \c PackExpansionExpr.
class PackElementExpr final : public Expr {
  SourceLoc EachLoc;
  Expr *PackRefExpr;

  PackElementExpr(SourceLoc eachLoc, Expr *packRefExpr,
                  bool implicit = false, Type type = Type())
      : Expr(ExprKind::PackElement, implicit, type),
        EachLoc(eachLoc), PackRefExpr(packRefExpr) {}

public:
  static PackElementExpr *create(ASTContext &ctx, SourceLoc eachLoc,
                                 Expr *packRefExpr, bool implicit = false,
                                 Type type = Type());

  Expr *getPackRefExpr() const { return PackRefExpr; }

  void setPackRefExpr(Expr *packRefExpr) {
    PackRefExpr = packRefExpr;
  }

  SourceLoc getStartLoc() const {
    return EachLoc;
  }

  SourceLoc getEndLoc() const {
    return PackRefExpr->getEndLoc();
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::PackElement;
  }
};

/// A pack expansion expression is a pattern expression followed by
/// the expansion operator '...'. The pattern expression contains
/// references to parameter packs of length N, and the expansion
/// operator will repeat the pattern N times.
///
/// Pack expansion expressions are permitted in expression contexts
/// that naturally accept a comma-separated list of values, including
/// call argument lists, the elements of a tuple value, and the source
/// of a for-in loop.
class PackExpansionExpr final : public Expr {
  SourceLoc RepeatLoc;
  Expr *PatternExpr;
  GenericEnvironment *Environment;

  PackExpansionExpr(SourceLoc repeatLoc,
                    Expr *patternExpr,
                    GenericEnvironment *environment,
                    bool implicit, Type type)
    : Expr(ExprKind::PackExpansion, implicit, type),
      RepeatLoc(repeatLoc), PatternExpr(patternExpr),
      Environment(environment) {}

public:
  static PackExpansionExpr *create(ASTContext &ctx,
                                   SourceLoc repeatLoc,
                                   Expr *patternExpr,
                                   GenericEnvironment *environment,
                                   bool implicit = false,
                                   Type type = Type());

  Expr *getPatternExpr() const { return PatternExpr; }

  void setPatternExpr(Expr *patternExpr) {
    PatternExpr = patternExpr;
  }

  GenericEnvironment *getGenericEnvironment() {
    return Environment;
  }

  void setGenericEnvironment(GenericEnvironment *env) {
    Environment = env;
  }

  SourceLoc getStartLoc() const {
    return RepeatLoc;
  }

  SourceLoc getEndLoc() const {
    return PatternExpr->getEndLoc();
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::PackExpansion;
  }
};

/// An expression to materialize a pack from a tuple containing a pack
/// expansion.
///
/// These nodes are created by CSApply and should only appear in a
/// type-checked AST in the context of a \c PackExpansionExpr .
class MaterializePackExpr final : public Expr {
  /// The expression from which to materialize a pack.
  Expr *FromExpr;

  /// The source location of (deprecated) \c .element
  SourceLoc ElementLoc;

  MaterializePackExpr(Expr *fromExpr, SourceLoc elementLoc,
                      Type type, bool implicit)
      : Expr(ExprKind::MaterializePack, implicit, type),
        FromExpr(fromExpr), ElementLoc(elementLoc) {}

public:
  static MaterializePackExpr *create(ASTContext &ctx, Expr *fromExpr,
                                     SourceLoc elementLoc, Type type,
                                     bool implicit = false);

  Expr *getFromExpr() const { return FromExpr; }

  void setFromExpr(Expr *fromExpr) {
    FromExpr = fromExpr;
  }

  SourceLoc getStartLoc() const {
    return FromExpr->getStartLoc();
  }

  SourceLoc getEndLoc() const {
    return ElementLoc.isValid() ? ElementLoc : FromExpr->getEndLoc();
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::MaterializePack;
  }
};

/// SequenceExpr - A list of binary operations which has not yet been
/// folded into a tree.  The operands all have even indices, while the
/// subexpressions with odd indices are all (potentially overloaded)
/// references to binary operators.
class SequenceExpr final : public Expr,
    private toolchain::TrailingObjects<SequenceExpr, Expr *> {
  friend TrailingObjects;

  /// The cached folded expression.
  Expr *FoldedExpr = nullptr;

  SequenceExpr(ArrayRef<Expr*> elements)
    : Expr(ExprKind::Sequence, /*Implicit=*/false) {
    Bits.SequenceExpr.NumElements = elements.size();
    assert(Bits.SequenceExpr.NumElements > 0 && "zero-length sequence!");
    std::uninitialized_copy(elements.begin(), elements.end(),
                            getTrailingObjects<Expr*>());
  }

public:
  static SequenceExpr *create(ASTContext &ctx, ArrayRef<Expr*> elements);

  SourceLoc getStartLoc() const {
    return getElement(0)->getStartLoc();
  }
  SourceLoc getEndLoc() const {
    return getElement(getNumElements() - 1)->getEndLoc();
  }
  
  unsigned getNumElements() const { return Bits.SequenceExpr.NumElements; }

  MutableArrayRef<Expr*> getElements() {
    return {getTrailingObjects<Expr*>(), static_cast<size_t>(Bits.SequenceExpr.NumElements)};
  }

  ArrayRef<Expr*> getElements() const {
    return {getTrailingObjects<Expr*>(), static_cast<size_t>(Bits.SequenceExpr.NumElements)};
  }

  Expr *getElement(unsigned i) const {
    return getElements()[i];
  }
  void setElement(unsigned i, Expr *e) {
    getElements()[i] = e;
  }

  /// Retrieve the folded expression, or \c nullptr if the SequencExpr has
  /// not yet been folded.
  Expr *getFoldedExpr() const {
    return FoldedExpr;
  }

  void setFoldedExpr(Expr *foldedExpr) {
    FoldedExpr = foldedExpr;
  }

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Sequence;
  }
};

/// A base class for closure expressions.
class AbstractClosureExpr : public DeclContext, public Expr {
  CaptureInfo Captures;

  /// The set of parameters.
  ParameterList *parameterList;

  /// Actor isolation of the closure.
  ActorIsolation actorIsolation;

public:
  AbstractClosureExpr(ExprKind Kind, Type FnType, bool Implicit,
                      DeclContext *Parent)
      : DeclContext(DeclContextKind::AbstractClosureExpr, Parent),
        Expr(Kind, Implicit, FnType),
        parameterList(nullptr),
        actorIsolation(ActorIsolation::forUnspecified()) {
    Bits.AbstractClosureExpr.Discriminator = InvalidDiscriminator;
  }

  /// If we find that a capture x of this AbstractClosureExpr belongs to a
  /// different isolation domain than the closure, add an ApplyIsolationCrossing
  /// to foundIsolationCrossing.
  ///
  /// \p foundIsolationCrossings an out parameter that contains the
  /// ApplyIsolationCrossing if any of the captures are isolation crossing and
  /// the index of the capture in the capture array. We return the index since
  /// all captures may not cross isolation boundaries and we may need to be able
  /// to look up the corresponding capture at the SIL level by index.
  void getIsolationCrossing(
      SmallVectorImpl<
          std::tuple<CapturedValue, unsigned, ApplyIsolationCrossing>>
          &foundIsolationCrossings);

  CaptureInfo getCaptureInfo() const {
    assert(Captures.hasBeenComputed());
    return Captures;
  }

  std::optional<CaptureInfo> getCachedCaptureInfo() const {
    if (!Captures.hasBeenComputed())
      return std::nullopt;
    return Captures;
  }

  void setCaptureInfo(CaptureInfo captures) {
    assert(captures.hasBeenComputed());
    Captures = captures;
  }

  /// Retrieve the parameters of this closure.
  ParameterList *getParameters() { return parameterList; }
  const ParameterList *getParameters() const { return parameterList; }
  void setParameterList(ParameterList *P);

  // Expose this to users.
  using DeclContext::setParent;

  /// Returns a discriminator which determines this expression's index
  /// in the sequence of closure expressions within the current
  /// function.
  ///
  /// There are separate sequences for explicit and implicit closures.
  /// This allows explicit closures to maintain a stable numbering
  /// across simple edits that introduce auto closures above them,
  /// which is the best we can reasonably do.
  ///
  /// (Autoclosures are likely to be eliminated immediately, even in
  /// unoptimized builds, so their names are fairly unimportant.  It's
  /// much more likely that explicit closures will survive
  /// optimization and therefore make it into e.g. stack traces.
  /// Having their symbol names be stable across minor code changes is
  /// therefore pretty useful for debugging.)
  unsigned getDiscriminator() const;

  /// Retrieve the raw discriminator, which may not have been computed yet.
  ///
  /// Only use this for queries that are checking for (e.g.) reentrancy or
  /// intentionally do not want to initiate verification.
  unsigned getRawDiscriminator() const {
    return Bits.AbstractClosureExpr.Discriminator;
  }

  void setDiscriminator(unsigned discriminator) {
    assert(getRawDiscriminator() == InvalidDiscriminator);
    assert(discriminator != InvalidDiscriminator);
    Bits.AbstractClosureExpr.Discriminator = discriminator;
  }
  enum : unsigned { InvalidDiscriminator = 0xFFFF };

  /// Retrieve the result type of this closure.
  Type getResultType(toolchain::function_ref<Type(Expr *)> getType =
                         [](Expr *E) -> Type { return E->getType(); }) const;

  /// Return whether this closure is throwing when fully applied.
  bool isBodyThrowing() const;

  /// Retrieve the "effective" thrown interface type, or std::nullopt if
  /// this closure cannot throw.
  ///
  /// Closures with untyped throws will produce "any Error", functions that
  /// cannot throw or are specified to throw "Never" will return std::nullopt.
  std::optional<Type> getEffectiveThrownType() const;

  /// \brief Return whether this closure is async when fully applied.
  bool isBodyAsync() const;

  /// Whether this closure is Sendable.
  bool isSendable() const;

  /// Whether this closure consists of a single expression.
  bool hasSingleExpressionBody() const;

  /// Retrieve the body for closure that has a single expression for
  /// its body, or \c nullptr if there is no single expression body.
  Expr *getSingleExpressionBody() const;

  /// Returns the body of closures that have a body
  /// returns nullptr if the closure doesn't have a body
  BraceStmt *getBody() const;

  /// Returns a boolean value indicating whether the body, if any, contains
  /// an explicit `return` statement.
  ///
  /// \returns `true` if the body contains an explicit `return` statement,
  /// `false` otherwise.
  bool bodyHasExplicitReturnStmt() const;

  /// Finds occurrences of explicit `return` statements within the body, if any.
  ///
  /// \param results An out container to which the results are added.
  void getExplicitReturnStmts(SmallVectorImpl<ReturnStmt *> &results) const;

  ActorIsolation getActorIsolation() const {
    return actorIsolation;
  }

  void setActorIsolation(ActorIsolation actorIsolation) {
    this->actorIsolation = actorIsolation;
  }

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_AbstractClosureExpr &&
           E->getKind() <= ExprKind::Last_AbstractClosureExpr;
  }

  static bool classof(const DeclContext *DC) {
    return DC->getContextKind() == DeclContextKind::AbstractClosureExpr;
  }

  using DeclContext::operator new;
  using DeclContext::operator delete;
  using Expr::dump;
};

/// SerializedAbstractClosureExpr - This represents what was originally an
/// AbstractClosureExpr during serialization. It is preserved only to maintain
/// the correct AST structure and remangling after deserialization.
class SerializedAbstractClosureExpr : public DeclContext {
  const Type Ty;
  toolchain::PointerIntPair<Type, 1> TypeAndImplicit;
  const unsigned Discriminator;

public:
  SerializedAbstractClosureExpr(Type Ty, bool Implicit, unsigned Discriminator,
                                DeclContext *Parent)
    : DeclContext(DeclContextKind::SerializedAbstractClosure, Parent),
      TypeAndImplicit(toolchain::PointerIntPair<Type, 1>(Ty, Implicit)),
      Discriminator(Discriminator) {}

  Type getType() const {
    return TypeAndImplicit.getPointer();
  }

  unsigned getDiscriminator() const {
    return Discriminator;
  }

  bool isImplicit() const {
    return TypeAndImplicit.getInt();
  }

  static bool classof(const DeclContext *DC) {
    return DC->getContextKind() == DeclContextKind::SerializedAbstractClosure;
  }
};

/// An explicit unnamed function expression, which can optionally have
/// named arguments.
///
/// \code
///     { $0 + $1 }
///     { a, b -> Int in a + b }
///     { (a : Int, b : Int) -> Int in a + b }
///     { [weak c] (a : Int) -> Int in a + c!.getFoo() }
/// \endcode
class ClosureExpr : public AbstractClosureExpr {
  friend class ExplicitCaughtTypeRequest;

public:
  enum class BodyState {
    /// The body was parsed.
    Parsed,

    /// The body was type-checked.
    TypeChecked,
  };

private:
  /// The attributes attached to the closure.
  DeclAttributes Attributes;

  /// The range of the brackets of the capture list, if present.
  SourceRange BracketRange;
    
  /// The (possibly null) VarDecl captured by this closure with the literal name
  /// "self". In order to recover this information at the time of name lookup,
  /// we must be able to access it from the associated DeclContext.
  /// Because the DeclContext inside a closure is the closure itself (and not
  /// the CaptureListExpr which would normally maintain this sort of
  /// information about captured variables), we need to have some way to access
  /// this information directly on the ClosureExpr.
  VarDecl *CapturedSelfDecl;

  /// The location of the "async", if present.
  SourceLoc AsyncLoc;

  /// The location of the "throws", if present.
  SourceLoc ThrowsLoc;
  
  /// The location of the '->' denoting an explicit return type,
  /// if present.
  SourceLoc ArrowLoc;

  /// The location of the "in", if present.
  SourceLoc InLoc;

  /// The explicitly-specified thrown type.
  TypeExpr *ThrownType;

  /// The explicitly-specified result type.
  toolchain::PointerIntPair<TypeExpr *, 2, BodyState> ExplicitResultTypeAndBodyState;

  /// The body of the closure.
  BraceStmt *Body;

  friend class GlobalActorAttributeRequest;

  bool hasNoGlobalActorAttribute() const {
    return Bits.ClosureExpr.NoGlobalActorAttribute;
  }

  void setHasNoGlobalActorAttribute() {
    Bits.ClosureExpr.NoGlobalActorAttribute = true;
  }

public:
  ClosureExpr(const DeclAttributes &attributes,
              SourceRange bracketRange, VarDecl *capturedSelfDecl,
              ParameterList *params, SourceLoc asyncLoc, SourceLoc throwsLoc,
              TypeExpr *thrownType, SourceLoc arrowLoc, SourceLoc inLoc,
              TypeExpr *explicitResultType, DeclContext *parent)
    : AbstractClosureExpr(ExprKind::Closure, Type(), /*Implicit=*/false,
                          parent),
      Attributes(attributes), BracketRange(bracketRange),
      CapturedSelfDecl(capturedSelfDecl),
      AsyncLoc(asyncLoc), ThrowsLoc(throwsLoc), ArrowLoc(arrowLoc),
      InLoc(inLoc), ThrownType(thrownType),
      ExplicitResultTypeAndBodyState(explicitResultType, BodyState::Parsed),
      Body(nullptr) {
    setParameterList(params);
    Bits.ClosureExpr.HasAnonymousClosureVars = false;
    Bits.ClosureExpr.ImplicitSelfCapture = false;
    Bits.ClosureExpr.InheritActorContext = false;
    Bits.ClosureExpr.InheritActorContextKind = 0;
    Bits.ClosureExpr.IsPassedToSendingParameter = false;
    Bits.ClosureExpr.NoGlobalActorAttribute = false;
    Bits.ClosureExpr.RequiresDynamicIsolationChecking = false;
    Bits.ClosureExpr.IsMacroArgument = false;
  }

  SourceRange getSourceRange() const;
  SourceLoc getStartLoc() const;
  SourceLoc getEndLoc() const;
  SourceLoc getLoc() const;

  BraceStmt *getBody() const { return Body; }
  void setBody(BraceStmt *S) { Body = S; }

  BraceStmt *getExpandedBody();

  DeclAttributes &getAttrs() { return Attributes; }
  const DeclAttributes &getAttrs() const { return Attributes; }

  /// Determine whether the parameters of this closure are actually
  /// anonymous closure variables.
  bool hasAnonymousClosureVars() const {
    return Bits.ClosureExpr.HasAnonymousClosureVars;
  }

  /// Set the parameters of this closure along with a flag indicating
  /// whether these parameters are actually anonymous closure variables.
  void setHasAnonymousClosureVars() {
    Bits.ClosureExpr.HasAnonymousClosureVars = true;
  }

  /// Whether this closure allows "self" to be implicitly captured without
  /// required "self." on each reference.
  bool allowsImplicitSelfCapture() const {
    return Bits.ClosureExpr.ImplicitSelfCapture;
  }

  void setAllowsImplicitSelfCapture(bool value = true) {
    Bits.ClosureExpr.ImplicitSelfCapture = value;
  }

  /// Whether this closure should implicitly inherit the actor context from
  /// where it was formed. This only affects @Sendable and sendable async
  /// closures.
  bool inheritsActorContext() const {
    return Bits.ClosureExpr.InheritActorContext;
  }

  /// Whether this closure should _always_ implicitly inherit the actor context
  /// regardless of whether the isolation parameter is captured or not.
  bool alwaysInheritsActorContext() const {
    if (!inheritsActorContext())
      return false;
    return getInheritActorIsolationModifier() ==
           InheritActorContextModifier::Always;
  }

  void setInheritsActorContext(bool value = true,
                               InheritActorContextModifier modifier =
                                   InheritActorContextModifier::None) {
    Bits.ClosureExpr.InheritActorContext = value;
    Bits.ClosureExpr.InheritActorContextKind = uint8_t(modifier);
    assert((static_cast<InheritActorContextModifier>(
                Bits.ClosureExpr.InheritActorContextKind) == modifier) &&
           "not enough bits for modifier");
  }

  InheritActorContextModifier getInheritActorIsolationModifier() const {
    assert(inheritsActorContext());
    return static_cast<InheritActorContextModifier>(
        Bits.ClosureExpr.InheritActorContextKind);
  }

  /// Whether the closure's concurrency behavior was determined by an
  /// \c \@preconcurrency declaration.
  bool isIsolatedByPreconcurrency() const {
    return Bits.ClosureExpr.IsolatedByPreconcurrency;
  }

  void setIsolatedByPreconcurrency(bool value = true) {
    Bits.ClosureExpr.IsolatedByPreconcurrency = value;
  }

  /// Whether the closure is a closure literal that is passed to a sending
  /// parameter.
  bool isPassedToSendingParameter() const {
    return Bits.ClosureExpr.IsPassedToSendingParameter;
  }

  void setIsPassedToSendingParameter(bool value = true) {
    Bits.ClosureExpr.IsPassedToSendingParameter = value;
  }

  /// True if this is an isolated closure literal that is passed 
  /// to a callee that has not been concurrency checked.
  bool requiresDynamicIsolationChecking() const {
    return Bits.ClosureExpr.RequiresDynamicIsolationChecking;
  }

  void setRequiresDynamicIsolationChecking(bool value = true) {
    Bits.ClosureExpr.RequiresDynamicIsolationChecking = value;
  }

  /// Whether this closure was type-checked as an argument to a macro. This
  /// is only populated after type-checking, and only exists for diagnostic
  /// logic. Do not add more uses of this.
  bool isMacroArgument() const {
    return Bits.ClosureExpr.IsMacroArgument;
  }

  void setIsMacroArgument(bool value = true) {
    Bits.ClosureExpr.IsMacroArgument = value;
  }

  /// Determine whether this closure expression has an
  /// explicitly-specified result type.
  bool hasExplicitResultType() const { return ArrowLoc.isValid(); }

  /// Retrieve the range of the \c '[' and \c ']' that enclose the capture list.
  SourceRange getBracketRange() const { return BracketRange; }
  
  /// Retrieve the location of the \c '->' for closures with an
  /// explicit result type.
  SourceLoc getArrowLoc() const {
    assert(hasExplicitResultType() && "No arrow location");
    return ArrowLoc;
  }

  /// Retrieve the location of the \c in for a closure that has it.
  SourceLoc getInLoc() const {
    return InLoc;
  }

  /// Retrieve the location of the 'async' for a closure that has it.
  SourceLoc getAsyncLoc() const {
    return AsyncLoc;
  }

  /// Retrieve the location of the 'throws' for a closure that has it.
  SourceLoc getThrowsLoc() const {
    return ThrowsLoc;
  }

  /// Retrieve the explicitly-thrown type.
  Type getExplicitThrownType() const;

  /// Retrieve the explicitly-thrown type representation.
  TypeRepr *getExplicitThrownTypeRepr() const {
    if (ThrownType)
      return ThrownType->getTypeRepr();

    return nullptr;
  }

  Type getExplicitResultType() const {
    assert(hasExplicitResultType() && "No explicit result type");
    return ExplicitResultTypeAndBodyState.getPointer()->getInstanceType();
  }
  void setExplicitResultType(Type ty);

  TypeRepr *getExplicitResultTypeRepr() const {
    assert(hasExplicitResultType() && "No explicit result type");
    return ExplicitResultTypeAndBodyState.getPointer()->getTypeRepr();
  }

  /// Returns the resolved macro for the given custom attribute
  /// attached to this closure expression.
  MacroDecl *getResolvedMacro(CustomAttr *customAttr);

  /// Determine whether the closure has a single expression for its
  /// body.
  ///
  /// This will be true for closures such as, e.g.,
  /// \code
  ///   { $0 + 1 }
  /// \endcode
  ///
  /// or
  ///
  /// \code
  ///   { x, y in x > y }
  /// \endcode
  ///
  /// ... even if the closure has been coerced to return Void by the type
  /// checker. This function does not return true for empty closures.
  bool hasSingleExpressionBody() const {
    return getSingleExpressionBody();
  }

  /// Retrieve the body for closure that has a single expression for
  /// its body, or \c nullptr if there is no single expression body.
  Expr *getSingleExpressionBody() const;

  /// Is this a completely empty closure?
  bool hasEmptyBody() const;

  /// VarDecl captured by this closure under the literal name \c self , if any.
  VarDecl *getCapturedSelfDecl() const {
    return CapturedSelfDecl;
  }

  /// Get the type checking state of this closure's body.
  BodyState getBodyState() const {
    return ExplicitResultTypeAndBodyState.getInt();
  }
  void setBodyState(BodyState v) {
    ExplicitResultTypeAndBodyState.setInt(v);
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Closure;
  }
  static bool classof(const AbstractClosureExpr *E) {
    return E->getKind() == ExprKind::Closure;
  }
  static bool classof(const DeclContext *C) {
    return isa<AbstractClosureExpr>(C) && classof(cast<AbstractClosureExpr>(C));
  }
};

/// This is an implicit closure of the contained subexpression that is usually
/// formed when a scalar expression is converted to @autoclosure function type.
/// \code
///   fn f(x : @autoclosure () -> Int)
///   f(42)  // AutoclosureExpr convert from Int to ()->Int
/// \endcode
class AutoClosureExpr : public AbstractClosureExpr {
  BraceStmt *Body;

public:
  enum class Kind : uint8_t {
    // An autoclosure with type () -> Result. Formed from type checking an
    // @autoclosure argument in a function call.
    None = 0,

    // An autoclosure with type (Args...) -> Result. Formed from type checking a
    // partial application.
    SingleCurryThunk = 1,

    // An autoclosure with type (Self) -> (Args...) -> Result. Formed from type
    // checking a partial application.
    DoubleCurryThunk = 2,

    // An autoclosure with type () -> Result that was formed for an async let.
    AsyncLet = 3,
  };

  AutoClosureExpr(Expr *Body, Type ResultTy, DeclContext *Parent)
      : AbstractClosureExpr(ExprKind::AutoClosure, ResultTy, /*Implicit=*/true,
                            Parent) {
    if (Body != nullptr)
      setBody(Body);

    Bits.AutoClosureExpr.Kind = 0;
  }

  Kind getThunkKind() const {
    return Kind(Bits.AutoClosureExpr.Kind);
  }

  void setThunkKind(Kind K) {
    Bits.AutoClosureExpr.Kind = uint8_t(K);
  }

  SourceRange getSourceRange() const;
  SourceLoc getStartLoc() const;
  SourceLoc getEndLoc() const;
  SourceLoc getLoc() const;

  BraceStmt *getBody() const { return Body; }
  void setBody(Expr *E);

  // Expose this to users.
  using DeclContext::setParent;

  /// Returns the body of the autoclosure as an \c Expr.
  ///
  /// The body of an autoclosure always consists of a single expression.
  Expr *getSingleExpressionBody() const;

  /// Unwraps a curry thunk. Basically, this gives you what the old AST looked
  /// like, before Sema started building curry thunks. This is really only
  /// meant for legacy usages.
  ///
  /// The behavior is as follows, based on the kind:
  /// - for double curry thunks, returns the original DeclRefExpr.
  /// - for single curry thunks, returns the ApplyExpr for the self call.
  /// - otherwise, returns nullptr for convenience.
  Expr *getUnwrappedCurryThunkExpr() const;

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::AutoClosure;
  }
  static bool classof(const AbstractClosureExpr *E) {
    return E->getKind() == ExprKind::AutoClosure;
  }
  static bool classof(const DeclContext *C) {
    return isa<AbstractClosureExpr>(C) && classof(cast<AbstractClosureExpr>(C));
  }
};

/// Instances of this structure represent elements of the capture list that can
/// optionally occur in a capture expression.
struct CaptureListEntry {
  PatternBindingDecl *PBD;

  explicit CaptureListEntry(PatternBindingDecl *PBD);

  static CaptureListEntry
  createParsed(ASTContext &Ctx, ReferenceOwnership ownershipKind,
               SourceRange ownershipRange, Identifier name, SourceLoc nameLoc,
               SourceLoc equalLoc, Expr *initializer, DeclContext *DC);

  VarDecl *getVar() const;
  bool isSimpleSelfCapture(bool excludeWeakCaptures = true) const;
};

/// CaptureListExpr - This expression represents the capture list on an explicit
/// closure.  Because the capture list is evaluated outside of the closure, this
/// CaptureList wraps the ClosureExpr.  The dynamic semantics are that evaluates
/// the variable bindings from the capture list, then evaluates the
/// subexpression (the closure itself) and returns the result.
class CaptureListExpr final : public Expr,
    private toolchain::TrailingObjects<CaptureListExpr, CaptureListEntry> {
  friend TrailingObjects;

  AbstractClosureExpr *closureBody;

  CaptureListExpr(ArrayRef<CaptureListEntry> captureList,
                  AbstractClosureExpr *closureBody)
    : Expr(ExprKind::CaptureList, /*Implicit=*/false, Type()),
      closureBody(closureBody) {
    assert(closureBody);
    Bits.CaptureListExpr.NumCaptures = captureList.size();
    std::uninitialized_copy(captureList.begin(), captureList.end(),
                            getTrailingObjects<CaptureListEntry>());
  }

public:
  static CaptureListExpr *create(ASTContext &ctx,
                                 ArrayRef<CaptureListEntry> captureList,
                                 AbstractClosureExpr *closureBody);

  ArrayRef<CaptureListEntry> getCaptureList() {
    return {getTrailingObjects<CaptureListEntry>(),
            static_cast<size_t>(Bits.CaptureListExpr.NumCaptures)};
  }
  AbstractClosureExpr *getClosureBody() { return closureBody; }
  const AbstractClosureExpr *getClosureBody() const { return closureBody; }

  void setClosureBody(AbstractClosureExpr *body) { closureBody = body; }

  /// This is a bit weird, but the capture list is lexically contained within
  /// the closure, so the ClosureExpr has the full source range.
  LANGUAGE_FORWARD_SOURCE_LOCS_TO(closureBody)

  // Implement isa/cast/dyncast/etc.
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::CaptureList;
  }
};

/// DynamicTypeExpr - "type(of: base)" - Produces a metatype value.
///
/// The metatype value comes from evaluating an expression then retrieving the
/// metatype of the result.
class DynamicTypeExpr : public Expr {
  SourceLoc KeywordLoc;
  SourceLoc LParenLoc;
  Expr *Base;
  SourceLoc RParenLoc;

public:
  explicit DynamicTypeExpr(SourceLoc KeywordLoc, SourceLoc LParenLoc,
                           Expr *Base, SourceLoc RParenLoc, Type Ty)
    : Expr(ExprKind::DynamicType, /*Implicit=*/false, Ty),
      KeywordLoc(KeywordLoc), LParenLoc(LParenLoc), Base(Base),
      RParenLoc(RParenLoc) { }

  Expr *getBase() const { return Base; }
  void setBase(Expr *base) { Base = base; }

  SourceLoc getLoc() const { return KeywordLoc; }
  SourceRange getSourceRange() const {
    return SourceRange(KeywordLoc, RParenLoc);
  }
  
  SourceLoc getStartLoc() const {
    return KeywordLoc;
  }
  SourceLoc getEndLoc() const {
    return RParenLoc;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DynamicType;
  }
};

/// An expression referring to an opaque object of a fixed type.
///
/// Opaque value expressions occur when a particular value within the AST
/// needs to be re-used without being re-evaluated or for a value that is
/// a placeholder. OpaqueValueExpr nodes are introduced by some other AST
/// node (say, an \c OpenExistentialExpr) and can only be used within the
/// subexpressions of that AST node.
class OpaqueValueExpr : public Expr {
  SourceRange Range;

public:
  explicit OpaqueValueExpr(SourceRange Range, Type Ty,
                           bool isPlaceholder = false)
      : Expr(ExprKind::OpaqueValue, /*Implicit=*/true, Ty), Range(Range) {
    Bits.OpaqueValueExpr.IsPlaceholder = isPlaceholder;
  }

  static OpaqueValueExpr *
  createImplicit(ASTContext &ctx, Type Ty, bool isPlaceholder = false,
                 AllocationArena arena = AllocationArena::Permanent);

  /// Whether this opaque value expression represents a placeholder that
  /// is injected before type checking to act as a placeholder for some
  /// value to be specified later.
  bool isPlaceholder() const { return Bits.OpaqueValueExpr.IsPlaceholder; }

  void setIsPlaceholder(bool value) {
    Bits.OpaqueValueExpr.IsPlaceholder = value;
  }

  SourceRange getSourceRange() const { return Range; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::OpaqueValue;
  }
};

/// A placeholder to substitute with a \c wrappedValue initialization expression
/// for a property with an attached property wrapper.
///
/// Wrapped value placeholder expressions are injected around the
/// \c wrappedValue argument in a synthesized \c init(wrappedValue:)
/// call. This injection happens for properties with attached property wrappers
/// that can be initialized out-of-line with a wrapped value expression, rather
/// than calling \c init(wrappedValue:) explicitly.
///
/// Wrapped value placeholders store the original initialization expression
/// if one exists, along with an opaque value placeholder that can be bound
/// to a different wrapped value expression.
class PropertyWrapperValuePlaceholderExpr : public Expr {
  SourceRange Range;
  OpaqueValueExpr *Placeholder;
  Expr *WrappedValue;
  bool IsAutoClosure = false;

  PropertyWrapperValuePlaceholderExpr(SourceRange Range, Type Ty,
                                      OpaqueValueExpr *placeholder,
                                      Expr *wrappedValue,
                                      bool isAutoClosure)
      : Expr(ExprKind::PropertyWrapperValuePlaceholder, /*Implicit=*/true, Ty),
        Range(Range), Placeholder(placeholder), WrappedValue(wrappedValue),
        IsAutoClosure(isAutoClosure) {}

public:
  static PropertyWrapperValuePlaceholderExpr *
  create(ASTContext &ctx, SourceRange range, Type ty, Expr *wrappedValue,
         bool isAutoClosure = false);

  /// The original wrappedValue initialization expression provided via
  /// \c = on a property with attached property wrappers.
  Expr *getOriginalWrappedValue() const {
    return WrappedValue;
  }

  void setOriginalWrappedValue(Expr *value) {
    WrappedValue = value;
  }

  /// An opaque value placeholder that will be used to substitute in a
  /// different wrapped value expression for out-of-line initialization.
  OpaqueValueExpr *getOpaqueValuePlaceholder() const {
    return Placeholder;
  }

  void setOpaqueValuePlaceholder(OpaqueValueExpr *placeholder) {
    Placeholder = placeholder;
  }

  bool isAutoClosure() const { return IsAutoClosure; }

  SourceRange getSourceRange() const { return Range; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::PropertyWrapperValuePlaceholder;
  }
};

/// An implicit applied property wrapper expression.
class AppliedPropertyWrapperExpr final : public Expr {
public:
  enum class ValueKind {
    WrappedValue,
    ProjectedValue
  };

private:
  /// The concrete callee which owns the property wrapper.
  ConcreteDeclRef Callee;

  /// The owning declaration.
  const ParamDecl *Param;

  /// The source location of the argument list.
  SourceLoc Loc;

  /// The value to which the property wrapper is applied for initialization.
  Expr *Value;

  /// The kind of value that the property wrapper is applied to.
  ValueKind Kind;

  AppliedPropertyWrapperExpr(ConcreteDeclRef callee, const ParamDecl *param, SourceLoc loc,
                             Type Ty, Expr *value, ValueKind kind)
      : Expr(ExprKind::AppliedPropertyWrapper, /*Implicit=*/true, Ty),
        Callee(callee), Param(param), Loc(loc), Value(value), Kind(kind) {}

public:
  static AppliedPropertyWrapperExpr *
  create(ASTContext &ctx, ConcreteDeclRef callee, const ParamDecl *param, SourceLoc loc,
         Type Ty, Expr *value, ValueKind kind);

  SourceRange getSourceRange() const { return Loc; }

  ConcreteDeclRef getCallee() { return Callee; }

  /// Returns the parameter declaration with the attached property wrapper.
  const ParamDecl *getParamDecl() const { return Param; };

  /// Returns the value that the property wrapper is applied to.
  Expr *getValue() { return Value; }

  /// Sets the value that the property wrapper is applied to.
  void setValue(Expr *value) { Value = value; }

  /// Returns the kind of value, between wrapped value and projected
  /// value, the property wrapper is applied to.
  ValueKind getValueKind() const { return Kind; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::AppliedPropertyWrapper;
  }
};

/// An expression referring to a default argument left unspecified at the
/// call site.
///
/// A DefaultArgumentExpr must only appear as a direct child of a
/// ParenExpr or a TupleExpr that is itself a call argument.
class DefaultArgumentExpr final : public Expr {
  friend class CallerSideDefaultArgExprRequest;

  /// The owning declaration.
  ConcreteDeclRef DefaultArgsOwner;

  /// The caller parameter index.
  unsigned ParamIndex;

  /// The source location of the argument list.
  SourceLoc Loc;

  /// Stores either a DeclContext or, upon type-checking, the caller-side
  /// default expression.
  PointerUnion<DeclContext *, Expr *> ContextOrCallerSideExpr;

  /// Whether this default argument is evaluated asynchronously because
  /// it's isolated to the callee's isolation domain.
  bool implicitlyAsync = false;

public:
  explicit DefaultArgumentExpr(ConcreteDeclRef defaultArgsOwner,
                               unsigned paramIndex, SourceLoc loc, Type Ty,
                               DeclContext *dc)
    : Expr(ExprKind::DefaultArgument, /*Implicit=*/true, Ty),
      DefaultArgsOwner(defaultArgsOwner), ParamIndex(paramIndex), Loc(loc),
      ContextOrCallerSideExpr(dc) { }

  SourceRange getSourceRange() const { return Loc; }

  ConcreteDeclRef getDefaultArgsOwner() const {
    return DefaultArgsOwner;
  }

  unsigned getParamIndex() const {
    return ParamIndex;
  }

  /// Retrieves the parameter declaration for this default argument.
  const ParamDecl *getParamDecl() const;

  /// Checks whether this is a caller-side default argument that is emitted
  /// directly at the call site.
  bool isCallerSide() const;

  /// For a caller-side default argument, retrieves the fully type-checked
  /// expression within the context of the call site.
  Expr *getCallerSideDefaultExpr() const;

  /// Get the required actor isolation for evaluating this default argument
  /// synchronously. If the caller does not meet the required isolation, the
  /// argument must be written explicitly at the call-site.
  ActorIsolation getRequiredIsolation() const;

  /// Whether this default argument is evaluated asynchronously because
  /// it's isolated to the callee's isolation domain.
  bool isImplicitlyAsync() const {
    return implicitlyAsync;
  }

  void setImplicitlyAsync() {
    implicitlyAsync = true;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DefaultArgument;
  }
};

/// ApplyExpr - Superclass of various function calls, which apply an argument to
/// a function to get a result.
class ApplyExpr : public Expr {
  /// The function being called.
  Expr *Fn;

  /// The list of arguments to call the function with.
  ArgumentList *ArgList;

  // If this apply crosses isolation boundaries, record the callee and caller
  // isolations in this struct.
  std::optional<ApplyIsolationCrossing> IsolationCrossing;

  /// Destination information for a thrown error, which includes any
  /// necessary conversions from the actual type thrown to the type that
  /// is expected by the enclosing context.
  ThrownErrorDestination ThrowDest;

protected:
  ApplyExpr(ExprKind kind, Expr *fn, ArgumentList *argList, bool implicit,
            Type ty = Type())
      : Expr(kind, implicit, ty), Fn(fn), ArgList(argList) {
    assert(ArgList);
    assert(classof((Expr*)this) && "ApplyExpr::classof out of date");
    Bits.ApplyExpr.ThrowsIsSet = false;
    Bits.ApplyExpr.ImplicitlyAsync = false;
    Bits.ApplyExpr.ImplicitlyThrows = false;
    Bits.ApplyExpr.NoAsync = false;
    Bits.ApplyExpr.ShouldApplyDistributedThunk = false;
  }

public:
  Expr *getFn() const { return Fn; }
  void setFn(Expr *e) { Fn = e; }
  Expr *getSemanticFn() const { return Fn->getSemanticsProvidingExpr(); }

  ArgumentList *getArgs() const { return ArgList; }
  void setArgs(ArgumentList *newArgList) { ArgList = newArgList; }

  /// Has the type-checker set the 'throws' bit yet?
  ///
  /// In general, this should only be used for debugging purposes.
  bool isThrowsSet() const { return Bits.ApplyExpr.ThrowsIsSet; }

  /// Does this application throw?  This is only meaningful after
  /// complete type-checking.
  ///
  /// Returns the thrown error destination, which includes both the type
  /// thrown from this application as well as the context's error type,
  /// which may be different.
  ThrownErrorDestination throws() const {
    assert(Bits.ApplyExpr.ThrowsIsSet);
    return ThrowDest;
  }

  void setThrows(ThrownErrorDestination throws) {
    assert(!Bits.ApplyExpr.ThrowsIsSet);
    Bits.ApplyExpr.ThrowsIsSet = true;
    ThrowDest = throws;
  }

  /// Is this a 'rethrows' function that is known not to throw?
  bool isNoThrows() const { return !throws(); }

  /// Is this a 'reasync' function that is known not to 'await'?
  bool isNoAsync() const { return Bits.ApplyExpr.NoAsync; }
  void setNoAsync(bool noAsync) {
    Bits.ApplyExpr.NoAsync = noAsync;
  }

  // Return the optionally stored ApplyIsolationCrossing instance - set iff this
  // ApplyExpr crosses isolation domains
  const std::optional<ApplyIsolationCrossing> getIsolationCrossing() const {
    return IsolationCrossing;
  }

  // Record that this apply crosses isolation domains, noting the isolation of
  // the caller and callee by storing them into the IsolationCrossing field
  void setIsolationCrossing(
      ActorIsolation callerIsolation, ActorIsolation calleeIsolation) {
    assert(!IsolationCrossing.has_value()
             && "IsolationCrossing should not be set twice");
    IsolationCrossing = {callerIsolation, calleeIsolation};
  }

  /// Is this application _implicitly_ required to be an async call?
  /// That is, does it need to be guarded by hop_to_executor.
  /// Note that this is _not_ a check for whether the callee is async!
  /// Only meaningful after complete type-checking.
  ///
  /// Generally, this comes up only when we have a non-self call to an actor
  /// instance's synchronous method. Such calls are conceptually treated as if
  /// they are wrapped with an async closure. For example,
  ///
  ///   act.syncMethod(a, b) 
  ///
  /// is equivalent to the eta-expanded version of act.syncMethod,
  ///
  ///   { (a1, b1) async in act.syncMethod(a1, b1) }(a, b)
  ///
  /// where the new closure is declared to be async.
  ///
  /// When the application is implicitly async, the result describes
  /// the actor to which we need to need to hop.
  std::optional<ActorIsolation> isImplicitlyAsync() const {
    if (!Bits.ApplyExpr.ImplicitlyAsync)
      return std::nullopt;

    auto isolationCrossing = getIsolationCrossing();
    assert(isolationCrossing.has_value()
           && "Implicitly async ApplyExprs should always "
              "have had IsolationCrossing set");

    return isolationCrossing.value().CalleeIsolation;
  }

  /// Note that this application is implicitly async and set the target.
  void setImplicitlyAsync(ActorIsolation target) {
    assert(getIsolationCrossing().has_value()
           && "ApplyExprs should always call setIsolationCrossing"
              " before setImplicitlyAsync");
    Bits.ApplyExpr.ImplicitlyAsync = true;
  }

  /// Is this application _implicitly_ required to be a throwing call?
  /// This can happen if the function is actually a distributed thunk
  /// invocation, which may throw, regardless of the target function throwing,
  /// e.g. a distributed instance method call on a 'remote' actor, may throw due
  /// to network issues reported by the transport, regardless if the actual
  /// target function can throw.
  bool implicitlyThrows() const {
    return Bits.ApplyExpr.ImplicitlyThrows;
  }
  void setImplicitlyThrows(bool flag) {
    Bits.ApplyExpr.ImplicitlyThrows = flag;
  }

  /// Informs IRGen to that this expression should be applied as its distributed
  /// thunk, rather than invoking the function directly.
  bool shouldApplyDistributedThunk() const {
    return Bits.ApplyExpr.ShouldApplyDistributedThunk;
  }
  void setShouldApplyDistributedThunk(bool flag) {
    Bits.ApplyExpr.ShouldApplyDistributedThunk = flag;
  }

  ValueDecl *getCalledValue(bool skipFunctionConversions = false) const;

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_ApplyExpr &&
           E->getKind() <= ExprKind::Last_ApplyExpr;
  }
};

/// CallExpr - Application of an argument to a function, which occurs
/// syntactically through juxtaposition with a TupleExpr whose
/// leading '(' is unspaced.
class CallExpr final : public ApplyExpr {
  CallExpr(Expr *fn, ArgumentList *argList, bool implicit, Type ty);

public:
  /// Create a new call expression.
  ///
  /// \param fn The function being called
  /// \param argList The argument list.
  static CallExpr *create(ASTContext &ctx, Expr *fn, ArgumentList *argList,
                          bool implicit);

  /// Create a new implicit call expression without any source-location
  /// information.
  ///
  /// \param fn The function being called.
  /// \param argList The argument list.
  static CallExpr *createImplicit(ASTContext &ctx, Expr *fn,
                                  ArgumentList *argList) {
    return create(ctx, fn, argList, /*implicit*/ true);
  }

  /// Create a new implicit call expression with no arguments and no
  /// source-location information.
  ///
  /// \param fn The nullary function being called.
  static CallExpr *createImplicitEmpty(ASTContext &ctx, Expr *fn);

  SourceLoc getStartLoc() const {
    SourceLoc fnLoc = getFn()->getStartLoc();
    return (fnLoc.isValid() ? fnLoc : getArgs()->getStartLoc());
  }
  SourceLoc getEndLoc() const {
    SourceLoc argLoc = getArgs()->getEndLoc();
    return (argLoc.isValid() ? argLoc : getFn()->getEndLoc());
  }
  
  SourceLoc getLoc() const { 
    SourceLoc FnLoc = getFn()->getLoc(); 
    return FnLoc.isValid() ? FnLoc : getArgs()->getStartLoc();
  }

  /// Retrieve the expression that directly represents the callee.
  ///
  /// The "direct" callee is the expression representing the callee
  /// after looking through top-level constructs that don't affect the
  /// identity of the callee, e.g., extra parentheses, optional
  /// unwrapping (?)/forcing (!), etc.
  Expr *getDirectCallee() const;

  static bool classof(const Expr *E) { return E->getKind() == ExprKind::Call; }
};
  
/// PrefixUnaryExpr - Prefix unary expressions like '!y'.
class PrefixUnaryExpr : public ApplyExpr {
  PrefixUnaryExpr(Expr *fn, ArgumentList *argList, Type ty = Type())
      : ApplyExpr(ExprKind::PrefixUnary, fn, argList, /*implicit*/ false, ty) {
    assert(argList->isUnlabeledUnary());
  }

public:
  static PrefixUnaryExpr *create(ASTContext &ctx, Expr *fn, Expr *operand,
                                 Type ty = Type());

  SourceLoc getLoc() const { return getFn()->getStartLoc(); }

  SourceLoc getStartLoc() const {
    return getFn()->getStartLoc();
  }
  SourceLoc getEndLoc() const { return getOperand()->getEndLoc(); }

  Expr *getOperand() const { return getArgs()->getExpr(0); }
  void setOperand(Expr *E) { getArgs()->setExpr(0, E); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::PrefixUnary;
  }
};

/// PostfixUnaryExpr - Postfix unary expressions like 'y!'.
class PostfixUnaryExpr : public ApplyExpr {
  PostfixUnaryExpr(Expr *fn, ArgumentList *argList, Type ty = Type())
      : ApplyExpr(ExprKind::PostfixUnary, fn, argList, /*implicit*/ false, ty) {
    assert(argList->isUnlabeledUnary());
  }

public:
  static PostfixUnaryExpr *create(ASTContext &ctx, Expr *fn, Expr *operand,
                                  Type ty = Type());

  SourceLoc getLoc() const { return getFn()->getStartLoc(); }

  SourceLoc getStartLoc() const { return getOperand()->getStartLoc(); }
  SourceLoc getEndLoc() const {
    return getFn()->getEndLoc();
  }

  Expr *getOperand() const { return getArgs()->getExpr(0); }
  void setOperand(Expr *E) { getArgs()->setExpr(0, E); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::PostfixUnary;
  }
};
  
/// BinaryExpr - Infix binary expressions like 'x+y'.  The argument is always
/// an implicit tuple expression of the type expected by the function.
class BinaryExpr : public ApplyExpr {
  BinaryExpr(Expr *fn, ArgumentList *argList, bool implicit, Type ty = Type())
      : ApplyExpr(ExprKind::Binary, fn, argList, implicit, ty) {
    assert(argList->size() == 2);
  }

public:
  static BinaryExpr *create(ASTContext &ctx, Expr *lhs, Expr *fn, Expr *rhs,
                            bool implicit, Type ty = Type());

  /// The left-hand argument of the binary operation.
  Expr *getLHS() const { return getArgs()->getExpr(0); }

  /// The right-hand argument of the binary operation.
  Expr *getRHS() const { return getArgs()->getExpr(1); }

  SourceLoc getLoc() const { return getFn()->getLoc(); }

  SourceRange getSourceRange() const { return getArgs()->getSourceRange(); }
  SourceLoc getStartLoc() const { return getArgs()->getStartLoc(); }
  SourceLoc getEndLoc() const { return getArgs()->getEndLoc(); }

  static bool classof(const Expr *E) { return E->getKind() == ExprKind::Binary;}
};

/// SelfApplyExpr - Abstract application that provides the 'self' pointer for
/// a method curried as (this : Self) -> (params) -> result.
///
/// The application of a curried method to 'self' semantically differs from
/// normal function application because the 'self' parameter can be implicitly
/// materialized from an rvalue.
class SelfApplyExpr : public ApplyExpr {
protected:
  SelfApplyExpr(ExprKind kind, Expr *fnExpr, ArgumentList *argList, Type ty)
      : ApplyExpr(kind, fnExpr, argList, fnExpr->isImplicit(), ty) {
    assert(argList->isUnary());
  }

public:
  Expr *getBase() const { return getArgs()->getExpr(0); }
  void setBase(Expr *E) { getArgs()->setExpr(0, E); }

  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_SelfApplyExpr &&
           E->getKind() <= ExprKind::Last_SelfApplyExpr;
  }
};

/// DotSyntaxCallExpr - Refer to a method of a type, e.g. P.x.  'x'
/// is modeled as a DeclRefExpr or OverloadSetRefExpr on the method.
class DotSyntaxCallExpr : public SelfApplyExpr {
  SourceLoc DotLoc;

  DotSyntaxCallExpr(Expr *fnExpr, SourceLoc dotLoc, ArgumentList *argList,
                    Type ty = Type())
      : SelfApplyExpr(ExprKind::DotSyntaxCall, fnExpr, argList, ty),
        DotLoc(dotLoc) {
    setImplicit(DotLoc.isInvalid());
  }

public:
  /// Create a new method reference to \p fnExpr on the base value \p baseArg.
  /// 
  /// If this is for a 'mutating' method, \p baseArg should be created using
  /// \c Argument::implicitInOut. Otherwise, \p Argument::unlabeled should be
  /// used. \p baseArg must not be labeled.
  static DotSyntaxCallExpr *create(ASTContext &ctx, Expr *fnExpr,
                                   SourceLoc dotLoc, Argument baseArg,
                                   Type ty = Type());

  SourceLoc getDotLoc() const { return DotLoc; }

  SourceLoc getLoc() const;
  SourceLoc getStartLoc() const;
  SourceLoc getEndLoc() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DotSyntaxCall;
  }
};

/// ConstructorRefCallExpr - Refer to a constructor for a type P.  The
/// actual reference to function which returns the constructor is modeled
/// as a DeclRefExpr.
class ConstructorRefCallExpr : public SelfApplyExpr {
  ConstructorRefCallExpr(Expr *fnExpr, ArgumentList *argList, Type ty = Type())
      : SelfApplyExpr(ExprKind::ConstructorRefCall, fnExpr, argList, ty) {}

public:
  static ConstructorRefCallExpr *create(ASTContext &ctx, Expr *fnExpr,
                                        Expr *baseExpr, Type ty = Type());

  SourceLoc getLoc() const { return getFn()->getLoc(); }
  SourceLoc getStartLoc() const { return getBase()->getStartLoc(); }
  SourceLoc getEndLoc() const { return getFn()->getEndLoc(); }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ConstructorRefCall;
  }
};
  
/// DotSyntaxBaseIgnoredExpr - When a.b resolves to something that does not need
/// the actual value of the base (e.g. when applied to a metatype, module, or
/// the base of a 'static' function) this expression node is created.  The
/// semantics are that its base is evaluated and discarded, then 'b' is
/// evaluated and returned as the result of the expression.
class DotSyntaxBaseIgnoredExpr : public Expr {
  Expr *LHS;
  SourceLoc DotLoc;
  Expr *RHS;
public:
  DotSyntaxBaseIgnoredExpr(Expr *LHS, SourceLoc DotLoc, Expr *RHS, Type rhsTy)
    : Expr(ExprKind::DotSyntaxBaseIgnored, /*Implicit=*/false, rhsTy),
      LHS(LHS), DotLoc(DotLoc), RHS(RHS) {
  }
  
  Expr *getLHS() const { return LHS; }
  void setLHS(Expr *E) { LHS = E; }
  SourceLoc getDotLoc() const { return DotLoc; }
  Expr *getRHS() const { return RHS; }
  void setRHS(Expr *E) { RHS = E; }

  SourceLoc getStartLoc() const {
    return DotLoc.isValid() ? LHS->getStartLoc() : RHS->getStartLoc();
  }
  SourceLoc getEndLoc() const { return RHS->getEndLoc(); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::DotSyntaxBaseIgnored;
  }
};
  
/// Represents an explicit cast, 'a as T' or 'a is T', where "T" is a
/// type, and "a" is the expression that will be converted to the type.
class ExplicitCastExpr : public Expr {
  Expr *SubExpr;
  SourceLoc AsLoc;
  TypeExpr *const CastTy;

protected:
  ExplicitCastExpr(ExprKind kind, Expr *sub, SourceLoc AsLoc, TypeExpr *castTy)
      : Expr(kind, /*Implicit=*/false), SubExpr(sub), AsLoc(AsLoc),
        CastTy(castTy) {}

public:
  Expr *getSubExpr() const { return SubExpr; }

  /// Get the type syntactically spelled in the cast. For some forms of checked
  /// cast this is different from the result type of the expression.
  Type getCastType() const { return CastTy->getInstanceType(); }
  void setCastType(Type type);

  TypeRepr *getCastTypeRepr() const { return CastTy->getTypeRepr(); }

  void setSubExpr(Expr *E) { SubExpr = E; }

  SourceLoc getLoc() const {
    if (AsLoc.isValid())
      return AsLoc;

    return SubExpr->getLoc();
  }

  SourceLoc getAsLoc() const {
    return AsLoc;
  }

  SourceRange getSourceRange() const {
    const SourceRange castTyRange = CastTy->getSourceRange();
    if (castTyRange.isInvalid())
      return SubExpr->getSourceRange();
    
    auto startLoc = SubExpr ? SubExpr->getStartLoc() : AsLoc;
    auto endLoc = castTyRange.End;
    
    return {startLoc, endLoc};
  }
  
  /// True if the node has been processed by SequenceExpr folding.
  bool isFolded() const { return SubExpr; }
  
  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_ExplicitCastExpr &&
           E->getKind() <= ExprKind::Last_ExplicitCastExpr;
  }
};

/// Return a string representation of a CheckedCastKind.
StringRef getCheckedCastKindName(CheckedCastKind kind);
  
/// Abstract base class for checked casts 'as' and 'is'. These represent
/// casts that can dynamically fail.
class CheckedCastExpr : public ExplicitCastExpr {
protected:
  CheckedCastExpr(ExprKind kind, Expr *sub, SourceLoc asLoc, TypeExpr *castTy)
      : ExplicitCastExpr(kind, sub, asLoc, castTy) {
    Bits.CheckedCastExpr.CastKind = unsigned(CheckedCastKind::Unresolved);
  }

public:
  /// Return the semantic kind of cast performed.
  CheckedCastKind getCastKind() const {
    return CheckedCastKind(Bits.CheckedCastExpr.CastKind);
  }
  void setCastKind(CheckedCastKind kind) {
    Bits.CheckedCastExpr.CastKind = unsigned(kind);
  }
  
  /// True if the cast has been type-checked and its kind has been set.
  bool isResolved() const {
    return getCastKind() >= CheckedCastKind::First_Resolved;
  }
  
  static bool classof(const Expr *E) {
    return E->getKind() >= ExprKind::First_CheckedCastExpr
        && E->getKind() <= ExprKind::Last_CheckedCastExpr;
  }
};

/// Represents an explicit forced checked cast, which converts
/// from a value of some type to some specified subtype and fails dynamically
/// if the value does not have that type.
/// Spelled 'a as! T' and produces a value of type 'T'.
class ForcedCheckedCastExpr final : public CheckedCastExpr {
  SourceLoc ExclaimLoc;

  ForcedCheckedCastExpr(Expr *sub, SourceLoc asLoc, SourceLoc exclaimLoc,
                        TypeExpr *type)
      : CheckedCastExpr(ExprKind::ForcedCheckedCast, sub, asLoc, type),
        ExclaimLoc(exclaimLoc) {}

public:
  static ForcedCheckedCastExpr *create(ASTContext &ctx, SourceLoc asLoc,
                                       SourceLoc exclaimLoc, TypeRepr *tyRepr);

  static ForcedCheckedCastExpr *createImplicit(ASTContext &ctx, Expr *sub,
                                               Type castTy);

  /// Retrieve the location of the '!' that follows 'as'.
  SourceLoc getExclaimLoc() const { return ExclaimLoc; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ForcedCheckedCast;
  }
};

/// Represents an explicit conditional checked cast, which converts
/// from a type to some subtype and produces an Optional value, which will be
/// .some(x) if the cast succeeds, or .none if the cast fails.
/// Spelled 'a as? T' and produces a value of type 'T?'.
class ConditionalCheckedCastExpr final : public CheckedCastExpr {
  SourceLoc QuestionLoc;

  ConditionalCheckedCastExpr(Expr *sub, SourceLoc asLoc, SourceLoc questionLoc,
                             TypeExpr *type)
      : CheckedCastExpr(ExprKind::ConditionalCheckedCast, sub, asLoc, type),
        QuestionLoc(questionLoc) {}

public:
  static ConditionalCheckedCastExpr *create(ASTContext &ctx, SourceLoc asLoc,
                                            SourceLoc questionLoc,
                                            TypeRepr *tyRepr);

  static ConditionalCheckedCastExpr *createImplicit(ASTContext &ctx, Expr *sub,
                                                    Type castTy);

  /// Retrieve the location of the '?' that follows 'as'.
  SourceLoc getQuestionLoc() const { return QuestionLoc; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ConditionalCheckedCast;
  }
};

/// Represents a runtime type check query, 'a is T', where 'T' is a type
/// and 'a' is a value of some related type. Evaluates to a Bool true if 'a' is
/// of the type and 'a as T' would succeed, false otherwise.
///
/// FIXME: We should support type queries with a runtime metatype value too.
class IsExpr final : public CheckedCastExpr {
  IsExpr(Expr *sub, SourceLoc isLoc, TypeExpr *type)
      : CheckedCastExpr(ExprKind::Is, sub, isLoc, type) {}

public:
  static IsExpr *create(ASTContext &ctx, SourceLoc isLoc, TypeRepr *tyRepr);

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Is;
  }
};

/// Represents an explicit coercion from a value to a specific type.
///
/// Spelled 'a as T' and produces a value of type 'T'.
class CoerceExpr final : public ExplicitCastExpr {
  /// Since there is already `asLoc` location,
  /// we use it to store `start` of the initializer
  /// call source range to save some storage.
  SourceLoc InitRangeEnd;

  CoerceExpr(Expr *sub, SourceLoc asLoc, TypeExpr *type)
      : ExplicitCastExpr(ExprKind::Coerce, sub, asLoc, type) {}

  CoerceExpr(SourceRange initRange, Expr *literal, TypeExpr *type)
      : ExplicitCastExpr(ExprKind::Coerce, literal, initRange.Start, type),
        InitRangeEnd(initRange.End) {}

public:
  static CoerceExpr *create(ASTContext &ctx, SourceLoc asLoc, TypeRepr *tyRepr);

  static CoerceExpr *createImplicit(ASTContext &ctx, Expr *sub, Type castTy);

  /// Create an implicit coercion expression for literal initialization
  /// preserving original source information, this way original call
  /// could be recreated if needed.
  static CoerceExpr *forLiteralInit(ASTContext &ctx, Expr *literal,
                                    SourceRange range, TypeRepr *literalTyRepr);

  bool isLiteralInit() const { return InitRangeEnd.isValid(); }

  SourceRange getSourceRange() const {
    return isLiteralInit()
            ? SourceRange(getAsLoc(), InitRangeEnd)
            : ExplicitCastExpr::getSourceRange();
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Coerce;
  }
};

/// Represents two expressions joined by the arrow operator '->', which
/// may be preceded by the 'throws' keyword. Currently this only exists to be
/// transformed into a FunctionTypeRepr by simplifyTypeExpr() in Sema.
class ArrowExpr : public Expr {
  SourceLoc AsyncLoc;
  SourceLoc ThrowsLoc;
  SourceLoc ArrowLoc;
  Expr *Args;
  Expr *Result;
  Expr *ThrownType;

public:
  ArrowExpr(Expr *Args, SourceLoc AsyncLoc, SourceLoc ThrowsLoc,
            Expr *ThrownType, SourceLoc ArrowLoc, Expr *Result)
    : Expr(ExprKind::Arrow, /*implicit=*/false, Type()),
      AsyncLoc(AsyncLoc), ThrowsLoc(ThrowsLoc), ArrowLoc(ArrowLoc), Args(Args),
      Result(Result), ThrownType(ThrownType)
  { }

  ArrowExpr(SourceLoc AsyncLoc, SourceLoc ThrowsLoc, Expr *ThrownType,
            SourceLoc ArrowLoc)
    : Expr(ExprKind::Arrow, /*implicit=*/false, Type()),
      AsyncLoc(AsyncLoc), ThrowsLoc(ThrowsLoc), ArrowLoc(ArrowLoc),
      Args(nullptr), Result(nullptr), ThrownType(ThrownType)
  { }

  Expr *getArgsExpr() const { return Args; }
  void setArgsExpr(Expr *E) { Args = E; }
  Expr *getResultExpr() const { return Result; }
  void setResultExpr(Expr *E) { Result = E; }
  Expr *getThrownTypeExpr() const { return ThrownType; }
  void setThrownTypeExpr(Expr *E) { ThrownType = E; }

  SourceLoc getAsyncLoc() const { return AsyncLoc; }
  SourceLoc getThrowsLoc() const { return ThrowsLoc; }
  SourceLoc getArrowLoc() const { return ArrowLoc; }
  bool isFolded() const { return Args != nullptr && Result != nullptr; }

  SourceLoc getSourceLoc() const { return ArrowLoc; }
  SourceLoc getStartLoc() const {
    return isFolded() ? Args->getStartLoc() :
           AsyncLoc.isValid() ? AsyncLoc :
           ThrowsLoc.isValid() ? ThrowsLoc : ArrowLoc;
  }
  SourceLoc getEndLoc() const {
    return isFolded() ? Result->getEndLoc() : ArrowLoc;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Arrow;
  }
};

/// Represents the rebinding of 'self' in a constructor that calls out
/// to another constructor. The result of the subexpression is assigned to
/// 'self', and the expression returns void.
///
/// When a super.init or delegating initializer is invoked, 'self' is
/// reassigned to the result of the initializer (after being downcast in the
/// case of super.init).
///
/// This is needed for reference types with ObjC interop, where
/// reassigning 'self' is a supported feature, and for value type delegating
/// constructors, where the delegatee constructor is responsible for
/// initializing 'self' in-place before the delegator's logic executes.
class RebindSelfInConstructorExpr : public Expr {
  Expr *SubExpr;
  VarDecl *Self;
public:
  RebindSelfInConstructorExpr(Expr *SubExpr, VarDecl *Self);
  
  LANGUAGE_FORWARD_SOURCE_LOCS_TO(SubExpr)
  
  VarDecl *getSelf() const { return Self; }
  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *Sub) { SubExpr = Sub; }

  OtherConstructorDeclRefExpr *getCalledConstructor(bool &isChainToSuper) const;
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::RebindSelfInConstructor;
  }
};

/// The ternary conditional expression 'x ? y : z'.
class TernaryExpr : public Expr {
  Expr *CondExpr, *ThenExpr, *ElseExpr;
  SourceLoc QuestionLoc, ColonLoc;
public:
  TernaryExpr(Expr *CondExpr, SourceLoc QuestionLoc, Expr *ThenExpr,
              SourceLoc ColonLoc, Expr *ElseExpr, Type Ty = Type())
      : Expr(ExprKind::Ternary, /*Implicit=*/false, Ty), CondExpr(CondExpr),
        ThenExpr(ThenExpr), ElseExpr(ElseExpr), QuestionLoc(QuestionLoc),
        ColonLoc(ColonLoc) {}

  TernaryExpr(SourceLoc QuestionLoc, Expr *ThenExpr, SourceLoc ColonLoc)
      : TernaryExpr(nullptr, QuestionLoc, ThenExpr, ColonLoc, nullptr) {}

  SourceLoc getLoc() const { return QuestionLoc; }
  SourceLoc getStartLoc() const {
    return (isFolded() ? CondExpr->getStartLoc() : QuestionLoc);
  }
  SourceLoc getEndLoc() const {
    return (isFolded() ? ElseExpr->getEndLoc() : ColonLoc);
  }
  SourceLoc getQuestionLoc() const { return QuestionLoc; }
  SourceLoc getColonLoc() const { return ColonLoc; }
  
  Expr *getCondExpr() const { return CondExpr; }
  void setCondExpr(Expr *E) { CondExpr = E; }
  
  Expr *getThenExpr() const { return ThenExpr; }
  void setThenExpr(Expr *E) { ThenExpr = E; }
  
  Expr *getElseExpr() const { return ElseExpr; }
  void setElseExpr(Expr *E) { ElseExpr = E; }
  
  /// True if the node has been processed by binary expression folding.
  bool isFolded() const { return CondExpr && ElseExpr; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Ternary;
  }
};

/// EnumIsCaseExpr - A boolean expression that is true if an enum value is of
/// a particular case.
class EnumIsCaseExpr : public Expr {
  Expr *SubExpr;
  TypeRepr *CaseRepr;
  EnumElementDecl *Element;
  
public:
  EnumIsCaseExpr(Expr *SubExpr, TypeRepr *CaseRepr, EnumElementDecl *Element)
      : Expr(ExprKind::EnumIsCase, /*implicit*/ true), SubExpr(SubExpr),
        CaseRepr(CaseRepr), Element(Element) {}

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *e) { SubExpr = e; }

  TypeRepr *getCaseTypeRepr() const { return CaseRepr; }

  EnumElementDecl *getEnumElement() const { return Element; }

  SourceLoc getLoc() const { return SubExpr->getLoc(); }
  SourceLoc getStartLoc() const { return SubExpr->getStartLoc(); }
  SourceLoc getEndLoc() const { return SubExpr->getEndLoc(); }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::EnumIsCase;
  }
};

/// AssignExpr - A value assignment, like "x = y".
class AssignExpr : public Expr {
  Expr *Dest;
  Expr *Src;
  SourceLoc EqualLoc;

public:  
  AssignExpr(Expr *Dest, SourceLoc EqualLoc, Expr *Src, bool Implicit)
    : Expr(ExprKind::Assign, Implicit),
      Dest(Dest), Src(Src), EqualLoc(EqualLoc) {}
  
  AssignExpr(SourceLoc EqualLoc)
    : AssignExpr(nullptr, EqualLoc, nullptr, /*Implicit=*/false)
  {}

  Expr *getDest() const { return Dest; }
  void setDest(Expr *e) { Dest = e; }
  Expr *getSrc() const { return Src; }
  void setSrc(Expr *e) { Src = e; }
  
  SourceLoc getEqualLoc() const { return EqualLoc; }
  
  SourceLoc getLoc() const {
    SourceLoc loc = EqualLoc;
    if (loc.isValid()) {
      return loc;
    }
    return getStartLoc();
  }
  SourceLoc getStartLoc() const {
    if (!isFolded()) return EqualLoc;
    return ( Dest->getStartLoc().isValid()
           ? Dest->getStartLoc()
           : Src->getStartLoc());
  }
  SourceLoc getEndLoc() const {
    if (!isFolded()) return EqualLoc;
    auto SrcEnd = Src->getEndLoc();
    return (SrcEnd.isValid() ? SrcEnd : Dest->getEndLoc());
  }
  
  /// True if the node has been processed by binary expression folding.
  bool isFolded() const { return Dest && Src; }
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::Assign;
  }
};

/// A pattern production that has been parsed but hasn't been resolved
/// into a complete pattern. Pattern checking converts these into standalone pattern
/// nodes or raises an error if a pattern production appears in an invalid
/// position.
class UnresolvedPatternExpr : public Expr {
  Pattern *subPattern;

public:
  explicit UnresolvedPatternExpr(Pattern *subPattern)
    : Expr(ExprKind::UnresolvedPattern, /*Implicit=*/false),
      subPattern(subPattern) { }
  
  const Pattern *getSubPattern() const { return subPattern; }
  Pattern *getSubPattern() { return subPattern; }
  void setSubPattern(Pattern *p) { subPattern = p; }
  
  SourceRange getSourceRange() const;
  SourceLoc getStartLoc() const;
  SourceLoc getEndLoc() const;
  SourceLoc getLoc() const;
  
  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::UnresolvedPattern;
  }
};


/// An editor placeholder (<#such as this#>) that occurred in an expression
/// context. If the placeholder is a typed one (see \c EditorPlaceholderData)
/// its type string will be typechecked and will be associated with this expr.
class EditorPlaceholderExpr : public Expr {
  Identifier Placeholder;
  SourceLoc Loc;
  TypeRepr *PlaceholderTy;
  TypeRepr *ExpansionTyR;
  Expr *SemanticExpr;

public:
  EditorPlaceholderExpr(Identifier Placeholder, SourceLoc Loc,
                        TypeRepr *PlaceholderTy, TypeRepr *ExpansionTyR)
      : Expr(ExprKind::EditorPlaceholder, /*Implicit=*/false),
        Placeholder(Placeholder), Loc(Loc), PlaceholderTy(PlaceholderTy),
        ExpansionTyR(ExpansionTyR), SemanticExpr(nullptr) {}

  Identifier getPlaceholder() const { return Placeholder; }
  SourceRange getSourceRange() const { return Loc; }
  TypeRepr *getPlaceholderTypeRepr() const { return PlaceholderTy; }

  /// The TypeRepr to be considered for placeholder expansion.
  TypeRepr *getTypeForExpansion() const { return ExpansionTyR; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::EditorPlaceholder;
  }

  Expr *getSemanticExpr() const { return SemanticExpr; }
  void setSemanticExpr(Expr *SE) { SemanticExpr = SE; }
};

/// A LazyInitializerExpr is used to embed an existing typechecked
/// expression --- like the initializer of a lazy variable --- into an
/// untypechecked AST.
class LazyInitializerExpr : public Expr {
  Expr *SubExpr;
public:
  LazyInitializerExpr(Expr *subExpr)
    : Expr(ExprKind::LazyInitializer, /*implicit*/ true),
      SubExpr(subExpr) {}

  SourceRange getSourceRange() const { return SubExpr->getSourceRange(); }
  SourceLoc getStartLoc() const { return SubExpr->getStartLoc(); }
  SourceLoc getEndLoc() const { return SubExpr->getEndLoc(); }
  SourceLoc getLoc() const { return SubExpr->getLoc(); }

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *subExpr) { SubExpr = subExpr; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::LazyInitializer;
  }
};

/// Produces the Objective-C selector of the referenced method.
///
/// \code
/// #selector(UIView.insertSubview(_:aboveSubview:))
/// \endcode
class ObjCSelectorExpr : public Expr {
  SourceLoc KeywordLoc;
  SourceLoc LParenLoc;
  SourceLoc ModifierLoc;
  Expr *SubExpr;
  SourceLoc RParenLoc;
  AbstractFunctionDecl *ResolvedMethod = nullptr;

public:
  /// The kind of #selector expression this is.
  enum ObjCSelectorKind {
    Method, Getter, Setter
  };

  ObjCSelectorExpr(ObjCSelectorKind kind, SourceLoc keywordLoc,
                   SourceLoc lParenLoc, SourceLoc modifierLoc, Expr *subExpr,
                   SourceLoc rParenLoc)
    : Expr(ExprKind::ObjCSelector, /*Implicit=*/false),
      KeywordLoc(keywordLoc), LParenLoc(lParenLoc),
      ModifierLoc(modifierLoc), SubExpr(subExpr), RParenLoc(rParenLoc) {
    Bits.ObjCSelectorExpr.SelectorKind = static_cast<unsigned>(kind);
  }

  Expr *getSubExpr() const { return SubExpr; }
  void setSubExpr(Expr *expr) { SubExpr = expr; }

  /// Whether this selector references a property getter or setter.
  bool isPropertySelector() const {
    switch (getSelectorKind()) {
    case ObjCSelectorKind::Method:
      return false;

    case ObjCSelectorKind::Getter:
    case ObjCSelectorKind::Setter:
      return true;
    }

    toolchain_unreachable("Unhandled ObjcSelectorKind in switch.");
  }

  /// Whether this selector references a method.
  bool isMethodSelector() const {
    switch (getSelectorKind()) {
    case ObjCSelectorKind::Method:
      return true;

    case ObjCSelectorKind::Getter:
    case ObjCSelectorKind::Setter:
      return false;
    }
  }

  /// Retrieve the Objective-C method to which this expression refers.
  AbstractFunctionDecl *getMethod() const { return ResolvedMethod; }

  /// Set the Objective-C method to which this expression refers.
  void setMethod(AbstractFunctionDecl *method) { ResolvedMethod = method; }

  SourceLoc getLoc() const { return KeywordLoc; }
  SourceRange getSourceRange() const {
    return SourceRange(KeywordLoc, RParenLoc);
  }

  /// The location at which the getter: or setter: starts. Requires the selector
  /// to be a getter or setter.
  SourceLoc getModifierLoc() const {
    assert(isPropertySelector() && "Modifiers only set on property selectors");
    return ModifierLoc;
  }

  /// Retrieve the kind of the selector (method, getter, setter)
  ObjCSelectorKind getSelectorKind() const {
    return static_cast<ObjCSelectorKind>(Bits.ObjCSelectorExpr.SelectorKind);
  }

  /// Override the selector kind.
  ///
  /// Used by the type checker to recover from ill-formed #selector
  /// expressions.
  void overrideObjCSelectorKind(ObjCSelectorKind newKind,
                                SourceLoc modifierLoc) {
    Bits.ObjCSelectorExpr.SelectorKind = static_cast<unsigned>(newKind);
    ModifierLoc = modifierLoc;
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::ObjCSelector;
  }
};

/// Produces a keypath string for the given referenced property.
///
/// \code
/// #keyPath(Person.friends.firstName)
/// \endcode
class KeyPathExpr : public Expr {
  SourceLoc StartLoc;
  SourceLoc LParenLoc;
  SourceLoc EndLoc;
  Expr *ObjCStringLiteralExpr = nullptr;

  // The parsed root of a Codira keypath (the section before an unusual dot, like
  // Foo.Bar in \Foo.Bar.?.baz).
  Expr *ParsedRoot = nullptr;
  // The parsed path of a Codira keypath (the section after an unusual dot, like
  // ?.baz in \Foo.Bar.?.baz).
  Expr *ParsedPath = nullptr;

  // The processed/resolved type, like Foo.Bar in \Foo.Bar.?.baz.
  TypeRepr *RootType = nullptr;

  /// Determines whether a key path starts with '.' which denotes necessity for
  /// a contextual root type.
  bool HasLeadingDot = false;

public:
  /// A single stored component, which will be one of:
  /// - an unresolved DeclNameRef, which has to be type-checked
  /// - a resolved ValueDecl, referring to
  /// - a subscript index expression, which may or may not be resolved
  /// - an optional chaining, forcing, or wrapping component
  /// - a code completion token
  class Component {
  public:
    enum class Kind : unsigned {
      Invalid,
      UnresolvedApply,
      UnresolvedMember,
      UnresolvedSubscript,
      Apply,
      Member,
      Subscript,
      OptionalForce,
      OptionalChain,
      OptionalWrap,
      Identity,
      TupleElement,
      DictionaryKey,
      CodeCompletion,
    };

  private:
    union DeclNameOrRef {
      DeclNameRef UnresolvedName;
      ConcreteDeclRef ResolvedDecl;
      
      DeclNameOrRef() : UnresolvedName{} {}
      DeclNameOrRef(DeclNameRef un) : UnresolvedName(un) {}
      DeclNameOrRef(ConcreteDeclRef rd) : ResolvedDecl(rd) {}
    } Decl;

    ArgumentList *ArgList;
    const ProtocolConformanceRef *HashableConformancesData;
    std::optional<FunctionRefInfo> ComponentFuncRefKind;

    unsigned TupleIndex;
    Kind KindValue;
    Type ComponentType;
    SourceLoc Loc;

    // Private constructor for unresolvedSubscript, subscript,
    // unresolvedApply and apply component.
    explicit Component(DeclNameOrRef decl, ArgumentList *argList,
                       ArrayRef<ProtocolConformanceRef> indexHashables,
                       Kind kind, Type type, SourceLoc loc);

    // Private constructor for property or #keyPath dictionary key.
    explicit Component(DeclNameOrRef decl, Kind kind, Type type, SourceLoc loc)
        : Component(kind, type, loc) {
      assert(kind == Kind::Member || kind == Kind::UnresolvedMember ||
             kind == Kind::DictionaryKey);
      Decl = decl;
    }

    // Private constructor for an unresolvedMember or member kind.
    explicit Component(DeclNameOrRef decl, Kind kind, Type type,
                       FunctionRefInfo funcRefKind, SourceLoc loc)
        : Decl(decl), ComponentFuncRefKind(funcRefKind), KindValue(kind),
          ComponentType(type), Loc(loc) {
      assert(kind == Kind::Member || kind == Kind::UnresolvedMember);
    }

    // Private constructor for tuple element kind.
    explicit Component(unsigned tupleIndex, Type elementType, SourceLoc loc)
        : Component(Kind::TupleElement, elementType, loc) {
      TupleIndex = tupleIndex;
    }

    // Private constructor for basic components with no additional information.
    explicit Component(Kind kind, Type type, SourceLoc loc)
        : Decl(), ComponentFuncRefKind(std::nullopt), KindValue(kind),
          ComponentType(type), Loc(loc) {}

  public:
    Component() : Component(Kind::Invalid, Type(), SourceLoc()) {}

    /// Create an unresolved component for an unresolved apply.
    static Component forUnresolvedApply(ASTContext &ctx,
                                        ArgumentList *argList) {
      return Component({}, argList, {}, Kind::UnresolvedApply, Type(),
                       argList->getLParenLoc());
    };

    /// Create an unresolved component for an unresolved member.
    static Component forUnresolvedMember(DeclNameRef UnresolvedName,
                                         FunctionRefInfo funcRefKind,
                                         SourceLoc loc) {
      return Component(UnresolvedName, Kind::UnresolvedMember, Type(),
                       funcRefKind, loc);
    }

    /// Create an unresolved component for a subscript.
    static Component forUnresolvedSubscript(ASTContext &ctx,
                                            ArgumentList *argList);

    /// Create an unresolved optional force `!` component.
    static Component forUnresolvedOptionalForce(SourceLoc BangLoc) {
      return Component(Kind::OptionalForce, Type(), BangLoc);
    }
    
    /// Create an unresolved optional chain `?` component.
    static Component forUnresolvedOptionalChain(SourceLoc QuestionLoc) {
      return Component(Kind::OptionalChain, Type(), QuestionLoc);
    }

    /// Create a component for resolved application.
    static Component forApply(ArgumentList *argList, Type applyType,
                              ArrayRef<ProtocolConformanceRef> indexHashables) {
      return Component({}, argList, indexHashables, Kind::Apply, applyType,
                       argList->getLParenLoc());
    };

    /// Create a component for a property.
    static Component forMember(ConcreteDeclRef property, Type propertyType,
                               SourceLoc loc) {
      return Component(property, Kind::Member, propertyType, loc);
    }

    /// Create a component for a dictionary key (#keyPath only).
    static Component forDictionaryKey(DeclNameRef UnresolvedName,
                                      Type valueType,
                                      SourceLoc loc) {
      return Component(UnresolvedName, Kind::DictionaryKey, valueType, loc);
    }
    
    /// Create a component for a subscript.
    static Component
    forSubscript(ASTContext &ctx, ConcreteDeclRef subscript,
                 ArgumentList *argList, Type elementType,
                 ArrayRef<ProtocolConformanceRef> indexHashables);

    /// Create an optional-forcing `!` component.
    static Component forOptionalForce(Type forcedType, SourceLoc bangLoc) {
      return Component(Kind::OptionalForce, forcedType, bangLoc);
    }
    
    /// Create an optional-chaining `?` component.
    static Component forOptionalChain(Type unwrappedType,
                                      SourceLoc questionLoc) {
      return Component(Kind::OptionalChain, unwrappedType, questionLoc);
    }
    
    /// Create an optional-wrapping component. This doesn't have a surface
    /// syntax but may appear when the non-optional result of an optional chain
    /// is implicitly wrapped.
    static Component forOptionalWrap(Type wrappedType) {
      return Component(Kind::OptionalWrap, wrappedType, SourceLoc());
    }
    
    static Component forIdentity(SourceLoc selfLoc) {
      return Component(Kind::Identity, Type(), selfLoc);
    }
    
    static Component forTupleElement(unsigned fieldNumber,
                                     Type elementType,
                                     SourceLoc loc) {
      return Component(fieldNumber, elementType, loc);
    }

    static Component forCodeCompletion(SourceLoc Loc) {
      return Component(Kind::CodeCompletion, Type(), Loc);
    }

    SourceLoc getLoc() const {
      return Loc;
    }
    
    SourceRange getSourceRange() const {
      if (auto *args = getArgs()) {
        return args->getSourceRange();
      }
      return Loc;
    }
    
    Kind getKind() const {
      return KindValue;
    }
    
    bool isValid() const {
      return getKind() != Kind::Invalid;
    }

    bool isResolved() const {
      if (!getComponentType())
        return false;

      switch (getKind()) {
      case Kind::Subscript:
      case Kind::OptionalChain:
      case Kind::OptionalWrap:
      case Kind::OptionalForce:
      case Kind::Member:
      case Kind::Apply:
      case Kind::Identity:
      case Kind::TupleElement:
      case Kind::DictionaryKey:
        return true;

      case Kind::UnresolvedSubscript:
      case Kind::UnresolvedMember:
      case Kind::UnresolvedApply:
      case Kind::Invalid:
      case Kind::CodeCompletion:
        return false;
      }
      toolchain_unreachable("unhandled kind");
    }

    ArgumentList *getArgs() const {
      switch (getKind()) {
      case Kind::Subscript:
      case Kind::UnresolvedSubscript:
      case Kind::UnresolvedApply:
      case Kind::Apply:
        return ArgList;

      case Kind::Invalid:
      case Kind::OptionalChain:
      case Kind::OptionalWrap:
      case Kind::OptionalForce:
      case Kind::UnresolvedMember:
      case Kind::Member:
      case Kind::Identity:
      case Kind::TupleElement:
      case Kind::DictionaryKey:
      case Kind::CodeCompletion:
        return nullptr;
      }
      toolchain_unreachable("unhandled kind");
    }

    void setArgs(ArgumentList *newArgs) {
      assert(getArgs() && "Should be replacing existing args");
      ArgList = newArgs;
    }

    ArrayRef<ProtocolConformanceRef> getIndexHashableConformances() const {
      switch (getKind()) {
      case Kind::Subscript:
      case Kind::Apply:
        if (!HashableConformancesData)
          return {};
        return {HashableConformancesData, ArgList->size()};

      case Kind::UnresolvedSubscript:
      case Kind::Invalid:
      case Kind::OptionalChain:
      case Kind::OptionalWrap:
      case Kind::OptionalForce:
      case Kind::UnresolvedApply:
      case Kind::UnresolvedMember:
      case Kind::Member:
      case Kind::Identity:
      case Kind::TupleElement:
      case Kind::DictionaryKey:
      case Kind::CodeCompletion:
        return {};
      }
      toolchain_unreachable("unhandled kind");
    }

    DeclNameRef getUnresolvedDeclName() const {
      switch (getKind()) {
      case Kind::UnresolvedMember:
      case Kind::DictionaryKey:
        return Decl.UnresolvedName;

      case Kind::Invalid:
      case Kind::Subscript:
      case Kind::UnresolvedSubscript:
      case Kind::OptionalChain:
      case Kind::OptionalWrap:
      case Kind::OptionalForce:
      case Kind::UnresolvedApply:
      case Kind::Apply:
      case Kind::Member:
      case Kind::Identity:
      case Kind::TupleElement:
      case Kind::CodeCompletion:
        toolchain_unreachable("no unresolved name for this kind");
      }
      toolchain_unreachable("unhandled kind");
    }

    bool hasDeclRef() const {
      switch (getKind()) {
      case Kind::Member:
      case Kind::Subscript:
        return true;

      case Kind::Invalid:
      case Kind::UnresolvedMember:
      case Kind::UnresolvedSubscript:
      case Kind::UnresolvedApply:
      case Kind::Apply:
      case Kind::OptionalChain:
      case Kind::OptionalWrap:
      case Kind::OptionalForce:
      case Kind::Identity:
      case Kind::TupleElement:
      case Kind::DictionaryKey:
      case Kind::CodeCompletion:
        return false;
      }
      toolchain_unreachable("unhandled kind");
    }

    ConcreteDeclRef getDeclRef() const {
      switch (getKind()) {
      case Kind::Member:
      case Kind::Subscript:
        return Decl.ResolvedDecl;

      case Kind::Invalid:
      case Kind::UnresolvedMember:
      case Kind::UnresolvedSubscript:
      case Kind::UnresolvedApply:
      case Kind::Apply:
      case Kind::OptionalChain:
      case Kind::OptionalWrap:
      case Kind::OptionalForce:
      case Kind::Identity:
      case Kind::TupleElement:
      case Kind::DictionaryKey:
      case Kind::CodeCompletion:
        toolchain_unreachable("no decl ref for this kind");
      }
      toolchain_unreachable("unhandled kind");
    }
      
    unsigned getTupleIndex() const {
      switch (getKind()) {
        case Kind::TupleElement:
          return TupleIndex;
                
        case Kind::Invalid:
        case Kind::UnresolvedMember:
        case Kind::UnresolvedSubscript:
        case Kind::UnresolvedApply:
        case Kind::Apply:
        case Kind::OptionalChain:
        case Kind::OptionalWrap:
        case Kind::OptionalForce:
        case Kind::Identity:
        case Kind::Member:
        case Kind::Subscript:
        case Kind::DictionaryKey:
        case Kind::CodeCompletion:
          toolchain_unreachable("no field number for this kind");
      }
      toolchain_unreachable("unhandled kind");
    }

    FunctionRefInfo getFunctionRefInfo() const {
      switch (getKind()) {
      case Kind::UnresolvedMember:
        assert(ComponentFuncRefKind &&
               "FunctionRefInfo should not be nullopt for UnresolvedMember");
        return *ComponentFuncRefKind;

      case Kind::Member:
      case Kind::Subscript:
      case Kind::Invalid:
      case Kind::UnresolvedSubscript:
      case Kind::OptionalChain:
      case Kind::OptionalWrap:
      case Kind::OptionalForce:
      case Kind::Identity:
      case Kind::TupleElement:
      case Kind::DictionaryKey:
      case Kind::CodeCompletion:
      case Kind::UnresolvedApply:
      case Kind::Apply:
        toolchain_unreachable("no function ref kind for this kind");
      }
      toolchain_unreachable("unhandled kind");
    }

    Type getComponentType() const {
      return ComponentType;
    }
    
    void setComponentType(Type t) {
      ComponentType = t;
    }
  };

private:
  toolchain::MutableArrayRef<Component> Components;

  KeyPathExpr(SourceLoc startLoc, Expr *parsedRoot, Expr *parsedPath,
              SourceLoc endLoc, bool hasLeadingDot, bool isObjC,
              bool isImplicit);

  /// Create a key path with unresolved root and path expressions.
  KeyPathExpr(SourceLoc backslashLoc, Expr *parsedRoot, Expr *parsedPath,
              bool hasLeadingDot, bool isImplicit);

  /// Create a key path with components.
  KeyPathExpr(ASTContext &ctx, SourceLoc startLoc,
              ArrayRef<Component> components, SourceLoc endLoc, bool isObjC,
              bool isImplicit);

public:
  /// Create a new parsed Codira key path expression.
  static KeyPathExpr *createParsed(ASTContext &ctx, SourceLoc backslashLoc,
     Expr *parsedRoot, Expr *parsedPath,
     bool hasLeadingDot);

  /// Create a new parsed #keyPath expression.
  static KeyPathExpr *createParsedPoundKeyPath(ASTContext &ctx,
                                               SourceLoc keywordLoc,
                                               SourceLoc lParenLoc,
                                               ArrayRef<Component> components,
                                               SourceLoc rParenLoc);

  /// Create an implicit Codira key path expression with a set of resolved
  /// components.
  static KeyPathExpr *createImplicit(ASTContext &ctx, SourceLoc backslashLoc,
                                     ArrayRef<Component> components,
                                     SourceLoc endLoc);

  /// Create an implicit Codira key path expression with a root and path
  /// expression to be resolved.
  static KeyPathExpr *createImplicit(ASTContext &ctx, SourceLoc backslashLoc,
                                     Expr *parsedRoot, Expr *parsedPath,
                                     bool hasLeadingDot);

  SourceLoc getLoc() const { return StartLoc; }
  SourceRange getSourceRange() const { return SourceRange(StartLoc, EndLoc); }

  /// Get the components array.
  ArrayRef<Component> getComponents() const {
    return Components;
  }
  MutableArrayRef<Component> getMutableComponents() {
    return Components;
  }
  
  /// Set the key path components. This copies over the components from the
  /// argument array.
  void setComponents(ASTContext &C, ArrayRef<Component> newComponents);

  /// Indicates if the key path expression is composed by a single invalid
  /// component. e.g. missing component `\Root`
  bool hasSingleInvalidComponent() const {
    if (ParsedRoot && ParsedRoot->getKind() == ExprKind::Type) {
      return Components.size() == 1 && !Components.front().isValid();
    }
    return false;
  }

  /// If the provided expression appears as an argument to a subscript component
  /// of the key path, returns the index of that component. Otherwise, returns
  /// \c None.
  std::optional<unsigned> findComponentWithSubscriptArg(Expr *arg);

  /// Retrieve the string literal expression, which will be \c NULL prior to
  /// type checking and a string literal after type checking for an
  /// @objc key path.
  Expr *getObjCStringLiteralExpr() const {
    return ObjCStringLiteralExpr;
  }

  /// Set the semantic expression.
  void setObjCStringLiteralExpr(Expr *expr) {
    ObjCStringLiteralExpr = expr;
  }

  Expr *getParsedRoot() const {
    assert(!isObjC() && "cannot get parsed root of ObjC keypath");
    return ParsedRoot;
  }
  void setParsedRoot(Expr *root) {
    assert(!isObjC() && "cannot get parsed root of ObjC keypath");
    ParsedRoot = root;
  }

  Expr *getParsedPath() const {
    assert(!isObjC() && "cannot get parsed path of ObjC keypath");
    return ParsedPath;
  }
  void setParsedPath(Expr *path) {
    assert(!isObjC() && "cannot set parsed path of ObjC keypath");
    ParsedPath = path;
  }

  TypeRepr *getExplicitRootType() const {
    assert(!isObjC() && "cannot get root type of ObjC keypath");
    return RootType;
  }
  void setExplicitRootType(TypeRepr *rootType) {
    assert(!isObjC() && "cannot set root type of ObjC keypath");
    RootType = rootType;
  }

  /// True if this is an ObjC key path expression.
  bool isObjC() const { return Bits.KeyPathExpr.IsObjC; }

  /// True if this key path expression has a leading dot.
  bool expectsContextualRoot() const { return HasLeadingDot; }

  BoundGenericType *getKeyPathType() const;

  Type getRootType() const;
  Type getValueType() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::KeyPath;
  }
};

/// Provides a value of type `(any Actor)?` that represents the actor
/// isolation of the current context, or `nil` if this is non-isolated
/// code.
///
/// This expression node is implicitly created by the type checker, and
/// has no in-source spelling.
class CurrentContextIsolationExpr : public Expr {
  Expr *actorExpr = nullptr;
  SourceLoc implicitLoc;

public:
  CurrentContextIsolationExpr(SourceLoc implicitLoc, Type type)
      : Expr(ExprKind::CurrentContextIsolation, /*isImplicit=*/true, type),
        implicitLoc(implicitLoc) {}

  /// The expression that produces the actor isolation value.
  Expr *getActor() const { return actorExpr; }

  void setActor(Expr *expr) {
    actorExpr = expr;
  }

  SourceLoc getLoc() const { return implicitLoc; }

  SourceRange getSourceRange() const {
    return SourceRange(implicitLoc, implicitLoc);
  }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::CurrentContextIsolation;
  }
};

/// Represents the unusual behavior of a . in a \ keypath expression, such as
/// \.[0] and \Foo.?.
class KeyPathDotExpr : public Expr {
  SourceLoc DotLoc;

public:
  KeyPathDotExpr(SourceLoc dotLoc)
      : Expr(ExprKind::KeyPathDot, /*isImplicit=*/true), DotLoc(dotLoc) {}

  SourceLoc getLoc() const { return DotLoc; }
  SourceRange getSourceRange() const { return SourceRange(DotLoc, DotLoc); }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::KeyPathDot;
  }
};

/// An expression that may wrap a statement which produces a single value.
class SingleValueStmtExpr : public Expr {
public:
  enum class Kind {
    If, Switch, Do, DoCatch
  };

private:
  Stmt *S;
  DeclContext *DC;

  SingleValueStmtExpr(Stmt *S, DeclContext *DC)
      : Expr(ExprKind::SingleValueStmt, /*isImplicit*/ true), S(S), DC(DC) {}

  /// Creates a new SingleValueStmtExpr wrapping a statement.
  static SingleValueStmtExpr *create(ASTContext &ctx, Stmt *S, DeclContext *DC);

public:
  /// Creates a new SingleValueStmtExpr wrapping a statement, and recursively
  /// attempts to wrap any branches of that statement that can become single
  /// value statement expressions.
  ///
  /// If \p mustBeExpr is true, branches will be eagerly wrapped even if they
  /// may not be valid SingleValueStmtExprs (which Sema will later diagnose).
  static SingleValueStmtExpr *createWithWrappedBranches(ASTContext &ctx,
                                                        Stmt *S,
                                                        DeclContext *DC,
                                                        bool mustBeExpr);

  /// Attempt to look through valid parent expressions to a child
  /// SingleValueStmtExpr.
  static SingleValueStmtExpr *tryDigOutSingleValueStmtExpr(Expr *E);

  /// Whether the last ASTNode in the given BraceStmt can potentially be used as
  /// the implicit result for a SingleValueStmtExpr. If \p mustBeSingleValueStmt
  /// is \c true, a result will be considered even if it may not be valid.
  static bool isLastElementImplicitResult(BraceStmt *BS, ASTContext &ctx,
                                          bool mustBeSingleValueStmt);

  /// Retrieves a resulting ThenStmt from the given BraceStmt, or \c nullptr if
  /// the brace does not have a resulting ThenStmt.
  static ThenStmt *getThenStmtFrom(BraceStmt *BS);

  /// Whether the given BraceStmt has a result to be produced from a parent
  /// SingleValueStmtExpr. Note this does not consider elements that may
  /// implicitly become results, check \c isLastElementImplicitResult for that.
  static bool hasResult(BraceStmt *BS) {
    return getThenStmtFrom(BS);
  }

  /// Retrieve the wrapped statement.
  Stmt *getStmt() const { return S; }
  void setStmt(Stmt *newS) { S = newS; }

  /// Retrieve the kind of statement being wrapped.
  Kind getStmtKind() const;

  /// Retrieve the complete set of branches for the underlying statement.
  ArrayRef<Stmt *> getBranches(SmallVectorImpl<Stmt *> &scratch) const;

  /// Retrieve the resulting ThenStmts from each branch of the
  /// SingleValueStmtExpr.
  ArrayRef<ThenStmt *>
  getThenStmts(SmallVectorImpl<ThenStmt *> &scratch) const;

  /// Retrieve the result expressions from each branch of the
  /// SingleValueStmtExpr.
  ArrayRef<Expr *>
  getResultExprs(SmallVectorImpl<Expr *> &scratch) const;

  DeclContext *getDeclContext() const { return DC; }

  SourceRange getSourceRange() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::SingleValueStmt;
  }
};

class TypeJoinExpr final : public Expr,
                           private toolchain::TrailingObjects<TypeJoinExpr, Expr *> {
  friend TrailingObjects;

  DeclRefExpr *Var;

  /// If this is joining the expression branches for a SingleValueStmtExpr,
  /// this holds the expr node. Otherwise, it is \c nullptr.
  SingleValueStmtExpr *SVE;

  size_t numTrailingObjects() const {
    return getNumElements();
  }

  MutableArrayRef<Expr *> getMutableElements() {
    return { getTrailingObjects<Expr *>(), getNumElements() };
  }

  TypeJoinExpr(toolchain::PointerUnion<DeclRefExpr *, TypeBase *> result,
               ArrayRef<Expr *> elements, SingleValueStmtExpr *SVE);

  static TypeJoinExpr *
  createImpl(ASTContext &ctx,
             toolchain::PointerUnion<DeclRefExpr *, TypeBase *> varOrType,
             ArrayRef<Expr *> elements,
             AllocationArena arena = AllocationArena::Permanent,
             SingleValueStmtExpr *SVE = nullptr);

public:
  static TypeJoinExpr *
  create(ASTContext &ctx, DeclRefExpr *var, ArrayRef<Expr *> exprs,
         AllocationArena arena = AllocationArena::Permanent) {
    return createImpl(ctx, var, exprs, arena);
  }

  static TypeJoinExpr *
  create(ASTContext &ctx, Type joinType, ArrayRef<Expr *> exprs,
         AllocationArena arena = AllocationArena::Permanent) {
    return createImpl(ctx, joinType.getPointer(), exprs, arena);
  }

  /// Create a join for the branch types of a SingleValueStmtExpr.
  static TypeJoinExpr *
  forBranchesOfSingleValueStmtExpr(ASTContext &ctx, Type joinType,
                                   SingleValueStmtExpr *SVE,
                                   AllocationArena arena);

  SourceLoc getLoc() const { return SourceLoc(); }
  SourceRange getSourceRange() const { return SourceRange(); }

  DeclRefExpr *getVar() const { return Var; }

  void setVar(DeclRefExpr *var) {
    assert(var && "cannot set variable reference to null");
    Var = var;
  }

  ArrayRef<Expr *> getElements() const {
    return { getTrailingObjects<Expr *>(), getNumElements() };
  }

  Expr *getElement(unsigned i) const {
    return getElements()[i];
  }

  void setElement(unsigned i, Expr *E) {
    getMutableElements()[i] = E;
  }

  /// If this is joining the expression branches for a SingleValueStmtExpr,
  /// this returns the expr node. Otherwise, returns \c nullptr.
  SingleValueStmtExpr *getSingleValueStmtExpr() const { return SVE; }

  unsigned getNumElements() const { return Bits.TypeJoinExpr.NumElements; }

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::TypeJoin;
  }
};

/// An invocation of a macro expansion, spelled with `#` for freestanding
/// macros or `@` for attached macros.
class MacroExpansionExpr final : public Expr,
                                 public FreestandingMacroExpansion {
private:
  DeclContext *DC;
  Expr *Rewritten;
  MacroRoles Roles;
  MacroExpansionDecl *SubstituteDecl;

public:
  enum : unsigned { InvalidDiscriminator = 0xFFFF };

  explicit MacroExpansionExpr(DeclContext *dc, MacroExpansionInfo *info,
                              MacroRoles roles, bool isImplicit = false,
                              Type ty = Type())
      : Expr(ExprKind::MacroExpansion, isImplicit, ty),
        FreestandingMacroExpansion(FreestandingMacroKind::Expr, info), DC(dc),
        Rewritten(nullptr), Roles(roles), SubstituteDecl(nullptr) {
  }

  static MacroExpansionExpr *
  create(DeclContext *dc, SourceLoc sigilLoc,
         DeclNameRef moduleName, DeclNameLoc moduleNameLoc,
         DeclNameRef macroName, DeclNameLoc macroNameLoc,
         SourceLoc leftAngleLoc,
         ArrayRef<TypeRepr *> genericArgs, SourceLoc rightAngleLoc,
         ArgumentList *argList, MacroRoles roles, bool isImplicit = false,
         Type ty = Type());

  Expr *getRewritten() const { return Rewritten; }
  void setRewritten(Expr *rewritten) { Rewritten = rewritten; }

  ArgumentList *getArgs() const {
    return FreestandingMacroExpansion::getArgs();
  }

  MacroRoles getMacroRoles() const { return Roles; }

  SourceLoc getLoc() const { return getPoundLoc(); }

  DeclContext *getDeclContext() const { return DC; }
  void setDeclContext(DeclContext *dc) { DC = dc; }

  SourceRange getSourceRange() const {
    return getExpansionInfo()->getSourceRange();
  }

  MacroExpansionDecl *createSubstituteDecl();
  MacroExpansionDecl *getSubstituteDecl() const;

  static bool classof(const Expr *E) {
    return E->getKind() == ExprKind::MacroExpansion;
  }
  static bool classof(const FreestandingMacroExpansion *expansion) {
    return expansion->getFreestandingMacroKind() == FreestandingMacroKind::Expr;
  }
};

inline bool Expr::isInfixOperator() const {
  return isa<BinaryExpr>(this) || isa<TernaryExpr>(this) ||
         isa<AssignExpr>(this) || isa<ExplicitCastExpr>(this);
}

inline Expr *const *CollectionExpr::getTrailingObjectsPointer() const {
  if (auto ty = dyn_cast<ArrayExpr>(this))
    return ty->getTrailingObjects<Expr*>();
  if (auto ty = dyn_cast<DictionaryExpr>(this))
    return ty->getTrailingObjects<Expr*>();
  toolchain_unreachable("Unhandled CollectionExpr!");
}

inline const SourceLoc *CollectionExpr::getTrailingSourceLocs() const {
  if (auto ty = dyn_cast<ArrayExpr>(this))
    return ty->getTrailingObjects<SourceLoc>();
  if (auto ty = dyn_cast<DictionaryExpr>(this))
    return ty->getTrailingObjects<SourceLoc>();
  toolchain_unreachable("Unhandled CollectionExpr!");
}

#undef LANGUAGE_FORWARD_SOURCE_LOCS_TO

void simple_display(toolchain::raw_ostream &out, const ClosureExpr *CE);
void simple_display(toolchain::raw_ostream &out, const DefaultArgumentExpr *expr);
void simple_display(toolchain::raw_ostream &out, const Expr *expr);

SourceLoc extractNearestSourceLoc(const ClosureExpr *expr);
SourceLoc extractNearestSourceLoc(const Expr *expr);

} // end namespace language

#endif
