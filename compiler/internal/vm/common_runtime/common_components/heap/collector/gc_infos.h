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
#ifndef COMMON_COMPONENTS_HEAP_COLLECTOR_GC_DEBUGGER_H
#define COMMON_COMPONENTS_HEAP_COLLECTOR_GC_DEBUGGER_H

#include <algorithm>
#include <list>
#include <map>
#include <vector>

#include "common_components/base/time_utils.h"
#include "common_components/log/log.h"

#ifndef NDEBUG
#define GCINFO_DEBUG (false)
#else
#define GCINFO_DEBUG (false)
#endif

namespace common {
#if defined(GCINFO_DEBUG) && GCINFO_DEBUG
class GCInfoNode {
public:
    enum RootType {
        REG_ROOT,
        SLOT_ROOT,
    };
    static GCInfoNode BuildNodeForMarking(uintptr_t startIP, uintptr_t ip, FrameAddress* fa)
    {
        CString time = TimeUtil::GetTimestamp();

#ifdef __APPLE__
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(fa);
#else
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(startIP);
#endif
        return GCInfoNode(funcDesc->GetFuncName(), time, startIP, ip, reinterpret_cast<uintptr_t>(fa->returnAddress));
    }
    GCInfoNode() = default;
    GCInfoNode(const CString& name, const CString& time, uintptr_t startIp, uintptr_t ip, uintptr_t retPC)
        : methodName(name), startPC(startIp), pc(ip), ra(retPC), timeStamp(time) {}
    virtual ~GCInfoNode() = default;
    virtual void DumpFrameGCInfo() const
    {
        DLOG(ENUM, "  time stamp: %s method name: %s, start ip: %p, frame pc: %p, return address: %p", timeStamp.Str(),
             methodName.Str(), startPC, pc, ra);
        DumpMapGCInfo(regRoots, "Register roots", "register: %d, root: %p  ");
        DumpMapGCInfo(slotRoots, "Slot roots", "offset: %d, root: %p  ");
        DumpMapGCInfo(invalidRegRoots, "Invalid register roots", "register id: %d, root: %p  ");
        DumpMapGCInfo(invalidSlotRoots, "Invalid slot roots", "offset: %d, root: %p  ");
    }
    template<bool isValid = true>
    void InsertSlotRoots(SlotBias off, const BaseObject* ref)
    {
        if (isValid) {
            slotRoots[off] = ref;
        } else {
            invalidSlotRoots[off] = ref;
        }
    }
    template<bool isValid = true>
    void InsertRegRoot(RegisterNum reg, const BaseObject* ref)
    {
        if (isValid) {
            regRoots[reg] = ref;
        } else {
            invalidRegRoots[reg] = ref;
        }
    }

protected:
    template<class MapType>
    static void DumpMapGCInfo(const MapType& rootMap, const CString& title, const CString& stringFormat)
    {
        constexpr size_t numPerRow = 5;
        constexpr size_t defaultCount = 0;
        DLOG(ENUM, "    %s: {", title.Str());
        size_t size = rootMap.size();
        size_t remain = size % numPerRow;
        size_t count = defaultCount;
        CString detail = "";
        for (auto x : rootMap) {
            if (count == defaultCount) {
                detail.Append("     ");
            }
            detail.Append(CString::FormatString(stringFormat.Str(), x.first, x.second));
            if (++count == numPerRow) {
                count = defaultCount;
                detail.Append("\n");
            }
        }
        if (remain != defaultCount) {
            detail.Append("\n");
        }
        detail.Append("    }\n");
        DLOG(ENUM, "%s", detail.Str());
    }

private:
    CString methodName;
    uintptr_t startPC;
    uintptr_t pc;
    uintptr_t ra;
    CString timeStamp;
    std::map<RegisterNum, const BaseObject*> regRoots;
    std::map<RegisterNum, const BaseObject*> invalidRegRoots;
    std::map<SlotBias, const BaseObject*> slotRoots;
    std::map<SlotBias, const BaseObject*> invalidSlotRoots;
};

class GCInfoNodeForFix : public GCInfoNode {
public:
    static GCInfoNodeForFix BuildNodeForFix(uintptr_t startIP, uintptr_t ip, FrameAddress* fa)
    {
        CString time = TimeUtil::GetTimestamp();

#ifdef __APPLE__
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(fa);
#else
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(startIP);
#endif
        return GCInfoNodeForFix(funcDesc->GetFuncName(), time, startIP, ip,
                                reinterpret_cast<uintptr_t>(fa->returnAddress));
    }
    GCInfoNodeForFix() = default;
    GCInfoNodeForFix(const CString& name, const CString& time, uintptr_t startIp, uintptr_t ip, uintptr_t retPC)
        : GCInfoNode(name, time, startIp, ip, retPC) {}
    virtual ~GCInfoNodeForFix() = default;
    void InsertDerivedPtrRef(BasePtrType base, DerivedPtrType derived) { derivedPtrRef[base] = derived; }
    void DumpFrameGCInfo() const override
    {
        GCInfoNode::DumpFrameGCInfo();
        DumpMapGCInfo(derivedPtrRef, "derived ptr references", "base object: %#zx, derived ptr: %#zx");
    }

private:
    std::map<BasePtrType, DerivedPtrType> derivedPtrRef;
};
class CurrentGCInfo {
public:
    ~CurrentGCInfo(){};
    void PushFrameInfoForMarking(const GCInfoNode& frameGCInfo) { gcInfosForMarking.push_back(frameGCInfo); }
    void PushFrameInfoForMarking(const GCInfoNode&& frameGCInfo) { gcInfosForMarking.push_back(frameGCInfo); }

