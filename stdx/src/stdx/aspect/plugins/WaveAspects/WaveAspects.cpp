/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include "Codira/Basic/Print.h"
#include "Codira/CHIR/CHIRBuilder.h"
#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Transformation/BlockGroupCopyHelper.h"
#include "Codira/CHIR/Type/CustomTypeDef.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "Codira/Mangle/CHIRTypeManglingUtils.h"
#include "Codira/MetaTransformation/MetaTransform.h"

#include <dirent.h>
#include <fstream>
#include <unistd.h>

using namespace Codira;

struct To {
    std::string packageName;
    std::string outerTypeName;
    std::string methodName;
    std::string funcTypeStr;
    bool isStatic;
    bool recursively;
};
struct Insert {
    std::string methodMangledName;
    size_t paramsNum;
    bool isMemberFunc;
    bool isStatic;
};
struct ProcessInfo {
    std::string annoClassName;
    Insert insert;
    To to;
};

// Used to collect @InsertAtEntry, @InsertAtExit, @ReplaceFuncBody information.
struct WaveAspects : public MetaTransform<CHIR::Func> {
    friend class MyError;
    WaveAspects(CHIR::CHIRBuilder& builder) : builder(builder)
    {
    }
    void Run(CHIR::Func& func);
    CHIR::CHIRBuilder& builder;

private:
    void ReadInfo();
    void ReadInfo(const std::string& annoInfoPath);
    CHIR::FuncBase* GetCalleeByProcessInfo(
        const ProcessInfo& pi, const CHIR::FuncBase& func, const std::vector<CHIR::Value*>& args) const;
    std::vector<CHIR::Value*> PrepareArguments(const CHIR::Func& caller, const ProcessInfo& pi);

    bool hasInitedProcessInfos = false;
    std::vector<ProcessInfo> processInfos;
    std::vector<std::string> packageIntFuncNames;

    bool hasError = false;
};

class MyError {
public:
    MyError(WaveAspects& plugin) : plugin(plugin)
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
    WaveAspects& plugin;
};

bool OuterTypeNameMeets(const CHIR::Func& func, const ProcessInfo& pi, CHIR::CHIRBuilder& builder)
{
    auto outer = func.GetParentCustomTypeDef();
    // If the `recursive` field in APILevel is `true`, it means, the override method in deriving class should also be
    // processed.
    if (!pi.to.recursively || !outer) {
        return pi.to.outerTypeName == (outer ? outer->GetSrcCodeIdentifier() : "");
    } else {
        if (pi.to.outerTypeName == outer->GetSrcCodeIdentifier()) {
            return true;
        }
        auto superTypes = outer->GetSuperTypesRecusively(builder);
        for (auto superType : superTypes) {
            if (superType->GetCustomTypeDef()->GetSrcCodeIdentifier() == pi.to.outerTypeName) {
                return true;
            }
        }
        return false;
    }
}

bool Meets(const CHIR::Func& func, const ProcessInfo& pi, CHIR::CHIRBuilder& builder)
{
    if (pi.to.packageName == func.GetPackageName()) {
        auto outer = func.GetParentCustomTypeDef();
        // Firstly, check if the outertype's name of func meets the ProcessInfo:
        if (OuterTypeNameMeets(func, pi, builder)) {
            // Secondly, check if the name of func meets the ProcessInfo:
            if (pi.to.methodName == func.GetSrcCodeIdentifier()) {
                auto funcType = func.GetFuncType();
                // Skip the `this` parameter automatically for instance method:
                if (outer && !func.TestAttr(Codira::CHIR::Attribute::STATIC)) {
                    auto paramTys = funcType->GetParamTypes();
                    paramTys = std::vector<CHIR::Type*>(paramTys.begin() + 1, paramTys.end());
                    funcType = builder.GetType<CHIR::FuncType>(paramTys, funcType->GetReturnType());
                }
                auto funcTypeStr = CHIR::GetTypeQualifiedName(*funcType);
                std::replace(funcTypeStr.begin(), funcTypeStr.end(), ':', '.');
                // Finally, check if the func signature meets the ProcessInfo:
                if (pi.to.funcTypeStr == funcTypeStr &&
                    pi.to.isStatic == func.TestAttr(Codira::CHIR::Attribute::STATIC)) {
                    return true;
                }
            }
        }
    }
    return false;
}

