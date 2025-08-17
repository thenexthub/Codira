//== clang/Basic/Sarif.h - SARIF Diagnostics Object Model -------*- C++ -*--==//
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
/// \file
/// Defines language::Core::SarifDocumentWriter, language::Core::SarifRule, language::Core::SarifResult.
///
/// The document built can be accessed as a JSON Object.
/// Several value semantic types are also introduced which represent properties
/// of the SARIF standard, such as 'artifact', 'result', 'rule'.
///
/// A SARIF (Static Analysis Results Interchange Format) document is JSON
/// document that describes in detail the results of running static analysis
/// tools on a project. Each (non-trivial) document consists of at least one
/// "run", which are themselves composed of details such as:
/// * Tool: The tool that was run
/// * Rules: The rules applied during the tool run, represented by
///   \c reportingDescriptor objects in SARIF
/// * Results: The matches for the rules applied against the project(s) being
///   evaluated, represented by \c result objects in SARIF
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html">The SARIF standard</a>
/// 2. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317836">SARIF<pre>reportingDescriptor</pre></a>
/// 3. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317638">SARIF<pre>result</pre></a>
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_SARIF_H
#define LANGUAGE_CORE_BASIC_SARIF_H

#include "language/Core/Basic/SourceLocation.h"
#include "language/Core/Basic/Version.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/JSON.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string>

namespace language::Core {

class SarifDocumentWriter;
class SourceManager;

namespace detail {

/// \internal
/// An artifact location is SARIF's way of describing the complete location
/// of an artifact encountered during analysis. The \c artifactLocation object
/// typically consists of a URI, and/or an index to reference the artifact it
/// locates.
///
/// This builder makes an additional assumption: that every artifact encountered
/// by \c clang will be a physical, top-level artifact. Which is why the static
/// creation method \ref SarifArtifactLocation::create takes a mandatory URI
/// parameter. The official standard states that either a \c URI or \c Index
/// must be available in the object, \c clang picks the \c URI as a reasonable
/// default, because it intends to deal in physical artifacts for now.
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317427">artifactLocation object</a>
/// 2. \ref SarifArtifact
class SarifArtifactLocation {
private:
  friend class language::Core::SarifDocumentWriter;

  std::optional<uint32_t> Index;
  std::string URI;

  SarifArtifactLocation() = delete;
  explicit SarifArtifactLocation(const std::string &URI) : URI(URI) {}

public:
  static SarifArtifactLocation create(toolchain::StringRef URI) {
    return SarifArtifactLocation{URI.str()};
  }

  SarifArtifactLocation setIndex(uint32_t Idx) {
    Index = Idx;
    return *this;
  }
};

/// \internal
/// An artifact in SARIF is any object (a sequence of bytes) addressable by
/// a URI (RFC 3986). The most common type of artifact for clang's use-case
/// would be source files. SARIF's artifact object is described in detail in
/// section 3.24.
//
/// Since every clang artifact MUST have a location (there being no nested
/// artifacts), the creation method \ref SarifArtifact::create requires a
/// \ref SarifArtifactLocation object.
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317611">artifact object</a>
class SarifArtifact {
private:
  friend class language::Core::SarifDocumentWriter;

  std::optional<uint32_t> Offset;
  std::optional<size_t> Length;
  std::string MimeType;
  SarifArtifactLocation Location;
  toolchain::SmallVector<std::string, 4> Roles;

  SarifArtifact() = delete;

  explicit SarifArtifact(const SarifArtifactLocation &Loc) : Location(Loc) {}

public:
  static SarifArtifact create(const SarifArtifactLocation &Loc) {
    return SarifArtifact{Loc};
  }

  SarifArtifact setOffset(uint32_t ArtifactOffset) {
    Offset = ArtifactOffset;
    return *this;
  }

  SarifArtifact setLength(size_t NumBytes) {
    Length = NumBytes;
    return *this;
  }

  SarifArtifact setRoles(std::initializer_list<toolchain::StringRef> ArtifactRoles) {
    Roles.assign(ArtifactRoles.begin(), ArtifactRoles.end());
    return *this;
  }

