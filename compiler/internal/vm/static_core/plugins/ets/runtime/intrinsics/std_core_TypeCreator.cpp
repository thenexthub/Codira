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

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-field.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-ins.h"
#include "assembler/assembly-program.h"
#include "assembler/assembly-record.h"
#include "assembler/assembly-type.h"
#include "ins_create_api.h"

#include "ets_coroutine.h"

#include "runtime/include/runtime.h"
#include "runtime/mem/vm_handle.h"
#include <libarkfile/include/source_lang_enum.h>
#include "types/ets_array.h"
#include "types/ets_class.h"
#include "types/ets_field.h"
#include "types/ets_method.h"
#include "types/ets_primitives.h"
#include "types/ets_runtime_linker.h"
#include "types/ets_string.h"
#include "types/ets_typeapi_create_panda_constants.h"
#include "types/ets_type.h"
#include "types/ets_typeapi.h"
#include "types/ets_typeapi_create.h"
#include "types/ets_typeapi_type.h"

namespace ark::ets::intrinsics {
namespace {

EtsString *ErrorFromCtx(TypeCreatorCtx *ctx)
{
    if (auto err = ctx->GetError(); err) {
        EtsString::CreateFromMUtf8(err.value().data(), err.value().size());
    }
    return nullptr;
}

pandasm::Type GetPandasmTypeFromDescriptor(TypeCreatorCtx *ctx, std::string_view descriptor)
{
    auto ret = pandasm::Type::FromDescriptor(descriptor);
    if (ret.IsObject()) {
        auto pandasmName = ret.GetPandasmName();
        ctx->AddRefTypeAsExternal(pandasmName);
        return pandasm::Type {std::move(pandasmName), ret.GetRank()};
    }
    return ret;
}

pandasm::Type EtsTypeToPandasm(EtsType type)
{
    ASSERT(type != EtsType::OBJECT);
    return pandasm::Type::FromPrimitiveId(ConvertEtsTypeToPandaType(type).GetId());
}

pandasm::Ins GetReturnStatement(EtsType type)
{
    switch (type) {
        case EtsType::BOOLEAN:
        case EtsType::BYTE:
        case EtsType::CHAR:
        case EtsType::SHORT:
        case EtsType::INT:
        case EtsType::FLOAT:
            return pandasm::Create_RETURN();
        case EtsType::LONG:
        case EtsType::DOUBLE:
            return pandasm::Create_RETURN_64();
        case EtsType::VOID:
            return pandasm::Create_RETURN_VOID();
        case EtsType::OBJECT:
            return pandasm::Create_RETURN_OBJ();
        case EtsType::UNKNOWN:
        default:
            UNREACHABLE();
    }
}

PandasmMethodCreator CreateCopiedMethod(TypeCreatorCtx *ctx, const std::string &prefix, EtsMethod *method)
{
    PandasmMethodCreator fn {prefix + method->GetName(), ctx};

    size_t refNum = 0;

    for (size_t argNum = 0; argNum < method->GetNumArgs(); argNum++) {
        auto etsType = method->GetArgType(argNum);
        if (etsType == EtsType::OBJECT) {
            fn.AddParameter(GetPandasmTypeFromDescriptor(ctx, method->GetRefArgType(refNum++)));
        } else {
            fn.AddParameter(EtsTypeToPandasm(etsType));
        }
    }
    auto etsRetType = method->GetReturnValueType();
    if (etsRetType == EtsType::OBJECT) {
        fn.AddResult(GetPandasmTypeFromDescriptor(ctx, method->GetReturnTypeDescriptor()));
    } else {
        fn.AddResult(EtsTypeToPandasm(etsRetType));
    }

    return fn;
}

void SetAccessFlags(pandasm::ItemMetadata *meta, const std::string_view &attr, EtsTypeAPIAccessModifier mod)
{
    switch (mod) {
        case EtsTypeAPIAccessModifier::PUBLIC:
            meta->SetAttributeValue(attr, typeapi_create_consts::ATTR_ACCESS_VAL_PUBLIC);
            break;
        case EtsTypeAPIAccessModifier::PRIVATE:
            meta->SetAttributeValue(attr, typeapi_create_consts::ATTR_ACCESS_VAL_PRIVATE);
            break;
        case EtsTypeAPIAccessModifier::PROTECTED:
            meta->SetAttributeValue(attr, typeapi_create_consts::ATTR_ACCESS_VAL_PROTECTED);
            break;
        default:
            UNREACHABLE();
    }
}

bool HasFeatureAttribute(EtsInt where, EtsTypeAPIAttributes attr)
{
    static_assert(sizeof(EtsInt) == sizeof(uint32_t));
    return (bit_cast<uint32_t>(static_cast<EtsInt>(attr)) & bit_cast<uint32_t>(where)) != 0;
}

template <typename T>
T *PtrFromLong(EtsLong ptr)
{
    return reinterpret_cast<T *>(ptr);
}

template <typename T>
EtsLong PtrToLong(T *ptr)
{
    return reinterpret_cast<EtsLong>(ptr);
}

}  // namespace

extern "C" {
EtsLong TypeAPITypeCreatorCtxCreate()
{
    auto ret = Runtime::GetCurrent()->GetInternalAllocator()->New<TypeCreatorCtx>();
    return reinterpret_cast<ssize_t>(ret);
}

void TypeAPITypeCreatorCtxDestroy(EtsLong ctx)
{
    Runtime::GetCurrent()->GetInternalAllocator()->Delete(PtrFromLong<TypeCreatorCtx>(ctx));
}

EtsString *TypeAPITypeCreatorCtxCommit(EtsLong ctxPtr, EtsArray *objects, EtsRuntimeLinker *targetLinker)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsArray> objectsHandle {coroutine, objects->GetCoreType()};

    auto ctx = PtrFromLong<TypeCreatorCtx>(ctxPtr);

    ctx->FlushTypeAPICtxDataRecordsToProgram();

    panda_file::MemoryWriter writer;
    auto status = pandasm::AsmEmitter::Emit(&writer, ctx->Program());
    if (!status) {
        return EtsString::CreateFromMUtf8(("can't write file in memory" + pandasm::AsmEmitter::GetLastError()).c_str());
    }

    // debug
    // panda_file::FileWriter {"/tmp/test.abc"}.WriteBytes(writer.GetData());

    auto *classLinkerContext = targetLinker->GetClassLinkerContext();
    ASSERT(!classLinkerContext->IsBootContext());
    auto linker = Runtime::GetCurrent()->GetClassLinker();
    auto &data = writer.GetData();
    auto file = panda_file::OpenPandaFileFromMemory(data.data(), data.size());

    ctx->SaveObjects(coroutine, objectsHandle, classLinkerContext);

    const auto *pf = file.get();
    // Panda file will be stored until runtime destruction
    linker->AddPandaFile(std::move(file), classLinkerContext);

    status = ctx->InitializeCtxDataRecord(coroutine, classLinkerContext, pf);
    if (UNLIKELY(!status)) {
        LOG(ERROR, RUNTIME) << "Type API context data initialization has failed with exception";
    }
    return nullptr;
}

EtsLong TypeAPITypeCreatorCtxClassCreate(EtsLong ctxPtr, EtsString *name, EtsInt attrs)
{
    auto *ctx = PtrFromLong<TypeCreatorCtx>(ctxPtr);
    pandasm::Record rec {std::string {name->GetMutf8()}, SourceLanguage::ETS};
    rec.conflict = true;

    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::FINAL)) {
        rec.metadata->SetAttribute(typeapi_create_consts::ATTR_FINAL);
    }

    auto res = ctx->AddPandasmRecord(std::move(rec));
    if (!res.second) {
        ctx->AddError("duplicate class " + res.first.name);
        return 0;
    }
    return PtrToLong(ctx->Alloc<ClassCreator>(&res.first, ctx));
}

