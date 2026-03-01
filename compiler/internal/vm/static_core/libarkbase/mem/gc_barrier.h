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

#ifndef PANDA_LIBPANDABASE_MEM_GC_BARRIER_H
#define PANDA_LIBPANDABASE_MEM_GC_BARRIER_H

#include "libarkbase/utils/bit_field.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <variant>

namespace ark::mem {

/// Represents Pre and Post barrier
enum BarrierPosition : uint8_t {
    BARRIER_POSITION_PRE = 0x1,   // Should be inserted before each store/load when reference stored/loaded
    BARRIER_POSITION_POST = 0x0,  // Should be inserted after each store/load when reference stored/loaded
};

/// Indicates if barrier for store or load
enum BarrierActionType : uint8_t {
    WRITE_BARRIER = 0x1,  // Should be used around store
    READ_BARRIER = 0x0,   // Should be used around load
};

namespace internal {
constexpr uint8_t BARRIER_POS_OFFSET = 0U;       // offset in bits for encoding position of barrier(pre or post)
constexpr uint8_t BARRIER_WRB_FLAG_OFFSET = 1U;  // offset in bits for WRB flag
}  // namespace internal

constexpr uint8_t EncodeBarrierType(uint8_t value, BarrierPosition position, BarrierActionType actionType)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    return (value << 2U) | (position << internal::BARRIER_POS_OFFSET) |
           (actionType << internal::BARRIER_WRB_FLAG_OFFSET);
}

/**
 * Should help to encode barrier for the compiler.
 * PreWrite barrier can be used for avoiding lost object problem.
 * PostWrite barrier used for tracking intergenerational or interregion references
 */
enum BarrierType : uint8_t {
    PRE_WRB_NONE = EncodeBarrierType(1U, BarrierPosition::BARRIER_POSITION_PRE, BarrierActionType::WRITE_BARRIER),
    PRE_RB_NONE = EncodeBarrierType(1U, BarrierPosition::BARRIER_POSITION_PRE, BarrierActionType::READ_BARRIER),
    POST_WRB_NONE = EncodeBarrierType(1U, BarrierPosition::BARRIER_POSITION_POST, BarrierActionType::WRITE_BARRIER),
    POST_RB_NONE = EncodeBarrierType(1U, BarrierPosition::BARRIER_POSITION_POST, BarrierActionType::READ_BARRIER),
    /**
     * Pre barrier for SATB.
     * Pseudocode:
     * load PRE_WRITE_BARRIER_ADDR -> concurrent_marking
     * if (UNLIKELY(concurrent_marking)) {
     *     load obj.field -> pre_val  // note: if store volatile - we need to have volatile load here
     *     if (pre_val != nullptr) {
     *         call STORE_IN_BUFF_TO_MARK_FUNC(pre_val);
     *     }
     * }
     * store obj.field <- new_val // STORE for which barrier generated
     *
     * Runtime should provide these parameters:
     * PRE_WRITE_BARRIER_ADDR - address of pre WRB function (!= nullptr) ---> indicates that we have concurrent
     * marking on
     * STORE_IN_BUFF_TO_MARK_FUNC - address of function to store replaced reference
     */
    PRE_SATB_BARRIER = EncodeBarrierType(2U, BarrierPosition::BARRIER_POSITION_PRE, BarrierActionType::WRITE_BARRIER),
    /**
     * Inter-region barrier. For GCs without explicit continuous young gen space.
     * Pseudocode:
     * store obj.field <- new_val // STORE for which barrier generated
     * // Check if new_val is nullptr first - then we don't need a barrier
     * if (new_val == null) {
     *     return
     * }
     * // Check if new_val and address of field is in different regions
     * // (each region contain 2^REGION_SIZE_BITS and aligned with 2^REGION_SIZE_BITS bytes)
     * if ((AddressOf(obj) XOR AddressOf(new_val)) >> REGION_SIZE_BITS) != 0) {
     *     call UPDATE_CARD_FUNC(obj, new_val);
     * }
     *
     * Runtime should provide these parameters:
     * REGION_SIZE_BITS - log2 of the size of region
     * UPDATE_CARD_FUNC - function which updates card corresponding to the obj.field
     */
    POST_INTERREGION_BARRIER =
        EncodeBarrierType(4U, BarrierPosition::BARRIER_POSITION_POST, BarrierActionType::WRITE_BARRIER),
    /* Note: cosider two-level card table for pre-barrier */

