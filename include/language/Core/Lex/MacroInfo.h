//===- MacroInfo.h - Information about #defined identifiers -----*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the language::Core::MacroInfo and language::Core::MacroDirective classes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LEX_MACROINFO_H
#define LANGUAGE_CORE_LEX_MACROINFO_H

#include "language/Core/Lex/Token.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Allocator.h"
#include <algorithm>
#include <cassert>

namespace language::Core {

class DefMacroDirective;
class IdentifierInfo;
class Module;
class Preprocessor;
class SourceManager;

/// Encapsulates the data about a macro definition (e.g. its tokens).
///
/// There's an instance of this class for every #define.
class MacroInfo {
  //===--------------------------------------------------------------------===//
  // State set when the macro is defined.

  /// The location the macro is defined.
  SourceLocation Location;

  /// The location of the last token in the macro.
  SourceLocation EndLocation;

  /// The list of arguments for a function-like macro.
  ///
  /// ParameterList points to the first of NumParameters pointers.
  ///
  /// This can be empty, for, e.g. "#define X()".  In a C99-style variadic
  /// macro, this includes the \c __VA_ARGS__ identifier on the list.
  IdentifierInfo **ParameterList = nullptr;

  /// This is the list of tokens that the macro is defined to.
  const Token *ReplacementTokens = nullptr;

  /// \see ParameterList
  unsigned NumParameters = 0;

  /// \see ReplacementTokens
  unsigned NumReplacementTokens = 0;

  /// Length in characters of the macro definition.
  mutable unsigned DefinitionLength;
  mutable bool IsDefinitionLengthCached : 1;

  /// True if this macro is function-like, false if it is object-like.
  bool IsFunctionLike : 1;

  /// True if this macro is of the form "#define X(...)" or
  /// "#define X(Y,Z,...)".
  ///
  /// The __VA_ARGS__ token should be replaced with the contents of "..." in an
  /// invocation.
  bool IsC99Varargs : 1;

  /// True if this macro is of the form "#define X(a...)".
  ///
  /// The "a" identifier in the replacement list will be replaced with all
  /// arguments of the macro starting with the specified one.
  bool IsGNUVarargs : 1;

  /// True if this macro requires processing before expansion.
  ///
  /// This is the case for builtin macros such as __LINE__, so long as they have
  /// not been redefined, but not for regular predefined macros from the
  /// "<built-in>" memory buffer (see Preprocessing::getPredefinesFileID).
  bool IsBuiltinMacro : 1;

  /// Whether this macro contains the sequence ", ## __VA_ARGS__"
  bool HasCommaPasting : 1;

  //===--------------------------------------------------------------------===//
  // State that changes as the macro is used.

  /// True if we have started an expansion of this macro already.
  ///
  /// This disables recursive expansion, which would be quite bad for things
  /// like \#define A A.
  bool IsDisabled : 1;

  /// True if this macro is either defined in the main file and has
  /// been used, or if it is not defined in the main file.
  ///
  /// This is used to emit -Wunused-macros diagnostics.
  bool IsUsed : 1;

  /// True if this macro can be redefined without emitting a warning.
  bool IsAllowRedefinitionsWithoutWarning : 1;

  /// Must warn if the macro is unused at the end of translation unit.
  bool IsWarnIfUnused : 1;

  /// Whether this macro was used as header guard.
  bool UsedForHeaderGuard : 1;

  // Only the Preprocessor gets to create these.
  MacroInfo(SourceLocation DefLoc);

public:
  /// Return the location that the macro was defined at.
  SourceLocation getDefinitionLoc() const { return Location; }

  /// Set the location of the last token in the macro.
  void setDefinitionEndLoc(SourceLocation EndLoc) { EndLocation = EndLoc; }

  /// Return the location of the last token in the macro.
  SourceLocation getDefinitionEndLoc() const { return EndLocation; }

  /// Get length in characters of the macro definition.
  unsigned getDefinitionLength(const SourceManager &SM) const {
    if (IsDefinitionLengthCached)
      return DefinitionLength;
    return getDefinitionLengthSlow(SM);
  }

  /// Return true if the specified macro definition is equal to
  /// this macro in spelling, arguments, and whitespace.
  ///
  /// \param Syntactically if true, the macro definitions can be identical even
  /// if they use different identifiers for the function macro parameters.
  /// Otherwise the comparison is lexical and this implements the rules in
  /// C99 6.10.3.
  bool isIdenticalTo(const MacroInfo &Other, Preprocessor &PP,
                     bool Syntactically) const;

