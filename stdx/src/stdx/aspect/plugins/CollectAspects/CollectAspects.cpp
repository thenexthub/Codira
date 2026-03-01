/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/Mangle/CHIRTypeManglingUtils.h"
#include "Codira/MetaTransformation/MetaTransform.h"

#include <fstream>

using namespace Codira;

// Used to collect @InsertAtEntry, @InsertAtExit, @ReplaceFuncBody information.
struct CollectAspects : public MetaTransform<CHIR::Func> {
    friend class MyError;
    CollectAspects(CHIR::CHIRBuilder& builder) : builder(builder), packageName(builder.GetCurPackage()->GetName())
    {
    }
    void Run(CHIR::Func& func);

private:
    bool GlobalVarsArePrimitiveLiterals();
    void CheckAndCollect(CHIR::Func& func);

    struct To {
        std::string packageName;
        std::string className;
        std::string methodName;
        std::string funcTypeStr;
        std::string isStatic;
        std::string recursively;
    };
    struct Insert {
        std::string methodMangledName;
        size_t paramsNum;
        bool isMemberFunc;
        bool isStatic;
    };
    struct OutputInfo {
        std::string annoClassName;
        Insert insert;
        To to;
    };

    CHIR::CHIRBuilder& builder;
    std::vector<OutputInfo> outputs;
    std::string packageInitMethodName;
    std::string packageName;
    bool isGlobalVarsChecked = false;
    bool hasError = false;
    bool hasCreateAnnoInfo = false;
};

bool CollectAspects::GlobalVarsArePrimitiveLiterals()
{
    const CHIR::Package* package = builder.GetCurPackage();
    for (auto gv : package->GetGlobalVars()) {
        if (gv->TestAttr(Codira::CHIR::Attribute::IMPORTED) || gv->TestAttr(Codira::CHIR::Attribute::COMPILER_ADD)) {
            continue;
        }
        auto type = gv->GetType()->StripAllRefs();
        if (!type->IsPrimitive()) {
            return false;
        }
        if (!gv->GetInitializer() || !gv->GetInitializer()->IsLiteral()) {
            return false;
        }
    }
    isGlobalVarsChecked = true;
    return true;
}

class MyError {
public:
    MyError(CollectAspects& plugin) : plugin(plugin)
    {
    }
    template <typename... Args> void Emit(const char* fmt, Args&&... args)
    {
        ++errorCnt;
        Errorf(fmt, args...);
    }
    ~MyError()
    {
        plugin.hasError = errorCnt > 0U;
    }

private:
    size_t errorCnt = 0U;
    CollectAspects& plugin;
};

