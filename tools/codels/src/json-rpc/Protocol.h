/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LSPSERVER_PROTOCOL_H
#define LSPSERVER_PROTOCOL_H

#include <cstdint>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "../languageserver/index/Symbol.h"
#include "CompletionType.h"
#include "ProtocolContent.h"
#include "WorkSpaceSymbolType.h"
#include "Codira/Basic/Position.h"

/**
 * According to the language service protocol to create structure
 * see https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#baseProtocol
 */
namespace ark {


class MessageErrorDetail {
public:
    std::string message = "";
    ErrorCode code;

    MessageErrorDetail() : message(""), code(ErrorCode::PARSE_ERROR) {};

    MessageErrorDetail(std::string message, ErrorCode code) : message(std::move(message)), code(code) {}
    ~MessageErrorDetail() {}
};

struct TextDocumentIdentifier {
    // The text document's URI.
    URIForFile uri;
    ~TextDocumentIdentifier() = default;
};

struct TextDocumentPositionParams {
    TextDocumentIdentifier textDocument;

    Codira::Position position;

    ~TextDocumentPositionParams() = default;
};

struct CrossLanguageJumpParams {
    std::string packageName;

    std::string name;

    std::string outerName;

    bool isCombined = false;

    ~CrossLanguageJumpParams() = default;
};

struct OverrideMethodsParams: public TextDocumentPositionParams {
    // Is extend type
    bool isExtend = false;
};

struct ExportsNameParams {
    TextDocumentIdentifier textDocument;

    Codira::Position position;

    std::string packageName;

    ~ExportsNameParams() = default;
};

// TypeHierarchy response
struct TypeHierarchyItem {
public:
    std::string name = "";

    SymbolKind kind = SymbolKind::FILE;

    URIForFile uri {};

    Range range {};

    Range selectionRange {};

    bool isKernel{false};
    bool isChildOrSuper{true};
    lsp::SymbolID symbolId = lsp::INVALID_SYMBOL_ID;

    bool operator==(const TypeHierarchyItem &rhs) const
    {
        return name == rhs.name && uri.file == rhs.uri.file && kind == rhs.kind &&
               range == rhs.range && selectionRange == rhs.selectionRange;
    }

    bool operator!=(const TypeHierarchyItem &rhs) const
    {
        return !(rhs == *this);
    }
};

struct CallHierarchyItem : public TypeHierarchyItem {
public:
    std::string detail;
};


struct CallHierarchyOutgoingCall {
    CallHierarchyItem to;
    std::vector<Range> fromRanges;
};

bool ToJSON(const CallHierarchyOutgoingCall &iter, nlohmann::json &reply);

struct CallHierarchyIncomingCall {
    CallHierarchyItem from;
    std::vector<Range> fromRanges;
};

bool ToJSON(const CallHierarchyIncomingCall &iter, nlohmann::json &reply);

bool ToJSON(const TypeHierarchyItem &iter, nlohmann::json &reply);

bool FromJSON(const nlohmann::json &params, TypeHierarchyItem &reply);

bool ToJSON(const CallHierarchyItem &iter, nlohmann::json &reply);

bool FromJSON(const nlohmann::json &params, CallHierarchyItem &reply);

bool FromJSON(const nlohmann::json &params, TextDocumentPositionParams &reply);

bool FromJSON(const nlohmann::json &params, CrossLanguageJumpParams &reply);

bool FromJSON(const nlohmann::json &params, OverrideMethodsParams &reply);

bool FromJSON(const nlohmann::json &params, ExportsNameParams &reply);

bool FromJSON(const nlohmann::json &params, CompletionContext &reply);

struct CompletionParams : public TextDocumentPositionParams {
    CompletionContext context;
};

bool FromJSON(const nlohmann::json &params, CompletionParams &reply);

bool ToJSON(const CompletionItem &iter, nlohmann::json &reply);

struct Signatures {
    std::string label = "";
    std::vector<std::string> parameters = {};

    bool operator==(const Signatures &other) const
    {
        if (this->parameters != other.parameters) {
            return false;
        }

        if (this->label != other.label) {
            return false;
        }
        return true;
    }

    bool operator<(const Signatures &other) const
    {
        if (this->parameters.size() > other.parameters.size()) {
            return false;
        }
        if (this->parameters.size() < other.parameters.size()) {
            return true;
        }
        if (this->label > other.label) {
            return false;
        }
        return true;
    }

