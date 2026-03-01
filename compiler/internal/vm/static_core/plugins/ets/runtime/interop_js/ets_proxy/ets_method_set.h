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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_SET_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_SET_H

#include "runtime/include/runtime.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {
class EtsClass;
}  // namespace ark::ets

namespace ark::ets::interop::js::ets_proxy {

// Set of methods that have the same name but different signatures
class EtsMethodSet {
public:
    static EtsMethodSet Create(EtsMethod *singleMethod);
    static EtsMethodSet Create(EtsMethod *singleMethod, const std::string &jsName);

    static EtsMethodSet Create(Method *singleMethod)
    {
        return Create(EtsMethod::FromRuntimeMethod(singleMethod));
    }

    const char *GetName() const;

    bool IsValid() const
    {
        return isValid_;
    }

    bool IsSetter()
    {
        return strncmp(GetName(), SETTER_BEGIN, strlen(SETTER_BEGIN)) == 0;
    }

    bool IsGetter()
    {
        return strncmp(GetName(), GETTER_BEGIN, strlen(GETTER_BEGIN)) == 0;
    }

    bool IsStatic() const
    {
        return anyMethod_->IsStatic();
    }

    bool IsConstructor() const
    {
        return anyMethod_->IsConstructor();
    }

    ALWAYS_INLINE EtsMethod *GetMethod(uint32_t parametersNum) const
    {
        // Try optional parameters
        uint32_t tempParameter = parametersNum;
        while (LIKELY(tempParameter < entries_.size())) {
            if (LIKELY(entries_[tempParameter] != nullptr)) {
                return entries_[tempParameter];
            }
            tempParameter++;
        }
        // Try rest params
        for (size_t params = std::min(static_cast<size_t>(parametersNum), entries_.size() - 1); params > 0; params--) {
            if (entries_[params] != nullptr && entries_[params]->GetPandaMethod()->HasVarArgs()) {
                return entries_[params];
            }
        }
        return entries_.front();
    }

    template <typename Callback>
    void EnumerateMethods(const Callback &callback) const
    {
        for (auto m : entries_) {
            if (nullptr != m) {
                callback(m);
            }
        }
    }

    EtsClass *GetEnclosingClass() const
    {
        return enclosingClass_;
    }

    void SetBaseMethodSet(EtsMethodSet *baseMethodSet)
    {
        baseMethodSet_ = baseMethodSet;
    }

    EtsMethodSet *GetBaseMethodSet() const
    {
        return baseMethodSet_;
    }

    void MergeWith(const EtsMethodSet &other);

private:
    explicit EtsMethodSet(EtsMethod *singleMethod, EtsClass *enclosingClass)
        : enclosingClass_(enclosingClass),
#ifndef NDEBUG
          name_(singleMethod->GetFullName(false)),
#endif
          entries_(PandaVector<EtsMethod *>(singleMethod->GetParametersNum() + 1)),
          anyMethod_(singleMethod),
          jsPublicName_(singleMethod->GetName())
    {
        entries_[singleMethod->GetParametersNum() -
                 static_cast<unsigned int>(singleMethod->GetPandaMethod()->HasVarArgs())] = singleMethod;
    }

    EtsMethodSet(EtsMethod *singleMethod, EtsClass *enclosingClass, std::string jsName)
        : enclosingClass_(enclosingClass),
#ifndef NDEBUG
          name_(singleMethod->GetFullName(false)),
#endif
          entries_(PandaVector<EtsMethod *>(singleMethod->GetParametersNum() + 1)),
          anyMethod_(singleMethod),
          jsPublicName_(std::move(jsName))
    {
        entries_[singleMethod->GetParametersNum() -
                 static_cast<unsigned int>(singleMethod->GetPandaMethod()->HasVarArgs())] = singleMethod;
    }

    EtsClass *const enclosingClass_;

#ifndef NDEBUG
    std::string name_;  // just for GDB
#endif

    // Index number in vector is a number of paramaters for O(1) lookup
    PandaVector<EtsMethod *> entries_;

    // Abritrary item from set to get common properties
    const EtsMethod *const anyMethod_;
    std::string jsPublicName_;

    bool isValid_ = true;

    EtsMethodSet *baseMethodSet_ = nullptr;
};

template <class Iterator>
PandaVector<Method *> CollectAllPandaMethods(Iterator begin, Iterator end)
{
    PandaVector<Method *> allMethods;
    allMethods.reserve(std::distance(begin, end));
    for (Iterator it = begin; it != end; ++it) {
        allMethods.push_back((*it)->GetPandaMethod());
    }
    return allMethods;
}

}  // namespace ark::ets::interop::js::ets_proxy

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_ETS_METHOD_SET_H
