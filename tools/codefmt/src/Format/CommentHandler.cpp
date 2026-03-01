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

#include "Format/CommentHandler.h"
#include "Format/OptionContext.h"
#include "Format/SimultaneousIterator.h"
#include "Codira/Basic/DiagnosticEngine.h"

#include <optional>

namespace Codira::Format {
using namespace Codira;
using namespace Codira::AST;

namespace {
const int MAX_LINEBREAKS = 2;
OptionContext& g_optionContext = OptionContext::GetInstance();
const int INDENT = g_optionContext.GetConfigOptions().indentWidth; // XXX: take from configuration

bool IsClosing(const Token &token)
{
    return token.kind == TokenKind::RCURL || token.kind == TokenKind::RPAREN || token.kind == TokenKind::RSQUARE;
}

bool IsMultilineComment(const Token &token)
{
    // Extract first two characters (start at position 0, max length 2)
    return token.Value().substr(0, 2) == "/*" && token.Value().find('\n') != std::string::npos;
}

struct InsertData {
    std::optional<Token> tokenBefore;
    std::optional<Token> tokenAfter;
    std::vector<Token> comments;

    InsertData(const std::optional<Token> tokenBefore, const std::optional<Token> tokenAfter,
        const std::vector<Token> comments)
        : tokenBefore(tokenBefore), tokenAfter(tokenAfter), comments(comments)
    {}
};

class CommentHandler : private SimultaneousIterator {
public:
    CommentHandler(const std::vector<Token> &originalTokens, const std::vector<Token> &outputTokens, SourceManager &sm)
        : SimultaneousIterator(originalTokens, outputTokens), sm(sm)
    {
        this->outputText = "";
    }

    std::string DoInsert()
    {
        if (originalTokens.empty()) {
            return outputText;
        }
        std::vector<Token> startOfFileComments = IterateAndCollectComments();
        if (!startOfFileComments.empty()) {
            auto firstNonCommentToken =
                originalIterator != originalTokens.end() ? std::optional(*originalIterator) : std::nullopt;
            InsertComments(InsertData(std::nullopt, firstNonCommentToken, startOfFileComments));
        }

        if (outputIterator == outputTokens.end()) {
            return outputText;
        }
        Position prevPosition = outputIterator->Begin();
        Position currentPosition = outputIterator->End();
        while (originalIterator != originalTokens.end() || outputIterator != outputTokens.end()) {
            auto commentsToInsertAfter = AdvanceAndComputeInsertData();
            if (!commentsToInsertAfter) {
                if (outputIterator != outputTokens.end()) {
                    currentPosition = outputIterator->End();
                } else if (prevPosition < currentPosition) {
                    InsertTextBetween(prevPosition, currentPosition);
                    prevPosition = currentPosition;
                }
                continue;
            }

            CheckCurrentPos(currentPosition);

            InsertTextBetween(prevPosition, currentPosition);
            InsertComments(*commentsToInsertAfter);

            if (outputIterator == outputTokens.end()) {
                continue;
            }

            prevPosition = outputIterator->Begin();
            currentPosition = outputIterator->End();

            if (std::prev(outputIterator)->kind == TokenKind::NL && outputText.back() == '\n') {
                Insert(' ', outputIterator->Begin().column - 1);
            } else if (std::prev(outputIterator)->kind != TokenKind::NL && outputText.back() == '\n') {
                Insert(' ', ComputeIndentFor(outputIterator, false));
            }
        }
        return outputText;
    }

private:
    SourceManager &sm;
    std::string outputText;

    void Insert(char character, int times)
    {
        for (int i = 0; i < times; ++i) {
            outputText.push_back(character);
        }
    }

    void InsertTextBetween(const Position &begin, const Position &end)
    {
        (void)outputText.append(sm.GetContentBetween(begin, end));
    }

