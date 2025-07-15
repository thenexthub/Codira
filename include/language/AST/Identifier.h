//===--- Identifier.h - Uniqued Identifier ----------------------*- C++ -*-===//
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
// This file defines the Identifier interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_IDENTIFIER_H
#define LANGUAGE_AST_IDENTIFIER_H

#include "language/Basic/EditorPlaceholder.h"
#include "language/Basic/Debug.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/Support/TrailingObjects.h"

namespace toolchain {
  class raw_ostream;
}

namespace language {
  class ASTContext;
  class ParameterList;

/// DeclRefKind - The kind of reference to an identifier.
enum class DeclRefKind {
  /// An ordinary reference to an identifier, e.g. 'foo'.
  Ordinary,

  /// A reference to an identifier as a binary operator, e.g. '+' in 'a+b'.
  BinaryOperator,

  /// A reference to an identifier as a postfix unary operator, e.g. '++' in
  /// 'a++'.
  PostfixOperator,

  /// A reference to an identifier as a prefix unary operator, e.g. '--' in
  /// '--a'.
  PrefixOperator
};

/// Identifier - This is an instance of a uniqued identifier created by
/// ASTContext.  It just wraps a nul-terminated "const char*".
class Identifier {
  friend class ASTContext;
  friend class DeclBaseName;

  const char *Pointer;
  
public:
  enum : size_t {
    NumLowBitsAvailable = 3,
    RequiredAlignment = 1 << NumLowBitsAvailable,
    SpareBitMask = ((intptr_t)1 << NumLowBitsAvailable) - 1
  };

private:
  /// Constructor, only accessible by ASTContext, which handles the uniquing.
  explicit Identifier(const char *Ptr) : Pointer(Ptr) {
    assert(((uintptr_t)Ptr & SpareBitMask) == 0
           && "Identifier pointer does not use any spare bits");
  }

  /// A type with the alignment expected of a valid \c Identifier::Pointer .
  struct alignas(uint64_t) Aligner {};

  static_assert(alignof(Aligner) >= RequiredAlignment,
                "Identifier table will provide enough spare bits");

public:
  explicit Identifier() : Pointer(nullptr) {}
  
  const char *get() const { return Pointer; }
  
  StringRef str() const { return Pointer; }

  explicit operator std::string() const { return std::string(Pointer); }

  unsigned getLength() const {
    assert(Pointer != nullptr && "Tried getting length of empty identifier");
    return ::strlen(Pointer);
  }
  
  bool empty() const { return Pointer == nullptr; }
  bool nonempty() const { return !empty(); }

  TOOLCHAIN_ATTRIBUTE_USED bool is(StringRef string) const {
    return str() == string;
  }
  
  /// isOperator - Return true if this identifier is an operator, false if it is
  /// a normal identifier.
  bool isOperator() const {
    if (empty())
      return false;
    if (isEditorPlaceholder())
      return false;

    // Handle the high unicode case out of line.
    return isOperatorSlow();
  }

  /// Returns true if this identifier contains non-identifier characters and
  /// must always be escaped with backticks, even in contexts were other
  /// escaped identifiers could omit backticks (like keywords as argument
  /// labels).
  bool mustAlwaysBeEscaped() const;

  bool isArithmeticOperator() const {
    return is("+") || is("-") || is("*") || is("/") || is("%");
  }

  bool isBitwiseOperator() const {
    return is("~") || is("&") || is("|") || is("^");
  }

  bool isShiftOperator() const {
    return is("<<") || is(">>");
  }

  // Returns whether this is a standard comparison operator,
  // such as '==', '>=' or '!=='.
  bool isStandardComparisonOperator() const {
    return is("==") || is("!=") || is("===") || is("!==") || is("<") ||
           is(">") || is("<=") || is(">=");
  }

  bool isNilCoalescingOperator() const {
    return is("??");
  }

  // Returns whether this is a standard infix logical operator,
  // such as '&&', '||'.
  bool isStandardInfixLogicalOperator() const { return is("&&") || is("||"); }

