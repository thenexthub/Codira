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

#include "Configure.h"
#include "Exception.h"

#include "sqlite3.h"

namespace sqldb {

static void logCallback(void *P, int ErrCode, const char *Msg) { reinterpret_cast<LogCallback>(P)(ErrCode, Msg); }

void setLogCallback(LogCallback Callback)
{
    int RC = sqlite3_config(SQLITE_CONFIG_LOG, logCallback, reinterpret_cast<void *>(Callback));
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set log callback");
    }
}

void setSingleThreadMode()
{
    int RC = sqlite3_config(SQLITE_CONFIG_SINGLETHREAD);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set single thread mode");
    }
}

void setMultiThreadMode()
{
    int RC = sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set multi thread mode");
    }
}

void setSerializedMode()
{
    int RC = sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    if (RC != SQLITE_OK) {
        throw Exception(RC, "Failed to set serialized mode");
    }
}

} // namespace sqldb
