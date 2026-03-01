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

/**
 * @file
 *
 * This file declares the ConditionalCompilation related classes, which provides conditional compilation capabilities.
 */
#ifndef CODIRA_CONDITIONAL_COMPILATION_H
#define CODIRA_CONDITIONAL_COMPILATION_H

#include "Codira/AST/Node.h"
#include "Codira/Frontend/CompilerInstance.h"

namespace Codira {
namespace AST {

class ConditionalCompilation {
public:
    friend class CompilerInstance;
    explicit ConditionalCompilation(CompilerInstance* ci);
    ~ConditionalCompilation();

    /// entrance of conditional compilation stage
    void HandleConditionalCompilation(const Package& root) const;
    /// file entrance of conditional compilation stage. Used by \ref HandleConditionalCompilation and macro expansion
    void HandleFileConditionalCompilation(File& file) const;

private:
    class ConditionalCompilationImpl* impl;
};
} // namespace AST
} // namespace Codira

#endif
