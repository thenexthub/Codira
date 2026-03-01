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

#include "abckit_static.h"

#include "helpers_static.h"
#include "name_util.h"
#include "string_util.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/adapter_static/inst_modifier.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "libabckit/src/logger.h"
#include "src/adapter_static/metadata_modify_static.h"
#include "static_core/abc2program/abc2program_driver.h"
#include "static_core/assembler/assembly-emitter.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <unordered_set>
#include <functional>
#include <regex>

// CC-OFFNXT(WordsTool.95 Google) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

namespace {
constexpr std::string_view ETS_ANNOTATION_CLASS = "ets.annotation.class";
constexpr std::string_view ETS_ANNOTATION_MODULE = "ets.annotation.Module";
constexpr std::string_view ETS_EXTENDS = "ets.extends";
constexpr std::string_view ETS_IMPLEMENTS = "ets.implements";
constexpr std::string_view ENUM_BASE = "std.core.BaseEnum";
constexpr std::string_view INTERFACE_GET_FUNCTION_PATTERN = ".*%%get-(.*)";
constexpr std::string_view INTERFACE_SET_FUNCTION_PATTERN = ".*%%set-(.*)";
constexpr std::string_view ASYNC_PREFIX = "%%async-";
constexpr std::string_view ARRAY_ENUM_SUFFIX = "[]";
constexpr std::string_view OBJECT_CLASS = "std.core.Object";
constexpr std::string_view OBJECT_LITERAL_NAME = "$ObjectLiteral";
constexpr std::string_view INTERFACE_FIELD_PREFIX = "%%property-";
constexpr std::string_view ASYNC_ORIGINAL_RETURN = "std.core.Promise;";
constexpr std::string_view PARTIAL_PREFIX = "%%partial-";
constexpr std::string_view LAZY_IMPORT = "%%lazyImport";
constexpr std::string_view DELIMITER = ".";
const std::unordered_set<std::string> GLOBAL_CLASS_NAMES = {"ETSGLOBAL", "_GLOBAL"};
constexpr auto BASE_ENUM_UNION =
    "{Ustd.core.Byte,std.core.Double,std.core.Float,std.core.Int,std.core.Long,std.core.Short,std.core.String}";
constexpr std::string_view UNION_PREFIX = "{U";
constexpr std::string_view UNION_PROP_PREFIX = "%%union_prop";
constexpr auto STD_CORE_MODULE = "std.core";
}  // namespace

namespace libabckit {
// Container of vectors and maps of some kinds of instances
// Including modules, classes, namespaces, interfaces, enums, functions, interfaceObjectLiterals and
// annotationInterfaces
struct Container {
    std::vector<std::unique_ptr<AbckitCoreClass>> classes;
    std::vector<std::unique_ptr<AbckitCoreInterface>> interfaces;
    std::vector<std::unique_ptr<AbckitCoreNamespace>> namespaces;
    std::vector<std::unique_ptr<AbckitCoreEnum>> enums;
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    std::vector<std::unique_ptr<AbckitCoreClass>> objectLiterals;
    std::vector<std::unique_ptr<AbckitCoreAnnotationInterface>> annotationInterfaces;
    std::vector<std::unique_ptr<AbckitCoreClass>> partials;
    std::unordered_map<std::string, AbckitCoreClass *> nameToPartials;
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> nameToModule;
    std::unordered_map<std::string, AbckitCoreNamespace *> nameToNamespace;
    std::unordered_map<std::string, AbckitCoreClass *> nameToClass;
    std::unordered_map<std::string, AbckitCoreClass *> nameToObjectLiteral;
    std::unordered_map<std::string, AbckitCoreInterface *> nameToInterface;
    std::unordered_map<std::string, AbckitCoreEnum *> nameToEnum;
    std::unordered_map<std::string, AbckitCoreAnnotationInterface *> nameToAnnotationInterface;
};

static void DestroyContainer(void *data)
{
    delete static_cast<Container *>(data);
}

static bool IsModuleName(const std::string &name)
{
    return GLOBAL_CLASS_NAMES.find(name) != GLOBAL_CLASS_NAMES.end();
}

static bool IsEnum(const pandasm::Record &record)
{
    return record.metadata->GetBase() == ENUM_BASE;
}

static bool IsPartial(const std::string &name)
{
    return name.rfind(PARTIAL_PREFIX) == 0;
}

static bool IsUnion(const std::string &name)
{
    if (name.size() < UNION_PREFIX.size()) {
        return false;
    }
    return name.rfind(UNION_PREFIX) == 0;
}
static bool IsLazyImport(const std::string &recordName)
{
    return recordName.find(LAZY_IMPORT) != std::string::npos;
}

static bool IsUnionProp(const std::string &className)
{
    if (className.size() < UNION_PROP_PREFIX.size()) {
        return false;
    }
    return className.rfind(UNION_PROP_PREFIX) == 0;
}

AbckitString *CreateNameString(AbckitFile *file, const std::string &name)
{
    return CreateStringStatic(file, name.data(), name.size());
}

pandasm::Function *FunctionGetImpl(AbckitCoreFunction *function)
{
    return function->GetArkTSImpl()->GetStaticImpl();
}

std::vector<std::string> GetAnnotationNames(
    const std::variant<pandasm::Function *, pandasm::Record *, pandasm::Field *> impl)
{
    return std::visit([](auto &&object) { return object->metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data()); },
                      impl);
}

static std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *, std::nullptr_t> GetTypeReference(
    AbckitFile *file, const std::string &typeName)
{
    const auto container = static_cast<Container *>(file->data);
    if (const auto it = container->nameToClass.find(typeName); it != container->nameToClass.end()) {
        return it->second;
    }

    if (const auto it = container->nameToInterface.find(typeName); it != container->nameToInterface.end()) {
        return it->second;
    }

    if (const auto it = container->nameToEnum.find(typeName); it != container->nameToEnum.end()) {
        return it->second;
    }

    if (const auto it = container->nameToPartials.find(typeName); it != container->nameToPartials.end()) {
        return it->second;
    }

    if (const auto it = container->nameToObjectLiteral.find(typeName); it != container->nameToObjectLiteral.end()) {
        return it->second;
    }

    LIBABCKIT_LOG(DEBUG) << "no reference found for type:" << typeName << std::endl;

    return nullptr;
}

static AbckitType *PandasmTypeToAbckitType(AbckitFile *file, const pandasm::Type &pandasmType)
{
    auto abckitName = CreateNameString(file, pandasmType.GetName());
    auto typeId = ArkPandasmTypeToAbckitTypeId(pandasmType);
    auto rank = pandasmType.GetRank();
    std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *, std::nullptr_t> reference = nullptr;
    if (pandasmType.IsObject()) {
        reference = GetTypeReference(file, pandasmType.GetName());
    }
    auto type = GetOrCreateType(file, typeId, rank, reference, abckitName);

    if (pandasmType.IsUnion()) {
        auto typeNames = pandasmType.GetComponentNames();
        for (const auto &typeName : typeNames) {
            auto abckitTypeName = CreateNameString(file, typeName);
            std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *, std::nullptr_t> unionReference =
                nullptr;
            if (pandasmType.IsObject()) {
                unionReference = GetTypeReference(file, typeName);
            }
            type->types.emplace_back(GetOrCreateType(file, typeId, rank, unionReference, abckitTypeName));
        }
    }
    return type;
}

static std::pair<std::string, std::string> ClassGetModuleNames(
    const std::string &fullName, const std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace)
{
    auto [moduleName, className] = ClassGetNames(fullName);
    if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
        moduleName = ClassGetModuleNames(moduleName, nameToNamespace).first;
    }
    return {moduleName, className};
}

