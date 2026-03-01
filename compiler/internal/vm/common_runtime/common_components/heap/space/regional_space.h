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
#ifndef COMMON_COMPONENTS_HEAP_SPACE_REGIONAL_SPACE_H
#define COMMON_COMPONENTS_HEAP_SPACE_REGIONAL_SPACE_H

#include <assert.h>
#include <list>
#include <map>
#include <set>
#include <thread>
#include <vector>

#include "common_interfaces/base/common.h"
#include "common_components/heap/allocator/alloc_util.h"
#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/mutator/mutator.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/mutator/mutator_manager.h"
#if defined(COMMON_SANITIZER_SUPPORT)
#include "common_components/base/asan_interface.h"
#endif

namespace common {
class RegionalSpace {
public:
    RegionalSpace(RegionManager& regionManager) : regionManager_(regionManager) {}

    RegionManager& GetRegionManager() { return regionManager_; }

protected:
    void ClearGCInfo(RegionList& list)
    {
        RegionList tmp("temp region list");
        list.CopyListTo(tmp);
        tmp.VisitAllRegions([](RegionDesc* region) {
            region->ClearMarkingCopyLine();
            region->ClearLiveInfo();
            region->ResetMarkBit();
        });
    }

    void InitRegionPhaseLine(RegionDesc* region)
    {
        if (region == nullptr) {
            return;
        }
        GCPhase phase = Mutator::GetMutator()->GetMutatorPhase();
        if (phase == GC_PHASE_ENUM || phase == GC_PHASE_MARK || phase == GC_PHASE_REMARK_SATB ||
            phase == GC_PHASE_POST_MARK) {
            region->SetMarkingLine();
        } else if (phase == GC_PHASE_PRECOPY || phase == GC_PHASE_COPY || phase == GC_PHASE_FIX) {
            region->SetMarkingLine();
            region->SetCopyLine();
        }
    }

    RegionManager& regionManager_;
};
} // namespace common
#endif // COMMON_COMPONENTS_HEAP_SPACE_REGIONAL_SPACE_H
