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

#ifndef CODIRA_EXTENDDEF_H
#define CODIRA_EXTENDDEF_H

#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Type/Type.h"

namespace Codira::CHIR {
class ExtendDef : public CustomTypeDef {
friend class CHIRContext;
friend class CHIRBuilder;
friend class CustomDefTypeConverter;

public:
    // ===--------------------------------------------------------------------===//
    // Base Information
    // ===--------------------------------------------------------------------===//
    Type* GetType() const override;
    void SetType(CustomType& ty) override;
    
    Type* GetExtendedType() const;
    void SetExtendedType(Type& ty);
    
    CustomTypeDef* GetExtendedCustomTypeDef() const;
    virtual std::vector<GenericType*> GetGenericTypeParams() const override;

    // ===--------------------------------------------------------------------===//
    // Super Parent
    // ===--------------------------------------------------------------------===//
    void RemoveParent(ClassType& parent);
    
protected:
    void PrintAttrAndTitle(std::stringstream& ss) const override;
    void PrintComment(std::stringstream& ss) const override;

private:
    explicit ExtendDef(
        const std::string& identifier, const std::string& pkgName, std::vector<GenericType*> genericParams = {});

    Type* extendedType{nullptr};
    std::vector<GenericType*> genericParams;
};
}
#endif