  SarifArtifact setMimeType(toolchain::StringRef ArtifactMimeType) {
    MimeType = ArtifactMimeType.str();
    return *this;
  }
};

} // namespace detail

enum class ThreadFlowImportance { Important, Essential, Unimportant };

/// The level of severity associated with a \ref SarifResult.
///
/// Of all the levels, \c None is the only one that is not associated with
/// a failure.
///
/// A typical mapping for clang's DiagnosticKind to SarifResultLevel would look
/// like:
/// * \c None: \ref language::Core::DiagnosticsEngine::Level::Remark, \ref language::Core::DiagnosticsEngine::Level::Ignored
/// * \c Note: \ref language::Core::DiagnosticsEngine::Level::Note
/// * \c Warning: \ref language::Core::DiagnosticsEngine::Level::Warning
/// * \c Error could be generated from one of:
///   - \ref language::Core::DiagnosticsEngine::Level::Warning with \c -Werror
///   - \ref language::Core::DiagnosticsEngine::Level::Error
///   - \ref language::Core::DiagnosticsEngine::Level::Fatal when \ref language::Core::DiagnosticsEngine::ErrorsAsFatal is set.
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317648">level property</a>
enum class SarifResultLevel { None, Note, Warning, Error };

/// A thread flow is a sequence of code locations that specify a possible path
/// through a single thread of execution.
/// A thread flow in SARIF is related to a code flow which describes
/// the progress of one or more programs through one or more thread flows.
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317744">threadFlow object</a>
/// 2. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317740">codeFlow object</a>
class ThreadFlow {
  friend class SarifDocumentWriter;

  CharSourceRange Range;
  ThreadFlowImportance Importance;
  std::string Message;

  ThreadFlow() = default;

public:
  static ThreadFlow create() { return {}; }

  ThreadFlow setRange(const CharSourceRange &ItemRange) {
    assert(ItemRange.isCharRange() &&
           "ThreadFlows require a character granular source range!");
    Range = ItemRange;
    return *this;
  }

  ThreadFlow setImportance(const ThreadFlowImportance &ItemImportance) {
    Importance = ItemImportance;
    return *this;
  }

  ThreadFlow setMessage(toolchain::StringRef ItemMessage) {
    Message = ItemMessage.str();
    return *this;
  }
};

/// A SARIF Reporting Configuration (\c reportingConfiguration) object contains
/// properties for a \ref SarifRule that can be configured at runtime before
/// analysis begins.
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317852">reportingConfiguration object</a>
class SarifReportingConfiguration {
  friend class language::Core::SarifDocumentWriter;

  bool Enabled = true;
  SarifResultLevel Level = SarifResultLevel::Warning;
  float Rank = -1.0f;

  SarifReportingConfiguration() = default;

public:
  static SarifReportingConfiguration create() { return {}; };

  SarifReportingConfiguration disable() {
    Enabled = false;
    return *this;
  }

  SarifReportingConfiguration enable() {
    Enabled = true;
    return *this;
  }

  SarifReportingConfiguration setLevel(SarifResultLevel TheLevel) {
    Level = TheLevel;
    return *this;
  }

  SarifReportingConfiguration setRank(float TheRank) {
    assert(TheRank >= 0.0f && "Rule rank cannot be smaller than 0.0");
    assert(TheRank <= 100.0f && "Rule rank cannot be larger than 100.0");
    Rank = TheRank;
    return *this;
  }
};

/// A SARIF rule (\c reportingDescriptor object) contains information that
/// describes a reporting item generated by a tool. A reporting item is
/// either a result of analysis or notification of a condition encountered by
/// the tool. Rules are arbitrary but are identifiable by a hierarchical
/// rule-id.
///
/// This builder provides an interface to create SARIF \c reportingDescriptor
/// objects via the \ref SarifRule::create static method.
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317836">reportingDescriptor object</a>
class SarifRule {
  friend class language::Core::SarifDocumentWriter;

  std::string Name;
  std::string Id;
  std::string Description;
  std::string HelpURI;
  SarifReportingConfiguration DefaultConfiguration;

  SarifRule() : DefaultConfiguration(SarifReportingConfiguration::create()) {}

public:
  static SarifRule create() { return {}; }

  SarifRule setName(toolchain::StringRef RuleName) {
    Name = RuleName.str();
    return *this;
  }

  SarifRule setRuleId(toolchain::StringRef RuleId) {
    Id = RuleId.str();
    return *this;
  }

