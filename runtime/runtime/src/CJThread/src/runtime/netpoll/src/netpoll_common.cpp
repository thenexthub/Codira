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


#include "log.h"
#include "securec.h"
#include "netpoll.h"
#include "netpoll_common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct NetpollMetaData *NetpollMetaDataInit(void)
{
    int error;
    struct NetpollMetaData *metaData;

    metaData = (struct NetpollMetaData *)malloc(sizeof(struct NetpollMetaData));
    if (metaData == nullptr) {
        LOG_ERROR(errno, "malloc failed, size: %u", sizeof(struct NetpollMetaData));
        return nullptr;
    }
    error = memset_s(metaData, sizeof(struct NetpollMetaData), 0, sizeof(struct NetpollMetaData));
    if (error != 0) {
        LOG_ERROR(error, "memset_s failed");
        free(metaData);
        return nullptr;
    }

    return metaData;
}

#ifdef __cplusplus
}
#endif
