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

#ifndef CODEFMT_ASTTOFORMATSOURCE_H
#define CODEFMT_ASTTOFORMATSOURCE_H

#include "Format/Doc.h"
#include "Format/DocProcessor/DocProcessor.h"
#include "Format/NodeFormatter/NodeFormatter.h"
#include "Codira/AST/ASTContext.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"
#include "Codira/Basic/Utils.h"
#include "Codira/Parse/Parser.h"
#include "Codira/Option/Option.h"

#include <limits>
#include <optional>

namespace Codira::Format {
static const Codira::Position INVALID_POSITION = Codira::Position{0, 0, 0};

struct Region {
    int startLine;
    int endLine;
    bool isWholeFile;

    Region() = default;
    Region(int start, int end, bool isWholeFile) noexcept : startLine(start), endLine(end), isWholeFile(isWholeFile) {}

    static const Region wholeFile;
};

struct RegionFormattingTracker {
    Codira::Position shouldFormatBegin;
    Codira::Position shouldFormatEnd;
    std::optional<Codira::Position> actuallyFormattedStart; // defines bounds of the top level ast nodes
    std::optional<Codira::Position> actuallyFormattedEnd;   // touched during fragment formatting
    std::optional<Codira::Position> cuttingPointOutsideRegionBegin;
    std::optional<Codira::Position> cuttingPointInsideRegionBegin;
    std::optional<Codira::Position> cuttingPointOutsideRegionEnd;
    std::optional<Codira::Position> cuttingPointInsideRegionEnd;

    RegionFormattingTracker() = default;

    explicit RegionFormattingTracker(const Region regionToFormat)
        : shouldFormatBegin(regionToFormat.startLine, 1),
          shouldFormatEnd(regionToFormat.endLine, std::numeric_limits<int>::max()),
          actuallyFormattedStart(std::nullopt),
          actuallyFormattedEnd(std::nullopt),
          cuttingPointOutsideRegionBegin(std::nullopt),
          cuttingPointInsideRegionBegin(std::nullopt),
          cuttingPointOutsideRegionEnd(std::nullopt),
          cuttingPointInsideRegionEnd(std::nullopt)
    {
    }

    void ProcessNodeFormatted(Ptr<Codira::AST::Node> node);
    bool IsInsideFormattedNode(Ptr<Codira::AST::Node> node) const;
    bool ShouldFormat(Ptr<Codira::AST::Node> node);
    std::optional<std::pair<Codira::Position, Codira::Position>> GetPreciseFragmentBounds(const SourceManager& sm);

private:
    void ProcessPotentialCuttingPoint(Codira::Position& pos);
};

class ASTToFormatSource {
public:
    RegionFormattingTracker& tracker;
    Codira::SourceManager& sm;

    ASTToFormatSource(RegionFormattingTracker& tracker, Codira::SourceManager& sm) : tracker(tracker), sm(sm) {}
    Doc ASTToDoc(Ptr<Codira::AST::Node> node, int level = 0, FuncOptions funcOptions = FuncOptions());
    std::string DocToString(Doc& doc);
    std::string DocToString(Doc& doc, int& pos, std::string& formatted);
    void AddAnnotations(Doc& doc, const std::vector<OwnedPtr<Codira::AST::Annotation>>& annotations, int level,
        bool changeLine = true);
    void AddGenericParams(Doc& doc, const Codira::AST::Generic& generic, int level);
    void AddGenericBound(Doc& doc, const Codira::AST::Generic& generic, int level);
    void AddBreakLineParam(
        Doc& doc, const Codira::AST::FuncParamList& funcParamList, int level, FuncOptions funcOptions);
    void AddMatchSelector(Doc& doc, const Codira::AST::MatchExpr& matchExpr, int level);
    void EditMacroStr(const Token& attr, std::string& macroStr, TokenKind& preTokenKind);
    bool WithoutSpace(TokenKind preTokenKind) const;
    bool IsMultipleLineArg(const std::vector<OwnedPtr<Codira::AST::FuncArg>>& args);
    bool IsMultipleLineCallExpr(const Codira::AST::CallExpr& callExpr) const;
    bool IsMultipleLineArrayLit(const int& rightSquarePosLine,
        const std::vector<OwnedPtr<Codira::AST::Expr>>& children) const;
    bool IsMultipleLineExpr(const std::vector<OwnedPtr<Codira::AST::Expr>>& children);
    bool IsMultipleLine(const OwnedPtr<Codira::AST::Expr>& expr);
    int DepthInMultipleMethodChain(const Codira::AST::MemberAccess& memberAccess);

    template <typename T> void MacroProcessor(const T& macro, Doc& doc, int level)
    {
        std::string macroStr;

        TokenKind preTokenKind = TokenKind::ILLEGAL;

        for (auto& attr : macro->invocation.attrs) {
            EditMacroStr(attr, macroStr, preTokenKind);
        }

        doc.members.emplace_back(DocType::STRING, level, macroStr);
    }

