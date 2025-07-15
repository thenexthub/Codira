//===------CompileJobCacheResult.h-- -----------------------------*-C++ -*-===//
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

#ifndef LANGUAGE_FRONTEND_COMPILEJOBCACHERESULT_H
#define LANGUAGE_FRONTEND_COMPILEJOBCACHERESULT_H

#include "language/Basic/FileTypes.h"
#include "toolchain/CAS/CASNodeSchema.h"
#include "toolchain/CAS/ObjectStore.h"

namespace language::cas {

class CompileJobResultSchema;
class CompileJobCacheResult : public toolchain::cas::ObjectProxy {
public:
  /// A single output file or stream.
  struct Output {
    /// The CAS object for this output.
    toolchain::cas::ObjectRef Object;
    /// The output kind.
    file_types::ID Kind;

    bool operator==(const Output &Other) const {
      return Object == Other.Object && Kind == Other.Kind;
    }
  };

  /// Retrieves each \c Output from this result.
  toolchain::Error
  forEachOutput(toolchain::function_ref<toolchain::Error(Output)> Callback) const;

  /// Loads all outputs concurrently and passes the resulting \c ObjectProxy
  /// objects to \p Callback. If there was an error during loading then the
  /// callback will not be invoked.
  toolchain::Error forEachLoadedOutput(
      toolchain::function_ref<toolchain::Error(Output, std::optional<ObjectProxy>)>
          Callback);

  size_t getNumOutputs() const;

  Output getOutput(size_t I) const;

  /// Retrieves a specific output specified by \p Kind, if it exists.
  std::optional<Output> getOutput(file_types::ID Kind) const;

  /// Print this result to \p OS.
  toolchain::Error print(toolchain::raw_ostream &OS);

  /// Helper to build a \c CompileJobCacheResult from individual outputs.
  class Builder {
  public:
    Builder();
    ~Builder();
    /// Treat outputs added for \p Path as having the given \p Kind. Otherwise
    /// they will have kind \c Unknown.
    void addKindMap(file_types::ID Kind, StringRef Path);
    /// Add an output with an explicit \p Kind.
    void addOutput(file_types::ID Kind, toolchain::cas::ObjectRef Object);
    /// Add an output for the given \p Path. There must be a a kind map for it.
    toolchain::Error addOutput(StringRef Path, toolchain::cas::ObjectRef Object);
    /// Build a single \c ObjectRef representing the provided outputs. The
    /// result can be used with \c CompileJobResultSchema to retrieve the
    /// original outputs.
    toolchain::Expected<toolchain::cas::ObjectRef> build(toolchain::cas::ObjectStore &CAS);

  private:
    struct PrivateImpl;
    PrivateImpl &Impl;
  };

private:
  toolchain::cas::ObjectRef getOutputObject(size_t I) const;
  toolchain::cas::ObjectRef getPathsListRef() const;
  file_types::ID getOutputKind(size_t I) const;
  toolchain::Expected<toolchain::cas::ObjectRef> getOutputPath(size_t I) const;

private:
  friend class CompileJobResultSchema;
  CompileJobCacheResult(const ObjectProxy &);
};

class CompileJobResultSchema
    : public toolchain::RTTIExtends<CompileJobResultSchema, toolchain::cas::NodeSchema> {
public:
  static char ID;

  CompileJobResultSchema(toolchain::cas::ObjectStore &CAS);

  /// Attempt to load \p Ref as a \c CompileJobCacheResult if it matches the
  /// schema.
  toolchain::Expected<CompileJobCacheResult> load(toolchain::cas::ObjectRef Ref) const;

  bool isRootNode(const CompileJobCacheResult::ObjectProxy &Node) const final;
  bool isNode(const CompileJobCacheResult::ObjectProxy &Node) const final;

  /// Get this schema's marker node.
  toolchain::cas::ObjectRef getKindRef() const { return KindRef; }

private:
  toolchain::cas::ObjectRef KindRef;
};

} // namespace language::cas

#endif