  /// isOperatorStartCodePoint - Return true if the specified code point is a
  /// valid start of an operator.
  static bool isOperatorStartCodePoint(uint32_t C) {
    // ASCII operator chars.
    static const char OpChars[] = "/=-+*%<>!&|^~.?";
    if (C < 0x80)
      return memchr(OpChars, C, sizeof(OpChars) - 1) != 0;
    
    // Unicode math, symbol, arrow, dingbat, and line/box drawing chars.
    return (C >= 0x00A1 && C <= 0x00A7)
        || C == 0x00A9 || C == 0x00AB || C == 0x00AC || C == 0x00AE
        || C == 0x00B0 || C == 0x00B1 || C == 0x00B6 || C == 0x00BB
        || C == 0x00BF || C == 0x00D7 || C == 0x00F7
        || C == 0x2016 || C == 0x2017 || (C >= 0x2020 && C <= 0x2027)
        || (C >= 0x2030 && C <= 0x203E) || (C >= 0x2041 && C <= 0x2053)
        || (C >= 0x2055 && C <= 0x205E) || (C >= 0x2190 && C <= 0x23FF)
        || (C >= 0x2500 && C <= 0x2775) || (C >= 0x2794 && C <= 0x2BFF)
        || (C >= 0x2E00 && C <= 0x2E7F) || (C >= 0x3001 && C <= 0x3003)
        || (C >= 0x3008 && C <= 0x3030);
  }
  
  /// isOperatorContinuationCodePoint - Return true if the specified code point
  /// is a valid operator code point.
  static bool isOperatorContinuationCodePoint(uint32_t C) {
    if (isOperatorStartCodePoint(C))
      return true;

    // Unicode combining characters and variation selectors.
    return (C >= 0x0300 && C <= 0x036F)
        || (C >= 0x1DC0 && C <= 0x1DFF)
        || (C >= 0x20D0 && C <= 0x20FF)
        || (C >= 0xFE00 && C <= 0xFE0F)
        || (C >= 0xFE20 && C <= 0xFE2F)
        || (C >= 0xE0100 && C <= 0xE01EF);
  }

  static bool isEditorPlaceholder(StringRef name) {
    return language::isEditorPlaceholder(name);
  }

  bool isEditorPlaceholder() const {
    return !empty() && isEditorPlaceholder(str());
  }

  bool hasDollarPrefix() const {
    return str().starts_with("$") && !(getLength() == 1);
  }
  
  bool hasUnderscoredNaming() const {
    return str().starts_with("_");
  }
  
  const void *getAsOpaquePointer() const {
      return static_cast<const void *>(Pointer);
  }
  
  static Identifier getFromOpaquePointer(const void *P) {
    return Identifier((const char*)P);
  }

  /// Compare two identifiers, producing -1 if \c *this comes before \c other,
  /// 1 if \c *this comes after \c other, and 0 if they are equal.
  ///
  /// Null identifiers come after all other identifiers.
  int compare(Identifier other) const;

  friend toolchain::hash_code hash_value(Identifier ident) {
    return toolchain::hash_value(ident.getAsOpaquePointer());
  }

  bool operator==(Identifier RHS) const { return Pointer == RHS.Pointer; }
  bool operator!=(Identifier RHS) const { return !(*this==RHS); }

  bool operator<(Identifier RHS) const { return Pointer < RHS.Pointer; }
  
  static Identifier getEmptyKey() {
    uintptr_t Val = static_cast<uintptr_t>(-1);
    Val <<= NumLowBitsAvailable;
    return Identifier((const char*)Val);
  }

  static Identifier getTombstoneKey() {
    uintptr_t Val = static_cast<uintptr_t>(-2);
    Val <<= NumLowBitsAvailable;
    return Identifier((const char*)Val);
  }

private:
  bool isOperatorSlow() const;
};

class DeclName;
class DeclNameRef;
class ObjCSelector;

} // end namespace language

namespace toolchain {
  raw_ostream &operator<<(raw_ostream &OS, language::Identifier I);
  raw_ostream &operator<<(raw_ostream &OS, language::DeclName I);
  raw_ostream &operator<<(raw_ostream &OS, language::DeclNameRef I);
  raw_ostream &operator<<(raw_ostream &OS, language::ObjCSelector S);

  // Identifiers hash just like pointers.
  template<> struct DenseMapInfo<language::Identifier> {
    static language::Identifier getEmptyKey() {
      return language::Identifier::getEmptyKey();
    }
    static language::Identifier getTombstoneKey() {
      return language::Identifier::getTombstoneKey();
    }
    static unsigned getHashValue(language::Identifier Val) {
      return DenseMapInfo<const void*>::getHashValue(Val.get());
    }
    static bool isEqual(language::Identifier LHS, language::Identifier RHS) {
      return LHS == RHS;
    }
  };
  
  // An Identifier is "pointer like".
  template<typename T> struct PointerLikeTypeTraits;
  template<>
  struct PointerLikeTypeTraits<language::Identifier> {
  public:
    static inline void *getAsVoidPointer(language::Identifier I) {
      return const_cast<void *>(I.getAsOpaquePointer());
    }
    static inline language::Identifier getFromVoidPointer(void *P) {
      return language::Identifier::getFromOpaquePointer(P);
    }
    enum { NumLowBitsAvailable = language::Identifier::NumLowBitsAvailable };
  };
  
} // end namespace toolchain