  /// Set or clear the isBuiltinMacro flag.
  void setIsBuiltinMacro(bool Val = true) { IsBuiltinMacro = Val; }

  /// Set the value of the IsUsed flag.
  void setIsUsed(bool Val) { IsUsed = Val; }

  /// Set the value of the IsAllowRedefinitionsWithoutWarning flag.
  void setIsAllowRedefinitionsWithoutWarning(bool Val) {
    IsAllowRedefinitionsWithoutWarning = Val;
  }

  /// Set the value of the IsWarnIfUnused flag.
  void setIsWarnIfUnused(bool val) { IsWarnIfUnused = val; }

  /// Set the specified list of identifiers as the parameter list for
  /// this macro.
  void setParameterList(ArrayRef<IdentifierInfo *> List,
                       toolchain::BumpPtrAllocator &PPAllocator) {
    assert(ParameterList == nullptr && NumParameters == 0 &&
           "Parameter list already set!");
    if (List.empty())
      return;

    NumParameters = List.size();
    ParameterList = PPAllocator.Allocate<IdentifierInfo *>(List.size());
    std::copy(List.begin(), List.end(), ParameterList);
  }

  /// Parameters - The list of parameters for a function-like macro.  This can
  /// be empty, for, e.g. "#define X()".
  using param_iterator = IdentifierInfo *const *;
  bool param_empty() const { return NumParameters == 0; }
  param_iterator param_begin() const { return ParameterList; }
  param_iterator param_end() const { return ParameterList + NumParameters; }
  unsigned getNumParams() const { return NumParameters; }
  ArrayRef<const IdentifierInfo *> params() const {
    return ArrayRef<const IdentifierInfo *>(ParameterList, NumParameters);
  }

  /// Return the parameter number of the specified identifier,
  /// or -1 if the identifier is not a formal parameter identifier.
  int getParameterNum(const IdentifierInfo *Arg) const {
    for (param_iterator I = param_begin(), E = param_end(); I != E; ++I)
      if (*I == Arg)
        return I - param_begin();
    return -1;
  }

  /// Function/Object-likeness.  Keep track of whether this macro has formal
  /// parameters.
  void setIsFunctionLike() { IsFunctionLike = true; }
  bool isFunctionLike() const { return IsFunctionLike; }
  bool isObjectLike() const { return !IsFunctionLike; }

  /// Varargs querying methods.  This can only be set for function-like macros.
  void setIsC99Varargs() { IsC99Varargs = true; }
  void setIsGNUVarargs() { IsGNUVarargs = true; }
  bool isC99Varargs() const { return IsC99Varargs; }
  bool isGNUVarargs() const { return IsGNUVarargs; }
  bool isVariadic() const { return IsC99Varargs || IsGNUVarargs; }

  /// Return true if this macro requires processing before expansion.
  ///
  /// This is true only for builtin macro, such as \__LINE__, whose values
  /// are not given by fixed textual expansions.  Regular predefined macros
  /// from the "<built-in>" buffer are not reported as builtins by this
  /// function.
  bool isBuiltinMacro() const { return IsBuiltinMacro; }

  bool hasCommaPasting() const { return HasCommaPasting; }
  void setHasCommaPasting() { HasCommaPasting = true; }

  /// Return false if this macro is defined in the main file and has
  /// not yet been used.
  bool isUsed() const { return IsUsed; }

  /// Return true if this macro can be redefined without warning.
  bool isAllowRedefinitionsWithoutWarning() const {
    return IsAllowRedefinitionsWithoutWarning;
  }

  /// Return true if we should emit a warning if the macro is unused.
  bool isWarnIfUnused() const { return IsWarnIfUnused; }

  /// Return the number of tokens that this macro expands to.
  unsigned getNumTokens() const { return NumReplacementTokens; }

  const Token &getReplacementToken(unsigned Tok) const {
    assert(Tok < NumReplacementTokens && "Invalid token #");
    return ReplacementTokens[Tok];
  }

  using const_tokens_iterator = const Token *;

  const_tokens_iterator tokens_begin() const { return ReplacementTokens; }
  const_tokens_iterator tokens_end() const {
    return ReplacementTokens + NumReplacementTokens;
  }
  bool tokens_empty() const { return NumReplacementTokens == 0; }
  ArrayRef<Token> tokens() const {
    return toolchain::ArrayRef(ReplacementTokens, NumReplacementTokens);
  }

