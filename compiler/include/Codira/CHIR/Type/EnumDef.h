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

#ifndef CODIRA_CHIR_ENUM_H
#define CODIRA_CHIR_ENUM_H

#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Type/Type.h"
#include <string>
#include <vector>

namespace Codira::CHIR {
struct EnumCtorInfo {
    std::string name;
    std::string mangledName;
    FuncType* funcType; /**< (AssociatedType_1, ..., AssociatedType_N) -> EnumType */
};

class EnumDef : public CustomTypeDef {
    friend class CustomDefTypeConverter;
public:
    // ===--------------------------------------------------------------------===//
    // Base Infomation
    // ===--------------------------------------------------------------------===//
    EnumType* GetType() const override;
    void SetType(CustomType& ty) override;
    
    /**
    * @brief an enum def like: enum XXX { A | B | ... }, is named NOT exhaustive
    *
    * @return true if enum is exhaustive
    */
    bool IsExhaustive() const;

    std::string ToString() const override;

    void AddCtor(EnumCtorInfo ctor);
    EnumCtorInfo GetCtor(size_t index) const;
    std::vector<EnumCtorInfo> GetCtors() const;
    void SetCtors(const std::vector<EnumCtorInfo>& items);

    /**
    * @brief check if all constructors is trivial
    *
    * @return true if all constructors do not have parameters
    */
    bool IsAllCtorsTrivial() const;

protected:
    void PrintAttrAndTitle(std::stringstream& ss) const override;

private:
    explicit EnumDef(std::string srcCodeIdentifier, std::string identifier, std::string pkgName, bool isNonExhaustive)
        : CustomTypeDef(srcCodeIdentifier, identifier, pkgName, CustomDefKind::TYPE_ENUM),
        nonExhaustive{isNonExhaustive}
    {
    }
    ~EnumDef() override = default;

    void PrintConstructor(std::stringstream& ss) const;
    friend class CHIRContext;
    friend class CHIRBuilder;

    std::vector<EnumCtorInfo> ctors;
    bool nonExhaustive;
};
} // namespace Codira::CHIR

#endif
