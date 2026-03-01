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

#ifndef GUARD_LOGGER_H
#define GUARD_LOGGER_H

#include "libarkbase/utils/logger.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_D LOG(DEBUG, GUARD)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_I LOG(INFO, GUARD)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_W LOG(WARNING, GUARD)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_E LOG(ERROR, GUARD)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_F LOG(FATAL, GUARD)

#endif  // GUARD_LOGGER_H
