/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "llvm_aot_compiler.h"

namespace ark::llvmbackend {

void CodeInfoProducer::DumpStackMap(const std::unique_ptr<const LLVMStackMap> &stackmap, std::ostream &stream)
{
    stream << "Header {" << std::endl;
    stream << "  Stack Map Version (current version is 3) [uint8]: " << stackmap->getVersion() << std::endl;
    stream << "}" << std::endl;

    stream << "StkSizeRecord [" << stackmap->getNumFunctions() << "]: [" << std::endl;
    for (const auto &rec : stackmap->functions()) {
        stream << "  {" << std::endl;
        DumpStackMapFunction(rec, stream, "    ");
        stream << "  }" << std::endl;
    }
    stream << ']' << std::endl;

    stream << "Constants [" << stackmap->getNumConstants() << "]: [" << std::endl;
    for (const auto &cst : stackmap->constants()) {
        stream << "  LargeConstant [uint64] : " << cst.getValue() << std::endl;
    }
    stream << ']' << std::endl;

    stream << "StkMapRecord [" << stackmap->getNumRecords() << "]: [" << std::endl;
    for (const auto &rec : stackmap->records()) {
        stream << "  {" << std::endl;
        DumpStackMapRecord(rec, stream, "    ");
        stream << "  }" << std::endl;
    }
    stream << ']' << std::endl;
}

void CodeInfoProducer::DumpStackMapFunction(const LLVMStackMap::FunctionAccessor &function, std::ostream &stream,
                                            const std::string &prefix)
{
    stream << prefix << "Function Address [uint64]: 0x" << std::hex << function.getFunctionAddress() << std::dec
           << std::endl;
    stream << prefix << "Stack Size [uint64]: 0x" << std::hex << function.getStackSize() << std::dec << std::endl;
    stream << prefix << "Record Count [uint64]: " << function.getRecordCount() << std::endl;
}

void CodeInfoProducer::DumpStackMapRecord(const LLVMStackMap::RecordAccessor &record, std::ostream &stream,
                                          const std::string &prefix)
{
    stream << prefix << "PatchPoint ID [uint64]: " << record.getID() << std::endl;
    stream << prefix << "Instruction Offset [uint32]: 0x" << std::hex << record.getInstructionOffset() << std::dec
           << std::endl;
    stream << prefix << "Location [" << record.getNumLocations() << "]: [" << std::endl;
    for (const auto &loc : record.locations()) {
        stream << prefix << "  {" << std::endl;
        DumpStackMapLocation(loc, stream, prefix + "    ");
        stream << prefix << "  }" << std::endl;
    }
    stream << prefix << "]" << std::endl;

    stream << prefix << "LiveOuts [" << record.getNumLiveOuts() << "]: [" << std::endl;
    for (const auto &liveout : record.liveouts()) {
        stream << prefix << "  {" << std::endl;
        stream << prefix << "    Dwarf RegNum [uint16]: " << liveout.getDwarfRegNum() << std::endl;
        stream << prefix << "    Size in Bytes [uint8_t]: " << liveout.getSizeInBytes() << std::endl;
        stream << prefix << "  }" << std::endl;
    }
    stream << prefix << "]" << std::endl;
}

void CodeInfoProducer::DumpStackMapLocation(const LLVMStackMap::LocationAccessor &location, std::ostream &stream,
                                            const std::string &prefix)
{
    const char *kind;
    switch (location.getKind()) {
        case LLVMStackMap::LocationKind::Register:
            kind = "0x1 | Register | Reg | Value in a register";
            break;
        case LLVMStackMap::LocationKind::Direct:
            kind = "0x2 | Direct | Reg + Offset | Frame index value";
            break;
        case LLVMStackMap::LocationKind::Indirect:
            kind = "0x3 | Indirect | [Reg + Offset] | Spilled value";
            break;
        case LLVMStackMap::LocationKind::Constant:
            kind = "0x4 | Constant | Offset | Small constant";
            break;
        case LLVMStackMap::LocationKind::ConstantIndex:
            kind = "0x5 | ConstIndex | Constants[Offset] | Large constant";
            break;
        default:
            kind = "Unknown";
            break;
    }
    stream << prefix << "Kind [uint8]: " << kind << std::endl;
    stream << prefix << "Location Size [uint16]: " << location.getSizeInBytes() << std::endl;
    stream << prefix << "Dwarf RegNum [uint16]: " << location.getDwarfRegNum() << std::endl;
    switch (location.getKind()) {
        case LLVMStackMap::LocationKind::Register:
            stream << 0;
            break;
        case LLVMStackMap::LocationKind::Direct:
        case LLVMStackMap::LocationKind::Indirect:
            stream << prefix << "Offset [int32]: " << location.getOffset();
            break;
        case LLVMStackMap::LocationKind::Constant:
            stream << prefix << "SmallConstant [int32]: " << location.getSmallConstant();
            break;
        case LLVMStackMap::LocationKind::ConstantIndex:
            stream << prefix << "ConstantIndex [int32]: " << location.getConstantIndex();
            break;
        default:
            stream << "Unknown";
            break;
    }
    stream << std::endl;
}

}  // namespace ark::llvmbackend
