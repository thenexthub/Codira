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

#ifndef ABCKIT_WRAPPER_VISITOR_WRAPPER_H
#define ABCKIT_WRAPPER_VISITOR_WRAPPER_H

#include <memory>

namespace abckit_wrapper {

/**
 * @brief VisitorWrapper
 * wrap another visitor
 */
class VisitorWrapper {
public:
    /**
     * @brief VisitorWrapper default Destructor
     */
    virtual ~VisitorWrapper() = default;
};

/**
 * @brief BaseVisitorWrapper
 * @tparam T wrapped visitor type
 */
template <typename T>
class BaseVisitorWrapper : public VisitorWrapper {
public:
    /**
     * BaseVisitorWrapper Constructor
     * @param visitor wrapped visitor
     */
    explicit BaseVisitorWrapper(T &visitor) : visitor_(visitor) {}

protected:
    T &visitor_;
};

/**
 * @brief WrappedVisitor
 * visitor can be wrapped by another visitor
 * @tparam Derived
 */
template <typename Derived>
class WrappedVisitor {
public:
    /**
     * @brief WrappedVisitor default Destructor
     */
    virtual ~WrappedVisitor() = default;

    /**
     * @brief Wrap another visitor
     * @tparam T wrapper visitor type
     * @tparam Args wrapper visitor constructor params type
     * @param args wrapper visitor constructor params
     * @return wrapped visitor
     */
    template <typename T, typename... Args>
    T &Wrap(Args... args)
    {
        auto wrapper = std::make_unique<T>(*static_cast<Derived *>(this), std::forward<Args>(args)...);
        auto wrapperPtr = wrapper.get();
        this->wrapper_ = std::move(wrapper);
        return *wrapperPtr;
    }

private:
    std::unique_ptr<VisitorWrapper> wrapper_;
};

}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_VISITOR_WRAPPER_H
