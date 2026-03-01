/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_VERIF_PARSER_H_
#define PANDA_VERIF_PARSER_H_

#include "charset.h"
#include "util/callable.h"

namespace ark::parser {

template <typename...>
struct type_sum;

template <typename...>
struct type_prod;

struct op_next;
struct op_end;
struct op_optional;
struct op_action;
struct op_sequence;
struct op_or;
struct op_and;
struct op_lookup;
struct op_not;
struct op_times;
struct op_times_ref;

template <typename A, typename B, typename C>
struct IfType {
};

template <typename A, typename C>
struct IfType<A, A, C> {
    using Type = C;
};

template <typename T>
T &RefTo();

template <typename T>
T ValOf();

enum class Action { START, PARSED, CANCEL };

template <typename Context, typename Type, typename Char, typename Iter>
struct BaseParser : public verifier::callable<bool(Context &, Iter &, Iter)> {
    template <typename F>
    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr BaseParser(const F &f) : verifier::callable<bool(Context &, Iter &, Iter)> {f}
    {
    }

    using Ctx = Context;
    using T = Type;

    BaseParser() = default;

    template <typename NType>
    using Next = BaseParser<Context, type_sum<NType, T>, Char, Iter>;

    using P = BaseParser<Context, type_sum<op_next, T>, Char, Iter>;

    static Next<Charset<Char>> OfCharset(const Charset<Char> &c)
    {
        static const auto L = [c](Context &, Iter &start, Iter end) {
            Iter s = start;
            while (s != end && c(*s)) {
                s++;
            }
            bool result = (s != start);
            if (result) {
                start = s;
            }
            return result;
        };
        return L;
    }

    static Next<Char *> OfString(Char *str)
    {
        static const auto L = [=](Context &, Iter &start, Iter end) {
            Iter s = start;
            Iter c = str;
            while (s != end && *c != 0 && *c == *s) {
                ++c;
                ++s;
            }
            bool result = (*c == 0);
            if (result) {
                start = s;
            }
            return result;
        };
        return L;
    }

    static Next<Char *> SkipComment(Char *str)
    {
        static const auto L = [=](Context &, Iter &start, Iter end) {
            Iter s = start;
            Iter c = str;
            while (s != end && isspace(*s)) {
                s++;
            }
            while (s != end && *c != '\0' && *c == *s) {
                c++;
                s++;
            }
            bool result = (*c == '\0');
            if (result) {
                // skip chars until end of string
                while (s != end && *s != '\r' && *s != '\n') {
                    s++;
                }
                start = s;
            }
            return result;
        };
        return L;
    }

    static Next<op_end> End()
    {
        static const auto L = [](Context &, Iter &start, Iter end) { return start == end; };
        return L;
    }

    Next<op_optional> operator~() &&
    {
        static const auto L = [p = *this](Context &c, Iter &start, Iter end) {
            p(c, start, end);
            return true;
        };
        return L;
    }

    Next<op_optional> operator~() const &
    {
        static const auto L = [this](Context &c, Iter &start, Iter end) {
            (*this)(c, start, end);
            return true;
        };
        return L;
    }

    template <typename F>
    auto operator|=(F f) const ->
        typename IfType<decltype(f(Action::START, RefTo<Context>(), RefTo<Iter>(), ValOf<Iter>(), ValOf<Iter>())), bool,
                        Next<type_prod<op_action, F>>>::Type
    {
        static const auto L = [p = *this, f](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (!f(Action::START, c, start, start, end)) {
                start = saved;
                return false;
            }
            bool result = p(c, start, end);
            auto newSaved = saved;
            if (!result) {
                f(Action::CANCEL, c, newSaved, start, end);
                start = saved;
                return false;
            }
            if (!f(Action::PARSED, c, newSaved, start, end)) {
                start = saved;
                return false;
            }
            return true;
        };
        return L;
    }

    template <typename F>
    auto operator|=(F f) const ->
        typename IfType<decltype(f(Action::START, RefTo<Context>(), RefTo<Iter>(), ValOf<Iter>())), bool,
                        Next<type_prod<op_action, F>>>::Type
    {
        static const auto L = [p = *this, f](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (!f(Action::START, c, start, start)) {
                start = saved;
                return false;
            }
            bool result = p(c, start, end);
            auto newSaved = saved;
            if (!result) {
                f(Action::CANCEL, c, newSaved, start);
                start = saved;
                return false;
            }
            if (!f(Action::PARSED, c, newSaved, start)) {
                start = saved;
                return false;
            }
            return true;
        };
        return L;
    }

