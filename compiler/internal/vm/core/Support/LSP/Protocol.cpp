//===--- Protocol.cpp - Language Server Protocol Implementation -----------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file contains the serialization code for the LSP structs.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/LSP/Protocol.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringSet.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/JSON.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;
using namespace vm::core::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
static bool mapOptOrNull(const toolchain::json::Value &Params,
                         toolchain::StringLiteral Prop, T &Out,
                         toolchain::json::Path Path) {
  const toolchain::json::Object *O = Params.getAsObject();
  assert(O);

  // Field is missing or null.
  auto *V = O->get(Prop);
  if (!V || V->getAsNull())
    return true;
  return fromJSON(*V, Out, Path.field(Prop));
}

//===----------------------------------------------------------------------===//
// LSPError
//===----------------------------------------------------------------------===//

char LSPError::ID;

//===----------------------------------------------------------------------===//
// URIForFile
//===----------------------------------------------------------------------===//

static bool isWindowsPath(StringRef Path) {
  return Path.size() > 1 && toolchain::isAlpha(Path[0]) && Path[1] == ':';
}

static bool isNetworkPath(StringRef Path) {
  return Path.size() > 2 && Path[0] == Path[1] &&
         toolchain::sys::path::is_separator(Path[0]);
}

static bool shouldEscapeInURI(unsigned char C) {
  // Unreserved characters.
  if ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') ||
      (C >= '0' && C <= '9'))
    return false;

  switch (C) {
  case '-':
  case '_':
  case '.':
  case '~':
  // '/' is only reserved when parsing.
  case '/':
  // ':' is only reserved for relative URI paths, which we doesn't produce.
  case ':':
    return false;
  }
  return true;
}

/// Encodes a string according to percent-encoding.
/// - Unreserved characters are not escaped.
/// - Reserved characters always escaped with exceptions like '/'.
/// - All other characters are escaped.
static void percentEncode(StringRef Content, std::string &Out) {
  for (unsigned char C : Content) {
    if (shouldEscapeInURI(C)) {
      Out.push_back('%');
      Out.push_back(toolchain::hexdigit(C / 16));
      Out.push_back(toolchain::hexdigit(C % 16));
    } else {
      Out.push_back(C);
    }
  }
}

/// Decodes a string according to percent-encoding.
static std::string percentDecode(StringRef Content) {
  std::string Result;
  for (auto I = Content.begin(), E = Content.end(); I != E; ++I) {
    if (*I == '%' && I + 2 < Content.end() && toolchain::isHexDigit(*(I + 1)) &&
        toolchain::isHexDigit(*(I + 2))) {
      Result.push_back(toolchain::hexFromNibbles(*(I + 1), *(I + 2)));
      I += 2;
    } else {
      Result.push_back(*I);
    }
  }
  return Result;
}

/// Return the set containing the supported URI schemes.
static StringSet<> &getSupportedSchemes() {
  static StringSet<> Schemes({"file", "test"});
  return Schemes;
}

/// Returns true if the given scheme is structurally valid, i.e. it does not
/// contain any invalid scheme characters. This does not check that the scheme
/// is actually supported.
static bool isStructurallyValidScheme(StringRef Scheme) {
  if (Scheme.empty())
    return false;
  if (!toolchain::isAlpha(Scheme[0]))
    return false;
  return toolchain::all_of(toolchain::drop_begin(Scheme), [](char C) {
    return toolchain::isAlnum(C) || C == '+' || C == '.' || C == '-';
  });
}

static toolchain::Expected<std::string> uriFromAbsolutePath(StringRef AbsolutePath,
                                                       StringRef Scheme) {
  std::string Body;
  StringRef Authority;
  StringRef Root = toolchain::sys::path::root_name(AbsolutePath);
  if (isNetworkPath(Root)) {
    // Windows UNC paths e.g. \\server\share => file://server/share
    Authority = Root.drop_front(2);
    AbsolutePath.consume_front(Root);
  } else if (isWindowsPath(Root)) {
    // Windows paths e.g. X:\path => file:///X:/path
    Body = "/";
  }
  Body += toolchain::sys::path::convert_to_slash(AbsolutePath);

  std::string Uri = Scheme.str() + ":";
  if (Authority.empty() && Body.empty())
    return Uri;

  // If authority if empty, we only print body if it starts with "/"; otherwise,
  // the URI is invalid.
  if (!Authority.empty() || StringRef(Body).starts_with("/")) {
    Uri.append("//");
    percentEncode(Authority, Uri);
  }
  percentEncode(Body, Uri);
  return Uri;
}

