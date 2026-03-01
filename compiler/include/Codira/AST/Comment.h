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

/**
 * @file
 *
 * This file declares cache for node.
 */
#ifndef CODIRA_AST_COMMENT_H
#define CODIRA_AST_COMMENT_H

#include <unordered_map>
#include <cstdint>
#include "Codira/Lex/Token.h"
namespace Codira::AST {

enum class CommentStyle : uint8_t {
    LEAD_LINE,
    TRAIL_CODE,
    OTHER,
};

enum class CommentKind : uint8_t {
    LINE,
    BLOCK,
    DOCUMENT, // block comment started with "/**" e.g. /**xxxx*/.  exclude: start with "/***", empty comment "/**/".
};

struct Comment {
    CommentStyle style;
    CommentKind kind;
    Token info;
    std::string ToString() const;
};

/// e.g.
/// // line 1
/// // line 2
/// main() { /*block 1*/ // line 3
///     // line 4
///     // line 6
/// return 0
/// }
// group 1: line 1, line 2, group 2: block 1, line 3, group 3: line 4, line6
struct CommentGroup {
    std::vector<Comment> cms;
    bool IsEmpty() const
    {
        return cms.empty();
    }
    std::string ToString() const;
};

///
/// Comments are classified into leadingComments, innerComments and trailingComments based on the location relationship
/// among nodes and comments, For details, see the description in AttachComment.cpp.
/// e.g.
/// /** c0 lead classDecl of class A */
/// class A { // c1 lead var decl of a
///     // c2 lead varDecl of a
///     var a = 1 // c3 trail varDecl of a
///     // c4 trail varDecl of a
/// } // c5 trail classDecl of A
/// // c6 lead funcDecl of foo
/// func foo(/* c7 inner funcParamList of foo */)
/// {
/// }
/// // c8 trail funcDecl of foo
///
/// main() {
///    0
/// }
///
struct CommentGroups {
    std::vector<CommentGroup> leadingComments;
    std::vector<CommentGroup> innerComments;
    std::vector<CommentGroup> trailingComments;
    bool IsEmpty() const
    {
        return leadingComments.empty() && innerComments.empty() && trailingComments.empty();
    }
    std::string ToString() const;
};

/**
 * all comment groups in the token stream and location-related information
 */
struct CommentGroupsLocInfo {
    std::vector<CommentGroup> commentGroups;
    // key: groupIndex value: preTokenIndex in tokenStream(ignore nl, semi, comment)
    std::unordered_map<size_t, size_t> cgPreInfo;
    // key: groupIndex value: followTokenIndex in tokenStream(ignore nl, comment, end)
    std::unordered_map<size_t, size_t> cgFollowInfo;
    const std::vector<Token>& tkStream;
};
}

#endif // CODIRA_AST_COMMENT_H
