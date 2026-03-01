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

#ifndef SOURCE_MAP_H
#define SOURCE_MAP_H

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <v8.h>
#include <vector>

#include "jsvm_util.h"

// Base64 VLQ decoding
int DecodeVLQ(const std::string& input, size_t& pos)
{
    // Const value
    constexpr int vlqBaseShift = 5;
    constexpr int vlqBaseMask = (1 << 5) - 1;
    constexpr int vlqContinuationMask = 1 << 5;
    static std::unordered_map<char, int> base64Map {
        { 'A', 0 },  { 'B', 1 },  { 'C', 2 },  { 'D', 3 },  { 'E', 4 },  { 'F', 5 },  { 'G', 6 },  { 'H', 7 },
        { 'I', 8 },  { 'J', 9 },  { 'K', 10 }, { 'L', 11 }, { 'M', 12 }, { 'N', 13 }, { 'O', 14 }, { 'P', 15 },
        { 'Q', 16 }, { 'R', 17 }, { 'S', 18 }, { 'T', 19 }, { 'U', 20 }, { 'V', 21 }, { 'W', 22 }, { 'X', 23 },
        { 'Y', 24 }, { 'Z', 25 }, { 'a', 26 }, { 'b', 27 }, { 'c', 28 }, { 'd', 29 }, { 'e', 30 }, { 'f', 31 },
        { 'g', 32 }, { 'h', 33 }, { 'i', 34 }, { 'j', 35 }, { 'k', 36 }, { 'l', 37 }, { 'm', 38 }, { 'n', 39 },
        { 'o', 40 }, { 'p', 41 }, { 'q', 42 }, { 'r', 43 }, { 's', 44 }, { 't', 45 }, { 'u', 46 }, { 'v', 47 },
        { 'w', 48 }, { 'x', 49 }, { 'y', 50 }, { 'z', 51 }, { '0', 52 }, { '1', 53 }, { '2', 54 }, { '3', 55 },
        { '4', 56 }, { '5', 57 }, { '6', 58 }, { '7', 59 }, { '8', 60 }, { '9', 61 }, { '+', 62 }, { '/', 63 }
    };

    // Decode VLQ
    int result = 0;
    int shift = 0;
    int digit;
    do {
        digit = base64Map[input[pos++]];
        result += (digit & vlqBaseMask) << shift;
        shift += vlqBaseShift;
    } while (digit & vlqContinuationMask);

    // Fix the sign
    int negative = result & 1;
    result >>= 1;
    return negative ? -result : result;
}

struct Offset {
public:
    constexpr Offset(int line, int column) : line(line), column(column) {}

    constexpr static Offset InvalidOffset()
    {
        return Offset(-1, -1);
    }

    bool IsInvalid() const
    {
        return line < 0 || column < 0;
    }

    bool operator<(const Offset& other) const
    {
        return line < other.line || (line == other.line && column < other.column);
    }

public:
    int line;
    int column;
};

struct Mappings {
public:
    explicit Mappings(Offset traceOffset,
             Offset sourceOffset = Offset::InvalidOffset(),
             std::string sourceName = "",
             int nameIdx = -1)
        : traceOffset(traceOffset), sourceOffset(sourceOffset), nameIdx(nameIdx), sourceName(sourceName)
    {}

    bool IsInvalid() const
    {
        return traceOffset.IsInvalid() || sourceOffset.IsInvalid();
    }

    bool operator<(const Mappings& other) const
    {
        return traceOffset < other.traceOffset;
    }

    std::string ToString() const
    {
        if (IsInvalid()) {
            return "";
        }
        return sourceName + ":" + std::to_string(sourceOffset.line + 1) + ":" + std::to_string(sourceOffset.column + 1);
    }

public:
    Offset traceOffset;
    Offset sourceOffset;
    int nameIdx;
    std::string sourceName;
};

class SourceMap {
public:
    SourceMap(v8::Isolate* isolate, v8::Local<v8::Context> context, v8::Local<v8::Object> payload)
        : isolate(isolate), context(context), payload(payload)
    {
        ParseMappingPayload();
    }

