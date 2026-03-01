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
 * This file provides some utils for CHIR type mangling.
 */

#ifndef CODIRA_MANGLE_CHIRMANGLINGUTILS_H
#define CODIRA_MANGLE_CHIRMANGLINGUTILS_H

#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIRMangling {
inline const std::string MANGLE_VIRTUAL_PREFIX = "_CV";
inline const std::string MANGLE_MUTABLE_PREFIX = "_CM";
inline const std::string MANGLE_FUNC_PREFIX = "_CC";
inline const std::string MANGLE_EXTEND_PREFIX = "$X";
inline const std::string MANGLE_INSTANTIATE_PREFIX = "_CI";
inline const std::string MANGLE_LAMBDA_PREFIX = "_CL";
inline const std::string MANGLE_OPERATOR_PREFIX = "_CO";
inline const std::string MANGLE_ANNOTATION_LAMBDA_PREFIX = "_CA";
inline const std::string MANGLE_CLOSURE_GENERIC_PREFIX = "$Cg";
inline const std::string MANGLE_CLOSURE_INSTANTIATE_PREFIX = "$Ci";
inline const std::string MANGLE_CLOSURE_FUNC_PREFIX = "$Cf";
inline const std::string MANGLE_CLOSURE_LAMBDA_PREFIX = "$Cl";
inline const std::string MANGLE_CLOSURE_WRAPPER_PREFIX = "$Cw";
inline const std::string MANGLE_ABSTRACT_INST_PREFIX = "$i";
inline const std::string MANGLE_ABSTRACT_GENERIC_PREFIX = "$vg";
inline const std::string MANGLE_ABSTRACT_INSTANTIATED_PREFIX = "$vi";
inline const std::string MANGLE_GENERIC_PREFIX = "$g";

/**
 * @brief Generate mangled name for virtual function.
 *
 * @param rawFunc The wrapper function.
 * @param customTypeDef The parent deference Type.
 * @param parentTy The parent class type.
 * @param isVirtual Indicates whether it is virtual.
 * @return std::string The mangled virtual function signature.
 */
std::string GenerateVirtualFuncMangleName(
    const Codira::CHIR::FuncBase* rawFunc, const Codira::CHIR::CustomTypeDef& customTypeDef,
    const Codira::CHIR::ClassType* parentTy, bool isVirtual);
/**
 * @brief Generate mangled name for instantiate function.
 *
 * @param baseName The origin identifier.
 * @param instTysInFunc The type args.
 * @return std::string The mangled instantiate function signature.
 */
std::string GenerateInstantiateFuncMangleName(const std::string& baseName,
    const std::vector<Codira::CHIR::Type*>& instTysInFunc);
/**
 * @brief Generate mangled name for lambda function.
 *
 * @param baseFunc The parent function.
 * @param counter The lambda wrapper index.
 * @return std::string The mangled lambda signature.
 */
std::string GenerateLambdaFuncMangleName(const Codira::CHIR::Func& baseFunc, size_t counter);
/**
 * @brief Overflow strategy to mangled name.
 *
 * @param ovf The overflow strategy.
 * @return std::string The mangled string for overflow strategy.
 */
std::string OverflowStrategyToString(OverflowStrategy ovf);

/**
 * @brief Generate mangled name for overflow operator function.
 *
 * @param name The operator function name.
 * @param ovf The overflow strategy.
 * @param isBinary Indicates whether it is binary.
 * @return std::string The mangled overflow operator function signature.
 */
std::string GenerateOverflowOperatorFuncMangleName(const std::string& name, OverflowStrategy ovf, bool isBinary,
    const Codira::CHIR::BuiltinType& type);
/**
 * @brief Generate mangled name for annotation function.
 *
 * @param name The origin annotation function signature.
 * @return std::string The mangled annotation function signature.
 */
std::string GenerateAnnotationFuncMangleName(const std::string& name);

namespace ClosureConversion {
/**
 * @brief Generate mangled name for generic base class.
 *
 * @param paramNum The function param type size.
 * @return std::string The mangled signature.
 */
std::string GenerateGenericBaseClassMangleName(size_t paramNum);
/**
 * @brief Generate mangled name for instantiated base class.
 *
 * @param funcType The closure conversion function type.
 * @return std::string The mangled signature.
 */
std::string GenerateInstantiatedBaseClassMangleName(const Codira::CHIR::FuncType& funcType);
/**
 * @brief Generate mangled name for global implement class.
 *
 * @param func The function in closure conversion.
 * @return std::string The mangled signature.
 */
std::string GenerateGlobalImplClassMangleName(const Codira::CHIR::FuncBase& func);
/**
 * @brief Generate mangled name for lambda implement class.
 *
 * @param func The lambda in closure conversion.
 * @param count The duplicate lambda count.
 * @return std::string The mangled signature.
 */
std::string GenerateLambdaImplClassMangleName(const Codira::CHIR::Lambda& func, size_t count);
/**
 * @brief Generate mangled name for wrapper class.
 *
 * @param def The instantiate auto environment base type.
 * @return std::string The mangled signature.
 */
std::string GenerateWrapperClassMangleName(const Codira::CHIR::ClassDef &def);
/**
 * @brief Generate mangled name for generic abstract function.
 *
 * @param def The auto environment base deference.
 * @return std::string The mangled signature.
 */
std::string GenerateGenericAbstractFuncMangleName(const Codira::CHIR::ClassDef &def);
/**
 * @brief Generate mangled name for instantiated abstract function.
 *
 * @param def The auto environment base deference.
 * @return std::string The mangled signature.
 */
std::string GenerateInstantiatedAbstractFuncMangleName(const Codira::CHIR::ClassDef &def);
/**
 * @brief Generate mangled name for generic override function.
 *
 * @param func The source function.
 * @return std::string The mangled signature.
 */
std::string GenerateGenericOverrideFuncMangleName(const Codira::CHIR::FuncBase &func);
/**
 * @brief Generate mangled name for instantiate override function.
 *
 * @param func The source function.
 * @return std::string The mangled signature.
 */
std::string GenerateInstOverrideFuncMangleName(const Codira::CHIR::FuncBase &func);
/**
 * @brief Generate mangled name for wrapper class generic override function.
 *
 * @param def The auto environment wrapper deference.
 * @return std::string The mangled signature.
 */
std::string GenerateWrapperClassGenericOverrideFuncMangleName(const Codira::CHIR::ClassDef &def);
/**
 * @brief Generate mangled name for wrapper class instantiate override function.
 *
 * @param def The auto environment wrapper deference.
 * @return std::string The mangled signature.
 */
std::string GenerateWrapperClassInstOverrideFuncMangleName(const Codira::CHIR::ClassDef &def);
} // namespace Codira::CHIRMangling::ClosureConversion
} // namespace Codira::CHIRMangling
#endif // CODIRA_MANGLE_CHIRMANGLINGUTILS_H
