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


#include "Heap.h"

#include "Collector/CollectorProxy.h"
#include "Collector/CollectorResources.h"
#include "WCollector/IdleBarrier.h"
#include "WCollector/EnumBarrier.h"
#include "WCollector/TraceBarrier.h"
#include "WCollector/PostTraceBarrier.h"
#include "WCollector/PreforwardBarrier.h"
#include "WCollector/ForwardBarrier.h"
#include "Mutator/MutatorManager.h"
#if defined(_WIN64)
#include <windows.h>
#include <psapi.h>
#endif
#if defined(__APPLE__)
#include <mach/mach.h>
#endif
namespace MapleRuntime {
Barrier** Heap::currentBarrierPtr = nullptr;
Barrier* Heap::stwBarrierPtr = nullptr;
MAddress Heap::heapStartAddr = 0;
MAddress Heap::heapCurrentEnd = 0;

static bool InitEnabledGCParam()
{
    auto enableGC = std::getenv("codeEnableGC");
    if (enableGC == nullptr) {
        return true;
    }
    if (strlen(enableGC) != 1) {
        LOG(RTLOG_ERROR, "Unsupported codeEnableGC, codeEnableGC should be 0 or 1.\n");
        return true;
    }

    switch (enableGC[0]) {
        case '0':
            return false;
        case '1':
            return true;
        default:
            LOG(RTLOG_ERROR, "Unsupported codeEnableGC, codeEnableGC should be 0 or 1.\n");
    }
    return true;
}

class HeapImpl : public Heap {
public:
    HeapImpl()
        : theSpace(Allocator::NewAllocator()), collectorResources(collectorProxy),
          collectorProxy(*theSpace, collectorResources), stwBarrier(collectorProxy),
        idleBarrier(collectorProxy), enumBarrier(collectorProxy), traceBarrier(collectorProxy),
        postTraceBarrier(collectorProxy), preforwardBarrier(collectorProxy), forwardBarrier(collectorProxy)
    {
        currentBarrier = &stwBarrier;
        stwBarrierPtr = &stwBarrier;
        Heap::currentBarrierPtr = &currentBarrier;
        RunType::InitRunTypeMap();
    }

    ~HeapImpl() override = default;
    void Init(const HeapParam& vmHeapParam) override;
    void Fini() override;

    bool IsSurvivedObject(const BaseObject* obj) const override
    {
        return RegionSpace::IsMarkedObject(obj) || RegionSpace::IsResurrectedObject(obj);
    }

    bool IsGcStarted() const override { return collectorResources.IsGcStarted(); }

    void WaitForGCFinish() override { return collectorResources.WaitForGCFinish(); }

    bool IsGCEnabled() const override { return isGCEnabled.load(); }

    void EnableGC(bool val) override { return isGCEnabled.store(val); }

    MAddress Allocate(size_t size, AllocType allocType) override;

    GCPhase GetGCPhase() const override;
    void SetGCPhase(const GCPhase phase) override;
    Collector& GetCollector() override;
    Allocator& GetAllocator() override;

    size_t GetMaxCapacity() const override;
    size_t GetCurrentCapacity() const override;
    size_t GetUsedPageSize() const override;
    size_t GetAllocatedSize() const override;
    MAddress GetStartAddress() const override;
    MAddress GetSpaceEndAddress() const override;
    void RegisterStaticRoots(Uptr addr, U32) override;
    void UnregisterStaticRoots(Uptr addr, U32) override;
    void VisitStaticRoots(const RefFieldVisitor& visitor) override;
    bool ForEachObj(const std::function<void(BaseObject*)>&, bool) const override;
    ssize_t GetHeapPhysicalMemorySize() const override;
    void InstallBarrier(const GCPhase phase) override;
    FinalizerProcessor& GetFinalizerProcessor() override;
    CollectorResources& GetCollectorResources() override;
    void RegisterAllocBuffer(AllocBuffer& buffer) override;
    void RemoveAllocBuffer(AllocBuffer& buffer) override;
    U64 RegisterExportRoot(BaseObject* obj) override;
    void VisitAllExportRoots(const RootVisitor& visitor) override;
    BaseObject* GetExportObject(U64 id) override;
    void RemoveExportObject(U64 id) override;
    void StopGCWork() override;
    void CrossAccessBarrier(I64 handle) override;
    void SetExportObjActiveState(U64 id, bool state) override;
    bool CheckExportObjState(U64 id, BaseObject* exportObj) override;

    class ScopedFileHandler {
    public:
        ScopedFileHandler(const char* fileName, const char* mode) { file = fopen(fileName, mode); }
        ~ScopedFileHandler()
        {
            if (file != nullptr) {
                fclose(file);
            }
        }
        FILE* GetFile() const { return file; }

    private:
        FILE* file = nullptr;
    };

private:
    // allocator is actually a subspace in heap
    Allocator* theSpace;

    CollectorResources collectorResources;

