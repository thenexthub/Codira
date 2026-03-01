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

#ifndef CPP_ABCKIT_BASE_CLASSES_H
#define CPP_ABCKIT_BASE_CLASSES_H

#include "config.h"

#include <memory>
#include <type_traits>

namespace abckit {

// Interface to provide global API-related features,
// a base for every API class defined
/**
 * @brief Entity
 */
class Entity {
public:
    /**
     * @brief Copy constructor allowed
     */
    Entity(const Entity &) = default;

    /**
     * @brief Copy assignment allowed
     * @return Entity
     */
    Entity &operator=(const Entity &) = default;

    /**
     * @brief Move constructor allowed
     */
    Entity(Entity &&) = default;

    /**
     * @brief Move assignment allowed
     * @return Entity
     */
    Entity &operator=(Entity &&) = default;

    /**
     * @brief Default constructor
     */
    Entity() = default;

    /**
     * @brief Destructor
     */
    virtual ~Entity() = default;

    /**
     * @brief Get api config
     * @return ApiConfig
     */
    virtual const ApiConfig *GetApiConfig() const = 0;
};

/**
 * @brief View
 */
template <typename T, typename = std::enable_if_t<std::is_pointer_v<T>>>
class View : public Entity {
public:
    /**
     * @brief Operator ==
     * @param rhs
     * @return bool
     */
    bool operator==(const View<T> &rhs) const
    {
        return GetView() == rhs.GetView();
    }

    /**
     * @brief Operator !=
     * @param rhs
     * @return bool
     */
    bool operator!=(const View<T> &rhs) const
    {
        return GetView() != rhs.GetView();
    }

    /**
     * @brief Operator bool
     * @return bool
     */
    explicit operator bool() const
    {
        return view_ != nullptr;
    }

protected:
    /**
     * @brief Constructor
     * @param ...a
     */
    template <typename... Args>
    explicit View(Args &&...a) : view_(std::forward<Args>(a)...)
    {
    }
    // Can move and copy views

    /**
     * @brief Copy constructor
     */
    View(const View &) = default;

    /**
     * @brief Copy assignment
     * @return View
     */
    View &operator=(const View &) = default;

    /**
     * @brief Move constructor
     */
    View(View &&) = default;

    /**
     * @brief Move assignment
     * @return View
     */
    View &operator=(View &&) = default;

protected:
    ~View() override = default;

    /**
     * @brief Get view
     * @return T
     */
    T GetView() const
    {
        return view_;
    }

    /**
     * @brief Set view
     * @param newView
     */
    void SetView(T newView)
    {
        view_ = newView;
    }

private:
    T view_;
};

/**
 * @brief ViewInResource
 */
template <typename T, typename R, typename = std::enable_if_t<std::is_pointer_v<R>>>
class ViewInResource : public View<T> {
public:
    /**
     * @brief Operator bool
     * @return bool
     */
    explicit operator bool() const
    {
        return resource_ != nullptr && *static_cast<const View<T> *>(this);
    }

protected:
    /**
     * @brief Constructor
     * @param ...a
     */
    template <typename... Args>
    explicit ViewInResource(Args &&...a) : View<T>(std::forward<Args>(a)...)
    {
    }

    // Can move and copy views in resource

    /**
     * @brief Copy constructor
     */
    ViewInResource(const ViewInResource &) = default;

    /**
     * @brief Copy assignment
     * @return ViewInResource
     */
    ViewInResource &operator=(const ViewInResource &) = default;

    /**
     * @brief Move constructor
     */
    ViewInResource(ViewInResource &&) = default;

    /**
     * @brief Move assignment
     * @return ViewInResource
     */
    ViewInResource &operator=(ViewInResource &&) = default;

    /**
     * @brief Destructor
     */
    ~ViewInResource() override = default;

    /**
     * @brief Get resource
     * @return R
     */
    R GetResource() const
    {
        return resource_;
    }

    /**
     * @brief Set resource
     * @param newResource
     */
    void SetResource(R newResource)
    {
        resource_ = newResource;
    }

    /**
     * @brief Struct for using in callbacks
     */
    template <typename D>
    struct Payload {
        /**
         * @brief data
         */
        D data;
        /**
         * @brief config
         */
        const ApiConfig *config;
        /**
         * @brief resource
         */
        R resource;
    };

private:
    R resource_;
};

// Resource - ptr semantics
/**
 * @brief Resource
 */
template <typename T>
class Resource : public Entity {
public:
    // No copy for resources
    /**
     * @brief Deleted copy constructor
     */
    Resource(const Resource &) = delete;

    /**
     * @brief Deleted copy assignment
     * @return Resource&
     */
    Resource &operator=(const Resource &) = delete;

    /**
     * @brief Operator ==
     * @param rhs
     * @return bool
     */
    bool operator==(const Resource &rhs) const
    {
        return GetResource() == rhs.GetResource();
    }

protected:
    /**
     * @brief Constructor
     * @param d
     * @param ...a
     */
    template <typename... Args>
    explicit Resource(std::unique_ptr<IResourceDeleter> d, Args &&...a)
        : deleter_(std::move(d)), resource_(std::forward<Args>(a)...)
    {
    }

    /**
     * @brief Constructor
     * @param ...a
     */
    template <typename... Args>
    explicit Resource(Args &&...a) : resource_(std::forward<Args>(a)...)
    {
    }

    // Resources are movable
    /**
     * @brief Move constructor
     * @param other
     */
    Resource(Resource &&other)
    {
        released_ = false;
        resource_ = other.ReleaseResource();
    };

    /**
     * @brief Move assignment
     * @param other
     * @return Resource&
     */
    Resource &operator=(Resource &&other)
    {
        released_ = false;
        resource_ = other.ReleaseResource();
        return *this;
    };

    /**
     * @brief Release resource
     * @return `T`
     */
    T ReleaseResource()
    {
        released_ = true;
        return resource_;
    }

    /**
     * @brief Get resource
     * @return `T`
     */
    T GetResource() const
    {
        return resource_;
    }

    /**
     * @brief Destructor
     */
    ~Resource() override
    {
        if (!released_) {
            deleter_->DeleteResource();
        }
    };

    /**
     * @brief Set deleter
     * @param deleter
     */
    void SetDeleter(std::unique_ptr<IResourceDeleter> deleter)
    {
        deleter_ = std::move(deleter);
    }

private:
    T resource_;
    std::unique_ptr<IResourceDeleter> deleter_ = std::make_unique<DefaultResourceDeleter>();
    bool released_ = false;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_BASE_CLASSES_H