static void DumpHierarchy(AbckitFile *file)
{
    std::function<void(AbckitCoreFunction * f, const std::string &indent)> dumpFunc;
    std::function<void(AbckitCoreClass * c, const std::string &indent)> dumpClass;

    dumpFunc = [&dumpFunc, &dumpClass](AbckitCoreFunction *f, const std::string &indent = "") {
        auto fName = FunctionGetImpl(f)->name;
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << fName << std::endl;
        for (auto &cNested : f->nestedClasses) {
            dumpClass(cNested.get(), indent + "  ");
        }
        for (auto &fNested : f->nestedFunction) {
            dumpFunc(fNested.get(), indent + "  ");
        }
    };

    dumpClass = [&dumpFunc](AbckitCoreClass *c, const std::string &indent = "") {
        auto cName = GetStaticImplRecord(c)->name;
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << cName << std::endl;
        for (auto &f : c->methods) {
            dumpFunc(f.get(), indent + "  ");
        }
    };

    std::function<void(AbckitCoreNamespace * n, const std::string &indent)> dumpNamespace =
        [&dumpFunc, &dumpClass, &dumpNamespace](AbckitCoreNamespace *n, const std::string &indent = "") {
            auto &nName = GetStaticImplRecord(n)->name;
            LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << nName << std::endl;
            for (auto &[_, n] : n->nt) {
                dumpNamespace(n.get(), indent + "  ");
            }
            for (auto &[_, c] : n->ct) {
                dumpClass(c.get(), indent + "  ");
            }
            for (auto &f : n->functions) {
                dumpFunc(f.get(), indent + "  ");
            }
        };

    for (auto &[mName, m] : file->localModules) {
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << mName << std::endl;
        for (auto &[_, n] : m->nt) {
            dumpNamespace(n.get(), "");
        }
        for (auto &[cName, c] : m->ct) {
            dumpClass(c.get(), "");
        }
        for (auto &f : m->functions) {
            dumpFunc(f.get(), "");
        }
    }
}

static std::unique_ptr<AbckitCoreAnnotation> CreateAnnotation(
    AbckitFile *file,
    std::variant<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreClassField *, AbckitCoreInterface *,
                 AbckitCoreInterfaceField *>
        owner,
    const std::string &name)
{
    LIBABCKIT_LOG(DEBUG) << "Found annotation :'" << name << "'\n";
    auto anno = std::make_unique<AbckitCoreAnnotation>();
    anno->name = CreateNameString(file, name);
    anno->owner = owner;
    anno->impl = std::make_unique<AbckitArktsAnnotation>();
    anno->GetArkTSImpl()->core = anno.get();
    return anno;
}

static std::unique_ptr<AbckitCoreFunctionParam> CreateFunctionParam(AbckitFile *file,
                                                                    const std::unique_ptr<AbckitCoreFunction> &function,
                                                                    const pandasm::Function::Parameter &parameter)
{
    auto param = std::make_unique<AbckitCoreFunctionParam>();
    param->function = function.get();
    param->type = PandasmTypeToAbckitType(file, parameter.type);
    AddFunctionUserToAbckitType(param->type, function.get());
    param->impl = std::make_unique<AbckitArktsFunctionParam>();
    param->GetArkTSImpl()->core = param.get();

    return param;
}

std::unique_ptr<AbckitCoreFunction> CollectFunction(AbckitFile *file, const std::string &functionName,
                                                    ark::pandasm::Function &functionImpl)
{
    const auto container = static_cast<Container *>(file->data);
    auto [moduleName, className] = FuncGetNames(functionName);
    auto function = std::make_unique<AbckitCoreFunction>();
    if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
        moduleName = ClassGetModuleNames(moduleName, container->nameToNamespace).first;
    }
    LIBABCKIT_LOG(DEBUG) << "  Found function. module: '" << moduleName << "' class: '" << className << "' function: '"
                         << functionName << "'\n";
    ASSERT(container->nameToModule.find(moduleName) != container->nameToModule.end());
    function->owningModule = container->nameToModule[moduleName].get();
    function->impl = std::make_unique<AbckitArktsFunction>();
    function->GetArkTSImpl()->impl = &functionImpl;
    function->GetArkTSImpl()->core = function.get();
    auto name = GetMangleFuncName(function.get());
    auto &nameToFunction = functionImpl.IsStatic() ? file->nameToFunctionStatic : file->nameToFunctionInstance;
    ASSERT(nameToFunction.count(name) == 0);
    nameToFunction.insert({name, function.get()});

    for (auto &annoImpl : functionImpl.metadata->GetAnnotations()) {
        LIBABCKIT_LOG(DEBUG) << "Found annotation. func: '" << name << "', anno: '" << annoImpl.GetName() << "'\n";
        auto anno = CreateAnnotation(file, function.get(), annoImpl.GetName());
        for (auto &annoElemImpl : annoImpl.GetElements()) {
            LIBABCKIT_LOG(DEBUG) << "Found annotation element. func: '" << name << "', anno: '" << annoImpl.GetName()
                                 << "', annoElem: '" << annoElemImpl.GetName() << "'\n";
            auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
            annoElem->ann = anno.get();
            annoElem->name = CreateStringStatic(file, annoElemImpl.GetName().data(), annoElemImpl.GetName().size());
            annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
            annoElem->GetArkTSImpl()->core = annoElem.get();
            auto value = FindOrCreateValueStatic(file, *annoElemImpl.GetValue());
            annoElem->value = value;

            anno->elements.emplace_back(std::move(annoElem));
        }
        function->annotations.emplace_back(std::move(anno));
        function->annotationTable.emplace(annoImpl.GetName(), function->annotations.back().get());
    }

    for (auto &functionParam : functionImpl.params) {
        function->parameters.emplace_back(CreateFunctionParam(file, function, functionParam));
    }

    function->returnType = PandasmTypeToAbckitType(function->owningModule->file, functionImpl.returnType);
    AddFunctionUserToAbckitType(function->returnType, function.get());

    return function;
}

static std::unique_ptr<AbckitCoreInterfaceField> CreateInterfaceField(
    const std::unique_ptr<AbckitCoreInterface> &interface, AbckitCoreFunction *function, const std::string &fieldName)
{
    auto field = std::make_unique<AbckitCoreInterfaceField>();
    auto file = interface->owningModule->file;
    field->name = CreateNameString(file, fieldName);
    field->impl = std::make_unique<AbckitArktsInterfaceField>();
    field->GetArkTSImpl()->core = field.get();
    field->owner = interface.get();
    field->flag = (ACC_PUBLIC | ACC_READONLY);

    auto returnType = function->GetArkTSImpl()->GetStaticImpl()->returnType;
    // Interface field is accessed by getter and setter, so we don't need to bind interface field type
    field->type = PandasmTypeToAbckitType(file, returnType);

    for (const auto &anno : function->annotations) {
        auto annotation = CreateAnnotation(file, field.get(), anno->name->impl.data());
        annotation->ai = anno->ai;
        annotation->ai->annotations.emplace_back(annotation.get());
        field->annotations.emplace_back(std::move(annotation));
    }
    return field;
}

template <typename AbckitCoreType, typename AbckitCoreTypeField>
static std::unique_ptr<AbckitCoreTypeField> CreateField(AbckitFile *file, AbckitCoreType *owner,
                                                        pandasm::Field &recordField)
{
    auto field = std::make_unique<AbckitCoreTypeField>(owner, &recordField);
    if constexpr (std::is_same_v<AbckitCoreType, AbckitCoreAnnotationInterface>) {
        field->name = CreateNameString(file, recordField.name);
    }
    field->type = PandasmTypeToAbckitType(file, recordField.type);
    auto optionalValue = recordField.metadata->GetValue();
    field->value = optionalValue.has_value() ? FindOrCreateValueStatic(file, optionalValue.value()) : nullptr;
    AddFieldUserToAbckitType(field->type, field.get());
    if constexpr (std::is_same_v<AbckitCoreType, AbckitCoreClass>) {
        for (auto &annoImpl : recordField.metadata->GetAnnotations()) {
            LIBABCKIT_LOG(DEBUG) << "Found annotation. field: '" << recordField.name << "', anno: '"
                                 << annoImpl.GetName() << "'\n";
            auto anno = CreateAnnotation(file, field.get(), annoImpl.GetName());
            for (auto &annoElemImpl : annoImpl.GetElements()) {
                LIBABCKIT_LOG(DEBUG) << "Found annotation element. field: '" << recordField.name << "', anno: '"
                                     << annoImpl.GetName() << "', annoElem: '" << annoElemImpl.GetName() << "'\n";
                auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
                annoElem->ann = anno.get();
                annoElem->name = CreateStringStatic(file, annoElemImpl.GetName().data(), annoElemImpl.GetName().size());
                annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
                annoElem->GetArkTSImpl()->core = annoElem.get();
                auto value = FindOrCreateValueStatic(file, *annoElemImpl.GetValue());
                annoElem->value = value;

                anno->elements.emplace_back(std::move(annoElem));
            }
            field->annotationTable.emplace(annoImpl.GetName(), std::move(anno));
        }
    }
    return field;
}

