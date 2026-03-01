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

#include "runtime/tooling/evaluation/expression_loader.h"

#include "libarkbase/utils/span.h"
#include "libarkfile/class_data_accessor.h"
#include "runtime/include/runtime.h"

namespace ark::tooling {

namespace {

bool ValidateEvaluationMethod(Method *method)
{
    if (method == nullptr) {
        LOG(WARNING, DEBUGGER) << "Evaluate failed to find method";
        return false;
    }
    // No arguments can be provided by interface.
    if (method->GetNumArgs() != 0) {
        LOG(WARNING, DEBUGGER) << "Evaluation cannot have any arguments, but got " << method->GetNumArgs();
        return false;
    }
    // Currently support only primitive types
    auto type = method->GetReturnType();
    if (!type.IsPrimitive()) {
        LOG(WARNING, DEBUGGER) << "Evaluation method can return only primitive values";
        return false;
    }

    return true;
}

/**
 * @brief Checks if class fully-qualified name meets evaluation class requirements.
 * @param demangledClassName tested class name, can be obtained via ClassDataAccessor::DemangledName.
 * @returns name of evaluation package, class and method on success, nullopt otherwise.
 */
std::optional<std::string> FindEvaluationMethodName(std::string_view demangledClassName)
{
    static constexpr std::string_view SUFFIX = "_eval";
    // CC-OFFNXT(G.NAM.03) project code style
    static constexpr char DELIMITER = '.';

    // Evaluation class fully-qualified name must consist of package and class
    // equal names: "<NAME>.<NAME>".
    auto iter = std::find(demangledClassName.begin(), demangledClassName.end(), DELIMITER);
    if (iter == demangledClassName.end()) {
        return {};
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (std::find(iter + 1, demangledClassName.end(), DELIMITER) != demangledClassName.end()) {
        return {};
    }
    // Check package and class names equality.
    size_t delimiterPos = iter - demangledClassName.begin();
    auto expectedMethodName = demangledClassName.substr(0, delimiterPos);
    if (expectedMethodName != demangledClassName.substr(delimiterPos + 1)) {
        return {};
    }
    // Check expected suffix.
    if (delimiterPos < SUFFIX.size() || expectedMethodName.substr(delimiterPos - SUFFIX.size()) != SUFFIX) {
        return {};
    }
    return std::string(expectedMethodName);
}

/**
 * @brief Finds evaluation class according to naming convention.
 * @param pf received panda file.
 * @returns pair of evaluation method name and class data accessor on success, nullopt otherwise.
 */
std::optional<std::pair<std::string, panda_file::ClassDataAccessor>> FindEvaluationMethodClass(
    const panda_file::File *pf)
{
    for (uint32_t id : pf->GetClasses()) {
        panda_file::File::EntityId entityId(id);
        if (pf->IsExternal(entityId)) {
            continue;
        }

        panda_file::ClassDataAccessor cda(*pf, entityId);
        std::string className = cda.DemangledName();
        auto expectedMethodName = FindEvaluationMethodName(className);
        if (expectedMethodName) {
            return std::make_pair(*expectedMethodName, cda);
        }
    }
    return {};
}

Method *LoadFileAndGetEntryMethod(ClassLinkerContext *ctx, std::unique_ptr<const panda_file::File> &&pf)
{
    // Extract information about evaluation entry method. By convention, the passed
    // bytecode must contain a class with a static method, which both have the same names as panda file has.
    auto optEvalClassInfo = FindEvaluationMethodClass(pf.get());
    if (!optEvalClassInfo) {
        LOG(WARNING, DEBUGGER) << "Evaluate failed to find entry class";
        return nullptr;
    }
    auto [methodName, cda] = *optEvalClassInfo;
    auto evalClassId = cda.GetClassId();
    const auto *descriptor = pf->GetStringData(evalClassId).data;

    // Check that the class is not loaded yet.
    auto *linker = Runtime::GetCurrent()->GetClassLinker();
    if (linker->GetClass(*pf, evalClassId, ctx, nullptr) != nullptr) {
        LOG(WARNING, DEBUGGER) << "Evaluation class is already loaded, which is not supported";
        return nullptr;
    }

    // Add panda file into context of the target thread.
    linker->AddPandaFile(std::move(pf), ctx);

    auto sourceLang = cda.GetSourceLang().value_or(panda_file::SourceLang::PANDA_ASSEMBLY);
    ClassLinkerExtension *extension = linker->GetExtension(sourceLang);
    ASSERT(extension != nullptr);

    // Linker must accept nullptr as error handler, otherwise managed exception will occur on failure.
    auto *klass = linker->GetClass(descriptor, true, extension->ResolveContext(ctx), nullptr);
    if (klass == nullptr) {
        LOG(WARNING, DEBUGGER) << "Evaluate failed to load class";
        return nullptr;
    }
    return klass->GetDirectMethod(utf::CStringAsMutf8(methodName.c_str()));
}

}  // namespace

Expected<Method *, Error> LoadExpressionBytecode(ClassLinkerContext *ctx, const std::string &bytecode)
{
    auto pf = ark::panda_file::OpenPandaFileFromMemory(bytecode.data(), bytecode.size());
    if (pf == nullptr) {
        return Unexpected(Error(Error::Type::INVALID_EXPRESSION, "failed to load panda file"));
    }

    auto *method = LoadFileAndGetEntryMethod(ctx, std::move(pf));
    if (!ValidateEvaluationMethod(method)) {
        return Unexpected(Error(Error::Type::INVALID_ENTRY_POINT, "invalid expression bytecode"));
    }
    return method;
}

}  // namespace ark::tooling