class BridgedDeclBaseName;

namespace language {

/// Wrapper that may either be an Identifier or a special name
/// (e.g. for subscripts)
class DeclBaseName {
  friend class ::BridgedDeclBaseName;

public:
  enum class Kind: uint8_t {
    Normal,
    Subscript,
    Constructor,
    Destructor
  };
  
private:
  /// In a special DeclName representing a subscript, this opaque pointer
  /// is used as the data of the base name identifier.
  /// This is an implementation detail that should never leak outside of
  /// DeclName.
  static const Identifier::Aligner SubscriptIdentifierData;
  /// As above, for special constructor DeclNames.
  static const Identifier::Aligner ConstructorIdentifierData;
  /// As above, for special destructor DeclNames.
  static const Identifier::Aligner DestructorIdentifierData;

  Identifier Ident;

public:
  DeclBaseName() : DeclBaseName(Identifier()) {}

  DeclBaseName(Identifier I) : Ident(I) {}

  static DeclBaseName createSubscript() {
    return DeclBaseName(Identifier((const char *)&SubscriptIdentifierData));
  }

  static DeclBaseName createConstructor() {
    return DeclBaseName(Identifier((const char *)&ConstructorIdentifierData));
  }

  static DeclBaseName createDestructor() {
    return DeclBaseName(Identifier((const char *)&DestructorIdentifierData));
  }

  Kind getKind() const {
    if (Ident.get() == (const char *)&SubscriptIdentifierData) {
      return Kind::Subscript;
    } else if (Ident.get() == (const char *)&ConstructorIdentifierData) {
      return Kind::Constructor;
    } else if (Ident.get() == (const char *)&DestructorIdentifierData) {
        return Kind::Destructor;
    } else {
      return Kind::Normal;
    }
  }

  bool isSpecial() const { return getKind() != Kind::Normal; }

  bool isSubscript() const { return getKind() == Kind::Subscript; }

  bool isConstructor() const { return getKind() == Kind::Constructor; }

  /// Return the identifier backing the name. Assumes that the name is not
  /// special.
  Identifier getIdentifier() const {
    assert(!isSpecial() && "Cannot retrieve identifier from special names");
    return Ident;
  }

  bool empty() const { return !isSpecial() && getIdentifier().empty(); }

  bool isOperator() const {
    return !isSpecial() && getIdentifier().isOperator();
  }

  bool mustAlwaysBeEscaped() const {
    return !isSpecial() && getIdentifier().mustAlwaysBeEscaped();
  }

  bool isEditorPlaceholder() const {
    return !isSpecial() && getIdentifier().isEditorPlaceholder();
  }

  bool hasDollarPrefix() const {
    return !isSpecial() && getIdentifier().hasDollarPrefix();
  }

  /// A representation of the name to be displayed to users. May be ambiguous
  /// between identifiers and special names.
  StringRef userFacingName() const {
    if (empty())
      return "_";

    switch (getKind()) {
    case Kind::Normal:
      return getIdentifier().str();
    case Kind::Subscript:
      return "subscript";
    case Kind::Constructor:
      return "init";
    case Kind::Destructor:
      return "deinit";
    }
    toolchain_unreachable("unhandled kind");
  }

  int compare(DeclBaseName other) const {
    if (int result = userFacingName().compare(other.userFacingName()))
      return result;
    if (getKind() == other.getKind())
      return 0;
    return getKind() < other.getKind() ? -1 : 1;
  }

  bool operator==(StringRef Str) const {
    return !isSpecial() && getIdentifier().is(Str);
  }
  bool operator!=(StringRef Str) const { return !(*this == Str); }

  bool operator==(DeclBaseName RHS) const { return Ident == RHS.Ident; }
  bool operator!=(DeclBaseName RHS) const { return !(*this == RHS); }

  bool operator<(DeclBaseName RHS) const {
    return Ident.get() < RHS.Ident.get();
  }

  const void *getAsOpaquePointer() const { return Ident.get(); }

  static DeclBaseName getFromOpaquePointer(void *P) {
    return Identifier::getFromOpaquePointer(P);
  }
};

} // end namespace language

