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

#include "Codira/CHIR/CHIR.h"
#include "Codira/CHIR/Interpreter/ConstEval.h"
#include "Codira/Utils/ProfileRecorder.h"

namespace Codira::CHIR {
using namespace AST;

namespace {
// used-defined @Annotation on a decl
struct AnnotationInfo {
    const ClassDecl* decl;
    AST::Annotation* anno;        // non-const for write back annotation result after consteval
    std::vector<GlobalVar*> vars; // one global variable for each element in the argument `target` array;
                                  // empty if no argument is provided
    std::vector<Func*> funcs;     // init funcs of vars, to be called in file init function
};

// custom annotation defined on a decl
struct CustomAnnoInstance {
    const AST::Annotation* src; // from which ast node this annotation check info is generated. used for diagnostics
    const ClassDef* def;
};

// all custom annotations on a TypeDef
struct CustomAnnoInfoOnType {
    const CustomTypeDef* type;
    std::vector<CustomAnnoInstance> annos;
};

struct CustomAnnoInfoOnDecl {
    const Decl* decl; // abstract function do not have CHIR symbol, use AST node
    std::vector<CustomAnnoInstance> annos;
};

// static variables that records the info first and are used to check later
// in case packages need compile in parallel, record g_annoInfo as member variable of class
// AnnotationTranslator, store it somewhere else; the two collections can be collected as well for better performance
// (one whole ast traversal less), or more conveniently not to store those two collections and to traverse the ast and
// to check against the stored g_annoInfo instead.
// record @Annotation info of translated Annotation classes
std::unordered_map<const ClassDef*, AnnotationInfo> g_annoInfo{};
// record custom annotation info of any codira decl
std::vector<CustomAnnoInfoOnType> g_typeAnnoInfo{};
std::vector<CustomAnnoInfoOnDecl> g_valueAnnoInfo{}; // custom annotation info on var, func, and prop
} // namespace

class AnnotationTranslator {
public:
    AnnotationTranslator(Translator& translator, CHIRBuilder& builder, const GlobalOptions& opts)
        : tr{translator}, bd{builder}, parallel{opts.GetJobs() > 1UL}
    {
    }

    static bool IsImported(const Decl& decl)
    {
        if (decl.outerDecl) {
            if (auto generic = decl.outerDecl->genericDecl) {
                return generic->TestAttr(AST::Attribute::IMPORTED);
            }
        }
        if (decl.genericDecl) {
            return decl.genericDecl->TestAttr(AST::Attribute::IMPORTED);
        }
        return decl.TestAttr(AST::Attribute::IMPORTED);
    }

    void CollectAnnoInfo(const Decl& decl, const CustomTypeDef& type) &&
    {
        if (SkipCollectCustomAnnoInfo(decl)) {
            // from ast node directly read annotation target info
            for (auto& anno : decl.annotations) {
                if (anno->kind == AST::AnnotationKind::ANNOTATION) {
                    PushAnnotation(StaticCast<ClassDef>(type), StaticCast<ClassDecl>(decl), *anno);
                    // no custom annotation check required for imported decl
                    return;
                }
            }
            return;
        }
        // collect both @Annotation and custom annotation in one annotation traverse
        std::vector<CustomAnnoInstance> res;
        for (size_t annoIndex = 0, customAnnoIdx = 0; annoIndex < decl.annotations.size(); ++annoIndex) {
            auto& anno = decl.annotations[annoIndex];
            // collect @Annotation info
            if (anno->kind == AST::AnnotationKind::ANNOTATION) {
                CollectAnnotationInfo(*anno, StaticCast<ClassDecl&>(decl), type);
                break; // cannot have both @Annotation and custom annotation
            }
            if (anno->kind == AnnotationKind::CUSTOM) {
                // collect custom annotation info for later check
                auto classDef = GetDefFromAnnotationInstance(*decl.annotationsArray->children[customAnnoIdx]);
                res.emplace_back(CustomAnnoInstance{anno.get(), classDef});
                ++customAnnoIdx;
            }
        }
        if (!res.empty()) {
            PushTypeAnno(type, std::move(res));
        }

        // collect custom annotations for members
        for (auto member : decl.GetMemberDeclPtrs()) {
            if (auto prop = DynamicCast<PropDecl>(member)) {
                CollectPropAnnoInfo(*prop);
            } else {
                CollectAnnoInfo(*member);
            }
        }
        if (auto enDecl = DynamicCast<EnumDecl>(&decl)) {
            for (auto& cons : enDecl->constructors) {
                CollectAnnoInfo(*cons);
            }
        }
    }