EtsLong TypeAPITypeCreatorCtxInterfaceCreate(EtsLong ctxPtr, EtsString *name)
{
    auto *ctx = PtrFromLong<TypeCreatorCtx>(ctxPtr);
    pandasm::Record rec {std::string {name->GetMutf8()}, SourceLanguage::ETS};
    rec.conflict = true;
    for (const auto &attr : typeapi_create_consts::ATTR_INTERFACE) {
        rec.metadata->SetAttribute(attr);
    }
    auto res = ctx->AddPandasmRecord(std::move(rec));
    if (!res.second) {
        ctx->AddError("duplicate class " + res.first.name);
        return 0;
    }
    return PtrToLong(ctx->Alloc<InterfaceCreator>(&res.first, ctx));
}

EtsString *TypeAPITypeCreatorCtxGetError(EtsLong ctxPtr)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctxPtr);
    return ErrorFromCtx(ctx);
}

EtsObject *TypeAPITypeCreatorCtxGetObjectsArrayForCCtor(EtsLong ctxPtr)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctxPtr);
    auto coro = EtsCoroutine::GetCurrent();
    return ctx->GetObjects(coro)->AsObject();
}

EtsString *TypeAPITypeCreatorCtxClassSetBase(EtsLong classPtr, EtsString *baseTd)
{
    auto creator = PtrFromLong<ClassCreator>(classPtr);
    auto par = GetPandasmTypeFromDescriptor(creator->GetCtx(), baseTd->GetMutf8());
    if (par.GetRank() != 0) {
        return EtsString::CreateFromMUtf8("can't have array base");
    }
    creator->GetRec()->metadata->SetAttributeValue(typeapi_create_consts::ATTR_EXTENDS, par.GetName());
    return nullptr;
}

