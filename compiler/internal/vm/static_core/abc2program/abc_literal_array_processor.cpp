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

#include "abc_literal_array_processor.h"
#include "abc2program_log.h"

namespace ark::abc2program {

AbcLiteralArrayProcessor::AbcLiteralArrayProcessor(panda_file::File::EntityId entityId, Abc2ProgramKeyData &keyData)
    : AbcFileEntityProcessor(entityId, keyData)
{
    literalDataAccessor_ = std::make_unique<panda_file::LiteralDataAccessor>(*file_, entityId_);
    FillProgramData();
}

template <typename T>
void AbcLiteralArrayProcessor::FillLiteralArrayData(pandasm::LiteralArray *litArray, const panda_file::LiteralTag &tag,
                                                    const panda_file::LiteralDataAccessor::LiteralValue &value) const
{
    panda_file::File::EntityId id(std::get<uint32_t>(value));
    auto sp = file_->GetSpanFromId(id);
    auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        for (size_t i = 0; i < len; i++) {
            pandasm::LiteralArray::Literal lit;
            lit.tag = tag;
            lit.value = bit_cast<T>(panda_file::helpers::Read<sizeof(T)>(&sp));
            litArray->literals.push_back(lit);
        }
    } else {
        for (size_t i = 0; i < len; i++) {
            auto strId = panda_file::helpers::Read<sizeof(T)>(&sp);
            pandasm::LiteralArray::Literal lit;
            lit.tag = tag;
            lit.value = stringTable_->GetStringById(panda_file::File::EntityId(strId));
            litArray->literals.push_back(lit);
        }
    }
}

void AbcLiteralArrayProcessor::FillLiteralData(pandasm::LiteralArray *litArray,
                                               const panda_file::LiteralDataAccessor::LiteralValue &value,
                                               const panda_file::LiteralTag &tag) const
{
    pandasm::LiteralArray::Literal lit;
    lit.tag = tag;
    switch (tag) {
        case panda_file::LiteralTag::BOOL: {
            lit.value = std::get<bool>(value);
            break;
        }
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE: {
            lit.value = std::get<uint8_t>(value);
            break;
        }
        case panda_file::LiteralTag::METHODAFFILIATE: {
            lit.value = std::get<uint16_t>(value);
            break;
        }
        case panda_file::LiteralTag::INTEGER: {
            lit.value = std::get<uint32_t>(value);
            break;
        }
        case panda_file::LiteralTag::BIGINT: {
            lit.value = std::get<uint64_t>(value);
            break;
        }
        case panda_file::LiteralTag::FLOAT: {
            lit.value = std::get<float>(value);
            break;
        }
        case panda_file::LiteralTag::DOUBLE: {
            lit.value = std::get<double>(value);
            break;
        }
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD: {
            lit.value = stringTable_->GetStringById(panda_file::File::EntityId(std::get<uint32_t>(value)));
            break;
        }
        case panda_file::LiteralTag::TAGVALUE: {
            return;
        }
        default: {
            LOG(ERROR, ABC2PROGRAM) << "Unsupported literal with tag 0x" << std::hex << static_cast<uint32_t>(tag);
            UNREACHABLE();
        }
    }
    litArray->literals.push_back(lit);
}

void AbcLiteralArrayProcessor::GetLiteralArray(pandasm::LiteralArray *litArray, const size_t index)
{
    LOG(DEBUG, ABC2PROGRAM) << "\n[getting literal array]\nindex: " << index;

    literalDataAccessor_->EnumerateLiteralVals(
        index, [this, litArray](const panda_file::LiteralDataAccessor::LiteralValue &value,
                                const panda_file::LiteralTag &tag) {
            litArray->literals.emplace_back(
                pandasm::LiteralArray::Literal {panda_file::LiteralTag::TAGVALUE, static_cast<uint8_t>(tag)});
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U1: {
                    FillLiteralArrayData<bool>(litArray, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I8:
                case panda_file::LiteralTag::ARRAY_U8: {
                    FillLiteralArrayData<uint8_t>(litArray, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I16:
                case panda_file::LiteralTag::ARRAY_U16: {
                    FillLiteralArrayData<uint16_t>(litArray, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I32:
                case panda_file::LiteralTag::ARRAY_U32: {
                    FillLiteralArrayData<uint32_t>(litArray, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I64:
                case panda_file::LiteralTag::ARRAY_U64: {
                    FillLiteralArrayData<uint64_t>(litArray, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_F32: {
                    FillLiteralArrayData<float>(litArray, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_F64: {
                    FillLiteralArrayData<double>(litArray, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_STRING: {
                    FillLiteralArrayData<uint32_t>(litArray, tag, value);
                    break;
                }
                default: {
                    FillLiteralData(litArray, value, tag);
                }
            }
        });
}

void AbcLiteralArrayProcessor::FillProgramData()
{
    LOG(DEBUG, ABC2PROGRAM) << "\n[getting literal arrays]\nid: " << entityId_ << " (0x" << std::hex << entityId_
                            << ")";

    size_t numLitarrays = literalDataAccessor_->GetLiteralNum();
    for (size_t index = 0; index < numLitarrays; index++) {
        ark::pandasm::LiteralArray litAr;
        GetLiteralArray(&litAr, index);
        program_->literalarrayTable.emplace(std::to_string(index), litAr);
    }
}

}  // namespace ark::abc2program
