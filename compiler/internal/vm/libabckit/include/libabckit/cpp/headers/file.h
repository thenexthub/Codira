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

#ifndef CPP_ABCKIT_FILE_H
#define CPP_ABCKIT_FILE_H

#include "base_classes.h"
#include "value.h"
#include "literal.h"
#include "literal_array.h"

#include "core/module.h"
#include "arkts/module.h"
#include "js/module.h"
#include "config.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace abckit {

/**
 * @brief File
 */
class File final : public Resource<AbckitFile *> {
    /// @brief To access private constructor
    friend class abckit::DynamicIsa;
    /// @brief To access private constructor
    friend class abckit::Instruction;

private:
    class FileDeleter final : public IResourceDeleter {
    public:
        FileDeleter(const ApiConfig *conf, const File &file) : conf_(conf), deleterFile_(file) {};
        FileDeleter(const FileDeleter &other) = delete;
        FileDeleter &operator=(const FileDeleter &other) = delete;
        FileDeleter(FileDeleter &&other) = delete;
        FileDeleter &operator=(FileDeleter &&other) = delete;
        ~FileDeleter() override = default;

        void DeleteResource() override
        {
            conf_->cApi_->closeFile(deleterFile_.GetResource());
        }

    private:
        const ApiConfig *conf_;
        const File &deleterFile_;
    };

public:
    /**
     * @brief Constructor
     * @param path
     */
    explicit File(std::string_view path) : File(path, std::make_unique<DefaultErrorHandler>()) {}

    /**
     * @brief Deleted constructor
     * @param file
     */
    File(const File &file) = delete;

    /**
     * @brief Deleted constructor
     * @param file
     * @return File
     */
    File &operator=(const File &file) = delete;

    /**
     * @brief Deleted constructor
     * @param file
     */
    File(File &&file) = delete;

    /**
     * @brief Deleted constructor
     * @param file
     * @return File
     */
    File &operator=(File &&file) = delete;

