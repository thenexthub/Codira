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

#include "Format/ASTToFormatSource.h"
#include "Format/SimultaneousIterator.h"
#include "Codira/Lex/Lexer.h"

#include <algorithm>
#include <limits>

using namespace Codira;
using namespace Codira::AST;
using namespace Codira::Format;

namespace Codira::Format {
const Region Region::wholeFile = Region(1, std::numeric_limits<int>::max(), true);
}

namespace {
const int MIN_MUL_MEMBERS = 2;
// this is a workaround for parser treating annotation and modifiers
// as if they're outside declarations [begin, end] range
Codira::Position GetBegin(Ptr<Node> n)
{
    auto begin = n->begin;
    if (n->IsDecl()) {
        Ptr<Decl> decl = StaticAs<ASTKind::DECL>(n);
        for (auto& a : decl->annotations) {
            if (a->begin < begin) {
                begin = a->begin;
            }
        }

        for (const auto& m : decl->modifiers) {
            if (m.begin < begin) {
                begin = m.begin;
            }
        }
    }
    return begin;
}

inline Codira::Position GetEnd(Ptr<Node> n)
{
    return n->end;
}
} // namespace

/*
Finding 4 positions:
cuttingPointOutsideRegionBegin < shouldFormatBegin <= cuttingPointInsideRegionBegin
    < cuttingPointInsideRegionEnd <= shouldFormatEnd <= cuttingPointOutsideRegionEnd

where cutting points are closest to shouldFormatBegin and End respectively.
Potential cutting points are begin or end positions of any ast node processed during formatting.
The goal is to choose smallest code fragment that includes given region.
*/
void RegionFormattingTracker::ProcessPotentialCuttingPoint(Codira::Position& pos)
{
    if (pos < shouldFormatBegin && (!cuttingPointOutsideRegionBegin || cuttingPointOutsideRegionBegin.value() < pos)) {
        cuttingPointOutsideRegionBegin = pos;
    }
    if ((pos >= shouldFormatBegin && pos < shouldFormatEnd) &&
        (!cuttingPointInsideRegionBegin || cuttingPointInsideRegionBegin.value() > pos)) {
        cuttingPointInsideRegionBegin = pos;
    }
    if ((pos > shouldFormatBegin && pos <= shouldFormatEnd) &&
        (!cuttingPointInsideRegionEnd || cuttingPointInsideRegionEnd.value() < pos)) {
        cuttingPointInsideRegionEnd = pos;
    }
    if ((pos > shouldFormatEnd) && (!cuttingPointOutsideRegionEnd || cuttingPointOutsideRegionEnd.value() < pos)) {
        cuttingPointOutsideRegionEnd = pos;
    }
}

/*
 Choose fragment bounds based on data stored during ast traversal.
 Example:
 Supposed we want to format 'let x = 3' line:

<1>func f() {
    let y = 5<2>
    <3>let x = 3<4>
    <5>let z = 6
}<6>

We can cut the fragment out at any points marksed as <number>. We choose those that make the fragment smallest.
*/
std::optional<std::pair<Codira::Position, Codira::Position>> RegionFormattingTracker::GetPreciseFragmentBounds(
    const SourceManager& sm)
{
    std::optional<Codira::Position> preciseFragmentBegin = std::nullopt;
    // <1>
    if (actuallyFormattedStart) {
        preciseFragmentBegin = actuallyFormattedStart.value();
    }
    // <2>
    if (cuttingPointOutsideRegionBegin) {
        preciseFragmentBegin = cuttingPointOutsideRegionBegin.value();
    }
    // <3>, we can only cut here if there is no code before it on that line
    if (cuttingPointInsideRegionBegin) {
        auto point = cuttingPointInsideRegionBegin.value();
        auto prefix =
            sm.GetContentBetween(Position(point.fileID, shouldFormatBegin.line, shouldFormatBegin.column), point);
        if (std::all_of(prefix.begin(), prefix.end(), isspace)) {
            preciseFragmentBegin = cuttingPointInsideRegionBegin.value();
        }
    }

    std::optional<Codira::Position> preciseFragmentEnd = std::nullopt;
    // <6>
    if (actuallyFormattedEnd) {
        preciseFragmentEnd = actuallyFormattedEnd.value();
    }
    // <5>
    if (cuttingPointOutsideRegionEnd) {
        preciseFragmentEnd = cuttingPointOutsideRegionEnd.value();
    }
    // <4>, we can only cut here if there is no code after it on that line
    if (cuttingPointInsideRegionEnd) {
        auto point = cuttingPointInsideRegionEnd.value();
        auto suffix =
            sm.GetContentBetween(point, Position(point.fileID, shouldFormatBegin.line, shouldFormatBegin.column));
        if (std::all_of(suffix.begin(), suffix.end(), isspace)) {
            preciseFragmentEnd = cuttingPointInsideRegionEnd.value();
        }
    }

    if (!preciseFragmentBegin || !preciseFragmentEnd) {
        return std::nullopt;
    }
    return std::make_pair(preciseFragmentBegin.value(), preciseFragmentEnd.value());
}

