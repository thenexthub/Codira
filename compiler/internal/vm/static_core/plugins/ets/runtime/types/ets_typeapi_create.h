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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_CREATE_H
#define PANDA_PLUGINS_ETS_TYPEAPI_CREATE_H

#include "assembly-function.h"
#include "assembly-program.h"
#include "class_linker_context.h"
#include "ets_typeapi_create_panda_constants.h"
#include "types/ets_type.h"

namespace ark {
template <typename T>
class VMHandle;
}  // namespace ark

namespace ark::coretypes {
class Array;
}  // namespace ark::coretypes

namespace ark::ets {
class EtsCoroutine;
class EtsArray;
}  // namespace ark::ets

namespace ark::ets {
class TypeCreatorCtx {
public:
    pandasm::Program &Program()
    {
        return prog_;
    }

    template <typename T, typename... A>
    T *Alloc(A &&...a)
    {
        auto p = std::make_shared<T>(std::forward<A>(a)...);
        auto ret = p.get();
        datas_.push_back(std::move(p));
        return ret;
    }

    /**
     * Adds field with permanent data used in TypeAPI$CtxData$_
     * also adds intializer into TypeAPI$CtxData$_.cctor that loads that id
     * from
     * @param id index of filed in init array
     * @param type type of that field
     * @returns field id that can be passed into pandasm instructions to access it
     */
    std::string AddInitField(uint32_t id, const pandasm::Type &type);

    /**
     * Adds TypeAPI$CtxData$_ to program if it was used
     * also emplaces necessary cctors
     */
    void FlushTypeAPICtxDataRecordsToProgram();

    /**
     * Saves objects into current context to provide them to TypeAPI$CtxData$_.cctor
     * @param objects must live until end of call to `InitializeCtxDataRecord` and may be changed inside this function
     */
    void SaveObjects(EtsCoroutine *coro, VMHandle<EtsArray> &objects, ClassLinkerContext *ctx);

    EtsArray *GetObjects(EtsCoroutine *coro);

    /// Initializes TypeAPI$CtxData$_
    [[nodiscard]] bool InitializeCtxDataRecord(EtsCoroutine *coro, ClassLinkerContext *ctx, const panda_file::File *pf);

    /**
     * Creates TypeAPI$CtxData record if it wasn't created before
     * @returns TypeAPI$CtxData record
     */
    pandasm::Record &GetTypeAPICtxDataRecord();

    pandasm::Record &AddRefTypeAsExternal(const std::string &name);

    std::pair<pandasm::Record &, bool> AddPandasmRecord(pandasm::Record &&record);

    /**
     * Lazily declares primitive reference wrapper
     * @returns pair of constructor and unwrapper
     */
    const std::pair<std::string, std::string> &DeclarePrimitive(const std::string &primTypeName);

    void AddError(std::string_view err)
    {
        error_ += err;
    }

    std::optional<std::string_view> GetError() const
    {
        if (error_.empty()) {
            return std::nullopt;
        }
        return error_;
    }

private:
    void LoadCreatedClasses(ClassLinkerContext *ctx, const panda_file::File *pf);

private:
    pandasm::Program prog_;
    pandasm::Record ctxDataRecord_ {"", panda_file::SourceLang::ETS};
    std::string ctxDataRecordName_ {};
    pandasm::Function ctxDataRecordCctor_ {"", panda_file::SourceLang::ETS};
    std::map<std::string, std::pair<std::string, std::string>> primitiveTypesCtorDtor_;
    std::vector<std::shared_ptr<void>> datas_;
    std::string error_;
    std::vector<std::string> createdRecords_;

    coretypes::Array *initArrObject_ {};
};

enum class TypeCreatorKind {
    CLASS,
    INTERFACE,
    LAMBDA_TYPE,
};

class TypeCreator {
public:
    virtual TypeCreatorKind GetKind() const = 0;
    virtual pandasm::Type GetType() const = 0;

    explicit TypeCreator(TypeCreatorCtx *ctx) : ctx_(ctx) {}

    TypeCreatorCtx *GetCtx() const
    {
        return ctx_;
    }

private:
    TypeCreatorCtx *ctx_;
};

class ClassCreator final : public TypeCreator {
public:
    explicit ClassCreator(pandasm::Record *rec, TypeCreatorCtx *ctx) : TypeCreator(ctx), rec_(rec) {}

    TypeCreatorKind GetKind() const override
    {
        return TypeCreatorKind::CLASS;
    }

    pandasm::Type GetType() const override
    {
        return pandasm::Type {rec_->name, 0, true};
    }

    pandasm::Record *GetRec() const
    {
        return rec_;
    }

private:
    pandasm::Record *rec_;
};

class InterfaceCreator final : public TypeCreator {
public:
    explicit InterfaceCreator(pandasm::Record *rec, TypeCreatorCtx *ctx) : TypeCreator(ctx), rec_(rec) {}

    TypeCreatorKind GetKind() const override
    {
        return TypeCreatorKind::INTERFACE;
    }

    pandasm::Type GetType() const override
    {
        return pandasm::Type {rec_->name, 0, true};
    }

    pandasm::Record *GetRec() const
    {
        return rec_;
    }

private:
    pandasm::Record *rec_;
};

class LambdaTypeCreator final : public TypeCreator {
public:
    explicit LambdaTypeCreator(TypeCreatorCtx *ctx) : TypeCreator(ctx)
    {
        for (const auto &attr : typeapi_create_consts::ATTR_INTERFACE) {
            rec_.metadata->SetAttribute(attr);
        }
        rec_.metadata->SetAttributeValue(typeapi_create_consts::ATTR_ACCESS,
                                         typeapi_create_consts::ATTR_ACCESS_VAL_PUBLIC);
    }

    TypeCreatorKind GetKind() const override
    {
        return TypeCreatorKind::LAMBDA_TYPE;
    }

    pandasm::Type GetType() const override
    {
        ASSERT(finished_);
        return pandasm::Type {name_, 0};
    }

    const std::string &GetFunctionName() const
    {
        ASSERT(finished_);
        return fnName_;
    }

    void AddParameter(pandasm::Type param);

    void AddResult(const pandasm::Type &type);

    void Create();

private:
    std::string name_ = STD_CORE_FUNCTION_PREFIX;
    std::string fnName_ = STD_CORE_FUNCTION_INVOKE_PREFIX;
    pandasm::Record rec_ {name_, SourceLanguage::ETS};
    pandasm::Function fn_ {fnName_, SourceLanguage::ETS};
    bool finished_ = false;
};

class PandasmMethodCreator {
public:
    PandasmMethodCreator(std::string name, TypeCreatorCtx *ctx)
        : ctx_(ctx), name_(name), fn_(std::move(name), SourceLanguage::ETS)
    {
    }

    void AddParameter(pandasm::Type param);

    void AddResult(pandasm::Type type);

    void Create();

    const std::string &GetFunctionName() const
    {
        ASSERT(finished_);
        return name_;
    }

    pandasm::Function &GetFn()
    {
        ASSERT(!finished_);
        return fn_;
    }

    TypeCreatorCtx *Ctx() const
    {
        return ctx_;
    }

private:
    TypeCreatorCtx *ctx_;
    std::string name_;
    pandasm::Function fn_;
    bool finished_ = false;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_CREATE_H
