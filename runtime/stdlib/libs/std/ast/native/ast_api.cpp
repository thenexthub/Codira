/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "ast_api.h"

#include <cmath>

#include "Codira/Basic/DiagnosticEmitter.h"
#include "Codira/Basic/Print.h"
#include "Codira/Basic/SourceManager.h"
#include "Codira/Frontend/CompilerInstance.h"
#include "Codira/Macro/MacroCommon.h"
#include "Codira/Macro/NodeSerialization.h"
#include "Codira/Macro/TokenSerialization.h"
#include "Codira/Parse/Parser.h"

using namespace Codira;

namespace {
// type is not an independent syntax. diagnosis engine does not provide a proper prompt message.
enum class ParseKind : uint8_t { EXPR, DECL, PROGRAM, PATTERN };

const std::string INVALID_POSITION_MSG = "There is a token with invalid position in the input.\n";

/// get the compileCoded compile option from MacroCall* if not null
bool GetCompileCoded(void* fptr)
{
    if (!fptr) {
        return false;
    }
    auto mc = reinterpret_cast<MacroCall*>(fptr);
    return mc->ci->invocation.globalOptions.compileCoded;
}

std::string ParseWithError(
    void* fptr, const std::vector<Token> oldTokens, ParseKind kind, ScopeKind scopeKind = ScopeKind::UNKNOWN_SCOPE)
{
    std::vector<Position> escapePosVec = {};
    MacroFormatter formatter = MacroFormatter(oldTokens, escapePosVec, 1);
    auto tokenStr = formatter.Produce(false);
    DiagnosticEngine diag;
    diag.EnableCheckRangeErrorCodeRatherICE();
    SourceManager sm;
    auto fileID = sm.AddSource("generate by tokens", tokenStr);
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    if (scopeKind == ScopeKind::UNKNOWN_SCOPE) {
        diag.DisableScopeCheck();
    }
    diag.SetDisableWarning(true);
    Parser declParser(fileID, tokenStr, diag, sm, false, GetCompileCoded(fptr));
    if (kind == ParseKind::PROGRAM) {
        declParser.ParseTopLevel();
    } else if (kind == ParseKind::EXPR) {
        declParser.ParseExprLibast();
    } else if (kind == ParseKind::PATTERN) {
        declParser.ParsePattern();
    } else {
        declParser.ParseDecl(scopeKind);
    }
    std::string outStr;
    auto ret = diag.GetCategoryDiagnosticsString(DiagCategory::PARSE, outStr);
    if (ret != DiagEngineErrorCode::NO_ERRORS) {
        diag.DisableCheckRangeErrorCodeRatherICE();
        return "DiagnEngineError occurs";
    }
    return outStr;
}

std::string LexWithError(void* fptr, const std::string code)
{
    DiagnosticEngine diag;
    diag.EnableCheckRangeErrorCodeRatherICE();
    SourceManager sm;
    auto fileID = sm.AddSource("generate by tokens", code);
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);

    diag.SetDisableWarning(true);
    Lexer lex(fileID, code, diag, sm, false);
    auto tokens = lex.GetTokens();

    std::string outStr;
    auto ret = diag.GetCategoryDiagnosticsString(DiagCategory::LEX, outStr);
    if (ret != DiagEngineErrorCode::NO_ERRORS) {
        diag.DisableCheckRangeErrorCodeRatherICE();
        return "DiagnEngineError occurs";
    }
    return outStr;
}

static char* CloneString(const std::string s, const size_t size)
{
    auto ret = (char*)malloc(size * sizeof(char));
    if (ret == nullptr) {
        return nullptr;
    }
    std::copy(s.begin(), s.end(), ret);
    ret[size - 1] = '\0';
    return ret;
}

void TryCombineDoubleArrow(MacroCall* macCall, std::vector<Token> inputTokens, std::vector<Token>& outputTokens)
{
    auto token0 = inputTokens[0];
    auto token1 = inputTokens[1];
    if (token0.End().column == token1.Begin().column) {
        // >= > trans to > =>
        if (token0.kind == TokenKind::GE) {
            token0 = Token(TokenKind::GT, ">", token0.Begin(), token0.Begin() + Len(TokenKind::GT));
            token1 = Token(TokenKind::DOUBLE_ARROW, "=>", token0.Begin() + Len(TokenKind::GT), token1.End());
        } else { // >>= > trans to >> =>
            token0 = Token(TokenKind::RSHIFT, ">>", token0.Begin(), token0.Begin() + Len(TokenKind::RSHIFT));
            token1 = Token(TokenKind::DOUBLE_ARROW, "=>", token0.Begin() + Len(TokenKind::RSHIFT), token1.End());
        }
    }
    if (macCall != nullptr) {
        token0 = Token(token0.kind, token0.Value(), macCall->GetBeginPos(), macCall->GetEndPos());
        token1 = Token(token1.kind, token1.Value(), macCall->GetBeginPos(), macCall->GetEndPos());
    }
    outputTokens.emplace_back(token0);
    outputTokens.emplace_back(token1);
    return;
}
} // namespace

