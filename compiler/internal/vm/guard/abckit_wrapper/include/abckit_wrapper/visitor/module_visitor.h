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

#ifndef ABCKIT_WRAPPER_MODULE_VISITOR_H
#define ABCKIT_WRAPPER_MODULE_VISITOR_H

#include "visitor_wrapper.h"
#include "class_visitor.h"
#include "annotation_visitor.h"

namespace abckit_wrapper {

class Module;

/**
 * @brief ModuleVisitor
 */
class ModuleVisitor : public WrappedVisitor<ModuleVisitor> {
public:
    /**
     * @brief Visit module
     * @param module visited module
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool Visit(Module *module) = 0;
};

/**
 * @brief ModuleFilter
 * filter the accepted module for visit
 */
class ModuleFilter : public ModuleVisitor, public BaseVisitorWrapper<ModuleVisitor> {
public:
    /**
     * @brief ModuleFilter Constructor
     * @param visitor ModuleVisitor
     */
    explicit ModuleFilter(ModuleVisitor &visitor) : BaseVisitorWrapper(visitor) {}

    /**
     * @brief Given module whether accepted
     * @param module visited module
     * @return `true`: module will be visit `false`: module will be skipped
     */
    virtual bool Accepted(Module *module) = 0;

private:
    bool Visit(Module *module) override;
};

/**
 * @brief LocalModuleFilter
 * all local module will be visited
 */
class LocalModuleFilter final : public ModuleFilter {
public:
    /**
     * @brief LocalModuleFilter Constructor
     * @param visitor ModuleVisitor
     */
    explicit LocalModuleFilter(ModuleVisitor &visitor) : ModuleFilter(visitor) {}

private:
    bool Accepted(Module *module) override;
};

/**
 * @brief ModuleClassVisitor
 * visit all module classes
 */
class ModuleClassVisitor final : public ModuleVisitor, public BaseVisitorWrapper<ClassVisitor> {
public:
    /**
     * @brief ModuleClassVisitor Constructor
     * @param visitor ClassVisitor
     */
    explicit ModuleClassVisitor(ClassVisitor &visitor) : BaseVisitorWrapper(visitor) {}

private:
    bool Visit(Module *module) override;
};

/**
 * @brief ModuleAnnotationVisitor
 * visit all module annotations
 */
class ModuleAnnotationVisitor final : public ModuleVisitor, public BaseVisitorWrapper<AnnotationVisitor> {
public:
    /**
     * @brief ModuleAnnotationsVisitor Constructor
     * @param visitor AnnotationVisitor
     */
    explicit ModuleAnnotationVisitor(AnnotationVisitor &visitor) : BaseVisitorWrapper(visitor) {}

private:
    bool Visit(Module *module) override;
};

}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_MODULE_VISITOR_H
