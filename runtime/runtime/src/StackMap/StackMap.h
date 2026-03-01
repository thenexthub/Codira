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


#ifndef MRT_STACKMAP_H
#define MRT_STACKMAP_H
#include <unordered_set>
#include "StackMap/CompressedStackMap.h"
namespace MapleRuntime {
class StackMap {
public:
    explicit StackMap(Uptr base) : isValid(false), stackBase(base) {}
    StackMap(bool valid, uintptr_t base) : isValid(valid), stackBase(base) {}
    StackMap(const StackMap& other) : isValid(other.isValid), stackBase(other.stackBase) {}
    StackMap& operator=(const StackMap& other)
    {
        if (this == &other) {
            return *this;
        }
        this->isValid = other.isValid;
        this->stackBase = other.stackBase;
        return *this;
    }
    StackMap(StackMap&& other) : isValid(other.isValid), stackBase(other.stackBase) {}
    StackMap& operator=(StackMap&& other)
    {
        if (this == &other) {
            return *this;
        }
        this->isValid = other.isValid;
        this->stackBase = other.stackBase;
        return *this;
    }
    virtual ~StackMap() = default;
    bool IsValid() const { return isValid; }

protected:
    bool isValid;
    Uptr stackBase;
};

class RootMap : public StackMap {
public:
    RootMap(Uptr base, PrologueRegisterClosure&& prologue) : StackMap(base), calleeSavedPrologue(std::move(prologue)) {}
    RootMap(bool valid, Uptr base, const StackMapEntry& entry, PrologueRegisterClosure&& prologue)
        : StackMap(valid, base), calleeSavedPrologue(std::move(prologue)), slotRoot(entry.BuildSlotRoot()),
          regRoot(entry.BuildRegRoot()) {}
    ~RootMap() override = default;
    RootMap(RootMap&& other)
        : StackMap(std::move(other)), calleeSavedPrologue(std::move(other.calleeSavedPrologue)),
          slotRoot(std::move(other.slotRoot)), regRoot(other.regRoot) {}
    RootMap& operator=(RootMap&& other)
    {
        if (this == &other) {
            return *this;
        }
        StackMap::operator=(std::move(other));
        this->calleeSavedPrologue = std::move(other.calleeSavedPrologue);
        this->slotRoot = std::move(other.slotRoot);
        this->regRoot = other.regRoot;
        return *this;
    }

    // ATTENTION: VisitRegRoots VisitSlotRoots VisitDerivedPtr must be invoked in a fixed order
    virtual bool VisitRegRoots(const RootVisitor& visitor, const RegDebugVisitor& debugFunc, RegSlotsMap& regSlotsMap)
    {
        return regRoot.VisitGCRoots(visitor, debugFunc, regSlotsMap);
    }
    virtual void VisitSlotRoots(const RootVisitor& visitor, const SlotDebugVisitor& debugFunc)
    {
        slotRoot.VisitGCRoots(visitor, debugFunc, stackBase);
    }

    void RecordCalleeSaved(RegSlotsMap& regSlotsMap) const
    {
        calleeSavedPrologue.RecordCalleeSaved(regSlotsMap, stackBase);
    }

protected:
    PrologueRegisterClosure calleeSavedPrologue;
    SlotRoot slotRoot;
    RegRoot regRoot;
};

class HeapReferenceMap : public RootMap {
public:
    HeapReferenceMap(Uptr base, PrologueRegisterClosure&& prologue) : RootMap(base, std::move(prologue)) {}
    HeapReferenceMap(bool valid, Uptr base, const StackMapEntry& entry, PrologueRegisterClosure&& prologue)
        : RootMap(valid, base, entry, std::move(prologue)), derivedPtr(entry.BuildDerivedPtrRoot()) {}
    HeapReferenceMap(HeapReferenceMap&& other)
        : RootMap(std::move(other)), derivedPtr(other.derivedPtr), rootsList(std::move(other.rootsList)) {}
    HeapReferenceMap& operator=(HeapReferenceMap&& other)
    {
        if (this == &other) {
            return *this;
        }
        RootMap::operator=(std::move(other));
        this->derivedPtr = other.derivedPtr;
        this->rootsList = std::move(other.rootsList);
        return *this;
    }
    ~HeapReferenceMap() override = default;
    bool VisitRegRoots(const RootVisitor& visitor, const RegDebugVisitor& debugFunc, RegSlotsMap& regSlotsMap) override
    {
        return regRoot.VisitGCRoots(visitor, debugFunc, regSlotsMap, &rootsList);
    }