    template <typename F>
    auto operator|=(F f) const -> typename IfType<decltype(f(Action::START, RefTo<Context>(), RefTo<Iter>())), bool,
                                                  Next<type_prod<op_action, F>>>::Type
    {
        static const auto L = [p = *this, f](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (!f(Action::START, c, start)) {
                start = saved;
                return false;
            }
            bool result = p(c, start, end);
            auto newSaved = saved;
            if (!result) {
                f(Action::CANCEL, c, newSaved);
                start = saved;
                return false;
            }
            if (!f(Action::PARSED, c, newSaved)) {
                start = saved;
                return false;
            }
            return true;
        };
        return L;
    }

    template <typename F>
    auto operator|=(F f) const ->
        typename IfType<decltype(f(Action::START, RefTo<Context>())), bool, Next<type_prod<op_action, F>>>::Type
    {
        static const auto L = [p = *this, f](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (!f(Action::START, c)) {
                start = saved;
                return false;
            }
            bool result = p(c, start, end);
            if (!result) {
                f(Action::CANCEL, c);
                start = saved;
                return false;
            }
            if (!f(Action::PARSED, c)) {
                start = saved;
                return false;
            }
            return true;
        };
        return L;
    }

    template <typename P>
    Next<type_prod<op_sequence, typename P::T>> operator>>(P pa) const
    {
        static const auto L = [left = *this, right = pa](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (left(c, start, end) && right(c, start, end)) {
                return true;
            }
            start = saved;
            return false;
        };
        return L;
    }

    template <typename P>
    Next<type_prod<op_or, typename P::T>> operator|(P pa) const
    {
        static const auto L = [left = *this, right = pa](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (left(c, start, end)) {
                return true;
            }
            start = saved;
            if (right(c, start, end)) {
                return true;
            }
            start = saved;
            return false;
        };
        return L;
    }

    template <typename P>
    Next<type_prod<op_and, typename P::T>> operator&(P pa) const
    {
        static const auto L = [left = *this, right = pa](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (left(c, start, end)) {
                start = saved;
                return right(c, start, end);
            }
            start = saved;
            return false;
        };
        return L;
    }

    template <typename P>
    Next<type_prod<op_lookup, typename P::T>> operator<<(P pa) const
    {
        static const auto L = [left = *this, right = pa](Context &c, Iter &start, Iter end) {
            auto saved1 = start;
            if (left(c, start, end)) {
                auto saved2 = start;
                if (right(c, start, end)) {
                    start = saved2;
                    return true;
                }
            }
            start = saved1;
            return false;
        };
        return L;
    }

    Next<op_not> operator!() &&
    {
        static const auto L = [p = *this](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if (p(c, start, end)) {
                start = saved;
                return false;
            }
            start = saved;
            return true;
        };
        return L;
    }

    Next<op_not> operator!() const &
    {
        static const auto L = [this](Context &c, Iter &start, Iter end) {
            auto saved = start;
            if ((*this)(c, start, end)) {
                start = saved;
                return false;
            }
            start = saved;
            return true;
        };
        return L;
    }

    Next<op_times> operator*() &&
    {
        static const auto L = [p = *this](Context &c, Iter &start, Iter end) {
            while (true) {
                auto saved = start;
                if (!p(c, start, end)) {
                    start = saved;
                    break;
                }
            }
            return true;
        };
        return L;
    }

    Next<op_times_ref> operator*() const &
    {
        static const auto L = [this](Context &c, Iter &start, Iter end) {
            while (true) {
                auto saved = start;
                if (!(*this)(c, start, end)) {
                    start = saved;
                    break;
                }
            }
            return true;
        };
        return L;
    }
};

struct initial;

template <typename Context, typename Char, typename Iter>
using Parser = BaseParser<Context, initial, Char, Iter>;

}  // namespace ark::parser

#endif  // PANDA_VERIF_PARSER_H_