  toolchain::MutableArrayRef<Token>
  allocateTokens(unsigned NumTokens, toolchain::BumpPtrAllocator &PPAllocator) {
    assert(ReplacementTokens == nullptr && NumReplacementTokens == 0 &&
           "Token list already allocated!");
    NumReplacementTokens = NumTokens;
    Token *NewReplacementTokens = PPAllocator.Allocate<Token>(NumTokens);
    ReplacementTokens = NewReplacementTokens;
    return toolchain::MutableArrayRef(NewReplacementTokens, NumTokens);
  }

  void setTokens(ArrayRef<Token> Tokens, toolchain::BumpPtrAllocator &PPAllocator) {
    assert(
        !IsDefinitionLengthCached &&
        "Changing replacement tokens after definition length got calculated");
    assert(ReplacementTokens == nullptr && NumReplacementTokens == 0 &&
           "Token list already set!");
    if (Tokens.empty())
      return;

    NumReplacementTokens = Tokens.size();
    Token *NewReplacementTokens = PPAllocator.Allocate<Token>(Tokens.size());
    std::copy(Tokens.begin(), Tokens.end(), NewReplacementTokens);
    ReplacementTokens = NewReplacementTokens;
  }

  /// Return true if this macro is enabled.
  ///
  /// In other words, that we are not currently in an expansion of this macro.
  bool isEnabled() const { return !IsDisabled; }

  void EnableMacro() {
    assert(IsDisabled && "Cannot enable an already-enabled macro!");
    IsDisabled = false;
  }

  void DisableMacro() {
    assert(!IsDisabled && "Cannot disable an already-disabled macro!");
    IsDisabled = true;
  }

  /// Determine whether this macro was used for a header guard.
  bool isUsedForHeaderGuard() const { return UsedForHeaderGuard; }

  void setUsedForHeaderGuard(bool Val) { UsedForHeaderGuard = Val; }

  void dump() const;

private:
  friend class Preprocessor;

  unsigned getDefinitionLengthSlow(const SourceManager &SM) const;
};

/// Encapsulates changes to the "macros namespace" (the location where
/// the macro name became active, the location where it was undefined, etc.).
///
/// MacroDirectives, associated with an identifier, are used to model the macro
/// history. Usually a macro definition (MacroInfo) is where a macro name
/// becomes active (MacroDirective) but #pragma push_macro / pop_macro can
/// create additional DefMacroDirectives for the same MacroInfo.
class MacroDirective {
public:
  enum Kind {
    MD_Define,
    MD_Undefine,
    MD_Visibility
  };

protected:
  /// Previous macro directive for the same identifier, or nullptr.
  MacroDirective *Previous = nullptr;

  SourceLocation Loc;

  /// MacroDirective kind.
  LLVM_PREFERRED_TYPE(Kind)
  unsigned MDKind : 2;

  /// True if the macro directive was loaded from a PCH file.
  LLVM_PREFERRED_TYPE(bool)
  unsigned IsFromPCH : 1;

  // Used by VisibilityMacroDirective ----------------------------------------//

  /// Whether the macro has public visibility (when described in a
  /// module).
  LLVM_PREFERRED_TYPE(bool)
  unsigned IsPublic : 1;

  MacroDirective(Kind K, SourceLocation Loc)
      : Loc(Loc), MDKind(K), IsFromPCH(false), IsPublic(true) {}

public:
  Kind getKind() const { return Kind(MDKind); }

  SourceLocation getLocation() const { return Loc; }

  /// Set previous definition of the macro with the same name.
  void setPrevious(MacroDirective *Prev) { Previous = Prev; }

  /// Get previous definition of the macro with the same name.
  const MacroDirective *getPrevious() const { return Previous; }

  /// Get previous definition of the macro with the same name.
  MacroDirective *getPrevious() { return Previous; }

  /// Return true if the macro directive was loaded from a PCH file.
  bool isFromPCH() const { return IsFromPCH; }

  void setIsFromPCH() { IsFromPCH = true; }

  class DefInfo {
    DefMacroDirective *DefDirective = nullptr;
    SourceLocation UndefLoc;
    bool IsPublic = true;

  public:
    DefInfo() = default;
    DefInfo(DefMacroDirective *DefDirective, SourceLocation UndefLoc,
            bool isPublic)
        : DefDirective(DefDirective), UndefLoc(UndefLoc), IsPublic(isPublic) {}

