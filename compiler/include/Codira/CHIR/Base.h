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

#ifndef CODIRA_CHIR_BASE_H
#define CODIRA_CHIR_BASE_H

#include <unordered_map>
#include <typeinfo>

#include "Codira/CHIR/Annotation.h"

namespace Codira::CHIR {
class Base {
public:
    template <typename T, typename... Args> void Set(Args&&... args)
    {
        anno.Set<T>(std::forward<Args>(args)...);
    }

    template <typename T> void Remove()
    {
        anno.Remove<T>();
    }

    // Get the value of the annotation T associated to this node
    template <typename T>
    decltype(std::declval<const AnnotationMap>().Get<T>()) Get() const
    {
        return anno.Get<T>();
    }
    template <class T>
    T& GetAnno()
    {
        return anno.GetAnno<T>();
    }

    virtual const DebugLocation& GetDebugLocation() const { return anno.GetDebugLocation(); }
    inline void SetDebugLocation(const DebugLocation& loc)
    {
        anno.SetDebugLocation(loc);
    }
    inline void SetDebugLocation(DebugLocation&& loc)
    {
        anno.SetDebugLocation(std::move(loc));
    }

    void CopyAnnotationMapFrom(const Base& other)
    {
        anno = other.anno;
    }

    std::string ToStringAnnotationMap() const { return anno.ToString(); }

    const AnnotationMap& GetAnno() const
    {
        return anno;
    }

    AnnotationMap MoveAnnotation()
    {
        return std::move(anno);
    }
    void SetAnnotation(AnnotationMap&& ot)
    {
        anno = std::move(ot);
    }

    virtual ~Base() = default;

private:
    AnnotationMap anno;
};
}
#endif
