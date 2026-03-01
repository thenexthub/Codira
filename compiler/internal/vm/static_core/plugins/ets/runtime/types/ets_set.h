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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SET_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SET_H

#include "plugins/ets/runtime/types/ets_map.h"

namespace ark::ets {

namespace test {
class EtsSetTest;
}  // namespace test

class EtsEscompatSet : public EtsObject {
public:
    EtsEscompatSet() = delete;
    ~EtsEscompatSet() = delete;

    NO_COPY_SEMANTIC(EtsEscompatSet);
    NO_MOVE_SEMANTIC(EtsEscompatSet);

    static constexpr size_t GetElementsOffset()
    {
        return MEMBER_OFFSET(EtsEscompatSet, elements_);
    }

    ObjectPointer<EtsEscompatMap> GetElements()
    {
        return elements_;
    }

private:
    ObjectPointer<EtsEscompatMap> elements_;

    friend class test::EtsSetTest;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_SET_H
