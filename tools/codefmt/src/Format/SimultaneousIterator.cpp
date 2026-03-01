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

#include "Format/SimultaneousIterator.h"

namespace Codira::Format {

namespace {
bool IsComment(const Token &tok)
{
    return tok.kind == TokenKind::COMMENT;
}

bool IsCommentOrNL(const Token &tok)
{
    return tok.kind == TokenKind::COMMENT || tok.kind == TokenKind::NL;
}

bool IsModifier(const Token &tok)
{
    return tok.kind >= TokenKind::STATIC && tok.kind <= TokenKind::OPERATOR;
}
}

SimultaneousIterator::SimultaneousIterator(
    const std::vector<Token> &originalTokens, const std::vector<Token> &outputTokens)
    : originalTokens(originalTokens), outputTokens(outputTokens)
{
    this->originalIterator = this->originalTokens.begin();
    this->outputIterator = this->outputTokens.begin();
}

std::vector<Token> SimultaneousIterator::IterateAndCollectComments()
{
    std::vector<Token> comments;
    while (originalIterator != originalTokens.end() && IsCommentOrNL(*originalIterator)) {
        if (IsComment(*originalIterator)) {
            comments.push_back(*originalIterator);
        }
        ++originalIterator;
    }
    return comments;
}

std::vector<Token> SimultaneousIterator::Advance()
{
    std::vector<Token> commentsBetween;
    if (originalIterator != originalTokens.end()) {
        ++originalIterator;
        while (originalIterator != originalTokens.end() && originalIterator->kind == TokenKind::SEMI) {
            ++originalIterator;
        }
        commentsBetween = IterateAndCollectComments();
    }

    if (outputIterator != outputTokens.end()) {
        ++outputIterator;

        while (outputIterator != outputTokens.end() &&
            (IsCommentOrNL(*outputIterator) || outputIterator->kind == TokenKind::SEMI)) {
            ++outputIterator;
        }
    }
    auto recovered = RecoverOnMismatch();
    if (recovered.has_value()) {
        auto commentsSkippedWhileRecovering = recovered.value();
        (void)commentsBetween.insert(commentsBetween.end(),
            commentsSkippedWhileRecovering.begin(), commentsSkippedWhileRecovering.end());
    }

    return commentsBetween;
}

std::optional<std::vector<Token>> SimultaneousIterator::RecoverOnMismatch()
{
    if (originalIterator == originalTokens.end() || outputIterator == outputTokens.end()) {
        return std::nullopt;
    }
    if (originalIterator->Value() == outputIterator->Value()) {
        return std::nullopt;
    }

    if (IsModifier(*originalIterator) && IsModifier(*outputIterator)) {
        // formatter can reorder modifiers
        return std::nullopt;
    }

    // try to recover to next non-whitespace token
    // first try to move original iterator forward
    // because we could delete tokens such as semicolon
    // so the output iterator would be ahead of original
    std::vector<Token> skippedComments;
    auto originalRecoverCandidate = std::next(originalIterator);
    while (originalRecoverCandidate != originalTokens.end() && IsCommentOrNL(*originalRecoverCandidate)) {
        if (IsComment(*originalRecoverCandidate)) {
            skippedComments.push_back(*originalRecoverCandidate);
        }
        ++originalRecoverCandidate;
    }
    if (originalRecoverCandidate != originalTokens.end() &&
        originalRecoverCandidate->Value() == outputIterator->Value()) {
        originalIterator = originalRecoverCandidate;
        return skippedComments;
    }

    auto outputRecoverCandidate = std::next(outputIterator);
    while (outputRecoverCandidate != outputTokens.end() && IsCommentOrNL(*outputRecoverCandidate)) {
        ++outputRecoverCandidate;
    }
    if (outputRecoverCandidate != outputTokens.end() &&
        outputRecoverCandidate->Value() == originalIterator->Value()) {
        outputIterator = outputRecoverCandidate;
        return std::nullopt;
    }

    return std::nullopt;
}

std::optional<std::pair<std::vector<Token>, std::vector<Token>>> ExtractTokensBetweenPositions(
    const std::vector<Token> &originalTokens, const std::vector<Token> &formattedTokens,
    const Codira::Position &startPosition, const Codira::Position &endPosition
    )
{
    auto simIter = SimultaneousIterator(originalTokens, formattedTokens);
    std::vector<Token>::const_iterator originalTokensSubsetFirst;
    std::vector<Token>::const_iterator outputTokensSubsetFirst;
    (void)simIter.IterateAndCollectComments(); // skip comments at the begining of original text
    while (true) {
        if (simIter.originalIterator == simIter.originalTokens.end() ||
            simIter.outputIterator == simIter.outputTokens.end() || simIter.originalIterator->Begin() > startPosition) {
            return std::nullopt;
        }
        if (simIter.originalIterator->Begin() == startPosition) {
            originalTokensSubsetFirst = simIter.originalIterator;
            outputTokensSubsetFirst = simIter.outputIterator;
            break;
        }
        if (simIter.originalIterator->End() == startPosition) {
            (void)simIter.Advance();
            originalTokensSubsetFirst = simIter.originalIterator;
            outputTokensSubsetFirst = simIter.outputIterator;
            break;
        }
        (void)simIter.Advance();
    }
    std::vector<Token>::const_iterator originalTokensSubsetEnd;
    std::vector<Token>::const_iterator outputTokensSubsetEnd;
    while (true) {
        if (simIter.originalIterator == simIter.originalTokens.end() ||
         simIter.outputIterator == simIter.outputTokens.end() || simIter.originalIterator->Begin() > endPosition) {
            return std::nullopt;
        }
        if (simIter.originalIterator->Begin() == endPosition) {
            originalTokensSubsetEnd = simIter.originalIterator;
            outputTokensSubsetEnd = simIter.outputIterator;
            break;
        }
        if (simIter.originalIterator->End() == endPosition) {
            originalTokensSubsetEnd = std::next(simIter.originalIterator);
            outputTokensSubsetEnd = std::next(simIter.outputIterator);
            break;
        }
        (void)simIter.Advance();
    }
    auto origFragment = std::vector<Token>(originalTokensSubsetFirst, originalTokensSubsetEnd);
    auto outputFragment = std::vector<Token>(outputTokensSubsetFirst, outputTokensSubsetEnd);
    while (origFragment.back().kind == TokenKind::NL) {
        origFragment.pop_back();
    }
    while (outputFragment.back().kind == TokenKind::NL) {
        outputFragment.pop_back();
    }
    return std::make_pair(std::move(origFragment), std::move(outputFragment));
}

} // namespace Codira::Format
