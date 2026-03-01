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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_ITABLE_BUILDER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_ITABLE_BUILDER_H_

#include "libarkbase/utils/span.h"
#include "runtime/include/class.h"
#include "runtime/include/itable_builder.h"
#include "runtime/include/itable.h"

namespace ark {

class ClassLinker;

}  // namespace ark

namespace ark::ets {

class EtsITableBuilder : public ITableBuilder {
public:
    explicit EtsITableBuilder(ClassLinkerErrorHandler *errHandler) : errorHandler_(errHandler) {}

    bool Build(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces, bool isInterface) override;

    bool Resolve(Class *klass) override;

    void UpdateClass(Class *klass) override;

    void DumpITable([[maybe_unused]] Class *klass) override;

    ITable GetITable() const override
    {
        return itable_;
    };

private:
    ITable itable_;
    ClassLinkerErrorHandler *errorHandler_;
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_ITABLE_BUILDER_H_