    /**
     * Post write barrier for CMC-GC. Will call the WriteBarrier func provided by CMC-GC.
     * add more details about pseudocode (issue #26247)
     * Pseudocode:
     * store obj.field <- new_val // STORE for which barrier generated
     * // Call Write Barrier
     * call CMC_WRITE_BARRIER(obj, field, new_val);
     *
     * Runtime should provide these parameters:
     * CMC_WRITE_BARRIER - CMC-GC Write Barrier function
     */
    POST_CMC_WRITE_BARRIER =
        EncodeBarrierType(5U, BarrierPosition::BARRIER_POSITION_POST, BarrierActionType::WRITE_BARRIER),

    /**
     * Pre read barrier for CMC-GC. Will call the ReadBarrier func provided by CMC-GC.
     * add more details about pseudocode (issue #26247)
     * Pseudocode:
     * // Call Read Barrier
     * call CMC_READ_BARRIER(obj, field);
     * load obj.field // LOAD for which barrier generated
     *
     * Runtime should provide these parameters:
     * CMC_READ_BARRIER - CMC-GC Read Barrier function
     */
    PRE_CMC_READ_BARRIER =
        EncodeBarrierType(2U, BarrierPosition::BARRIER_POSITION_PRE, BarrierActionType::READ_BARRIER),
};

constexpr bool IsPreBarrier(BarrierType barrierType)
{
    return BitField<uint8_t, internal::BARRIER_POS_OFFSET, 1>::Get(barrierType) ==
           BarrierPosition::BARRIER_POSITION_PRE;
}

constexpr bool IsPostBarrier(BarrierType barrierType)
{
    return BitField<uint8_t, internal::BARRIER_POS_OFFSET, 1>::Get(barrierType) ==
           BarrierPosition::BARRIER_POSITION_POST;
}

constexpr bool IsWriteBarrier(BarrierType barrierType)
{
    return BitField<uint8_t, internal::BARRIER_WRB_FLAG_OFFSET, 1>::Get(barrierType) ==
           BarrierActionType::WRITE_BARRIER;
}

constexpr bool IsReadBarrier(BarrierType barrierType)
{
    return BitField<uint8_t, internal::BARRIER_WRB_FLAG_OFFSET, 1>::Get(barrierType) == BarrierActionType::READ_BARRIER;
}

static_assert(IsPreBarrier(BarrierType::PRE_SATB_BARRIER));
static_assert(IsWriteBarrier(BarrierType::PRE_SATB_BARRIER));
static_assert(IsPostBarrier(BarrierType::POST_INTERREGION_BARRIER));
static_assert(IsWriteBarrier(BarrierType::POST_INTERREGION_BARRIER));

constexpr bool IsEmptyBarrier(BarrierType barrierType)
{
    return (barrierType == BarrierType::PRE_WRB_NONE) || (barrierType == BarrierType::POST_WRB_NONE) ||
           (barrierType == BarrierType::PRE_RB_NONE) || (barrierType == BarrierType::POST_RB_NONE);
}

using ObjRefProcessFunc = void (*)(void *);
using ObjTwoRefProcessFunc = void (*)(const void *, const void *);

enum class BarrierOperandType {
    ADDRESS = 0,                      // just an address (void*)
    PRE_WRITE_BARRIER_ADDRESS,        // contains nullptr or address of function (during concurrent marking) with this
                                      // sig: void foo(void* );
    UINT8_ADDRESS,                    // contains address of uint8_t value
    FUNC_WITH_OBJ_REF_ADDRESS,        // contains address of function with this sig: void foo(void* );
    UINT8_LITERAL,                    // contains uint8_t value
    FUNC_WITH_TWO_OBJ_REF_ADDRESSES,  // contains address of function with this sig: void foo(void* , void* );
};

using BarrierOperandValue = std::variant<void *, bool *, std::atomic<ObjRefProcessFunc> *, uint8_t *, ObjRefProcessFunc,
                                         uint8_t, ObjTwoRefProcessFunc>;

class BarrierOperand {
public:
    // NOLINTNEXTLINE(modernize-pass-by-value)
    BarrierOperand(BarrierOperandType barrierOperandType, BarrierOperandValue barrierOperandValue)
        : barrierOperandType_(barrierOperandType), barrierOperandValue_(barrierOperandValue)
    {
    }

    inline BarrierOperandType GetType() const
    {
        return barrierOperandType_;
    }

    inline BarrierOperandValue GetValue() const
    {
        return barrierOperandValue_;
    }

    virtual ~BarrierOperand() = default;

    DEFAULT_COPY_SEMANTIC(BarrierOperand);
    DEFAULT_MOVE_SEMANTIC(BarrierOperand);

private:
    BarrierOperandType barrierOperandType_;
    BarrierOperandValue barrierOperandValue_;
};

}  // namespace ark::mem

#endif  // PANDA_LIBPANDABASE_MEM_GC_BARRIER_H
