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

#ifndef LSPSERVER_FINDOVERRIDEMETHODSUTILS_H
#define LSPSERVER_FINDOVERRIDEMETHODSUTILS_H

#include "Codira/AST/Node.h"
#include "Codira/Basic/Utils.h"

namespace ark {
using namespace Codira;
using namespace Codira::AST;

template<typename T>
std::string Join(const std::vector<T>& items, const std::string& insert)
{
    std::string result;
    bool first = true;
    for (const T& item: items) {
        if (!first) {
            result += insert;
        }
        first = false;
        result += item.ToString();
    }
    return result;
}

template<typename T>
std::string PtrJoin(const std::vector<T>& items, const std::string& insert)
{
    std::string result;
    bool first = true;
    for (const T& item: items) {
        if (!first) {
            result += insert;
        }
        first = false;
        result += item->ToString();
    }
    return result;
}

struct TypeDetail {
    std::string identifier;
    TypeDetail() = default;
    virtual ~TypeDetail() = default;
    explicit TypeDetail(std::string identifier): identifier(std::move(identifier)) {}
    virtual std::string ToString() const
    {
        return identifier;
    }

    virtual void SetIdentifier(const std::string& oldId, const std::string& newId) {}

    virtual bool Comparable(const std::unique_ptr<TypeDetail>& other)
    {
        return false;
    }

    virtual void Diff(const std::unique_ptr<TypeDetail>& other, std::unordered_map<std::string, std::string>& diffs) {}
};

struct CommonTypeDetail: public TypeDetail {
    CommonTypeDetail() = default;
    explicit  CommonTypeDetail(std::string identifier): TypeDetail(std::move(identifier)) {}
    void SetIdentifier(const std::string& oldId, const std::string& newId) override
    {
        if (identifier == oldId) {
            identifier = newId;
        }
    }

    bool Comparable(const std::unique_ptr<TypeDetail> &other) override
    {
        return true;
    }

    void Diff(const std::unique_ptr<TypeDetail>& other, std::unordered_map<std::string, std::string>& diffs) override
    {
        if (!Comparable(other)) {
            return;
        }
        if (this->identifier != other->identifier) {
            diffs[this->identifier] = other->ToString();
        }
    }
};

struct ClassLikeTypeDetail: public TypeDetail {
    ClassLikeTypeDetail() = default;
    explicit ClassLikeTypeDetail(std::string identifier): TypeDetail(std::move(identifier)) {}
    std::vector<std::unique_ptr<TypeDetail>> generics;

    std::string ToString() const override
    {
        std::string detail = identifier;
        if (generics.empty()) {
            return detail;
        }
        detail += "<";
        detail += PtrJoin(generics, ", ");
        detail += ">";
        return detail;
    }

    void SetIdentifier(const std::string& oldId, const std::string& newId) override
    {
        for (auto& generic: generics) {
            generic->SetIdentifier(oldId, newId);
        }
    }

    bool Comparable(const std::unique_ptr<TypeDetail> &other) override
    {
        auto casted = dynamic_cast<ClassLikeTypeDetail*>(other.get());
        if (!casted) {
            return false;
        }

        if (this->identifier == casted->identifier && this->generics.size() == casted->generics.size()) {
            return true;
        }

        return false;
    }

    void Diff(const std::unique_ptr<TypeDetail> &other, std::unordered_map<std::string, std::string> &diffs) override
    {
        if (!Comparable(other)) {
            return;
        }

        if (auto casted = dynamic_cast<ClassLikeTypeDetail*>(other.get())) {
            for (size_t i = 0; i < generics.size(); i++) {
                generics[i]->Diff(casted->generics[i], diffs);
            }
        }
    }
};

struct FuncLikeTypeDetail: public TypeDetail {
    FuncLikeTypeDetail() = default;
    std::vector<std::unique_ptr<TypeDetail>> params;
    std::unique_ptr<TypeDetail> ret;

