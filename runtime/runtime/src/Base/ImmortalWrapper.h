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


#ifndef MRT_IMMORTAL_WRAPPER_H
#define MRT_IMMORTAL_WRAPPER_H

#include <new>
#include <utility>

namespace MapleRuntime {
// utility class to avoid un-ordered static global destruction
template<class T>
class ImmortalWrapper {
public:
    using pointer = typename std::add_pointer<T>::type;
    using lref = typename std::add_lvalue_reference<T>::type;

    template<class... Args>
    explicit ImmortalWrapper(Args&&... args)
    {
        new (buffer) T(std::forward<Args>(args)...);
    }
    ImmortalWrapper(const ImmortalWrapper&) = delete;
    ImmortalWrapper& operator=(const ImmortalWrapper&) = delete;
    ~ImmortalWrapper() = default;
    inline pointer operator->() { return reinterpret_cast<pointer>(buffer); }

    inline lref operator*() { return reinterpret_cast<lref>(buffer); }

private:
    alignas(T) unsigned char buffer[sizeof(T)] = { 0 };
};
} // namespace MapleRuntime
#endif // MRT_IMMORTAL_WRAPPER_H
