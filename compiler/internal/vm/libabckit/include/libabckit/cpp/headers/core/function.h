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

#ifndef CPP_ABCKIT_CORE_FUNCTION_H
#define CPP_ABCKIT_CORE_FUNCTION_H

#include "../base_classes.h"
#include "annotation.h"

#include <functional>
#include <string>
#include <vector>

namespace abckit::core {

/**
 * @brief Function
 */
class Function : public ViewInResource<AbckitCoreFunction *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class core::Interface;
    /// @brief to access private constructor
    friend class core::Enum;
    /// @brief to access private constructor
    friend class core::Namespace;
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class abckit::Instruction;
    /// @brief abckit::DefaultHash<Function>
    friend class abckit::DefaultHash<Function>;
    /// @brief abckit::DynamicIsa
    friend class abckit::DynamicIsa;
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class arkts::Namespace;
    /// @brief to access private constructor
    friend class arkts::Module;

protected:
    /// @brief Core API View type
    using CoreViewT = Function;

public:
    /**
     * @brief Construct a new empty Function object
     */
    Function() : ViewInResource(nullptr), conf_(nullptr)
    {
        SetResource(nullptr);
    };

    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(const Function &other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return Function&
     */
    Function &operator=(const Function &other) = default;

    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(Function &&other) = default;  // CC-OFF(G.CLS.07): design decision, detail: base_concepts.h

    /**
     * @brief Constructor
     * @param other
     * @return Function&
     */
    Function &operator=(Function &&other) = default;

    /**
     * @brief Destroy the Function object
     */
    ~Function() override = default;

    /**
     * @brief Create the `Graph` object
     * @return Created `Graph`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Graph CreateGraph() const;

    /**
     * @brief Sets graph for Function.
     * @param [ in ] graph - Graph to be set.
     * @return New state of Function
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if Function is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if Graph itself is false.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `File`s owning `function` and `graph` are
     * differ.
     * @note Allocates
     */
    Function SetGraph(const Graph &graph) const;

    /**
     * @brief Get the name
     * @return std::string
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::string GetName() const;

    /**
     * @brief Returns binary file that the current `Function` is a part of.
     * @return Pointer to the `File`. It should be nullptr if current `Function` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    const File *GetFile() const;

    /**
     * @brief Get the annotation
     * @return std::vector<core::Annotation>
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::Annotation> GetAnnotations() const;

    /**
     * @brief Tells if method is static.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsStatic() const;

    /**
     * @brief Tells if method is public.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsPublic() const;

    /**
     * @brief Tells if method is protected.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsProtected() const;

    /**
     * @brief Tells if method is private.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsPrivate() const;

    /**
     * @brief Tells if method is internal.
     * @return bool
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsInternal() const;

    /**
     * @brief Tells if Function is defined in the same binary or externally in another binary.
     * @return Returns `true` if Function is defined in another binary and `false` if defined locally.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsExternal() const;

    /**
     * @brief Get the Module object
     * @return `core::Module`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    core::Module GetModule() const;

    /**
     * @brief Get the Parent Class object
     * @return `core::Class`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    core::Class GetParentClass() const;

    /**
     * @brief Returns parent function for function.
     * @return `core::Function`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Function GetParentFunction() const;

    /**
     * @brief Returns parent namespace for function.
     * @return `core::Namespace`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    core::Namespace GetParentNamespace() const;

    /**
     * @brief Returns parameters for function.
     * @return `std::vector<core::FunctionParam>`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    std::vector<core::FunctionParam> GetParameters() const;

    /**
     * @brief Enumeraterated nested functions
     * @return `false` if was early exited. Otherwise - `true`.
     * @param cb - Callback that will be invoked.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     */
    bool EnumerateNestedFunctions(const std::function<bool(core::Function)> &cb) const;

    /**
     * @brief Enumerates nested classes
     * @return `false` if was early exited. Otherwise - `true`.
     * @param cb - Callback that will be invoked.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     */
    bool EnumerateNestedClasses(const std::function<bool(core::Class)> &cb) const;

    /**
     * @brief Enumerates annotations of Function, invoking callback `cb` for each annotation.
     * The return value of `cb` used as a signal to continue (true) or early-exit (false) enumeration.
     * @param cb that will be invoked.
     * @return `false` if was early exited. Otherwise - `true`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     */
    bool EnumerateAnnotations(const std::function<bool(core::Annotation)> &cb) const;

    /**
     * @brief Tells if function is constructor.
     * @return Returns `true` if function is constructor and `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsCtor() const;

    /**
     * @brief Tells if function is static constructor.
     * @return Returns `true` if function is static constructor and `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsCctor() const;

    /**
     * @brief Tells if function is anonymous.
     * @return Returns `true` if function is anonymous and `false` otherwise.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    bool IsAnonymous() const;

    /**
     * @brief Sets graph for function.
     * @param [ in ] graph - Graph to be set.
     * @return New state of Function.
     * @note Allocates
     */
    Function SetGraph(Graph &graph) const;

    /**
     * @brief Returns return type for function.
     * @return `core::Type`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     */
    Type GetReturnType() const;

private:
    Function(AbckitCoreFunction *func, const ApiConfig *conf, const File *file) : ViewInResource(func), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_FUNCTION_H