std::unique_ptr<AbckitCoreEnumField> EnumCreateField(AbckitFile *file, AbckitCoreEnum *owner,
                                                     ark::pandasm::Field &recordField)
{
    return CreateField<AbckitCoreEnum, AbckitCoreEnumField>(file, owner, recordField);
}

std::unique_ptr<AbckitCoreClassField> ClassCreateField(AbckitFile *file, AbckitCoreClass *owner,
                                                       ark::pandasm::Field &recordField)
{
    return CreateField<AbckitCoreClass, AbckitCoreClassField>(file, owner, recordField);
}

std::unique_ptr<AbckitCoreModuleField> ModuleCreateField(AbckitFile *file, AbckitCoreModule *owner,
                                                         ark::pandasm::Field &recordField)
{
    return CreateField<AbckitCoreModule, AbckitCoreModuleField>(file, owner, recordField);
}

static void CreateModule(AbckitFile *file, const std::string &moduleName, pandasm::Record &record)
{
    auto *container = static_cast<Container *>(file->data);
    auto &nameToModule = container->nameToModule;
    if (nameToModule.find(moduleName) != nameToModule.end()) {
        LIBABCKIT_LOG(FATAL) << "Duplicated ETSGLOBAL for module: " << moduleName << '\n';
    }

    LIBABCKIT_LOG(DEBUG) << "Found module: '" << moduleName << "'\n";
    auto m = std::make_unique<AbckitCoreModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_ARK_TS_V2;
    m->moduleName = CreateNameString(file, moduleName);
    m->impl = std::make_unique<AbckitArktsModule>(&record, m.get());

    nameToModule.insert({moduleName, std::move(m)});
}

static void CreateExternalModule(AbckitFile *file, const std::string &moduleName, pandasm::Record &record)
{
    auto *container = static_cast<Container *>(file->data);
    auto &nameToModule = container->nameToModule;
    auto m = std::make_unique<AbckitCoreModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_ARK_TS_V2;
    m->moduleName = CreateNameString(file, moduleName);
    m->impl = std::make_unique<AbckitArktsModule>(&record, m.get());
    m->GetArkTSImpl()->core = m.get();
    m->isExternal = true;

    nameToModule.emplace(moduleName, std::move(m));
}

static std::unique_ptr<AbckitCoreNamespace> CreateNamespace(pandasm::Record &record, const std::string &namespaceName)
{
    LIBABCKIT_LOG(DEBUG) << "  Found namespace: '" << namespaceName << "'\n";
    return std::make_unique<AbckitCoreNamespace>(nullptr, AbckitArktsNamespace(&record));
}

template <typename AbckitCoreType, typename AbckitArktsType>
static std::unique_ptr<AbckitCoreType> CreateInstance(
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule, const std::string &moduleName,
    const std::string &instanceName, ark::pandasm::Record &record)
{
    if (IsUnion(moduleName)) {
        LIBABCKIT_LOG(DEBUG) << "  Found union instance" << moduleName + "." + instanceName << "'\n";
        const auto &module = nameToModule[STD_CORE_MODULE];
        return std::make_unique<AbckitCoreType>(module.get(), AbckitArktsType(&record));
    }
    LIBABCKIT_LOG(DEBUG) << "  Found instance. module: '" << moduleName << "' instance: '" << instanceName << "'\n";
    ASSERT(nameToModule.find(moduleName) != nameToModule.end());
    const auto &module = nameToModule[moduleName];
    return std::make_unique<AbckitCoreType>(module.get(), AbckitArktsType(&record));
}

static std::string FindOriginalFunctionName(const std::string &asyncFunctionName,
                                            const std::unordered_map<std::string, AbckitCoreFunction *> &nameToFunction)
{
    // format: prefix.%%async-funcName:params;returnType;
    auto pos1 = asyncFunctionName.find(ASYNC_PREFIX);
    auto pos2 = asyncFunctionName.rfind(OBJECT_CLASS);
    auto size = ASYNC_PREFIX.size();

    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return asyncFunctionName;
    }

    std::string baseNameWithParams =
        asyncFunctionName.substr(0, pos1) + asyncFunctionName.substr(pos1 + size, pos2 - pos1 - size);

    std::vector<std::string> possibleReturnTypes = {"void;", "std.core.Promise;"};

    for (const auto &returnType : possibleReturnTypes) {
        std::string candidateName = baseNameWithParams + returnType;
        if (nameToFunction.find(candidateName) != nameToFunction.end()) {
            return candidateName;
        }
    }
    return baseNameWithParams + std::string {ASYNC_ORIGINAL_RETURN};
}

static void AssignAsyncFunction(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);
    for (auto item = container->functions.begin(); item != container->functions.end();) {
        auto name = GetMangleFuncName(item->get());
        if (name.size() <= ASYNC_PREFIX.size() || name.find(ASYNC_PREFIX) == std::string::npos) {
            ++item;
            continue;
        }

        auto &nameToFunction = item->get()->GetArkTSImpl()->GetStaticImpl()->IsStatic() ? file->nameToFunctionStatic
                                                                                        : file->nameToFunctionInstance;
        auto originalName = FindOriginalFunctionName(name, nameToFunction);
        LIBABCKIT_LOG(DEBUG) << "Async Function Name: " << name << std::endl;
        LIBABCKIT_LOG(DEBUG) << "Async Function Original Name: " << originalName << std::endl;
        ASSERT(nameToFunction.find(originalName) != nameToFunction.end());
        nameToFunction.at(originalName)->asyncImpl = std::move(*item);
        item = container->functions.erase(item);
    }
}

static void AssignArrayClass(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);
    for (auto item = container->classes.begin(); item != container->classes.end();) {
        auto name = GetStaticImplRecord(item->get())->name;
        if (!StringUtil::IsEndWith(name, ARRAY_ENUM_SUFFIX.data())) {
            ++item;
            continue;
        }

        auto originClassName = name.substr(0, name.size() - ARRAY_ENUM_SUFFIX.length());
        auto classImpl = file->nameToClass.find(originClassName);
        if (classImpl == file->nameToClass.end()) {
            ++item;
            continue;
        }
        LIBABCKIT_LOG(DEBUG) << "Found array class:" << name << "\n";
        std::visit([&](auto &&object) { object->array = std::move(*item); }, classImpl->second);
        item = container->classes.erase(item);
    }
}

static void AssignOwningModuleOfNamespace(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);

    for (const auto &ns : container->namespaces) {
        auto fullName = GetStaticImplRecord(ns.get())->name;
        auto moduleName = ClassGetModuleNames(fullName, container->nameToNamespace).first;
        ns->owningModule = container->nameToModule[moduleName].get();
    }
}

static void AssignAnnotationInterfaceToAnnotation(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);

    auto assign = [container](const std::string &annotationName, AbckitCoreAnnotation *annotation) {
        if (container->nameToAnnotationInterface.find(annotationName) != container->nameToAnnotationInterface.end()) {
            auto &ai = container->nameToAnnotationInterface.at(annotationName);
            annotation->ai = ai;
            ai->annotations.emplace_back(annotation);
        }
    };

    for (const auto &klass : container->classes) {
        for (auto &[annotationName, annotation] : klass->annotationTable) {
            assign(annotationName, annotation);
        }
        for (const auto &field : klass->fields) {
            for (const auto &[annotationName, annotation] : field->annotationTable) {
                assign(annotationName, annotation.get());
            }
        }
    }

    for (const auto &[_, partial] : container->nameToPartials) {
        for (const auto &field : partial->fields) {
            for (const auto &[annotationName, annotation] : field->annotationTable) {
                assign(annotationName, annotation.get());
            }
        }
    }

    for (const auto &iface : container->interfaces) {
        for (const auto &[annotationName, annotation] : iface->annotationTable) {
            assign(annotationName, annotation.get());
        }
    }

    for (const auto &function : container->functions) {
        for (auto &[annotationName, annotation] : function->annotationTable) {
            assign(annotationName, annotation);
        }
    }
}

