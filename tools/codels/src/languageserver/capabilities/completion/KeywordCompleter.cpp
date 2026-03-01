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

#include "KeywordCompleter.h"
#include "CompletionImpl.h"

using namespace Codira::Meta;

namespace ark {
std::unordered_set<TokenKind> InitKeyWordKinds()
{
    std::unordered_set<TokenKind> result;
    size_t tokensSize = sizeof(Codira::TOKENS) / sizeof(Codira::TOKENS[0]);
    for (size_t i = 0; i < tokensSize; i++) {
        std::string token{TOKENS[i]};
        if (token.empty() || !isalpha(token[0])) {
            continue;
        }
        result.insert(static_cast<TokenKind>(i));
    }
    return result;
}

std::unordered_set<TokenKind> KeywordCompleter::keyWordKinds = InitKeyWordKinds();
std::unordered_set<TokenKind> KeywordCompleter::declKeyWordKinds = {
    TokenKind::CLASS, TokenKind::STRUCT, TokenKind::ENUM, TokenKind::INTERFACE,
    TokenKind::FUNC, TokenKind::TYPE, TokenKind::MACRO, TokenKind::PROP, TokenKind::LET, TokenKind::VAR
};

std::vector<CodeSnippet> KeywordCompleter::codeSnippetList = {
    {"for", "for (pattern in expression) {}", "for (${1:pattern} in ${0:expression}) {\n\t\n}"},
    {"while", "while (expression) {}", "while (${0:expression}) {\n\t\n}"},
    {"do", "do {} while (expression)", "do {\n\t\n} while (${0:expression})"},
    {"func", "func name(){}", "func ${1:name}($2) {\n\t$0\n}"},
    {"func", "func name<T>(){}", "func ${1:name}<T>($2) {\n\t$0\n}"},
    {"if", "if (condExpr) {}", "if (${0:condExpr}) {\n\t\n}"}, {"else", "else {}", "else {\n\t$0\n}"},
    {"class", "class name {}", "class ${1:name} {\n\t$0\n}"},
    {"class", "class name<T> {}", "class ${1:name}<T> {\n\t$0\n}"},
    {"interface", "interface name {}", "interface ${1:name} {\n\t$0\n}"},
    {"interface", "interface name<T> {}", "interface ${1:name}<T> {\n\t$0\n}"},
    {"match", "match (condExpr) {}", "match (${0:condExpr}) {\n\t\n}"},
    {"case", "case pattern => expressions", "case ${1:pattern} => ${0:expressions}"},
    {"enum", "enum name {}", "enum ${1:name} {\n\t$0\n}"}, {"enum", "enum name<T> {}", "enum ${1:name}<T> {\n\t$0\n}"},
    {"struct", "struct name {}", "struct ${1:name} {\n\t$0\n}"},
    {"struct", "struct name<T> {}", "struct ${1:name}<T> {\n\t$0\n}"}, {"init", "init() {}", "init($0) {\n\t\n}"},
    {"try", "try {}", "try {\n\t$0\n}"}, {"catch", "catch (name) {}", "catch (${0:name}) {\n\t\n}"},
    {"finally", "finally {}", "finally {\n\t$0\n}"},
    {"type", "type newName = originalName", "type ${1:newName}  = ${0:originalName}"},
    {"extend", "extend name{}", "extend ${0:name}{\n\t\n}"},
    {"extend", "extend<T> name<T>{}", "extend<T> ${0:name}<T>{\n\t\n}"}, {"quote", "quote()", "quote(${0: })"},
    {"macro", "macro name(input:Tokens):Tokens{}", "macro ${1:name}(${2:input}: Tokens): Tokens {\n\t$0\n}"},
    {"macro", "macro name(attr:Tokens, input:Tokens): Tokens {}",
        "macro ${1:name}(${2:attr}: Tokens, ${3:input}: Tokens): Tokens {\n\t$0\n}"},
    {"foreign", "foreign {}", "foreign {\n\t$0\n}"}, {"main", "main() {}", "main() {\n\t$0\n}"},
    {"import", "import pkgName.* as newName.*", "import ${1:pkgName.*} as ${0:newName.*}"},
    {"When", "When[expression]", "When[${0:expression}]"},
    {"VArray", "VArray<T, $IntLiteral>", "VArray<${1:T}, $${2:IntLiteral}>"},
    {"VArray", "VArray<T, $IntLiteral>(initElement: (Int64) -> T)",
        "VArray<${1:T}, $${2:IntLiteral}>(${0:initElement: (Int64) -> T})"},
    {"VArray", "VArray<T, $IntLiteral>(repeat!: T)", "VArray<${1:T}, $${2:IntLiteral}>(repeat: ${0:T})"},
    {"spawn", "spawn {}", "spawn {\n\t$0\n}"},
    {"spawn", "spawn(threadContext) {}", "spawn(${1:threadContext}) {\n\t$0\n}"},
    {"Annotation", "Annotation", "Annotation"},
    {"Deprecated", "Deprecated", "Deprecated"},
    {"Frozen", "Frozen", "Frozen"},
    {"IfAvailable", "IfAvailable", "IfAvailable"},
    {"IfAvailable", "IfAvailable(level: , {=> }, {=> })",
        "IfAvailable(${1:level: ,} {=>\n\t$2\n}, {=>\n\t$3\n})"},
    {"IfAvailable", "IfAvailable(syscap: , {=> }, {=> })",
        "IfAvailable(${1:syscap: ,} {=>\n\t$2\n}, {=>\n\t$3\n})"},
    {"prop", "prop name: T {get(){val}}", "prop ${0:name}: ${1:T} {\n\tget() {\n\t\t${3:val}\n\t}\n}"},
    {"prop", "prop name: T {get(){val} set(v){}}",
        "prop ${0:name}: ${1:T} {\n\tget() {\n\t\t${3:val}\n\t}\n\tset(v) {\n\t}\n}"},
};

std::vector<std::string> KeywordCompleter::keyWordFromLSP = {"true", "false", "When"};

void KeywordCompleter::AddKeyWord(
    const char *tokens[], int size, ark::CompletionResult &result, std::function<bool(const char *)> condition)
{
    for (int i = 0; i < size; i++) {
        std::string token(tokens[i]);
        if (token.empty() || !isalpha(token[0]) || !condition(token.c_str())) {
            continue;
        }
        CodeCompletion item;
        item.name = item.label = item.insertText = token;
        item.kind = CompletionItemKind::CIK_KEYWORD;
        item.sortType = SortType::KEYWORD;
        result.completions.push_back(item);
    }
}

void KeywordCompleter::AddKeyWordByLSP(ark::CompletionResult &result)
{
    for (auto &keyWord : keyWordFromLSP) {
        CodeCompletion item;
        item.kind = CompletionItemKind::CIK_KEYWORD;
        item.name = item.label = item.insertText = keyWord;
        item.sortType = SortType::KEYWORD;
        result.completions.push_back(item);
    }
    for (auto &iter : codeSnippetList) {
        CodeCompletion item;
        item.kind = CompletionItemKind::CIK_KEYWORD;
        item.name = iter.keyWord;
        item.detail = iter.label;
        item.label = iter.label;
        item.insertText = iter.snippet;
        item.sortType = SortType::KEYWORD;
        result.completions.push_back(item);
    }
}

void KeywordCompleter::Complete(ark::CompletionResult &result)
{
    AddKeyWord(Codira::TOKENS, static_cast<int>(sizeof(Codira::TOKENS) / sizeof(Codira::TOKENS[0])), result,
        [](const char *token) { return !IsExperimental(token); });
    AddKeyWord(Codira::ANNOTATION_TOKENS,
        static_cast<int>(sizeof(Codira::ANNOTATION_TOKENS) / sizeof(Codira::ANNOTATION_TOKENS[0])), result,
        [](const char *) { return true; });
    AddKeyWordByLSP(result);
}
} // namespace ark
