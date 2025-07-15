//===--- CodeCompletionResultSink.h ---------------------------------------===//
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

#ifndef LANGUAGE_IDE_CODECOMPLETIONRESULTSINK
#define LANGUAGE_IDE_CODECOMPLETIONRESULTSINK

#include "language/IDE/CodeCompletionResult.h"

namespace language {
namespace ide {

struct CodeCompletionResultSink {
  using AllocatorPtr = std::shared_ptr<toolchain::BumpPtrAllocator>;

  /// The allocator used to allocate results "native" to this sink.
  AllocatorPtr Allocator;

  /// Allocators that keep alive "foreign" results imported into this sink from
  /// other sinks.
  std::vector<AllocatorPtr> ForeignAllocators;

  /// Whether to annotate the results with XML.
  bool annotateResult = false;

  /// Whether to emit object literals if desired.
  bool includeObjectLiterals = true;

  /// Whether to emit type initializers in addition to type names in expression
  /// position.
  bool addInitsToTopLevel = false;

  /// Whether to include an item without any default arguments.
  bool addCallWithNoDefaultArgs = true;

private:
  /// Whether the code completion results computed for this sink are intended to
  /// only be stored in the cache. In this case no contextual information is
  /// computed and all types in \c ContextFreeCodeCompletionResult should be
  /// USR-based instead of AST-based.
  USRBasedTypeArena *USRTypeArena = nullptr;

public:
  std::vector<CodeCompletionResult *> Results;

  /// A single-element cache for module names stored in Allocator, keyed by a
  /// clang::Module * or language::ModuleDecl *.
  std::pair<void *, NullTerminatedStringRef> LastModule;

  CodeCompletionResultSink()
      : Allocator(std::make_shared<toolchain::BumpPtrAllocator>()) {}

  toolchain::BumpPtrAllocator &getAllocator() { return *Allocator; }

  /// Marks the sink as producing results for the code completion cache.
  /// In this case the produced results will not contain any contextual
  /// information and all types in the \c ContextFreeCodeCompletionResult are
  /// USR-based.
  void setProduceContextFreeResults(USRBasedTypeArena &USRTypeArena) {
    this->USRTypeArena = &USRTypeArena;
  }

  /// See \c setProduceContextFreeResults.
  bool shouldProduceContextFreeResults() const {
    return USRTypeArena != nullptr;
  }

  /// If \c shouldProduceContextFreeResults is \c true, returns the arena in
  /// which the USR-based types of the \c ContextFreeCodeCompletionResult should
  /// be stored.
  USRBasedTypeArena &getUSRTypeArena() const {
    assert(USRTypeArena != nullptr &&
           "Must only be called if shouldProduceContextFreeResults is true");
    return *USRTypeArena;
  }
};

} // end namespace ide
} // end namespace language

#endif // LANGUAGE_IDE_CODECOMPLETIONRESULTSINK