static toolchain::Expected<std::string> getAbsolutePath(StringRef Authority,
                                                   StringRef Body) {
  if (!Body.starts_with("/"))
    return toolchain::createStringError(
        toolchain::inconvertibleErrorCode(),
        "File scheme: expect body to be an absolute path starting "
        "with '/': " +
            Body);
  SmallString<128> Path;
  if (!Authority.empty()) {
    // Windows UNC paths e.g. file://server/share => \\server\share
    ("//" + Authority).toVector(Path);
  } else if (isWindowsPath(Body.substr(1))) {
    // Windows paths e.g. file:///X:/path => X:\path
    Body.consume_front("/");
  }
  Path.append(Body);
  toolchain::sys::path::native(Path);
  return std::string(Path);
}

static toolchain::Expected<std::string> parseFilePathFromURI(StringRef OrigUri) {
  StringRef Uri = OrigUri;

  // Decode the scheme of the URI.
  size_t Pos = Uri.find(':');
  if (Pos == StringRef::npos)
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "Scheme must be provided in URI: " +
                                       OrigUri);
  StringRef SchemeStr = Uri.substr(0, Pos);
  std::string UriScheme = percentDecode(SchemeStr);
  if (!isStructurallyValidScheme(UriScheme))
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "Invalid scheme: " + SchemeStr +
                                       " (decoded: " + UriScheme + ")");
  Uri = Uri.substr(Pos + 1);

  // Decode the authority of the URI.
  std::string UriAuthority;
  if (Uri.consume_front("//")) {
    Pos = Uri.find('/');
    UriAuthority = percentDecode(Uri.substr(0, Pos));
    Uri = Uri.substr(Pos);
  }

  // Decode the body of the URI.
  std::string UriBody = percentDecode(Uri);

  // Compute the absolute path for this uri.
  if (!getSupportedSchemes().contains(UriScheme)) {
    return toolchain::createStringError(toolchain::inconvertibleErrorCode(),
                                   "unsupported URI scheme `" + UriScheme +
                                       "' for workspace files");
  }
  return getAbsolutePath(UriAuthority, UriBody);
}

toolchain::Expected<URIForFile> URIForFile::fromURI(StringRef Uri) {
  toolchain::Expected<std::string> FilePath = parseFilePathFromURI(Uri);
  if (!FilePath)
    return FilePath.takeError();
  return URIForFile(std::move(*FilePath), Uri.str());
}

toolchain::Expected<URIForFile> URIForFile::fromFile(StringRef AbsoluteFilepath,
                                                StringRef Scheme) {
  toolchain::Expected<std::string> Uri =
      uriFromAbsolutePath(AbsoluteFilepath, Scheme);
  if (!Uri)
    return Uri.takeError();
  return fromURI(*Uri);
}

StringRef URIForFile::scheme() const { return uri().split(':').first; }

void URIForFile::registerSupportedScheme(StringRef Scheme) {
  getSupportedSchemes().insert(Scheme);
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, URIForFile &Result,
                         toolchain::json::Path Path) {
  if (std::optional<StringRef> Str = Value.getAsString()) {
    toolchain::Expected<URIForFile> ExpectedUri = URIForFile::fromURI(*Str);
    if (!ExpectedUri) {
      Path.report("unresolvable URI");
      consumeError(ExpectedUri.takeError());
      return false;
    }
    Result = std::move(*ExpectedUri);
    return true;
  }
  return false;
}

toolchain::json::Value toolchain::lsp::toJSON(const URIForFile &Value) {
  return Value.uri();
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os, const URIForFile &Value) {
  return Os << Value.uri();
}