    explicit FuncLikeTypeDetail(std::string identifier): TypeDetail(std::move(identifier)) {}

    std::string ToString() const override
    {
        std::string detail = "(";
        detail += PtrJoin(params, ", ");
        detail += ")";
        detail += " -> ";
        if (ret) {
            detail += ret->ToString();
        }
        return detail;
    }

    void SetIdentifier(const std::string& oldId, const std::string& newId) override
    {
        for (auto& param: params) {
            param->SetIdentifier(oldId, newId);
        }
        if (ret) {
            ret->SetIdentifier(oldId, newId);
        }
    }

    bool Comparable(const std::unique_ptr<TypeDetail> &other) override
    {
        auto casted = dynamic_cast<FuncLikeTypeDetail*>(other.get());
        if (!casted) {
            return false;
        }

        if (this->params.size() == casted->params.size()) {
            return true;
        }

        return false;
    }

    void Diff(const std::unique_ptr<TypeDetail> &other, std::unordered_map<std::string, std::string> &diffs) override
    {
        if (!Comparable(other)) {
            return;
        }

        if (auto casted = dynamic_cast<FuncLikeTypeDetail*>(other.get())) {
            for (size_t i = 0; i < params.size(); i++) {
                if (params[i]) {
                    params[i]->Diff(casted->params[i], diffs);
                }
            }
            if (ret) {
                ret->Diff(casted->ret, diffs);
            }
        }
    }
};

struct VArrayTypeDetail: public TypeDetail {
    std::unique_ptr<TypeDetail> tyArg;
    size_t size = 0;

    std::string ToString() const override
    {
        std::string detail = identifier;
        detail += "<";
        if (tyArg) {
            detail += tyArg->ToString();
        }
        detail += ", $";
        detail += std::to_string(size);
        detail += ">";
        return detail;
    }

    void SetIdentifier(const std::string& oldId, const std::string& newId) override
    {
        tyArg->SetIdentifier(oldId, newId);
    }

    bool Comparable(const std::unique_ptr<TypeDetail> &other) override
    {
        auto casted = dynamic_cast<VArrayTypeDetail*>(other.get());
        return casted;
    }

    void Diff(const std::unique_ptr<TypeDetail> &other, std::unordered_map<std::string, std::string> &diffs) override
    {
        if (!Comparable(other)) {
            return;
        }
        auto casted = dynamic_cast<VArrayTypeDetail*>(other.get());
        tyArg->Diff(casted->tyArg, diffs);
    }
};

struct TupleTypeDetail: public TypeDetail {
    std::vector<std::unique_ptr<TypeDetail>> params;

    std::string ToString() const override
    {
        std::string detail;
        detail += "(";
        detail += PtrJoin(params, ", ");
        detail += ")";
        return detail;
    }

    void SetIdentifier(const std::string& oldId, const std::string& newId) override
    {
        for (auto& param: params) {
            if (param) {
                param->SetIdentifier(oldId, newId);
            }
        }
    }

    bool Comparable(const std::unique_ptr<TypeDetail> &other) override
    {
        auto casted = dynamic_cast<TupleTypeDetail*>(other.get());
        if (!casted) {
            return false;
        }

        if (casted->params.size() != this->params.size()) {
            return false;
        }

        for (size_t i = 0; i < params.size(); i++) {
            if (!params[i] || !casted->params[i] || !params[i]->Comparable(casted->params[i])) {
                return false;
            }
        }

        return true;
    }

    void Diff(const std::unique_ptr<TypeDetail> &other, std::unordered_map<std::string, std::string> &diffs) override
    {
        if (!Comparable(other)) {
            return;
        }
        auto casted = dynamic_cast<TupleTypeDetail*>(other.get());
        for (size_t i = 0; i < params.size(); i++) {
            params[i]->Diff(casted->params[i], diffs);
        }
    }
};

struct FuncParamDetail {
    std::unique_ptr<TypeDetail> type;
    std::string identifier;
    bool isNamed;