EtsString *TypeAPITypeCreatorCtxInterfaceAddBase(EtsLong ifacePtr, EtsString *baseTd)
{
    auto creator = PtrFromLong<InterfaceCreator>(ifacePtr);
    auto par = GetPandasmTypeFromDescriptor(creator->GetCtx(), baseTd->GetMutf8());
    if (par.GetRank() != 0) {
        return EtsString::CreateFromMUtf8("can't have array base");
    }
    creator->GetRec()->metadata->SetAttributeValue(typeapi_create_consts::ATTR_IMPLEMENTS, par.GetName());
    return nullptr;
}

EtsString *TypeAPITypeCreatorCtxGetTypeDescFromPointer(EtsLong nptr)
{
    auto creator = PtrFromLong<TypeCreator>(nptr);
    auto type = creator->GetType();
    return EtsString::CreateFromMUtf8(type.GetDescriptor(!type.IsPrimitive()).c_str());
}

EtsString *TypeAPITypeCreatorCtxMethodAddParam(EtsLong methodPtr, EtsString *paramTd, [[maybe_unused]] EtsString *name,
                                               [[maybe_unused]] EtsInt attrs)
{
    // NOTE(kprokopenko): dump meta info
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);
    auto type = GetPandasmTypeFromDescriptor(m->Ctx(), paramTd->GetMutf8());
    m->AddParameter(std::move(type));
    return nullptr;
}

EtsLong TypeAPITypeCreatorCtxMethodCreate(EtsLong containingTypePtr, EtsString *name, EtsInt attrs)
{
    auto klass = PtrFromLong<ClassCreator>(containingTypePtr);
    auto nameStr = klass->GetRec()->name;
    nameStr += ".";
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::CONSTRUCTOR)) {
        nameStr += panda_file::GetCtorName(SourceLanguage::ETS);
    } else {
        if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::GETTER)) {
            nameStr += GETTER_BEGIN;
        } else if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::SETTER)) {
            nameStr += SETTER_BEGIN;
        }
        nameStr += name->GetMutf8();
    }
    auto ret = klass->GetCtx()->Alloc<PandasmMethodCreator>(std::move(nameStr), klass->GetCtx());
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::STATIC)) {
        ret->GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    } else {
        ret->GetFn().params.emplace_back(pandasm::Type {klass->GetRec()->name, 0}, SourceLanguage::ETS);
    }
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::ABSTRACT)) {
        ret->GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_NOIMPL);
    }
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::CONSTRUCTOR)) {
        ret->GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_CTOR);
    }
    return PtrToLong(ret);
}

EtsString *TypeAPITypeCreatorCtxMethodAddAccessMod(EtsLong methodPtr, EtsInt access)
{
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);
    auto accessMod = static_cast<EtsTypeAPIAccessModifier>(access);
    SetAccessFlags(m->GetFn().metadata.get(), typeapi_create_consts::ATTR_ACCESS_FUNCTION, accessMod);
    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAdd(EtsLong methodPtr)
{
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);
    m->Create();
    return ErrorFromCtx(m->Ctx());
}