    void CheckCurrentPos(Position& currentPosition)
    {
        if (std::prev(outputIterator)->kind == TokenKind::SEMI) {
            currentPosition = std::prev(outputIterator)->End();
        }

        if (std::prev(outputIterator)->kind == TokenKind::NL) {
            if (std::prev(std::prev(outputIterator))->kind == TokenKind::SEMI) {
                currentPosition = std::prev(std::prev(outputIterator))->End();
            }
        }

        if (std::prev(outputIterator)->kind == TokenKind::NL) {
            if (std::prev(std::prev(outputIterator))->kind == TokenKind::NL) {
                if (std::prev(std::prev(std::prev(outputIterator)))->kind == TokenKind::SEMI) {
                    currentPosition = std::prev(std::prev(std::prev(outputIterator)))->End();
                }
            }
        }
    }

    std::vector<Token>::const_iterator FindFirstTokenOnThisLine(std::vector<Token>::const_iterator token)
    {
        if (token == outputTokens.begin()) {
            return token;
        }
        for (;;) {
            auto prev = std::prev(token);
            if (prev == outputTokens.begin()) {
                return prev;
            }
            if (prev->kind == TokenKind::NL) {
                return token;
            }
            --token;
        }
    }

    /*
     * This find correct indentation when the formatting was split by eol comment.
     * When isComment is true, we're effectively trying to find for the token before this one.
     * The basic idea is when we're indenting the start of the line then the indent
     * is always the same as the formatted code.
     * If we need to split the line then the code would be indented one level.
     * Everything else is trying to add custom behaviour for common special cases so the formatting looks more natural.
     */
    int ComputeIndentFor(const std::vector<Token>::const_iterator tokenToIndent, bool isComment)
    {
        if (tokenToIndent == outputTokens.end()) {
            return 0;
        }
        auto firstOnThisLine = FindFirstTokenOnThisLine(tokenToIndent);
        auto sameIndent = (firstOnThisLine->Begin().column - 1);
        auto oneLevelIndented = sameIndent + INDENT;

        if (!isComment && tokenToIndent == firstOnThisLine) {
            return sameIndent;
        }

        auto isFirstOnThisLine = tokenToIndent == firstOnThisLine;
        auto isLastOnThisLine = std::next(tokenToIndent) == outputTokens.end() ||
            std::next(tokenToIndent)->kind == TokenKind::NL;

        auto isLastClosingBrace = isLastOnThisLine && IsClosing(*tokenToIndent);
        if (isComment && isLastClosingBrace) {
            return oneLevelIndented;
        }
        if (!isComment && isLastClosingBrace) {
            return sameIndent;
        }
        if (!isComment && tokenToIndent->kind == TokenKind::LCURL) {
            return sameIndent;
        }
        if (!isFirstOnThisLine) {
            // pattern matching 'case' always creats indented region
            if (firstOnThisLine->kind == TokenKind::CASE) {
                return oneLevelIndented;
            }
            // arrow in lambda or 'case' clauses can be formatted in various ways
            if (firstOnThisLine->kind == TokenKind::DOUBLE_ARROW ||
                std::prev(tokenToIndent)->kind == TokenKind::DOUBLE_ARROW ||
                tokenToIndent->kind == TokenKind::DOUBLE_ARROW) {
                return sameIndent;
            }
            return oneLevelIndented;
        }
        return sameIndent;
    }

    std::optional<InsertData> AdvanceAndComputeInsertData()
    {
        std::optional<Token> tokenBefore = std::nullopt;

        if (originalIterator != originalTokens.end()) {
            tokenBefore = std::optional(*originalIterator);
        }

        auto commentsBetween = Advance();
        if (commentsBetween.empty()) {
            return std::nullopt;
        }
        std::optional<Token> tokenAfter =
            (originalIterator != originalTokens.end()) ? std::optional(*originalIterator) : std::nullopt;

        return InsertData(tokenBefore, tokenAfter, commentsBetween);
    }

    void EnsureSpaceAfterStar(std::string &line, size_t starPosition)
    {
        auto firstNonStarSymbolPosition = line.find_first_not_of('*', starPosition);
        if (firstNonStarSymbolPosition == 0) {
            return;
        }
        if (firstNonStarSymbolPosition == std::string::npos || line.size() <= firstNonStarSymbolPosition) {
            return;
        }
        if (std::isspace(line[firstNonStarSymbolPosition]) || line[firstNonStarSymbolPosition] == '/') {
            return;
        }
        (void)line.insert(firstNonStarSymbolPosition, " ");
    }