//===----------------------------------------------------------------------===//
// ClientCapabilities
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         ClientCapabilities &Result, toolchain::json::Path Path) {
  const toolchain::json::Object *O = Value.getAsObject();
  if (!O) {
    Path.report("expected object");
    return false;
  }
  if (const toolchain::json::Object *TextDocument = O->getObject("textDocument")) {
    if (const toolchain::json::Object *DocumentSymbol =
            TextDocument->getObject("documentSymbol")) {
      if (std::optional<bool> HierarchicalSupport =
              DocumentSymbol->getBoolean("hierarchicalDocumentSymbolSupport"))
        Result.hierarchicalDocumentSymbol = *HierarchicalSupport;
    }
    if (auto *CodeAction = TextDocument->getObject("codeAction")) {
      if (CodeAction->getObject("codeActionLiteralSupport"))
        Result.codeActionStructure = true;
    }
  }
  if (auto *Window = O->getObject("window")) {
    if (std::optional<bool> WorkDoneProgressSupport =
            Window->getBoolean("workDoneProgress"))
      Result.workDoneProgress = *WorkDoneProgressSupport;
  }
  return true;
}

//===----------------------------------------------------------------------===//
// ClientInfo
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, ClientInfo &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  if (!O || !O.map("name", Result.name))
    return false;

  // Don't fail if we can't parse version.
  O.map("version", Result.version);
  return true;
}

//===----------------------------------------------------------------------===//
// InitializeParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, TraceLevel &Result,
                         toolchain::json::Path Path) {
  if (std::optional<StringRef> Str = Value.getAsString()) {
    if (*Str == "off") {
      Result = TraceLevel::Off;
      return true;
    }
    if (*Str == "messages") {
      Result = TraceLevel::Messages;
      return true;
    }
    if (*Str == "verbose") {
      Result = TraceLevel::Verbose;
      return true;
    }
  }
  return false;
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         InitializeParams &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  if (!O)
    return false;
  // We deliberately don't fail if we can't parse individual fields.
  O.map("capabilities", Result.capabilities);
  O.map("trace", Result.trace);
  mapOptOrNull(Value, "clientInfo", Result.clientInfo, Path);

  return true;
}

//===----------------------------------------------------------------------===//
// TextDocumentItem
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         TextDocumentItem &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("uri", Result.uri) &&
         O.map("languageId", Result.languageId) && O.map("text", Result.text) &&
         O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// TextDocumentIdentifier
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const TextDocumentIdentifier &Value) {
  return toolchain::json::Object{{"uri", Value.uri}};
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         TextDocumentIdentifier &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("uri", Result.uri);
}

//===----------------------------------------------------------------------===//
// VersionedTextDocumentIdentifier
//===----------------------------------------------------------------------===//

toolchain::json::Value
toolchain::lsp::toJSON(const VersionedTextDocumentIdentifier &Value) {
  return toolchain::json::Object{
      {"uri", Value.uri},
      {"version", Value.version},
  };
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         VersionedTextDocumentIdentifier &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("uri", Result.uri) && O.map("version", Result.version);
}

//===----------------------------------------------------------------------===//
// Position
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, Position &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("line", Result.line) &&
         O.map("character", Result.character);
}

toolchain::json::Value toolchain::lsp::toJSON(const Position &Value) {
  return toolchain::json::Object{
      {"line", Value.line},
      {"character", Value.character},
  };
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os, const Position &Value) {
  return Os << Value.line << ':' << Value.character;
}

//===----------------------------------------------------------------------===//
// Range
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, Range &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("start", Result.start) && O.map("end", Result.end);
}

toolchain::json::Value toolchain::lsp::toJSON(const Range &Value) {
  return toolchain::json::Object{
      {"start", Value.start},
      {"end", Value.end},
  };
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os, const Range &Value) {
  return Os << Value.start << '-' << Value.end;
}

//===----------------------------------------------------------------------===//
// Location
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, Location &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("uri", Result.uri) && O.map("range", Result.range);
}

toolchain::json::Value toolchain::lsp::toJSON(const Location &Value) {
  return toolchain::json::Object{
      {"uri", Value.uri},
      {"range", Value.range},
  };
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os, const Location &Value) {
  return Os << Value.range << '@' << Value.uri;
}

