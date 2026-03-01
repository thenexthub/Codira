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

#ifndef CODIRA_IRGENERATOR_H
#define CODIRA_IRGENERATOR_H

#include <memory>
#include <type_traits>

namespace Codira {
namespace CHIR {
class Package;
class Func;
class BasicBlock;
class Expression;
} // namespace CHIR

namespace CodeGen {
class IRGeneratorImpl {
public:
    virtual ~IRGeneratorImpl() = default;
    virtual void EmitIR() = 0;

protected:
    IRGeneratorImpl() = default;
};

/**
 * Each `IRGenerator` instantiation needs an implementation class which inherits `IRGeneratorImpl`.
 * The implementation class can have its specific `EmitIR` method.
 */
template <typename Impl = IRGeneratorImpl,
    typename = typename std::enable_if<std::is_base_of<IRGeneratorImpl, Impl>::value>::type>
class IRGenerator {
public:
    void EmitIR()
    {
        impl->EmitIR();
    }
    IRGenerator() = delete;

protected:
    explicit IRGenerator(std::unique_ptr<Impl> impl) : impl(std::move(impl))
    {
    }
    std::unique_ptr<Impl> impl;
};
} // namespace CodeGen
} // namespace Codira
#endif // CODIRA_IRGENERATOR_H
