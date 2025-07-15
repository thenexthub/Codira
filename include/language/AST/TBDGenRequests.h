//===--- TBDGenRequests.h - TBDGen Requests ---------------------*- C++ -*-===//
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
//  This file defines TBDGen requests.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_TBDGEN_REQUESTS_H
#define LANGUAGE_TBDGEN_REQUESTS_H

#include "toolchain/ExecutionEngine/JITSymbol.h"

#include "language/AST/ASTTypeIDs.h"
#include "language/AST/SimpleRequest.h"
#include "language/IRGen/Linking.h"
#include "language/IRGen/TBDGen.h"
#include "language/SIL/SILDeclRef.h"

namespace toolchain {

class DataLayout;
class Triple;

namespace MachO {
class InterfaceFile;
} // end namespace MachO

template <class AllocatorTy>
class StringSet;
} // end namespace toolchain

namespace language {

class FileUnit;
class ModuleDecl;

namespace apigen {
class API;
} // end namespace apigen

class TBDGenDescriptor final {
  using FileOrModule = toolchain::PointerUnion<FileUnit *, ModuleDecl *>;
  FileOrModule Input;
  TBDGenOptions Opts;

  TBDGenDescriptor(FileOrModule input, const TBDGenOptions &opts)
      : Input(input), Opts(opts) {
    assert(input);
  }

public:
  /// Returns the file or module we're emitting TBD for.
  FileOrModule getFileOrModule() const { return Input; }

  /// If the input is a single file, returns that file. Otherwise returns
  /// \c nullptr.
  FileUnit *getSingleFile() const;

  /// Returns the parent module for TBD emission.
  ModuleDecl *getParentModule() const;

  /// Returns the TBDGen options.
  const TBDGenOptions &getOptions() const { return Opts; }
  TBDGenOptions &getOptions() { return Opts; }

  const StringRef getDataLayoutString() const;
  const toolchain::Triple &getTarget() const;

  bool operator==(const TBDGenDescriptor &other) const;
  bool operator!=(const TBDGenDescriptor &other) const {
    return !(*this == other);
  }

  static TBDGenDescriptor forFile(FileUnit *file, const TBDGenOptions &opts) {
    return TBDGenDescriptor(file, opts);
  }

  static TBDGenDescriptor forModule(ModuleDecl *M, const TBDGenOptions &opts) {
    return TBDGenDescriptor(M, opts);
  }
};

toolchain::hash_code hash_value(const TBDGenDescriptor &desc);
void simple_display(toolchain::raw_ostream &out, const TBDGenDescriptor &desc);
SourceLoc extractNearestSourceLoc(const TBDGenDescriptor &desc);

using TBDFile = toolchain::MachO::InterfaceFile;

/// Computes the TBD file for a given Codira module or file.
class GenerateTBDRequest
    : public SimpleRequest<GenerateTBDRequest,
                           TBDFile(TBDGenDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  TBDFile evaluate(Evaluator &evaluator, TBDGenDescriptor desc) const;
};

/// Retrieve the public symbols for a file or module.
class PublicSymbolsRequest
    : public SimpleRequest<PublicSymbolsRequest,
                           std::vector<std::string>(TBDGenDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  std::vector<std::string>
  evaluate(Evaluator &evaluator, TBDGenDescriptor desc) const;
};

/// Retrieve API information for a file or module.
class APIGenRequest
    : public SimpleRequest<APIGenRequest,
                           apigen::API(TBDGenDescriptor),
                           RequestFlags::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  apigen::API evaluate(Evaluator &evaluator, TBDGenDescriptor desc) const;
};

/// Describes the origin of a particular symbol, including the stage of
/// compilation it is introduced, as well as information on what decl introduces
/// it.
class SymbolSource {
public:
  enum class Kind {
    /// A symbol introduced when emitting a SIL decl.
    SIL,

    /// A symbol introduced when emitting a SIL global variable.
    /// Only used in piecewise compilation for Immediate mode.
    Global,

    /// A symbol introduced when emitting LLVM IR.
    IR,

    /// A symbol used to customize linker behavior, introduced by TBDGen.
    LinkerDirective,

    /// A symbol with an unknown origin.
    // FIXME: This should be eliminated.
    Unknown
  };
  Kind kind;

private:
  union {
    VarDecl *Global;
    SILDeclRef silDeclRef;
    irgen::LinkEntity irEntity;
  };

  explicit SymbolSource(SILDeclRef ref) : kind(Kind::SIL) {
    silDeclRef = ref;
  }

  SymbolSource(VarDecl *Global) : kind(Kind::Global), Global(Global) {}

  explicit SymbolSource(irgen::LinkEntity entity) : kind(Kind::IR) {
    irEntity = entity;
  }
  explicit SymbolSource(Kind kind) : kind(kind) {
    assert(kind == Kind::LinkerDirective || kind == Kind::Unknown);
  }

public:
  static SymbolSource forSILDeclRef(SILDeclRef ref) {
    return SymbolSource{ref};
  }