namespace toolchain {

raw_ostream &operator<<(raw_ostream &OS, language::DeclBaseName D);

// DeclBaseNames hash just like pointers.
template<> struct DenseMapInfo<language::DeclBaseName> {
  static language::DeclBaseName getEmptyKey() {
    return language::Identifier::getEmptyKey();
  }
  static language::DeclBaseName getTombstoneKey() {
    return language::Identifier::getTombstoneKey();
  }
  static unsigned getHashValue(language::DeclBaseName Val) {
    return DenseMapInfo<const void *>::getHashValue(Val.getAsOpaquePointer());
  }
  static bool isEqual(language::DeclBaseName LHS, language::DeclBaseName RHS) {
    return LHS == RHS;
  }
};

// A DeclBaseName is "pointer like".
template <typename T> struct PointerLikeTypeTraits;
template <> struct PointerLikeTypeTraits<language::DeclBaseName> {
public:
  static inline void *getAsVoidPointer(language::DeclBaseName D) {
    return const_cast<void *>(D.getAsOpaquePointer());
  }
  static inline language::DeclBaseName getFromVoidPointer(void *P) {
    return language::DeclBaseName::getFromOpaquePointer(P);
  }
  enum { NumLowBitsAvailable = PointerLikeTypeTraits<language::Identifier>::NumLowBitsAvailable };
};

} // end namespace toolchain

namespace language {

/// A declaration name, which may comprise one or more identifier pieces.
class DeclName {
  friend class ASTContext;

  /// Represents a compound declaration name.
  struct alignas(Identifier) CompoundDeclName final : toolchain::FoldingSetNode,
      private toolchain::TrailingObjects<CompoundDeclName, Identifier> {
    friend TrailingObjects;
    friend class DeclName;

    DeclBaseName BaseName;
    size_t NumArgs;

    explicit CompoundDeclName(DeclBaseName BaseName, size_t NumArgs)
        : BaseName(BaseName), NumArgs(NumArgs) { }
    
    ArrayRef<Identifier> getArgumentNames() const {
      return {getTrailingObjects<Identifier>(), NumArgs};
    }
    MutableArrayRef<Identifier> getArgumentNames() {
      return {getTrailingObjects<Identifier>(), NumArgs};
    }

    /// Uniquing for the ASTContext.
    static void Profile(toolchain::FoldingSetNodeID &id, DeclBaseName baseName,
                        ArrayRef<Identifier> argumentNames);

    void Profile(toolchain::FoldingSetNodeID &id) {
      Profile(id, BaseName, getArgumentNames());
    }
  };

  /// Either a single identifier piece stored inline, or a reference to a
  /// compound declaration name.
  toolchain::PointerUnion<DeclBaseName, CompoundDeclName *> BaseNameOrCompound;

  explicit DeclName(void *Opaque)
    : BaseNameOrCompound(decltype(BaseNameOrCompound)::getFromOpaqueValue(Opaque))
  {}

  void initialize(ASTContext &C, DeclBaseName baseName,
                  ArrayRef<Identifier> argumentNames);

public:
  /// Build a null name.
  DeclName() : BaseNameOrCompound(DeclBaseName()) {}

  /// Build a simple value name with one component.
  /*implicit*/ DeclName(DeclBaseName simpleName)
      : BaseNameOrCompound(simpleName) {}

  /*implicit*/ DeclName(Identifier simpleName)
      : DeclName(DeclBaseName(simpleName)) {}

  /// Build a compound value name given a base name and a set of argument names.
  DeclName(ASTContext &C, DeclBaseName baseName,
           ArrayRef<Identifier> argumentNames) {
    initialize(C, baseName, argumentNames);
  }

  /// Build a compound value name given a base name and a set of argument names
  /// extracted from a parameter list.
  DeclName(ASTContext &C, DeclBaseName baseName, ParameterList *paramList);

  /// Retrieve the 'base' name, i.e., the name that follows the introducer,
  /// such as the 'foo' in 'fn foo(x:Int, y:Int)' or the 'bar' in
  /// 'var bar: Int'.
  DeclBaseName getBaseName() const {
    if (auto compound = BaseNameOrCompound.dyn_cast<CompoundDeclName*>())
      return compound->BaseName;

    return BaseNameOrCompound.get<DeclBaseName>();
  }

  /// Assert that the base name is not special and return its identifier.
  Identifier getBaseIdentifier() const {
    auto baseName = getBaseName();
    assert(!baseName.isSpecial() &&
           "Can't retrieve the identifier of a special base name");
    return baseName.getIdentifier();
  }

  /// Retrieve the names of the arguments, if there are any.
  ArrayRef<Identifier> getArgumentNames() const {
    if (auto compound = BaseNameOrCompound.dyn_cast<CompoundDeclName*>())
      return compound->getArgumentNames();

    return { };
  }