static void AssignSuperClass(std::unordered_map<std::string, AbckitCoreClass *> &nameToClass,
                             std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface,
                             const std::vector<std::unique_ptr<AbckitCoreClass>> &classes)
{
    for (const auto &klass : classes) {
        auto record = GetStaticImplRecord(klass.get());
        std::string fullName = record->name;
        std::string parentClassName = record->metadata->GetBase();
        if (nameToClass.find(parentClassName) != nameToClass.end()) {
            LIBABCKIT_LOG(DEBUG) << "  Found Super Class. Class: '" << fullName << "' Super Class: '" << parentClassName
                                 << "'\n";
            auto &parentClass = nameToClass[parentClassName];
            klass->superClass = parentClass;
            parentClass->subClasses.emplace_back(klass.get());
        }

        for (const auto &interfaceName : record->metadata->GetInterfaces()) {
            if (nameToInterface.find(interfaceName) != nameToInterface.end()) {
                LIBABCKIT_LOG(DEBUG) << "  Found Implemented Interface. Class: '" << fullName
                                     << "' Implemented Interface: '" << interfaceName << "'\n";
                auto &implementedInterface = nameToInterface[interfaceName];
                klass->interfaces.emplace_back(implementedInterface);
                implementedInterface->classes.emplace_back(klass.get());
            }
        }
    }
}

static void AssignSuperInterface(std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface,
                                 const std::vector<std::unique_ptr<AbckitCoreInterface>> &interfaces)
{
    for (const auto &interface : interfaces) {
        auto record = GetStaticImplRecord(interface.get());
        std::string fullName = record->name;
        for (const auto &superInterfaceName : record->metadata->GetInterfaces()) {
            if (nameToInterface.find(superInterfaceName) != nameToInterface.end()) {
                LIBABCKIT_LOG(DEBUG) << "  Found Super Interface. Interface: '" << fullName << "' Super Interface: '"
                                     << superInterfaceName << "'\n";
                auto &superInterface = nameToInterface[superInterfaceName];
                interface->superInterfaces.emplace_back(superInterface);
                superInterface->subInterfaces.emplace_back(interface.get());
            }
        }
    }
}

static void AssignObjectLiteral(AbckitFile *file, std::vector<std::unique_ptr<AbckitCoreClass>> &objectLiterals,
                                std::unordered_map<std::string, AbckitCoreClass *> &nameToClass,
                                std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface,
                                std::unordered_map<std::string, AbckitCoreClass *> &nameToPartial)
{
    for (auto &objectLiteral : objectLiterals) {
        const auto record = GetStaticImplRecord(objectLiteral.get());
        LIBABCKIT_LOG(DEBUG) << "Assign Object Literal: " << record->name << '\n';
        std::string objectLiteralName = record->name;
        auto implements = record->metadata->GetAttributeValues(ETS_IMPLEMENTS.data());
        if (!implements.empty()) {
            const auto &interfaceName = implements.front();
            if (IsPartial(std::get<1>(ClassGetNames(interfaceName)))) {
                nameToPartial[interfaceName]->objectLiterals.emplace_back(std::move(objectLiteral));
            } else {
                auto it = nameToInterface.find(interfaceName);
                if (it != nameToInterface.end()) {
                    it->second->objectLiterals.emplace_back(std::move(objectLiteral));
                } else {
                    nameToClass[interfaceName]->objectLiterals.emplace_back(std::move(objectLiteral));
                }
            }
            // Remove objectLiteral from file->nameToClass after it's moved
            file->nameToClass.erase(objectLiteralName);
            continue;
        }
        for (const auto &className : record->metadata->GetAttributeValues(ETS_EXTENDS.data())) {
            if (IsPartial(std::get<1>(ClassGetNames(className)))) {
                nameToPartial[className]->objectLiterals.emplace_back(std::move(objectLiteral));
            } else {
                if (className == OBJECT_CLASS) {
                    continue;
                }
                ASSERT(nameToClass.find(className) != nameToClass.end());
                nameToClass[className]->objectLiterals.emplace_back(std::move(objectLiteral));
            }
        }
        // Remove objectLiteral from file->nameToClass after it's moved
        file->nameToClass.erase(objectLiteralName);
    }
}

static void AssignPartial(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);
    auto &nameToClass = file->nameToClass;
    auto &partials = container->partials;

    for (auto &partial : partials) {
        std::string name = GetStaticImplRecord(partial.get())->name;
        LIBABCKIT_LOG(DEBUG) << "Assign Partial: " << name << '\n';
        auto [moduleName, className] = ClassGetNames(name);
        std::string originalName = moduleName + DELIMITER.data() + className.substr(PARTIAL_PREFIX.size());
        auto it = nameToClass.find(originalName);
        if (it != nameToClass.end()) {
            if (const auto klass = std::get_if<AbckitCoreClass *>(&it->second)) {
                if ((*klass)->superClass != nullptr && (*klass)->superClass->partial != nullptr) {
                    LIBABCKIT_LOG(DEBUG) << "Assign super Partial: "
                                         << GetStaticImplRecord((*klass)->superClass->partial.get())->name << '\n';
                    (*klass)->superClass->partial->subClasses.emplace_back(partial.get());
                }
            }
            std::visit(
                [&partial](auto &&object) {
                    partial->parentNamespace = object->parentNamespace;
                    object->partial = std::move(partial);
                },
                it->second);
            // Remove partial from nameToClass after it's moved to the target object
            nameToClass.erase(name);
        }
    }
}

template <typename AbckitCoreType>
static void AssignInstance(const std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                           const std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                           std::vector<std::unique_ptr<AbckitCoreType>> &instances)
{
    for (auto &instance : instances) {
        std::string fullName = GetStaticImplRecord(instance.get())->name;
        auto [moduleName, instanceName] = ClassGetNames(fullName);
        if (IsUnion(fullName)) {
            LIBABCKIT_LOG(DEBUG) << "assign union class: " << fullName << '\n';
            nameToModule.at(STD_CORE_MODULE)->InsertInstance(instanceName, std::move(instance));
            continue;
        }
        if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
            LIBABCKIT_LOG(DEBUG) << "namespaceName, className, fullName: " << moduleName << ", " << instanceName << ", "
                                 << fullName << '\n';
            instance->parentNamespace = nameToNamespace.at(moduleName);
            if constexpr (std::is_same_v<AbckitCoreType, AbckitCoreEnum>) {
                instance->array->parentNamespace = instance->parentNamespace;
            }
            nameToNamespace.at(moduleName)->InsertInstance(instanceName, std::move(instance));
            continue;
        }
        LIBABCKIT_LOG(DEBUG) << "moduleName, className, fullName: " << moduleName << ", " << instanceName << ", "
                             << fullName << '\n';
        auto &instanceModule = nameToModule.at(moduleName);
        instanceModule->InsertInstance(instanceName, std::move(instance));
    }
}

static bool IsInterfaceGetFunction(const std::string &functionName, std::smatch &match)
{
    return std::regex_match(functionName, match, std::regex(INTERFACE_GET_FUNCTION_PATTERN.data()));
}

static bool IsInterfaceSetFunction(const std::string &functionName, std::smatch &match)
{
    return std::regex_match(functionName, match, std::regex(INTERFACE_SET_FUNCTION_PATTERN.data()));
}

