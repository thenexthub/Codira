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

#ifndef CODIRA_CHIR_DESERIALIZER_IMPL_H
#define CODIRA_CHIR_DESERIALIZER_IMPL_H

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif
#include <flatbuffers/PackageFormat_generated.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#include <fstream>
#include <iostream>
#include <utility>

#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/CHIRContext.h"
#include "Codira/CHIR/Package.h"
#include "Codira/CHIR/Serializer/CHIRDeserializer.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {

class CHIRDeserializer::CHIRDeserializerImpl {
public:
    void ConfigBase(const PackageFormat::Base* buffer, Base& obj);
    void ConfigValue(const PackageFormat::Value* buffer, Value& obj);
    void ConfigCustomTypeDef(const PackageFormat::CustomTypeDef* buffer, CustomTypeDef& obj);
    void ConfigExpression(const PackageFormat::Expression* buffer, Expression& obj);
    template <typename T, typename FBT> T Create(const FBT* obj);
    template <typename T, typename FBT> std::vector<T> Create(const flatbuffers::Vector<FBT>* vec);
    template <typename T, typename FBT> T* Deserialize(const FBT* obj);
    template <typename T> std::vector<T*> GetValue(const flatbuffers::Vector<uint32_t>* vec);
    template <typename T> std::vector<T*> GetType(const flatbuffers::Vector<uint32_t>* vec);
    template <typename T> std::vector<T*> GetExpression(const flatbuffers::Vector<uint32_t>* vec);
    template <typename T> std::vector<T*> GetCustomTypeDef(const flatbuffers::Vector<uint32_t>* vec);
    template <typename T> T* GetValue(uint32_t id);
    Value* GetValue(uint32_t id);
    template <typename T> T* GetType(uint32_t id);
    Type* GetType(uint32_t id);
    template <typename T> T* GetExpression(uint32_t id);
    Expression* GetExpression(uint32_t id);
    template <typename T> T* GetCustomTypeDef(uint32_t id);
    CustomTypeDef* GetCustomTypeDef(uint32_t id);

    template <typename T, typename FBT> void Config(const FBT* buffer, T& obj);

    void Run(const PackageFormat::CHIRPackage* package);
    explicit CHIRDeserializerImpl(CHIRBuilder& chirBuilder, bool compilePlatform = false)
        : builder(chirBuilder), compilePlatform(compilePlatform){};

private:
    Codira::CHIR::CHIRBuilder& builder;
    bool compilePlatform = false;
    const PackageFormat::CHIRPackage* pool{};

    // Package object maps
    std::unordered_map<uint32_t, Type*> id2Type{{0, nullptr}};
    std::unordered_map<uint32_t, Value*> id2Value{{0, nullptr}};
    std::unordered_map<uint32_t, Expression*> id2Expression{{0, nullptr}};
    std::unordered_map<uint32_t, CustomTypeDef*> id2CustomTypeDef{{0, nullptr}};

    // lazy GenericType config
    std::vector<std::pair<GenericType*, const PackageFormat::GenericType*>> genericTypeConfig;

    void ResetImportedValuesUnderPackage();
    void ResetImportedDefsUnderPackage();
};
}
#endif