    // collector is closely related to barrier. but we do not put barrier inside collector because even without
    // collector (i.e. no-gc), allocator and barrier (interface to access heap) is still needed.
    CollectorProxy collectorProxy;

    ExportRootTable exportRootsTable;
    Barrier stwBarrier;
    IdleBarrier idleBarrier;
    EnumBarrier enumBarrier;
    TraceBarrier traceBarrier;
    PostTraceBarrier postTraceBarrier;
    PreforwardBarrier preforwardBarrier;
    ForwardBarrier forwardBarrier;
    Barrier* currentBarrier = nullptr;

    // manage gc roots entry
    StaticRootTable staticRootTable;

    std::atomic<bool> isGCEnabled = { true };
}; // end class HeapImpl

static ImmortalWrapper<HeapImpl> g_heapInstance;

MAddress HeapImpl::Allocate(size_t size, AllocType allocType) { return theSpace->Allocate(size, allocType); }

bool HeapImpl::ForEachObj(const std::function<void(BaseObject*)>& visitor, bool safe) const
{
    return theSpace->ForEachObj(visitor, safe);
}

void HeapImpl::Init(const HeapParam& param)
{
    theSpace->Init(param);
    Heap::GetHeap().EnableGC(InitEnabledGCParam());
    collectorProxy.Init();
    collectorResources.Init();
}

void HeapImpl::Fini()
{
    collectorResources.Fini();
    collectorProxy.Fini();
    if (theSpace != nullptr) {
        delete theSpace;
        theSpace = nullptr;
    }
}

Collector& HeapImpl::GetCollector() { return collectorProxy.GetCurrentCollector(); }

Allocator& HeapImpl::GetAllocator() { return *theSpace; }

void HeapImpl::InstallBarrier(const GCPhase phase)
{
    if (phase == GCPhase::GC_PHASE_ENUM) {
        currentBarrier = &enumBarrier;
    } else if (phase == GCPhase::GC_PHASE_TRACE || phase == GCPhase::GC_PHASE_CLEAR_SATB_BUFFER) {
        currentBarrier = &traceBarrier;
    } else if (phase == GCPhase::GC_PHASE_PREFORWARD) {
        currentBarrier = &preforwardBarrier;
    } else if (phase == GCPhase::GC_PHASE_FORWARD) {
        currentBarrier = &forwardBarrier;
    } else if (phase == GCPhase::GC_PHASE_IDLE) {
        currentBarrier = &idleBarrier;
    } else if (phase == GCPhase::GC_PHASE_POST_TRACE) {
        currentBarrier = &postTraceBarrier;
    }
    DLOG(GCPHASE, "install barrier for gc phase %u", phase);
}

GCPhase HeapImpl::GetGCPhase() const { return collectorProxy.GetGCPhase(); }

void HeapImpl::SetGCPhase(const GCPhase phase) { collectorProxy.SetGCPhase(phase); }

size_t HeapImpl::GetMaxCapacity() const { return theSpace->GetMaxCapacity(); }

size_t HeapImpl::GetCurrentCapacity() const { return theSpace->GetCurrentCapacity(); }

size_t HeapImpl::GetUsedPageSize() const { return theSpace->GetUsedPageSize(); }

size_t HeapImpl::GetAllocatedSize() const { return theSpace->AllocatedBytes(); }

MAddress HeapImpl::GetStartAddress() const { return theSpace->GetSpaceStartAddress(); }

MAddress HeapImpl::GetSpaceEndAddress() const { return theSpace->GetSpaceEndAddress(); }

Heap& Heap::GetHeap() { return *g_heapInstance; }

void HeapImpl::RegisterStaticRoots(Uptr addr, U32 size)
{
    staticRootTable.RegisterRoots(reinterpret_cast<StaticRootTable::StaticRootArray*>(addr), size);
}

void HeapImpl::UnregisterStaticRoots(Uptr addr, U32 size)
{
    staticRootTable.UnregisterRoots(reinterpret_cast<StaticRootTable::StaticRootArray*>(addr), size);
}

void HeapImpl::VisitStaticRoots(const RefFieldVisitor& visitor) { staticRootTable.VisitRoots(visitor); }

#if defined(_WIN64)
ssize_t HeapImpl::GetHeapPhysicalMemorySize() const
{
    PROCESS_MEMORY_COUNTERS memCounter;
    HANDLE hProcess = GetCurrentProcess();
    if (GetProcessMemoryInfo(hProcess, &memCounter, sizeof(memCounter))) {
        size_t physicalMemorySize = memCounter.WorkingSetSize;
        if (physicalMemorySize > std::numeric_limits<ssize_t>::max()) {
            LOG(RTLOG_ERROR, "PhysicalMemorySize is too large");
            CloseHandle(hProcess);
            return -2; // -2: Return value of exception.
        }
        CloseHandle(hProcess);
        return static_cast<ssize_t>(physicalMemorySize);
    } else {
        LOG(RTLOG_ERROR, "GetHeapPhysicalMemorySize fail");
        CloseHandle(hProcess);
        return -2; // -2: Return value of exception.
    }
    return 0;
}

#elif defined(__APPLE__)
ssize_t HeapImpl::GetHeapPhysicalMemorySize() const
{
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count) != KERN_SUCCESS)
        return -2;  // -2: Return value of exception.
    return t_info.resident_size;
}