    void CollectAnnoInfo(const Decl& decl) const
    {
        if (SkipCollectCustomAnnoInfo(decl)) {
            return;
        }
        if (auto res = CollectCustomAnnoInstances(decl); !res.empty()) {
            PushValueAnno(decl, std::move(res));
        }
        if (auto func = DynamicCast<FuncDecl>(&decl)) {
            if (func->funcBody) {
                for (auto& param : func->funcBody->paramLists[0]->params) {
                    if (auto res = CollectCustomAnnoInstances(*param); !res.empty()) {
                        PushValueAnno(*param, std::move(res));
                    }
                }
            }
        }
    }

private:
    Translator& tr;
    CHIRBuilder& bd;
    bool parallel;

    static std::mutex valueLock;
    static std::mutex typeLock;
    static std::mutex annoLock;

    void PushValueAnno(const Decl& decl, std::vector<CustomAnnoInstance>&& annos) const
    {
        if (parallel) {
            std::lock_guard g{valueLock};
            g_valueAnnoInfo.emplace_back(CustomAnnoInfoOnDecl{&decl, std::move(annos)});
        } else {
            g_valueAnnoInfo.emplace_back(CustomAnnoInfoOnDecl{&decl, std::move(annos)});
        }
    }
    void PushTypeAnno(const CustomTypeDef& def, std::vector<CustomAnnoInstance>&& annos) const
    {
        if (parallel) {
            std::lock_guard g{typeLock};
            g_typeAnnoInfo.emplace_back(CustomAnnoInfoOnType{&def, std::move(annos)});
        } else {
            g_typeAnnoInfo.emplace_back(CustomAnnoInfoOnType{&def, std::move(annos)});
        }
    }
    void PushAnnotation(const ClassDef& def, const ClassDecl& decl, AST::Annotation& anno) const
    {
        if (parallel) {
            std::lock_guard g{annoLock};
            g_annoInfo.emplace(&def, AnnotationInfo{&decl, &anno});
        } else {
            g_annoInfo.emplace(&def, AnnotationInfo{&decl, &anno});
        }
    }
    void PushAnnotation(const ClassDef& def, AnnotationInfo&& info) const
    {
        if (parallel) {
            std::lock_guard g{annoLock};
            g_annoInfo.emplace(&def, std::move(info));
        } else {
            g_annoInfo.emplace(&def, std::move(info));
        }
    }

    bool SkipCollectCustomAnnoInfo(const Decl& decl) const
    {
        if (tr.opts.enIncrementalCompilation && !decl.toBeCompiled) {
            return true;
        }
        return IsImported(decl);
    }

    // collect @Annotation info placed on `decl`
    void CollectAnnotationInfo(AST::Annotation& anno, const ClassDecl& decl, const CustomTypeDef& type)
    {
        if (anno.args.empty()) {
            // emplace empty annotation info for @Annotation without argument
            PushAnnotation(StaticCast<ClassDef>(type), decl, anno);
            return;
        }
        auto info = CreateAnnotationInfo(decl, anno);
        PushAnnotation(StaticCast<ClassDef>(type), std::move(info));
    }

    // get the referenced CHIR Annotation class from a custom annotation instance (constructor call)
    ClassDef* GetDefFromAnnotationInstance(const Expr& expr) const
    {
        auto resolvedFunction = StaticCast<const CallExpr&>(expr).resolvedFunction;
        CODEC_NULLPTR_CHECK(resolvedFunction);
        auto type = tr.TranslateType(*resolvedFunction->outerDecl->ty);
        return StaticCast<ClassType*>(StaticCast<RefType*>(type)->GetBaseType())->GetClassDef();
    }

    // create a global variable for each element in the @Annotation targets. This can be reduced to a single var should
    // CODIRA support array as literal type
    static std::string GetAnnotationVarName(const std::string& basename, size_t index)
    {
        return basename + ANNOTATION_VAR_POSTFIX + std::to_string(index);
    }

    // create a struct that contains all information of a @Annotation placed on a class
    AnnotationInfo CreateAnnotationInfo(const ClassDecl& decl, AST::Annotation& anno)
    {
        auto annoTargetNum = StaticCast<ArrayLit&>(*anno.args[0]->expr).children.size();
        std::vector<GlobalVar*> vars(annoTargetNum);
        std::vector<Func*> funcs(annoTargetNum);
        for (size_t i{0}; i < annoTargetNum; ++i) {
            auto var = CreateAnnotationVar(decl, anno, i);
            auto func = CreateAnnoVarInit(*var, anno, i);
            vars[i] = var;
            funcs[i] = func;
        }
        return {&decl, &anno, std::move(vars), std::move(funcs)};
    }

