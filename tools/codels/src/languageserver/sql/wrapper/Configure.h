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

#include <string_view>
#include <type_traits>

/**
 * All interfaces listed in this header file are not threadsafe. The
 * application must ensure that no other interfaces are invoked by other
 * threads while these functions are running.
 */

namespace sqldb {

/**
 * The error logger callback function.
 * The first argument to the error logger callback is an integer extended error
 * code. The second argument to the error logger callback is the text of the
 * error message. The error message text is stored in a fixed-length stack
 * buffer in the calling function and so will only be valid for the duration of
 * the error logger callback function. The error logger should make a copy of
 * this message into persistent storage if retention of the message is needed.
 */
using LogCallback = std::add_pointer_t<void(int, std::string_view)>;

/**
 * Configure SQLite global error log. The supplied Callback function must
 * not invoke any SQLite interface. In a multi-threaded application, the
 * callback function must also be threadsafe.
 */
void setLogCallback(LogCallback Callback);

/**
 * Set the threading mode to single-thread. A call to this function disables
 * all mutexing and puts SQLite into a mode where it can only be used by a
 * single thread.
 */
void setSingleThreadMode();

/**
 * Set the threading mode to multi-thread. A call to this function disables
 * mutexing on database connection and prepared statement objects. The
 * application is responsible for serializing access to database connections
 * and prepared statements. But other mutexes are enabled so that SQLite will
 * be safe to use in a multi-threaded environment as long as no two threads
 * attempt to use the same database connection at the same time.
 */
void setMultiThreadMode();

/**
 * Set the threading mode to serialized. This enables all mutexes including the
 * recursive mutexes on database connection and prepared statement objects. In
 * this mode (which is the default) the SQLite will itself serialize access to
 * database connections and prepared statements so that the application is free
 * to use the same database connection or the same prepared statement in
 * different threads at the same time.
 */
void setSerializedMode();

} // namespace sqldb
