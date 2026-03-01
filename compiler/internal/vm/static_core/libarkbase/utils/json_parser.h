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

#ifndef COMPILER_TOOLS_PAOC_JSON_PARSER_H
#define COMPILER_TOOLS_PAOC_JSON_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory>
#include <algorithm>

#include "libarkbase/macros.h"

WEAK_FOR_LTO_START

namespace ark {

class JsonObject {
public:
    class Value;
    using StringT = std::string;
    using NumT = double;
    using BoolT = bool;
    using ArrayT = std::vector<Value>;
    using Key = StringT;
    using JsonObjPointer = std::unique_ptr<JsonObject>;

    class Value {
    public:
        Value() = default;
        ~Value() = default;
        NO_COPY_SEMANTIC(Value);
        Value(Value &&rhs) noexcept  // NOLINT(bugprone-exception-escape)
        {
            value_ = std::move(rhs.value_);
            rhs.value_ = std::monostate {};
        }
        Value &operator=(Value &&rhs) noexcept  // NOLINT(bugprone-exception-escape)
        {
            value_ = std::move(rhs.value_);
            rhs.value_ = std::monostate {};
            return *this;
        }

        template <typename T>
        void SetValue(T &&rhs) noexcept
        {
            value_ = std::forward<T>(rhs);
        }

        template <typename T>
        T *Get()
        {
            return std::get_if<T>(&value_);
        }
        template <typename T>
        const T *Get() const
        {
            return std::get_if<T>(&value_);
        }

    private:
        std::variant<std::monostate, StringT, NumT, BoolT, ArrayT, JsonObjPointer> value_;
    };

    // Recursive descent parser:
    class Parser {
    public:
        explicit Parser(JsonObject *target) : currentObj_(target) {}
        PANDA_PUBLIC_API bool Parse(const std::string &text);
        PANDA_PUBLIC_API bool Parse(std::streambuf *streamBuf);

        virtual ~Parser() = default;

        NO_COPY_SEMANTIC(Parser);
        NO_MOVE_SEMANTIC(Parser);

    private:
        bool Parse();

        bool GetJsonObject(JsonObject *emptyObj);
        void SaveSourceString(std::streampos posStart);
        bool GetValue();
        bool GetNull();
        bool GetString(char delim);
        bool GetJsonString();
        bool GetNum();
        bool GetBool();
        bool GetArray();
        bool InsertKeyValuePairIn(JsonObject *obj);

        char GetSymbol();
        char PeekSymbol();
        bool TryGetSymbol(int symbol);

        static bool IsWhitespace(int symbol);

    private:
        std::istream istream_ {nullptr};
        JsonObject *currentObj_ {nullptr};
        Value parsedTemp_;
        StringT stringTemp_;

        size_t logRecursionLevel_ {0};
    };

public:
    JsonObject() = default;
    ~JsonObject() = default;
    NO_COPY_SEMANTIC(JsonObject);
    NO_MOVE_SEMANTIC(JsonObject);
    explicit JsonObject(const std::string &text)
    {
        Parser(this).Parse(text);
    }
    explicit JsonObject(std::streambuf *streamBuf)
    {
        Parser(this).Parse(streamBuf);
    }

    size_t GetSize() const
    {
        ASSERT(valuesMap_.size() == keys_.size());
        ASSERT(valuesMap_.size() == stringMap_.size());
        return valuesMap_.size();
    }

    size_t GetIndexByKey(const Key &key) const
    {
        auto it = std::find(keys_.begin(), keys_.end(), key);
        if (it != keys_.end()) {
            return it - keys_.begin();
        }
        return static_cast<size_t>(-1);
    }
    const auto &GetKeyByIndex(size_t idx) const
    {
        ASSERT(idx < GetSize());
        return keys_[idx];
    }

    bool HasKey(const Key &key) const noexcept
    {
        return valuesMap_.find(key) != valuesMap_.end();
    }

    template <typename T>
    const T *GetValue(const Key &key) const
    {
        auto iter = valuesMap_.find(key);
        return (iter == valuesMap_.end()) ? nullptr : iter->second.Get<T>();
    }
    const StringT *GetValueSourceString(const Key &key) const
    {
        auto iter = stringMap_.find(key);
        return (iter == stringMap_.end()) ? nullptr : &iter->second;
    }
    template <typename T>
    const T *GetValue(size_t idx) const
    {
        auto iter = valuesMap_.find(GetKeyByIndex(idx));
        return (iter == valuesMap_.end()) ? nullptr : iter->second.Get<T>();
    }

    const auto &GetUnorderedMap() const
    {
        return valuesMap_;
    }

    bool IsValid() const
    {
        return isValid_;
    }

private:
    bool isValid_ {false};
    std::unordered_map<Key, Value> valuesMap_;
    // String representation is stored additionally as a "source" of scalar values:
    std::unordered_map<Key, StringT> stringMap_;

    // Stores the order in which keys were added (allows to access elements by index):
    std::vector<Key> keys_;
};

}  // namespace ark

WEAK_FOR_LTO_END

#endif  // COMPILER_TOOLS_PAOC_JSON_PARSER_H
