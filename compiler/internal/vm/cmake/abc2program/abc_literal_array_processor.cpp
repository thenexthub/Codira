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

namespace panda::abc2program {

AbcLiteralArrayProcessor::AbcLiteralArrayProcessor(panda_file::File::EntityId entity_id,
                                                   Abc2ProgramEntityContainer &entity_container,
                                                   panda_file::LiteralDataAccessor &literal_data_accessor)
    : AbcFileEntityProcessor(entity_id, entity_container), literal_data_accessor_(literal_data_accessor) {}

void AbcLiteralArrayProcessor::FillProgramData()
{
    GetLiteralArrayById(&literal_array_, entity_id_);
    program_->literalarray_table.emplace(entity_container_.GetLiteralArrayIdName(entity_id_.GetOffset()),
                                         std::move(literal_array_));
}

void AbcLiteralArrayProcessor::FillModuleRequestPhase()
{
    auto sp = file_->GetSpanFromId(entity_id_);
    auto literal_vals_num = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);
    for (size_t i = 0; i < literal_vals_num; i++) {
        pandasm::LiteralArray::Literal lit;
        lit.tag_ = panda_file::LiteralTag::INTEGER_8;
        lit.value_ = static_cast<uint8_t>(panda_file::helpers::Read<sizeof(uint8_t)>(&sp));
        literal_array_.literals_.emplace_back(lit);
    }
    program_->literalarray_table.emplace(entity_container_.GetLiteralArrayIdName(entity_id_.GetOffset()),
                                         std::move(literal_array_));
}

void AbcLiteralArrayProcessor::GetLiteralArrayById(pandasm::LiteralArray *lit_array,
                                                   panda_file::File::EntityId lit_array_id) const
{
    literal_data_accessor_.EnumerateLiteralVals(
        lit_array_id, [this, lit_array](
                    const panda_file::LiteralDataAccessor::LiteralValue &value,
                    const panda_file::LiteralTag &tag) {
            FillLiteralData(lit_array, value, tag);
        });
}

void AbcLiteralArrayProcessor::FillLiteralData(pandasm::LiteralArray *lit_array,
                                               const panda_file::LiteralDataAccessor::LiteralValue &value,
                                               const panda_file::LiteralTag &tag) const
{
    pandasm::LiteralArray::Literal value_lit;
    pandasm::LiteralArray::Literal tag_lit;
    value_lit.tag_ = tag;
    switch (tag) {
        case panda_file::LiteralTag::BOOL:
            value_lit.value_ = std::get<bool>(value);
            break;
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE:
        case panda_file::LiteralTag::BUILTINTYPEINDEX:
            value_lit.value_ = std::get<uint8_t>(value);
            break;
        case panda_file::LiteralTag::METHODAFFILIATE:
            value_lit.value_ = std::get<uint16_t>(value);
            break;
        case panda_file::LiteralTag::LITERALBUFFERINDEX:
        case panda_file::LiteralTag::INTEGER:
            value_lit.value_ = std::get<uint32_t>(value);
            break;
        case panda_file::LiteralTag::DOUBLE:
            value_lit.value_ = std::get<double>(value);
            break;
        case panda_file::LiteralTag::STRING:
        case panda_file::LiteralTag::ETS_IMPLEMENTS:
            value_lit.value_ = GetStringById(panda_file::File::EntityId{std::get<uint32_t>(value)});
            break;
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GETTER:
        case panda_file::LiteralTag::SETTER:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD: {
            panda_file::File::EntityId entity_id(std::get<uint32_t>(value));
            value_lit.value_ = entity_container_.GetFullMethodNameById(entity_id);
            break;
        }
        case panda_file::LiteralTag::LITERALARRAY:
            value_lit.value_ = entity_container_.GetLiteralArrayIdName(std::get<uint32_t>(value));
            entity_container_.TryAddUnprocessedNestedLiteralArrayId(std::get<uint32_t>(value));
            break;
        case panda_file::LiteralTag::TAGVALUE:
            value_lit.value_ = std::get<uint8_t>(value);
            break;
        default:
            UNREACHABLE();
    }

    tag_lit.tag_ = panda_file::LiteralTag::TAGVALUE;
    tag_lit.value_ = static_cast<uint8_t>(value_lit.tag_);

    lit_array->literals_.emplace_back(tag_lit);
    lit_array->literals_.emplace_back(value_lit);
}

} // namespace panda::abc2program