#else
ssize_t HeapImpl::GetHeapPhysicalMemorySize() const
{
    CString smapsFile = CString("/proc/") + CString(MapleRuntime::GetPid()) + "/smaps";
    ScopedFileHandler fileHandler(smapsFile.Str(), "r");
    FILE* file = fileHandler.GetFile();
    if (file == nullptr) {
        LOG(RTLOG_ERROR, "GetHeapPhysicalMemorySize(): fail to open the file");
        return -1;
    }
    const int bufSize = 256;
    char buf[bufSize] = { '\0' };
    while (fgets(buf, bufSize, file) != nullptr) {
        uint64_t startAddr = 0;
        uint64_t endAddr = 0;
        // expect 2 parameters are both written.
        constexpr int expectResult = 2;
        int ret = sscanf_s(buf, "%lx-%lx rw-p", &startAddr, &endAddr);
        if (ret == expectResult && startAddr <= GetHeap().GetStartAddress() && endAddr >= GetHeap().GetStartAddress()) {
            ssize_t physicalMemorySize = 0;
            uint64_t tmpStartAddr = endAddr;
            do {
                bool getPss = false;
                if (tmpStartAddr != endAddr) {
                    LOG(RTLOG_ERROR, "GetHeapPhysicalMemorySize(): fail to read the file");
                    return -2; // -2: Return value of exception.
                }
                do {
                    (void)fgets(buf, bufSize, file);
                    ssize_t size = 0;
                    if (sscanf_s(buf, "Pss:%zuKB", &size) == 1) {
                        physicalMemorySize += size;
                        getPss = true;
                    }
                } while (sscanf_s(buf, "%lx-%lx", &startAddr, &endAddr) != 2); // expect 2 parameters are both written.
                if (!getPss) {
                    LOG(RTLOG_ERROR, "GetHeapPhysicalMemorySize(): fail to read pss value");
                    return -2; // -2: Return value of exception.
                }
                tmpStartAddr = endAddr;
            } while (endAddr <= GetHeap().GetSpaceEndAddress());
            return physicalMemorySize * KB;
        }
    }
    LOG(RTLOG_ERROR, "GetHeapPhysicalMemorySize fail");
    return -2; // -2: Return value of exception.
}
#endif

FinalizerProcessor& HeapImpl::GetFinalizerProcessor() { return collectorResources.GetFinalizerProcessor(); }

CollectorResources& HeapImpl::GetCollectorResources() { return collectorResources; }

void HeapImpl::StopGCWork() { collectorResources.StopGCWork(); }

void HeapImpl::RegisterAllocBuffer(AllocBuffer& buffer) { GetAllocator().RegisterAllocBuffer(buffer); }

void HeapImpl::RemoveAllocBuffer(AllocBuffer &buffer) { GetAllocator().RemoveAllocBuffer(buffer); }

void HeapImpl::VisitAllExportRoots(const RootVisitor &visitor)
{
    exportRootsTable.VisitGCRoots(visitor);
}

BaseObject* HeapImpl::GetExportObject(U64 id)
{
    return exportRootsTable.GetExportRoot(id);
}

U64 HeapImpl::RegisterExportRoot(BaseObject *obj)
{
    if (!IsHeapAddress(obj)) {
        return std::numeric_limits<U64>::max();
    }
    return exportRootsTable.RegisterExportRoot(obj);
}

void HeapImpl::RemoveExportObject(U64 id)
{
    exportRootsTable.RemoveExportRoot(id);
}

void HeapImpl::CrossAccessBarrier(I64 id)
{
    BaseObject* recordObj = GetExportObject(id);
    if (recordObj == nullptr) {
        return;
    }
    if (GetGCPhase() == GCPhase::GC_PHASE_PREFORWARD) {
        auto& collector = GetCollector();
        if (collector.IsGhostFromObject(recordObj) &&
            !collector.IsUnmovableFromObject(recordObj)) {
                recordObj = collector.ForwardObject(recordObj);
            }
    }

    reinterpret_cast<TracingCollector&>(GetCollector()).ResurrectExportObject(recordObj);
    SetExportObjActiveState(id, true);
}

void HeapImpl::SetExportObjActiveState(U64 id, bool state)
{
    exportRootsTable.SetActiveState(id, state);
}

bool HeapImpl::CheckExportObjState(U64 id, BaseObject *exportObj)
{
    return exportRootsTable.CheckActiveState(id, exportObj);
}
} // namespace MapleRuntime