    void VisitSlotRoots(const RootVisitor& visitor, const SlotDebugVisitor& debugFunc) override
    {
        slotRoot.VisitGCRoots(visitor, debugFunc, stackBase, &rootsList);
    }

    // VisitDerivedPtr must be invoked after VisitRegRoots and VisitSlotRoots;
    void VisitDerivedPtr(const DerivedPtrVisitor& derivedVisitor, const DerivedPtrDebugVisitor debugVisitor,
                         RegSlotsMap& regSlotsMap)
    {
        for (auto it = rootsList.begin(); it != rootsList.end(); ++it) {
            if (!derivedPtr.VisitDerivedPtr(derivedVisitor, debugVisitor, regSlotsMap, *it, stackBase)) {
                break;
            }
        }
    }

private:
    DerivedPtr derivedPtr;
    std::list<Uptr> rootsList;
};

class MethodMap : public StackMap {
public:
    explicit MethodMap(Uptr base) : StackMap(base) {}
    MethodMap(bool valid, Uptr base, const StackMapEntry& entry)
        : StackMap(valid, base), lineNum(entry.BuildLineNum()) {}
    ~MethodMap() override = default;
    LineNum GetLineNum() const { return lineNum; }

private:
    LineNum lineNum = 0;
};

// StackPtr means pointer to the address on the stack;
class StackPtrMap : public StackMap {
public:
    StackPtrMap(Uptr base, PrologueRegisterClosure&& prologue)
        : StackMap(base), calleeSavedPrologue(std::move(prologue)) {}
    StackPtrMap(bool valid, Uptr base, const StackMapEntry& entry, PrologueRegisterClosure&& prologue)
        : StackMap(valid, base), calleeSavedPrologue(std::move(prologue)),
          gcSlotRoot(entry.BuildSlotRoot()), gcRegRoot(entry.BuildRegRoot()),
          stackPtrSlotRoot(entry.BuildStackSlotRoot()), stackPtrRegRoot(entry.BuildStackRegRoot()),
          derivedPtr(entry.BuildDerivedPtrRoot()) {}

    ~StackPtrMap() override = default;
    StackPtrMap(StackPtrMap&& other)
        : StackMap(std::move(other)), calleeSavedPrologue(std::move(other.calleeSavedPrologue)),
          gcSlotRoot(std::move(other.gcSlotRoot)), gcRegRoot(other.gcRegRoot),
          stackPtrSlotRoot(std::move(other.stackPtrSlotRoot)), stackPtrRegRoot(other.stackPtrRegRoot),
          derivedPtr(other.derivedPtr), rootsList(std::move(other.rootsList)) {}
    StackPtrMap& operator=(StackPtrMap&& other)
    {
        if (this == &other) {
            return *this;
        }
        StackMap::operator=(std::move(other));
        this->calleeSavedPrologue = std::move(other.calleeSavedPrologue);
        this->gcSlotRoot = std::move(other.gcSlotRoot);
        this->gcRegRoot = other.gcRegRoot;
        this->stackPtrSlotRoot = std::move(other.stackPtrSlotRoot);
        this->stackPtrRegRoot = other.stackPtrRegRoot;
        this->derivedPtr = other.derivedPtr;
        this->rootsList = std::move(other.rootsList);
        return *this;
    }

    bool VisitReg(const StackPtrVisitor& traceAndFixPtrVisitor, const StackPtrVisitor& fixPtrVisitor,
                  const RegDebugVisitor& debugFunc, RegSlotsMap& regSlotsMap)
    {
        // Reuse GC interface
        return gcRegRoot.VisitGCRoots(traceAndFixPtrVisitor, debugFunc, regSlotsMap, &rootsList) &&
            stackPtrRegRoot.VisitGCRoots(fixPtrVisitor, debugFunc, regSlotsMap, nullptr);
    }