    const DefMacroDirective *getDirective() const { return DefDirective; }
    DefMacroDirective *getDirective() { return DefDirective; }

    inline SourceLocation getLocation() const;
    inline MacroInfo *getMacroInfo();

    const MacroInfo *getMacroInfo() const {
      return const_cast<DefInfo *>(this)->getMacroInfo();
    }

    SourceLocation getUndefLocation() const { return UndefLoc; }
    bool isUndefined() const { return UndefLoc.isValid(); }

    bool isPublic() const { return IsPublic; }

    bool isValid() const { return DefDirective != nullptr; }
    bool isInvalid() const { return !isValid(); }

    explicit operator bool() const { return isValid(); }

    inline DefInfo getPreviousDefinition();

    const DefInfo getPreviousDefinition() const {
      return const_cast<DefInfo *>(this)->getPreviousDefinition();
    }
  };

  /// Traverses the macro directives history and returns the next
  /// macro definition directive along with info about its undefined location
  /// (if there is one) and if it is public or private.
  DefInfo getDefinition();
  const DefInfo getDefinition() const {
    return const_cast<MacroDirective *>(this)->getDefinition();
  }

  bool isDefined() const {
    if (const DefInfo Def = getDefinition())
      return !Def.isUndefined();
    return false;
  }

  const MacroInfo *getMacroInfo() const {
    return getDefinition().getMacroInfo();
  }
  MacroInfo *getMacroInfo() { return getDefinition().getMacroInfo(); }

  /// Find macro definition active in the specified source location. If
  /// this macro was not defined there, return NULL.
  const DefInfo findDirectiveAtLoc(SourceLocation L,
                                   const SourceManager &SM) const;

  void dump() const;

  static bool classof(const MacroDirective *) { return true; }
};

/// A directive for a defined macro or a macro imported from a module.
class DefMacroDirective : public MacroDirective {
  MacroInfo *Info;

public:
  DefMacroDirective(MacroInfo *MI, SourceLocation Loc)
      : MacroDirective(MD_Define, Loc), Info(MI) {
    assert(MI && "MacroInfo is null");
  }
  explicit DefMacroDirective(MacroInfo *MI)
      : DefMacroDirective(MI, MI->getDefinitionLoc()) {}

  /// The data for the macro definition.
  const MacroInfo *getInfo() const { return Info; }
  MacroInfo *getInfo() { return Info; }

  static bool classof(const MacroDirective *MD) {
    return MD->getKind() == MD_Define;
  }

  static bool classof(const DefMacroDirective *) { return true; }
};

/// A directive for an undefined macro.
class UndefMacroDirective : public MacroDirective {
public:
  explicit UndefMacroDirective(SourceLocation UndefLoc)
      : MacroDirective(MD_Undefine, UndefLoc) {
    assert(UndefLoc.isValid() && "Invalid UndefLoc!");
  }

  static bool classof(const MacroDirective *MD) {
    return MD->getKind() == MD_Undefine;
  }

  static bool classof(const UndefMacroDirective *) { return true; }
};

/// A directive for setting the module visibility of a macro.
class VisibilityMacroDirective : public MacroDirective {
public:
  explicit VisibilityMacroDirective(SourceLocation Loc, bool Public)
      : MacroDirective(MD_Visibility, Loc) {
    IsPublic = Public;
  }

  /// Determine whether this macro is part of the public API of its
  /// module.
  bool isPublic() const { return IsPublic; }

  static bool classof(const MacroDirective *MD) {
    return MD->getKind() == MD_Visibility;
  }

  static bool classof(const VisibilityMacroDirective *) { return true; }
};

inline SourceLocation MacroDirective::DefInfo::getLocation() const {
  if (isInvalid())
    return {};
  return DefDirective->getLocation();
}

inline MacroInfo *MacroDirective::DefInfo::getMacroInfo() {
  if (isInvalid())
    return nullptr;
  return DefDirective->getInfo();
}

inline MacroDirective::DefInfo
MacroDirective::DefInfo::getPreviousDefinition() {
  if (isInvalid() || DefDirective->getPrevious() == nullptr)
    return {};
  return DefDirective->getPrevious()->getDefinition();
}

/// Represents a macro directive exported by a module.
///
/// There's an instance of this class for every macro #define or #undef that is
/// the final directive for a macro name within a module. These entities also
/// represent the macro override graph.
///
/// These are stored in a FoldingSet in the preprocessor.
class ModuleMacro : public toolchain::FoldingSetNode {
  friend class Preprocessor;

