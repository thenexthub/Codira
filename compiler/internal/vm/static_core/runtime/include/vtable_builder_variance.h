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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_H
#define PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_H

#include "runtime/class_linker_context.h"
#include "runtime/include/vtable_builder_base.h"

namespace ark {

template <class ProtoCompatibility, class OverridePred>
class VarianceVTableBuilder final : public VTableBuilderBase<false> {
public:
    explicit VarianceVTableBuilder(ClassLinkerErrorHandler *errHandler) : VTableBuilderBase(errHandler) {}

private:
    [[nodiscard]] bool ProcessClassMethod(const MethodInfo *info) override;
    [[nodiscard]] bool ProcessDefaultMethod(ITable itable, size_t itableIdx, MethodInfo *methodInfo) override;

    [[nodiscard]] bool ProcessProxyClassMethod(const MethodInfo *info) override;

    std::optional<MethodInfo const *> ScanConflictingDefaultMethods(const MethodInfo *info);

    static bool IsOverriddenBy(const ClassLinkerContext *ctx, Method::ProtoId const &base, Method::ProtoId const &derv);

    static bool IsOverriddenOrOverrides(const ClassLinkerContext *ctx, Method::ProtoId const &p1,
                                        Method::ProtoId const &p2);
};

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_H
