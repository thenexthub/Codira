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

#ifndef CODIRA_CHIR_SERIALIZER_IMPL_H
#define CODIRA_CHIR_SERIALIZER_IMPL_H

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif
#include <flatbuffers/PackageFormat_generated.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include <deque>
#include <fstream>
#include <iostream>
#include <queue>

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/CHIRContext.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Serializer/CHIRSerializer.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/UserDefinedType.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {

class CHIRSerializer::CHIRSerializerImpl {
public:
    explicit CHIRSerializerImpl(const Package& package) : package(package){};

    // Utility
    void Save(const std::string& filename, ToCHIR::Phase phase);
    void Initialize();
    void Dispatch();

private:
    const Package& package;

    flatbuffers::FlatBufferBuilder builder;
    std::deque<const Value*> valueQueue;
    std::queue<const Type*> typeQueue;
    std::queue<const Expression*> exprQueue;
    std::deque<const CustomTypeDef*> defQueue;

    uint32_t typeCount = 0;
    uint32_t valueCount = 0;
    uint32_t exprCount = 0;
    uint32_t defCount = 0;

    // Id maps
    std::unordered_map<const Type*, uint32_t> type2Id{{nullptr, 0}};
    std::unordered_map<const Value*, uint32_t> value2Id{{nullptr, 0}};
    std::unordered_map<const Expression*, uint32_t> expr2Id{{nullptr, 0}};
    std::unordered_map<const CustomTypeDef*, uint32_t> def2Id{{nullptr, 0}};

    // Kind Indicators
    std::vector<uint8_t> typeKind{};
    std::vector<uint8_t> valueKind{};
    std::vector<uint8_t> exprKind{};
    std::vector<uint8_t> defKind{};

    // Containers
    std::vector<flatbuffers::Offset<void>> allType{};
    std::vector<flatbuffers::Offset<void>> allValue{};
    std::vector<flatbuffers::Offset<void>> allExpression{};
    std::vector<flatbuffers::Offset<void>> allCustomTypeDef{};

    // Serializers
    template <typename FBT, typename T> flatbuffers::Offset<FBT> Serialize(const T& obj);
    template <typename FBT, typename T> std::vector<flatbuffers::Offset<FBT>> SerializeVec(const std::vector<T>& vec);
    template <typename FBT, typename T>
    std::vector<flatbuffers::Offset<FBT>> SerializeSetToVec(const std::unordered_set<T>& set) const;
    std::vector<flatbuffers::Offset<PackageFormat::VTableElement>> SerializeVTable(const VTableType& obj);
    // Dispatchers
    template <typename T> flatbuffers::Offset<void> Dispatch(const T& obj);

    // Fetch ID
    template <typename T> uint32_t GetId(const T* obj);
    template <typename T, typename E> std::vector<uint32_t> GetId(std::vector<E*> vec);
    template <typename T, typename E> std::vector<uint32_t> GetId(std::vector<Ptr<E>> vec);
    template <typename T, typename E> std::vector<uint32_t> GetId(const std::unordered_set<E*>& set) const;
    // other to save
    unsigned packageInitFunc{};
    unsigned packageLiteralInitFunc{};
    uint32_t maxImportedValueId = 0;
    uint32_t maxImportedStructId = 0;
    uint32_t maxImportedClassId = 0;
    uint32_t maxImportedEnumId = 0;
    uint32_t maxImportedExtendId = 0;
};
} // namespace Codira::CHIR

#endif