bool RegionFormattingTracker::IsInsideFormattedNode(Ptr<Codira::AST::Node> node) const
{
    return actuallyFormattedStart && actuallyFormattedEnd &&
        (GetBegin(node) >= actuallyFormattedStart && GetEnd(node) <= actuallyFormattedEnd);
}

bool RegionFormattingTracker::ShouldFormat(Ptr<Codira::AST::Node> node)
{
    auto shouldFormat = !(shouldFormatBegin > GetEnd(node) || shouldFormatEnd < GetBegin(node));
    // extend formatting range for nodes that can't be properly formatted partially
    if (shouldFormat && node->astKind == ASTKind::BINARY_EXPR) {
        shouldFormatBegin = std::min(GetBegin(node), shouldFormatBegin);
        shouldFormatEnd = std::max(GetEnd(node), shouldFormatEnd);
    }
    if (shouldFormat && node->astKind == ASTKind::MACRO_EXPAND_DECL) {
        shouldFormatBegin = std::min(GetBegin(node), shouldFormatBegin);
        shouldFormatEnd = std::max(GetEnd(node), shouldFormatEnd);
    }

    return shouldFormat;
}

void RegionFormattingTracker::ProcessNodeFormatted(Ptr<Codira::AST::Node> node)
{
    auto begin = GetBegin(node);
    auto end = GetEnd(node);
    if (!actuallyFormattedStart || actuallyFormattedStart.value() > begin) {
        actuallyFormattedStart = begin;
    }
    if (!actuallyFormattedEnd || actuallyFormattedEnd.value() < end) {
        actuallyFormattedEnd = end;
    }

    ProcessPotentialCuttingPoint(begin);
    ProcessPotentialCuttingPoint(end);
}

Doc ASTToFormatSource::ASTToDoc(Ptr<Codira::AST::Node> node, int level, FuncOptions funcOptions)
{
    Doc doc;
    if (node == nullptr) {
        doc.type = DocType::INVALID;
        return doc;
    }
    if (funcOptions.isInsideBuildNode) {
        if (formatNodeMap.find(node->astKind) != formatNodeMap.end()) {
            formatNodeMap[node->astKind]->ASTToDoc(doc, node, level, funcOptions);
        }
        return doc;
    }
    auto shouldFormat = tracker.ShouldFormat(node);
    auto insideFormattedNode = tracker.IsInsideFormattedNode(node);
    if (!shouldFormat && !insideFormattedNode) {
        doc.type = DocType::INVALID;
        doc.value = "";
        return doc;
    }
    if (node->astKind != ASTKind::FILE) {
        tracker.ProcessNodeFormatted(node);
    }
    if (!shouldFormat && insideFormattedNode) {
        doc.type = DocType::STRING;
        doc.value = sm.GetContentBetween(GetBegin(node), GetEnd(node));
        return doc;
    }
    if (formatNodeMap.find(node->astKind) != formatNodeMap.end()) {
        formatNodeMap[node->astKind]->ASTToDoc(doc, node, level, funcOptions);
    }
    return doc;
}

void ASTToFormatSource::AddBreakLineParam(
    Doc& doc, const Codira::AST::FuncParamList& funcParamList, int level, FuncOptions funcOptions)
{
    for (auto it = funcParamList.params.begin(); it != funcParamList.params.end(); ++it) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        doc.members.emplace_back(ASTToDoc(it->get(), level + 1, funcOptions));
        if (*it == funcParamList.params.back()) {
            break;
        }
        doc.members.emplace_back(DocType::STRING, level + 1, ",");
    }
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, ")");
}