    void VisitSlot(const StackPtrVisitor& traceAndFixPtrVisitor, const StackPtrVisitor& fixPtrVisitor,
                   const SlotDebugVisitor& debugFunc)
    {
        // Reuse GC interface
        gcSlotRoot.VisitGCRoots(traceAndFixPtrVisitor, debugFunc, stackBase, &rootsList);
        stackPtrSlotRoot.VisitGCRoots(fixPtrVisitor, debugFunc, stackBase, nullptr);
    }

    void VisitDerivedPtr(const DerivedPtrVisitor& derivedVisitor, RegSlotsMap& regSlotsMap)
    {
        for (auto it = rootsList.begin(); it != rootsList.end(); ++it) {
            if (!derivedPtr.VisitDerivedPtrForStackGrow(derivedVisitor, nullptr, regSlotsMap, *it, stackBase)) {
                break;
            }
        }
    }

    void RecordCalleeSaved(RegSlotsMap& regSlotsMap) const
    {
        calleeSavedPrologue.RecordCalleeSaved(regSlotsMap, stackBase);
    }

protected:
    PrologueRegisterClosure calleeSavedPrologue;
    // The gcXxxxRoot table mainly stores pointers to objects abort GC on the heap.
    // Some pointers are assigned to the stack after escape analysis.
    // So we're going to parse nonEscaptedXxxxRoot tables to find the non-escaped pointers.
    SlotRoot gcSlotRoot;
    RegRoot gcRegRoot;
    SlotRoot stackPtrSlotRoot;
    RegRoot stackPtrRegRoot;
    DerivedPtr derivedPtr;
    std::list<Uptr> rootsList;
};

class StackMapBuilder {
public:
    StackMapBuilder(uintptr_t start, uintptr_t frame, uintptr_t base)
        : startPC(start), framePC(frame), stackBase(base), funcDesc(nullptr) {}
    StackMapBuilder(uintptr_t start, uintptr_t frame, uintptr_t base, uint64_t *pFuncDesc)
        : startPC(start), framePC(frame), stackBase(base), funcDesc(pFuncDesc) {}
    ~StackMapBuilder() = default;

    template<class MapType>
    MapType Build() const
    {
        PrologueRegisterClosure closure;
        PrologueVisitor visitor = [&closure](PrologueRegisterClosure::Type type, uint32_t value) {
            switch (type) {
                case PrologueRegisterClosure::Type::CALLEE_REGISTER:
                    closure.calleeSaved.push_back(value);
                    break;
                case PrologueRegisterClosure::Type::OFFSET:
                    closure.offset.push_back(value);
                    break;
            }
        };
#ifdef __APPLE__
        auto head = CompressedStackMapHead::GetStackMapHead(stackBase, visitor);
#else
        auto head = CompressedStackMapHead::GetStackMapHead(startPC, visitor);
#endif
        auto entry = head.GetStackMapEntry(startPC, framePC);
        if (!entry.IsValid()) {
            return MapType(stackBase, std::move(closure));
        }
        return MapType(true, stackBase, entry, std::move(closure));
    }

protected:
    uintptr_t startPC;
    uintptr_t framePC;
    uintptr_t stackBase;
    uint64_t *funcDesc;
};

// specialization for MethodMap which avoids using malloc().
template<>
inline MethodMap StackMapBuilder::Build<MethodMap>() const
{
#ifdef __APPLE__
    auto head = CompressedStackMapHead::GetStackMapHead(stackBase, nullptr, funcDesc);
#else
    auto head = CompressedStackMapHead::GetStackMapHead(startPC, nullptr, funcDesc);
#endif
    auto entry = head.GetStackMapEntry(startPC, framePC);
    if (!entry.IsValid()) {
        return MethodMap(stackBase);
    }
    return MethodMap(true, stackBase, entry);
}
} // namespace MapleRuntime
#endif // MRT_STACKMAP_H