    void FormatLineFromMultilineComment(std::string &line, int lineNumber, int linesCount, int indent)
    {
        if (lineNumber == 0) {
            Insert(' ', indent);
            EnsureSpaceAfterStar(line, 1);
            return;
        }
        int originalIndentation = static_cast<int>(line.find_first_not_of(' '));
        Utils::TrimString(line);
        if (line.empty()) {
            return;
        }
        auto startsWithStar = line.front() == '*';
        auto isSimpleLastLine = lineNumber == linesCount - 1 && line == "*/";
        int minIndentationFromFirstLine = isSimpleLastLine ? 1 : (startsWithStar ? 1 : 0);
        Insert(' ', startsWithStar ? indent + minIndentationFromFirstLine :
            std::max(originalIndentation, indent + minIndentationFromFirstLine));
        EnsureSpaceAfterStar(line, 0);
    }

    void InsertComment(const Token &token, int indent)
    {
        if (!IsMultilineComment(token)) {
            Insert(' ', indent);
            (void)outputText.append(token.Value());
            return;
        }

        auto lines = Utils::SplitLines(token.Value());
        for (size_t i = 0; i < lines.size(); ++i) {
            std::string line = lines[i];
            FormatLineFromMultilineComment(line, (int)i, (int)lines.size(), indent);
            (void)outputText.append(line);
            if (i != lines.size() - 1) {
                Insert('\n', 1);
            }
        }
    }

    bool IsDoubleSeparate()
    {
        if (outputIterator != outputTokens.begin() && std::prev(outputIterator)->kind == TokenKind::NL &&
            std::prev(outputIterator) != outputTokens.begin() &&
            prev(std::prev(outputIterator))->kind == TokenKind::NL) {
            return true;
        }
        return false;
    }

    void InsertComments(const InsertData &insertData)
    {
        auto &comments = insertData.comments;
        auto it = comments.begin();

        auto nextNonCommentTokenPosition =
            insertData.tokenAfter ? insertData.tokenAfter->Begin() : Position(comments.back().Begin().line + 1, 1);

        bool isReadyInsertNL = false;
        while (it != comments.end()) {
            auto token = *it;

            // manage whitespace characters before each of the comments
            Position prevTokenPos;
            if (it == comments.begin()) {
                prevTokenPos = insertData.tokenBefore ? insertData.tokenBefore->End() : Position(1, 1);
            } else {
                prevTokenPos = std::prev(it)->End();
            }

            auto lineBreaksBefore = token.Begin().line - prevTokenPos.line;
            Insert('\n', std::min(!isReadyInsertNL ? MAX_LINEBREAKS : MAX_LINEBREAKS - 1, lineBreaksBefore));
            if (lineBreaksBefore == 0 && insertData.tokenBefore) {
                Insert(' ', 1);
            }

            auto shouldAlignToNextNonCommentToken = lineBreaksBefore > 0;
            if (shouldAlignToNextNonCommentToken) {
                InsertComment(token, ComputeIndentFor(outputIterator, true));
            } else {
                InsertComment(token, 0);
            }

            if (lineBreaksBefore == 0 && IsDoubleSeparate()) {
                Insert('\n', 1);
                isReadyInsertNL = true;
            }

            ++it;
        }

        // manage whitespace characters after the last comment

        auto lineBreaksAfter = nextNonCommentTokenPosition.line - comments.back().End().line;
        if (lineBreaksAfter > 0 && IsMultilineComment(comments.back()) && insertData.tokenBefore) {
            // End() is incorrect for multiline comments, we can be more precise after parser is fixed
            lineBreaksAfter = 1;
        }

        Insert('\n', std::min(!isReadyInsertNL ? MAX_LINEBREAKS : MAX_LINEBREAKS - 1, lineBreaksAfter));
        if (lineBreaksAfter == 0) {
            Insert(' ', 1);
        }
    }
};
}

std::string InsertComments(const std::vector<Token> &originalTokens, const std::vector<Token> &formattedTokens,
    SourceManager &sm)
{
    auto commentHandler = CommentHandler(originalTokens, formattedTokens, sm);
    return commentHandler.DoInsert();
}
} // namespace Codira::Format
