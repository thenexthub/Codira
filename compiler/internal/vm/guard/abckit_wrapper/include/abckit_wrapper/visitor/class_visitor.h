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

#ifndef ABCKIT_WRAPPER_CLASS_VISITOR_H
#define ABCKIT_WRAPPER_CLASS_VISITOR_H

#include "visitor_wrapper.h"
#include "member_visitor.h"

namespace abckit_wrapper {

class Class;

/**
 * @brief ClassVisitor
 */
class ClassVisitor : public WrappedVisitor<ClassVisitor> {
public:
    /**
     * @brief Visit class
     * @param clazz visited clazz
     * @return `false` if was early exited. Otherwise - `true`.
     */
    virtual bool Visit(Class *clazz) = 0;
};

/**
 * @brief ClassFilter
 * filter the accepted class for visit
 */
class ClassFilter : public ClassVisitor, public BaseVisitorWrapper<ClassVisitor> {
public:
    /**
     * @brief ClassFilter Constructor
     * @param visitor ClassVisitor
     */
    explicit ClassFilter(ClassVisitor &visitor) : BaseVisitorWrapper(visitor) {}

    /**
     * @brief Given class whether accepted
     * @param clazz visited class
     * @return `true`: class will be visit `false`: class will be skipped
     */
    virtual bool Accepted(Class *clazz) = 0;

private:
    bool Visit(Class *clazz) override;
};

/**
 * @brief LeafClassVisitor
 * filter the leaf class (subClass is empty)
 */
class LeafClassVisitor final : public ClassFilter {
public:
    /**
     * @brief LeafClassVisitor Constructor
     * @param visitor ClassVisitor
     */
    explicit LeafClassVisitor(ClassVisitor &visitor) : ClassFilter(visitor) {}

private:
    bool Accepted(Class *clazz) override;
};

/**
 * @brief ClassMemberVisitor
 */
class ClassMemberVisitor final : public ClassVisitor, public BaseVisitorWrapper<MemberVisitor> {
public:
    /**
     * @brief ClassMemberVisitor Constructor
     * @param visitor MemberVisitor
     */
    explicit ClassMemberVisitor(MemberVisitor &visitor) : BaseVisitorWrapper(visitor) {}

private:
    bool Visit(Class *clazz) override;
};

}  // namespace abckit_wrapper

#endif  // ABCKIT_WRAPPER_CLASS_VISITOR_H
