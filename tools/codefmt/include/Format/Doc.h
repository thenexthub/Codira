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

#ifndef CODEFMT_DOC_H
#define CODEFMT_DOC_H

#include <string>
#include <vector>

namespace Codira::Format {

struct FormattingOptions {
    int indentWidth{4};        // default indentation width
    int lineLength{120};       // default line width
    std::string newLine{"\n"}; // default newline character
    bool allowMultiLineMethodChain = false;
    int multipleLineMethodChainLevel = 5;
    bool multipleLineMethodChainOverLineLength = true;
};

struct FuncOptions {
    bool patternOrEnum;
    bool isLambda;
    bool isSpawn;
    bool isMultipleLineMacroExpendParam;
    bool isMethodChainning;
    bool isInsideBuildNode;

    explicit FuncOptions(bool patternOrEnum = false, bool isLambda = false, bool isSpawn = false,
        bool isMultipleLineMacroExpendParam = false, bool isMethodChainning = false, bool isInsideBuildNode = false)
        : patternOrEnum(patternOrEnum),
          isLambda(isLambda),
          isSpawn(isSpawn),
          isMultipleLineMacroExpendParam(isMultipleLineMacroExpendParam),
          isMethodChainning(isMethodChainning),
          isInsideBuildNode(isInsideBuildNode)
    {
    }
};

enum class DocType {
    FILE,
    CONCAT,
    GROUP,
    ARGS,
    FUNC_ARG,
    LINE_DOT,
    DOT,
    LAMBDA,
    LAMBDA_BODY,
    MEMBER_ACCESS,
    FILL,
    IF_BREAK,
    BREAK_PARENT,
    JOIN,
    LINE,
    SOFTLINE_WITH_SPACE,
    SOFTLINE,
    HARDLINE,
    LITERALLINE,
    LINE_SUFFIX,
    LINE_SUFFIX_BOUNDARY,
    INDENT,
    ALIGN,
    STRING,
    DOC_COMMENT,
    LINE_COMMENT,
    SUFFIX_COMMENT,
    SEPARATE,
    INVALID,
};

enum class Mode {
    MODE_FLAT,  // don't do line break, use space
    MODE_BREAK, // do line break as much as possible
    INVALID,
};

struct Doc {
    DocType type = DocType::INVALID;
    int indent{};
    std::string value;
    std::vector<Doc> members;
    Doc() = default;
    Doc(DocType ty, int ind, std::string val)
    {
        type = ty;
        indent = ind;
        value = std::move(val);
    }
    Doc(DocType ty, int ind, std::vector<Doc> mem)
    {
        type = ty;
        indent = ind;
        members = std::move(mem);
    }
};
} // namespace Codira::Format

#endif // CODEFMT_DOC_H