static void CreateAndAssignInterfaceField(const std::unique_ptr<AbckitCoreInterface> &interface,
                                          AbckitCoreFunction *function, const std::string &functionName)
{
    std::smatch match;
    if (IsInterfaceGetFunction(functionName, match)) {
        const auto &fieldName = INTERFACE_FIELD_PREFIX.data() + match[1].str();
        LIBABCKIT_LOG(DEBUG) << "Found interface Field: " << fieldName << "\n";
        interface->fields.emplace(fieldName, CreateInterfaceField(interface, function, fieldName));
    } else if (IsInterfaceSetFunction(functionName, match)) {
        const auto &fieldName = INTERFACE_FIELD_PREFIX.data() + match[1].str();
        // Handle case where only setter exists without getter
        if (interface->fields.find(fieldName) == interface->fields.end()) {
            LIBABCKIT_LOG(DEBUG) << "Found interface Field (setter only): " << fieldName << "\n";
            interface->fields.emplace(fieldName, CreateInterfaceField(interface, function, fieldName));
        }
        interface->fields[fieldName]->flag ^= ACC_READONLY;
    }
}

template <typename AbckitCoreType>
using HandlerFunc = std::function<void(AbckitCoreType *, std::unique_ptr<AbckitCoreFunction> &&, const std::string &,
                                       const std::string &)>;

template <typename AbckitCoreType>
struct HandlerInfo {
    // handler to the function if condition is true
    HandlerFunc<AbckitCoreType> handler;
    // Tell if function belong to some vector
    std::function<bool(AbckitCoreType *, const std::string &)> condition;
};

static void AssignAsyncImplParent(const AbckitCoreFunction *function)
{
    if (function->asyncImpl) {
        function->asyncImpl->parent = function->parent;
    }
}

template <typename AbckitCoreType>
static void HandleGlobalFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                 const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Module Function. module: '" << className << "' function: '" << functionName
                         << "'\n";
    function->parent = owningInstance;
    AssignAsyncImplParent(function.get());
    owningInstance->functions.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleClassFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Class Function. class: '" << className << "' function: '" << functionName << "'\n";
    auto &klass = owningInstance->ct[className];
    function->parent = klass.get();
    AssignAsyncImplParent(function.get());
    klass->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleNamespaceFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &namespaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Namespace Function. namespace: '" << namespaceName << "' function: '"
                         << functionName << "'\n";
    auto &ns = owningInstance->nt[namespaceName];
    function->parent = ns.get();
    AssignAsyncImplParent(function.get());
    ns->functions.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleInterfaceFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &interfaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Interface Function. interface: '" << interfaceName << "' function: '"
                         << functionName << "'\n";
    auto &interface = owningInstance->it[interfaceName];
    CreateAndAssignInterfaceField(interface, function.get(), functionName);
    function->parent = interface.get();
    AssignAsyncImplParent(function.get());
    interface->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleEnumFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                               const std::string &enumName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Enum Function. enum: '" << enumName << "' function: '" << functionName << "'\n";
    auto &enm = owningInstance->et[enumName];
    function->parent = enm.get();
    AssignAsyncImplParent(function.get());
    enm->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void ProcessFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                            const std::string &className, const std::string &functionName)
{
    std::vector<HandlerInfo<AbckitCoreType>> handlers = {
        {HandleGlobalFunction<AbckitCoreType>,
         [](AbckitCoreType *, const std::string &className) { return IsModuleName(className); }},
        {HandleNamespaceFunction<AbckitCoreType>,
         [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->nt.find(className) != owningInstance->nt.end();
         }},
        {HandleClassFunction<AbckitCoreType>,
         [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->ct.find(className) != owningInstance->ct.end();
         }},
        {HandleInterfaceFunction<AbckitCoreType>,
         [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->it.find(className) != owningInstance->it.end();
         }},
        {HandleEnumFunction<AbckitCoreType>, [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->et.find(className) != owningInstance->et.end();
         }}};

    for (const auto &handlerInfo : handlers) {
        if (handlerInfo.condition(owningInstance, className)) {
            handlerInfo.handler(owningInstance, std::move(function), className, functionName);
            return;
        }
    }
}

static void AssignFunctions(AbckitFile *file,
                            std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                            std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                            std::unordered_map<std::string, AbckitCoreClass *> &nameToObjectLiteral,
                            std::vector<std::unique_ptr<AbckitCoreFunction>> &functions)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);
    for (auto &function : functions) {
        std::string functionName = FunctionGetImpl(function.get())->name;
        auto [moduleName, className] = FuncGetNames(functionName);
        auto recordName = moduleName;
        recordName.append(".").append(className);
        if (nameToObjectLiteral.find(recordName) != nameToObjectLiteral.end()) {
            auto &objectLiteral = nameToObjectLiteral.at(recordName);
            function->parent = objectLiteral;
            objectLiteral->methods.emplace_back(std::move(function));
            continue;
        }
        if (IsPartial(className)) {
            auto partial = container->nameToPartials[recordName];
            function->parent = partial;
            partial->methods.emplace_back(std::move(function));
            continue;
        }
        if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
            ProcessFunction(nameToNamespace[moduleName], std::move(function), className, functionName);
            continue;
        }
        auto &functionModule = nameToModule[moduleName];
        ProcessFunction(functionModule.get(), std::move(function), className, functionName);
        ASSERT(function == nullptr);
    }
}

static void MoveUnion(Container *container)
{
    LIBABCKIT_LOG_FUNC;

    const auto &stdModule = container->nameToModule.at(STD_CORE_MODULE);
    for (auto &[_, module] : container->nameToModule) {
        if (module->isExternal) {
            continue;
        }

        auto it = module->ct.begin();
        while (it != module->ct.end()) {
            const auto &[className, klass] = *it;
            if (IsUnionProp(className)) {
                LIBABCKIT_LOG(DEBUG) << "Union prop: " << className << '\n';
                // 将union_prop类移动到std.core module中
                stdModule->ct[className] = std::move(it->second);
                it = module->ct.erase(it);
            } else {
                ++it;
            }
        }
    }
}

static bool IsNamespace(const std::string &className, pandasm::Record &record)
{
    if (IsModuleName(className)) {
        return false;
    }
    const auto &attributes = GetAnnotationNames(&record);
    return std::any_of(attributes.begin(), attributes.end(),
                       [=](const std::string &name) { return name == ETS_ANNOTATION_MODULE; });
}

static bool IsObjectLiteral(const std::string &name)
{
    return name.find(OBJECT_LITERAL_NAME) != std::string::npos;
}

static bool IsInterface(const pandasm::Record &record)
{
    return (record.metadata->GetAccessFlags() & ACC_INTERFACE) != 0;
}

static bool IsAnnotationInterface(const pandasm::Record &record)
{
    return (record.metadata->GetAccessFlags() & ACC_ANNOTATION) != 0;
}

static const std::string LAMBDA_RECORD_KEY = "%%lambda-";
[[maybe_unused]] static const std::string INIT_FUNC_NAME = "_$init$_";
static const std::string TRIGGER_CCTOR_FUNC_NAME = "_$trigger_cctor$_";

static bool ShouldCreateFuncWrapper(const pandasm::Function &functionImpl, const std::string &functionName)
{
    return (!functionImpl.metadata->IsForeign() &&
            (functionName.find(TRIGGER_CCTOR_FUNC_NAME, 0) == std::string::npos) && !IsLazyImport(functionName));
}

static void CollectAllFunctions(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);

    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    for (auto &[functionName, functionImpl] : prog->functionStaticTable) {
        LIBABCKIT_LOG(DEBUG) << "function function key:" << functionName << std::endl;
        LIBABCKIT_LOG(DEBUG) << "function function value:" << functionImpl.name << std::endl;

        if (ShouldCreateFuncWrapper(functionImpl, functionName)) {
            container->functions.emplace_back(CollectFunction(file, functionName, functionImpl));
        }
    }
    for (auto &[functionName, functionImpl] : prog->functionInstanceTable) {
        LIBABCKIT_LOG(DEBUG) << "function function at instance table:" << functionName << std::endl;

        if (ShouldCreateFuncWrapper(functionImpl, functionName)) {
            container->functions.emplace_back(CollectFunction(file, functionName, functionImpl));
        }
    }
}

