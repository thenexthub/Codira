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


#include <cstdlib>
#include "external.h"

#ifdef __cplusplus
extern "C" {
#endif

struct SchdfdManager *SchdfdManagerInit()
{
    struct SchdfdManager *schdfdManager = (struct SchdfdManager *)malloc(sizeof(SchdfdManager));
    if (schdfdManager == nullptr) {
        return nullptr;
    }
    schdfdManager->initLock = PTHREAD_MUTEX_INITIALIZER;
    for (int i = 0; i < SCHDFD_SLOTS_MAX_LAYER; i++) {
        schdfdManager->slots[i] = nullptr;
    }
    return schdfdManager;
}

void FreeSchdfdManager(struct SchdfdManager *schdfdManager)
{
    if (schdfdManager == nullptr) {
        return;
    }
    struct SchdfdFd* schdfdFd;
    auto slots = schdfdManager->slots;
    for (int i = 0; i < SCHDFD_SLOTS_MAX_LAYER; i++) {
        if (slots[i] == nullptr) {
            continue;
        }
        for (int j = 0; j < SCHDFD_SLOTS_LAYER_MAX_LINE_NUM; j++) {
            if (slots[i][j] == nullptr) {
                continue;
            }
            for (int k = 0; k < SCHDFD_SLOTS_LINE_MAX_FD_NUM; k++) {
                schdfdFd = slots[i][j][k].schdFd;
                FreeSchdfdFd(schdfdFd);
            }
            free(slots[i][j]);
            slots[i][j] = nullptr;
        }
        free(slots[i]);
        slots[i] = nullptr;
    }

    free(schdfdManager);
}

void FreeSchdfdFd(struct SchdfdFd *schdfdFd)
{
    if (schdfdFd == nullptr) {
        return;
    }
    if (schdfdFd->pd != nullptr) {
        free(schdfdFd->pd);
        schdfdFd->pd = nullptr;
    }
    free(schdfdFd);
}

#ifdef __cplusplus
}
#endif