    Mappings FindEntry(int lineOffset, int columnOffset)
    {
        int count = sourceMappings.size();
        if (count == 0) {
            return Mappings(Offset::InvalidOffset());
        }

        int left = 0;
        Offset offset(lineOffset, columnOffset);

        while (count > 1) {
            int step = count >> 1;
            int middle = left + step;
            if (offset < sourceMappings[middle].traceOffset) {
                count = step;
            } else {
                left = middle;
                count -= step;
            }
        }

        if (left == 0 && offset < sourceMappings[0].traceOffset) {
            return Mappings(Offset::InvalidOffset());
        }

        return sourceMappings[left];
    }

private:
    void ParseMappingPayload();
    void ParseMap(v8::Local<v8::Value> map, int line, int column);
    void ParseMappings(const std::string& mappings,
                       const std::vector<std::string>& sources,
                       int lineNumber,
                       int columnNumber);
    void ParseSections(v8::Local<v8::Value> sections);
    std::vector<std::string> ParseSourceNames(v8::Local<v8::Array> sources);

private:
    v8::Isolate* isolate;
    v8::Local<v8::Context> context;
    v8::Local<v8::Object> payload;
    std::vector<Mappings> sourceMappings;
};

void SourceMap::ParseMappingPayload()
{
    auto sections = payload->Get(context, v8::String::NewFromUtf8Literal(isolate, "sections"));
    if (!sections.IsEmpty() && sections.ToLocalChecked()->ToBoolean(isolate)->Value()) {
        ParseSections(sections.ToLocalChecked());
    } else {
        ParseMap(payload, 0, 0);
    }

    std::sort(sourceMappings.begin(), sourceMappings.end());
}

std::vector<std::string> SourceMap::ParseSourceNames(v8::Local<v8::Array> sources)
{
    std::vector<std::string> names;

    for (uint32_t i = 0; i < sources->Length(); ++i) {
        auto element = sources->Get(context, i);
        // should be string
        v8::Local<v8::Value> fromMaybe;
        if (!element.ToLocal(&fromMaybe) || !fromMaybe->IsString()) {
            names.emplace_back("");
            continue;
        }
        v8::String::Utf8Value source(isolate, fromMaybe);
        names.emplace_back(*source);
    }

    return names;
}

void SourceMap::ParseMap(v8::Local<v8::Value> map, int line, int column)
{
    if (!map->IsObject()) {
        return;
    }

    auto mapObj = map.As<v8::Object>();

    // Get sources
    auto maybeSources = mapObj->Get(context, v8::String::NewFromUtf8Literal(isolate, "sources"));
    v8::Local<v8::Value> fromMaybe;
    if (!maybeSources.ToLocal(&fromMaybe) || !fromMaybe->IsArray()) {
        return;
    }
    auto arr = fromMaybe.As<v8::Array>();
    auto names = ParseSourceNames(arr);

    // Get mappings
    v8::Local<v8::Value> mappingsValue =
        mapObj->Get(context, v8::String::NewFromUtf8Literal(isolate, "mappings")).ToLocalChecked();
    if (!mappingsValue->IsString()) {
        return;
    }
    v8::String::Utf8Value mappingsStr(isolate, mappingsValue);

    // Parse mappings
    ParseMappings(*mappingsStr, names, line, column);
}

