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

#ifndef CPP_ABCKIT_UTILS_H
#define CPP_ABCKIT_UTILS_H

#include "../../c/statuses.h"

#include <string>

#ifdef ABCKIT_USE_EXCEPTIONS
#include <stdexcept>
#endif

namespace abckit {

class ApiConfig;

/**
 * @brief Status to status desc str
 * @param status AbckitStatus
 * @return status desc str
 */
// CC-OFFNXT(G.FUD.06) perf critical
// NOLINTBEGIN(performance-unnecessary-value-param)
inline std::string StatusToString(AbckitStatus status)
{
    switch (status) {
        case ABCKIT_STATUS_NO_ERROR:
            return "No error";
        case ABCKIT_STATUS_BAD_ARGUMENT:
            return "Bad argument";
        case ABCKIT_STATUS_MEMORY_ALLOCATION:
            return "Memory allocation error";
        case ABCKIT_STATUS_WRONG_MODE:
            return "Wrong mode";
        case ABCKIT_STATUS_WRONG_TARGET:
            return "Wrong target";
        case ABCKIT_STATUS_WRONG_LITERAL_TYPE:
            return "Wrong literal type";
        case ABCKIT_STATUS_UNSUPPORTED:
            return "Unsupported feature";
        case ABCKIT_STATUS_WRONG_CTX:
            return "Wrong context";
        case ABCKIT_STATUS_INTERNAL_ERROR:
            return "Internal error";
        default:
            break;
    }
    return "Unsupported status type";
}
// NOLINTEND(performance-unnecessary-value-param)

#ifdef ABCKIT_USE_EXCEPTIONS
/**
 * @brief Exception
 */
class Exception : public std::runtime_error {
public:
    /**
     * @brief Constructor
     * @param e - status
     */
    explicit Exception(AbckitStatus e) : std::runtime_error(StatusToString(e)) {}

    /**
     * @brief what
     * @return what
     */
    // NOLINTNEXTLINE(readability-identifier-naming)
    virtual const char *what() const noexcept override
    {
        return std::runtime_error::what();
    }
};
#else

/**
 * @brief Exception
 */
class Exception {
public:
    /**
     * @brief Constructor
     * @param e - status
     */
    explicit Exception(AbckitStatus e) : whatMessage_(StatusToString(e)) {}

    /**
     * @brief what
     * @return string
     */
    // CC-OFFNXT(G.NAM.03) made to be compatible with std::runtime_error::what method
    // NOLINTNEXTLINE(readability-identifier-naming)
    const char *what() const noexcept
    {
        // CC-OFFNXT(G.STD.04) made to be compatible with std::runtime_error::what method
        return whatMessage_.c_str();
    }

private:
    std::string whatMessage_;
};
#endif

/**
 * @brief IErrorHandler
 */
class IErrorHandler {
public:
    /**
     * Сonstructor
     */
    IErrorHandler() = default;

    /**
     * @brief Constructor
     * @param other
     */
    IErrorHandler(const IErrorHandler &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return IErrorHandler
     */
    IErrorHandler &operator=(const IErrorHandler &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    IErrorHandler(IErrorHandler &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return IErrorHandler
     */
    IErrorHandler &operator=(IErrorHandler &&other) = default;

    /**
     * @brief Destructor
     */
    virtual ~IErrorHandler() = default;

    //! @cond Doxygen_Suppress
    virtual void HandleError(Exception &&e) = 0;
    //! @endcond
};

/**
 * @brief DefaultErrorHandler
 */
class DefaultErrorHandler final : public IErrorHandler {
public:
    /**
     * Сonstructor
     */
    DefaultErrorHandler() = default;

    /**
     * @brief Constructor
     * @param other
     */
    DefaultErrorHandler(const DefaultErrorHandler &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return DefaultErrorHandler
     */
    DefaultErrorHandler &operator=(const DefaultErrorHandler &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    DefaultErrorHandler(DefaultErrorHandler &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return DefaultErrorHandler
     */
    DefaultErrorHandler &operator=(DefaultErrorHandler &&other) = default;

    /**
     * @brief Destructor
     */
    ~DefaultErrorHandler() override = default;

    /**
     * Handle error
     * @param e - exception
     */
    void HandleError([[maybe_unused]] Exception &&e) override
    {
// Default behaviour - do nothing.
// If at compile time exceptions are enabled - re-throw.
#ifdef ABCKIT_USE_EXCEPTIONS
        throw e;
#endif
    }
};

/**
 * @brief IResourceDeleter
 */
class IResourceDeleter {
public:
    /**
     * Сonstructor
     */
    IResourceDeleter() = default;

    /**
     * @brief Constructor
     * @param other
     */
    IResourceDeleter(const IResourceDeleter &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return IResourceDeleter
     */
    IResourceDeleter &operator=(const IResourceDeleter &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    IResourceDeleter(IResourceDeleter &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return IResourceDeleter
     */
    IResourceDeleter &operator=(IResourceDeleter &&other) = default;

    /**
     * @brief Destructor
     */
    virtual ~IResourceDeleter() = default;

    //! @cond Doxygen_Suppress
    virtual void DeleteResource() = 0;
    //! @endcond
};

/**
 * @brief DefaultResourceDeleter
 */
class DefaultResourceDeleter final : public IResourceDeleter {
public:
    /**
     * Сonstructor
     */
    DefaultResourceDeleter() = default;

    /**
     * @brief Constructor
     * @param other
     */
    DefaultResourceDeleter(const DefaultResourceDeleter &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return `DefaultResourceDeleter`
     */
    DefaultResourceDeleter &operator=(const DefaultResourceDeleter &other) = default;

    /**
     * @brief Constructor
     * @param other
     */
    DefaultResourceDeleter(DefaultResourceDeleter &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return `DefaultResourceDeleter`
     */
    DefaultResourceDeleter &operator=(DefaultResourceDeleter &&other) = default;

    /**
     * Destructor
     */
    ~DefaultResourceDeleter() override = default;

    /**
     * @brief Delete resource
     */
    void DeleteResource() override
    { /* Do nothing by default. Debug log here, probably? */
    }
};

}  // namespace abckit

#endif  // CPP_ABCKIT_UTILS_H
