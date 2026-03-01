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

#ifndef CODIRA_CHIR_CLASS_H
#define CODIRA_CHIR_CLASS_H

#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Codira::CHIR {
class ClassDef : public CustomTypeDef {
    friend class CustomDefTypeConverter;
    friend class CHIRDeserializer;
    friend class CHIRSerializer;

public:
    // ===--------------------------------------------------------------------===//
    // Base Infomation
    // ===--------------------------------------------------------------------===//
    ClassType* GetType() const override;
    void SetType(CustomType& ty) override;
    
    bool IsAbstract() const;
    bool IsClass() const;
    bool IsInterface() const;

    std::string ToString() const override;

    /**
     * @brief Whether this class is user defined annotation.
     *
     * @return return true for classes that are marked with the @Annotation annotation.
     */
    bool IsAnnotation() const;
    void SetAnnotation(bool value);
    // ===--------------------------------------------------------------------===//
    // Super Parent
    // ===--------------------------------------------------------------------===//
    ClassType* GetSuperClassTy() const;
    ClassDef* GetSuperClassDef() const;
    bool HasSuperClass() const;
    void SetSuperClassTy(ClassType& ty);

    // ===--------------------------------------------------------------------===//
    // Member Function
    // ===--------------------------------------------------------------------===//
    void AddMethod(class FuncBase* method, bool recordOrder = true) override;
    void AddAbstractMethod(AbstractMethodInfo methodInfo, bool recordOrder = true);
    std::vector<AbstractMethodInfo> GetAbstractMethods() const;
    void SetAbstractMethods(const std::vector<AbstractMethodInfo>& methods);
    const std::vector<std::string>& GetAllMethodMangledNames() const;
    void SetAllMethodMangledNames(const std::vector<std::string>& names);
    FuncBase* GetFinalizer() const;

protected:
    void PrintComment(std::stringstream& ss) const override;
    
private:
    explicit ClassDef(std::string srcCodeIdentifier, std::string identifier,
        std::string pkgName, bool isClass);
    ~ClassDef() override = default;
    friend class CHIRContext;
    friend class CHIRBuilder;
    void PrintAbstractMethod(std::stringstream& ss) const;

    bool isClass = false;           // class or interface
    bool isAnnotation = false;      // whether the class is modified by @Annotation
    ClassType* superClassTy = nullptr;
    std::vector<AbstractMethodInfo> abstractMethods;
    std::vector<std::string> allMethodMangledNames;
};
} // namespace Codira::CHIR

#endif