void SourceMap::ParseMappings(const std::string& mappings,
                              const std::vector<std::string>& sources,
                              int lineNumber,
                              int columnNumber)
{
    size_t pos = 0;
    int sourceIndex = 0;
    int sourceLineNumber = 0;
    int sourceColumnNumber = 0;
    int nameIndex = 0;

    while (pos < mappings.length()) {
        if (mappings[pos] == ',') {
            pos++;
        } else {
            while (pos < mappings.length() && mappings[pos] == ';') {
                lineNumber++;
                columnNumber = 0;
                pos++;
            }

            if (pos == mappings.length()) {
                break;
            }
        }

        columnNumber += DecodeVLQ(mappings, pos);
        if (pos >= mappings.length() || mappings[pos] == ',' || mappings[pos] == ';') {
            sourceMappings.emplace_back(Offset(lineNumber, columnNumber));
            continue;
        }

        int sourceIndexDelta = DecodeVLQ(mappings, pos);
        if (sourceIndexDelta) {
            sourceIndex += sourceIndexDelta;
        }
        sourceLineNumber += DecodeVLQ(mappings, pos);
        sourceColumnNumber += DecodeVLQ(mappings, pos);

        if (pos < mappings.length() && mappings[pos] != ',' && mappings[pos] != ';') {
            nameIndex += DecodeVLQ(mappings, pos);
        }
        sourceMappings.emplace_back(Offset(lineNumber, columnNumber), Offset(sourceLineNumber, sourceColumnNumber),
                                    sources[sourceIndex], nameIndex);
    }
}

void SourceMap::ParseSections(v8::Local<v8::Value> sections)
{
    if (!sections->IsArray()) {
        return;
    }

    v8::Local<v8::Array> arr = sections.As<v8::Array>();
    for (uint32_t i = 0; i < arr->Length(); ++i) {
        auto element = arr->Get(context, i);

        v8::Local<v8::Value> fromMaybe;
        if (!element.ToLocal(&fromMaybe) || !fromMaybe->IsObject()) {
            continue;
        }
        auto section = fromMaybe.As<v8::Object>();

        auto maybeMap = section->Get(context, v8::String::NewFromUtf8Literal(isolate, "map"));
        if (maybeMap.IsEmpty()) {
            continue;
        }

        auto maybeOffset = section->Get(context, v8::String::NewFromUtf8Literal(isolate, "offset"));
        if (!maybeOffset.ToLocal(&fromMaybe) || !fromMaybe->IsObject()) {
            continue;
        }
        auto offset = fromMaybe.As<v8::Object>();

        auto maybeLine = offset->Get(context, v8::String::NewFromUtf8Literal(isolate, "line"));
        if (!maybeLine.ToLocal(&fromMaybe)) {
            continue;
        }
        auto maybeInt = fromMaybe->ToInt32(context);
        int line = maybeInt.IsEmpty() ? -1 : maybeInt.ToLocalChecked()->Value();

        auto maybeColumn = offset->Get(context, v8::String::NewFromUtf8Literal(isolate, "column"));
        if (!maybeColumn.ToLocal(&fromMaybe)) {
            continue;
        }
        maybeInt = fromMaybe->ToInt32(context);
        int column = maybeInt.IsEmpty() ? -1 : maybeInt.ToLocalChecked()->Value();
        if (line == -1 || column == -1) {
            continue;
        }

        ParseMap(maybeMap.ToLocalChecked(), line, column);
    }
}

v8::MaybeLocal<v8::Value> HandleError(v8::Isolate* isolate,
                                      v8::Local<v8::Context> ctx,
                                      v8::Local<v8::Value> error,
                                      v8::Local<v8::Array> trace)
{
    v8::Local<v8::String> stackStr;
    if (!error->ToString(ctx).ToLocal(&stackStr)) {
        return v8::MaybeLocal<v8::Value>();
    }

    auto left = v8::String::NewFromUtf8Literal(isolate, "\n    at ");

    for (uint32_t i = 0; i < trace->Length(); ++i) {
        v8::Local<v8::Value> element = trace->Get(ctx, i).ToLocalChecked();
        v8::Local<v8::String> str;

        if (!element->ToString(ctx).ToLocal(&str)) {
            return v8::MaybeLocal<v8::Value>();
        }

        auto traceStr = v8::String::Concat(isolate, left, str);

        stackStr = v8::String::Concat(isolate, stackStr, traceStr);
    }

    return stackStr;
}

