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
 * This file provides unicode normalization functions.
 */

#include "Codira/Utils/Unicode.h"

#include <algorithm>
#include <climits>
#include <vector>

#include "Codira/Utils/CheckUtils.h"
#include "UnicodeTables/NormalisationData.generated.inc"

namespace Codira::Unicode {
namespace {
uint64_t MyHash(UTF32 key, UTF32 salt, uint64_t n)
{
    UTF32 t1;
    (void)__builtin_uadd_overflow(key, salt, &t1);
    constexpr auto other = 2654435769;
    UTF32 y;
    (void)__builtin_umul_overflow(t1, other, &y);
    constexpr auto other2 = 0x31415926;
    UTF32 t2;
    (void)__builtin_umul_overflow(key, other2, &t2);
    y = y ^ t2;
    return (static_cast<uint64_t>(y) * static_cast<uint64_t>(n)) >> (sizeof(uint32_t) * CHAR_BIT);
}

template <class Key, class Value, class KV, int saltLen>
std::optional<Value> MetaLookup(UTF32 x, const short (&salt)[saltLen], const std::pair<UTF32, KV>* kv,
    Key (*keyProjection)(const std::pair<UTF32, KV>&),
    std::optional<Value> (*valueProjection)(const std::pair<UTF32, KV>&),
    std::optional<Value> defaultVal = std::nullopt)
{
    UTF32 s = static_cast<unsigned short>(salt[MyHash(x, 0, saltLen)]);
    auto& keyValuePair = kv[MyHash(x, s, saltLen)];
    return x == keyProjection(keyValuePair) ? valueProjection(keyValuePair) : defaultVal;
}

template <class T>
inline UTF32 PairLookupFk(const std::pair<UTF32, T>& kv)
{
    return kv.first;
}

template <class V>
inline std::optional<V> PairLookupFvOpt(const std::pair<UTF32, V>& kv)
{
    return {kv.second};
}
}

std::optional<UTF32> CompositionTable(UTF32 c1, UTF32 c2)
{
    constexpr UTF32 m = 0x10000;
    constexpr UTF32 m2 = 16;
    if (c1 < m && c2 < m) {
        return MetaLookup<UTF32>((c1 << m2) | c2, COMPOSITION_TABLE_SALT, COMPOSITION_TABLE_KV, PairLookupFk,
            PairLookupFvOpt, std::optional<UTF32>{std::nullopt});
    }
    return CompositionTableAstral(c1, c2);
}

namespace {
///@{
/**
 See Unicode 15.0.0 3.12 Hangul Syllable Decomposition Common Constants
 */
constexpr UTF32 SBASE = 0xAC00;
constexpr UTF32 LBASE = 0x1100;
constexpr UTF32 VBASE = 0x1161;
constexpr UTF32 TBASE = 0x11A7;
constexpr UTF32 LCOUNT = 19;
constexpr UTF32 VCOUNT = 21;
constexpr UTF32 TCOUNT = 28;
constexpr UTF32 NCOUNT = VCOUNT * TCOUNT;
constexpr UTF32 SCOUNT = LCOUNT * NCOUNT;

constexpr UTF32 SLAST = SBASE + SCOUNT - 1;
constexpr UTF32 LLAST = LBASE + LCOUNT - 1;
constexpr UTF32 VLAST = VBASE + VCOUNT - 1;
constexpr UTF32 TLAST = TBASE + TCOUNT - 1;
constexpr UTF32 TFIRST = TBASE + 1;
///@}

bool IsHangulSyllable(UTF32 c)
{
    return static_cast<UTF32>(c) >= SBASE && static_cast<UTF32>(c) < SBASE + SCOUNT;
}

void DecomposeHangul(UTF32 c, std::function<void(UTF32)>&& emitChar)
{
    // at most 0x2baf, the size of Hangul Syllables block
    auto sindex = c - SBASE;
    // at most 0x2baf / (21 * 28) = 19
    auto lindex = sindex / NCOUNT;
    // at most 0x1113
    emitChar(LBASE + lindex);
    // at most 21
    auto vindex = (sindex % NCOUNT) / TCOUNT;
    // at most 0x1176
    emitChar(VBASE + vindex);
    // at most 27
    auto tindex = sindex % TCOUNT;
    if (tindex > 0) {
        emitChar(TBASE + tindex);
    }
}
}

std::optional<UTF32> ComposeHangul(UTF32 a, UTF32 b)
{
    if (a >= LBASE && a <= LLAST && b >= VBASE && b <= VLAST) {
        auto lindex = a - LBASE;
        auto vindex = b - VBASE;
        auto lvindex = lindex * NCOUNT + vindex * TCOUNT;
        auto s = SBASE + lvindex;
        return {s};
    }
    if (a >= SBASE && a <= SLAST && b >= TFIRST && b <= TLAST && (a - SBASE) % TCOUNT == 0) {
        return {a + b - TBASE};
    }
    return {};
}

namespace {
template <class D>
void Decompose(UTF32 c, D&& decomposeChar, std::function<void(UTF32)>&& emitChar)
{
    // ASCII never decomposes
    if (c <= '\x7f') {
        emitChar(c);
        return;
    }

    if (IsHangulSyllable(c)) {
        DecomposeHangul(c, std::move(emitChar));
        return;
    }

    if (auto d = decomposeChar(c)) {
        for (auto it = d->first; it != d->second; ++it) {
            emitChar(*it);
        }
        return;
    }

    emitChar(c);
}

std::optional<std::pair<const UTF32*, const UTF32*>> CanonicalFullyDecomposed(UTF32 c)
{
    auto r = MetaLookup<UTF32>(c, CANONICAL_DECOMPOSED_SALT, CANONICAL_DECOMPOSED_KV, PairLookupFk, PairLookupFvOpt,
        std::optional<std::pair<short, short>>{std::nullopt});
    if (!r) {
        return std::nullopt;
    }
    return std::pair{CANONICAL_DECOMPOSED_CHARS + r->first, CANONICAL_DECOMPOSED_CHARS + r->first + r->second};
}

template <class F>
void DecomposeCanonical(UTF32 c, F&& emitChar)
{
    Decompose(c, CanonicalFullyDecomposed, emitChar);
}

/**
 Range class, upper exclusive.
 */
template <class T>
struct Range {
    static_assert(std::is_integral_v<T>);
    T begin;
    T end;
};

template <class T>
std::ostream& operator<<(std::ostream& out, const std::optional<T>& value)
{
    if (value) {
        out << std::string_view{"Some("};
        out << *value;
        out << std::string_view{")"};
    } else {
        out << std::string_view{"None"};
    }
    return out;
}
template <class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& value)
{
    for (auto v:value) {
        out<<v<<',';
    }
    return out;
}

struct Decompositions {
    const UTF8* begin;
    const UTF8* end;
    std::vector<std::pair<uint_fast8_t, UTF32>> buffer;
    Range<size_t> ready;

