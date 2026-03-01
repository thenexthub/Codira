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

#include "ArkServer.h"
#include <cangjie/Utils/ConstantsUtils.h>
#include <cangjie/Utils/FileUtil.h>
#include <string>
#include <utility>

#include "capabilities/definition/CrossLanguangeDefinition.h"
#include "capabilities/documentSymbol/DocumentSymbolImpl.h"
#include "capabilities/hover/HoverImpl.h"
#include "capabilities/semanticHighlight/SemanticTokensAdaptor.h"
#include "capabilities/signatureHelp/SignatureHelpImpl.h"
#include "capabilities/typeHierarchy/TypeHierarchyImpl.h"
#include "capabilities/workspaceSymbol/FindSymbols.h"
#include "common/Constants.h"

#undef INTERFACE

namespace {
bool Contains(const Codira::StringPart &str, Codira::Position pos)
{
    return str.begin.column < pos.column < str.begin.column + str.value.size();
}
};

namespace ark {
using namespace Codira;
bool CompareCallHierarchyOutgoingCall(const CallHierarchyOutgoingCall &letf, const CallHierarchyOutgoingCall &right)
{
    return letf.fromRanges[0].start < right.fromRanges[0].start;
}

ArkServer::ArkServer(Callbacks *callbacks) : callback(callbacks)
{
    arkScheduler = std::make_unique<ark::ArkScheduler>(callback);
    arkSchedulerOfComplete = std::make_unique<ark::ArkScheduler>(callback);
    arkSchedulerOfSignature = std::make_unique<ark::ArkScheduler>(callback);
}

void ArkServer::FindSemanticTokensHighlight(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply)](const InputsAndAST &inputAST) {
        SemanticTokens result {};
        // To avoid queue task in runWithAST crashing
        if (inputAST.ast == nullptr || !CompilerCodiraProject::GetInstance()->FileHasSemaCache(file)) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        SemanticTokensAdaptor::FindSemanticTokens(*(inputAST.ast), result, inputAST.ast->fileID);
        nlohmann::json jsonValue;
        // Wrapper for the packet to sent
        for (auto &iter : result.data) {
            (void)jsonValue["data"].push_back(iter);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("SemanticTokens", file, action);
}


void ArkServer::FindDocumentHighlights(const std::string &file, const TextDocumentPositionParams &params,
                                       const Callback<ValueOrError> &reply) const
{
    auto action = [file, params, reply = std::move(reply), this](const InputsAndAST& inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        std::set<DocumentHighlight> result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }

        DocumentHighlightImpl::FindDocumentHighlights(*(inputAST.ast), result, pos);
        
        if (inputAST.useASTCache) {
            std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
            UpdateModifierDiag(inputAST, diagnostics, file);
            callback->ReadyForDiagnostics(file, inputAST.inputs.version, diagnostics);
        }

        nlohmann::json jsonValue;
        for (auto &iter : result) {
            nlohmann::json temp;
            temp["kind"] = static_cast<int>(iter.kind);
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("Highlights", file, action);
}

void ArkServer::FindTypeHierarchys(const std::string &file, const TextDocumentPositionParams &params,
                                   const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyItem result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyImpl::FindTypeHierarchyImpl(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        nlohmann::json temp;
        if (result.kind != SymbolKind::FILE && ToJSON(result, temp)) {
            (void) jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("TypeHierarchy", file, action);
}

void ArkServer::FindSuperTypes(const std::string &file, const TypeHierarchyItem &params,
                               const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<TypeHierarchyItem> results;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyImpl::FindSuperTypesImpl(results, params);
        nlohmann::json jsonValue;
        for (const auto &result: results) {
            nlohmann::json temp;
            if (ToJSON(result, temp)) {
                (void) jsonValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("SuperTypes", file, action);
}

void ArkServer::FindSubTypes(const std::string &file, const TypeHierarchyItem &params,
                             const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<TypeHierarchyItem> results;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        TypeHierarchyImpl::FindSubTypesImpl(results, params);
        nlohmann::json jsonValue;
        for (const auto &item: results) {
            nlohmann::json temp;
            if (ToJSON(item, temp)) {
                (void) jsonValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("SubTypes", file, action);
}

void ArkServer::FindCallHierarchys(const std::string &file, const TextDocumentPositionParams &params,
                                   const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        CallHierarchyItem result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        CallHierarchyImpl::FindCallHierarchyImpl(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        nlohmann::json temp;
        if (result.kind != SymbolKind::FILE && ToJSON(result, temp)) {
            (void) jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("CallHierarchy", file, action);
}

void ArkServer::FindOnIncomingCalls(const std::string &file, const CallHierarchyItem &params,
                                    const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<CallHierarchyIncomingCall> results;
        CallHierarchyImpl::FindOnIncomingCallsImpl(results, params);
        nlohmann::json jsValue;
        for (const auto &result: results) {
            nlohmann::json temp;
            if (ToJSON(result, temp)) {
                (void) jsValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsValue);
        reply(value);
    };
    arkScheduler->RunWithAST("OnIncomingCalls", file, action);
}

void ArkServer::FindOnOutgoingCalls(const std::string &file, const CallHierarchyItem &params,
                                    const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply = std::move(reply)](const InputsAndAST &inputAST) {
        std::vector<CallHierarchyOutgoingCall> results;
        CallHierarchyImpl::FindOnOutgoingCallsImpl(results, params);
        std::sort(results.begin(), results.end(), CompareCallHierarchyOutgoingCall);
        nlohmann::json jsnValue;
        for (const auto &item: results) {
            nlohmann::json temp;
            if (ToJSON(item, temp)) {
                (void) jsnValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsnValue);
        reply(value);
    };
    arkScheduler->RunWithAST("OnOutgoingCalls", file, action);
}

void ArkServer::FindReferences(const std::string &file,
                               const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST& inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        ReferencesResult result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindReferencesImpl::FindReferences(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        for (auto &iter : result.References) {
            nlohmann::json temp;
            temp["uri"] = iter.uri.file;
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("References", file, action);
}

void ArkServer::FindFileReferences(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply), this](const InputsAndAST& inputAST) {
        ReferencesResult result;
        std::string unixPath = PathWindowsToLinux(file);
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindReferencesImpl::FindFileReferences(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        for (auto &iter : result.References) {
            nlohmann::json temp;
            if (unixPath.compare(URI::Resolve(iter.uri.file)) == 0) {
                continue;
            }
            temp["uri"] = iter.uri.file;
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("FileReferences", file, action);
}

void GetCurPkgUseAge(Ptr<Decl> decl, const ArkAST &ast, ReferencesResult &result)
{
    if (!decl || !ast.file || !ast.file->curPackage) {
        return;
    }
    auto user = FindDeclUsage(*decl, *ast.file->curPackage);
    for (const auto &U : user) {
        if (U->astKind == ASTKind::MEMBER_ACCESS) {
            continue;
        }
        auto range = GetProperRange(U, ast.tokens);
        Location loc = {URI::URIFromAbsolutePath(U->curFile->filePath).ToString(), range};
        (void)result.References.emplace(loc);
    }
}

void ArkServer::GetExportsName(
        const std::string &file, const ExportsNameParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        int fileId = CompilerCodiraProject::GetInstance()->GetFileID(file);
        if (fileId < 0) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        Codira::Position pos =
                Codira::Position{static_cast<unsigned int>(fileId), params.position.line, params.position.column};
        ReferencesResult result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        ArkAST &ast = *(inputAST.ast);
        Logger &logger = Logger::Instance();
        logger.LogMessage(MessageType::MSG_LOG, "FindReferencesImpl::FindReferences in.");

        // adjust position from IDE to AST
        pos = PosFromIDE2Char(pos);
        PositionIDEToUTF8(ast.tokens, pos, *ast.file);

        if (ast.IsFilterToken(pos)) {
            return;
        }
        // get curFilePath
        std::string curFilePath = ast.file ? ast.file->filePath : "";
        pos.fileID = ast.fileID;
        std::vector<Symbol *> syms;
        std::vector<Ptr<Codira::AST::Decl>> decls;
        Ptr<Decl> oldDecl = ast.GetDeclByPosition(pos, syms, decls, {true, true});
        if (oldDecl == nullptr) {
            return;
        }
        if (syms[0] && IsResourcePos(ast, syms[0]->node, pos)) {
            return;
        }
        DealMemberParam(curFilePath, syms, decls, oldDecl);
        // generic param decl
        if (oldDecl->astKind == ASTKind::GENERIC_PARAM_DECL) {
            return;
        }

        if (!decls.empty()) {
            // First verify if the downstream package status is stale
            auto definedPkg = decls[0]->fullPackageName;
            // Find all downstream packages
            auto downPackages = CompilerCodiraProject::GetInstance()->GetDependencyGraph()->GetDependents(definedPkg);
            // Check the status of all downstream packages
            auto tasks = CompilerCodiraProject::GetInstance()->GetCodeoManager()->CheckStatus(downPackages);
            // Compile all downstream packages before searching for references
            CompilerCodiraProject::GetInstance()->SubmitTasksToPool(tasks);
        }

        lsp::SymbolIndex *index = ark::CompilerCodiraProject::GetInstance()->GetIndex();
        if (!index) {
            return;
        }
        int curIdx = ast.GetCurToken(pos, 0, static_cast<int>(ast.tokens.size()) - 1);
        ExportIDItem exportIdItem;
        for (auto &decl : decls) {
            if (decl->astKind == Codira::AST::ASTKind::PACKAGE_DECL) {
                return;
            }
            auto id = GetSymbolId(*decl);
            if (!IsGlobalOrMemberOrItsParam(*decl)) {
                // For a local variable, maybe a function or a variable
                GetCurPkgUseAge(decl, ast, result);
                continue;
            }
            if (id == lsp::INVALID_SYMBOL_ID) {
                continue;
            }
            bool ret = CrossLanguangeDefinition::GetExportSID(GetArrayFromID(id), exportIdItem);
            if (ret) {
                nlohmann::json jsonValue;
                jsonValue["exportName"] = exportIdItem.exportName;
                jsonValue["containerName"] = exportIdItem.containerName;
                ValueOrError val(ValueOrErrorCheck::VALUE, jsonValue);
                reply(val);
            }
            break;
        }
    };
    arkScheduler->RunWithAST("GetExportsName", file, action);
}

void ArkServer::ApplyFileRefactor(const std::string &file,
    const std::string &selectedElement,
    const std::string &target,
    bool &isTest,
    const Callback<ValueOrError> &reply) const
{
    auto action = [file, target, selectedElement, isTest, reply = reply](const InputsAndAST &inputAST) {
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FileRefactorRespParams result;
        FileMove::FileMoveRefactor(inputAST.ast, result, file, selectedElement, target);
        nlohmann::json jsonValue;
        if (!isTest) {
            ToJSON(result, jsonValue);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("FileRefactor", file, action);
}

void ArkServer::FindWorkspaceSymbols(const std::string &query, const Callback<ValueOrError> &reply) const
{
    auto action = [query, reply = std::move(reply)](const InputsAndAST &) {
        auto result = lsp::GetWorkspaceSymbols(query);
        nlohmann::json jsonValue;
        for (auto &symbol : result) {
            nlohmann::json temp;
            if (ToJSON(symbol, temp)) {
                (void) jsonValue.push_back(temp);
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    std::string file;
    arkScheduler->RunWithAST("Symbol", file, action);
}

void ArkServer::FindHover(const std::string &file,
                          const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) mutable {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        Hover result = Hover();
        nlohmann::json jsonValue;
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (inputAST.ast == nullptr) {
            nullValueReply();
            return;
        }
        int ret = HoverImpl::FindHover(*(inputAST.ast), result, pos);
        if (ret == -1) {
            nullValueReply();
            return;
        }
        std::stringstream temp;
        for (auto &iter : result.markedString) {
            temp << iter;
        }
        if (temp.str().empty()) { // sometimes we dont have valid hover message
            nullValueReply();
            return;
        }
        jsonValue["contents"]["language"] = "Codira";
        jsonValue["contents"]["value"] = temp.str();

        jsonValue["range"]["start"]["line"] = result.range.start.line;
        jsonValue["range"]["start"]["character"] = result.range.start.column;
        jsonValue["range"]["end"]["line"] = result.range.end.line;
        jsonValue["range"]["end"]["character"] = result.range.end.column;
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };

    arkScheduler->RunWithAST("Hover", file, action);
}

void ArkServer::LocateSymbolAt(const std::string &file,
                               const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        LocatedSymbol result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        bool ret = LocateSymbolAtImpl::LocateSymbolAt(*(inputAST.ast), result, pos);
        nlohmann::json jsonValue;
        if (ret) {
            jsonValue["uri"] = result.Definition.uri.file;
            jsonValue["range"]["start"]["line"] = result.Definition.range.start.line;
            jsonValue["range"]["start"]["character"] = result.Definition.range.start.column;
            jsonValue["range"]["end"]["line"] = result.Definition.range.end.line;
            jsonValue["range"]["end"]["character"] = result.Definition.range.end.column;
            if (!result.CrossMessage.empty()) {
                for (auto &message: result.CrossMessage) {
                    nlohmann::json Value;
                    Value["targetLanguage"] = message.targetLanguage;
                    Value["functionName"] = message.functionName;
                    Value["retType"] = message.retType;
                    for (auto &item: message.functionParameters) {
                        (void) Value["functionParameters"].push_back(item);
                    }
                    (void) jsonValue["crossMessage"].push_back(Value);
                }
            }
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("Definition", file, action);
}

void ArkServer::LocateRegisterCrossSymbolAt(
        const CrossLanguageJumpParams &params, const Callback<ValueOrError> &reply) const
{
    RegisterCrossSymbolsResult result{};
    bool ret = CrossLanguangeDefinition::getRegisterCrossSymbols(params, result);
    nlohmann::json jsonValue;
    if (ret) {
        for (const auto &item : result.registerItems) {
            nlohmann::json temp;
            temp["definition"]["uri"] = item.definition.uri.file;
            temp["definition"]["range"]["start"]["line"] = item.definition.range.start.line;
            temp["definition"]["range"]["start"]["character"] = item.definition.range.start.column;
            temp["definition"]["range"]["end"]["line"] = item.definition.range.end.line;
            temp["definition"]["range"]["end"]["character"] = item.definition.range.end.column;
            temp["declaration"]["uri"] = item.declaration.uri.file;
            temp["declaration"]["range"]["start"]["line"] = item.declaration.range.start.line;
            temp["declaration"]["range"]["start"]["character"] = item.declaration.range.start.column;
            temp["declaration"]["range"]["end"]["line"] = item.declaration.range.end.line;
            temp["declaration"]["range"]["end"]["character"] = item.declaration.range.end.column;
            temp["registerName"] = item.registerName;
            temp["registerType"] = item.registerType;
            jsonValue.push_back(std::move(temp));
        }
    }
    ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
    reply(value);
}

void ArkServer::LocateCrossSymbolAt(const CrossLanguageJumpParams &params, const Callback<ValueOrError> &reply) const
{
    CrossSymbolsResult result{};
    bool ret = CrossLanguangeDefinition::getCrossSymbols(params, result);
    nlohmann::json jsonValue;
    if (ret) {
        for (auto &iter : result.locations) {
            nlohmann::json temp;
            temp["uri"] = iter.uri.file;
            temp["range"]["start"]["line"] = iter.range.start.line;
            temp["range"]["start"]["character"] = iter.range.start.column;
            temp["range"]["end"]["line"] = iter.range.end.line;
            temp["range"]["end"]["character"] = iter.range.end.column;
            (void)jsonValue.push_back(temp);
        }
    }
    ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
    reply(value);
}

void ArkServer::FindDocumentLink(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [this, file, reply = std::move(reply)](const InputsAndAST &inputAST) {
        nlohmann::json jsonValue;
        // jsonValue should be a documentLink array
        jsonValue = nlohmann::json::array();
        ValueOrError val(ValueOrErrorCheck::VALUE, jsonValue);
        reply(val);
        std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(file);
        UpdateModifierDiag(inputAST, diagnostics, file);
        callback->ReadyForDiagnostics(file, inputAST.inputs.version, diagnostics);
    };
    arkScheduler->RunWithAST("DocumentLink", file, action);
}

void ArkServer::UpdateModifierDiag(const InputsAndAST &inputAST,
    std::vector<DiagnosticToken> &diagnostics, const std::string &file) const
{
    if (!inputAST.ast || !inputAST.ast->file || !inputAST.ast->file->curPackage) {
        return;
    }
    auto fullPackageName = inputAST.ast->file->curPackage->fullPackageName;
    auto ret =
        CompilerCodiraProject::GetInstance()->CheckPackageModifier(*inputAST.ast->file, fullPackageName);
    if (ret) {
        auto it = std::find_if(diagnostics.begin(), diagnostics.end(), [](const DiagnosticToken &diagToken) {
            return diagToken.message ==
                   "the access level of child package can't be higher than that of parent package";
        });
        if (it != diagnostics.end()) {
            DiagnosticToken removeDiag = std::move(*it);
            diagnostics.erase(it);
            callback->RemoveDiagnostic(file, removeDiag);
        }
    }
}

void ArkServer::FindCompletion(const CompletionParams &params, const std::string &file,
                               const Callback<ValueOrError> &reply) const
{
    Trace::Log("ArkServer::FindCompletion in.");

    auto nullValueReply = [reply]() {
        ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
        reply(value);
    };

    int fileId;
    if (Options::GetInstance().IsOptionSet("test")) {
        fileId = CompilerCodiraProject::GetInstance()->GetFileID(file);
    } else {
        fileId = CompilerCodiraProject::GetInstance()->GetFileIDForCompete(file);
    }
    if (fileId < 0) {
        nullValueReply();
        return;
    }

    Codira::Position pos = {
        static_cast<unsigned int>(fileId),
        params.position.line,
        params.position.column
    };

    auto action = [reply, nullValueReply, params, pos](const InputsAndAST &input) {
        CompletionResult result;
        std::string prefix;
        if (input.ast == nullptr) {
            nullValueReply();
            return;
        }
        CompletionImpl::CodeComplete(*(input.ast), pos, result, prefix);
        CompilerCodiraProject::GetInstance()->ClearParseCache();
        CompletionList completionList;
        for (auto &iter : result.completions) {
            if (prefix.back() == '.' || IsMatchingCompletion(prefix, iter.name)) {
                if (iter.name.find(BOX_DECL_PREFIX) != std::string::npos) { continue; }
                auto score = CompilerCodiraProject::GetInstance()->CalculateScore(iter, prefix, result.cursorDepth);
                completionList.items.push_back(iter.Render(GetSortText(score), prefix));
            }
        }

        nlohmann::json jsonItems;
        for (auto &iter : completionList.items) {
            nlohmann::json value;
            if (!ToJSON(iter, value)) { continue; }
            (void)jsonItems.push_back(value);
        }
        ValueOrError val(ValueOrErrorCheck::VALUE, jsonItems);
        reply(val);
        CompilerCodiraProject::GetInstance()->ClearParseCache();
    };

    arkSchedulerOfComplete->RunWithASTCache("Completion", file, pos, action);
}

void ArkServer::FindSignatureHelp(const SignatureHelpParams &params, const std::string &file,
                                  const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](InputsAndAST inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        SignatureHelp result = SignatureHelp();
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (!inputAST.ast || !inputAST.ast->semaCache || !inputAST.ast->semaCache->packageInstance) {
            return;
        }
        SignatureHelpImpl signatureHelp(*(inputAST.ast), result,
                                           inputAST.ast->semaCache->packageInstance->importManager, params, pos);
        signatureHelp.FindSignatureHelp();
        if (result.signatures.empty()) {
            nullValueReply();
            return;
        }
        nlohmann::json jsonValue;
        jsonValue["activeParameter"] = result.activeParameter;
        jsonValue["activeSignature"] = result.activeSignature;
        for (auto &iter : result.signatures) {
            nlohmann::json temp;
            temp["label"] = iter.label;
            for (auto &param : iter.parameters) {
                nlohmann::json paramTemp;
                paramTemp["label"] = param;
                (void)temp["parameters"].push_back(paramTemp);
            }
            (void)jsonValue["signatures"].push_back(temp);
        }

        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    auto nullValueReply = [reply]() {
        ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
        reply(value);
    };
 
    int fileId;
    if (Options::GetInstance().IsOptionSet("test")) {
        fileId = CompilerCodiraProject::GetInstance()->GetFileID(file);
    } else {
        fileId = CompilerCodiraProject::GetInstance()->GetFileIDForCompete(file);
    }
    if (fileId < 0) {
        nullValueReply();
        return;
    }
 
    Codira::Position pos = {
        static_cast<unsigned int>(fileId),
        params.position.line,
        params.position.column
    };
    arkSchedulerOfSignature->RunWithASTCache("SignatureHelp", file, pos, action);
}

void ArkServer::PrepareRename(const std::string &file,
                              const TextDocumentPositionParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [file, params, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        MessageErrorDetail errorInfo;
        Range range = PrepareRename::PrepareImpl(*(inputAST.ast), pos, errorInfo);
        if (!errorInfo.message.empty()) {
            reply(ValueOrError(ValueOrErrorCheck::ERR, errorInfo));
        } else {
            nlohmann::json jsonValue;
            if (range.start.line == -1) {
                jsonValue = nullptr;
            } else {
                jsonValue["start"]["line"] = range.start.line;
                jsonValue["start"]["character"] = range.start.column;
                jsonValue["end"]["line"] = range.end.line;
                jsonValue["end"]["character"] = range.end.column;
            }
            ValueOrError val(ValueOrErrorCheck::VALUE, jsonValue);
            reply(val);
        }
    };
    arkScheduler->RunWithAST("PrepareRename", file, action);
}

void ArkServer::Rename(const std::string &file, const RenameParams &params,
                       const Callback<ValueOrError> &reply) const
{
    auto action = [this, file, params, reply = std::move(reply)](const InputsAndAST &inputAST) mutable {
        int fileId = CompilerCodiraProject::GetInstance()->GetFileID(file);
        if (fileId < 0) {
            return;
        }
        const Codira::Position pos = {
            static_cast<unsigned int>(fileId),
            params.position.line,
            params.position.column
        };

        auto newName = params.newName;
        std::vector<TextDocumentEdit> result;
        std::string path = RenameImpl::Rename(*(inputAST.ast), result, pos, newName, *callback);
        nlohmann::json jsonValue;
        if (result.empty()) {
            jsonValue = nullptr;
        }
        for (auto &iter : result) {
            nlohmann::json temp;
            (void) ToJSON(iter, temp);
            (void) jsonValue.push_back(temp);
        }
        nlohmann::json ret;
        ret["documentChanges"] = jsonValue;
        ValueOrError value(ValueOrErrorCheck::VALUE, ret);
        reply(value);
        if (!path.empty()) {
            this->callback->isRenameDefined = true;
            this->callback->path = path;
        }
    };
    arkScheduler->RunWithAST("Rename", file, action);
}

void ArkServer::FindBreakpoints(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply)](const InputsAndAST &inputAST) mutable {
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (inputAST.ast == nullptr) {
            nullValueReply();
            return;
        }
        std::set<BreakpointLocation> result;
        BreakpointsImpl::Breakpoints(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        if (result.empty()) {
            jsonValue = nullptr;
        }
        for (auto &breakpointLocation : result) {
            nlohmann::json temp;
            (void) ToJSON(breakpointLocation, temp);
            (void) jsonValue.push_back(temp);
        }
        nlohmann::json ret;
        ret["breakpointLocation"] = jsonValue;
        ValueOrError value(ValueOrErrorCheck::VALUE, ret);
        reply(value);
    };
    arkScheduler->RunWithAST("FindBreakpoints", file, action);
}

void ArkServer::FindCodeLens(const std::string &file, const Callback<ValueOrError> &reply) const
{
    auto action = [file, reply = std::move(reply)](const InputsAndAST &inputAST) mutable {
        auto nullValueReply = [reply]() {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
        };
        if (inputAST.ast == nullptr) {
            nullValueReply();
            return;
        }
        std::vector<CodeLens> result;
        CodeLensImpl::GetCodeLens(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        if (result.empty()) {
            jsonValue = nullptr;
        }
        for (auto &codeLens : result) {
            nlohmann::json temp;
            (void) ToJSON(codeLens, temp);
            (void) jsonValue.push_back(temp);
        }
        nlohmann::json ret;
        ret = jsonValue;
        ValueOrError value(ValueOrErrorCheck::VALUE, ret);
        reply(value);
    };
    arkScheduler->RunWithAST("FindCodeLens", file, action);
}

void ArkServer::ChangeWatchedFiles(const std::string &file, FileChangeType type, DocCache *docMgr) const
{
    auto action = [this, file, type, docMgr](const InputsAndAST &input) {
        if (type == FileChangeType::CHANGED) {
            Logger::Instance().LogMessage(MessageType::MSG_INFO, "change the file:  " + file);
            if (docMgr->GetDoc(file).version != -1) {
                return;
            }
            const std::string &contents = GetFileContents(file);
            if (std::hash<std::string>{}(contents) == std::hash<std::string>{}(docMgr->GetDoc(file).contents)) {
                Logger::Instance().LogMessage(MessageType::MSG_INFO, "recieve file change, but contens are same.");
                return;
            }
            int64_t version = docMgr->AddDoc(file, 0, contents);
            AddDoc(file, contents, version, ark::NeedDiagnostics::YES, true);
            return;
        }
        if (type == FileChangeType::CREATED) {
            Logger::Instance().LogMessage(MessageType::MSG_INFO, "creat the file:  " + file);
            std::string contents = docMgr->GetDoc(file).contents;
            if (docMgr->GetDoc(file).version == -1) {
                contents = GetFileContents(file);
            }
            int64_t version = docMgr->AddDoc(file, 0, contents);
            AddDoc(file, contents, version, ark::NeedDiagnostics::YES, true);
            return;
        }
        if (type == FileChangeType::DELETED) {
            Logger::Instance().LogMessage(MessageType::MSG_INFO, "delete the file:  " + file);
            CompilerCodiraProject::GetInstance()->IncrementForFileDelete(file);
            CompilerCodiraProject::GetInstance()->GetBgIndexDB()->DeleteFiles({file});
            this->callback->RemoveDocByFile(input.inputs.fileName);
        }
        if (!FileUtil::FileExist(input.onEditFile)) {
            return;
        }
        std::vector<DiagnosticToken> diagnostics = callback->GetDiagsOfCurFile(input.onEditFile);
        callback->ReadyForDiagnostics(input.onEditFile, input.inputs.version, diagnostics);
    };

    auto taskName = "ChangeWatchedFiles " + file;
    arkScheduler->RunWithAST(taskName, file, action);
}

void ArkServer::AddDoc(const std::string &file, const std::string &contents,
    int64_t version, NeedDiagnostics needDiagnostics, bool forceRebuild) const
{
    ParseInputs parseInputs;
    parseInputs.fileName = file;
    parseInputs.contents = contents;
    parseInputs.version = version;
    parseInputs.forceRebuild = forceRebuild;
    arkScheduler->Update(parseInputs, needDiagnostics);
}

void ArkServer::FindDocumentSymbol(const DocumentSymbolParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, reply](const InputsAndAST &inputAST) mutable {
        std::vector<DocumentSymbol> result;
        auto filePath = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
        // To avoid queue task in runWithAST crashing
        if (inputAST.ast == nullptr || !CompilerCodiraProject::GetInstance()->FileHasSemaCache(filePath)
            || Codira::FileUtil::HasExtension(filePath, CONSTANTS::CODIRA_MACRO_FILE_EXTENSION)) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        DocumentSymbolImpl::FindDocumentSymbols(*(inputAST.ast), result);
        nlohmann::json jsonValue;
        for (auto &documentSymbol: result) {
            nlohmann::json temp;
            if (!ToJSON(documentSymbol, temp)) {
                continue;
            }
            (void)jsonValue.push_back(temp);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    std::string file = FileStore::NormalizePath(URI::Resolve(params.textDocument.uri.file));
    arkScheduler->RunWithAST("DocumentSymbol", file, action);
}

void ArkServer::FindOverrideMethods(const std::string &file,
                               const OverrideMethodsParams &params, const Callback<ValueOrError> &reply) const
{
    auto action = [params, file, reply = std::move(reply), this](const InputsAndAST &inputAST) {
        Codira::Position pos = AlterPosition(file, params);
        if (pos == INVALID_POSITION) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindOverrideMethodResult result;
        if (inputAST.ast == nullptr) {
            ValueOrError value(ValueOrErrorCheck::VALUE, nullptr);
            reply(value);
            return;
        }
        FindOverrideMethodsImpl::FindOverrideMethods(*(inputAST.ast), result, pos, params.isExtend);
        nlohmann::json jsonValue;
        for (auto item: result.overrideMethods) {
            nlohmann::json overrideItem;
            overrideItem["importItem"] = item.package + CONSTANTS::DOT + item.identifier;
            overrideItem["fullPackageName"] = item.package;
            overrideItem["identifier"] = item.identifier;
            overrideItem["kind"] = item.kind;
            for (auto method: item.overrideMethodInfos) {
                nlohmann::json jsonItem;
                jsonItem["deprecated"] = method.deprecated;
                jsonItem["isProp"] = method.isProp;
                jsonItem["signatureWithRet"] = method.signatureWithRet;
                jsonItem["insertText"] = method.insertText;
                overrideItem["data"].push_back(jsonItem);
            }
            jsonValue.push_back(overrideItem);
        }
        ValueOrError value(ValueOrErrorCheck::VALUE, jsonValue);
        reply(value);
    };
    arkScheduler->RunWithAST("OverrideMethods", file, action);
}

Codira::Position ArkServer::AlterPosition(const std::string &file,
                                           const TextDocumentPositionParams &params) const
{
    int fileId = CompilerCodiraProject::GetInstance()->GetFileID(file);
    if (fileId < 0) {
        return INVALID_POSITION;
    }
    return Codira::Position{
            static_cast<unsigned int>(fileId),
            params.position.line,
            params.position.column
    };
}

static std::vector<std::unique_ptr<Tweak::Selection>> CreateTweakSelection(const InputsAndAST &inputAST,
    const std::string &file, Range range, std::map<std::string, std::string> extraOptions)
{
    std::vector<std::unique_ptr<Tweak::Selection>> result;

    SelectionTree::CreateEach(*inputAST.ast, file, range.start,
        range.end, [&](SelectionTree selectionTree) {
            // cannot tweak:
            // if selected range is not GLOBAL_VAR/MEMBER_VAR/GLOBAL_FUNC_BODY/MEMBER_FUNC_BODY/EXTEND_FUNC_BODY
            if (!selectionTree.root() || selectionTree.SelectedScope() == SelectionTree::Scope::UNKNOWN) {
                return false;
            }
            auto tweakSelection =
                std::make_unique<Tweak::Selection>(*inputAST.ast, range, std::move(selectionTree), extraOptions);

            result.push_back(std::move(tweakSelection));
            return true;
        });
    return std::move(result);
}

void ArkServer::EnumerateTweaks(const std::string &file, Range range, const Callback<std::vector<TweakRef>> &cb) const
{
    auto action = [file, range, cb = std::move(cb)]
        (const InputsAndAST &inputAST) mutable {
            std::vector<TweakRef> res;
            if (!inputAST.ast) {
                cb(std::move(res));
                return;
            }

            // update pos fileID
            range.start.fileID = inputAST.ast->fileID;
            range.end.fileID = inputAST.ast->fileID;

            // check current token is the kind which required in function CheckTokenKind(TokenKind)
            range.start = PosFromIDE2Char(range.start);
            PositionIDEToUTF8(inputAST.ast->tokens, range.start, *inputAST.ast->file);
            range.end = PosFromIDE2Char(range.end);
            PositionIDEToUTF8(inputAST.ast->tokens, range.end, *inputAST.ast->file);

            // get Selections by range
            // visit all Tweaks check tweak available
            auto selections =
                CreateTweakSelection(inputAST, file, range, std::map<std::string, std::string>());
            if (selections.empty()) {
                cb(std::move(res));
                return;
            }
            std::unordered_set<std::string> preparedTweaks;
            auto deduplicatingFilter = [&preparedTweaks](const Tweak &tweak) {
                return !preparedTweaks.count(tweak.Id());
            };
            for (const auto &selection : selections) {
                if (!selection) {
                    continue;
                }
                for (const auto &tweak
                    : Tweak::PrepareTweaks(*selection, deduplicatingFilter)) {
                    if (!tweak) {
                        continue;
                    }
                    res.push_back({tweak->Id(), tweak->Title(), tweak->Kind(),
                        tweak->ExtraOptions()});
                    preparedTweaks.insert(tweak->Id());
                }
            }
            cb(std::move(res));
        };
    arkScheduler-> RunWithAST("EnumerateTweaks", file, std::move(action));
}

void ArkServer::ApplyTweak(const std::string &file, Range selection, const std::string &id,
    std::map<std::string, std::string> extraOptions, const Callback<Tweak::Effect> &cb)
{
    auto action = [file, selection, id, extraOptions, cb = std::move(cb), this]
        (const InputsAndAST &inputAST) mutable {
            Tweak::Effect effect;
            if (!inputAST.ast || !inputAST.ast->sourceManager) {
                return cb(std::move(effect));
            }

            selection.start.fileID = inputAST.ast->fileID;
            selection.end.fileID = inputAST.ast->fileID;

            selection.start = PosFromIDE2Char(selection.start);
            PositionIDEToUTF8(inputAST.ast->tokens, selection.start, *inputAST.ast->file);
            selection.end = PosFromIDE2Char(selection.end);
            PositionIDEToUTF8(inputAST.ast->tokens, selection.end, *inputAST.ast->file);

            auto selections = CreateTweakSelection(inputAST, file, selection, extraOptions);
            if (selections.empty()) {
                return cb(std::move(effect));
            }
            for (const auto &sel : selections) {
                if (!sel) {
                    continue;
                }
                auto tweak = Tweak::PrepareTweak(id, *sel);
                if (!tweak.has_value() || !tweak.value()) {
                    continue;
                }
                auto effectOpt = tweak.value()->Apply(*sel);
                if (effectOpt.has_value()) {
                    effect = effectOpt.value();
                    break;
                }
            }

            return cb(std::move(effect));
        };
    arkScheduler->RunWithAST("ApplyTweak", file, std::move(action));
}
} // namespace ark