    /**
     * @brief Constructor
     * @param path
     * @param eh
     */
    File(std::string_view path, std::unique_ptr<IErrorHandler> eh)
        : Resource(AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)->openAbc(path.data(), path.size())),
          conf_(std::move(eh))
    {
        CheckError(&conf_);
        SetDeleter(std::make_unique<FileDeleter>(&conf_, *this));
    }
    /**
     * @brief Destructor
     */
    ~File() override = default;

    /**
     * @brief Creates value item `AbckitValue` containing the given boolean value `value`.
     * @param val
     * @return `Value`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     */
    abckit::Value CreateValueU1(bool val) const;

    /**
     * @brief Creates value item containing the given int value `value`.
     * @param val
     * @return `Value`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     */
    abckit::Value CreateValueInt(int val) const;
    /**
     * @brief Creates value item containing the given double value `value`.
     * @param val
     * @return `Value`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     */
    abckit::Value CreateValueDouble(double val) const;

    /**
     * @brief Creates value item containing the given bool value `value`.
     * @param val
     * @return `Value`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     */
    abckit::Literal CreateLiteralBool(bool val) const;

    /**
     * @brief Creates Literal containing the given byte value `value`.
     * @return `Literal`.
     * @param [ in ] value - Byte value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    abckit::Literal CreateLiteralU8(uint8_t value) const;

    /**
     * @brief Creates Literal containing the given short value `value`.
     * @return `Literal`.
     * @param [ in ] value - Short value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    abckit::Literal CreateLiteralU16(uint16_t value) const;

    /**
     * @brief Creates Literal containing the given integer value `value`.
     * @return `Literal`.
     * @param [ in ] value - Integer value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    abckit::Literal CreateLiteralU32(uint32_t value) const;

    /**
     * @brief Creates Literal containing the given long value `value`.
     * @return `Literal`.
     * @param [ in ] value - Long value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    Literal CreateLiteralU64(uint64_t value) const;

    /**
     * @brief Creates Literal containing the given float value `value`.
     * @return `Literal`.
     * @param [ in ] value - Float value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    Literal CreateLiteralFloat(float value) const;

    /**
     * @brief Creates value item containing the given bool value `value`.
     * @param val
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @return `Value`
     */
    abckit::Literal CreateLiteralDouble(double val) const;

    /**
     * @brief Creates literal containing the given literal array `litarr`.
     * @param val
     * @return `Literal`
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'val' is false.
     */
    abckit::Literal CreateLiteralLiteralArray(const abckit::LiteralArray &val) const;

    /**
     * @brief Creates literal array value item from the given value items array `literals`.
     * @param literals
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if one or more elements from 'literals' are false.
     * @return `LiteralArray`
     */
    abckit::LiteralArray CreateLiteralArray(const std::vector<abckit::Literal> &literals) const;

    /**
     * @brief Creates literal containing the given string `val`.
     * @return `Literal`.
     * @param [ in ] val - string that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    abckit::Literal CreateLiteralString(std::string_view val) const;

    /**
     * @brief Creates literal containing null value.
     * @return `Literal`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    abckit::Literal CreateLiteralNullValue() const;

    /**
     * @brief Creates Literal containing the given function `function`.
     * @return `Literal`.
     * @param [ in ] function - Function that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if 'function' is false.
     * @note Allocates
     */
    Literal CreateLiteralMethod(core::Function &function) const;

    /**
     * @brief Writes `file` to the specified `path`.
     * @param path - path to file
     */
    void WriteAbc(std::string_view path) const
    {
        GetApiConfig()->cApi_->writeAbc(GetResource(), path.data(), path.size());
        CheckError(GetApiConfig());
    }

    /**
     * @brief Returns a vector of core::Module's in a particular file
     * @return vector of `Module`.
     */
    std::vector<core::Module> GetModules() const;

    /**
     * @brief Enumerates modules that are defined in binary file `file`, invoking callback `cb` for each of
     * such modules.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     */
    bool EnumerateModules(const std::function<bool(core::Module)> &cb) const;

    /**
     * @brief Enumerates modules that are defined in other binary file, but are referenced in binary file `file`,
     * invoking callback `cb` for each of such modules.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     */
    bool EnumerateExternalModules(const std::function<bool(core::Module)> &cb) const;

    /**
     * @brief Creates type according to type id `id`.
     * @return `Type`
     * @param [ in ] id - Type id corresponding to the type being created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    Type CreateType(enum AbckitTypeId id) const;

    /**
     * @brief Creates reference type according to class `klass`.
     * @return `Type`
     * @param [ in ] klass - Class from which the type is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is false.
     * @note Allocates
     */
    Type CreateReferenceType(core::Class &klass) const;

    /**
     * @brief Creates value item `Value` containing the given string `value`.
     * @return `Value`.
     * @param [ in ] value - string value from which value item is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    Value CreateValueString(std::string_view value) const;

    /**
     * @brief Creates literal array value item with size `size` from the given value items array `value`.
     * @return `Value`
     * @param [ in ] value - Value items from which literal array item is created.
     * @param [ in ] size - Size of the literal array value item to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if one or more elements from 'value' are false.
     */
    Value CreateLiteralArrayValue(std::vector<Value> &value, size_t size) const;

    /**
     * @brief Creates Literal containing the given оffset of method affiliate `value`.
     * @return `Literal`.
     * @param [ in ] value - Offset of method affiliate.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current File is false.
     * @note Allocates
     */
    Literal CreateLiteralMethodAffiliate(uint16_t value) const;

    /**
     * @brief Creates an external Arkts module with target `ABCKIT_TARGET_ARK_TS_V1` and adds it to the File.
     * @return Newly created Module.
     * @param [ in ] name - Data that is used to create the external module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Allocates
     */
    arkts::Module AddExternalModuleArktsV1(std::string_view name) const;

    /**
     * @brief Creates an external Arkts module with target `ABCKIT_TARGET_ARK_TS_V2` and adds it to the File.
     * @return Newly created Module.
     * @param [ in ] name - Data that is used to create the external module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Allocates
     */
    arkts::Module AddExternalModuleArktsV2(std::string_view name) const;

    /**
     * @brief Creates an external Js module and adds it to the file `file`.
     * @return `js::Module`
     * @param [ in ] name - Data that is used to create the external module.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if view itself is NULL.
     * @note Allocates
     */
    js::Module AddExternalModuleJs(std::string_view name) const;

protected:
    /**
     * @brief Get api config
     * @return ApiConfig
     */
    const ApiConfig *GetApiConfig() const override
    {
        return &conf_;
    }

    /**
     * @brief Struct for using in callbacks
     */
    template <typename D>
    struct Payload {
        /**
         * @brief data
         */
        D data;
        /**
         * @brief config
         */
        const ApiConfig *config;
        /**
         * @brief resource
         */
        const File *resource;
    };

private:
    const ApiConfig conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_FILE_H