void ASTToFormatSource::AddGenericParams(Doc& doc, const Codira::AST::Generic& generic, int level)
{
    if (generic.typeParameters.empty()) {
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, "<");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& typeParam : generic.typeParameters) {
        auto typeParamDoc = ASTToDoc(typeParam.get(), level + 1);
        if (typeParam->commaPos != INVALID_POSITION) {
            typeParamDoc.value += ",";
        }
        group.members.emplace_back(typeParamDoc);
        if (typeParam != generic.typeParameters.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, " ");
        }
    }
    doc.members.emplace_back(group);
    doc.members.emplace_back(DocType::STRING, level, ">");
}

void ASTToFormatSource::AddGenericBound(Doc& doc, const Codira::AST::Generic& generic, int level)
{
    if (generic.genericConstraints.empty()) {
        return;
    }
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& genericCons : generic.genericConstraints) {
        group.members.emplace_back(ASTToDoc(genericCons.get(), level + 1));
        if (genericCons != generic.genericConstraints.back()) {
            group.members.emplace_back(DocType::STRING, level + 1, ",");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}

void ASTToFormatSource::AddMatchSelector(Doc& doc, const Codira::AST::MatchExpr& matchExpr, int level)
{
    if (matchExpr.selector) {
        // selector expr
        if (matchExpr.leftParenPos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, "(");
        }
        doc.members.emplace_back(ASTToDoc(matchExpr.selector.get(), level));
        if (matchExpr.rightParenPos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, ")");
        }
        doc.members.emplace_back(DocType::STRING, level, " {");
    } else {
        doc.members.emplace_back(DocType::STRING, level, "{");
    }
}

void ASTToFormatSource::AddAnnotations(
    Doc& doc, const std::vector<OwnedPtr<Codira::AST::Annotation>>& annotations, int level, bool changeLine)
{
    for (auto& annotation : annotations) {
        doc.members.emplace_back(ASTToDoc(annotation.get(), level));
        if (changeLine) {
            doc.members.emplace_back(DocType::LINE, level, "");
        } else {
            doc.members.emplace_back(DocType::STRING, level, " ");
        }
    }
}

std::string ASTToFormatSource::DocToString(Doc& doc)
{
    std::vector<std::pair<Doc, Mode>> leftCmd;
    std::string formatted;
    int pos = 0;
    leftCmd.emplace_back(std::pair<Doc, Mode>(doc, Mode::MODE_FLAT));
    while (!leftCmd.empty()) {
        std::pair<Doc, Mode> current = leftCmd.back();
        leftCmd.pop_back();
        if (toStringMap.find(current.first.type) != toStringMap.end()) {
            toStringMap[current.first.type]->DocToString(formatted, pos, current, leftCmd);
        }
    }
    return formatted;
}

std::string ASTToFormatSource::DocToString(Doc& doc, int& pos, std::string& formatted)
{
    std::vector<std::pair<Doc, Mode>> leftCmd;
    leftCmd.emplace_back(std::pair<Doc, Mode>(doc, Mode::MODE_FLAT));
    while (!leftCmd.empty()) {
        std::pair<Doc, Mode> current = leftCmd.back();
        leftCmd.pop_back();
        if (toStringMap.find(current.first.type) != toStringMap.end()) {
            toStringMap[current.first.type]->DocToString(formatted, pos, current, leftCmd);
        }
    }
    return formatted;
}

void ASTToFormatSource::AddModifier(Doc& doc, const std::set<Codira::AST::Modifier>& modifiers, int level)
{
    static std::vector<std::set<TokenKind>> MODIFIER_PRIORITY = {
        {TokenKind::COMMON, TokenKind::PLATFORM},
        {TokenKind::PUBLIC, TokenKind::PRIVATE, TokenKind::INTERNAL, TokenKind::PROTECTED},
        {TokenKind::OPEN, TokenKind::SEALED, TokenKind::STATIC},
        {TokenKind::ABSTRACT},
        {TokenKind::OVERRIDE, TokenKind::REDEF},
        {TokenKind::UNSAFE, TokenKind::FOREIGN},
        {TokenKind::CONST, TokenKind::MUT},
        {TokenKind::OPERATOR}
    };

    for (auto priority : MODIFIER_PRIORITY) {
        for (auto modifier : modifiers) {
            if (priority.find(modifier.modifier) != priority.end()) {
                AddModifier(doc, modifier, level);
            }
        }
    }
}