EtsMethod *GetEtsMethod(ObjectHeader *methodTypeObj)
{
    ASSERT(methodTypeObj != nullptr);
    auto *methodType = EtsTypeAPIType::FromCoreType(methodTypeObj);
    return EtsMethod::FromTypeDescriptor(methodType->GetRuntimeTypeDescriptor()->GetMutf8(),
                                         methodType->GetContextLinker());
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyFromMethod(EtsLong methodPtr, ObjectHeader *methodTypeObj)
{
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);
    auto *meth = GetEtsMethod(methodTypeObj);
    ASSERT(meth != nullptr);
    auto ctx = m->Ctx();

    auto parentMethodClassName = GetPandasmTypeFromDescriptor(m->Ctx(), meth->GetClass()->GetDescriptor());

    // NOTE(kprokopenko): implement type checking

    auto parentMethod = CreateCopiedMethod(ctx, parentMethodClassName.GetName() + ".", meth);
    parentMethod.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    parentMethod.Create();

    m->GetFn().AddInstruction(pandasm::Create_CALL_RANGE(0, parentMethod.GetFunctionName()));
    m->GetFn().AddInstruction(GetReturnStatement(meth->GetReturnValueType()));

    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyFromLambda(EtsLong methodPtr, EtsInt lambdaObjectId,
                                                        ObjectHeader *methodTypeObj)
{
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);
    auto ctx = m->Ctx();

    auto *meth = GetEtsMethod(methodTypeObj);
    if (meth == nullptr) {
        return EtsString::CreateFromMUtf8("method is absent");
    }

    auto *lambdaClass = meth->GetClass();
    ASSERT(lambdaClass != nullptr);
    ASSERT(lambdaClass->IsInitialized());
    const auto *klassTd = lambdaClass->GetDescriptor();
    auto klassName = GetPandasmTypeFromDescriptor(m->Ctx(), klassTd);
    auto fld = m->Ctx()->AddInitField(lambdaObjectId, pandasm::Type {klassName, 0});

    auto externalFn = CreateCopiedMethod(ctx, klassName.GetName() + ".", meth);
    externalFn.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
    externalFn.Create();

    m->GetFn().regsNum = 1U;
    m->GetFn().AddInstruction(pandasm::Create_LDSTATIC_OBJ(fld));
    m->GetFn().AddInstruction(pandasm::Create_STA_OBJ(0));
    if (EtsMethod::ToRuntimeMethod(meth)->IsFinal()) {
        m->GetFn().AddInstruction(pandasm::Create_CALL_RANGE(0, externalFn.GetFunctionName()));
    } else {
        m->GetFn().AddInstruction(pandasm::Create_CALL_VIRT_RANGE(0, externalFn.GetFunctionName()));
    }
    auto returnType = m->GetFn().returnType;
    if (returnType.IsObject() && (returnType.GetName() != typeapi_create_consts::TYPE_OBJECT)) {
        m->GetFn().AddInstruction(pandasm::Create_CHECKCAST(returnType.GetName()));
    }

    m->GetFn().AddInstruction(GetReturnStatement(meth->GetReturnValueType()));

    return ErrorFromCtx(m->Ctx());
}

static LambdaTypeCreator PrepareLambdaTypeCreator(TypeCreatorCtx *ctx)
{
    LambdaTypeCreator lambda {ctx};
    lambda.AddParameter(pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 0});
    lambda.AddParameter(pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 0});
    lambda.AddResult(pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 0});
    lambda.Create();
    return lambda;
}

static constexpr int TMP_REG = 0;
static constexpr int LMB_REG = 0;
static constexpr int RECV_REG = LMB_REG + 1;
static constexpr int ARR_REG = RECV_REG + 1;
static constexpr int ARGS_REG_START = ARR_REG + 1;

static void AddLambdaParamInst(pandasm::Function &fn, TypeCreatorCtx *ctx)
{
    const auto isStatic = fn.metadata->GetAttribute(std::string {typeapi_create_consts::ATTR_STATIC});
    for (size_t i = 0; i < fn.params.size(); i++) {
        // adjust array store index
        const auto &par = fn.params[i];
        if (i != 0 && (i != 1 || isStatic)) {
            fn.AddInstruction(pandasm::Create_INCI(TMP_REG, 1));
        }
        if (par.type.IsObject()) {
            fn.AddInstruction(pandasm::Create_LDA_OBJ(ARGS_REG_START + i));
        } else {
            auto ctor = ctx->DeclarePrimitive(par.type.GetComponentName()).first;
            fn.AddInstruction(pandasm::Create_INITOBJ_SHORT(ARGS_REG_START + i, 0, ctor));
        }
        if (i == 0 && !isStatic) {
            // set recv
            fn.AddInstruction(pandasm::Create_STA_OBJ(RECV_REG));
        } else {
            // set arg
            fn.AddInstruction(pandasm::Create_STARR_OBJ(ARR_REG, TMP_REG));
        }
    }
}

