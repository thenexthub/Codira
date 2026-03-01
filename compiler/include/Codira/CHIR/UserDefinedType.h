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

#ifndef CODIRA_CHIR_USER_DEFINED_TYPE_H
#define CODIRA_CHIR_USER_DEFINED_TYPE_H

#include "Codira/AST/Node.h"
#include "Codira/CHIR/AttributeInfo.h"
#include <functional>
#include <map>
#include <vector>

namespace Codira::CHIR {

class FuncType;
class Type;
class GenericType;
class FuncBase;
class AttributeInfo;
class ClassType;
class Translator;
class Value;
class CustomTypeDef;

class CHIRTypeCompare {
public:
    bool operator()(const Type* key1, const Type* key2) const;
};

struct VirtualFuncTypeInfo {
    FuncType* sigType{nullptr}; // instantiated type, (param types)->Unit, param types exclude `this` type
    FuncType* originalType{nullptr}; // virtual func's original func type from parent def, (param types)->retType,
                                     // param types include `this` type
    Type* parentType{nullptr}; // CustomType or extended type(may be primitive type)
    Type* returnType{nullptr}; // instantiated type
    std::vector<GenericType*> methodGenericTypeParams; // store `T` of `func foo<T>()`
};

// for vtable
struct VirtualFuncInfo {
    std::string srcCodeIdentifier;
    FuncBase* instance{nullptr};
    AttributeInfo attr;
    VirtualFuncTypeInfo typeInfo; // store virtual func's type info,
                                  // if virtual func need to be wrappered, don't update type info
};

using VTableType = std::map<ClassType*, std::vector<VirtualFuncInfo>, CHIRTypeCompare>;

using TranslateASTNodeFunc = std::function<Value*(const Codira::AST::Decl&, Translator&)>;

struct FuncSigInfo {
    std::string funcName;         // src code name
    FuncType* funcType{nullptr};  // declared type, including `this` type and return type
                                  // there may be generic type in it
    std::vector<GenericType*> genericTypeParams;
};

struct FuncCallType {
    std::string funcName;         // src code name
    FuncType* funcType{nullptr};  // inst type, including `this` type and return type
    std::vector<Type*> genericTypeArgs;
};

struct VTableSearchRes {
    ClassType* instSrcParentType{nullptr};     // instantiated by instantiate func type
    ClassType* halfInstSrcParentType{nullptr}; // instantiated by current def
    FuncType* originalFuncType{nullptr};       // a generic func type, from current def, not parent def
    FuncBase* instance{nullptr};
    CustomTypeDef* originalDef{nullptr};       // this virtual func belongs to a vtable,
                                               // and this vtable belongs to a CustomTypeDef
    std::vector<GenericType*> genericTypeParams;
    AttributeInfo attr;
    size_t offset{0};
};

using ConvertTypeFunc = std::function<Type*(Type&)>;
}
#endif
