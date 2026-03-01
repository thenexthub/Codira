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

/**
 * @file
 *
 * This file implements some util functions for CHIR mangling.
 */

#include "Codira/Mangle/CHIRManglingUtils.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Type/ClassDef.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Type/EnumDef.h"
#include "Codira/CHIR/Type/StructDef.h"
#include "Codira/Utils/CastingTemplate.h"
#include "Codira/Mangle/BaseMangler.h"
#include "Codira/Mangle/CHIRTypeManglingUtils.h"
#include <sstream>

using namespace Codira::CHIR;
using namespace Codira::MangleUtils;
namespace {
std::string ReplaceManglePrefixWith(const std::string& mangledName, const std::string newPrefix)
{
    if (mangledName.find(Codira::MANGLE_CODIRA_PREFIX) == 0) {
        // Replace _C (2 characters) with user specified prefix.
        return newPrefix + mangledName.substr(Codira::MANGLE_PREFIX_LEN);
    } else {
        CODEC_ASSERT(false && "Mangle name has no _C prefix to be replaced.");
        return mangledName;
    }
}
} // namespace
namespace Codira::CHIRMangling {
std::string GenerateVirtualFuncMangleName(
    const FuncBase* rawFunc, const CustomTypeDef& customTypeDef, const ClassType* parentTy, bool isVirtual)
{
    std::stringstream ss;
    // "_CV" represents function wrapper for virtual functions. "_CM" represents function wrapper for mutable functions.
    std::string prefix = isVirtual ? MANGLE_VIRTUAL_PREFIX : MANGLE_MUTABLE_PREFIX;
    std::string originalName = ReplaceManglePrefixWith(rawFunc->GetIdentifierWithoutPrefix(), prefix);
    ss << originalName;
    std::vector<std::string> customTyGenericTypeStack;
    for (auto genericType : customTypeDef.GetGenericTypeParams()) {
        customTyGenericTypeStack.emplace_back(genericType->GetSrcCodeIdentifier());
    }
    if (customTypeDef.IsExtend()) {
        ss << MANGLE_EXTEND_PREFIX << MangleType(*customTypeDef.GetType(), customTyGenericTypeStack);
        std::vector<std::string> implementTypes;
        for (auto& ty : customTypeDef.GetImplementedInterfaceTys()) {
            implementTypes.emplace_back(MangleType(*ty, customTyGenericTypeStack));
        }
        // Sort implement types, so the order of how implement types show up don't affect compatibility.
        std::sort(implementTypes.begin(), implementTypes.end());
        for (auto& implementType : implementTypes) {
            ss << implementType;
        }
        ss << customTypeDef.GetPackageName().size() << customTypeDef.GetPackageName();
    } else {
        ss << MANGLE_DOLLAR_PREFIX << ReplaceManglePrefixWith(customTypeDef.GetIdentifierWithoutPrefix(), "");
    }
    ss << MANGLE_DOLLAR_PREFIX << MangleType(*parentTy, customTyGenericTypeStack);
    return ss.str();
}

std::string GenerateInstantiateFuncMangleName(const std::string& baseName, const std::vector<Type*>& instTysInFunc)
{
    std::stringstream ss;
    // "_CI" represents function wrapper for Instantiate functions.
    ss << ReplaceManglePrefixWith(baseName, MANGLE_INSTANTIATE_PREFIX) << MANGLE_DOLLAR_PREFIX;
    if (instTysInFunc.empty()) {
        ss << MANGLE_VOID_TY_SUFFIX;
    } else {
        for (auto& paramTy : instTysInFunc) {
            ss << MangleType(*paramTy);
        }
    }
    return ss.str();
}

std::string GenerateLambdaFuncMangleName(const Func& baseFunc, size_t counter)
{
    std::stringstream ss;
    // "_CL" represents for compiler generated anonymous functions (lambdas).
    ss << ReplaceManglePrefixWith(baseFunc.GetIdentifierWithoutPrefix(), MANGLE_LAMBDA_PREFIX) <<
        MANGLE_DOLLAR_PREFIX << counter << MANGLE_WILDCARD_PREFIX;
    return ss.str();
}

std::string OverflowStrategyToString(OverflowStrategy ovf)
{
    switch (ovf) {
        case OverflowStrategy::WRAPPING:
            return "&";
        case OverflowStrategy::THROWING:
            return "~";
        default:
            return "%";
    }
}

std::string GenerateOverflowOperatorFuncMangleName(const std::string& name, OverflowStrategy ovf, bool isBinary,
    const BuiltinType& type)
{
    std::stringstream ss;
    // "_CO" represents for compiler generated operator split functions.
    ss << MANGLE_OPERATOR_PREFIX << OverflowStrategyToString(ovf);
    if (MangleUtils::OPERATOR_TYPE_MANGLE.count(name) == 0) {
        CODEC_ASSERT(false && "Unsupported name for overflow operator mangling.");
    }
    ss << MangleUtils::OPERATOR_TYPE_MANGLE.at(name) << MANGLE_FUNC_PARAM_TYPE_PREFIX << MangleType(type);
    if (isBinary) {
        ss << MangleType(type);
    }
    return ss.str();
}

std::string GenerateAnnotationFuncMangleName(const std::string& name)
{
    // replace "_CN" with "_CAF"
    return MANGLE_CODIRA_PREFIX + "AF" + name.substr(Codira::MANGLE_PREFIX_LEN + 1);
}

namespace ClosureConversion {
std::string GenerateGenericBaseClassMangleName(size_t paramNum)
{
    std::stringstream ss;
    // `$C` is a special prefix for closure conversion class declarations. `g` stands for generic.
    // `$Cg` is followed by a number with an underscore suffix.
    ss << MANGLE_CLOSURE_GENERIC_PREFIX << paramNum << MANGLE_WILDCARD_PREFIX;
    return ss.str();
}

std::string GenerateInstantiatedBaseClassMangleName(const FuncType& funcType)
{
    std::stringstream ss;
    // `$C` is a special prefix for closure conversion class declarations. `i` stands for instantiate.
    // `$Ci` is followed by a type mangle name.
    ss << MANGLE_CLOSURE_INSTANTIATE_PREFIX << MangleType(funcType);
    return ss.str();
}

std::string GenerateGlobalImplClassMangleName(const FuncBase& func)
{
    std::stringstream ss;
    ss << ReplaceManglePrefixWith(func.GetIdentifierWithoutPrefix(), MANGLE_CLOSURE_FUNC_PREFIX);
    return ss.str();
}

std::string GenerateLambdaImplClassMangleName(const Lambda& func, size_t count)
{
    std::stringstream ss;
    ss << ReplaceManglePrefixWith(func.GetIdentifier(), MANGLE_CLOSURE_LAMBDA_PREFIX) << MANGLE_DOLLAR_PREFIX <<
        count;
    return ss.str();
}

std::string GenerateWrapperClassMangleName(const ClassDef &def)
{
    std::stringstream ss;
    // `$C` is a special prefix for closure conversion class declarations. `w` stands for wrapper.
    // `$Cw` is followed by a class (declaration) mangle name.
    ss << MANGLE_CLOSURE_WRAPPER_PREFIX << def.GetIdentifierWithoutPrefix();
    return ss.str();
}

std::string GenerateGenericAbstractFuncMangleName(const ClassDef &def)
{
    return MANGLE_FUNC_PREFIX + def.GetIdentifier() + MANGLE_ABSTRACT_GENERIC_PREFIX;
}

std::string GenerateInstantiatedAbstractFuncMangleName(const ClassDef &def)
{
    return MANGLE_FUNC_PREFIX + def.GetIdentifier() + MANGLE_ABSTRACT_INSTANTIATED_PREFIX;
}

std::string GenerateGenericOverrideFuncMangleName(const FuncBase &func)
{
    return ReplaceManglePrefixWith(func.GetIdentifierWithoutPrefix(), MANGLE_FUNC_PREFIX) + MANGLE_GENERIC_PREFIX;
}

std::string GenerateInstOverrideFuncMangleName(const FuncBase &func)
{
    return ReplaceManglePrefixWith(func.GetIdentifierWithoutPrefix(), MANGLE_FUNC_PREFIX) +
        MANGLE_ABSTRACT_INST_PREFIX;
}

std::string GenerateWrapperClassGenericOverrideFuncMangleName(const ClassDef &def)
{
    return MANGLE_FUNC_PREFIX + def.GetIdentifierWithoutPrefix() + MANGLE_GENERIC_PREFIX;
}

std::string GenerateWrapperClassInstOverrideFuncMangleName(const ClassDef &def)
{
    return MANGLE_FUNC_PREFIX + def.GetIdentifierWithoutPrefix() + MANGLE_ABSTRACT_INST_PREFIX;
}
} // namespace ClosureConversion
} // namespace Codira::CHIR