std::vector<CHIR::Value*> WaveAspects::PrepareArguments(const CHIR::Func& caller, const ProcessInfo& pi)
{
    MyError myError(*this);

    std::vector<CHIR::Value*> args;
    if (pi.insert.paramsNum != 0U) {
        // Check if parameters matched:
        if (caller.GetNumOfParams() != pi.insert.paramsNum) {
            myError.Emit("%s and %s have unmatched number of parameters.\n", caller.GetSrcCodeIdentifier().c_str(),
                pi.insert.methodMangledName.c_str());
            return {};
        }
        auto _args = caller.GetParams();
        // Aspect is a global function or static method,
        if (!pi.insert.isMemberFunc || pi.insert.isStatic) {
            args = std::vector<CHIR::Value*>(_args.begin(), _args.end());
        } else { // Aspect is an instance method,
            // the waved is an instance method
            if (caller.GetParentCustomTypeDef() && !caller.TestAttr(Codira::CHIR::Attribute::STATIC)) {
                args = std::vector<CHIR::Value*>(_args.begin(), _args.end());
            } else { // the waved is a global function or a static method
                myError.Emit("Do not wave %s which is an instance method into a global function or a static method.\n",
                    pi.insert.methodMangledName.c_str());
                return {};
            }
        }
    }
    return args;
}

namespace {
CHIR::Type* ComputeThisTypeOfCallee(const CHIR::FuncBase& callee, const std::vector<CHIR::Value*>& args)
{
    CHIR::Type* thisType = nullptr;
    if (callee.IsMemberFunc()) {
        if (callee.TestAttr(Codira::CHIR::Attribute::STATIC)) {
            thisType = callee.GetOuterDeclaredOrExtendedDef()->GetType();
        } else {
            CODEC_ASSERT(!args.empty());
            thisType = args[0]->GetType();
        }
    }
    return thisType;
}

CHIR::FuncBase* GetFuncByMangledName(CHIR::CHIRBuilder& builder, const std::string& funcMangledName)
{
    auto curPackage = builder.GetChirContext().GetCurPackage();
    for (auto iv : curPackage->GetImportedVarAndFuncs()) {
        if (!iv->IsFunc()) {
            continue;
        }
        auto func = dynamic_cast<CHIR::FuncBase*>(static_cast<CHIR::ImportedFunc*>(iv));
        if (func->GetIdentifierWithoutPrefix() == funcMangledName) {
            return func;
        }
    }
    for (auto func : curPackage->GetGlobalFuncs()) {
        if (func->GetIdentifierWithoutPrefix() == funcMangledName) {
            return func;
        }
    }
    return nullptr;
}

CHIR::Apply* CreateApplyAtFuncBeginning(
    CHIR::CHIRBuilder& builder, const CHIR::Func& caller, CHIR::FuncBase& callee, const std::vector<CHIR::Value*>& args)
{
    auto resultTy = callee.GetFuncType()->GetReturnType();
    auto entry = caller.GetEntryBlock();
    auto funcCallCxt = CHIR::FuncCallContext {
        .args = args,
        .thisType = ComputeThisTypeOfCallee(callee, args)
    };
    auto apply =
        builder.CreateExpression<CHIR::Apply>(CHIR::INVALID_LOCATION, resultTy, &callee, funcCallCxt, entry);
    apply->MoveBefore(entry->GetExpressionByIdx(0));
    return apply;
}
} // namespace

CHIR::FuncBase* WaveAspects::GetCalleeByProcessInfo(
    const ProcessInfo& pi, const CHIR::FuncBase& func, const std::vector<CHIR::Value*>& args) const
{
    CHIR::FuncBase* callee = GetFuncByMangledName(builder, pi.insert.methodMangledName);
    if (!callee) {
        if (pi.insert.paramsNum == 0) {
            callee = builder.CreateImportedVarOrFunc<CHIR::ImportedFunc>(
                builder.GetType<CHIR::FuncType>(std::vector<CHIR::Type*>{}, builder.GetUnitTy()),
                pi.insert.methodMangledName, "", "", "");
        } else {
            std::vector<CHIR::Type*> paramTypes;
            CHIR::Type* retType = builder.GetUnitTy();
            for (auto arg : args) {
                paramTypes.emplace_back(arg->GetType());
            }
            if (pi.annoClassName == "ReplaceFuncBody") {
                retType = func.GetReturnType();
                paramTypes.emplace_back(builder.GetType<CHIR::FuncType>(paramTypes, retType));
            }
            callee = builder.CreateImportedVarOrFunc<CHIR::ImportedFunc>(
                builder.GetType<CHIR::FuncType>(paramTypes, builder.GetUnitTy()), pi.insert.methodMangledName, "", "",
                "");
        }
    }
    return callee;
}