    GlobalVar* CreateAnnotationVar(const ClassDecl& decl, const AST::Annotation& anno, size_t i)
    {
        auto& basename = decl.mangledName;
        auto varname = GetAnnotationVarName(basename, i);
        auto& expr = *StaticCast<AST::ArrayLit>(*anno.args[0]->expr).children[i];
        auto loc = tr.TranslateLocation(expr);
        auto annotationTargetType = tr.TranslateType(*expr.ty);
        auto varType = bd.GetType<RefType>(annotationTargetType);
        auto srcCodeIdentifier = decl.identifier.Val() + ANNOTATION_VAR_POSTFIX + std::to_string(i);
        auto var = bd.CreateGlobalVar(
            std::move(loc), varType, varname, std::move(srcCodeIdentifier), varname, decl.fullPackageName);
        var->Set<SkipCheck>(SkipKind::SKIP_DCE_WARNING);
        var->EnableAttr(Attribute::COMPILER_ADD);
        var->EnableAttr(Attribute::NO_DEBUG_INFO);
        var->EnableAttr(Attribute::CONST);
        var->EnableAttr(Attribute::NO_REFLECT_INFO);
        var->Set<LinkTypeInfo>(Linkage::INTERNAL);
        return var;
    }
    
    // we need a unique name for each annotation target element for each @Annotation instance.
    // @Annotation placed on VarWithPatternDecl cannot use the variable's identifier (it is empty), and we do not have
    // mangledName on CHIR, so we use its raw mangled name.
    static std::string GetAnnotationTargetElementInitName(const GlobalVar& var)
    {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        auto name = MANGLE_CODIRA_PREFIX + MANGLE_GLOBAL_VARIABLE_INIT_PREFIX;
        if (var.GetSrcCodeIdentifier().empty()) {
            name += var.GetRawMangledName();
        } else {
            name += MangleUtils::MangleName(var.GetPackageName()) + MangleUtils::MangleName(var.GetSrcCodeIdentifier());
        }
        return name + MANGLE_FUNC_PARAM_TYPE_PREFIX + MANGLE_VOID_TY_SUFFIX;
#endif
    }

    Func* CreateAnnoVarInit(GlobalVar& var, AST::Annotation& anno, size_t i)
    {
        auto funcname = GetAnnotationTargetElementInitName(var);
        auto& expr = *StaticCast<AST::ArrayLit>(*anno.args[0]->expr).children[i];
        auto loc = tr.TranslateLocation(expr);

        auto func =
            tr.CreateEmptyGVInitFunc(funcname, "", funcname, var.GetPackageName(), Linkage::INTERNAL, loc, true);
        func->EnableAttr(Attribute::COMPILER_ADD);
        func->EnableAttr(Attribute::NO_DEBUG_INFO);

        auto value = Translator::TranslateASTNode(expr, tr);
        tr.GetCurrentBlock()->AppendExpression(
            bd.CreateExpression<Store>(std::move(loc), bd.GetUnitTy(), value, &var, tr.GetCurrentBlock()));
        if (auto curBlock = tr.GetCurrentBlock(); curBlock->GetTerminator() == nullptr) {
            curBlock->AppendExpression(bd.CreateTerminator<Exit>(curBlock));
        }
        var.SetInitFunc(*func);
        return func;
    }

    std::vector<CustomAnnoInstance> CollectCustomAnnoInstances(const Decl& decl) const
    {
        std::vector<CustomAnnoInstance> res;
        for (size_t annoIndex{0}, customIndex{0}; annoIndex < decl.annotations.size(); ++annoIndex) {
            auto& anno = decl.annotations[annoIndex];
            if (anno->kind == AST::AnnotationKind::CUSTOM) {
                auto classDef = GetDefFromAnnotationInstance(*decl.annotationsArray->children[customIndex++]);
                res.emplace_back(CustomAnnoInstance{anno.get(), classDef});
            }
        }
        return res;
    }