    explicit Decompositions(const std::string& s): begin{reinterpret_cast<const UTF8*>(s.c_str())},
        end{reinterpret_cast<const UTF8*>(s.c_str() + s.size())}, buffer{}, ready{0, 0} {}
    Decompositions(const UTF8* b, const UTF8* e) : begin{b}, end{e}, buffer{}, ready{0, 0} {}
    
    std::vector<UTF32> Get() &&
    {
        cur = begin;
        std::vector<UTF32> res;
        std::optional<UTF32> k;
        while ((k = Next())) {
            res.push_back(*k);
        }
        return res;
    }

    std::optional<UTF32> Next()
    {
        while (ready.end == 0) {
            if (cur < end) {
                UTF32 codepoint{ReadOneUnicodeChar(&cur, end)};
                DecomposeCanonical(codepoint, [this](UTF32 p) { PushBack(p); });
            } else {
                if (buffer.empty()) {
                    return {};
                } else {
                    SortPending();
                    ready.end = buffer.size();
                    break;
                }
            }
        }
        auto ch = buffer[ready.begin].second;
        IncrementNextReady();
        return {ch};
    }

private:
    const UTF8* cur{nullptr};

    void PushBack(UTF32 c)
    {
        auto cc = GetCanonicalCombiningClass(c);
        if (cc == 0) {
            SortPending();
            buffer.push_back({cc, c});
            ready.end = buffer.size();
        } else {
            buffer.push_back({cc, c});
        }
    }

    static bool F(const std::pair<UTF8, char>& a, const std::pair<UTF8, char>& b)
    {
        return a.first < b.first;
    }

    void SortPending()
    {
        if (ready.end < buffer.size()) {
            std::stable_sort(buffer.begin() + static_cast<long>(ready.end), buffer.end(), F);
        }
    }

    void ResetBuffer()
    {
        auto pending = buffer.size() - ready.end;
        for (size_t i{0}; i < pending; ++i) {
            buffer[i] = buffer[i + ready.end];
        }
        buffer.erase(buffer.begin() + static_cast<long>(pending), buffer.end());
        ready = {0, 0};
    }