//===----------------------------------------------------------------------===//
// TextDocumentPositionParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         TextDocumentPositionParams &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument) &&
         O.map("position", Result.position);
}

//===----------------------------------------------------------------------===//
// ReferenceParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         ReferenceContext &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.mapOptional("includeDeclaration", Result.includeDeclaration);
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         ReferenceParams &Result, toolchain::json::Path Path) {
  TextDocumentPositionParams &Base = Result;
  toolchain::json::ObjectMapper O(Value, Path);
  return fromJSON(Value, Base, Path) && O &&
         O.mapOptional("context", Result.context);
}

//===----------------------------------------------------------------------===//
// DidOpenTextDocumentParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         DidOpenTextDocumentParams &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidCloseTextDocumentParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         DidCloseTextDocumentParams &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DidChangeTextDocumentParams
//===----------------------------------------------------------------------===//

LogicalResult
TextDocumentContentChangeEvent::applyTo(std::string &Contents) const {
  // If there is no range, the full document changed.
  if (!range) {
    Contents = text;
    return success();
  }

  // Try to map the replacement range to the content.
  toolchain::SourceMgr TmpScrMgr;
  TmpScrMgr.AddNewSourceBuffer(toolchain::MemoryBuffer::getMemBuffer(Contents),
                               SMLoc());
  SMRange RangeLoc = range->getAsSMRange(TmpScrMgr);
  if (!RangeLoc.isValid())
    return failure();

  Contents.replace(RangeLoc.Start.getPointer() - Contents.data(),
                   RangeLoc.End.getPointer() - RangeLoc.Start.getPointer(),
                   text);
  return success();
}

LogicalResult TextDocumentContentChangeEvent::applyTo(
    ArrayRef<TextDocumentContentChangeEvent> Changes, std::string &Contents) {
  for (const auto &Change : Changes)
    if (failed(Change.applyTo(Contents)))
      return failure();
  return success();
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         TextDocumentContentChangeEvent &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("range", Result.range) &&
         O.map("rangeLength", Result.rangeLength) && O.map("text", Result.text);
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         DidChangeTextDocumentParams &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument) &&
         O.map("contentChanges", Result.contentChanges);
}

//===----------------------------------------------------------------------===//
// MarkupContent
//===----------------------------------------------------------------------===//

static toolchain::StringRef toTextKind(MarkupKind Kind) {
  switch (Kind) {
  case MarkupKind::PlainText:
    return "plaintext";
  case MarkupKind::Markdown:
    return "markdown";
  }
  llvm_unreachable("Invalid MarkupKind");
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os, MarkupKind Kind) {
  return Os << toTextKind(Kind);
}

toolchain::json::Value toolchain::lsp::toJSON(const MarkupContent &Mc) {
  if (Mc.value.empty())
    return nullptr;

  return toolchain::json::Object{
      {"kind", toTextKind(Mc.kind)},
      {"value", Mc.value},
  };
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const Hover &Hover) {
  toolchain::json::Object Result{{"contents", toJSON(Hover.contents)}};
  if (Hover.range)
    Result["range"] = toJSON(*Hover.range);
  return std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbol
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const DocumentSymbol &Symbol) {
  toolchain::json::Object Result{{"name", Symbol.name},
                            {"kind", static_cast<int>(Symbol.kind)},
                            {"range", Symbol.range},
                            {"selectionRange", Symbol.selectionRange}};

  if (!Symbol.detail.empty())
    Result["detail"] = Symbol.detail;
  if (!Symbol.children.empty())
    Result["children"] = Symbol.children;
  return std::move(Result);
}

//===----------------------------------------------------------------------===//
// DocumentSymbolParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         DocumentSymbolParams &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DiagnosticRelatedInformation
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         DiagnosticRelatedInformation &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("location", Result.location) &&
         O.map("message", Result.message);
}

toolchain::json::Value toolchain::lsp::toJSON(const DiagnosticRelatedInformation &Info) {
  return toolchain::json::Object{
      {"location", Info.location},
      {"message", Info.message},
  };
}

