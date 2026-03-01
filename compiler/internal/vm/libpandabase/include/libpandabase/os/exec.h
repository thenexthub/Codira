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

#ifndef LIBPANDABASE_OS_EXEC_H
#define LIBPANDABASE_OS_EXEC_H

#include "utils/expected.h"
#include "utils/span.h"
#include "os/error.h"
#include <array>

namespace panda::os::exec {

Expected<int, Error> Exec(Span<const char *> args);

template <typename... Args>
decltype(auto) Exec(Args... args)
{
    std::array<const char *, sizeof...(args) + 1> arguments = {args..., nullptr};
    return os::exec::Exec(Span(arguments));
}

}  // namespace panda::os::exec

#endif  // LIBPANDABASE_OS_EXEC_H