  SarifRule setDescription(toolchain::StringRef RuleDesc) {
    Description = RuleDesc.str();
    return *this;
  }

  SarifRule setHelpURI(toolchain::StringRef RuleHelpURI) {
    HelpURI = RuleHelpURI.str();
    return *this;
  }

  SarifRule
  setDefaultConfiguration(const SarifReportingConfiguration &Configuration) {
    DefaultConfiguration = Configuration;
    return *this;
  }
};

/// A SARIF result (also called a "reporting item") is a unit of output
/// produced when one of the tool's \c reportingDescriptor encounters a match
/// on the file being analysed by the tool.
///
/// This builder provides a \ref SarifResult::create static method that can be
/// used to create an empty shell onto which attributes can be added using the
/// \c setX(...) methods.
///
/// For example:
/// \code{.cpp}
/// SarifResult result = SarifResult::create(...)
///                         .setRuleId(...)
///                         .setDiagnosticMessage(...);
/// \endcode
///
/// Reference:
/// 1. <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317638">SARIF<pre>result</pre></a>
class SarifResult {
  friend class language::Core::SarifDocumentWriter;

  // NOTE:
  // This type cannot fit all possible indexes representable by JSON, but is
  // chosen because it is the largest unsigned type that can be safely
  // converted to an \c int64_t.
  uint32_t RuleIdx;
  std::string RuleId;
  std::string DiagnosticMessage;
  toolchain::SmallVector<CharSourceRange, 8> Locations;
  toolchain::SmallVector<ThreadFlow, 8> ThreadFlows;
  std::optional<SarifResultLevel> LevelOverride;

  SarifResult() = delete;
  explicit SarifResult(uint32_t RuleIdx) : RuleIdx(RuleIdx) {}

public:
  static SarifResult create(uint32_t RuleIdx) { return SarifResult{RuleIdx}; }

  SarifResult setIndex(uint32_t Idx) {
    RuleIdx = Idx;
    return *this;
  }

  SarifResult setRuleId(toolchain::StringRef Id) {
    RuleId = Id.str();
    return *this;
  }

  SarifResult setDiagnosticMessage(toolchain::StringRef Message) {
    DiagnosticMessage = Message.str();
    return *this;
  }

  SarifResult setLocations(toolchain::ArrayRef<CharSourceRange> DiagLocs) {
#ifndef NDEBUG
    for (const auto &Loc : DiagLocs) {
      assert(Loc.isCharRange() &&
             "SARIF Results require character granular source ranges!");
    }
#endif
    Locations.assign(DiagLocs.begin(), DiagLocs.end());
    return *this;
  }
  SarifResult setThreadFlows(toolchain::ArrayRef<ThreadFlow> ThreadFlowResults) {
    ThreadFlows.assign(ThreadFlowResults.begin(), ThreadFlowResults.end());
    return *this;
  }

  SarifResult setDiagnosticLevel(const SarifResultLevel &TheLevel) {
    LevelOverride = TheLevel;
    return *this;
  }
};

/// This class handles creating a valid SARIF document given various input
/// attributes. However, it requires an ordering among certain method calls:
///
/// 1. Because every SARIF document must contain at least 1 \c run, callers
///    must ensure that \ref SarifDocumentWriter::createRun is called before
///    any other methods.
/// 2. If SarifDocumentWriter::endRun is called, callers MUST call
///    SarifDocumentWriter::createRun, before invoking any of the result
///    aggregation methods such as SarifDocumentWriter::appendResult etc.
class SarifDocumentWriter {
private:
  const toolchain::StringRef SchemaURI{
      "https://docs.oasis-open.org/sarif/sarif/v2.1.0/cos02/schemas/"
      "sarif-schema-2.1.0.json"};
  const toolchain::StringRef SchemaVersion{"2.1.0"};

  /// \internal
  /// Return a pointer to the current tool. Asserts that a run exists.
  toolchain::json::Object &getCurrentTool();

  /// \internal
  /// Checks if there is a run associated with this document.
  ///
  /// \return true on success
  bool hasRun() const;

  /// \internal
  /// Reset portions of the internal state so that the document is ready to
  /// receive data for a new run.
  void reset();