//===----------------------------------------------------------------------===//
// Diagnostic
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(DiagnosticTag Tag) {
  return static_cast<int>(Tag);
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, DiagnosticTag &Result,
                         toolchain::json::Path Path) {
  if (std::optional<int64_t> I = Value.getAsInteger()) {
    Result = (DiagnosticTag)*I;
    return true;
  }

  return false;
}

toolchain::json::Value toolchain::lsp::toJSON(const Diagnostic &Diag) {
  toolchain::json::Object Result{
      {"range", Diag.range},
      {"severity", (int)Diag.severity},
      {"message", Diag.message},
  };
  if (Diag.category)
    Result["category"] = *Diag.category;
  if (!Diag.source.empty())
    Result["source"] = Diag.source;
  if (Diag.relatedInformation)
    Result["relatedInformation"] = *Diag.relatedInformation;
  if (!Diag.tags.empty())
    Result["tags"] = Diag.tags;
  return std::move(Result);
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, Diagnostic &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  if (!O)
    return false;
  int Severity = 0;
  if (!mapOptOrNull(Value, "severity", Severity, Path))
    return false;
  Result.severity = (DiagnosticSeverity)Severity;

  return O.map("range", Result.range) && O.map("message", Result.message) &&
         mapOptOrNull(Value, "category", Result.category, Path) &&
         mapOptOrNull(Value, "source", Result.source, Path) &&
         mapOptOrNull(Value, "relatedInformation", Result.relatedInformation,
                      Path) &&
         mapOptOrNull(Value, "tags", Result.tags, Path);
}

//===----------------------------------------------------------------------===//
// PublishDiagnosticsParams
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const PublishDiagnosticsParams &Params) {
  return toolchain::json::Object{
      {"uri", Params.uri},
      {"diagnostics", Params.diagnostics},
      {"version", Params.version},
  };
}

//===----------------------------------------------------------------------===//
// TextEdit
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, TextEdit &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("range", Result.range) && O.map("newText", Result.newText);
}

toolchain::json::Value toolchain::lsp::toJSON(const TextEdit &Value) {
  return toolchain::json::Object{
      {"range", Value.range},
      {"newText", Value.newText},
  };
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os, const TextEdit &Value) {
  Os << Value.range << " => \"";
  toolchain::printEscapedString(Value.newText, Os);
  return Os << '"';
}

//===----------------------------------------------------------------------===//
// CompletionItemKind
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         CompletionItemKind &Result, toolchain::json::Path Path) {
  if (std::optional<int64_t> IntValue = Value.getAsInteger()) {
    if (*IntValue < static_cast<int>(CompletionItemKind::Text) ||
        *IntValue > static_cast<int>(CompletionItemKind::TypeParameter))
      return false;
    Result = static_cast<CompletionItemKind>(*IntValue);
    return true;
  }
  return false;
}

CompletionItemKind toolchain::lsp::adjustKindToCapability(
    CompletionItemKind Kind,
    CompletionItemKindBitset &SupportedCompletionItemKinds) {
  size_t KindVal = static_cast<size_t>(Kind);
  if (KindVal >= kCompletionItemKindMin &&
      KindVal <= SupportedCompletionItemKinds.size() &&
      SupportedCompletionItemKinds[KindVal])
    return Kind;

  // Provide some fall backs for common kinds that are close enough.
  switch (Kind) {
  case CompletionItemKind::Folder:
    return CompletionItemKind::File;
  case CompletionItemKind::EnumMember:
    return CompletionItemKind::Enum;
  case CompletionItemKind::Struct:
    return CompletionItemKind::Class;
  default:
    return CompletionItemKind::Text;
  }
}

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         CompletionItemKindBitset &Result,
                         toolchain::json::Path Path) {
  if (const toolchain::json::Array *ArrayValue = Value.getAsArray()) {
    for (size_t I = 0, E = ArrayValue->size(); I < E; ++I) {
      CompletionItemKind KindOut;
      if (fromJSON((*ArrayValue)[I], KindOut, Path.index(I)))
        Result.set(size_t(KindOut));
    }
    return true;
  }
  return false;
}

