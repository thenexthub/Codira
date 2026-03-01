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

#ifndef TAINT_DATA_H
#define TAINT_DATA_H

#include "common/CommonData.h"

namespace Codira::CodeCheck::TaintData {
static const int32_t ALL_ARGS = -2;
static const int32_t RET_VAL = -1;
static const int32_t ARGS_MIN_IDX = 1;

enum class IsTaint {
    IS_TAINT,
    NOT_TAINT,
    NOT_SURE,
    NOT_CHANGE_OR_TAINT
};

struct PropagateInfo {
    int32_t src;
    int32_t dst;
    IsTaint isTaint;
    PropagateInfo(int32_t src, int32_t dst, IsTaint isTaint) : src(src), dst(dst), isTaint(isTaint) {}
};

static const std::vector<std::pair<AstFuncInfo, int32_t>> taintSource = {
    { AstFuncInfo("build", "HttpResponseBuilder", {"HttpResponseBuilder"}, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("build", "HttpRequestBuilder", {"HttpRequestBuilder"}, "HttpRequest", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("send", "Client", { "Client", "Request" }, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("get", "Client", { "Client", "String" }, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("head", "Client", { "Client", "String" }, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("post", "Client", { "Client", "String", "String", "InputStream" }, "Response", "stdx.net.http"),
        RET_VAL },
    { AstFuncInfo("postForm", "Client", { "Client", "String", "Form" }, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("postJson", "Client", { "Client", "String", "JsonValue" },
         "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("put", "Client", { "Client", "String", "String", "InputStream" }, "HttpResponse", "stdx.net.http"),
        RET_VAL },
    { AstFuncInfo("putForm", "Client", { "Client", "String", "Form" }, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("putForm", "Client", { "Client", "String", "JsonValue" }, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("delete", "Client", { "Client", "String", "String", "InputStream" }, "HttpResponse", "stdx.net.http"),
        RET_VAL },
    { AstFuncInfo("deleteForm", "Client", { "Client", "String", "Form" }, "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("deleteJson", "Client", { "Client", "String", "JsonValue" },
         "HttpResponse", "stdx.net.http"), RET_VAL },
    { AstFuncInfo("read", "TcpSocket", { "TcpSocket", "Array", NOT_CARE }, "", "std.net"), 2 },
    /* console */
    { AstFuncInfo("read", "ConsoleReader", {"ConsoleReader"}, "Option", "std.console"), RET_VAL },
    { AstFuncInfo("readln", "ConsoleReader", {"ConsoleReader"}, "Option", "std.console"), RET_VAL },
    { AstFuncInfo("readToEnd", "ConsoleReader", {"ConsoleReader"}, "Option", "std.console"), RET_VAL },
    /* file stream */
    { AstFuncInfo("read", "InputStream", { "InputStream", "Array" }, "", "std.io"), 2 },
    { AstFuncInfo("readToEnd", "", { "InputStream" }, "Array", "std.io"), RET_VAL },
    /* global function */
    { AstFuncInfo("gethostname", "", {}, "String", "std.posix"), RET_VAL }
};

static const std::vector<std::pair<AstFuncInfo, PropagateInfo>> taintPropagate = {
      /* Response */
    { AstFuncInfo("cookies", "HttpResponse", { "HttpResponse" }, "Array", "stdx.net.http"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("copyTo", "HttpResponse", { "HttpResponse", "OutputStream" }, "", "stdx.net.http"),
        PropagateInfo(1, 2, IsTaint::NOT_SURE) },
    { AstFuncInfo("isClosed", "HttpResponse", { "HttpResponse" }, "", "stdx.net.http"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("isUncompressed", "HttpResponse", { "HttpResponse" }, "", "stdx.net.http"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("location", "HttpResponse", { "HttpResponse" }, "URL", "stdx.net.http"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("protoAtLeast", "HttpResponse", { NOT_CARE }, "", "stdx.net.http"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("readTextToEnd", "HttpResponse", { "HttpResponse" }, "String", "stdx.net.http"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("readToEnd", "HttpResponse", { "HttpResponse" }, "Array", "stdx.net.http"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    /* Collection and Array */
    { AstFuncInfo("[]", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("[]", NOT_CARE, { NOT_CARE, ANY_TYPE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(3, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("[]", ANY_TYPE, { "Range" }, ANY_TYPE, NOT_CARE),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("==", ANY_TYPE, { ANY_TYPE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("!=", ANY_TYPE, { ANY_TYPE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("add", ANY_TYPE, { ANY_TYPE }, NOT_CARE, "std.collection"),
      PropagateInfo(2, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("add", "ArrayList", { "Collection" }, "", "std.collection"),
      PropagateInfo(2, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("arrayInit1", "", { ANY_TYPE, ANY_TYPE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(2, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("capacity", ANY_TYPE, {ANY_TYPE}, "", "std.collection"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("clear", ANY_TYPE, {ANY_TYPE}, NOT_CARE, "std.collection"),
        PropagateInfo(1, 1, IsTaint::NOT_TAINT) },
    { AstFuncInfo("clone", ANY_TYPE, {ANY_TYPE}, ANY_TYPE, "std.collection"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("get", ANY_TYPE, { ANY_TYPE }, "Option", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("init", ANY_TYPE, { NOT_CARE }, NOT_CARE, NOT_CARE),
        PropagateInfo(ALL_ARGS, 1, IsTaint::NOT_SURE) },
    { AstFuncInfo("init", ANY_TYPE, { NOT_CARE, "Protocol" }, "Void", NOT_CARE),
        PropagateInfo(ALL_ARGS, 1, IsTaint::NOT_SURE) },
    { AstFuncInfo("add", ANY_TYPE, { "", ANY_TYPE }, NOT_CARE, "std.collection"),
      PropagateInfo(3, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("add", "ArrayList", { "", "Collection" }, "", "std.collection"),
      PropagateInfo(3, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("iterator", NOT_CARE, {NOT_CARE}, ANY_TYPE, NOT_CARE),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("keys", "HashMap", {"HashMap"}, "Keys", "std.collection"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("next", ANY_TYPE, {ANY_TYPE}, "Option", "std.collection"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("next", ANY_TYPE, {ANY_TYPE}, "Option", "std.core"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("prepend", "ArrayList", { "ArrayList", ANY_TYPE }, "", "std.collection"),
      PropagateInfo(2, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("prependAll", "ArrayList", { "ArrayList", "Collection" }, "", "std.collection"),
      PropagateInfo(2, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("add", "HashMap", { "HashMap", ANY_TYPE, ANY_TYPE }, "Option", "std.collection"),
      PropagateInfo(3, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("add", "HashSet", { "HashSet", ANY_TYPE }, "", "std.collection"),
      PropagateInfo(2, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("add", ANY_TYPE, { ANY_TYPE, "Collection" }, "", "std.collection"),
      PropagateInfo(2, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("addIfAbsent", "HashMap", { "HashMap", ANY_TYPE, ANY_TYPE }, "EntryView", "std.collection"),
      PropagateInfo(3, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("remove", ANY_TYPE, { NOT_CARE }, ANY_TYPE, "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("remove", ANY_TYPE, { ANY_TYPE, "Collection" }, "", "std.collection"),
      PropagateInfo(1, 1, IsTaint::NOT_SURE) },
    { AstFuncInfo("removeIf", ANY_TYPE, { ANY_TYPE }, "", "std.collection"),
        PropagateInfo(1, 1, IsTaint::NOT_SURE) },
    { AstFuncInfo("reserve", ANY_TYPE, { ANY_TYPE, "" }, "", "std.collection"),
        PropagateInfo(1, 1, IsTaint::NOT_SURE) },
    { AstFuncInfo("reverse", ANY_TYPE, {ANY_TYPE}, "", "std.collection"),
        PropagateInfo(1, 1, IsTaint::NOT_SURE) },
    { AstFuncInfo("set", "ArrayList", { "", ANY_TYPE }, "", "std.collection"),
      PropagateInfo(3, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("slice", "ArrayList", { "ArrayList", "Range" }, "ArrayList", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("sortBy", "ArrayList", { "ArrayList", ANY_TYPE, "" }, "", "std.collection"),
      PropagateInfo(1, 1, IsTaint::NOT_SURE) },
    { AstFuncInfo("values", "HashMap", {"HashMap"}, "Values", "std.collection"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    /* StringBuilder */
    { AstFuncInfo("byteCount", "StringBuilder", {"StringBuilder"}, "", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("data", "StringBuilder", {"StringBuilder"}, "Array", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("equals", "StringBuilder", { "StringBuilder", "StringBuilder" }, "", "std.collection"),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("isAscii", "StringBuilder", {"StringBuilder"}, "", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("pop", "StringBuilder", {"StringBuilder"}, "Option", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("retain", "StringBuilder", { "StringBuilder", ANY_TYPE }, "StringBuilder", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("toCArray", "StringBuilder", {"StringBuilder"}, "CPointer", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("toCharArray", "StringBuilder", {"StringBuilder"}, "Array", "std.collection"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    /* Json Data */
    { AstFuncInfo("add", "JsonArray", { "JsonArray", "JsonValue" }, "", "stdx.encoding.json"),
      PropagateInfo(2, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    { AstFuncInfo("asArray", "JsonValue", {"JsonValue"}, "JsonArray", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("asBool", "JsonValue", {"JsonValue"}, "JsonBool", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("asFloat", "JsonValue", {"JsonValue"}, "JsonFloat", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("asInt", "JsonValue", {"JsonValue"}, "JsonInt", "stdx.encoding.json"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("asNull", "JsonValue", {"JsonValue"}, "JsonNull", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("asObject", "JsonValue", {"JsonValue"}, "JsonObject", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("asString", "JsonValue", {"JsonValue"}, "JsonString", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("containsKey", "JsonObject", { "JsonObject", "String" }, "", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("fromStr", "JsonValue", { "JsonValue", "String" }, "JsonValue", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("get", "JsonArray", { "JsonArray", "" }, "Option", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("get", "JsonObject", { "JsonObject", "String" }, "Option", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("getFields", "JsonObject", {"JsonObject"}, ANY_TYPE, "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("getItems", "JsonArray", {"JsonArray"}, NOT_CARE, "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("getValue", ANY_TYPE, {ANY_TYPE}, "String", "stdx.encoding.json"),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("toJsonString", ANY_TYPE, {ANY_TYPE}, "String", "stdx.encoding.json"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("put", "JsonObject", { "JsonObject", "String", "JsonValue" }, "Option", "stdx.encoding.json"),
      PropagateInfo(3, 1, IsTaint::NOT_CHANGE_OR_TAINT) },
    /* string */
    { AstFuncInfo("concat", "String", { "String", "String" }, "String", "std.core"),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_SURE) },
    /* others */
    { AstFuncInfo("getOrThrow", ANY_TYPE, { NOT_CARE }, ANY_TYPE, "std.core"),
      PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("toString", NOT_CARE, {NOT_CARE}, "String", NOT_CARE),
        PropagateInfo(1, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("+", ANY_TYPE, { ANY_TYPE, ANY_TYPE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_SURE) },
    { AstFuncInfo("matches", "Regex", { "Regex", "String" }, NOT_CARE, "std.regex"),
      PropagateInfo(2, RET_VAL, IsTaint::NOT_SURE) },
    /* fuzzy matching */
    { AstFuncInfo("contains", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("containsAll", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("empty", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("isEmpty", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("len", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("length", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("size", NOT_CARE, { NOT_CARE }, ANY_TYPE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("$statusget", NOT_CARE, { "HttpResponse" }, NOT_CARE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("$bodySizeget", NOT_CARE, { "HttpResponse" }, NOT_CARE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("$versionget", NOT_CARE, { "HttpResponse" }, NOT_CARE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) },
    { AstFuncInfo("$urlget", NOT_CARE, { "HttpRequest" }, NOT_CARE, NOT_CARE),
      PropagateInfo(ALL_ARGS, RET_VAL, IsTaint::NOT_TAINT) }
};
}
#endif
