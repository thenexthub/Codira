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

#pragma once

#include "Bind.h"

#include <stdexcept>
#include <string>

namespace sqldb {

/**
 * Named SQL parameter.
 */
template <typename T>
struct Parameter {
    const char *Name;
    const T &Value;
};

#ifndef DOXYGEN_IGNORE

template <typename T>
struct traits::Bind<Parameter<T>> {
    static int call(sqlite3_stmt *S, int &I, const Parameter<T> &V)
    {
#ifndef NO_EXCEPTIONS
        if ((I = sqlite::bind_parameter_index(S, V.Name)) == 0) {
            throw std::invalid_argument("Invalid named parameter: " + std::string(V.Name));
        }
#else
        I = sqlite::bind_parameter_index(S, V.Name);
#endif
        return traits::bind(S, I, V.Value);
    }
};

#endif // DOXYGEN_IGNORE

} // namespace sqldb