//===----------------------------------------------------------------------===//
// CompletionItem
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const CompletionItem &Value) {
  assert(!Value.label.empty() && "completion item label is required");
  toolchain::json::Object Result{{"label", Value.label}};
  if (Value.kind != CompletionItemKind::Missing)
    Result["kind"] = static_cast<int>(Value.kind);
  if (!Value.detail.empty())
    Result["detail"] = Value.detail;
  if (Value.documentation)
    Result["documentation"] = Value.documentation;
  if (!Value.sortText.empty())
    Result["sortText"] = Value.sortText;
  if (!Value.filterText.empty())
    Result["filterText"] = Value.filterText;
  if (!Value.insertText.empty())
    Result["insertText"] = Value.insertText;
  if (Value.insertTextFormat != InsertTextFormat::Missing)
    Result["insertTextFormat"] = static_cast<int>(Value.insertTextFormat);
  if (Value.textEdit)
    Result["textEdit"] = *Value.textEdit;
  if (!Value.additionalTextEdits.empty()) {
    Result["additionalTextEdits"] =
        toolchain::json::Array(Value.additionalTextEdits);
  }
  if (Value.deprecated)
    Result["deprecated"] = Value.deprecated;
  return std::move(Result);
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os,
                                   const CompletionItem &Value) {
  return Os << Value.label << " - " << toJSON(Value);
}

bool toolchain::lsp::operator<(const CompletionItem &Lhs,
                          const CompletionItem &Rhs) {
  return (Lhs.sortText.empty() ? Lhs.label : Lhs.sortText) <
         (Rhs.sortText.empty() ? Rhs.label : Rhs.sortText);
}

//===----------------------------------------------------------------------===//
// CompletionList
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const CompletionList &Value) {
  return toolchain::json::Object{
      {"isIncomplete", Value.isIncomplete},
      {"items", toolchain::json::Array(Value.items)},
  };
}

//===----------------------------------------------------------------------===//
// CompletionContext
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         CompletionContext &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  int TriggerKind;
  if (!O || !O.map("triggerKind", TriggerKind) ||
      !mapOptOrNull(Value, "triggerCharacter", Result.triggerCharacter, Path))
    return false;
  Result.triggerKind = static_cast<CompletionTriggerKind>(TriggerKind);
  return true;
}

//===----------------------------------------------------------------------===//
// CompletionParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         CompletionParams &Result, toolchain::json::Path Path) {
  if (!fromJSON(Value, static_cast<TextDocumentPositionParams &>(Result), Path))
    return false;
  if (const toolchain::json::Value *Context = Value.getAsObject()->get("context"))
    return fromJSON(*Context, Result.context, Path.field("context"));
  return true;
}

//===----------------------------------------------------------------------===//
// ParameterInformation
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const ParameterInformation &Value) {
  assert((Value.labelOffsets || !Value.labelString.empty()) &&
         "parameter information label is required");
  toolchain::json::Object Result;
  if (Value.labelOffsets)
    Result["label"] = toolchain::json::Array(
        {Value.labelOffsets->first, Value.labelOffsets->second});
  else
    Result["label"] = Value.labelString;
  if (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  return std::move(Result);
}

//===----------------------------------------------------------------------===//
// SignatureInformation
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const SignatureInformation &Value) {
  assert(!Value.label.empty() && "signature information label is required");
  toolchain::json::Object Result{
      {"label", Value.label},
      {"parameters", toolchain::json::Array(Value.parameters)},
  };
  if (!Value.documentation.empty())
    Result["documentation"] = Value.documentation;
  return std::move(Result);
}

raw_ostream &toolchain::lsp::operator<<(raw_ostream &Os,
                                   const SignatureInformation &Value) {
  return Os << Value.label << " - " << toJSON(Value);
}

//===----------------------------------------------------------------------===//
// SignatureHelp
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const SignatureHelp &Value) {
  assert(Value.activeSignature >= 0 &&
         "Unexpected negative value for number of active signatures.");
  assert(Value.activeParameter >= 0 &&
         "Unexpected negative value for active parameter index");
  return toolchain::json::Object{
      {"activeSignature", Value.activeSignature},
      {"activeParameter", Value.activeParameter},
      {"signatures", toolchain::json::Array(Value.signatures)},
  };
}