    void IncrementNextReady()
    {
        auto next = ready.begin + 1;
        if (next == ready.end) {
            ResetBuffer();
        } else {
            ready.begin = next;
        }
    }
};

std::optional<UTF32> Compose(UTF32 a, UTF32 b)
{
    if (auto h = ComposeHangul(a, b)) {
        return h;
    }
    return CompositionTable(a, b);
}

struct RecompositionState {
    size_t sz;
    enum class V {
        COMPOSING,
        PURGING,
        FINISHED
    } v;
};

struct Recompositions {
    ArrayRef<UTF32> source;
    RecompositionState state;
    std::vector<UTF32> buffer;
    std::optional<UTF32> composee;
    std::optional<short> lastccc;

    explicit Recompositions(ArrayRef<UTF32> s): source{s}, state{0, RecompositionState::V::COMPOSING} {}

    std::string Get() &&
    {
        std::string res{};
        std::optional<UTF32> c{};
        cur = source.begin();
        while ((c = Next())) {
            char r[UNI_MAX_UTF8_BYTES_PER_CODE_POINT];
            char* endPtr{r};
            bool convRes = ConvertCodepointToUTF8(*c, endPtr);
            CODEC_ASSERT(convRes);
            for (char* k{r}; k < endPtr; ++k) {
                res.push_back(*k);
            }
        }
        return res;
    }

private:
    // current position of inner iterator
    const UTF32* cur{nullptr};

    std::optional<UTF32> Next()
    {
        auto end{source.end()};
        while (true) {
#if defined __GNUC__ && not defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#endif
            switch (state.v) {
                case RecompositionState::V::COMPOSING: {
                    while (cur != end) {
                        UTF32 ch = *cur++;
                        auto cc = GetCanonicalCombiningClass(ch);
                        UTF32 k;
                        if (composee) {
                            k = *composee;
                        } else {
                            if (cc != 0) {
                                return {ch};
                            }
                            composee = {ch};
                            continue;
                        }
                        if (!lastccc) {
                            auto comp = Compose(k, ch);
                            if (comp) {
                                composee = comp;
                                continue;
                            } else {
                                if (cc == 0) {
                                    composee = {ch};
                                    return {k};
                                }
                                buffer.push_back(ch);
                                lastccc = {cc};
                            }
                        } else {
                            auto lclass = *lastccc;
                            if (lclass >= cc) {
                                // ch is blocked from composee
                                if (cc == 0) {
                                    composee = {ch};
                                    lastccc = {};
                                    state = {0, RecompositionState::V::PURGING};
                                    return {k};
                                }
                                buffer.push_back(ch);
                                lastccc = {cc};
                                continue;
                            }
                            auto comp = Compose(k, ch);
                            if (comp) {
                                composee = comp;
                                continue;
                            } else {
                                buffer.push_back(ch);
                                lastccc = cc;
                            }
                        }
                    }
                    state = {0, RecompositionState::V::FINISHED};
                    if (composee) {
                        auto b = composee;
                        composee = {};
                        return b;
                    }
                    break;
                }
                case RecompositionState::V::PURGING:
                    if (buffer.size() <= state.sz) {
                        buffer.clear();
                        state.v = RecompositionState::V::COMPOSING;
                    } else {
                        return buffer[state.sz++];
                    }
                    break;
                case RecompositionState::V::FINISHED:
                    if (buffer.size() <= state.sz) {
                        buffer.clear();
                        auto comp = composee;
                        composee = {};
                        return comp;
                    } else {
                        return buffer[state.sz++];
                    }
            }
        }
#if defined __GNUC__ && not defined __clang__
#pragma GCC diagnostic pop
#endif
    }
};
}

std::string CanonicalDecompose(const std::string& s)
{
    Decompositions d{s};
    auto decomposed = std::move(d).Get();
    std::string res;
    ConvertUTF32ToUTF8String(decomposed, res);
    return res;
}

std::string CanonicalRecompose(const std::string& s)
{
    Decompositions d{s};
    auto decomposed = std::move(d).Get();
    Recompositions r{decomposed};
    return std::move(r).Get();
}

std::string CanonicalRecompose(const UTF8* begin, const UTF8* end)
{
    Decompositions d{begin, end};
    auto decomposed = std::move(d).Get();
    Recompositions r{decomposed};
    return std::move(r).Get();
}
}
