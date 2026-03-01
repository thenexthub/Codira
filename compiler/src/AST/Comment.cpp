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
 * This file implements the Comment related structs.
 */
#include "Codira/AST/Comment.h"

#include "Codira/Basic/StringConvertor.h"

namespace Codira {
std::string AST::CommentGroups::ToString() const
{
    if (IsEmpty()) {
        return "{}";
    }
    std::string str{"{"};
    bool needComma = false;
    if (!leadingComments.empty()) {
        str += "\"leadingComments\":[";
        for (auto cg : leadingComments) {
            if (needComma) {
                str += ", ";
            }
            str += cg.ToString();
            needComma = true;
        }
        str += "]";
    }
    if (!innerComments.empty()) {
        if (needComma) {
            str += ", ";
        }
        needComma = false;
        str += "\"innerComments\":[";
        for (auto cg : innerComments) {
            if (needComma) {
                str += ",";
            }
            str += cg.ToString();
            needComma = true;
        }
        str += "]";
        needComma = true;
    }
    if (!trailingComments.empty()) {
        if (needComma) {
            str += ", ";
        }
        needComma = false;
        str += "\"trailingComments\":[";
        for (auto cg : trailingComments) {
            if (needComma) {
                str += ", ";
            }
            str += cg.ToString();
            needComma = true;
        }
        str += "]";
        needComma = true;
    }
    str += "}";
    return str;
}
std::string AST::CommentGroup::ToString() const
{
    if (IsEmpty()) {
        return "{\"cms\":[]}";
    }
    std::string str{"{\"cms\":["};
    bool needComma = false;
    for (auto c : cms) {
        if (needComma) {
            str += ", ";
        }
        str += c.ToString();
        needComma = true;
    }
    str += "]}";
    return str;
}
std::string AST::Comment::ToString() const
{
    std::string str;
    str += "{\"style\":";
    switch (style) {
        case CommentStyle::LEAD_LINE:
            str += "\"leadLine\"";
            break;
        case CommentStyle::TRAIL_CODE:
            str += "\"trailCode\"";
            break;
        case CommentStyle::OTHER:
            str += "\"other\"";
            break;
    }
    str += ", \"kind\":";
    switch (kind) {
        case CommentKind::LINE:
            str += "\"line\"";
            break;
        case CommentKind::BLOCK:
            str += "\"block\"";
            break;
        case CommentKind::DOCUMENT:
            str += "\"doc\"";
            break;
    }
    str += ", \"info\":\"" + StringConvertor::EscapeToJsonString(info.Value()) + "\"}";
    return str;
}
}