//===----------------------------------------------------------------------===//
// DocumentLinkParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         DocumentLinkParams &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument);
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const DocumentLink &Value) {
  return toolchain::json::Object{
      {"range", Value.range},
      {"target", Value.target},
  };
}

//===----------------------------------------------------------------------===//
// InlayHintsParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         InlayHintsParams &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range);
}

//===----------------------------------------------------------------------===//
// InlayHint
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const InlayHint &Value) {
  return toolchain::json::Object{{"position", Value.position},
                            {"kind", (int)Value.kind},
                            {"label", Value.label},
                            {"paddingLeft", Value.paddingLeft},
                            {"paddingRight", Value.paddingRight}};
}
bool toolchain::lsp::operator==(const InlayHint &Lhs, const InlayHint &Rhs) {
  return std::tie(Lhs.position, Lhs.kind, Lhs.label) ==
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}
bool toolchain::lsp::operator<(const InlayHint &Lhs, const InlayHint &Rhs) {
  return std::tie(Lhs.position, Lhs.kind, Lhs.label) <
         std::tie(Rhs.position, Rhs.kind, Rhs.label);
}

toolchain::raw_ostream &toolchain::lsp::operator<<(toolchain::raw_ostream &Os,
                                         InlayHintKind Value) {
  switch (Value) {
  case InlayHintKind::Parameter:
    return Os << "parameter";
  case InlayHintKind::Type:
    return Os << "type";
  }
  llvm_unreachable("Unknown InlayHintKind");
}

//===----------------------------------------------------------------------===//
// CodeActionContext
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         CodeActionContext &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  if (!O || !O.map("diagnostics", Result.diagnostics))
    return false;
  O.map("only", Result.only);
  return true;
}

//===----------------------------------------------------------------------===//
// CodeActionParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value,
                         CodeActionParams &Result, toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("textDocument", Result.textDocument) &&
         O.map("range", Result.range) && O.map("context", Result.context);
}

//===----------------------------------------------------------------------===//
// WorkspaceEdit
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &Value, WorkspaceEdit &Result,
                         toolchain::json::Path Path) {
  toolchain::json::ObjectMapper O(Value, Path);
  return O && O.map("changes", Result.changes);
}

toolchain::json::Value toolchain::lsp::toJSON(const WorkspaceEdit &Value) {
  toolchain::json::Object FileChanges;
  for (auto &Change : Value.changes)
    FileChanges[Change.first] = toolchain::json::Array(Change.second);
  return toolchain::json::Object{{"changes", std::move(FileChanges)}};
}

//===----------------------------------------------------------------------===//
// CodeAction
//===----------------------------------------------------------------------===//

const toolchain::StringLiteral CodeAction::kQuickFix = "quickfix";
const toolchain::StringLiteral CodeAction::kRefactor = "refactor";
const toolchain::StringLiteral CodeAction::kInfo = "info";

toolchain::json::Value toolchain::lsp::toJSON(const CodeAction &Value) {
  toolchain::json::Object CodeAction{{"title", Value.title}};
  if (Value.kind)
    CodeAction["kind"] = *Value.kind;
  if (Value.diagnostics)
    CodeAction["diagnostics"] = toolchain::json::Array(*Value.diagnostics);
  if (Value.isPreferred)
    CodeAction["isPreferred"] = true;
  if (Value.edit)
    CodeAction["edit"] = *Value.edit;
  return std::move(CodeAction);
}

//===----------------------------------------------------------------------===//
// ShowMessageParams
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const ShowMessageParams &Params) {
  auto Out = toolchain::json::Object{
      {"type", static_cast<int>(Params.type)},
      {"message", Params.message},
  };
  if (Params.actions)
    Out["actions"] = *Params.actions;
  return Out;
}

toolchain::json::Value toolchain::lsp::toJSON(const MessageActionItem &Params) {
  return toolchain::json::Object{{"title", Params.title}};
}
