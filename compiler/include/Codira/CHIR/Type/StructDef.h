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

#ifndef CODIRA_CHIR_STRUCT_H
#define CODIRA_CHIR_STRUCT_H

#include "Codira/CHIR/Type/CustomTypeDef.h"
#include <string>
#include <vector>

namespace Codira::CHIR {
class StructDef : public CustomTypeDef {
public:
    // ===--------------------------------------------------------------------===//
    // Base Infomation
    // ===--------------------------------------------------------------------===//
    StructType* GetType() const override;
    void SetType(CustomType& ty) override;

    /**
     * @brief return true if this struct annotated with @C
     */
    bool IsCStruct() const;
    void SetCStruct(bool value);

protected:
    void PrintComment(std::stringstream& ss) const override;
    
private:
    explicit StructDef(std::string srcCodeIdentifier, std::string identifier, std::string pkgName)
        : CustomTypeDef(srcCodeIdentifier, identifier, pkgName, CustomDefKind::TYPE_STRUCT)
    {}
    ~StructDef() override = default;
    friend class CHIRContext;
    friend class CHIRBuilder;

    bool isC = false;
};
} // namespace Codira::CHIR

#endif