    Signatures(): label(""), parameters() {};
};

struct SignatureHelp {
    unsigned int activeSignature = 0;

    unsigned int activeParameter = 0;

    std::vector<Signatures> signatures = {};

    bool operator==(const SignatureHelp &other) const
    {
        if (this->signatures.size() != other.signatures.size()) {
            return false;
        }
        bool isEqual = std::tie(activeSignature, activeParameter) ==
                       std::tie(other.activeSignature, other.activeParameter);
        if (!isEqual) {
            return false;
        }
        for (size_t i = 0; i < other.signatures.size(); i++) {
            if (i >= this->signatures.size() || !(this->signatures[i] == other.signatures[i])) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const SignatureHelp &other) const
    {
        return !(*this == other);
    }
};

struct SignatureHelpContext {
    bool isRetrigger = false;
    SignatureHelp activeSignatureHelp = {};
    std::string triggerCharacter = "";

    SignatureHelpContext(): isRetrigger(false), activeSignatureHelp(), triggerCharacter("") {};
};

struct SignatureHelpParams : public TextDocumentPositionParams {
    SignatureHelpContext context;
};

bool FromJSON(const nlohmann::json &params, SignatureHelpContext &reply);

struct CompletionList {
    std::vector<CompletionItem> items{};
};

struct FileWatchedEvent {
    TextDocumentIdentifier textDocument;
    FileChangeType type;
};

struct DidChangeWatchedFilesParam {
    std::vector<FileWatchedEvent> changes;
};

bool FromJSON(const nlohmann::json &params, DidChangeWatchedFilesParam &reply);

struct RenameParams {
    TextDocumentIdentifier textDocument;

    Codira::Position position;

    std::string newName;
};

bool FromJSON(const nlohmann::json &params, RenameParams &reply);

bool FromJSON(const nlohmann::json &params,  SignatureHelpParams &reply);

struct TextDocumentParams {
    TextDocumentIdentifier textDocument;
};

bool FromJSON(const nlohmann::json &params, TextDocumentParams &tdReply);

struct BreakpointLocation {
    std::string uri;
    Range range;

    bool operator<(const BreakpointLocation &rhs) const
    {
        return std::tie(this->uri, this->range) < std::tie(rhs.uri, rhs.range);
    }
};

bool ToJSON(const BreakpointLocation &params, nlohmann::json &reply);

struct ExecutableRange {
    std::string uri;
    std::string projectName;
    std::string packageName;
    std::string className;
    std::string functionName;
    Range range;
    // used for refactor
    std::string tweakId;
    // used for refactor
    std::map<std::string, std::string> extraOptions;

    bool operator<(const ExecutableRange &rhs) const
    {
        return std::tie(this->uri, this->projectName, this->packageName, this->className,
                        this->functionName, this->range) <
        std::tie(rhs.uri, rhs.projectName, rhs.packageName, rhs.className, rhs.functionName, rhs.range);
    }
};

struct Command {
    std::string title;
    std::string command;
    std::set<ExecutableRange> arguments;
    const static std::string APPLY_EDIT_COMMAND;
};

bool ToJSON(const Command &params, nlohmann::json &reply);

struct CodeLens {
    Range range;
    Command command;

    bool operator<(const CodeLens &rhs) const
    {
        return std::tie(this->range, this->command.title, this->command.command) <
        std::tie(rhs.range, rhs.command.title, rhs.command.command);
    }
};

bool ToJSON(const ExecutableRange &params, nlohmann::json &reply);

bool ToJSON(const CodeLens &params, nlohmann::json &reply);

struct DocumentHighlight {
    Range range;
    DocumentHighlightKind kind = DocumentHighlightKind::TEXT;

    bool operator<(const DocumentHighlight &rhs) const
    {
        return std::tie(this->range, this->kind) < std::tie(rhs.range, rhs.kind);
    }
};

struct Hover {
    Range range;
    std::vector<std::string> markedString = {};
    Hover(): range(), markedString() {};
};

struct TextDocumentItem {
    URIForFile uri;

    std::string languageId;

    int64_t version;

    std::string text;
};

struct DidOpenTextDocumentParams {
    TextDocumentItem textDocument;
};

bool FromJSON(const nlohmann::json &params, DidOpenTextDocumentParams &reply);

struct VersionedTextDocumentIdentifier : public TextDocumentIdentifier {
    std::int64_t version = -1;
};

struct TextDocumentEdit {
    VersionedTextDocumentIdentifier textDocument;