  static SymbolSource forGlobal(VarDecl *Global) { return Global; }

  static SymbolSource forIRLinkEntity(irgen::LinkEntity entity) {
    return SymbolSource{entity};
  }
  static SymbolSource forLinkerDirective() {
    return SymbolSource{Kind::LinkerDirective};
  }
  static SymbolSource forUnknown() {
    return SymbolSource{Kind::Unknown};
  }

  bool isLinkerDirective() const {
    return kind == Kind::LinkerDirective;
  }

  SILDeclRef getSILDeclRef() const {
    assert(kind == Kind::SIL);
    return silDeclRef;
  }

  VarDecl *getGlobal() const {
    assert(kind == Kind::Global);
    return Global;
  }

  irgen::LinkEntity getIRLinkEntity() const {
    assert(kind == Kind::IR);
    return irEntity;
  }

  /// Typecheck the entity wrapped by this `SymbolSource`
  void typecheck() const {
    switch (kind) {
    case Kind::SIL: {
      if (auto *AFD = silDeclRef.getAbstractFunctionDecl())
        // If this entity is a `SILFunction`, check its body
        AFD->getTypecheckedBody();
      break;
    }
    case Kind::Global:
    case Kind::IR:
    case Kind::LinkerDirective:
    case Kind::Unknown:
      // Nothing to do
      break;
    }
  }

  friend toolchain::hash_code hash_value(const SymbolSource &S) {
    auto Kind = S.kind;
    switch (Kind) {
    case Kind::SIL:
      return toolchain::hash_combine(Kind, S.silDeclRef);
    case Kind::Global:
      return toolchain::hash_combine(Kind, S.Global);
    case Kind::IR:
      return toolchain::hash_combine(Kind, S.irEntity);
    case Kind::LinkerDirective:
    case Kind::Unknown:
      return toolchain::hash_value(Kind);
    }
  }

  friend bool operator==(const SymbolSource &LHS, const SymbolSource &RHS) {
    if (LHS.kind != RHS.kind)
      return false;
    switch (LHS.kind) {
    case Kind::SIL:
      return LHS.silDeclRef == RHS.silDeclRef;
    case Kind::Global:
      return LHS.Global == RHS.Global;
    case Kind::IR:
      return LHS.irEntity == RHS.irEntity;
    case Kind::LinkerDirective:
    case Kind::Unknown:
      return true;
    }
  }

  friend bool operator!=(const SymbolSource &LHS, const SymbolSource &RHS) {
    return !(LHS == RHS);
  }

  toolchain::JITSymbolFlags getJITSymbolFlags() const {
    switch (kind) {
    case Kind::SIL:
      return toolchain::JITSymbolFlags::Callable | toolchain::JITSymbolFlags::Exported;
    case Kind::Global:
      return toolchain::JITSymbolFlags::Exported;
    case Kind::IR:
      toolchain_unreachable("Unimplemented: Symbol flags for LinkEntities");
    case Kind::LinkerDirective:
      toolchain_unreachable("Unsupported: Symbol flags for linker directives");
    case Kind::Unknown:
      toolchain_unreachable("Unsupported: Symbol flags for unknown source");
    }
  }
};

/// Maps a symbol back to its source for lazy compilation.
using SymbolSourceMap = toolchain::StringMap<SymbolSource>;

/// Computes a map of symbols to their SymbolSource for a file or module.
class SymbolSourceMapRequest
    : public SimpleRequest<SymbolSourceMapRequest,
                           const SymbolSourceMap *(TBDGenDescriptor),
                           RequestFlags::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  const SymbolSourceMap *evaluate(Evaluator &evaluator,
                                  TBDGenDescriptor desc) const;

public:
  // Cached.
  bool isCached() const { return true; }
};

/// Report that a request of the given kind is being evaluated, so it
/// can be recorded by the stats reporter.
template <typename Request>
void reportEvaluatedRequest(UnifiedStatsReporter &stats,
                            const Request &request);

/// The zone number for TBDGen.
#define LANGUAGE_TYPEID_ZONE TBDGen
#define LANGUAGE_TYPEID_HEADER "language/AST/TBDGenTypeIDZone.def"
#include "language/Basic/DefineTypeIDZone.h"
#undef LANGUAGE_TYPEID_ZONE
#undef LANGUAGE_TYPEID_HEADER

// Set up reporting of evaluated requests.
#define LANGUAGE_REQUEST(Zone, RequestType, Sig, Caching, LocOptions)             \
template<>                                                                     \
inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,                \
                                   const RequestType &request) {               \
  ++stats.getFrontendCounters().RequestType;                                   \
}
#include "language/AST/TBDGenTypeIDZone.def"
#undef LANGUAGE_REQUEST

} // end namespace language

#endif // LANGUAGE_TBDGEN_REQUESTS_H