void ASTToFormatSource::AddModifier(Doc& doc, Codira::AST::Modifier& modifier, int level)
{
    doc.members.emplace_back(ASTToDoc(&modifier, level));
    doc.members.emplace_back(DocType::STRING, level, " ");
}

void ASTToFormatSource::EditMacroStr(const Codira::Token& attr, std::string& macroStr, TokenKind& preTokenKind)
{
    const std::vector<TokenKind> TOKEN_KIND_WITHOUT_SPACE{TokenKind::LPAREN, TokenKind::RPAREN, TokenKind::COMMA,
        TokenKind::COLON, TokenKind::NOT, TokenKind::LT, TokenKind::GT, TokenKind::DOT};

    std::string quote = attr.isSingleQuote ? "'" : "\"";
    if (attr.kind == TokenKind::STRING_LITERAL) {
        auto iter = std::find(TOKEN_KIND_WITHOUT_SPACE.begin(), TOKEN_KIND_WITHOUT_SPACE.end(), attr.kind);
        if (iter == TOKEN_KIND_WITHOUT_SPACE.end() && WithoutSpace(preTokenKind)) {
            macroStr += " ";
        }
        preTokenKind = attr.kind;
        macroStr += quote + attr.Value() + quote;
    } else if (attr.kind == TokenKind::JSTRING_LITERAL) {
        auto iter = std::find(TOKEN_KIND_WITHOUT_SPACE.begin(), TOKEN_KIND_WITHOUT_SPACE.end(), attr.kind);
        if (iter == TOKEN_KIND_WITHOUT_SPACE.end() && WithoutSpace(preTokenKind)) {
            macroStr += " ";
        }
        preTokenKind = attr.kind;
        macroStr += "J" + quote + attr.Value() + quote;
    } else if (attr.kind == TokenKind::MULTILINE_RAW_STRING) {
        for (unsigned i = 0; i < attr.delimiterNum; ++i) {
            macroStr += "#";
        }
        preTokenKind = attr.kind;
        macroStr += "\"";
        macroStr += attr.Value();
        macroStr += "\"";
        for (unsigned i = 0; i < attr.delimiterNum; ++i) {
            macroStr += "#";
        }
    } else if (attr.kind == TokenKind::MULTILINE_STRING) {
        preTokenKind = attr.kind;
        macroStr += "\"\"\"\n";
        macroStr += attr.Value();
        macroStr += R"(""")";
    } else if (attr.kind == TokenKind::RUNE_LITERAL) {
        auto iter = std::find(TOKEN_KIND_WITHOUT_SPACE.begin(), TOKEN_KIND_WITHOUT_SPACE.end(), attr.kind);
        if (iter == TOKEN_KIND_WITHOUT_SPACE.end() && WithoutSpace(preTokenKind)) {
            macroStr += " ";
        }
        preTokenKind = attr.kind;
        std::string runeSymbol = "r";
        macroStr += runeSymbol + "\'" + attr.Value() + "\'";
    } else {
        auto iter = std::find(TOKEN_KIND_WITHOUT_SPACE.begin(), TOKEN_KIND_WITHOUT_SPACE.end(), attr.kind);
        if (preTokenKind == TokenKind::COMMA ||
            (iter == TOKEN_KIND_WITHOUT_SPACE.end() && WithoutSpace(preTokenKind))) {
            macroStr += " ";
        }
        preTokenKind = attr.kind;
        macroStr += attr.Value();
    }
}

bool ASTToFormatSource::WithoutSpace(TokenKind preTokenKind) const
{
    return preTokenKind != TokenKind::ILLEGAL && preTokenKind != TokenKind::LPAREN && preTokenKind != TokenKind::LT &&
        preTokenKind != TokenKind::DOT;
}

bool ASTToFormatSource::IsMultipleLineArg(const std::vector<OwnedPtr<Codira::AST::FuncArg>>& args)
{
    for (auto& arg : args) {
        auto isMulti = IsMultipleLine(arg->expr);
        if (isMulti) {
            return isMulti;
        }
    }
    return false;
}

