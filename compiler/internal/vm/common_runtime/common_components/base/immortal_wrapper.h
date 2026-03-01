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

#ifndef COMMON_COMPONENTS_BASE_IMMORTAL_WRAPPER_H
#define COMMON_COMPONENTS_BASE_IMMORTAL_WRAPPER_H

#include <new>
#include <utility>

namespace common {
// Utility class ensuring ordered destruction of static global objects to prevent dependency-related issues during
// program termination.
template<class Type>
class ImmortalWrapper {
public:
    template<class... Args>
    ImmortalWrapper(Args&&... args)
    {
        new (buffer_) Type(std::forward<Args>(args)...);
    }
    ImmortalWrapper(const ImmortalWrapper&) = delete;
    ImmortalWrapper& operator=(const ImmortalWrapper&) = delete;
    ~ImmortalWrapper() = default;
    inline typename std::add_pointer<Type>::type operator->()
    {
        return reinterpret_cast<typename std::add_pointer<Type>::type>(buffer_);
    }

    inline typename std::add_lvalue_reference<Type>::type operator*()
    {
        return reinterpret_cast<typename std::add_lvalue_reference<Type>::type>(buffer_);
    }

private:
    alignas(Type) unsigned char buffer_[sizeof(Type)] = { 0 };
};
} // namespace common
#endif // COMMON_COMPONENTS_BASE_IMMORTAL_WRAPPER_H