static void CollectExternalModules(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    const auto container = static_cast<Container *>(file->data);

    for (auto &[recordName, record] : prog->recordTable) {
        if (!record.metadata->IsForeign()) {
            continue;
        }
        if (IsLazyImport(recordName)) {
            LIBABCKIT_LOG(DEBUG) << "Skipping lazyImport record: " << recordName << '\n';
            continue;
        }
        std::vector<std::string> moduleNames;
        pandasm::Type type = pandasm::Type::FromName(recordName);
        if (type.IsUnion()) {
            LIBABCKIT_LOG(DEBUG) << "Record:" << recordName << " is a union\n";
            auto recordNames = type.GetComponentNames();
            for (auto &recordName : recordNames) {
                auto [moduleName, _] = ClassGetNames(recordName);
                moduleNames.push_back(moduleName);
            }
        } else {
            auto [moduleName, _] = ClassGetNames(recordName);
            moduleNames.push_back(moduleName);
        }

        for (auto &moduleName : moduleNames) {
            if (container->nameToModule.find(moduleName) != container->nameToModule.end()) {
                continue;
            }

            if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
                continue;
            }

            if (prog->recordTable.find(moduleName) != prog->recordTable.end()) {
                LIBABCKIT_LOG(DEBUG) << "Found External Namespace: " << moduleName << '\n';
                container->namespaces.emplace_back(CreateNamespace(prog->recordTable.at(moduleName), moduleName));
                container->nameToNamespace.emplace(moduleName, container->namespaces.back().get());
                continue;
            }

            LIBABCKIT_LOG(DEBUG) << "Found External Module: " << moduleName << '\n';
            CreateExternalModule(file, moduleName, record);
        }
    }
}

static void CollectExternalFunctions(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);

    for (auto &[functionName, functionImpl] : prog->functionStaticTable) {
        if (!functionImpl.metadata->IsForeign() || IsLazyImport(functionName)) {
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Found External Static Function: " << functionName << std::endl;

        auto [moduleName, className] = FuncGetNames(functionName);
        LIBABCKIT_LOG(DEBUG) << "moduleName: " << moduleName << std::endl;
        LIBABCKIT_LOG(DEBUG) << "className: " << className << std::endl;
        auto function = std::make_unique<AbckitCoreFunction>();

        AbckitCoreModule *resolvedModule = nullptr;
        if (container->nameToModule.find(moduleName) != container->nameToModule.end()) {
            resolvedModule = container->nameToModule.at(moduleName).get();
        } else if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
            auto rootModuleName = ClassGetModuleNames(moduleName, container->nameToNamespace).first;
            if (container->nameToModule.find(rootModuleName) != container->nameToModule.end()) {
                resolvedModule = container->nameToModule.at(rootModuleName).get();
            }
        }

        function->owningModule = resolvedModule;

        function->impl = std::make_unique<AbckitArktsFunction>();
        function->GetArkTSImpl()->impl = &functionImpl;
        function->GetArkTSImpl()->core = function.get();

        function->returnType = PandasmTypeToAbckitType(file, functionImpl.returnType);
        AddFunctionUserToAbckitType(function->returnType, function.get());

        for (auto &functionParam : functionImpl.params) {
            function->parameters.emplace_back(CreateFunctionParam(file, function, functionParam));
        }

        for (const auto &anno : GetAnnotationNames(&functionImpl)) {
            function->annotations.emplace_back(CreateAnnotation(file, function.get(), anno));
            function->annotationTable.emplace(anno, function->annotations.back().get());
        }

        auto name = GetMangleFuncName(function.get());
        file->nameToExternalFunction.insert({name, function.get()});
        file->externalFunctions.emplace_back(std::move(function));
    }

    for (auto &[functionName, functionImpl] : prog->functionInstanceTable) {
        if (!functionImpl.metadata->IsForeign() || IsLazyImport(functionName)) {
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Found External Instance Function: " << functionName << std::endl;

        auto [moduleName, className] = FuncGetNames(functionName);
        LIBABCKIT_LOG(DEBUG) << "moduleName: " << moduleName << std::endl;
        LIBABCKIT_LOG(DEBUG) << "className: " << className << std::endl;
        auto function = std::make_unique<AbckitCoreFunction>();

        AbckitCoreModule *resolvedModule = nullptr;
        if (container->nameToModule.find(moduleName) != container->nameToModule.end()) {
            resolvedModule = container->nameToModule.at(moduleName).get();
        } else if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
            auto rootModuleName = ClassGetModuleNames(moduleName, container->nameToNamespace).first;
            if (container->nameToModule.find(rootModuleName) != container->nameToModule.end()) {
                resolvedModule = container->nameToModule.at(rootModuleName).get();
            }
        }

        function->owningModule = resolvedModule;

        function->impl = std::make_unique<AbckitArktsFunction>();
        function->GetArkTSImpl()->impl = &functionImpl;
        function->GetArkTSImpl()->core = function.get();

        function->returnType = PandasmTypeToAbckitType(file, functionImpl.returnType);
        AddFunctionUserToAbckitType(function->returnType, function.get());

        for (auto &functionParam : functionImpl.params) {
            function->parameters.emplace_back(CreateFunctionParam(file, function, functionParam));
        }

        for (const auto &anno : GetAnnotationNames(&functionImpl)) {
            function->annotations.emplace_back(CreateAnnotation(file, function.get(), anno));
            function->annotationTable.emplace(anno, function->annotations.back().get());
        }

        auto name = GetMangleFuncName(function.get());
        file->nameToExternalFunction.insert({name, function.get()});
        file->externalFunctions.emplace_back(std::move(function));
    }
}

static std::set<std::string> CollectAnnotationNames(pandasm::Program *prog)
{
    std::set<std::string> annotationNames;
    for (auto &[recordName, record] : prog->recordTable) {
        if (IsLazyImport(recordName)) {
            LIBABCKIT_LOG(DEBUG) << "Skipping lazyImport record: " << recordName << '\n';
            continue;
        }
        for (auto &annotationName : record.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
            annotationNames.insert(annotationName);
        }
        for (auto &recordField : record.fieldList) {
            for (auto &annotationName : recordField.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
                annotationNames.insert(annotationName);
            }
        }
    }
    for (auto &[functionName, function] : prog->functionStaticTable) {
        if (IsLazyImport(functionName)) {
            LIBABCKIT_LOG(DEBUG) << "Skipping lazyImport function: " << functionName << '\n';
            continue;
        }
        for (auto &annotationName : function.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
            annotationNames.insert(annotationName);
        }
        for (auto &functionParam : function.params) {
            for (auto &annotationName :
                 functionParam.GetOrCreateMetadata()->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
                annotationNames.insert(annotationName);
            }
        }
    }
    for (auto &[functionName, function] : prog->functionInstanceTable) {
        if (IsLazyImport(functionName)) {
            LIBABCKIT_LOG(DEBUG) << "Skipping lazyImport function: " << functionName << '\n';
            continue;
        }
        for (auto &annotationName : function.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
            annotationNames.insert(annotationName);
        }
        for (auto &functionParam : function.params) {
            for (auto &annotationName :
                 functionParam.GetOrCreateMetadata()->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
                annotationNames.insert(annotationName);
            }
        }
    }
    return annotationNames;
}

// NOTE: external records don't have accessflags in abc
static void UpdateAnnotationRecords(pandasm::Program *prog)
{
    auto annotationNames = CollectAnnotationNames(prog);
    for (auto &[recordName, record] : prog->recordTable) {
        if (!record.metadata->IsForeign() || IsLazyImport(recordName)) {
            continue;
        }
        if (annotationNames.find(recordName) != annotationNames.end()) {
            pandasm::Record recordCopy(recordName, panda_file::SourceLang::ETS);
            recordCopy.metadata->SetAttribute("external");
            recordCopy.metadata->SetAccessFlags(record.metadata->GetAccessFlags() | ACC_ANNOTATION);
            prog->recordTable.insert_or_assign(recordName, std::move(recordCopy));
        }
    }
}

