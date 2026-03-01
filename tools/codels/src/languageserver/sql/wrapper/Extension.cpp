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

#ifdef SQLITE_EXTENSION_API

#include "Extension.h"
#include "Connection.h"

#include "ScopeExit.h"

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1

#include <exception>
#include <vector>

namespace sqldb {

static std::vector<void (*)(Connection &)> &getInitializeFunctions()
{
    static std::vector<void (*)(Connection &)> InitializeFunctions;
    return InitializeFunctions;
}

RegisterExtension::RegisterExtension(void (*InitializeFunction)(class Connection &))
{
    getInitializeFunctions().push_back(InitializeFunction);
}

} // namespace sqldb

#ifdef _WIN32
#define SQLDB_EXPORT_API __declspec(dllexport)
#else
#define SQLDB_EXPORT_API
#endif

/**
 * NOLINTNEXTLINE(readability-identifier-naming)
 */
extern "C" int SQLDB_EXPORT_API sqlite3_extension_init(sqlite3 *DB, char **ErrMsg, const sqlite3_api_routines *API)
{
    int RC = sqlite::OK;
    SQLITE_EXTENSION_INIT2(API);
    sqldb::Connection Connection(DB);
    sqldb::ScopeExit ReleaseDB = [&]() noexcept { DB = Connection.release(); };
    try {
        for (auto &InitializeExtension : sqldb::getInitializeFunctions())
            InitializeExtension(Connection);
    } catch (const std::exception &Exception) {
        if (ErrMsg != nullptr) {
            *ErrMsg = sqlite3_mprintf("%s", Exception.what());
        }
        RC = sqlite::Error;
    } catch (...) {
        if (ErrMsg != nullptr) {
            *ErrMsg = sqlite3_mprintf("Unknown error");
        }
        RC = sqlite::Error;
    }
    return RC;
}

#endif