    void CollectPropAnnoInfo(const PropDecl& decl) const
    {
        if (auto res = CollectCustomAnnoInstances(decl); !res.empty()) {
            PushValueAnno(decl, std::move(res));
        }
    }
};
std::mutex AnnotationTranslator::valueLock{};
std::mutex AnnotationTranslator::typeLock{};
std::mutex AnnotationTranslator::annoLock{};

// interfaces between AnnotationTranslator and other modules are implemented here
void Translator::CollectTypeAnnotation(const InheritableDecl& decl, const CustomTypeDef& cl)
{
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    // neither collect nor check annotations in compute annotations stage
    // Since annotations must be consistent between common and platform sides,
    // platform declarations can reuse serialized annotations directly.
    if (isComputingAnnos || cl.TestAttr(CHIR::Attribute::DESERIALIZED)) {
        return;
    }
#endif
    AnnotationTranslator{*this, builder, opts}.CollectAnnoInfo(decl, cl);
}
void Translator::CollectValueAnnotation(const Decl& decl)
{
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    if (isComputingAnnos) {
        return;
    }
#endif
    // Since annotations must be consistent between common and platform sides,
    // platform declarations can reuse serialized annotations directly.
    if (decl.TestAttr(AST::Attribute::IMPORTED) || decl.TestAttr(AST::Attribute::GENERIC_INSTANTIATED) ||
        decl.TestAttr(AST::Attribute::PLATFORM)) {
        return;
    }
    AnnotationTranslator{*this, builder, opts}.CollectAnnoInfo(decl);
}

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
void GlobalVarInitializer::InsertAnnotationVarInitInto(Func& packageInit)
{
    for (auto& info : g_annoInfo) {
        for (auto init : info.second.funcs) {
            initFuncsForConstVar.emplace_back(init);
            InsertInitializerIntoPackageInitializer(*init, packageInit);
        }
    }
}
#endif

namespace {
struct AnnotationTarget {
    unsigned val;
    bool Matches(unsigned v) const
    {
        return (val & v) != 0;
    }
};

const std::string_view ANNOTATION_TARGET_2_STRING[]{
    "type", "parameter", "init", "member property", "member function", "member variable", "enum constructor",
    "global function", "global variable", "extend"};

class AnnotationChecker {
public:
    explicit AnnotationChecker(const DiagAdapter& d) : diag{d}
    {
    }

    bool CheckAnnotations() &&
    {
        Utils::ProfileRecorder p{"CHIR", "AnnotationTargetCheck"};
        WriteBackAnnotations();
        auto res = CheckAnnotationsImpl();
        return res;
    }

private:
    const DiagAdapter& diag;

    // write @Annotation info from the result of consteval back to the AST nodes
    void WriteBackAnnotations()
    {
        std::unordered_map<const ClassDef*, AnnotationTarget> cmap;
        for (auto info : g_annoInfo) {
            if (!info.first->TestAttr(Attribute::IMPORTED)) {
                // write back to AST if not imported
                WriteBackAnnotation(info.second);
            }
            // this operation is mutable and cannot be run in parallel
            cmap.emplace(info.first, AnnotationTarget{info.second.anno->target});
        }
        g_annoInfo.clear();
        checkMap = std::move(cmap);
    }

    void WriteBackAnnotation(const AnnotationInfo& info) const
    {
        auto& vars = info.vars;
        if (vars.empty()) {
            info.anno->EnableAllTargets();
            return;
        }
        for (size_t i{0}; i < vars.size(); ++i) {
            if (!vars[i]->GetInitializer() || vars[i]->GetInitializer()->IsNullLiteral()) {
                // if const eval fails to replace the initializer with a constant value, find the store
                // statement from its init func
                info.anno->EnableTarget(static_cast<AST::AnnotationTarget>(GetUnsignedValFromInit(*info.funcs[i])));
            } else {
                info.anno->EnableTarget(static_cast<AST::AnnotationTarget>(
                    StaticCast<IntLiteral*>(vars[i]->GetInitializer())->GetUnsignedVal()));
            }
        }
    }

    // check map, from Annotation class to AnnotationTarget
    std::unordered_map<const ClassDef*, AnnotationTarget> checkMap;

    static unsigned GetUnsignedValFromInit(const Func& func)
    {
        // the init func after consteval shall be
        // %1: unsigned64 = ConstantInt(xxx)
        // %2: Enum-AnnotationTarget = TypeCast(Enum-AnnotationTarget, %1)
        // %3 = Store(%2, `var`)
        // just find the int literal and retrieve its value
        for (auto exp : func.GetEntryBlock()->GetNonTerminatorExpressions()) {
            if (auto uval = DynamicCast<Constant>(exp)) {
                return static_cast<unsigned>(uval->GetUnsignedIntLitVal());
            }
        }
        InternalError("Annotation checking failed");
        return 0;
    }