static void CollectAnnotations(AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);
    for (const auto &klass : container->classes) {
        for (auto &annoImpl : GetStaticImplRecord(klass.get())->metadata->GetAnnotations()) {
            LIBABCKIT_LOG(DEBUG) << "Found annotation. class: '" << GetStaticImplRecord(klass.get())->name
                                 << "', anno: '" << annoImpl.GetName() << "'\n";
            auto anno = CreateAnnotation(file, klass.get(), annoImpl.GetName());
            for (auto &annoElemImpl : annoImpl.GetElements()) {
                LIBABCKIT_LOG(DEBUG) << "Found annotation element. class: '" << GetStaticImplRecord(klass.get())->name
                                     << "', anno: '" << annoImpl.GetName() << "', annoElem: '" << annoElemImpl.GetName()
                                     << "'\n";
                auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
                annoElem->ann = anno.get();
                annoElem->name = CreateStringStatic(file, annoElemImpl.GetName().data(), annoElemImpl.GetName().size());
                annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
                annoElem->GetArkTSImpl()->core = annoElem.get();
                auto value = FindOrCreateValueStatic(file, *annoElemImpl.GetValue());
                annoElem->value = value;

                anno->elements.emplace_back(std::move(annoElem));
            }
            klass->annotations.emplace_back(std::move(anno));
            klass->annotationTable.emplace(annoImpl.GetName(), klass->annotations.back().get());
        }
    }
    for (const auto &iface : container->interfaces) {
        for (auto &annoImpl : GetStaticImplRecord(iface.get())->metadata->GetAnnotations()) {
            LIBABCKIT_LOG(DEBUG) << "Found annotation. interface: '" << GetStaticImplRecord(iface.get())->name
                                 << "', anno: '" << annoImpl.GetName() << "'\n";
            auto anno = CreateAnnotation(file, iface.get(), annoImpl.GetName());
            for (auto &annoElemImpl : annoImpl.GetElements()) {
                LIBABCKIT_LOG(DEBUG) << "Found annotation element. interface: '"
                                     << GetStaticImplRecord(iface.get())->name << "', anno: '" << annoImpl.GetName()
                                     << "', annoElem: '" << annoElemImpl.GetName() << "'\n";
                auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
                annoElem->ann = anno.get();
                annoElem->name = CreateStringStatic(file, annoElemImpl.GetName().data(), annoElemImpl.GetName().size());
                annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
                annoElem->GetArkTSImpl()->core = annoElem.get();
                auto value = FindOrCreateValueStatic(file, *annoElemImpl.GetValue());
                annoElem->value = value;

                anno->elements.emplace_back(std::move(annoElem));
            }
            iface->annotationTable.emplace(annoImpl.GetName(), std::move(anno));
        }
    }
}

template <typename AbckitCoreType, typename AbckitCoreTypeField>
static void AssignField(AbckitFile *file, AbckitCoreType *coreInstance)
{
    auto record = GetStaticImplRecord(coreInstance);
    for (auto &recordField : record->fieldList) {
        LIBABCKIT_LOG(DEBUG) << "Found Field: " << recordField.name << "\n";
        auto field = CreateField<AbckitCoreType, AbckitCoreTypeField>(file, coreInstance, recordField);
        file->nameToField.emplace(NameUtil::GetFieldFullName(record, &recordField), field.get());
        coreInstance->fields.emplace_back(std::move(field));
    }
}

static void CollectAllInstanceFields(AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    const auto container = static_cast<Container *>(file->data);

    for (const auto &[_, module] : container->nameToModule) {
        AssignField<AbckitCoreModule, AbckitCoreModuleField>(file, module.get());
    }

    for (const auto &[_, ns] : container->nameToNamespace) {
        AssignField<AbckitCoreNamespace, AbckitCoreNamespaceField>(file, ns);
    }

    for (const auto &[_, enm] : container->nameToEnum) {
        AssignField<AbckitCoreEnum, AbckitCoreEnumField>(file, enm);
    }

    for (const auto &[_, ai] : container->nameToAnnotationInterface) {
        AssignField<AbckitCoreAnnotationInterface, AbckitCoreAnnotationInterfaceField>(file, ai);
    }

    for (const auto &[_, objectLiteral] : container->nameToObjectLiteral) {
        AssignField<AbckitCoreClass, AbckitCoreClassField>(file, objectLiteral);
    }

    for (const auto &[_, partial] : container->nameToPartials) {
        AssignField<AbckitCoreClass, AbckitCoreClassField>(file, partial);
    }

    for (const auto &[_, klass] : container->nameToClass) {
        AssignField<AbckitCoreClass, AbckitCoreClassField>(file, klass);
    }
}

static void CollectPartial(AbckitFile *file, const std::string &moduleName, const std::string &className,
                           const std::string &recordName, ark::pandasm::Record &record)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);
    auto partial =
        CreateInstance<AbckitCoreClass, AbckitArktsClass>(container->nameToModule, moduleName, className, record);
    container->nameToPartials.emplace(recordName, partial.get());
    file->nameToClass.emplace(recordName, partial.get());
    container->partials.emplace_back(std::move(partial));
}

static void CollectObjectLiteral(AbckitFile *file, const std::string &moduleName, const std::string &className,
                                 const std::string &recordName, ark::pandasm::Record &record)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);
    auto objectLiteral =
        CreateInstance<AbckitCoreClass, AbckitArktsClass>(container->nameToModule, moduleName, className, record);
    container->nameToObjectLiteral.emplace(recordName, objectLiteral.get());
    file->nameToClass.emplace(recordName, objectLiteral.get());
    container->objectLiterals.emplace_back(std::move(objectLiteral));
}

static void CollectAllInstances(AbckitFile *file, pandasm::Program *prog)
{
    LIBABCKIT_LOG_FUNC;

    const auto container = static_cast<Container *>(file->data);

    for (auto &[recordName, record] : prog->recordTable) {
        if (IsLazyImport(recordName)) {
            LIBABCKIT_LOG(DEBUG) << "Skipping lazyImport record: " << recordName << '\n';
            continue;
        }
        auto [moduleName, className] = ClassGetModuleNames(recordName, container->nameToNamespace);
        if (IsModuleName(className) ||
            container->nameToNamespace.find(recordName) != container->nameToNamespace.end()) {
            // NOTE: find and fill AbckitCoreImportDescriptor
            continue;
        }

        if (recordName == BASE_ENUM_UNION) {
            LIBABCKIT_LOG(DEBUG) << "Record:" << recordName << " is a union\n";
            continue;
        }

        auto &nameToModule = container->nameToModule;
        // Collect partial
        if (IsPartial(className)) {
            CollectPartial(file, moduleName, className, recordName, record);
            continue;
        }

        // Collect Interface
        if (IsInterface(record)) {
            auto iface =
                CreateInstance<AbckitCoreInterface, AbckitArktsInterface>(nameToModule, moduleName, className, record);
            container->nameToInterface[recordName] = iface.get();
            file->nameToClass.emplace(recordName, iface.get());
            container->interfaces.emplace_back(std::move(iface));
            continue;
        }

        // Collect Enum
        if (IsEnum(record)) {
            auto enm = CreateInstance<AbckitCoreEnum, AbckitArktsEnum>(nameToModule, moduleName, className, record);
            container->nameToEnum.emplace(recordName, enm.get());
            file->nameToClass.emplace(recordName, enm.get());
            container->enums.emplace_back(std::move(enm));
            continue;
        }

        // Collect AnnotationInterface
        if (IsAnnotationInterface(record)) {
            auto ai = CreateInstance<AbckitCoreAnnotationInterface, AbckitArktsAnnotationInterface>(
                nameToModule, moduleName, className, record);
            container->nameToAnnotationInterface.emplace(recordName, ai.get());
            container->annotationInterfaces.emplace_back(std::move(ai));
            continue;
        }

        // Collect Local ObjectLiteral
        if (IsObjectLiteral(recordName) && !record.metadata->IsForeign()) {
            CollectObjectLiteral(file, moduleName, className, recordName, record);
            continue;
        }

        // Collect Class
        auto klass = CreateInstance<AbckitCoreClass, AbckitArktsClass>(nameToModule, moduleName, className, record);
        container->nameToClass.emplace(recordName, klass.get());
        file->nameToClass.emplace(recordName, klass.get());
        container->classes.emplace_back(std::move(klass));
    }
    CollectAnnotations(file);
    CollectAllInstanceFields(file);
}

