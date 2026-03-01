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

#ifndef COMPILER_OPTIMIZER_IR_SPILL_FILL_DATA_H
#define COMPILER_OPTIMIZER_IR_SPILL_FILL_DATA_H

#include "datatype.h"
#include "constants.h"
#include "locations.h"

namespace panda::compiler {

using LocationType = Location::Kind;

class SpillFillData {
public:
    SpillFillData() = default;
    SpillFillData(LocationType src_type, LocationType dst_type, unsigned src_val, unsigned dst_val, DataType::Type tp)
        : src_(src_type, src_val), dst_(dst_type, dst_val), type_(tp)
    {
    }
    SpillFillData(Location src, Location dst, DataType::Type type) : src_(src), dst_(dst), type_(type) {}

    LocationType SrcType() const
    {
        return src_.GetKind();
    }
    LocationType DstType() const
    {
        return dst_.GetKind();
    }
    auto SrcValue() const
    {
        return src_.GetValue();
    }
    auto DstValue() const
    {
        return dst_.GetValue();
    }
    void SetSrc(Location loc)
    {
        src_ = loc;
    }
    void SetDst(Location loc)
    {
        dst_ = loc;
    }
    Location GetSrc() const
    {
        return src_;
    }
    Location GetDst() const
    {
        return dst_;
    }
    DataType::Type GetType() const
    {
        return type_;
    }
    DataType::Type GetCommonType() const
    {
        return DataType::GetCommonType(type_);
    }
    void SetType(DataType::Type type)
    {
        type_ = type;
    }

    void Dump(std::ostream &stm, Arch arch) const
    {
        GetSrc().Dump(stm, arch);
        stm << " -> ";
        GetDst().Dump(stm, arch);
        stm << " [" << ToString(GetType()) << "]";
    }

private:
    Location src_;
    Location dst_;
    DataType::Type type_ {DataType::NO_TYPE};
};

#ifndef IR_FOR_LIBARK_DEFECT_SCAN_AUX
static_assert(sizeof(SpillFillData) <= sizeof(uint64_t));
#endif

namespace sf_data {

inline auto ToString(const SpillFillData &sf, Arch arch)
{
    std::stringstream ss;
    sf.Dump(ss, arch);
    return ss.str();
}
}  // namespace sf_data

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_SPILL_FILL_DATA_H