namespace {
bool HasFunction(const CHIR::Package& package, std::string funcName)
{
    for (auto func : package.GetGlobalFuncs()) {
        if (func->GetIdentifierWithoutPrefix() == funcName) {
            return true;
        }
    }
    for (auto imported : package.GetImportedVarAndFuncs()) {
        if (imported->IsFunc() && imported->GetIdentifierWithoutPrefix() == funcName) {
            return true;
        }
    }
    return false;
}
} // namespace

void WaveAspects::ReadInfo()
{
    char currentDir[1024];
    if (!getcwd(currentDir, sizeof(currentDir))) {
        perror("Failed to access cwd");
        exit(EXIT_FAILURE);
    }
    DIR* dp;
    struct dirent* ep;
    if ((dp = opendir(currentDir)) == NULL) {
        perror("Failed to access cwd");
        return;
    }
    while ((ep = readdir(dp)) != NULL) {
        if (strstr(ep->d_name, ".annoinfo") != NULL && ep->d_name[0] != '.') {
            ReadInfo(ep->d_name);
            auto packageInitFuncName = packageIntFuncNames.back();
            if (!HasFunction(*builder.GetCurPackage(), packageInitFuncName)) {
                // Call package global init of AOP
                auto pkgInit = builder.GetCurPackage()->GetPackageInitFunc();
                auto bb = pkgInit->GetEntryBlock();
                bb = bb->GetTerminator()->GetSuccessor(1);
                auto initF = builder.CreateImportedVarOrFunc<CHIR::ImportedFunc>(
                    builder.GetType<CHIR::FuncType>(std::vector<CHIR::Type*>{}, builder.GetVoidTy()),
                    packageInitFuncName, "", "", "");
                std::vector<CHIR::Value*> args;
                auto funcCallCxt = CHIR::FuncCallContext {
                    .args = args,
                    .thisType = ComputeThisTypeOfCallee(*initF, args)
                };
                auto apply = builder.CreateExpression<CHIR::Apply>(
                    CHIR::INVALID_LOCATION, builder.GetVoidTy(), initF, funcCallCxt, bb);
                apply->MoveBefore(bb->GetExpressionByIdx(2));
            }
        }
    }
    closedir(dp);
}

// 实现 plugin
void WaveAspects::Run(CHIR::Func& func)
{
    if (hasError) {
        return;
    }
    // Make sure only initialize processInfos once:
    if (!hasInitedProcessInfos) {
        ReadInfo();
        hasInitedProcessInfos = true;
    }
    //
    for (auto pi : processInfos) {
        if (Meets(func, pi, builder)) {
            auto args = PrepareArguments(func, pi);
            auto callee = GetCalleeByProcessInfo(pi, func, args);
            if (pi.annoClassName == "InsertAtEntry") {
                auto resultTy = callee->GetFuncType()->GetReturnType();
                auto entry = func.GetEntryBlock();
                auto funcCallCxt = CHIR::FuncCallContext {
                    .args = args,
                    .thisType = ComputeThisTypeOfCallee(*callee, args)
                };
                auto apply = builder.CreateExpression<CHIR::Apply>(
                    CHIR::INVALID_LOCATION, resultTy, callee, funcCallCxt, entry);
                apply->MoveBefore(entry->GetExpressionByIdx(0));
            } else if (pi.annoClassName == "InsertAtExit") {
                auto preAction = [this, args, callee](CHIR::Expression& expr) {
                    if (expr.GetExprKind() == Codira::CHIR::ExprKind::EXIT) {
                        auto resultTy = callee->GetFuncType()->GetReturnType();
                        auto bb = expr.GetParentBlock();
                        auto funcCallCxt = CHIR::FuncCallContext {
                            .args = args,
                            .thisType = ComputeThisTypeOfCallee(*callee, args)
                        };
                        auto apply = builder.CreateExpression<CHIR::Apply>(
                            CHIR::INVALID_LOCATION, resultTy, callee, funcCallCxt, bb);
                        apply->MoveBefore(&expr);
                    }
                    return CHIR::VisitResult::CONTINUE;
                };
                CHIR::Visitor::Visit(func, preAction);
            } else if (pi.annoClassName == "ReplaceFuncBody") {
                auto funcName = func.GetIdentifierWithoutPrefix();
                size_t suffix = 0U;
                auto originalFuncName = funcName + ".original" + std::to_string(suffix);
                while (GetFuncByMangledName(builder, originalFuncName) != nullptr) {
                    ++suffix;
                    originalFuncName = funcName + ".original" + std::to_string(suffix);
                }
                auto newFunc = builder.CreateFunc(func.GetDebugLocation(), func.GetFuncType(), originalFuncName,
                    func.GetSrcCodeIdentifier(), "", func.GetPackageName());
                // create parameters
                for (auto paramTy : func.GetFuncType()->GetParamTypes()) {
                    builder.CreateParameter(paramTy, CHIR::INVALID_LOCATION, *newFunc);
                }
                CHIR::BlockGroupCopyHelper helper(builder);
                auto [clonedBlockGroup, newBlockGroupRetValue] = helper.CloneBlockGroup(*func.GetBody(), *newFunc);
                newFunc->InitBody(*clonedBlockGroup);
                size_t retValueExprIdx = 0U;
                for (auto expr : func.GetBody()->GetEntryBlock()->GetExpressions()) {
                    if (expr->GetExprKind() == Codira::CHIR::ExprKind::DEBUGEXPR) {
                        continue;
                    }
                    if (auto rst = expr->GetResult(); rst && rst->IsRetValue()) {
                        break;
                    }
                    ++retValueExprIdx;
                }
                newFunc->SetReturnValue(
                    *clonedBlockGroup->GetEntryBlock()->GetExpressionByIdx(retValueExprIdx)->GetResult());
                for (size_t i = 0; i < func.GetParams().size(); ++i) {
                    func.GetParams()[i]->ReplaceWith(*newFunc->GetParams()[i], clonedBlockGroup);
                }

                CHIR::BlockGroup* body = builder.CreateBlockGroup(func);
                func.DestroySelf();
                func.InitBody(*body);
                auto entry = builder.CreateBlock(body);
                body->SetEntryBlock(entry);
                auto returnType = func.GetReturnType();
                auto ret = CHIR::CreateAndAppendExpression<CHIR::Allocate>(
                    builder, builder.GetType<CHIR::RefType>(returnType), returnType, entry);
                func.SetReturnValue(*ret->GetResult());
                args.emplace_back(newFunc);
                auto apply = CreateApplyAtFuncBeginning(builder, func, *callee, args);
                entry->AppendExpression(builder.CreateExpression<CHIR::Store>(
                    CHIR::INVALID_LOCATION, builder.GetUnitTy(), apply->GetResult(), ret->GetResult(), entry));
                entry->AppendExpression(builder.CreateTerminator<CHIR::Exit>(CHIR::INVALID_LOCATION, entry));
            }
        }
    }

    if (hasError) {
        Errorf("plugin terminated by some unexpected usages.\n");
        throw NullPointerException();
    }
}

