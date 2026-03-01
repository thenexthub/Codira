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

#ifndef CODIRA_CHIR_ANALYSIS_FLAT_SET_H
#define CODIRA_CHIR_ANALYSIS_FLAT_SET_H

#include "Codira/CHIR/Analysis/Analysis.h"

namespace Codira::CHIR {

/**
 * @brief abstract domain for pass reach definition analysis
 * @tparam T abstract value
 */
template <typename T> class FlatSet : AbstractDomain<FlatSet<T>> {
public:
    /**
     * @brief domain kind, Bottom means non-initialization state or non-reachable state, top means all possible state,
     * Elem means normal state.
     */
    enum class FlatSetKind : uint8_t { Bottom, Elem, Top };

    /**
     * @brief constructor of Flat Set domain.
     * @param isTop flag whether create a top domain, otherwise a bottom one.
     */
    explicit FlatSet(bool isTop) : flatSetKind(isTop ? FlatSetKind::Top : FlatSetKind::Bottom), elem(nullptr)
    {
    }

    /**
     * @brief constructor of Flat Set domain.
     * @param elem normal value state.
     */
    explicit FlatSet(T elem) : flatSetKind(FlatSetKind::Elem), elem(elem)
    {
    }

    /**
     * @brief destructor.
     */
    virtual ~FlatSet()
    {
    }

    /**
     * @brief join domains of flat set.
     * @param rhs other domain to join with.
     * @return whether changed after join.
     */
    bool Join(const FlatSet<T>& rhs) override
    {
        if (flatSetKind == FlatSetKind::Top || rhs.flatSetKind == FlatSetKind::Bottom) {
            return false;
        }

        if (rhs.flatSetKind == FlatSetKind::Top) {
            flatSetKind = FlatSetKind::Top;
            return true;
        }

        if (flatSetKind == FlatSetKind::Bottom) {
            *this = rhs;
            return true;
        }

        if (this->elem == rhs.elem) {
            return false;
        } else {
            flatSetKind = FlatSetKind::Top;
            return true;
        }
    }

    /// judge whether abstract domain is bottom.
    bool IsBottom() const override
    {
        return flatSetKind == FlatSetKind::Bottom;
    }

    /// judge whether abstract domain is top.
    bool IsTop() const
    {
        return flatSetKind == FlatSetKind::Top;
    }

    /// output string of flat set abstract domain.
    std::string ToString() const override
    {
        if (flatSetKind == FlatSetKind::Top) {
            return "top";
        } else if (flatSetKind == FlatSetKind::Bottom) {
            return "bottom";
        } else {
            CODEC_NULLPTR_CHECK(elem);
            return elem->ToString();
        }
    }

    /// set to top or bottom
    void SetToBound(bool isTop)
    {
        flatSetKind = isTop ? FlatSetKind::Top : FlatSetKind::Bottom;
    }

    /// get elem value
    std::optional<T*> GetElem() const
    {
        if (flatSetKind == FlatSetKind::Elem) {
            return elem;
        } else {
            return std::nullopt;
        }
    }

    // update elem value to domain
    void UpdateElem(T* ele)
    {
        flatSetKind = FlatSetKind::Elem;
        elem = ele;
    }

protected:
    explicit FlatSet() : AbstractDomain<FlatSet<T>>()
    {
    }

    FlatSetKind flatSetKind;
    T* elem;
};

} // namespace Codira::CHIR

#endif