void CollectAspects::CheckAndCollect(CHIR::Func& func)
{
    auto annoInfo = func.GetAnnoInfo();
    if (annoInfo.annoPairs.empty()) {
        return;
    }

    MyError myError(*this);

    const std::set<std::string> whiteList1 = {"InsertAtEntry", "InsertAtExit"};
    const std::set<std::string> whiteList2 = {"ReplaceFuncBody"};

    bool inWhiteList1 = false;
    bool inWhiteList2 = false;
    for (auto& annoPair : annoInfo.annoPairs) {
        bool isFirstKind = whiteList1.find(annoPair.annoClassName) != whiteList1.end();
        bool isSecondKind = whiteList2.find(annoPair.annoClassName) != whiteList2.end();

        inWhiteList1 |= isFirstKind;
        inWhiteList2 |= isSecondKind;
        if (inWhiteList1 && inWhiteList2) {
            myError.Emit("%s: Annotations @InsertAtEntry/@InsertAtExit and @ReplaceFuncBody are mutually exclusive and "
                         "must not be applied to the same function.\n",
                func.GetSrcCodeIdentifier().c_str());
            return;
        }
        if (isFirstKind || isSecondKind) {
            if (!isGlobalVarsChecked && !GlobalVarsArePrimitiveLiterals()) {
                myError.Emit("In `%s` package where AOP annotations are used, all global variables must be initialized "
                             "with literals of primitive types only.\n",
                    builder.GetCurPackage()->GetName().c_str());
                return;
            }
            /// ----- Validation -----
            if (!func.TestAttr(Codira::CHIR::Attribute::PUBLIC) ||
                func.Get<Codira::CHIR::LinkTypeInfo>() != Linkage::EXTERNAL) {
                myError.Emit(
                    "%s: Function with AOP annotations must be public.\n", func.GetSrcCodeIdentifier().c_str());
                return;
            }
            if (func.TestAttr(Codira::CHIR::Attribute::FOREIGN)) {
                myError.Emit(
                    "%s: Function with AOP annotations must not be foreign.\n", func.GetSrcCodeIdentifier().c_str());
                return;
            }
            auto outerDef = func.GetOuterDeclaredOrExtendedDef();
            if (!func.GetGenericTypeParams().empty() || (outerDef && !outerDef->GetGenericTypeParams().empty())) {
                myError.Emit(
                    "%s: Function with AOP annotations must not be generic.\n", func.GetSrcCodeIdentifier().c_str());
                return;
            }
            /// ----------------------

            auto packageName = annoPair.paramValues[0U];
            auto className = annoPair.paramValues[1U];
            auto methodName = annoPair.paramValues[2U];
            auto isStatic = annoPair.paramValues[3U];
            auto isRecursive = annoPair.paramValues[isSecondKind ? 4U : 5U];
            bool isInsMethod = outerDef && !func.TestAttr(Codira::CHIR::Attribute::STATIC);
            /// ----- Validation -----
            if (outerDef && !func.TestAttr(Codira::CHIR::Attribute::STATIC)) {
                if (className.empty() || isStatic == "true") {
                    myError.Emit("%s: Instance member function must not be woven into global function or static member "
                                 "function.\n",
                        func.GetSrcCodeIdentifier().c_str());
                    return;
                }
                if (CHIR::GetTypeQualifiedName(*outerDef->GetType()) != (packageName + ":" + className)) {
                    myError.Emit(
                        "%s: Don't wave an instance method into another instance method with different `this` type.\n",
                        func.GetSrcCodeIdentifier().c_str());
                    return;
                }
            } else if (func.GetFuncType()->GetParamTypes().size() > (isFirstKind ? 0U : 1U) && !className.empty() &&
                isStatic == "false") {
                if (CHIR::GetTypeQualifiedName(*func.GetFuncType()->GetParamType(0U)) !=
                    (packageName + ":" + className)) {
                    myError.Emit("%s: Don't wave a method into another instance method with different `this` type.\n",
                        func.GetSrcCodeIdentifier().c_str());
                    return;
                }
            }
            std::string wovenFuncTypeStr = "";
            std::string aspectFuncTypeStr = "";
            auto paramTys = func.GetFuncType()->GetParamTypes();
            if (isFirstKind && func.GetFuncType()->GetParamTypes().size() > (isInsMethod ? 1U : 0U)) {
                wovenFuncTypeStr = annoPair.paramValues[4U];
                if (isInsMethod || (!className.empty() && isStatic == "false")) {
                    paramTys = std::vector<CHIR::Type*>(paramTys.begin() + 1, paramTys.end());
                }
                aspectFuncTypeStr =
                    CHIR::GetTypeQualifiedName(*builder.GetType<CHIR::FuncType>(paramTys, builder.GetUnitTy()));
                aspectFuncTypeStr = aspectFuncTypeStr.substr(0, aspectFuncTypeStr.size() - 6U); // 6 means "->Unit"
            } else if (isSecondKind && func.GetFuncType()->GetParamTypes().size() > (isInsMethod ? 2U : 1U)) {
                wovenFuncTypeStr = CHIR::GetTypeQualifiedName(*func.GetFuncType()->GetParamTypes().back());
                paramTys.pop_back();
                aspectFuncTypeStr = CHIR::GetTypeQualifiedName(
                    *builder.GetType<CHIR::FuncType>(paramTys, func.GetFuncType()->GetReturnType()));
            }
            std::replace(aspectFuncTypeStr.begin(), aspectFuncTypeStr.end(), ':', '.');
            std::replace(wovenFuncTypeStr.begin(), wovenFuncTypeStr.end(), ':', '.');
            if (wovenFuncTypeStr.find(aspectFuncTypeStr) != 0) {
                myError.Emit("The aspect function %s has different FuncType with the waved.\n",
                    func.GetSrcCodeIdentifier().c_str());
                return;
            }
            /// ----------------------

            if (className.empty() && isRecursive != "false") {
                myError.Emit("%s: The `recursive` field must be false if `className` is an empty string.\n",
                    func.GetSrcCodeIdentifier().c_str());
                return;
            }
            Insert insert = {func.GetIdentifierWithoutPrefix(),
                func.GetNumOfParams() - (annoPair.annoClassName == "ReplaceFuncBody" ? 1U : 0U),
                func.GetParentCustomTypeDef() != nullptr, func.TestAttr(Codira::CHIR::Attribute::STATIC)};
            if (isSecondKind) {
                /// ----- Validation -----
                if (func.GetNumOfParams() <= 0) {
                    myError.Emit("%s: Function annotated with @ReplaceFuncBody must have at least one parameter.\n",
                        func.GetSrcCodeIdentifier().c_str());
                    return;
                }
                auto lastParameterType = func.GetParams().back()->GetType();
                if (!lastParameterType->IsFunc()) {
                    myError.Emit("%s: Last parameter of the function annotated with @ReplaceFuncBody must be of "
                                 "function type.\n",
                        func.GetSrcCodeIdentifier().c_str());
                    return;
                }
                auto originalFuncType = static_cast<CHIR::FuncType*>(lastParameterType);
                if (func.GetReturnType() != originalFuncType->GetReturnType()) {
                    myError.Emit("%s: Return type of the function annotated with @ReplaceFuncBody must be the same as "
                                 "the return type of the function it is woven into.\n",
                        func.GetSrcCodeIdentifier().c_str());
                    return;
                }
                /// ----------------------

                // is a non-static method:
                if (!className.empty() && isStatic == "false") {
                    auto funcType = static_cast<CHIR::FuncType*>(func.GetParam(func.GetNumOfParams() - 1U)->GetType());
                    auto paramTys = funcType->GetParamTypes();
                    // check the validation of `this` parameter
                    if (paramTys.empty() ||
                        CHIR::GetTypeQualifiedName(*paramTys[0]) != (packageName + ":" + className)) {
                        myError.Emit("%s: For any function annotated with @ReplaceFuncBody, if it is intended to be "
                                     "woven into an instance member function, and its last parameter is of function "
                                     "type, the first parameter of this function type must be the same as the type "
                                     "indicated by packageName and className.\n",
                            func.GetSrcCodeIdentifier().c_str());
                        return;
                    }
                    // skip `this` parameter
                    paramTys = std::vector<CHIR::Type*>(paramTys.begin() + 1U, paramTys.end());
                    funcType = builder.GetType<CHIR::FuncType>(paramTys, funcType->GetReturnType());
                    auto sig = CHIR::GetTypeQualifiedName(*funcType);
                    std::replace(sig.begin(), sig.end(), ':', '.');
                    To to = {packageName, className, methodName, sig, isStatic, isRecursive};
                    outputs.emplace_back(OutputInfo{annoPair.annoClassName, insert, to});
                } else {
                    auto sig = CHIR::GetTypeQualifiedName(*func.GetParam(func.GetNumOfParams() - 1)->GetType());
                    std::replace(sig.begin(), sig.end(), ':', '.');
                    To to = {packageName, className, methodName, sig, isStatic, isRecursive};
                    outputs.emplace_back(OutputInfo{annoPair.annoClassName, insert, to});
                }
            } else {
                /// ----- Validation -----
                if (!func.GetReturnType()->IsUnit()) {
                    myError.Emit("%s: Return type of function annotated with @%s must be Unit.\n",
                        func.GetSrcCodeIdentifier().c_str(), annoPair.annoClassName.c_str());
                    return;
                }
                /// ----------------------
                auto sig = annoPair.paramValues[4U];
                To to = {packageName, className, methodName, sig, isStatic, isRecursive};
                outputs.emplace_back(OutputInfo{annoPair.annoClassName, insert, to});
            }

            if (!hasCreateAnnoInfo) {
                std::ofstream outFile(packageName + ".annoinfo");
                if (!outFile) {
                    myError.Emit("Failed to create file.\n");
                    return;
                }
                outFile << builder.GetCurPackage()->GetPackageInitFunc()->GetIdentifierWithoutPrefix() << std::endl;
                hasCreateAnnoInfo = true;
                outFile.close();
            }
            std::ofstream outFile(packageName + ".annoinfo", std::ios_base::app);
            if (!outFile) {
                myError.Emit("Failed to access file.\n");
                return;
            }
            auto output = outputs.back();
            outFile << output.annoClassName << std::endl
                    << output.insert.methodMangledName << std::endl
                    << output.insert.paramsNum << std::endl
                    << output.insert.isMemberFunc << std::endl
                    << output.insert.isStatic << std::endl;
            outFile << output.to.packageName << std::endl
                    << output.to.className << std::endl
                    << output.to.methodName << std::endl
                    << output.to.funcTypeStr << std::endl
                    << output.to.isStatic << std::endl
                    << output.to.recursively << std::endl;
            outFile.close();
        }
    }
}

// 实现 plugin
void CollectAspects::Run(CHIR::Func& func)
{
    if (hasError) {
        return;
    }

    CheckAndCollect(func);

    if (hasError) {
        Errorf("plugin terminated by some unexpected usages.\n");
        throw NullPointerException();
    }
}

// register plugin
CHIR_PLUGIN(CollectAspects)