    template <typename T>
    void AnnotationProcessor(const T& macro, Doc& doc, int level, bool& isMultipleLine, AST::Annotation& annotation)
    {
        isMultipleLine = macro->invocation.rightSquarePos.line != macro->invocation.attrs.back().End().line;
        FuncOptions funcOptions;
        funcOptions.isInsideBuildNode = true;
        if (isMultipleLine) {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
            for (auto& arg : annotation.args) {
                doc.members.emplace_back(ASTToDoc(arg.get(), level + 1, funcOptions));
                if (arg->commaPos != INVALID_POSITION) {
                    doc.members.emplace_back(DocType::STRING, level + 1, ",");
                }
                if (arg != annotation.args.back()) {
                    doc.members.emplace_back(DocType::LINE, level + 1, "");
                }
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        } else {
            for (auto& arg : annotation.args) {
                doc.members.emplace_back(ASTToDoc(arg.get(), level, funcOptions));
                if (arg->commaPos != INVALID_POSITION) {
                    doc.members.emplace_back(DocType::STRING, level, ",");
                }
                if (arg != annotation.args.back()) {
                    doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
                }
            }
        }
    }

    template <typename T> void AddMacroExpandNode(Doc& doc, const T& macro, int level, bool patternOrEnum,
        bool isParam, FuncOptions& funcOptions)
    {
        doc.type = DocType::CONCAT;
        doc.indent = level;
        if (!macro->annotations.empty()) {
            AddAnnotations(doc, macro->annotations, level);
        }

        std::string compileTimeVisibleStr = macro->invocation.isCompileTimeVisible ? "!" : "";
        doc.members.emplace_back(DocType::STRING, level, "@" + compileTimeVisibleStr + macro->invocation.fullName);
        auto argStr = sm.GetContentBetween(macro->invocation.leftSquarePos.fileID, macro->invocation.leftSquarePos,
            macro->invocation.rightSquarePos + 1);
        DiagnosticEngine diag;
        Parser parser(argStr, diag, sm);
        auto annotation = MakeOwned<AST::Annotation>();
        parser.ParseAnnotationArguments(*annotation);

        bool hasInvalidExpr = false;
        if (parser.GetDiagnosticEngine().GetErrorCount() > 0) {
            hasInvalidExpr = true;
        }

        if (macro->invocation.leftSquarePos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, "[");
        }

        if (hasInvalidExpr) {
            MacroProcessor(macro, doc, level);
        }

        bool isMultipleLine = false;
        if (!annotation->args.empty() && !hasInvalidExpr) {
            AnnotationProcessor(macro, doc, level, isMultipleLine, *annotation);
        }
        if (macro->invocation.rightSquarePos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, "]");
        }

        if (macro->invocation.leftParenPos != INVALID_POSITION && macro->invocation.rightParenPos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level,
                sm.GetContentBetween(macro->invocation.leftParenPos.fileID, macro->invocation.leftParenPos,
                    macro->invocation.rightParenPos + 1));
        }

        if (macro->invocation.decl != nullptr) {
            if (!funcOptions.isMultipleLineMacroExpendParam &&
                macro->invocation.decl->astKind == AST::ASTKind::MACRO_EXPAND_PARAM) {
                funcOptions.isMultipleLineMacroExpendParam = true;
            }
            if (isParam && !isMultipleLine && !funcOptions.isMultipleLineMacroExpendParam) {
                doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level, "");
            } else {
                doc.members.emplace_back(DocType::LINE, level, "");
            }
        }
        funcOptions.patternOrEnum = patternOrEnum;
        doc.members.emplace_back(ASTToDoc(macro->invocation.decl.get(), level, funcOptions));
    };

    template <typename T> void AddEmptyBody(Doc& doc, const T&, int level, bool isSameLine = true)
    {
        doc.members.emplace_back(DocType::STRING, level, " {");
        if (!isSameLine) {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(DocType::STRING, level, "}");
    };

    template <typename T> void AddBodyMembers(Doc& doc, const std::vector<T>& members, int level)
    {
        int lastEndLine = -1;
        for (auto& n : members) {
            if (lastEndLine != -1) {
                if (n->begin.line > lastEndLine + 1) {
                    doc.members.emplace_back(DocType::SEPARATE, level, "");
                }
                doc.members.emplace_back(DocType::LINE, level, "");
            }
            doc.members.emplace_back(ASTToDoc(n.get(), level));
            lastEndLine = n->end.line;
        }
    };

    void AddModifier(Doc& doc, const std::set<Codira::AST::Modifier>& modifiers, int level);
    void AddModifier(Doc& doc, Codira::AST::Modifier& modifier, int level);

    template <typename T, typename... Args> void RegisterNode(AST::ASTKind kind, Args&&... args)
    {
        formatNodeMap[kind] = std::make_shared<T>(*this, std::forward<Args>(args)...);
    }

    template <typename T, typename... Args> void RegisterDocProcessors(DocType type, Args&&... args)
    {
        toStringMap[type] = std::make_shared<T>(*this, std::forward<Args>(args)...);
    }

private:
    std::map<AST::ASTKind, std::shared_ptr<NodeFormatter>> formatNodeMap;
    std::map<DocType, std::shared_ptr<DocProcessor>> toStringMap;
};
} // namespace Codira::Format

#endif // CODEFMT_ASTTOFORMATSOURCE_H