static void AddBoxedTypeCheck(pandasm::Function &fn, const std::string &primTypeName)
{
    std::string objectTypeName {typeapi_create_consts::TYPE_BOXED_PREFIX};
    if (primTypeName == "u1") {
        objectTypeName += "Boolean";
    } else if (primTypeName == "i8") {
        objectTypeName += "Byte";
    } else if (primTypeName == "i16") {
        objectTypeName += "Short";
    } else if (primTypeName == "i32") {
        objectTypeName += "Int";
    } else if (primTypeName == "i64") {
        objectTypeName += "Long";
    } else if (primTypeName == "u16") {
        objectTypeName += "Char";
    } else if (primTypeName == "void") {
        objectTypeName += "Void";
    } else if (primTypeName == "f32") {
        objectTypeName += "Float";
    } else if (primTypeName == "f64") {
        objectTypeName += "Double";
    } else {
        UNREACHABLE();
    }
    fn.AddInstruction(pandasm::Create_CHECKCAST(objectTypeName));
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyFromErasedLambda(EtsLong methodPtr, EtsInt lambdaId)
{
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);

    m->Ctx()->AddRefTypeAsExternal(std::string {typeapi_create_consts::TYPE_OBJECT});

    auto lambda = PrepareLambdaTypeCreator(m->Ctx());

    auto savedLambdaField = m->Ctx()->AddInitField(lambdaId, lambda.GetType());

    auto &fn = m->GetFn();
    auto isStatic = fn.metadata->GetAttribute(std::string {typeapi_create_consts::ATTR_STATIC});
    fn.regsNum = ARGS_REG_START;
    const auto &ret = fn.returnType;
    auto pars = fn.params.size();
    auto arrLen = pars - (isStatic ? 0 : 1);
    fn.AddInstruction(pandasm::Create_MOVI(TMP_REG, arrLen));
    fn.AddInstruction(
        pandasm::Create_NEWARR(ARR_REG, TMP_REG, pandasm::Type {typeapi_create_consts::TYPE_OBJECT, 1}.GetName()));

    fn.AddInstruction(pandasm::Create_MOVI(TMP_REG, 0));

    if (isStatic) {
        fn.AddInstruction(pandasm::Create_LDA_NULL());
        fn.AddInstruction(pandasm::Create_STA_OBJ(RECV_REG));
    }

    AddLambdaParamInst(fn, m->Ctx());

    fn.AddInstruction(pandasm::Create_LDSTATIC_OBJ(savedLambdaField));
    fn.AddInstruction(pandasm::Create_STA_OBJ(LMB_REG));

    fn.AddInstruction(pandasm::Create_CALL_VIRT(LMB_REG, RECV_REG, ARR_REG, 0, lambda.GetFunctionName()));

    if (!fn.returnType.IsObject() && !fn.returnType.IsVoid()) {
        AddBoxedTypeCheck(fn, fn.returnType.GetComponentName());
        fn.AddInstruction(pandasm::Create_STA_OBJ(0));

        auto destr = m->Ctx()->DeclarePrimitive(fn.returnType.GetComponentName()).second;
        fn.AddInstruction(pandasm::Create_CALL_SHORT(0, 0, destr));
    } else if (fn.returnType.IsObject()) {
        fn.AddInstruction(pandasm::Create_CHECKCAST(ret.GetName()));
    }
    auto returnId = fn.returnType.GetId();
    fn.AddInstruction(GetReturnStatement(ConvertPandaTypeToEtsType(panda_file::Type {returnId})));

    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddBodyDefault(EtsLong methodPtr)
{
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);
    auto &fn = m->GetFn();

    // call default constructor in case of constructor
    if (fn.metadata->IsCtor()) {
        auto selfName = m->GetFn().params.front().type.GetName();
        auto &recordTable = m->Ctx()->Program().recordTable;
        auto found = recordTable.find(selfName);
        ASSERT(found != recordTable.end());
        // clang-format off
        auto superName = found->second.metadata->GetAttributeValue(std::string {typeapi_create_consts::ATTR_EXTENDS})
                            .value();
        // clang-format on
        m->Ctx()->AddRefTypeAsExternal(superName);
        PandasmMethodCreator superCtor {superName + "." + panda_file::GetCtorName(panda_file::SourceLang::ETS),
                                        m->Ctx()};
        superCtor.AddParameter(pandasm::Type {superName, 0, true});
        superCtor.GetFn().metadata->SetAttribute(typeapi_create_consts::ATTR_EXTERNAL);
        superCtor.Create();

        fn.AddInstruction(pandasm::Create_CALL_SHORT(0, 0, superCtor.GetFunctionName()));
    }

    const auto &ret = fn.returnType;
    if (ret.IsVoid()) {
        fn.AddInstruction(pandasm::Create_RETURN_VOID());
    } else if (ret.IsObject()) {
        fn.AddInstruction(pandasm::Create_LDA_NULL());
        fn.AddInstruction(pandasm::Create_RETURN_OBJ());
        // return EtsString::CreateFromMUtf8("can't make default return for object type");
    } else if (ret.IsFloat32()) {
        fn.AddInstruction(pandasm::Create_FLDAI(0));
        fn.AddInstruction(pandasm::Create_RETURN());
    } else if (ret.IsFloat64()) {
        fn.AddInstruction(pandasm::Create_FLDAI_64(0));
        fn.AddInstruction(pandasm::Create_RETURN_64());
    } else if (ret.IsPrim64()) {
        fn.AddInstruction(pandasm::Create_LDAI_64(0));
        fn.AddInstruction(pandasm::Create_RETURN_64());
    } else {
        fn.AddInstruction(pandasm::Create_LDAI(0));
        fn.AddInstruction(pandasm::Create_RETURN());
    }

    return ErrorFromCtx(m->Ctx());
}