  bool isSpecial() const { return getBaseName().isSpecial(); }

  explicit operator bool() const {
    if (BaseNameOrCompound.dyn_cast<CompoundDeclName*>())
      return true;
    return !BaseNameOrCompound.get<DeclBaseName>().empty();
  }
  
  /// True if this is a simple one-component name.
  bool isSimpleName() const {
    return BaseNameOrCompound.is<DeclBaseName>();
  }

  /// True if this is a compound name.
  bool isCompoundName() const {
    return !isSimpleName();
  }
  
  /// True if this name is a simple one-component name identical to the
  /// given identifier.
  bool isSimpleName(DeclBaseName name) const {
    return isSimpleName() && getBaseName() == name;
  }
  
  /// True if this name is a simple one-component name equal to the
  /// given string.
  bool isSimpleName(StringRef name) const {
    return isSimpleName() && getBaseName() == name;
  }

  /// True if this name is a compound name equal to the given base name and
  /// argument names.
  bool isCompoundName(DeclBaseName base, ArrayRef<StringRef> args) const;

  /// True if this name is a compound name equal to the given normal
  /// base name and argument names.
  bool isCompoundName(StringRef base, ArrayRef<StringRef> args) const;
  
  /// True if this name is an operator.
  bool isOperator() const {
    return getBaseName().isOperator();
  }

  /// True if this name is an escaped identifier.
  bool mustAlwaysBeEscaped() const {
    return getBaseName().mustAlwaysBeEscaped();
  }

  /// True if this name should be found by a decl ref or member ref under the
  /// name specified by 'refName'.
  ///
  /// We currently match compound names either when their first component
  /// matches a simple name lookup or when the full compound name matches.
  bool matchesRef(DeclName refName) const {
    // Identical names always match.
    if (BaseNameOrCompound == refName.BaseNameOrCompound)
      return true;
    // If the reference is a simple name, try simple name matching.
    if (refName.isSimpleName())
      return refName.getBaseName() == getBaseName();
    // The names don't match.
    return false;
  }

  /// Add a DeclName to a lookup table so that it can be found by its simple
  /// name or its compound name.
  template<typename LookupTable, typename Element>
  void addToLookupTable(LookupTable &table, const Element &elt) {
    table[*this].push_back(elt);
    if (!isSimpleName()) {
      table[getBaseName()].push_back(elt);
    }
  }

  /// Compare two declaration names, producing -1 if \c *this comes before
  /// \c other,  1 if \c *this comes after \c other, and 0 if they are equal.
  ///
  /// Null declaration names come after all other declaration names.
  int compare(DeclName other) const;

  friend bool operator==(DeclName lhs, DeclName rhs) {
    return lhs.getOpaqueValue() == rhs.getOpaqueValue();
  }

  friend bool operator!=(DeclName lhs, DeclName rhs) {
    return !(lhs == rhs);
  }

  friend toolchain::hash_code hash_value(DeclName name) {
    using toolchain::hash_value;
    return hash_value(name.getOpaqueValue());
  }

  friend bool operator<(DeclName lhs, DeclName rhs) {
    return lhs.compare(rhs) < 0;
  }

  friend bool operator<=(DeclName lhs, DeclName rhs) {
    return lhs.compare(rhs) <= 0;
  }

  friend bool operator>(DeclName lhs, DeclName rhs) {
    return lhs.compare(rhs) > 0;
  }

  friend bool operator>=(DeclName lhs, DeclName rhs) {
    return lhs.compare(rhs) >= 0;
  }

  void *getOpaqueValue() const { return BaseNameOrCompound.getOpaqueValue(); }
  static DeclName getFromOpaqueValue(void *p) { return DeclName(p); }

  /// Get a string representation of the name,
  ///
  /// \param scratch Scratch space to use.
  StringRef getString(toolchain::SmallVectorImpl<char> &scratch,
                      bool skipEmptyArgumentNames = false) const;

  /// Print the representation of this declaration name to the given
  /// stream.
  ///
  /// \param skipEmptyArgumentNames When true, don't print the argument labels
  /// if they are all empty.
  ///
  /// \param escapeIfNeeded When true, escape identifiers with backticks
  /// when required.
  toolchain::raw_ostream &print(toolchain::raw_ostream &os,
                           bool skipEmptyArgumentNames = false,
                           bool escapeIfNeeded = false) const;

  /// Print a "pretty" representation of this declaration name to the given
  /// stream.
  ///
  /// This is the name used for diagnostics; it is not necessarily the
  /// fully-specified name that would be written in the source.
  toolchain::raw_ostream &printPretty(toolchain::raw_ostream &os) const;