    std::vector<TextEdit> textEdits{};
};

struct TextDocumentContentChangeEvent {
    std::optional<Range> range;

    std::optional<int> rangeLength;

    std::string text;
};

struct DidChangeTextDocumentParams {
    VersionedTextDocumentIdentifier textDocument;

    std::vector<TextDocumentContentChangeEvent> contentChanges{};
};

bool FromJSON(const nlohmann::json &params, DidChangeTextDocumentParams &reply);

struct DidCloseTextDocumentParams {
    TextDocumentIdentifier textDocument;
};

bool FromJSON(const nlohmann::json &params, DidCloseTextDocumentParams &reply);

struct DidSaveTextDocumentParams {
    TextDocumentIdentifier textDocument;
};

struct TrackCompletionParams {
    std::string label;
};

bool FromJSON(const nlohmann::json &params, TrackCompletionParams &reply);

struct SemanticTokensParams {
    TextDocumentIdentifier textDocument;
};

struct SemanticTokens {
    std::vector<int> data;
};

bool FromJSON(const nlohmann::json &params, SemanticTokensParams &reply);

struct TextDocumentClientCapabilities {
    bool documentHighlightClientCapabilities = false;

    bool hoverClientCapabilities = false;

    bool diagnosticVersionSupport = false;

    bool documentLinkClientCapabilities = false;

    bool typeHierarchyCapabilities = false;
};

struct ClientCapabilities {
    TextDocumentClientCapabilities textDocumentClientCapabilities;
};

struct InitializeParams {
    std::string rootPath = "";

    URIForFile rootUri;

    nlohmann::json initializationOptions;

    ClientCapabilities capabilities;

    InitializeParams(): rootPath("") {};
};

bool FromJSON(const nlohmann::json &params, InitializeParams &reply);

bool FetchTextDocument(const nlohmann::json &textDocument, InitializeParams &reply);

bool FromJSON(const nlohmann::json &params, TextDocumentIdentifier &reply);

struct SemanticHighlightToken {
    HighlightKind kind = HighlightKind::MISSING_H;
    Range range;
};

struct DiagnosticRelatedInformation {
    Location location;

    std::string message;
};

struct CodeAction;

struct DiagFix {
    bool addImport{false};
    bool removeImport{false};
};

struct DiagnosticToken {
    Range range;

    int severity = 0;

    int code;

    std::string source = "";

    std::string message = "";

    std::vector<int> tags{};

    std::optional<std::vector<DiagnosticRelatedInformation>> relatedInformation{};

    std::optional<int> category{};

    std::optional<std::vector<CodeAction>> codeActions{};

    std::optional<DiagFix> diaFix = DiagFix{false, false};

    DiagnosticToken()
        : range(), severity(0), code(0), source(""), message(""), relatedInformation() {};
};

// In a time-consuming complete scenario, the tip is displayed, indicating that the user needs to wait.
struct CompletionTip {
    // The URI for which completion tip is display.
    URIForFile uri;
    // the position of tip
    Codira::Position position = {0, -1, -1};
    // tip message
    std::string tip;
};

bool ToJSON(const DiagnosticToken &iter, nlohmann::json &reply);

bool FromJSON(const nlohmann::json &params, DiagnosticToken &result);

bool ToJSON(const DiagnosticRelatedInformation &info, nlohmann::json &reply);

bool FromJSON(const nlohmann::json &param, DiagnosticRelatedInformation &info);

struct DiagnosticCompare {
    bool operator()(const DiagnosticToken &lhs, const DiagnosticToken &rhs) const
    {
        return std::tie(lhs.range, lhs.message) < std::tie(rhs.range, rhs.message);
    }
};
struct PublishDiagnosticsParams {
    URIForFile uri;

    std::vector<DiagnosticToken> diagnostics;

    std::optional<int64_t> version;
};
bool ToJSON(const PublishDiagnosticsParams &params, nlohmann::json &reply);

struct WorkspaceEdit {
    std::map<std::string, std::vector<TextEdit>> changes;
};
bool ToJSON(const WorkspaceEdit &params, nlohmann::json &reply);

struct CodeAction {
    std::string title = "";

    std::string kind = "";

    const static std::string QUICKFIX_ADD_IMPORT;

    const static std::string QUICKFIX_REMOVE_IMPORT;

    const static std::string REFACTOR_KIND;

