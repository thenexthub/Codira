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

#ifndef CODEFMT_SIMULTANEOUSITERATOR_H
#define CODEFMT_SIMULTANEOUSITERATOR_H

#include "Codira/Lex/Token.h"
#include "Codira/Basic/SourceManager.h"

#include <string>
#include <vector>
#include <optional>

namespace Codira::Format {

// iterates over two tokenstreams simulteneously
// originalTokens correspond to text before formatting
// outputTokens correspond to text after formatting
// both token streams should correspond to the same 'fragment' of text
// ignores new line tokens
// allows to iterate over matching pairs of tokens
// can collect comments from original tokens stream
class SimultaneousIterator {
public:
    SimultaneousIterator(const std::vector<Token> &originalTokens, const std::vector<Token> &outputTokens);
    // originalIterator skips comments and nl and is set at next non-comment token
    // returns a vector of skipped comment tokens
    std::vector<Token> IterateAndCollectComments();
    // advances both iterators (skipping comments and nl) and sets both to next matching pair of tokens,
    // returns a vector of skipped comment tokens
    std::vector<Token> Advance();

    const std::vector<Token> &originalTokens;
    const std::vector<Token> &outputTokens;
    std::vector<Token>::const_iterator originalIterator;
    std::vector<Token>::const_iterator outputIterator;

private:
    std::optional<std::vector<Token>> RecoverOnMismatch();
};

// find subset of original tokens between [startPosition, endPosition] and corresponding subset of output tokens
std::optional<std::pair<std::vector<Token>, std::vector<Token>>> ExtractTokensBetweenPositions(
    const std::vector<Token> &originalTokens, const std::vector<Token> &formattedTokens,
    const Codira::Position &startPosition, const Codira::Position &endPosition
);

} // namespace Codira::Format

#endif // CODEFMT_SIMULTANEOUSITERATOR_H