    void PushFrameInfoForFix(const GCInfoNodeForFix& frameGCInfo) { gcInfosForFix.push_back(frameGCInfo); }
    void PushFrameInfoForFix(const GCInfoNodeForFix&& frameGCInfo) { gcInfosForFix.push_back(frameGCInfo); }
    void Clear()
    {
        gcInfosForMarking.clear();
        std::vector<GCInfoNode>().swap(gcInfosForMarking);
        gcInfosForFix.clear();
        std::vector<GCInfoNodeForFix>().swap(gcInfosForFix);
    }
    void DumpFrameInfo() const
    {
        DLOG(ENUM, "  fix roots info:");
        std::for_each(gcInfosForFix.begin(), gcInfosForFix.end(),
                      [](const GCInfoNodeForFix& info) { info.DumpFrameGCInfo(); });
        DLOG(ENUM, "  marking roots info:");
        std::for_each(gcInfosForMarking.begin(), gcInfosForMarking.end(),
                      [](const GCInfoNode& info) { info.DumpFrameGCInfo(); });
    }

private:
    std::vector<GCInfoNode> gcInfosForMarking;
    std::vector<GCInfoNodeForFix> gcInfosForFix;
};
class GCInfos {
public:
    GCInfos() = default;
    ~GCInfos() = default;
    void CreateCurrentGCInfo()
    {
        constexpr size_t numLimit = 10;
        if (gcInfos.size() >= numLimit) {
            auto& front = gcInfos.front();
            front.Clear();
            gcInfos.pop_front();
        }
        gcInfos.push_back(CurrentGCInfo());
    }

    void PushFrameInfoForMarking(const GCInfoNode& frameGCInfo)
    {
        GetCurrentGCInfo().PushFrameInfoForMarking(frameGCInfo);
    }

    void PushFrameInfoForMarking(const GCInfoNode&& frameGCInfo)
    {
        GetCurrentGCInfo().PushFrameInfoForMarking(frameGCInfo);
    }

    void PushFrameInfoForFix(const GCInfoNodeForFix& infoNodeFoxFix)
    {
        GetCurrentGCInfo().PushFrameInfoForFix(infoNodeFoxFix);
    }
    void PushFrameInfoForFix(const GCInfoNodeForFix&& infoNodeForFix)
    {
        GetCurrentGCInfo().PushFrameInfoForFix(infoNodeForFix);
    }

    void DumpGCInfos() const
    {
        size_t size = gcInfos.size();
        DLOG(ENUM, " current thread happened %d times GC", size);
        size_t i = 1;
        std::for_each(gcInfos.rbegin(), gcInfos.rend(), [&i](const CurrentGCInfo& cur) {
            DLOG(ENUM, " the %d scan stack: ", i++);
            cur.DumpFrameInfo();
        });
    }

private:
    CurrentGCInfo& GetCurrentGCInfo()
    {
        if (UNLIKELY_CC(gcInfos.empty())) {
            CreateCurrentGCInfo();
        }
        return gcInfos.back();
    }
    std::list<CurrentGCInfo> gcInfos;
};
#endif
} // namespace common

#endif  // COMMON_COMPONENTS_HEAP_COLLECTOR_GC_DEBUGGER_H
