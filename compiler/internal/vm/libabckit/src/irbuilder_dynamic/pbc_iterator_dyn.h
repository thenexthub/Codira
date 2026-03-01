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

#ifndef LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_PBC_ITERATOR_H
#define LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_PBC_ITERATOR_H

#include "libabckit/src/irbuilder_dynamic/bytecode_inst.h"
#include "libabckit/src/irbuilder_dynamic/bytecode_inst-inl.h"

namespace libabckit {

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

struct BytecodeIterator {
    explicit BytecodeIterator(BytecodeInst inst) : inst_(std::move(inst)) {}
    explicit BytecodeIterator(const uint8_t *data) : inst_(data) {}

    BytecodeIterator &operator++()
    {
        inst_ = inst_.GetNext();
        return *this;
    }

    BytecodeInst operator*()
    {
        return inst_;
    }
    bool operator!=(const BytecodeIterator &rhs)
    {
        return inst_.GetAddress() != rhs.inst_.GetAddress();
    }

private:
    BytecodeInst inst_;
};

struct BytecodeInstructions {
    BytecodeInstructions(const uint8_t *data, size_t size) : data_(data), size_(size) {}

    // NOLINTNEXTLINE(readability-identifier-naming)
    BytecodeIterator begin() const
    {
        return BytecodeIterator(data_);
    }
    // NOLINTNEXTLINE(readability-identifier-naming)
    BytecodeIterator end() const
    {
        return BytecodeIterator(data_ + size_);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    size_t GetPc(const BytecodeInst &inst) const
    {
        return inst.GetAddress() - data_;
    }
    size_t GetSize() const
    {
        return size_;
    }

private:
    const uint8_t *data_;
    size_t size_;
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_PBC_ITERATOR_H
