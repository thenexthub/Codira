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

#include "SemanticTokensAdaptor.h"

#include <vector>
#include <map>
namespace ark {
const std::vector<std::string> SemanticTokensAdaptor::TOKEN_MODIFIERS = { "declaration", "documentation", "static",
    "abstract", "deprecated", "async", "readonly" };
// TOKEN_TYPES content correspondence SemanticTokenTypes
const std::vector<std::string> SemanticTokensAdaptor::TOKEN_TYPES = { "comment", "keyword", "string", "number",
    "regexp", "operator", "namespace", "type", "struct", "class", "interface", "enum", "typeParameter", "function",
    "member", "property", "macro", "variable", "parameter", "label" };

// Second member of vector is currently zero, its reserved for tokenModifiers in case of future use
const std::map<HighlightKind, std::vector<int>> SemanticTokensAdaptor::HIGHLIGHT_TO_TOKEN_KIND_MAP = {
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::FILE_H, {static_cast<int>(SemanticTokenTypes::TYPE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::MODULE_H, {static_cast<int>(SemanticTokenTypes::TYPE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::NAMESPACE_H, {static_cast<int>(SemanticTokenTypes::NAMESPACE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::PACKAGE_H, {static_cast<int>(SemanticTokenTypes::MACRO_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::CLASS_H, {static_cast<int>(SemanticTokenTypes::CLASS_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::METHOD_H, {static_cast<int>(SemanticTokenTypes::FUNCTION_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::PROPERTY_H, {static_cast<int>(SemanticTokenTypes::PROPERTY_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::FIELD_H, {static_cast<int>(SemanticTokenTypes::VARIABLE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::CONSTRUCTOR_H, {static_cast<int>(SemanticTokenTypes::FUNCTION_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::ENUM_H, {static_cast<int>(SemanticTokenTypes::ENUM_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::INTERFACE_H, {static_cast<int>(SemanticTokenTypes::INTERFACE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::FUNCTION_H, {static_cast<int>(SemanticTokenTypes::FUNCTION_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::VARIABLE_H, {static_cast<int>(SemanticTokenTypes::VARIABLE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::CONSTANT_H, {static_cast<int>(SemanticTokenTypes::NUMBER_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::NUMBER_H, {static_cast<int>(SemanticTokenTypes::NUMBER_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::BOOLEAN_H, {static_cast<int>(SemanticTokenTypes::NUMBER_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::ARRAY_H, {static_cast<int>(SemanticTokenTypes::NUMBER_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::OBJECT_H, {static_cast<int>(SemanticTokenTypes::VARIABLE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::KEY_H, {static_cast<int>(SemanticTokenTypes::KEYWORD_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::MISSING_H, {static_cast<int>(SemanticTokenTypes::LABEL_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::ENUMMEMBER_H, {static_cast<int>(SemanticTokenTypes::VARIABLE_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::STRUCT_H, {static_cast<int>(SemanticTokenTypes::STRUCT_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::EVENT_H, {static_cast<int>(SemanticTokenTypes::LABEL_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::OPERATOR_H, {static_cast<int>(SemanticTokenTypes::OPERATOR_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::TYPEPARAMETER_H, {static_cast<int>(SemanticTokenTypes::TYPE_PARAMETER_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::COMMENT_H, {static_cast<int>(SemanticTokenTypes::COMMENT_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::RECORD_H, {static_cast<int>(SemanticTokenTypes::CLASS_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::TRAIT_H, {static_cast<int>(SemanticTokenTypes::CLASS_T), 0}),
    std::map<HighlightKind, std::vector<int>>::value_type(
        HighlightKind::INACTIVECODE_H, {static_cast<int>(SemanticTokenTypes::LABEL_T), 0}),
};

// Outer Interface
void SemanticTokensAdaptor::FindSemanticTokens(const ArkAST &ast, SemanticTokens &result, unsigned int fileID)
{
    // Store previous format results
    std::vector<SemanticHighlightToken> highlightVec;
    // Call original interface
    SemanticHighlightImpl::FindHighlightsTokens(ast, highlightVec, fileID);
    // Do the conversion
    FromHighlightToSemaTokens(highlightVec, result);
}

void SemanticTokensAdaptor::FromHighlightToSemaTokens(const std::vector<SemanticHighlightToken> &originVec,
    SemanticTokens &semaTokens)
{
    std::set<SemanticTokensFormat> semanticTokensSet;
    SemanticTokensFormat tempTokens = {};
    // Reorganize the vector and sort.
    for (auto item: originVec) {
        if (item.range.start.line != item.range.end.line) {
            // If the semantic token range more than one line, it should not be highlight
            continue;
        }
        tempTokens.startChar = item.range.start.column;
        tempTokens.line = item.range.end.line; // Start is also okay
        tempTokens.length = item.range.end.column - item.range.start.column;
        std::vector<int> retTemp = TokenKindConversion(item.kind);
        tempTokens.tokenType = retTemp[0];
        tempTokens.tokenTypeModifier = retTemp[1];
        (void)semanticTokensSet.insert(tempTokens);
    }
    // Prepare for the Reply: Increment Format
    ReadyForSemanticTokenMsg(semanticTokensSet, semaTokens);
}

void SemanticTokensAdaptor::ReadyForSemanticTokenMsg(const std::set<SemanticTokensFormat>& semaTokensRawSet,
    SemanticTokens& semaTokens)
{
    if (semaTokensRawSet.empty()) {
        return; // Edge condition
    }
    // We push in the first one without conversion.
    auto it = semaTokensRawSet.begin();
    semaTokens.data.push_back(it->line);
    semaTokens.data.push_back(it->startChar);
    semaTokens.data.push_back(it->length);
    semaTokens.data.push_back(it->tokenType);
    int preLine = it->line;
    int preStarChar = it->startChar;
    // For now we haven't use modifier, should consider bit operation while using.
    semaTokens.data.push_back(it->tokenTypeModifier);
    ++it;
    for (; it != semaTokensRawSet.end(); ++it) {
        if (it->line == preLine) {
            // Increment it!
            semaTokens.data.push_back(it->line - preLine);
            semaTokens.data.push_back(it->startChar - preStarChar);
            semaTokens.data.push_back(it->length);
            semaTokens.data.push_back(it->tokenType);
            // For now we haven't use modifier, should consider bit operation while using.
            semaTokens.data.push_back(it->tokenTypeModifier);
        } else {
            semaTokens.data.push_back(it->line - preLine);
            semaTokens.data.push_back(it->startChar);
            semaTokens.data.push_back(it->length);
            semaTokens.data.push_back(it->tokenType);
            semaTokens.data.push_back(it->tokenTypeModifier);
        }
        preLine = it->line;
        preStarChar = it->startChar;
    }
}

std::vector<int> SemanticTokensAdaptor::TokenKindConversion(const HighlightKind &highlighKind)
{
    std::vector<int> ret;
    auto kindIter
        = HIGHLIGHT_TO_TOKEN_KIND_MAP.find(highlighKind);
    if (kindIter != HIGHLIGHT_TO_TOKEN_KIND_MAP.end()) {
        ret = kindIter->second;
    } else {
        ret = { 0, 0 };
    }
    return ret;
}
}