  /// The name defined by the macro.
  const IdentifierInfo *II;

  /// The body of the #define, or nullptr if this is a #undef.
  MacroInfo *Macro;

  /// The module that exports this macro.
  Module *OwningModule;

  /// The number of module macros that override this one.
  unsigned NumOverriddenBy = 0;

  /// The number of modules whose macros are directly overridden by this one.
  unsigned NumOverrides;

  ModuleMacro(Module *OwningModule, const IdentifierInfo *II, MacroInfo *Macro,
              ArrayRef<ModuleMacro *> Overrides)
      : II(II), Macro(Macro), OwningModule(OwningModule),
        NumOverrides(Overrides.size()) {
    std::copy(Overrides.begin(), Overrides.end(),
              reinterpret_cast<ModuleMacro **>(this + 1));
  }

public:
  static ModuleMacro *create(Preprocessor &PP, Module *OwningModule,
                             const IdentifierInfo *II, MacroInfo *Macro,
                             ArrayRef<ModuleMacro *> Overrides);

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    return Profile(ID, OwningModule, II);
  }

  static void Profile(toolchain::FoldingSetNodeID &ID, Module *OwningModule,
                      const IdentifierInfo *II) {
    ID.AddPointer(OwningModule);
    ID.AddPointer(II);
  }

  /// Get the name of the macro.
  const IdentifierInfo *getName() const { return II; }

  /// Get the ID of the module that exports this macro.
  Module *getOwningModule() const { return OwningModule; }

  /// Get definition for this exported #define, or nullptr if this
  /// represents a #undef.
  MacroInfo *getMacroInfo() const { return Macro; }

  /// Iterators over the overridden module IDs.
  /// \{
  using overrides_iterator = ModuleMacro *const *;

  overrides_iterator overrides_begin() const {
    return reinterpret_cast<overrides_iterator>(this + 1);
  }

  overrides_iterator overrides_end() const {
    return overrides_begin() + NumOverrides;
  }

  ArrayRef<ModuleMacro *> overrides() const {
    return toolchain::ArrayRef(overrides_begin(), overrides_end());
  }
  /// \}

  /// Get the number of macros that override this one.
  unsigned getNumOverridingMacros() const { return NumOverriddenBy; }
};

/// A description of the current definition of a macro.
///
/// The definition of a macro comprises a set of (at least one) defining
/// entities, which are either local MacroDirectives or imported ModuleMacros.
class MacroDefinition {
  toolchain::PointerIntPair<DefMacroDirective *, 1, bool> LatestLocalAndAmbiguous;
  ArrayRef<ModuleMacro *> ModuleMacros;

public:
  MacroDefinition() = default;
  MacroDefinition(DefMacroDirective *MD, ArrayRef<ModuleMacro *> MMs,
                  bool IsAmbiguous)
      : LatestLocalAndAmbiguous(MD, IsAmbiguous), ModuleMacros(MMs) {}

  /// Determine whether there is a definition of this macro.
  explicit operator bool() const {
    return getLocalDirective() || !ModuleMacros.empty();
  }

  /// Get the MacroInfo that should be used for this definition.
  MacroInfo *getMacroInfo() const {
    if (!ModuleMacros.empty())
      return ModuleMacros.back()->getMacroInfo();
    if (auto *MD = getLocalDirective())
      return MD->getMacroInfo();
    return nullptr;
  }

  /// \c true if the definition is ambiguous, \c false otherwise.
  bool isAmbiguous() const { return LatestLocalAndAmbiguous.getInt(); }

  /// Get the latest non-imported, non-\#undef'd macro definition
  /// for this macro.
  DefMacroDirective *getLocalDirective() const {
    return LatestLocalAndAmbiguous.getPointer();
  }

  /// Get the active module macros for this macro.
  ArrayRef<ModuleMacro *> getModuleMacros() const { return ModuleMacros; }

  template <typename Fn> void forAllDefinitions(Fn F) const {
    if (auto *MD = getLocalDirective())
      F(MD->getMacroInfo());
    for (auto *MM : getModuleMacros())
      F(MM->getMacroInfo());
  }
};

} // namespace language::Core

#endif // LANGUAGE_CORE_LEX_MACROINFO_H