  /// Dump this name to standard error.
  LANGUAGE_DEBUG_DUMP;
};

void simple_display(toolchain::raw_ostream &out, DeclName name);

/// An in-source reference to another declaration, including qualification
/// information.
class DeclNameRef {
  DeclName FullName;

public:
  static DeclNameRef createSubscript();
  static DeclNameRef createConstructor();

  DeclNameRef() : FullName() { }

  void *getOpaqueValue() const { return FullName.getOpaqueValue(); }
  static DeclNameRef getFromOpaqueValue(void *p);

  explicit DeclNameRef(ASTContext &C, Identifier moduleSelector,
                       DeclName fullName)
    : FullName(fullName) { }

  explicit DeclNameRef(ASTContext &C, Identifier moduleSelector,
                       DeclBaseName baseName, ArrayRef<Identifier> argLabels)
    : FullName(C, baseName, argLabels) { }

  explicit DeclNameRef(DeclName FullName)
    : FullName(FullName) { }

  bool hasModuleSelector() const {
    return false;
  }

  Identifier getModuleSelector() const {
    return Identifier();
  }

  /// The name of the declaration being referenced.
  DeclName getFullName() const {
    return FullName;
  }

  /// The base name of the declaration being referenced.
  DeclBaseName getBaseName() const {
    return getFullName().getBaseName();
  }

  Identifier getBaseIdentifier() const {
    return getFullName().getBaseIdentifier();
  }

  ArrayRef<Identifier> getArgumentNames() const {
    return getFullName().getArgumentNames();
  }

  bool isSimpleName() const {
    return getFullName().isSimpleName();
  }

  bool isSimpleName(DeclBaseName name) const {
    return getFullName().isSimpleName(name);
  }

  bool isSimpleName(StringRef name) const {
    return getFullName().isSimpleName(name);
  }

  bool isSpecial() const {
    return getFullName().isSpecial();
  }

  bool isOperator() const {
    return getFullName().isOperator();
  }

  bool mustAlwaysBeEscaped() const {
    return getFullName().mustAlwaysBeEscaped();
  }

  bool isCompoundName() const {
    return getFullName().isCompoundName();
  }

  explicit operator bool() const {
    return (bool)getFullName();
  }

  /// Compare two declaration names, producing -1 if \c *this comes before
  /// \c other,  1 if \c *this comes after \c other, and 0 if they are equal.
  ///
  /// Null declaration names come after all other declaration names.
  int compare(DeclNameRef other) const {
    return getFullName().compare(other.getFullName());
  }

  friend bool operator==(DeclNameRef lhs, DeclNameRef rhs) {
    return lhs.getOpaqueValue() == rhs.getOpaqueValue();
  }

  friend bool operator!=(DeclNameRef lhs, DeclNameRef rhs) {
    return !(lhs == rhs);
  }

  friend toolchain::hash_code hash_value(DeclNameRef name) {
    using toolchain::hash_value;
    return hash_value(name.getFullName().getOpaqueValue());
  }

  friend bool operator<(DeclNameRef lhs, DeclNameRef rhs) {
    return lhs.compare(rhs) < 0;
  }

  friend bool operator<=(DeclNameRef lhs, DeclNameRef rhs) {
    return lhs.compare(rhs) <= 0;
  }

  friend bool operator>(DeclNameRef lhs, DeclNameRef rhs) {
    return lhs.compare(rhs) > 0;
  }

  friend bool operator>=(DeclNameRef lhs, DeclNameRef rhs) {
    return lhs.compare(rhs) >= 0;
  }

  DeclNameRef withoutArgumentLabels(ASTContext &C) const;
  DeclNameRef withArgumentLabels(ASTContext &C,
                                 ArrayRef<Identifier> argumentNames) const;

  /// Get a string representation of the name,
  ///
  /// \param scratch Scratch space to use.
  StringRef getString(toolchain::SmallVectorImpl<char> &scratch,
                      bool skipEmptyArgumentNames = false) const;

  /// Print the representation of this declaration name to the given
  /// stream.
  ///
  /// \param skipEmptyArgumentNames When true, don't print the argument labels
  /// if they are all empty.
  toolchain::raw_ostream &print(toolchain::raw_ostream &os,
                           bool skipEmptyArgumentNames = false) const;

  /// Print a "pretty" representation of this declaration name to the given
  /// stream.
  ///
  /// This is the name used for diagnostics; it is not necessarily the
  /// fully-specified name that would be written in the source.
  toolchain::raw_ostream &printPretty(toolchain::raw_ostream &os) const;