    void SetGenericType(const std::string& oldTy, const std::string& newTy) const
    {
        type->SetIdentifier(oldTy, newTy);
    }

    std::string ToString() const
    {
        std::string detail = identifier;
        if (isNamed) {
            detail += "!";
        }
        detail += ": ";
        if (type) {
            detail += type->ToString();
        }
        return detail;
    }
};

struct FuncParamDetailList {
    std::vector<FuncParamDetail> params;
    bool isVariadic = false;

    void SetGenericType(const std::string& oldTy, const std::string& newTy)
    {
        for (auto& param: params) {
            param.SetGenericType(oldTy, newTy);
        }
    }

    std::string ToString() const
    {
        std::string detail;
        detail += Join(params, ", ");
        if (isVariadic) {
            detail += ", ...";
        }
        return detail;
    }
};

struct FuncDetail {
    std::vector<std::string> modifiers;
    std::string identifier;
    FuncParamDetailList params;
    std::unique_ptr<TypeDetail> retType;

    void SetGenericType(const std::string& oldTy, const std::string& newTy)
    {
        params.SetGenericType(oldTy, newTy);
        if (retType) {
            retType->SetIdentifier(oldTy, newTy);
        }
    }

    std::string ToString() const
    {
        std::string detail = Codira::Utils::JoinStrings(modifiers, " ");
        if (!detail.empty()) {
            detail += " ";
        }
        detail += "func ";
        detail += identifier;
        detail += "(";
        detail += params.ToString();
        detail += "): ";
        if (retType) {
            detail += retType->ToString();
        }
        return detail;
    }

    std::string GetFunctionName() const
    {
        std::string detail;
        detail += identifier;
        detail += "(";
        detail += params.ToString();
        detail += ")";
        return detail;
    }

    std::string GetFunctionWithRet() const
    {
        std::string detail = GetFunctionName();
        if (retType) {
            detail += ": " + retType->ToString();
        }
        return detail;
    }
};

struct PropDetail {
    std::vector<std::string> modifiers;
    std::string identifier;
    std::unique_ptr<TypeDetail> type;

    void SetGenericType(const std::string& oldTy, const std::string& newTy) const
    {
        if (type) {
            type->SetIdentifier(oldTy, newTy);
        }
    }

    std::string ToString() const
    {
        std::string detail = Codira::Utils::JoinStrings(modifiers, " ");
        detail += " ";
        detail += "prop";
        detail += " ";
        detail += identifier;
        detail += ": ";
        if (type) {
            detail += type->ToString();
        }
        return detail;
    }
};

inline std::vector<std::pair<Attribute, std::string>> attr2str = {
    {Attribute::PRIVATE, "private"},
    {Attribute::INTERNAL, "internal"},
    {Attribute::PROTECTED, "protected"},
    {Attribute::PUBLIC, "public"},
    {Attribute::SEALED, "sealed"},
    {Attribute::STATIC, "static"},
    {Attribute::UNSAFE, "unsafe"},
    {Attribute::MUT, "mut"},
    {Attribute::ABSTRACT, "abstract"},
    {Attribute::OPEN, "open"},
};

FuncDetail ResolveFuncDetail(const Ptr<FuncDecl>& funcDecl);

PropDetail ResolvePropDetail(const Ptr<PropDecl>& propDecl);

std::vector<std::string> ResolveDeclModifiers(const Ptr<Decl>& Decl);

std::string ResolveDeclIdentifier(const Ptr<Decl>& funcDecl);

FuncParamDetailList ResolveFuncParamList(const Ptr<FuncDecl>& funcDecl);

std::unique_ptr<TypeDetail> ResolveFuncRetType(const Ptr<FuncDecl>& funcDecl);

std::unique_ptr<TypeDetail> ResolveType(const Ptr<Ty>& type);
}

#endif // LSPSERVER_FINDOVERRIDEMETHODSUTILS_H