static void CollectAllStrings(pandasm::Program *prog, AbckitFile *file)
{
    for (auto &sImpl : prog->strings) {
        auto s = std::make_unique<AbckitString>();
        s->impl = sImpl;
        file->strings.insert({sImpl, std::move(s)});
    }
}

static void CreateWrappers(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    file->program = prog;
    auto container = std::make_unique<Container>();
    Container *containerPtr = container.get();
    file->data = container.release();
    file->destoryData = &DestroyContainer;
    // Collect modules and namespaces
    for (auto &[recordName, record] : prog->recordTable) {
        if (record.metadata->IsForeign()) {
            // NOTE: Create AbckitCoreImportDescriptor and AbckitCoreModuleDescriptor
            continue;
        }
        if (IsLazyImport(recordName)) {
            LIBABCKIT_LOG(DEBUG) << "Skipping lazyImport record: " << recordName << '\n';
            continue;
        }
        auto [moduleName, className] = ClassGetNames(recordName);
        if (IsModuleName(className)) {
            CreateModule(file, moduleName, record);
            continue;
        }

        if (IsNamespace(className, record)) {
            containerPtr->namespaces.emplace_back(CreateNamespace(record, className));
            containerPtr->nameToNamespace.emplace(recordName, containerPtr->namespaces.back().get());
        }
    }
    CollectExternalModules(prog, file);
    CollectExternalFunctions(prog, file);
    AssignOwningModuleOfNamespace(file);

    // Collect classes, interfaces, enums, interfaceObjectLiteral and annotationInterface
    CollectAllInstances(file, prog);

    // Functions
    CollectAllFunctions(prog, file);

    UpdateAnnotationRecords(prog);

    // Assign
    AssignAsyncFunction(file);
    AssignArrayClass(file);
    AssignAnnotationInterfaceToAnnotation(file);
    AssignSuperClass(containerPtr->nameToClass, containerPtr->nameToInterface, containerPtr->classes);
    AssignSuperInterface(containerPtr->nameToInterface, containerPtr->interfaces);
    AssignObjectLiteral(file, containerPtr->objectLiterals, containerPtr->nameToClass, containerPtr->nameToInterface,
                        containerPtr->nameToPartials);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->classes);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->interfaces);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->annotationInterfaces);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->enums);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->namespaces);
    AssignPartial(file);
    AssignFunctions(file, containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->nameToObjectLiteral,
                    containerPtr->functions);
    MoveUnion(containerPtr);

    // NOTE: AbckitCoreExportDescriptor
    // NOTE: AbckitModulePayload

    for (auto &[moduleName, module] : containerPtr->nameToModule) {
        if (module->isExternal) {
            file->externalModules.emplace(moduleName, std::move(module));
        } else {
            file->localModules.emplace(moduleName, std::move(module));
        }
    }

    DumpHierarchy(file);

    // Strings
    CollectAllStrings(prog, file);
}

struct CtxIInternal {
    ark::abc2program::Abc2ProgramDriver *driver = nullptr;
};

AbckitFile *OpenAbcStatic(const char *path, size_t len)
{
    LIBABCKIT_LOG_FUNC;
    auto spath = std::string(path, len);
    LIBABCKIT_LOG(DEBUG) << spath << '\n';
    auto *abc2program = new ark::abc2program::Abc2ProgramDriver();
    if (!abc2program->Compile(spath)) {
        LIBABCKIT_LOG(DEBUG) << "Failed to open " << spath << "\n";
        delete abc2program;
        return nullptr;
    }
    pandasm::Program &prog = abc2program->GetProgram();
    auto file = new AbckitFile();
    file->frontend = Mode::STATIC;
    CreateWrappers(&prog, file);

    auto pf = panda_file::File::Open(spath);
    if (pf == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "Failed to panda_file::File::Open\n";
        delete abc2program;
        delete file;
        return nullptr;
    }

    auto pandaFileVersion = pf->GetHeader()->version;
    uint8_t *fileVersion = pandaFileVersion.data();

    file->version = new uint8_t[ABCKIT_VERSION_SIZE];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(fileVersion, fileVersion + sizeof(uint8_t) * ABCKIT_VERSION_SIZE, file->version);

    file->internal = new CtxIInternal {abc2program};
    return file;
}

void CloseFileStatic(AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    for (auto &[_, type] : file->types) {
        if (type != nullptr) {
            type->functionUsers.clear();
            type->fieldTypeUsers.clear();
            type->types.clear();
        }
    }
    for (auto &[_, type] : file->types) {
        if (type != nullptr) {
            delete type;
        }
    }
    file->types.clear();

    if (file->needOptimize) {
        std::unordered_map<AbckitCoreFunction *, FunctionStatus *> &functionsMap = file->functionsMap;
        for (auto &[_, status] : functionsMap) {
            DestroyGraphStaticSync(status->graph);
            delete status;
        }
        functionsMap.clear();

        if (file->generateStatus.pf != nullptr) {
            delete static_cast<panda_file::File *>(file->generateStatus.pf);
        }
        if (file->generateStatus.maps != nullptr) {
            delete static_cast<pandasm::AsmEmitter::PandaFileToPandaAsmMaps *>(file->generateStatus.maps);
        }
    }

    if ((file->destoryData != nullptr) && (file->data != nullptr)) {
        file->destoryData(file->data);
    }
    file->data = nullptr;
    auto *ctxIInternal = reinterpret_cast<struct CtxIInternal *>(file->internal);
    delete ctxIInternal->driver;
    delete ctxIInternal;
    delete[] file->version;
    delete file;
}

void WriteAbcStatic(AbckitFile *file, const char *path, size_t len)
{
    if (file->needOptimize) {
        std::unordered_map<AbckitCoreFunction *, FunctionStatus *> &functionsMap = file->functionsMap;
        for (auto &[func, status] : functionsMap) {
            if (status->writeBack) {
                FunctionSetGraphStaticSync(func, status->graph);
            }
            delete status;
        }
        functionsMap.clear();
    }

    LIBABCKIT_LOG_FUNC;
    auto spath = std::string(path, len);
    LIBABCKIT_LOG(DEBUG) << spath << '\n';

    if (file->needRefreshInsts) {
        InstModifier modifier(file);
        modifier.Modify();
    }

    auto program = file->GetStaticProgram();

    auto emitDebugInfo = false;
    std::map<std::string, size_t> *statp = nullptr;
    ark::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = nullptr;

    if (!pandasm::AsmEmitter::Emit(spath, *program, statp, mapsp, emitDebugInfo)) {
        LIBABCKIT_LOG(ERROR) << "FAILURE: " << pandasm::AsmEmitter::GetLastError() << '\n';
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    LIBABCKIT_LOG(DEBUG) << "SUCCESS\n";
}

void DestroyGraphStatic(AbckitGraph *graph)
{
    if (!graph->file->needOptimize) {
        DestroyGraphStaticSync(graph);
    }
}

void DestroyGraphStaticSync(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    auto *ctxGInternal = reinterpret_cast<CtxGInternal *>(graph->internal);
    // dirty hack to obtain PandaFile pointer
    // NOTE(mshimenkov): refactor it
    auto *fileWrapper =
        reinterpret_cast<panda_file::File *>(ctxGInternal->runtimeAdapter->GetBinaryFileForMethod(nullptr));
    delete fileWrapper;
    delete ctxGInternal->runtimeAdapter;
    delete ctxGInternal->irInterface;
    delete ctxGInternal->localAllocator;
    delete ctxGInternal->allocator;
    delete ctxGInternal;
    delete graph;
}

}  // namespace libabckit