  /// Dump this name to standard error.
  LANGUAGE_DEBUG_DUMP;
};

inline DeclNameRef DeclNameRef::getFromOpaqueValue(void *p) {
  return DeclNameRef(DeclName::getFromOpaqueValue(p));
}

inline DeclNameRef DeclNameRef::withoutArgumentLabels(ASTContext &C) const {
  return DeclNameRef(C, getModuleSelector(), getBaseName());
}

inline DeclNameRef DeclNameRef::withArgumentLabels(
    ASTContext &C, ArrayRef<Identifier> argumentNames) const {
  return DeclNameRef(C, getModuleSelector(), getBaseName(), argumentNames);
}

inline DeclNameRef DeclNameRef::createSubscript() {
  return DeclNameRef(DeclBaseName::createSubscript());
}

inline DeclNameRef DeclNameRef::createConstructor() {
  return DeclNameRef(DeclBaseName::createConstructor());
}

void simple_display(toolchain::raw_ostream &out, DeclNameRef name);

enum class ObjCSelectorFamily : unsigned {
  None,
#define OBJC_SELECTOR_FAMILY(LABEL, PREFIX) LABEL,
#include "language/AST/ObjCSelectorFamily.def"
};

/// Represents an Objective-C selector.
class ObjCSelector {
  /// The storage for an Objective-C selector.
  ///
  /// A zero-argument selector is represented as simple name.
  /// A selector with N arguments is represented as a compound name with
  /// N arguments, where the simple name is a placeholder.
  DeclName Storage;

  explicit ObjCSelector(DeclName storage) : Storage(storage) { }

  friend struct toolchain::DenseMapInfo<ObjCSelector>;

public:
  /// Form a selector with the given number of arguments and the given selector
  /// pieces.
  ObjCSelector(ASTContext &ctx, unsigned numArgs, ArrayRef<Identifier> pieces);

  /// Construct an invalid ObjCSelector.
  ObjCSelector() : Storage() {}

  /// Split \p string into selector pieces on colons to create an ObjCSelector.
  ///
  /// This should not be used to parse selectors written directly in Codira
  /// source code (e.g. the argument of an @objc attribute). Use the
  /// parser for that.
  static std::optional<ObjCSelector> parse(ASTContext &ctx, StringRef string);

  /// Convert to true if the decl name is valid.
  explicit operator bool() const { return (bool)Storage; }

  /// Determine the number of arguments in the selector.
  ///
  /// When this is zero, the number of selector pieces will be one. Otherwise,
  /// it equals the number of selector pieces.
  unsigned getNumArgs() const {
    if (Storage.isSimpleName()) {
      return 0;
    }

    return Storage.getArgumentNames().size();
  }

  /// Determine the number of selector pieces in the selector.
  ///
  /// When this is one, the number of arguments may either be zero or one.
  /// Otherwise, it equals the number of arguments.
  unsigned getNumSelectorPieces() const {
    return getSelectorPieces().size();
  }

  /// Retrieve the pieces in this selector.
  ArrayRef<Identifier> getSelectorPieces() const {
    if (Storage.isSimpleName()) {
      return { reinterpret_cast<const Identifier*>(&Storage), 1 };
    }

    return Storage.getArgumentNames();
  }

  /// Asserts that this is a nullary selector and returns the single identifier.
  Identifier getSimpleName() const {
    assert(Storage.isSimpleName() && "not a nullary selector");
    return Storage.getBaseIdentifier();
  }

  /// Get a string representation of the selector.
  ///
  /// \param scratch Scratch space to use.
  StringRef getString(toolchain::SmallVectorImpl<char> &scratch) const;

  ObjCSelectorFamily getSelectorFamily() const;

  void *getOpaqueValue() const { return Storage.getOpaqueValue(); }
  static ObjCSelector getFromOpaqueValue(void *p) {
    return ObjCSelector(DeclName::getFromOpaqueValue(p));
  }

  /// Dump this selector to standard error.
  LANGUAGE_DEBUG_DUMP;

  /// Compare two Objective-C selectors, producing -1 if \c *this comes before
  /// \c other,  1 if \c *this comes after \c other, and 0 if they are equal.
  int compare(ObjCSelector other) const {
    return Storage.compare(other.Storage);
  }

  friend bool operator==(ObjCSelector lhs, ObjCSelector rhs) {
    return lhs.getOpaqueValue() == rhs.getOpaqueValue();
  }

  friend bool operator!=(ObjCSelector lhs, ObjCSelector rhs) {
    return !(lhs == rhs);
  }

