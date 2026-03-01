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

#ifndef CODIRA_AST_IDENTIFIER
#define CODIRA_AST_IDENTIFIER

#include <string>

#include "Codira/Basic/Position.h"
#include "Codira/Utils/CheckUtils.h"

namespace Codira {
/**
 * Data class that stores the string value after NFC transformation, start location and length of a source code
 * identifier. Note that the length of source code identifier may not equal to the length of \ref value because
 * of NFC transformation.
 */
struct Identifier {
    Identifier() : v{}
    {
    }
    Identifier(std::string s, const Position& begin, const Position& end);
    /**
     * helper function, to be used as the string value of the identifier
     */
    operator const std::string&() const
    {
        return v;
    }

    ///@{
    /**
     * Assignment from string, string_view, or const char[N]. These assignment functions assert that \p identifier
     * is in NFC form.
     */
    Identifier& operator=(std::string_view identifier);
    template <int N> Identifier& operator=(const char (&identifier)[N])
    {
        return (*this = std::string_view(identifier));
    }
    Identifier& operator=(const std::string& identifier);
    Identifier& operator=(std::string&& identifier);
    Identifier& operator=(const Identifier& other) = default;
    Identifier& operator=(Identifier&& other) = default;
    Identifier(const Identifier& other) = default;
    Identifier(Identifier&& other) = default;
    ///@}

    const std::string& Val() const
    {
        return v;
    }
    bool Valid() const;
    bool Empty() const
    {
        return v.empty();
    }

    /**< clear identifier string value and length */
    void Clear()
    {
        v = {};
        beginPos = INVALID_POSITION;
        endPos = INVALID_POSITION;
    }

    inline friend bool operator==(const std::string& l, const Identifier& r);

    bool operator==(std::string_view other) const
    {
        return v == other;
    }
    bool operator!=(std::string_view other) const
    {
        return !(*this == other);
    }
    bool operator==(const Identifier& other) const
    {
        return v == other.v;
    }
    bool operator!=(const Identifier& other) const
    {
        return !(*this == other);
    }

    inline friend std::string operator+(const std::string& prefix, const Identifier& r);
    std::string operator+(const std::string& postfix) const
    {
        return v + postfix;
    }

    Identifier& operator+=(const std::string& other)
    {
        std::string v1{std::move(v) + other};
        SetValue(std::move(v1));
        return *this;
    }

    const Position& Begin() const
    {
        return beginPos;
    }
    const Position& End() const
    {
        return endPos;
    }

    void SetPos(const Position& begin, const Position& end)
    {
        beginPos = begin;
        endPos = end;
    }
    void SetFileID(unsigned fileID)
    {
        beginPos.fileID = endPos.fileID = fileID;
    }

    size_t Length() const
    {
        CODEC_ASSERT(beginPos.line == endPos.line);
        return static_cast<size_t>(static_cast<ssize_t>(endPos.column - beginPos.column));
    }
    
    /**< check position is zero */
    bool ZeroPos() const
    {
        return beginPos.IsZero();
    }

    friend std::ostream& operator<<(std::ostream& out, const Identifier& identifier);

private:
    std::string v; /**< string value of the identifier */
    Position beginPos{INVALID_POSITION};  /**< start location */
    Position endPos{INVALID_POSITION};

    void SetValue(std::string&& s); // private setter for debug issue
};

inline bool operator==(const std::string& l, const Identifier& r)
{
    return l == r.v;
}
inline std::string operator+(const std::string& prefix, const Identifier& r)
{
    return prefix + r.v;
}

/**
 * A source identifier differs from its base class \ref Identifier in that it can be a rawIdentifier (i.e. surrounded
 * by a pair of backquotes).
 */
struct SrcIdentifier : public Identifier {
    SrcIdentifier() : Identifier{}, raw{false}
    {
    }
    explicit SrcIdentifier(std::string s) : Identifier{std::move(s), INVALID_POSITION, INVALID_POSITION}, raw{false}
    {
    }
    SrcIdentifier(std::string s, const Position& begin, const Position& end, bool isRaw)
        : Identifier{std::move(s), begin, end}, raw{isRaw}
    {
    }

    bool IsRaw() const
    {
        return raw;
    }
    void SetRaw(bool v)
    {
        raw = v;
    }

    std::string GetRawText() const
    {
        return raw ? "`" + Val() + "`" : Val();
    }
    Position GetRawPos() const
    {
        return raw ? Begin() - 1 : Begin();
    }
    Position GetRawEndPos() const
    {
        return raw ? End() + 1 : End();
    }

    ///@{
    /**
     * Assignment from raw string. This function asserts \p v conforms to NFC and the identifier is not raw.
     */
    using Identifier::operator=;
    SrcIdentifier& operator=(const SrcIdentifier& other) = default;
    SrcIdentifier& operator=(SrcIdentifier&& other) = default;

    SrcIdentifier(const SrcIdentifier& other) = default;
    SrcIdentifier(SrcIdentifier&& other) = default;
    ///@}

private:
    bool raw; /**< is raw identifier */
};
}
#endif