bool ASTToFormatSource::IsMultipleLineExpr(const std::vector<OwnedPtr<Expr>>& children)
{
    for (auto& child : children) {
        auto isMulti = IsMultipleLine(child);
        if (isMulti) {
            return isMulti;
        }
    }
    return false;
}

bool ASTToFormatSource::IsMultipleLine(const OwnedPtr<Codira::AST::Expr>& expr)
{
    switch (expr->astKind) {
        case ASTKind::LAMBDA_EXPR: {
            auto argLamda = As<ASTKind::LAMBDA_EXPR>(expr.get());
            if (argLamda == nullptr || argLamda->funcBody == nullptr || argLamda->funcBody->body == nullptr ||
                argLamda->funcBody->body->body.size() <= 1) {
                return false;
            } else {
                return true;
            }
        }
        case ASTKind::CALL_EXPR: {
            auto argCallExpr = As<ASTKind::CALL_EXPR>(expr.get());
            if (argCallExpr == nullptr) {
                return false;
            }
            return IsMultipleLineCallExpr(*argCallExpr) || IsMultipleLineArg(argCallExpr->args);
        }
        case ASTKind::ARRAY_LIT: {
            auto argArrayLit = As<ASTKind::ARRAY_LIT>(expr.get());
            if (argArrayLit == nullptr) {
                return false;
            }
            return IsMultipleLineArrayLit(argArrayLit->rightSquarePos.line, argArrayLit->children)
                || IsMultipleLineExpr(argArrayLit->children);
        }
        case ASTKind::TRAIL_CLOSURE_EXPR: {
            auto argTrailClosureExpr = As<ASTKind::TRAIL_CLOSURE_EXPR>(expr.get());
            if (argTrailClosureExpr == nullptr || argTrailClosureExpr->lambda == nullptr ||
                argTrailClosureExpr->lambda->funcBody == nullptr ||
                argTrailClosureExpr->lambda->funcBody->body == nullptr ||
                argTrailClosureExpr->lambda->funcBody->body->body.size() <= 1) {
                return false;
            } else {
                return true;
            }
        }
        default:
            break;
    }
    return false;
}

bool ASTToFormatSource::IsMultipleLineCallExpr(const Codira::AST::CallExpr& callExpr) const
{
    if (callExpr.args.size() < MIN_MUL_MEMBERS) {
        return false;
    }

    if (callExpr.rightParenPos.line == callExpr.args.back()->end.line) {
        return false;
    }

    return true;
}

bool ASTToFormatSource::IsMultipleLineArrayLit(const int& rightSquarePosLine,
    const std::vector<OwnedPtr<Expr>>& children) const
{
    if (children.size() < MIN_MUL_MEMBERS) {
        return false;
    }

    if (rightSquarePosLine == children.back()->end.line) {
        return false;
    }
    return true;
}

int ASTToFormatSource::DepthInMultipleMethodChain(const MemberAccess& memberAccess)
{
    if (memberAccess.baseExpr->astKind == ASTKind::CALL_EXPR) {
        auto ce = As<ASTKind::CALL_EXPR>(memberAccess.baseExpr.get());
        if (ce->baseFunc && ce->baseFunc->astKind == ASTKind::MEMBER_ACCESS) {
            auto ma = As<ASTKind::MEMBER_ACCESS>(ce->baseFunc.get());
            return 1 + DepthInMultipleMethodChain(*ma);
        } else {
            return 1;
        }
    } else if (memberAccess.baseExpr->astKind == ASTKind::SUBSCRIPT_EXPR) {
        auto pSubscriptExpr = As<ASTKind::SUBSCRIPT_EXPR>(memberAccess.baseExpr.get());
        if (pSubscriptExpr->baseExpr && pSubscriptExpr->baseExpr->astKind == ASTKind::MEMBER_ACCESS) {
            auto ma = As<ASTKind::MEMBER_ACCESS>(pSubscriptExpr->baseExpr.get());
            return 1 + DepthInMultipleMethodChain(*ma);
        } else {
            return 1;
        }
    } else if (memberAccess.baseExpr->astKind == ASTKind::MEMBER_ACCESS) {
        auto ma = As<ASTKind::MEMBER_ACCESS>(memberAccess.baseExpr.get());
        return 1 + DepthInMultipleMethodChain(*ma);
    } else {
        return 1;
    }
}