extern "C" {
ParseRes* CODE_AST_Lex(void* fptr, const char* code)
{
    Codira::ICE::TriggerPointSetter iceSetter(CompileStage::PARSE);
    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    diag.SetDisableWarning(true);
    std::string cangjieCode(code);
    Lexer lex(cangjieCode, diag, sm, false, false);
    std::vector<Token> tokens{};

    MacroCall* macCall = nullptr;
    if (fptr != nullptr) {
        macCall = reinterpret_cast<MacroCall*>(fptr);
    }
    while (true) {
        if (lex.Seeing({TokenKind::GE, TokenKind::GT}, false, false) || lex.Seeing({TokenKind::RSHIFT_ASSIGN, TokenKind::GT}, false, false)) {
            auto geToken = lex.Next();
            auto gtToken = lex.Next();
            TryCombineDoubleArrow(macCall, {geToken, gtToken}, tokens);
        }
        Token token = lex.Next();
        if (macCall == nullptr) {
            tokens.emplace_back(token);
        } else {
            auto tk = Token(token.kind, token.Value(), macCall->GetBeginPos(), macCall->GetEndPos());
            tk.isSingleQuote = token.isSingleQuote;
            if (token.kind == TokenKind::MULTILINE_RAW_STRING) {
                tk.delimiterNum = token.delimiterNum;
            }
            tokens.emplace_back(tk);
        }
        if (token.kind == TokenKind::END) {
            break;
        }
    }

    ParseRes* result = (ParseRes*)malloc(sizeof(ParseRes));
    if (result == nullptr) {
        return nullptr;
    }
    if (diag.GetErrorCount()) {
        std::string errMsg;
        result->node = nullptr;
        errMsg = LexWithError(fptr, cangjieCode);
        result->eMsg = CloneString(errMsg, errMsg.size() + 1);
        return result;
    }
    result->node = TokenSerialization::GetTokensBytesWithHead(tokens);
    result->eMsg = nullptr;
    return result;
}

ParseRes* CODE_AST_ParseExpr(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter)
{
    Codira::ICE::TriggerPointSetter iceSetter(CompileStage::PARSE);
    std::vector<Token> tokens = TokenSerialization::GetTokensFromBytes(tokensBytes);
    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    diag.DisableScopeCheck();
    diag.SetDisableWarning(true);
    diag.EnableCheckRangeErrorCodeRatherICE();
    Parser parser(tokens, diag, sm, false, GetCompileCoded(fptr));
    auto expr = parser.ParseExprLibast();
    while (parser.Skip(TokenKind::NL) || parser.Skip(TokenKind::SEMI)) {
    }

    // secondary parsing for printing error codes, and not modify return tokens position
    ParseRes* result = (ParseRes*)malloc(sizeof(ParseRes));
    if (result == nullptr) {
        return nullptr;
    }
    if (diag.GetErrorCount()) {
        std::string errMsg;
        result->node = nullptr;
        auto ret = diag.GetCategoryDiagnosticsString(DiagCategory::PARSE, errMsg);
        if (ret != DiagEngineErrorCode::NO_ERRORS) {
            diag.DisableCheckRangeErrorCodeRatherICE();
            result->eMsg = CloneString(INVALID_POSITION_MSG, INVALID_POSITION_MSG.size() + 1);
            return result;
        }
        errMsg = ParseWithError(fptr, tokens, ParseKind::EXPR);
        result->eMsg = (char*)malloc((errMsg.size() + 1) * sizeof(char));
        // result free on cangjie side
        if (result->eMsg == nullptr) {
            return result;
        }
        std::copy(errMsg.begin(), errMsg.end(), result->eMsg);
        result->eMsg[errMsg.size()] = '\0';
        return result;
    }
    NodeSerialization::NodeWriter nodeWriter(expr.get());
    if (tokenCounter) {
        *tokenCounter = static_cast<int64_t>(parser.GetProcessedTokens());
    }
    result->node = nodeWriter.ExportNode();
    result->eMsg = nullptr;
    return result;
}

ParseRes* CODE_AST_ParseAnnotationArguments(const uint8_t* tokensBytes)
{
    Codira::ICE::TriggerPointSetter iceSetter(CompileStage::PARSE);
    std::vector<Token> tokens = TokenSerialization::GetTokensFromBytes(tokensBytes);
    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    diag.DisableScopeCheck();
    diag.SetDisableWarning(true);
    diag.EnableCheckRangeErrorCodeRatherICE();
    Parser parser(tokens, diag, sm, false, false);
    auto node = parser.ParseCustomAnnotation();

    ParseRes* result = (ParseRes*)malloc(sizeof(ParseRes));
    // result free on cangjie side
    if (result == nullptr) {
        return nullptr;
    }
    NodeSerialization::NodeWriter nodeWriter(node.get());
    result->node = nodeWriter.ExportNode();
    result->eMsg = nullptr;
    return result;
}

ParseRes* CODE_AST_ParsePattern(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter)
{
    Codira::ICE::TriggerPointSetter iceSetter(CompileStage::PARSE);
    std::vector<Token> tokens = TokenSerialization::GetTokensFromBytes(tokensBytes);
    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    diag.DisableScopeCheck();
    diag.SetDisableWarning(true);
    diag.EnableCheckRangeErrorCodeRatherICE();
    Parser parser(tokens, diag, sm, false, GetCompileCoded(fptr));
    auto node = parser.ParsePattern();

    ParseRes* result = (ParseRes*)malloc(sizeof(ParseRes));
    // result free on cangjie side
    if (result == nullptr) {
        return nullptr;
    }
    if (diag.GetErrorCount()) {
        std::string errMsg;
        result->node = nullptr;
        auto ret = diag.GetCategoryDiagnosticsString(DiagCategory::PARSE, errMsg);
        if (ret != DiagEngineErrorCode::NO_ERRORS) {
            diag.DisableCheckRangeErrorCodeRatherICE();
            result->eMsg = CloneString(INVALID_POSITION_MSG, INVALID_POSITION_MSG.size() + 1);
            return result;
        }
        errMsg = ParseWithError(fptr, tokens, ParseKind::PATTERN);
        result->eMsg = (char*)malloc((errMsg.size() + 1) * sizeof(char));
        // result free on cangjie side
        if (result->eMsg == nullptr) {
            return result;
        }
        std::copy(errMsg.begin(), errMsg.end(), result->eMsg);
        result->eMsg[errMsg.size()] = '\0';
        return result;
    }
    NodeSerialization::NodeWriter nodeWriter(node.get());
    if (tokenCounter) {
        *tokenCounter = static_cast<int64_t>(parser.GetProcessedTokens());
    }
    result->node = nodeWriter.ExportNode();
    result->eMsg = nullptr;
    return result;
}

ParseRes* CODE_AST_ParseType(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter)
{
    Codira::ICE::TriggerPointSetter iceSetter(CompileStage::PARSE);
    std::vector<Token> tokens = TokenSerialization::GetTokensFromBytes(tokensBytes);
    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    diag.DisableScopeCheck();
    diag.SetDisableWarning(true);
    diag.EnableCheckRangeErrorCodeRatherICE();
    Parser parser(tokens, diag, sm, false, GetCompileCoded(fptr));
    diag.EmitCategoryDiagnostics(DiagCategory::PARSE);
    auto node = parser.ParseType();

    ParseRes* result = (ParseRes*)malloc(sizeof(ParseRes));
    // result free on cangjie side
    if (result == nullptr) {
        return nullptr;
    }
    if (diag.GetErrorCount()) {
        result->node = nullptr;
        result->eMsg = nullptr;
        std::string errMsg;
        auto ret = diag.GetCategoryDiagnosticsString(DiagCategory::PARSE, errMsg);
        if (ret != DiagEngineErrorCode::NO_ERRORS) {
            diag.DisableCheckRangeErrorCodeRatherICE();
            result->eMsg = CloneString(INVALID_POSITION_MSG, INVALID_POSITION_MSG.size() + 1);
            return result;
        }
        return result;
    }
    NodeSerialization::NodeWriter nodeWriter(node.get());
    if (tokenCounter) {
        *tokenCounter = static_cast<int64_t>(parser.GetProcessedTokens());
    }
    result->node = nodeWriter.ExportNode();
    result->eMsg = nullptr;
    return result;
}

ParseRes* CODE_ParseDeclCommon(void* fptr, const uint8_t* tokensBytes, ScopeKind scopeKind, int64_t* tokenCounter)
{
    Codira::ICE::TriggerPointSetter iceSetter(CompileStage::PARSE);
    std::vector<Token> tokens = TokenSerialization::GetTokensFromBytes(tokensBytes);
    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    // unknow_scope can not distingulish attribute is legal or not, so disable scope check.
    // such as FuncDecl in class can use open, topLevel can not.
    if (scopeKind == ScopeKind::UNKNOWN_SCOPE) {
        diag.DisableScopeCheck();
    }
    diag.SetDisableWarning(true);
    diag.EnableCheckRangeErrorCodeRatherICE();
    Parser declParser(tokens, diag, sm, false, GetCompileCoded(fptr));
    // Using unknown_scope to differ normal parser and parser in libast. Normal parsing step has solid scope.
    auto decl = declParser.ParseDecl(scopeKind);

    ParseRes* result = (ParseRes*)malloc(sizeof(ParseRes));
    // result free on cangjie side
    if (result == nullptr) {
        return nullptr;
    }
    if (diag.GetErrorCount()) {
        std::string errMsg;
        result->node = nullptr;
        auto ret = diag.GetCategoryDiagnosticsString(DiagCategory::PARSE, errMsg);
        if (ret != DiagEngineErrorCode::NO_ERRORS) {
            diag.DisableCheckRangeErrorCodeRatherICE();
            result->eMsg = CloneString(INVALID_POSITION_MSG, INVALID_POSITION_MSG.size() + 1);
            return result;
        }
        errMsg = ParseWithError(fptr, tokens, ParseKind::DECL, scopeKind);
        result->eMsg = (char*)malloc((errMsg.size() + 1) * sizeof(char));
        // result free on cangjie side
        if (result->eMsg == nullptr) {
            return result;
        }
        std::copy(errMsg.begin(), errMsg.end(), result->eMsg);
        result->eMsg[errMsg.size()] = '\0';
        return result;
    }
    NodeSerialization::NodeWriter nodeWriter(decl.get());
    if (tokenCounter) {
        *tokenCounter = static_cast<int64_t>(declParser.GetProcessedTokens());
    }
    result->node = nodeWriter.ExportNode();
    result->eMsg = nullptr;
    return result;
}

ParseRes* CODE_AST_ParseDecl(void* fptr, const uint8_t* tokensBytes, int64_t* tokenCounter)
{
    return CODE_ParseDeclCommon(fptr, tokensBytes, ScopeKind::UNKNOWN_SCOPE, tokenCounter);
}

ParseRes* CODE_AST_ParsePropMemberDecl(void* fptr, const uint8_t* tokensBytes)
{
    return CODE_ParseDeclCommon(fptr, tokensBytes, ScopeKind::PROP_MEMBER_GETTER_BODY, nullptr);
}

ParseRes* CODE_AST_ParsePrimaryConstructor(void* fptr, const uint8_t* tokensBytes)
{
    return CODE_ParseDeclCommon(fptr, tokensBytes, ScopeKind::CLASS_BODY, nullptr);
}

ParseRes* CODE_AST_ParseTopLevel(void* fptr, const uint8_t* tokensBytes)
{
    Codira::ICE::TriggerPointSetter iceSetter(CompileStage::PARSE);
    std::vector<Token> tokens = TokenSerialization::GetTokensFromBytes(tokensBytes);
    DiagnosticEngine diag;
    SourceManager sm;
    diag.SetSourceManager(&sm);
    diag.SetDiagnoseStatus(true);
    diag.DisableScopeCheck();
    diag.SetDisableWarning(true);
    diag.EnableCheckRangeErrorCodeRatherICE();
    Parser parser(tokens, diag, sm, false, GetCompileCoded(fptr));
    auto file = parser.ParseTopLevel();
    ParseRes* result = (ParseRes*)malloc(sizeof(ParseRes));
    // result free on cangjie side
    if (result == nullptr) {
        return nullptr;
    }
    if (diag.GetErrorCount()) {
        std::string errMsg;
        result->node = nullptr;
        auto ret = diag.GetCategoryDiagnosticsString(DiagCategory::PARSE, errMsg);
        if (ret != DiagEngineErrorCode::NO_ERRORS) {
            diag.DisableCheckRangeErrorCodeRatherICE();
            result->eMsg = CloneString(INVALID_POSITION_MSG, INVALID_POSITION_MSG.size() + 1);
            return result;
        }
        errMsg = ParseWithError(fptr, tokens, ParseKind::PROGRAM);
        result->eMsg = (char*)malloc((errMsg.size() + 1) * sizeof(char));
        // result free on cangjie side
        if (result->eMsg == nullptr) {
            return result;
        }
        std::copy(errMsg.begin(), errMsg.end(), result->eMsg);
        result->eMsg[errMsg.size()] = '\0';
        return result;
    }
    NodeSerialization::NodeWriter nodeWriter(file.get());
    result->node = nodeWriter.ExportNode();
    result->eMsg = nullptr;
    return result;
}

bool CODE_CheckParentContext(void* fptr, char* parent, bool report)
{
    auto macCall = reinterpret_cast<MacroCall*>(fptr);
    return macCall->CheckParentContext(parent, report);
}

void CODE_SetItemInfo(void* fptr, char* key, void* value, uint8_t type)
{
    auto macCall = reinterpret_cast<MacroCall*>(fptr);
    macCall->SetItemMacroContext(key, value, type);
}

void*** CODE_GetChildMessages(void* fptr, char* children)
{
    auto macCall = reinterpret_cast<MacroCall*>(fptr);
    return macCall->GetChildMessagesFromMacroContext(children);
}

void CODE_CheckAddSpace(const uint8_t* tokBytes, bool* spaceFlag)
{
    std::vector<Token> tokens = TokenSerialization::GetTokensFromBytes(tokBytes);
    CODEC_ASSERT(tokens.size() != 0);
    for (size_t loop = 0; loop < tokens.size() - 1; loop++) {
        spaceFlag[loop] = CheckAddSpace(tokens[loop], tokens[loop + 1]);
    }
    spaceFlag[tokens.size() - 1] = false;
}

void CODE_GetMacroPosition(void* fptr, unsigned int* fileID, int* line, int* column)
{
    auto macCall = reinterpret_cast<MacroCall*>(fptr);
    auto pos = macCall->GetBeginPos();
    *fileID = pos.fileID;
    *line = pos.line;
    *column = pos.column;
    return;
}

const uint8_t DIAG_REPORT_RANGE_ERROR = 1;
const uint8_t DIAG_REPORT_FILEID_ERROR = 2;

uint8_t CODE_AST_DiagReport(
    void* fptr, const int* level, const uint8_t* tokensBytes, const char* message, const char* hint)
{
    if (fptr == nullptr) {
        return 0;
    }

    auto tokens = TokenSerialization::GetTokensFromBytes(tokensBytes);

    auto macCall = reinterpret_cast<MacroCall*>(fptr);
    auto invocation = macCall->GetInvocation().get();

    auto begin = invocation->atPos;
    auto end = invocation->identifierPos + invocation->fullName.size();
    // we optimize the display length because the printout doesn't look good caused by the last NL token.
    auto tokensEndPos = (tokens.empty()) ? end : tokens.back().End();
    if (!tokens.empty() && tokens.back().kind == TokenKind::NL) {
        tokensEndPos = tokens.back().Begin();
    }
    Range range = (tokens.empty()) ? MakeRange(begin, end) : MakeRange(tokens[0].Begin(), tokensEndPos);
    if (range.begin.fileID != macCall->GetBeginPos().fileID) {
        return DIAG_REPORT_FILEID_ERROR;
    }
    if (!tokens.empty() && (range.begin <= macCall->GetBeginPos() || range.end > macCall->GetEndPos())) {
        return DIAG_REPORT_RANGE_ERROR;
    }

    macCall->DiagReport(*level, range, message, hint);
    return 0;
}

const int FLOAT64_PRECISION = 16;

extern char* CODE_AST_Float64ToCPointer(const double num)
{
    std::string s;
    std::stringstream stream;
    if (std::isfinite(num) && floorf(num) - num == 0) {
        stream << std::fixed << std::setprecision(1) << num;
    } else {
        stream << std::setprecision(FLOAT64_PRECISION) << num;
    }
    s = stream.str();
    return CloneString(s, s.size() + 1);
}
}
