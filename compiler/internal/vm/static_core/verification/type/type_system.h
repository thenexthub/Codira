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

#ifndef PANDA_TYPE_SYSTEM_HPP
#define PANDA_TYPE_SYSTEM_HPP

#include "libarkbase/macros.h"
#include "runtime/include/class.h"
#include "runtime/include/language_context.h"
#include "runtime/include/method.h"
#include "runtime/include/mem/panda_containers.h"

#include "verification/type/type_type.h"
#include "verification/value/variables.h"

#include "include/class_linker.h"

#include <memory>
#include <variant>
#include <functional>
#include <algorithm>

namespace ark::verifier::plugin {
class Plugin;
}  // namespace ark::verifier::plugin

namespace ark::verifier {
class VerifierService;

/*
Design decision:
1. There are special initial and final types, named as Bot and Top, and all types are implicitly related
   as Bot <: type <: Top
*/

class TypeSystem {
public:
    // NOTE(vdyadov): change Id to hash from filename_id and entity id
    using TypeId = panda_file::Type::TypeId;

    explicit TypeSystem(VerifierService *service, panda_file::SourceLang lang = panda_file::SourceLang::PANDA_ASSEMBLY);

    NO_COPY_SEMANTIC(TypeSystem);
    DEFAULT_MOVE_SEMANTIC(TypeSystem);
    ~TypeSystem() = default;

    void SetSupertypeOfArray(Type supertype)
    {
        supertypeOfArray_ = supertype;
    }

    Type SupertypeOfArray() const
    {
        return supertypeOfArray_;
    }
    Type Object() const
    {
        return object_;
    }
    Type StringClass() const
    {
        return string_;
    }
    Type ClassClass() const
    {
        return class_;
    }
    Type Throwable() const
    {
        return throwable_;
    }

    Variables::Var NewVar()
    {
        return variables_.NewVar();
    }

    Type NormalizedTypeOf(Type type);
    MethodSignature const *GetMethodSignature(Method const *method);

    PandaUnorderedSet<Type> const *SupertypesOfClass(Class const *klass);

    void DisplayTypeSystem(std::function<void(PandaString const &)> const &handler);

    /* Make TypeSystem remember the class for dumping */
    void MentionClass(Class const *klass);

    Type BootDescriptorToType(uint8_t const *descr);

    void ResetTypeSpans()
    {
        typeSpans_.clear();
    }

    Span<Type const> GetTypeSpan(size_t start, size_t sz) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return {typeSpans_.data() + start, sz};
    }

    Span<Type> GetNewTypeSpan(size_t sz)
    {
        size_t start = typeSpans_.size();
        typeSpans_.resize(start + sz);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return {typeSpans_.data() + start, sz};
    }

    size_t GetSpanIndex(Span<Type> span)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return span.begin() - typeSpans_.data();
    }

private:
    VerifierService *service_;
    plugin::Plugin const *plugin_;
    LanguageContext langCtx_;
    ClassLinkerContext *bootLinkerCtx_;

    PandaUnorderedMap<Type, Type> normalizedTypeOf_;
    PandaUnorderedMap<Method::UniqId, Method const *> methodOfId_;
    PandaUnorderedMap<Method::UniqId, MethodSignature> signatureOfMethod_;
    PandaUnorderedMap<Class const *, PandaUnorderedSet<Type>> supertypesCache_;
    PandaUnorderedSet<Class const *> knownClasses_;

    // Storage for members of intersection and union types.
    PandaVector<Type> typeSpans_;

    Type supertypeOfArray_;
    Type object_;
    Type string_;
    Type class_;
    Type throwable_;
    Variables variables_;

    void ExtendBySupers(PandaUnorderedSet<Type> *set, Class const *klass);

    void DisplayClasses(std::function<void(PandaString const &)> const &handler) const;
    void DisplayMethods(std::function<void(PandaString const &, PandaString const &)> const &handler) const;
    void DisplaySubtyping(std::function<void(PandaString const &, PandaString const &)> const &handler);
};

}  // namespace ark::verifier

#endif  // !PANDA_TYPE_SYSTEM_HPP
