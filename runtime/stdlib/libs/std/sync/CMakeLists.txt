# ===----------------------------------------------------------------------===
#
#  Copyright (c) NeXTHub Corporation. All Rights Reserved.
#  DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
#  Author: Tunjay Akbarli
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at:
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
#  Middletown, DE 19709, New Castle County, USA.
#
# ===----------------------------------------------------------------------===

if(CODIRA_CODEGEN_CODENATIVE_BACKEND)
    set(CODENATIVE_SYNC_SRCS
        atomic.code
        monitor.code
        ireentrant_mutex.code
        lock_interface.code
        mutex_codenative.code
        read_write_lock.code
        reentrant_mutex.code
        reentrant_read_write_mutex.code
        native.code
        barrier.code
        sync_counter.code
        sync_list.code
        semaphore.code
        timer.code
        CACHE INTERNAL "")
endif()
