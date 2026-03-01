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
#include "common_components/heap/collector/collector_proxy.h"

namespace common {
void CollectorProxy::Init(const RuntimeParam& param)
{
    arkCollector_.Init(param);

    if (currentCollector_ == nullptr) {
        currentCollector_ = &arkCollector_;
    }
}

void CollectorProxy::Fini() { arkCollector_.Fini(); }

void CollectorProxy::RunGarbageCollection(uint64_t gcIndex, GCReason reason, GCType gcType)
{
    switch (reason) {
        case GC_REASON_HEU:
        case GC_REASON_YOUNG:
        case GC_REASON_BACKUP:
            currentCollector_ = &arkCollector_;
            break;
        case GC_REASON_OOM:
        case GC_REASON_FORCE:
            currentCollector_ = &arkCollector_;
            break;
        default:
            currentCollector_ = &arkCollector_;
            break;
    }
    currentCollector_->MarkGCStart();
    currentCollector_->RunGarbageCollection(gcIndex, reason, gcType);
}
} // namespace common
