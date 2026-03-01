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


#ifndef MRT_COPY_COLLECTOR_H
#define MRT_COPY_COLLECTOR_H

#include "Allocator/RegionSpace.h"
#include "Common/StateWord.h"
#include "TracingCollector.h"

namespace MapleRuntime {
class CopyCollector : public TracingCollector {
public:
    explicit CopyCollector(Allocator& allocator, CollectorResources& resources) : TracingCollector(allocator, resources)
    {
        collectorType = CollectorType::COPY_COLLECTOR;
    }
    ~CopyCollector() override = default;

    MRT_EXPORT void RunGarbageCollection(uint64_t gcIndex, GCReason reason) override;
    void CopyObject(const BaseObject& fromObj, BaseObject& toObj, size_t size) const;
    void PostGarbageCollection(uint64_t gcIndex) override;

protected:
    virtual BaseObject* ForwardObjectExclusive(BaseObject* obj) = 0;
    virtual void ForwardFromSpace();
    virtual void RefineFromSpace();

    virtual void DoGarbageCollection() = 0;

private:
};
} // namespace MapleRuntime
#endif // MRT_COPY_COLLECTOR_H