  /// \internal
  /// Return a mutable reference to the current run, after asserting it exists.
  ///
  /// \note It is undefined behavior to call this if a run does not exist in
  /// the SARIF document.
  toolchain::json::Object &getCurrentRun();

  /// Create a code flow object for the given threadflows.
  /// See \ref ThreadFlow.
  ///
  /// \note It is undefined behavior to call this if a run does not exist in
  /// the SARIF document.
  toolchain::json::Object
  createCodeFlow(const toolchain::ArrayRef<ThreadFlow> ThreadFlows);

  /// Add the given threadflows to the ones this SARIF document knows about.
  toolchain::json::Array
  createThreadFlows(const toolchain::ArrayRef<ThreadFlow> ThreadFlows);

  /// Add the given \ref CharSourceRange to the SARIF document as a physical
  /// location, with its corresponding artifact.
  toolchain::json::Object createPhysicalLocation(const CharSourceRange &R);

public:
  SarifDocumentWriter() = delete;

  /// Create a new empty SARIF document with the given source manager.
  SarifDocumentWriter(const SourceManager &SourceMgr) : SourceMgr(SourceMgr) {}

  /// Release resources held by this SARIF document.
  ~SarifDocumentWriter() = default;

  /// Create a new run with which any upcoming analysis will be associated.
  /// Each run requires specifying the tool that is generating reporting items.
  void createRun(const toolchain::StringRef ShortToolName,
                 const toolchain::StringRef LongToolName,
                 const toolchain::StringRef ToolVersion = CLANG_VERSION_STRING);

  /// If there is a current run, end it.
  ///
  /// This method collects various book-keeping required to clear and close
  /// resources associated with the current run, but may also allocate some
  /// for the next run.
  ///
  /// Calling \ref endRun before associating a run through \ref createRun leads
  /// to undefined behaviour.
  void endRun();

  /// Associate the given rule with the current run.
  ///
  /// Returns an integer rule index for the created rule that is unique within
  /// the current run, which can then be used to create a \ref SarifResult
  /// to add to the current run. Note that a rule must exist before being
  /// referenced by a result.
  ///
  /// \pre
  /// There must be a run associated with the document, failing to do so will
  /// cause undefined behaviour.
  size_t createRule(const SarifRule &Rule);

  /// Append a new result to the currently in-flight run.
  ///
  /// \pre
  /// There must be a run associated with the document, failing to do so will
  /// cause undefined behaviour.
  /// \pre
  /// \c RuleIdx used to create the result must correspond to a rule known by
  /// the SARIF document. It must be the value returned by a previous call
  /// to \ref createRule.
  void appendResult(const SarifResult &SarifResult);

  /// Return the SARIF document in its current state.
  /// Calling this will trigger a copy of the internal state including all
  /// reported diagnostics, resulting in an expensive call.
  toolchain::json::Object createDocument();

private:
  /// Source Manager to use for the current SARIF document.
  const SourceManager &SourceMgr;

  /// Flag to track the state of this document:
  /// A closed document is one on which a new runs must be created.
  /// This could be a document that is freshly created, or has recently
  /// finished writing to a previous run.
  bool Closed = true;

  /// A sequence of SARIF runs.
  /// Each run object describes a single run of an analysis tool and contains
  /// the output of that run.
  ///
  /// Reference: <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317484">run object</a>
  toolchain::json::Array Runs;

  /// The list of rules associated with the most recent active run. These are
  /// defined using the diagnostics passed to the SarifDocument. Each rule
  /// need not be unique through the result set. E.g. there may be several
  /// 'syntax' errors throughout code under analysis, each of which has its
  /// own specific diagnostic message (and consequently, RuleId). Rules are
  /// also known as "reportingDescriptor" objects in SARIF.
  ///
  /// Reference: <a href="https://docs.oasis-open.org/sarif/sarif/v2.1.0/os/sarif-v2.1.0-os.html#_Toc34317556">rules property</a>
  toolchain::SmallVector<SarifRule, 32> CurrentRules;

  /// The list of artifacts that have been encountered on the most recent active
  /// run. An artifact is defined in SARIF as a sequence of bytes addressable
  /// by a URI. A common example for clang's case would be files named by
  /// filesystem paths.
  toolchain::StringMap<detail::SarifArtifact> CurrentArtifacts;
};
} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_SARIF_H