    const static std::string INFO_KIND;

    std::optional<std::vector<DiagnosticToken>> diagnostics;

    bool isPreferred = false;

    std::optional<WorkspaceEdit> edit{};

    std::optional<Command> command;
};
bool ToJSON(const CodeAction &params, nlohmann::json &reply);

bool ToJSON(const TextDocumentEdit &params, nlohmann::json &reply);

struct DocumentLinkParams {
    TextDocumentIdentifier textDocument;
};

bool FromJSON(const nlohmann::json &params, DocumentLinkParams &reply);

struct DocumentSymbolParams {
    TextDocumentIdentifier textDocument;
};

struct DocumentSymbol {
    std::string name;
    std::string detail;
    SymbolKind kind;
    Range range;
    Range selectionRange;
    std::vector<DocumentSymbol> children;

    bool operator<(const DocumentSymbol &other) const
    {
        return std::tie(selectionRange, range, kind, name) <
               std::tie(other.selectionRange, other.range, other.kind, other.name);
    }

    bool operator==(const DocumentSymbol &other) const
    {
        if (this->children.size() != other.children.size()) {
            return false;
        }
        bool isEqual = std::tie(kind, name, detail, selectionRange, range) ==
                       std::tie(other.kind, other.name, other.detail, other.selectionRange, other.range);
        if (!isEqual) {
            return false;
        }
        for (size_t i = 0; i < other.children.size(); i++) {
            if (!(this->children[i] == other.children[i])) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const DocumentSymbol &other) const
    {
        return !(*this == other);
    }
};

bool FromJSON(const nlohmann::json &params, DocumentSymbolParams &dsReply);

bool ToJSON(const DocumentSymbol &item, nlohmann::json &result);

struct CodeActionContext {
    std::vector<DiagnosticToken> diagnostics;

    std::optional<std::vector<std::string>> only;
};

bool FromJSON(const nlohmann::json &params, CodeActionContext &reply);

struct CodeActionParams {
    TextDocumentIdentifier textDocument;

    Range range;

    CodeActionContext context;
};

bool FromJSON(const nlohmann::json &params, CodeActionParams &reply);

struct TweakArgs {
    URIForFile file;

    Range selection;

    std::string tweakID;

    std::map<std::string, std::string> extraOptions;
};

bool FromJSON(const nlohmann::json &params, TweakArgs &reply);

bool ToJSON(const TweakArgs &item, nlohmann::json &reply);

struct ExecuteCommandParams {
    std::string command;

    nlohmann::json arguments;
};

bool FromJSON(const nlohmann::json &params, ExecuteCommandParams &reply);

struct ApplyWorkspaceEditParams {
    WorkspaceEdit edit;
};

bool ToJSON(const ApplyWorkspaceEditParams &item, nlohmann::json &reply);

struct FileRefactorReqParams {
    TextDocumentIdentifier file;

    TextDocumentIdentifier targetPath;

    TextDocumentIdentifier selectedElement;
};

bool FromJSON(const nlohmann::json &params, FileRefactorReqParams &reply);

enum class FileRefactorChangeType {
    // add import.
    ADD = 1,
    // change import.
    CHANGED = 2,
    // delete import.
    DELETED = 3,
};

struct FileRefactorChange {
    FileRefactorChangeType type;

    Range range;

    std::string content;

    bool operator<(const FileRefactorChange &rhs) const
    {
        if (type != rhs.type) {
            return static_cast<int>(type) < static_cast<int>(rhs.type);
        }

        if (range != rhs.range) {
            return range < rhs.range;
        }
        return content < rhs.content;
    }

    bool operator==(const FileRefactorChange &rhs) const
    {
        return type == rhs.type && range == rhs.range && content == rhs.content;
    }
};

struct FileRefactorRespParams {
    std::map<std::string, std::set<FileRefactorChange>> changes;
};

bool ToJSON(const FileRefactorRespParams &item, nlohmann::json &reply);

class MessageHeaderEndOfLine {
public:
    static std::string& GetEol()
    {
        return eol;
    }

    static void SetEol(const std::string& str)
    {
        eol = str;
    }

    static bool GetIsDeveco()
    {
        return isDeveco;
    }

    static void SetIsDeveco(bool flag)
    {
        isDeveco = flag;
    }
private:
    static std::string eol;
    static bool isDeveco;
};
} // namespace ark
#endif // LSPSERVER_PROTOCOL_H