    // paralle version. Current impl uses the serialised version
    [[maybe_unused]] bool CheckAnnotationsParallelImpl() const
    {
        constexpr unsigned f{8};
        constexpr unsigned g{9};
        size_t threadNum = std::thread::hardware_concurrency() * f / g;

        Utils::TaskQueue qu{threadNum};
        std::vector<Utils::TaskResult<bool>> results;

        for (auto& info : g_typeAnnoInfo) {
            results.emplace_back(qu.AddTask<bool>([&info, this] { return CheckType(info); }));
        }
        for (auto& info : g_valueAnnoInfo) {
            results.emplace_back(qu.AddTask<bool>([&info, this] { return CheckValue(info); }));
        }
        qu.RunAndWaitForAllTasksCompleted();

        g_typeAnnoInfo.clear();
        g_valueAnnoInfo.clear();
        return std::all_of(results.begin(), results.end(), [](auto& res) { return res.get(); });
    }
    bool CheckAnnotationsImpl() const
    {
        bool res = true;
        for (auto& info : g_typeAnnoInfo) {
            res = CheckType(info) && res;
        }
        for (auto& info : g_valueAnnoInfo) {
            res = CheckValue(info) && res;
        }
        return res;
    }

    using TargetT = AST::AnnotationTargetT;
    static TargetT GetTarget(const Decl& decl)
    {
        if (Is<FuncParam>(decl)) {
            return static_cast<TargetT>(AST::AnnotationTarget::PARAMETER);
        }
        if (Is<PropDecl>(decl)) {
            return static_cast<TargetT>(AST::AnnotationTarget::MEMBER_PROPERTY);
        }
        // enum constructor must be checked before var and func decl
        if (decl.TestAttr(AST::Attribute::ENUM_CONSTRUCTOR)) {
            return static_cast<TargetT>(AST::AnnotationTarget::ENUM_CONSTRUCTOR);
        }
        if (Is<VarDeclAbstract>(decl)) {
            if (decl.TestAttr(AST::Attribute::GLOBAL)) {
                return static_cast<TargetT>(AST::AnnotationTarget::GLOBAL_VARIABLE);
            }
            return static_cast<TargetT>(AST::AnnotationTarget::MEMBER_VARIABLE);
        }
        auto func = StaticCast<FuncDecl>(&decl);
        if (func->TestAttr(AST::Attribute::CONSTRUCTOR)) {
            return static_cast<TargetT>(AST::AnnotationTarget::INIT);
        }
        if (decl.TestAttr(AST::Attribute::GLOBAL)) {
            return static_cast<TargetT>(AST::AnnotationTarget::GLOBAL_FUNCTION);
        }
        return static_cast<TargetT>(AST::AnnotationTarget::MEMBER_FUNCTION);
    }

    bool CheckValue(const CustomAnnoInfoOnDecl& info) const
    {
        auto targetid = GetTarget(*info.decl);
        unsigned target = 1u << targetid;
        bool res{true};
        for (auto& annotation : info.annos) {
            auto targets = checkMap.at(annotation.def);
            if (!targets.Matches(target)) {
                (void)diag.diag.DiagnoseRefactor(DiagKindRefactor::chir_annotation_not_applicable, *annotation.src,
                    annotation.src->identifier, std::string{ANNOTATION_TARGET_2_STRING[targetid]});
                res = false;
            }
        }
        return res;
    }

#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    static TargetT GetTarget(const CustomTypeDef& type)
    {
        return static_cast<TargetT>(Is<ExtendDef>(type) ? AST::AnnotationTarget::EXTEND : AST::AnnotationTarget::TYPE);
    }
#endif

    bool CheckType(const CustomAnnoInfoOnType& info) const
    {
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
        auto targetid = GetTarget(*info.type);
#endif
        unsigned target = 1u << targetid;
        bool res{true};
        for (auto& annotation : info.annos) {
            auto targets = checkMap.at(annotation.def);
            if (!targets.Matches(target)) {
                (void)diag.diag.DiagnoseRefactor(DiagKindRefactor::chir_annotation_not_applicable, *annotation.src,
                    annotation.src->identifier, std::string{ANNOTATION_TARGET_2_STRING[targetid]});
                res = false;
            }
        }
        return res;
    }
};
} // unnamed namespace

bool ToCHIR::RunAnnotationChecks()
{
    Utils::ProfileRecorder r{"CHIR", "AnnotationCheck"};
    if (!AnnotationChecker{diag}.CheckAnnotations()) {
        return false;
    }
    return true;
}
} // namespace Codira::CHIR