void WaveAspects::ReadInfo(const std::string& annoInfoPath)
{
    auto absPath = FileUtil::GetAbsPath(annoInfoPath);
    if (!absPath.has_value()) {
        std::cerr << "Invalid file path." << std::endl;
        return;
    }
    std::ifstream inFile(absPath.value());
    if (!inFile) {
        std::cerr << "Cannot access this file." << std::endl;
    } else {
        // The first line in .annoinfo file is the mangledName of DFX package global init function.
        std::string str;
        std::getline(inFile, str);
        packageIntFuncNames.emplace_back(str);
        // The following content lists the information about each processInfo.
        // Each processInfo consists of the following components:
        //   1)  annotation name
        //   2)  mangledName of aspect method
        //   3)  the number of parameters of aspect method
        //   4)  whether aspect method is a member function: "1" means true
        //   5)  whether aspect method is a static function: "1" means true
        //   6)  package name of the method to be woven
        //   7)  outerType name of the method to be woven
        //   8)  mangledName of the method to be woven
        //   9)  the signature of method
        //  10)  whether the weaved is static method
        //  11)  whether weave into the override method in subtypes: "true" means true
        std::string annoClassName;
        while (std::getline(inFile, annoClassName)) {
            ProcessInfo pi;
            pi.annoClassName = annoClassName;                  // 1)
            std::getline(inFile, pi.insert.methodMangledName); // 2)
            std::string tmp;
            std::getline(inFile, tmp);
            pi.insert.paramsNum = std::stoul(tmp); // 3)
            std::getline(inFile, tmp);
            pi.insert.isMemberFunc = (tmp == "1"); // 4)
            std::getline(inFile, tmp);
            pi.insert.isStatic = (tmp == "1");         // 5)
            std::getline(inFile, pi.to.packageName);   // 6)
            std::getline(inFile, pi.to.outerTypeName); // 7)
            std::getline(inFile, pi.to.methodName);    // 8)
            std::getline(inFile, pi.to.funcTypeStr);   // 9)
            std::getline(inFile, tmp);
            pi.to.isStatic = (tmp == "true"); // 10)
            std::getline(inFile, tmp);
            pi.to.recursively = (tmp == "true"); // 11)
            processInfos.emplace_back(pi);
        }
        inFile.close();
    }
}

// register plugin
CHIR_PLUGIN(WaveAspects)
