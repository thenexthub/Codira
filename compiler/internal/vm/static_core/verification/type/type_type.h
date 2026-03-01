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

#ifndef PANDA_VERIFICATION_TYPE_TYPE_TYPE_H
#define PANDA_VERIFICATION_TYPE_TYPE_TYPE_H

#include "verification/util/mem.h"

#include "libarkbase/macros.h"
#include "runtime/include/class.h"
#include "runtime/include/method.h"

#include <variant>

namespace ark::verifier {
class TypeSystem;

class Type {
    /* Invariant:
       Intersections can only contain (builtins or classes).
       Unions can only contain intersections or elementary types.
       Top and Bot cannot be parts of intersections or unions.
    */
public:
    enum Builtin : size_t {
        TOP = 1,
        U1,
        I8,
        U8,
        I16,
        U16,
        I32,
        U32,
        F32,
        F64,
        I64,
        U64,
        INTEGRAL8,
        INTEGRAL16,
        INTEGRAL32,
        INTEGRAL64,
        FLOAT32,
        FLOAT64,
        BITS32,
        BITS64,
        PRIMITIVE,
        REFERENCE,
        NULL_REFERENCE,
        OBJECT,
        ARRAY,
        BOT,

        LAST
    };

public:
    Type() = default;
    ~Type() = default;
    DEFAULT_COPY_SEMANTIC(Type);
    DEFAULT_MOVE_SEMANTIC(Type);

    explicit Type(Builtin builtin) : content_ {builtin} {}
    explicit Type(Class const *klass) : content_ {reinterpret_cast<uintptr_t>(klass)}
    {
        if (klass->IsPrimitive()) {
            *this = FromTypeId(klass->GetType().GetId());
        }
    }

    PandaString ToString(TypeSystem const *tsys) const;

private:
    explicit Type(uintptr_t content) : content_ {content} {}

    static Type Intersection(Span<Type> span, TypeSystem *tsys);
    static Type Union(Span<Type> span, TypeSystem *tsys);
    PandaString IntersectionToString(TypeSystem const *&tsys) const;
    PandaString UnionToString(TypeSystem const *&tsys) const;

private:
    static int constexpr INTERSECTION_TAG = 1;
    static int constexpr UNION_TAG = 2;

    static size_t constexpr BITS_FOR_SPAN_SIZE = 8;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    static size_t constexpr SPAN_MASK = (1 << BITS_FOR_SPAN_SIZE) - 1;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    static size_t constexpr MAX_SPAN_SIZE = (1 << BITS_FOR_SPAN_SIZE) - 1;

    static size_t SpanSize(uintptr_t v)
    {
        return v & SPAN_MASK;
    }

    static size_t SpanIndex(uintptr_t v)
    {
        return v >> BITS_FOR_SPAN_SIZE;
    }

    static uintptr_t ConstructPayload(size_t spanSize, size_t spanIndex)
    {
        ASSERT(spanSize > 0);
        ASSERT(spanSize <= MAX_SPAN_SIZE);
        return (spanIndex << BITS_FOR_SPAN_SIZE) | spanSize;
    }

public:
    static Type FromTypeId(panda_file::Type::TypeId tid);

    static Type Bot()
    {
        return Type {Builtin::BOT};
    }
    static Type Top()
    {
        return Type {Builtin::TOP};
    }

    ALWAYS_INLINE bool IsNone() const
    {
        return content_ == 0;
    }
    ALWAYS_INLINE bool IsValid() const
    {
        return content_ != 0;
    }
    ALWAYS_INLINE bool IsBuiltin() const
    {
        return IsValid() && content_ < Builtin::LAST;
    }
    ALWAYS_INLINE bool IsClass() const
    {
        return IsRTClass() && !reinterpret_cast<Class const *>(content_)->IsUnionClass();
    }
    ALWAYS_INLINE bool IsIntersection() const
    {
        return IsNotPointer(content_) && GetTag(content_) == INTERSECTION_TAG;
    }
    ALWAYS_INLINE bool IsUnion() const
    {
        return (IsRTClass() && GetRTClass()->IsUnionClass()) ||
               (IsNotPointer(content_) && GetTag(content_) == UNION_TAG);
    }

    ALWAYS_INLINE Builtin GetBuiltin() const
    {
        ASSERT(IsBuiltin());
        return static_cast<Builtin>(content_);
    }
    ALWAYS_INLINE Class const *GetClass() const
    {
        ASSERT(IsClass());
        return GetRTClass();
    }

    bool IsConsistent() const;

    panda_file::Type::TypeId ToTypeId() const;

    size_t GetTypeWidth() const;

    Type GetArrayElementType(TypeSystem *tsys) const;

    // Careful: span is invalidated whenever a new intersection or union is created.
    Span<Type const> GetIntersectionMembers(TypeSystem const *tsys) const;
    Span<Type const> GetUnionMembers(TypeSystem const *tsys) const;

private:
    uintptr_t content_ {0};

    ALWAYS_INLINE bool IsRTClass() const
    {
        return IsValid() && !IsBuiltin() && IsPointer(content_);
    }
    ALWAYS_INLINE Class const *GetRTClass() const
    {
        ASSERT(IsRTClass());
        return reinterpret_cast<Class const *>(content_);
    }

    static Type IntersectSpans(Span<Type const> lhs, Span<Type const> rhs, TypeSystem *tsys);

    friend bool IsSubtypeImpl(Type lhs, Type rhs, TypeSystem *tsys);
    friend Type TpIntersection(Type lhs, Type rhs, TypeSystem *tsys);
    friend Type TpUnion(Type lhs, Type rhs, TypeSystem *tsys);
    friend struct std::hash<Type>;
    friend bool operator==(Type lhs, Type rhs);
};

ALWAYS_INLINE inline bool operator==(Type lhs, Type rhs)
{
    return lhs.content_ == rhs.content_;
}

ALWAYS_INLINE inline bool operator!=(Type lhs, Type rhs)
{
    return !(lhs == rhs);
}

ALWAYS_INLINE inline bool IsSubtype(Type lhs, Type rhs, TypeSystem *tsys)
{
    return lhs == rhs || IsSubtypeImpl(lhs, rhs, tsys);
}

struct MethodSignature {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    PandaVector<Type> args;
    Type result;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    PandaString ToString(TypeSystem const *tsys) const
    {
        std::stringstream ss;
        ss << "(";
        bool first = true;
        for (auto const &arg : args) {
            if (first) {
                first = false;
            } else {
                ss << ", ";
            }
            ss << arg.ToString(tsys);
        }
        ss << ") -> ";
        ss << result.ToString(tsys);
        return PandaString(ss.str());
    }
};

}  // namespace ark::verifier

namespace std {
template <>
struct hash<ark::verifier::Type> {
    size_t operator()(ark::verifier::Type tp) const
    {
        return hash<uintptr_t>()(tp.content_);
    }
};
}  // namespace std

#endif  // PANDA_VERIFICATION_TYPE_TYPE_TYPE_H
