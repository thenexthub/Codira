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
#ifndef PANDA_GC_BARRIER_H
#define PANDA_GC_BARRIER_H

#include "libarkbase/mem/gc_barrier.h"
#include "libarkbase/mem/ringbuf/lock_free_ring_buffer.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/mem/gc/card_table.h"

namespace ark::mem {

/// Base barrier set
class GCBarrierSet {
public:
    GCBarrierSet() = delete;
    GCBarrierSet(mem::InternalAllocatorPtr allocator, BarrierType preReadType, BarrierType preType,
                 BarrierType postType)
        : preReadType_(preReadType),
          preType_(preType),
          postType_(postType),
          preOperands_(allocator->Adapter()),
          postOperands_(allocator->Adapter())
    {
    }

    NO_COPY_SEMANTIC(GCBarrierSet);
    NO_MOVE_SEMANTIC(GCBarrierSet);
    virtual ~GCBarrierSet() = 0;

    BarrierType GetPreReadType() const
    {
        ASSERT(IsPreBarrier(preReadType_));
        return preReadType_;
    }

    BarrierType GetPreType() const
    {
        ASSERT(IsPreBarrier(preType_));
        return preType_;
    }

    BarrierType GetPostType() const
    {
        ASSERT(IsPostBarrier(postType_));
        return postType_;
    }

    virtual bool IsPreReadBarrierEnabled()
    {
        return !mem::IsEmptyBarrier(preReadType_);
    }

    virtual bool IsPreBarrierEnabled()
    {
        return !mem::IsEmptyBarrier(preType_);
    }

    /**
     * Pre barrier. Used by interpreter.
     * @param pre_val_addr - reference currently(before store/load happened) stored in the field
     */
    virtual void PreBarrier(void *preValAddr) = 0;

    /**
     * Post barrier. Used by interpeter.
     * @param obj_addr - address of object whose field we change
     * @param offset   - field offset
     * @param val_addr - reference stored into or loaded from the field
     */
    virtual void PostBarrier(const void *objAddr, size_t offset, void *valAddr) = 0;

    /**
     * Post barrier for range write. Used by interpeter.
     * @param obj_addr - address of the object
     * @param offset - offset from obj_addr
     * @param count - written bytes count
     */
    virtual void PostBarrier(const void *objAddr, size_t offset, size_t count) = 0;

    /**
     * Get barrier operand (literal, function pointer, address etc. See enum BarrierType for details.
     * Should be used for barrier generation in Compiler.
     * @param name - string with name of operand
     * @return barrier operand (value is address or literal)
     */
    BarrierOperand GetBarrierOperand(BarrierPosition barrierPosition, std::string_view name);

    BarrierOperand GetPostBarrierOperand(std::string_view name);

    virtual void *PreReadBarrier([[maybe_unused]] const void *objAddr, [[maybe_unused]] size_t offset)
    {
        UNREACHABLE();
    }

protected:
    /**
     * Add barrier operand if there are no operands with this name
     * @param barrier_position - pre or post position of barrier with added operand
     * @param name - name of operand
     * @param barrier_operand - operand
     */
    void AddBarrierOperand(BarrierPosition barrierPosition, std::string_view name, const BarrierOperand &barrierOperand)
    {
        if (barrierPosition == BarrierPosition::BARRIER_POSITION_PRE) {
            ASSERT(preOperands_.find(name) == preOperands_.end());
            preOperands_.insert({name.data(), barrierOperand});
        } else {
            ASSERT(barrierPosition == BarrierPosition::BARRIER_POSITION_POST);
            ASSERT(postOperands_.find(name) == postOperands_.end());
            postOperands_.insert({name.data(), barrierOperand});
        }
    }

private:
    BarrierType preReadType_;  // Type of PRE-Read barrier
    BarrierType preType_;      // Type of PRE-Write barrier.
    BarrierType postType_;     // Type of POST-Write barrier.
    PandaMap<PandaString, BarrierOperand> preOperands_;
    PandaMap<PandaString, BarrierOperand> postOperands_;
};

/// BarrierSet with barriers do nothing
class GCDummyBarrierSet : public GCBarrierSet {
public:
    explicit GCDummyBarrierSet(mem::InternalAllocatorPtr allocator)
        : GCBarrierSet(allocator, BarrierType::PRE_RB_NONE, BarrierType::PRE_WRB_NONE, BarrierType::POST_WRB_NONE)
    {
    }

    NO_COPY_SEMANTIC(GCDummyBarrierSet);
    NO_MOVE_SEMANTIC(GCDummyBarrierSet);
    ~GCDummyBarrierSet() override = default;

    void PreBarrier([[maybe_unused]] void *preValAddr) override {}

    void PostBarrier([[maybe_unused]] const void *objAddr, [[maybe_unused]] size_t offset,
                     [[maybe_unused]] void *storedValAddr) override
    {
    }

    void PostBarrier([[maybe_unused]] const void *objAddr, [[maybe_unused]] size_t offset,
                     [[maybe_unused]] size_t count) override
    {
    }
};

class GCG1BarrierSet : public GCBarrierSet {
public:
    using ThreadLocalCardQueues = PandaVector<CardTable::CardPtr>;
    static constexpr size_t G1_POST_BARRIER_RING_BUFFER_SIZE = 1024 * 8;
    using G1PostBarrierRingBufferType = mem::LockFreeBuffer<mem::CardTable::CardPtr, G1_POST_BARRIER_RING_BUFFER_SIZE>;

