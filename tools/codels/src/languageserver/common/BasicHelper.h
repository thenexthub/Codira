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

#ifndef LSPSERVER_BASICHELPER_H
#define LSPSERVER_BASICHELPER_H

#ifdef _WIN32
#include "windows.h"
#elif __linux__
#include "unistd.h"
#include "sys/sysinfo.h"
#endif

#include <functional>
#include <cstdint>
#include "../../json-rpc/Protocol.h"
#include "Codira/Basic/Position.h"

namespace ark {
template<typename T>
using Callback = std::function<void(T)>;

std::int32_t GetOffsetFromPosition(const std::string &, Codira::Position);
} // namespace ark

#endif // LSPSERVER_BASICHELPER_H