EtsString *TypeAPITypeCreatorCtxMethodAddResult(EtsLong methodPtr, EtsString *descriptor)
{
    auto m = PtrFromLong<PandasmMethodCreator>(methodPtr);
    m->AddResult(GetPandasmTypeFromDescriptor(m->Ctx(), descriptor->GetMutf8().c_str()));
    return ErrorFromCtx(m->Ctx());
}

EtsLong TypeAPITypeCreatorCtxLambdaTypeCreate(EtsLong ctxPtr, [[maybe_unused]] EtsInt attrs)
{
    auto ctx = PtrFromLong<TypeCreatorCtx>(ctxPtr);
    // NOTE(kprokopenko): add attributes
    auto fn = ctx->Alloc<LambdaTypeCreator>(ctx);
    return PtrToLong(fn);
}

EtsString *TypeAPITypeCreatorCtxLambdaTypeAddParam(EtsLong ftPtr, EtsString *td, [[maybe_unused]] EtsInt attrs)
{
    // NOTE(kprokopenko): dump meta info
    auto creator = PtrFromLong<LambdaTypeCreator>(ftPtr);
    creator->AddParameter(GetPandasmTypeFromDescriptor(creator->GetCtx(), td->GetMutf8()));
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxLambdaTypeAddResult(EtsLong ftPtr, EtsString *td)
{
    auto creator = PtrFromLong<LambdaTypeCreator>(ftPtr);
    creator->AddResult(GetPandasmTypeFromDescriptor(creator->GetCtx(), td->GetMutf8()));
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxLambdaTypeAdd(EtsLong ftPtr)
{
    auto creator = PtrFromLong<LambdaTypeCreator>(ftPtr);
    creator->Create();
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxClassAddIface(EtsLong classPtr, EtsString *descr)
{
    auto creator = PtrFromLong<ClassCreator>(classPtr);
    auto iface = GetPandasmTypeFromDescriptor(creator->GetCtx(), descr->GetMutf8());
    creator->GetRec()->metadata->SetAttributeValue(typeapi_create_consts::ATTR_IMPLEMENTS, iface.GetName());
    return ErrorFromCtx(creator->GetCtx());
}

EtsString *TypeAPITypeCreatorCtxClassAddField(EtsLong classPtr, EtsString *name, EtsString *descr, EtsInt attrs,
                                              EtsInt access)
{
    auto klass = PtrFromLong<ClassCreator>(classPtr);
    auto type = GetPandasmTypeFromDescriptor(klass->GetCtx(), descr->GetMutf8());
    pandasm::Field fld {SourceLanguage::ETS};
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::STATIC)) {
        fld.metadata->SetAttribute(typeapi_create_consts::ATTR_STATIC);
    }
    if (HasFeatureAttribute(attrs, EtsTypeAPIAttributes::READONLY)) {
        fld.metadata->SetAttribute(typeapi_create_consts::ATTR_FINAL);
    }
    fld.name = name->GetMutf8();
    fld.type = pandasm::Type(type, 0);
    auto accessMod = static_cast<EtsTypeAPIAccessModifier>(access);
    SetAccessFlags(fld.metadata.get(), typeapi_create_consts::ATTR_ACCESS_FIELD, accessMod);
    klass->GetRec()->fieldList.emplace_back(std::move(fld));
    return ErrorFromCtx(klass->GetCtx());
}
}

}  // namespace ark::ets::intrinsics