    GCG1BarrierSet(mem::InternalAllocatorPtr allocator,
                   // PRE ARGS:
                   ObjRefProcessFunc preStoreFunc,
                   // POST ARGS:
                   ObjTwoRefProcessFunc postFunc, uint8_t regionSizeBitsCount, CardTable *cardTable,
                   ThreadLocalCardQueues *updatedRefsQueue, os::memory::Mutex *queueLock)
        : GCBarrierSet(allocator, BarrierType::PRE_RB_NONE, BarrierType::PRE_SATB_BARRIER,
                       BarrierType::POST_INTERREGION_BARRIER),
          preStoreFunc_(preStoreFunc),
          postFunc_(postFunc),
          regionSizeBitsCount_(regionSizeBitsCount),
          cardTable_(cardTable),
          minAddr_(ToVoidPtr(cardTable->GetMinAddress())),
          updatedRefsQueue_(updatedRefsQueue),
          queueLock_(queueLock)
    {
        ASSERT(preStoreFunc_ != nullptr);
        ASSERT(postFunc_ != nullptr);
        // PRE
        AddBarrierOperand(
            BarrierPosition::BARRIER_POSITION_PRE, "STORE_IN_BUFF_TO_MARK_FUNC",
            BarrierOperand(BarrierOperandType::FUNC_WITH_OBJ_REF_ADDRESS, BarrierOperandValue(preStoreFunc_)));
        // POST
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "REGION_SIZE_BITS",
                          BarrierOperand(BarrierOperandType::UINT8_LITERAL, BarrierOperandValue(regionSizeBitsCount_)));
        AddBarrierOperand(
            BarrierPosition::BARRIER_POSITION_POST, "UPDATE_CARD_FUNC",
            BarrierOperand(BarrierOperandType::FUNC_WITH_TWO_OBJ_REF_ADDRESSES, BarrierOperandValue(postFunc_)));
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "CARD_TABLE_ADDR",
                          BarrierOperand(BarrierOperandType::UINT8_ADDRESS,
                                         BarrierOperandValue(reinterpret_cast<uint8_t *>(*cardTable->begin()))));
        AddBarrierOperand(BarrierPosition::BARRIER_POSITION_POST, "MIN_ADDR",
                          BarrierOperand(BarrierOperandType::ADDRESS, BarrierOperandValue(minAddr_)));
    }

    bool IsPreBarrierEnabled() override;

    void PreBarrier(void *preValAddr) override;

    void PostBarrier(const void *objAddr, size_t offset, void *storedValAddr) override;

    void PostBarrier(const void *objAddr, size_t offset, size_t count) override;

    void Enqueue(CardTable::CardPtr card);

    void PostCardToQueue(CardTable::CardPtr card);

    ~GCG1BarrierSet() override = default;

    CardTable *GetCardTable() const
    {
        return cardTable_;
    }

    NO_COPY_SEMANTIC(GCG1BarrierSet);
    NO_MOVE_SEMANTIC(GCG1BarrierSet);

private:
    void Invalidate(CardTable::CardPtr begin, CardTable::CardPtr last);
    using PostFuncT = std::function<void(const void *, const void *)> *;
    // Store operands explicitly for interpreter perf
    // PRE BARRIER
    ObjRefProcessFunc preStoreFunc_ {nullptr};
    // POST BARRIER
    /// Function which is called for the post barrier if all conditions
    ObjTwoRefProcessFunc postFunc_;
    /// How much bits needed for the region
    uint8_t regionSizeBitsCount_ {0};
    /// Card table pointer
    CardTable *cardTable_ {nullptr};
    /// Minimal address used by VM. Used as a base for card index calculation
    void *minAddr_ {nullptr};
    ThreadLocalCardQueues *updatedRefsQueue_;
    os::memory::Mutex *queueLock_;
};

/// BarrierSet for hybird CMC GC
class GCCMCBarrierSet : public GCBarrierSet {
public:
    explicit GCCMCBarrierSet(mem::InternalAllocatorPtr allocator)
        : GCBarrierSet(allocator, BarrierType::PRE_CMC_READ_BARRIER, BarrierType::PRE_WRB_NONE,
                       BarrierType::POST_CMC_WRITE_BARRIER)
    {
    }

    NO_COPY_SEMANTIC(GCCMCBarrierSet);
    NO_MOVE_SEMANTIC(GCCMCBarrierSet);
    ~GCCMCBarrierSet() override = default;

    void PreBarrier([[maybe_unused]] void *preValAddr) override
    {
        UNREACHABLE();
    }

    void PostBarrier(const void *objAddr, size_t offset, void *storedValAddr) override;

    void PostBarrier(const void *objAddr, size_t offset, size_t count) override;

    bool IsPreReadBarrierEnabled() override
    {
        return true;
    }

    void *PreReadBarrier(const void *objAddr, size_t offset) override;

private:
    class CMCWriteBarrierHandle;
};

}  // namespace ark::mem

#endif  // PANDA_GC_BARRIER_H