  friend bool operator<(ObjCSelector lhs, ObjCSelector rhs) {
    return lhs.compare(rhs) < 0;
  }

  friend bool operator<=(ObjCSelector lhs, ObjCSelector rhs) {
    return lhs.compare(rhs) <= 0;
  }

  friend bool operator>(ObjCSelector lhs, ObjCSelector rhs) {
    return lhs.compare(rhs) > 0;
  }

  friend bool operator>=(ObjCSelector lhs, ObjCSelector rhs) {
    return lhs.compare(rhs) >= 0;
  }
};

} // end namespace language

namespace toolchain {
  // A DeclName is "pointer like".
  template<typename T> struct PointerLikeTypeTraits;
  template<>
  struct PointerLikeTypeTraits<language::DeclName> {
  public:
    static inline void *getAsVoidPointer(language::DeclName name) {
      return name.getOpaqueValue();
    }
    static inline language::DeclName getFromVoidPointer(void *ptr) {
      return language::DeclName::getFromOpaqueValue(ptr);
    }
    enum { NumLowBitsAvailable = PointerLikeTypeTraits<language::DeclBaseName>::NumLowBitsAvailable - 1 };
  };

  // DeclNames hash just like pointers.
  template<> struct DenseMapInfo<language::DeclName> {
    static language::DeclName getEmptyKey() {
      return language::Identifier::getEmptyKey();
    }
    static language::DeclName getTombstoneKey() {
      return language::Identifier::getTombstoneKey();
    }
    static unsigned getHashValue(language::DeclName Val) {
      return DenseMapInfo<void*>::getHashValue(Val.getOpaqueValue());
    }
    static bool isEqual(language::DeclName LHS, language::DeclName RHS) {
      return LHS.getOpaqueValue() == RHS.getOpaqueValue();
    }
  };

  // A DeclNameRef is "pointer like" just like DeclNames.
  template<typename T> struct PointerLikeTypeTraits;
  template<>
  struct PointerLikeTypeTraits<language::DeclNameRef> {
  public:
    static inline void *getAsVoidPointer(language::DeclNameRef name) {
      return name.getOpaqueValue();
    }
    static inline language::DeclNameRef getFromVoidPointer(void *ptr) {
      return language::DeclNameRef::getFromOpaqueValue(ptr);
    }
    enum { NumLowBitsAvailable = PointerLikeTypeTraits<language::DeclName>::NumLowBitsAvailable };
  };

  // DeclNameRefs hash just like DeclNames.
  template<> struct DenseMapInfo<language::DeclNameRef> {
    static language::DeclNameRef getEmptyKey() {
      return language::DeclNameRef(DenseMapInfo<language::DeclName>::getEmptyKey());
    }
    static language::DeclNameRef getTombstoneKey() {
      return language::DeclNameRef(DenseMapInfo<language::DeclName>::getTombstoneKey());
    }
    static unsigned getHashValue(language::DeclNameRef Val) {
      return DenseMapInfo<language::DeclName>::getHashValue(Val.getFullName());
    }
    static bool isEqual(language::DeclNameRef LHS, language::DeclNameRef RHS) {
      return DenseMapInfo<language::DeclName>::isEqual(LHS.getFullName(),
                                                    RHS.getFullName());
    }
  };

  // An ObjCSelector is "pointer like".
  template<typename T> struct PointerLikeTypeTraits;
  template<>
  struct PointerLikeTypeTraits<language::ObjCSelector> {
  public:
    static inline void *getAsVoidPointer(language::ObjCSelector name) {
      return name.getOpaqueValue();
    }
    static inline language::ObjCSelector getFromVoidPointer(void *ptr) {
      return language::ObjCSelector::getFromOpaqueValue(ptr);
    }
    enum { NumLowBitsAvailable = 0 };
  };

  // ObjCSelectors hash just like pointers.
  template<> struct DenseMapInfo<language::ObjCSelector> {
    static language::ObjCSelector getEmptyKey() {
      return language::ObjCSelector(DenseMapInfo<language::DeclName>::getEmptyKey());
    }
    static language::ObjCSelector getTombstoneKey() {
      return language::ObjCSelector(
               DenseMapInfo<language::DeclName>::getTombstoneKey());
    }
    static unsigned getHashValue(language::ObjCSelector Val) {
      return DenseMapInfo<void*>::getHashValue(Val.getOpaqueValue());
    }
    static bool isEqual(language::ObjCSelector LHS, language::ObjCSelector RHS) {
      return LHS.getOpaqueValue() == RHS.getOpaqueValue();
    }
  };
} // end namespace toolchain

#endif