static std::string SourceMapRunner = R"JS(
result =  (t, originalSouceInfo) => {
    const str = '\n    at ';
    try {
        if (originalSouceInfo != "") {
            let fileName = t.getFileName();
            if (fileName === undefined) {
                fileName = t.getEvalOrigin()
            }
            const fnName = t.getFunctionName() ?? t.getMethodName();
            const typeName = t.getTypeName();
            const namePrefix = typeName !== null && typeName !== 'global' ? `${typeName}.` : '';
            const originalName = `${namePrefix}${fnName||'<anonymous>'}`;
            const hasName = !!originalName;
            return `${str}${originalName}${hasName?' (':''}` + originalSouceInfo + `${hasName?')':''}`
        }
        return `${str}${t}`
    } catch (e) {
        return `${str}${t}`
    }
}
)JS";

int GetAndCallFunction(v8::Isolate* isolate,
                       v8::Local<v8::Context> ctx,
                       v8::Local<v8::Object> obj,
                       v8::Local<v8::String> funcName)
{
    auto maybeFunc = obj->Get(ctx, funcName);

    v8::Local<v8::Value> value;
    if (!maybeFunc.ToLocal(&value) || !value->IsFunction()) {
        return -1;
    }

    auto func = value.As<v8::Function>();

    // eval obj.funcName()
    auto maybeRes = func->Call(ctx, obj, 0, nullptr);
    if (maybeRes.IsEmpty()) {
        return -1;
    }

    auto maybeInt = maybeRes.ToLocalChecked()->ToInt32(ctx);
    return maybeInt.IsEmpty() ? -1 : maybeInt.ToLocalChecked()->Value();
}

v8::MaybeLocal<v8::Value> ParseSourceMap(v8::Isolate* isolate,
                                         v8::Local<v8::Context> ctx,
                                         v8::Local<v8::Value> error,
                                         v8::Local<v8::Array> trace,
                                         v8::Local<v8::Function> toStringFunc,
                                         std::string& sourceMapContent)
{
    v8::TryCatch tryCatch(isolate);
    auto sourceMapStr = v8::String::NewFromUtf8(isolate, sourceMapContent.c_str(), v8::NewStringType::kNormal,
                                                sourceMapContent.length())
                            .ToLocalChecked();

    // Parse json string to object
    v8::Local<v8::Value> sourceMapObj;
    if (!v8::JSON::Parse(ctx, sourceMapStr).ToLocal(&sourceMapObj) || !sourceMapObj->IsObject()) {
        return HandleError(isolate, ctx, error, trace);
    }

    v8::Local<v8::String> stackStr;
    if (!error->ToString(ctx).ToLocal(&stackStr)) {
        return v8::MaybeLocal<v8::Value>();
    }

    SourceMap sourceMap(isolate, ctx, sourceMapObj.As<v8::Object>());

    // Get line and column
    auto getLineStr = v8::String::NewFromUtf8Literal(isolate, "getLineNumber");
    auto getColumnStr = v8::String::NewFromUtf8Literal(isolate, "getColumnNumber");

    for (uint32_t i = 0; i < trace->Length(); ++i) {
        v8::Local<v8::Value> element = trace->Get(ctx, i).ToLocalChecked();
        if (!element->IsObject()) {
            continue;
        }

        auto t = element.As<v8::Object>();
        auto line = GetAndCallFunction(isolate, ctx, t, getLineStr) - 1;
        auto column = GetAndCallFunction(isolate, ctx, t, getColumnStr) - 1;

        auto str = sourceMap.FindEntry(line, column).ToString();
        auto originalSouceInfo =
            v8::String::NewFromUtf8(isolate, str.c_str(), v8::NewStringType::kNormal, str.length()).ToLocalChecked();

        v8::Local<v8::Value> args[] = { t, originalSouceInfo };
        auto traceStr = toStringFunc->Call(ctx, v8::Undefined(isolate), jsvm::ArraySize(args), args)
                            .ToLocalChecked()
                            .As<v8::String>();
        stackStr = v8::String::Concat(isolate, stackStr, traceStr);
    }

    // Check execption
    if (tryCatch.HasCaught()) {
        return HandleError(isolate, ctx, error, trace);
    }

    return stackStr;
}

#endif