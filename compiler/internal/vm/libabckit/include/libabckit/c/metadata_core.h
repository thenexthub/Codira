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

#ifndef LIBABCKIT_METADATA_H
#define LIBABCKIT_METADATA_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif /* __cplusplus */

#include "declarations.h"
#include "api_version.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * This enum is a part of the public API, returned by `ModuleGetTarget`.
 */
enum AbckitTarget {
    /*
     * For cases when user does not have needed language extension.
     */
    ABCKIT_TARGET_UNKNOWN,
    /*
     * For `.abc` files obtained from mainline pipeline from `.ets` files.
     */
    ABCKIT_TARGET_ARK_TS_V1,
    /*
     * Reserved for future versions.
     */
    ABCKIT_TARGET_ARK_TS_V2,
    /*
     * Reserved for future versions.
     */
    ABCKIT_TARGET_TS,
    /*
     * For `.abc` files obtained from `.js` files.
     */
    ABCKIT_TARGET_JS,
    /*
     * Reserved for future versions.
     */
    ABCKIT_TARGET_NATIVE,
    /*
     * NOTE: Extend for other module types.
     */
};

enum AbckitTypeId {
    ABCKIT_TYPE_ID_INVALID,
    ABCKIT_TYPE_ID_VOID,
    ABCKIT_TYPE_ID_U1,
    ABCKIT_TYPE_ID_I8,
    ABCKIT_TYPE_ID_U8,
    ABCKIT_TYPE_ID_I16,
    ABCKIT_TYPE_ID_U16,
    ABCKIT_TYPE_ID_I32,
    ABCKIT_TYPE_ID_U32,
    ABCKIT_TYPE_ID_F32,
    ABCKIT_TYPE_ID_F64,
    ABCKIT_TYPE_ID_I64,
    ABCKIT_TYPE_ID_U64,
    ABCKIT_TYPE_ID_STRING,
    ABCKIT_TYPE_ID_ANY,
    ABCKIT_TYPE_ID_REFERENCE,
    ABCKIT_TYPE_ID_LITERALARRAY
};

/* NOTE: (knazarov) may need to prune this enum in order to only expose user-friendly literal types */
enum AbckitLiteralTag {
    /* common */
    ABCKIT_LITERAL_TAG_INVALID,
    ABCKIT_LITERAL_TAG_TAGVALUE,
    ABCKIT_LITERAL_TAG_BOOL,
    ABCKIT_LITERAL_TAG_BYTE,
    ABCKIT_LITERAL_TAG_SHORT,
    ABCKIT_LITERAL_TAG_INTEGER,
    ABCKIT_LITERAL_TAG_LONG,
    ABCKIT_LITERAL_TAG_FLOAT,
    ABCKIT_LITERAL_TAG_DOUBLE,
    ABCKIT_LITERAL_TAG_STRING,
    ABCKIT_LITERAL_TAG_METHOD,
    ABCKIT_LITERAL_TAG_GENERATORMETHOD,
    ABCKIT_LITERAL_TAG_ACCESSOR,
    ABCKIT_LITERAL_TAG_METHODAFFILIATE,
    ABCKIT_LITERAL_TAG_ARRAY_U1,
    ABCKIT_LITERAL_TAG_ARRAY_U8,
    ABCKIT_LITERAL_TAG_ARRAY_I8,
    ABCKIT_LITERAL_TAG_ARRAY_U16,
    ABCKIT_LITERAL_TAG_ARRAY_I16,
    ABCKIT_LITERAL_TAG_ARRAY_U32,
    ABCKIT_LITERAL_TAG_ARRAY_I32,
    ABCKIT_LITERAL_TAG_ARRAY_U64,
    ABCKIT_LITERAL_TAG_ARRAY_I64,
    ABCKIT_LITERAL_TAG_ARRAY_F32,
    ABCKIT_LITERAL_TAG_ARRAY_F64,
    ABCKIT_LITERAL_TAG_ARRAY_STRING,
    ABCKIT_LITERAL_TAG_ASYNCGENERATORMETHOD,
    ABCKIT_LITERAL_TAG_NULLVALUE,

    /* static */
    ABCKIT_LITERAL_TAG_BIGINT,
    ABCKIT_LITERAL_TAG_ASYNCMETHOD,

    /* dynamic */
    ABCKIT_LITERAL_TAG_LITERALBUFFERINDEX,
    ABCKIT_LITERAL_TAG_LITERALARRAY,
    ABCKIT_LITERAL_TAG_BUILTINTYPEINDEX,
    ABCKIT_LITERAL_TAG_GETTER,
    ABCKIT_LITERAL_TAG_SETTER
};

enum { ABCKIT_VERSION_SIZE = 4 };

/**
 * @brief Struct that holds the pointers to the non-modifying API for core Abckit types.
 */
struct CAPI_EXPORT AbckitInspectApi {
    /* ========================================
     * Language-independent abstractions
     * ======================================== */

    /* ========================================
     * File
     * ======================================== */

    /**
     * @brief Read version of provided binary file.
     * @return Version of the binary file.
     * @param [ in ] file - Binary file to read version from.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     */
    AbckitFileVersion (*fileGetVersion)(AbckitFile *file);

    /**
     * @brief Enumerates modules that are defined in binary file `file`, invoking callback `cb` for each of such
     * modules.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] file - Binary file to enumerate local modules in.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*fileEnumerateModules)(AbckitFile *file, void *data, bool (*cb)(AbckitCoreModule *module, void *data));

    /**
     * @brief Enumerates modules that are defined in other binary file, but are referenced in binary file `file`,
     * invoking callback `cb` for each of such modules.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] file - Binary file to enumerate external modules in.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*fileEnumerateExternalModules)(AbckitFile *file, void *data,
                                         bool (*cb)(AbckitCoreModule *module, void *data));

    /* ========================================
     * String
     * ======================================== */

    /**
     * @brief Converts value of type `AbckitString` to C-style null-terminated string. Returned pointer should not be
     * used to write to the memory.
     * @return C-style string that corresponds to the `value`.
     * @param [ in ] value - Value of type `AbckitString` that needs to be converted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value`  is NULL.
     */
    const char *(*abckitStringToString)(AbckitString *value);

    /* ========================================
     * Type
     * ======================================== */

    /**
     * @brief Returns type id for the given value of type `AbckitType`.
     * @return Type id of the `t` argument.
     * @param [ in ] t - Type to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `t` is NULL.
     */
    enum AbckitTypeId (*typeGetTypeId)(AbckitType *t);

    /**
     * @brief Returns instance of the `AbckitCoreClass` that the type `t` is reference to.
     * @return Pointer to the `AbckitCoreClass` that `t` references.
     * @param [ in ] t - Type to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `t` is NULL.
     */
    AbckitCoreClass *(*typeGetReferenceClass)(AbckitType *t);

    /**
     * @brief Returns type name for the given value of type `AbckitType`.
     * @return Type name of the `t` argument.
     * @param [ in ] t - Type to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `t` is NULL.
     */
    AbckitString *(*typeGetName)(AbckitType *t);

    /**
     * @brief Returns rank of the given type `t`.
     * @return Rank of the `t` argument.
     * @param [ in ] t - Type to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `t` is NULL.
     */
    size_t (*typeGetRank)(AbckitType *t);

    /**
     * @brief Returns boolean value that tells if the given type `t` is a union.
     * @return Boolean value that tells if the given type `t` is a union.
     * @param [ in ] t - Type to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `t` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for dynamic type.
     */
    bool (*typeIsUnion)(AbckitType *t);

    /**
     * @brief Enumerates union types of the given type `t`, invoking callback `cb` for each of it's
     * union types.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] t - Type to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `t` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for dynamic type.
     */
    bool (*typeEnumerateUnionTypes)(AbckitType *t, void *data, bool (*cb)(AbckitType *type, void *data));

    /* ========================================
     * Value
     * ======================================== */

    /**
     * @brief Returns binary file that the given `value` is a part of.
     * @return Pointer to the file that contains given `value`.
     * @param [ in ] value - Value item to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     */
    AbckitFile *(*valueGetFile)(AbckitValue *value);

    /**
     * @brief Returns type that given `value` has.
     * @return Pointer to the `AbckitType` that corresponds to provided `value`.
     * @param [ in ] value - Value item to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     */
    AbckitType *(*valueGetType)(AbckitValue *value);

    /**
     * @brief Returns boolean value that given `value` holds.
     * @return Boolean value that is stored in the `value`.
     * @param [ in ] value - Value item to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` type id differs from `ABCKIT_TYPE_ID_U1`.
     */
    bool (*valueGetU1)(AbckitValue *value);

    /**
     * @brief Returns int value that given `value` holds.
     * @return Int value that is stored in the `value`.
     * @param [ in ] value - Value item to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` type id differs from `ABCKIT_TYPE_ID_INT`.
     */
    int (*valueGetInt)(AbckitValue *value);

    /**
     * @brief Returns double value that given `value` holds.
     * @return Double value that is stored in the `value`.
     * @param [ in ] value - Value item to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` type id differs from `ABCKIT_TYPE_ID_F64`.
     */
    double (*valueGetDouble)(AbckitValue *value);

    /**
     * @brief Returns string value that given `value` holds.
     * @return Pointer to the `AbckitString` value that is stored in the `value`.
     * @param [ in ] value - Value item to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` type id differs from `ABCKIT_TYPE_ID_STRING`.
     * @note Allocates
     */
    AbckitString *(*valueGetString)(AbckitValue *value);

    /**
     * @brief Returns literal array that given `value` holds.
     * @return Pointer to the `AbckitLiteralArray`.
     * @param [ in ] value - Value item to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` type id differs from `ABCKIT_TYPE_ID_LITERALARRAY`.
     */
    AbckitLiteralArray *(*arrayValueGetLiteralArray)(AbckitValue *value);

    /* ========================================
     * Literal
     * ======================================== */

    /**
     * @brief Returns binary file that the given `lit` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     */
    AbckitFile *(*literalGetFile)(AbckitLiteral *lit);

    /**
     * @brief Returns type tag that the given `lit` has.
     * @return Tag of the literal `lit`.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     */
    enum AbckitLiteralTag (*literalGetTag)(AbckitLiteral *lit);

    /**
     * @brief Returns boolean value for `lit`.
     * @return Boolean value of the `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_BOOL`.
     */
    bool (*literalGetBool)(AbckitLiteral *lit);

    /**
     * @brief Returns byte value for `lit`.
     * @return Byte value of the `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_BYTE`,
     * `ABCKIT_LITERAL_TAG_NULLVALUE`, `ABCKIT_LITERAL_TAG_TAGVALUE` or `ABCKIT_LITERAL_TAG_ACCESSOR`.
     */
    uint8_t (*literalGetU8)(AbckitLiteral *lit);
    /**
     * @brief Returns short value for `lit`.
     * @return Short value of the `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_SHORT`.
     */
    uint16_t (*literalGetU16)(AbckitLiteral *lit);

    /**
     * @brief Returns method affiliate value for `lit`.
     * @return uint16_t containing method affiliate оffset that is stored in the `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from
     * `ABCKIT_LITERAL_TAG_METHODAFFILIATE`.
     */
    uint16_t (*literalGetMethodAffiliate)(AbckitLiteral *lit);

    /**
     * @brief Returns integer value for `lit`.
     * @return uint32_t value for `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_INTEGER`.
     */
    uint32_t (*literalGetU32)(AbckitLiteral *lit);

    /**
     * @brief Returns long value that is sotred in `lit` literal.
     * @return uint64_t value for `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_LONG`.
     */
    uint64_t (*literalGetU64)(AbckitLiteral *lit);

    /**
     * @brief Returns float value that is sotred in `lit` literal.
     * @return float value for `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_FLOAT`.
     */
    float (*literalGetFloat)(AbckitLiteral *lit);

    /**
     * @brief Returns double value that is sotred in `lit` literal.
     * @return double value for `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_DOUBLE`.
     */
    double (*literalGetDouble)(AbckitLiteral *lit);

    /**
     * @brief Returns literal array that is stored insode literal `lit`.
     * @return Pointer to the `AbckitLiteralArray` that holds the literal array.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_LITERALARRAY`.
     */
    AbckitLiteralArray *(*literalGetLiteralArray)(AbckitLiteral *lit);

    /**
     * @brief Returns string value for `lit`.
     * @return Pointer to the `AbckitString` that holds the string value of the `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_STRING`.
     * @note Allocates
     */
    AbckitString *(*literalGetString)(AbckitLiteral *lit);

    /**
     * @brief Returns method descriptor that is stored in `lit` literal.
     * @return Pointer to the `AbckitString` that holds the string value of the `lit` literal.
     * @param [ in ] lit - Literal to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `lit` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_LITERAL_TYPE` error if `lit` tag differs from `ABCKIT_LITERAL_TAG_STRING`.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for literal emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     * @note Allocates
     */
    AbckitString *(*literalGetMethod)(AbckitLiteral *lit);

    /* ========================================
     * LiteralArray
     * ======================================== */

    /**
     * @brief Enumerates elements of the literal array `litArr`, invoking callback `cb` for each of it's
     * elements.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] litArr - Literal array to enumerate elements in.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `litArr` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for literal emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    bool (*literalArrayEnumerateElements)(AbckitLiteralArray *litArr, void *data,
                                          bool (*cb)(AbckitFile *file, AbckitLiteral *lit, void *data));

    /* ========================================
     * Language-dependent abstractions
     * ======================================== */

    /* ========================================
     * Module
     * ======================================== */

    /**
     * @brief Returns binary file that the given module `m` is a part of.
     * @return Pointer to the `AbckitFile` that contains given module `m`.
     * @param [ in ] m - Module to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     */
    AbckitFile *(*moduleGetFile)(AbckitCoreModule *m);

    /**
     * @brief Returns the target that the given module `m` was compiled for.
     * @return Target of the current module.
     * @param [ in ] m - Module to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     */
    enum AbckitTarget (*moduleGetTarget)(AbckitCoreModule *m);

    /**
     * @brief Returns the name of the given module `m`.
     * @return Pointer to the `AbckitString` that contains the name of the module.
     * @param [ in ] m - Module to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Allocates
     */
    AbckitString *(*moduleGetName)(AbckitCoreModule *m);

    /**
     * @brief Tells if module is defined in the same binary or externally in another binary.
     * @return Returns `true` if given module `m` is defined in another binary and `false` if defined locally.
     * @param [ in ] m - Module to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     */
    bool (*moduleIsExternal)(AbckitCoreModule *m);

    /**
     * @brief Enumerates imports of the module `m`, invoking callback `cb` for each import.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for module not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    bool (*moduleEnumerateImports)(AbckitCoreModule *m, void *data,
                                   bool (*cb)(AbckitCoreImportDescriptor *i, void *data));

    /**
     * @brief Enumerates exports of the module `m`, invoking callback `cb` for each export.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for module not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    bool (*moduleEnumerateExports)(AbckitCoreModule *m, void *data,
                                   bool (*cb)(AbckitCoreExportDescriptor *i, void *data));

    /**
     * @brief Enumerates namespaces of the module `m`, invoking callback `cb` for each namespace.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*moduleEnumerateNamespaces)(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreNamespace *n, void *data));

    /**
     * @brief Enumerates classes of the module `m`, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*moduleEnumerateClasses)(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreClass *klass, void *data));

    /**
     * @brief Enumerates interfaces of the module `m`, invoking callback `cb` for each interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for module not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*moduleEnumerateInterfaces)(AbckitCoreModule *m, void *data,
                                      bool (*cb)(AbckitCoreInterface *iface, void *data));

    /**
     * @brief Enumerates enums of the module `m`, invoking callback `cb` for each enum.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for module not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*moduleEnumerateEnums)(AbckitCoreModule *m, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data));

    /**
     * @brief Enumerates fields of the module `m`, invoking callback `cb` for each field.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for module not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*moduleEnumerateFields)(AbckitCoreModule *m, void *data,
                                  bool (*cb)(AbckitCoreModuleField *field, void *data));

    /**
     * @brief Enumerates top level functions of the module `m`, invoking callback `cb` for each top level function.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*moduleEnumerateTopLevelFunctions)(AbckitCoreModule *m, void *data,
                                             bool (*cb)(AbckitCoreFunction *function, void *data));

    /**
     * @brief Enumerates anonymous functions of the module `m`, invoking callback `cb` for each anonymous function.
     * @return`false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*moduleEnumerateAnonymousFunctions)(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreFunction *function, void *data));

    /**
     * @brief Enumerates annotation interfaces of the module `m`, invoking callback `cb` for each annotation interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] m - Module to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `m` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*moduleEnumerateAnnotationInterfaces)(AbckitCoreModule *m, void *data,
                                                bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

    /* ========================================
     * Namespace
     * ======================================== */

    /**
     * @brief Returns owning module for namespace `n`.
     * @return Pointer to the `AbckitCoreModule`.
     * @param [ in ] n - Namespace to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     */
    AbckitCoreModule *(*namespaceGetModule)(AbckitCoreNamespace *n);

    /**
     * @brief Returns name of the namespace `n`.
     * @return Pointer to the `AbckitString` containig the name of the namespace.
     * @param [ in ] n - Namespace to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Allocates
     */
    AbckitString *(*namespaceGetName)(AbckitCoreNamespace *n);

    /**
     * @brief Tells if namespace is defined in the same binary or externally in another binary.
     * @return Returns `true` if given namespace `ns` is defined in another binary and `false` if defined locally.
     * @param [ in ] ns - Namespace to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ns` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for namespaces not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*namespaceIsExternal)(AbckitCoreNamespace *ns);

    /**
     * @brief Returns parent namespace for namespace `n`.
     * @return Pointer to the `AbckitCoreNamespace` or NULL if `n` has no parent namespace.
     * @param [ in ] n - Namespace to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     */
    AbckitCoreNamespace *(*namespaceGetParentNamespace)(AbckitCoreNamespace *n);

    /**
     * @brief Enumerates namespaces defined inside of the namespace `n`, invoking callback `cb` for each inner
     * namespace.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] n - Namespace to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*namespaceEnumerateNamespaces)(AbckitCoreNamespace *n, void *data,
                                         bool (*cb)(AbckitCoreNamespace *klass, void *data));

    /**
     * @brief Enumerates classes of the namespace `n`, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] n - Namespace to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*namespaceEnumerateClasses)(AbckitCoreNamespace *n, void *data,
                                      bool (*cb)(AbckitCoreClass *klass, void *data));

    /**
     * @brief Enumerates interfaces of the namespace `n`, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] n - Namespace to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for namespaces not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*namespaceEnumerateInterfaces)(AbckitCoreNamespace *n, void *data,
                                         bool (*cb)(AbckitCoreInterface *iface, void *data));

    /**
     * @brief Enumerates enums of the namespace `n`, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] n - Namespace to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for namespaces not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*namespaceEnumerateEnums)(AbckitCoreNamespace *n, void *data, bool (*cb)(AbckitCoreEnum *enm, void *data));

    /**
     * @brief Enumerates classes of the namespace `n`, invoking callback `cb` for each class.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] n - Namespace to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for namespaces not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*namespaceEnumerateFields)(AbckitCoreNamespace *n, void *data,
                                     bool (*cb)(AbckitCoreNamespaceField *field, void *data));

    /**
     * @brief Enumerates top level functions of the namespace `n`, invoking callback `cb` for each top level function.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] n - Namespace to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*namespaceEnumerateTopLevelFunctions)(AbckitCoreNamespace *n, void *data,
                                                bool (*cb)(AbckitCoreFunction *function, void *data));

    /**
     * @brief Enumerates annotation interfaces of the namespace `n`, invoking callback `cb` for each annotation
     * interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] n - Namespace to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `n` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for namespaces not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*namespaceEnumerateAnnotationInterfaces)(AbckitCoreNamespace *n, void *data,
                                                   bool (*cb)(AbckitCoreAnnotationInterface *ai, void *data));

    /* ========================================
     * ImportDescriptor
     * ======================================== */

    /**
     * @brief Returns binary file that the given import descriptor `i` is a part of.
     * @return Pointer to the `AbckitFile` that the import `i` is a part of.
     * @param [ in ] i - Import descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `i` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for import descriptor `i` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    AbckitFile *(*importDescriptorGetFile)(AbckitCoreImportDescriptor *i);

    /**
     * @brief Returns importing module for import descriptor `i`.
     * @return Pointer to the `AbckitCoreModule` that holds the module that the import `i` is a part of.
     * @param [ in ] i - Import descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `i` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for import descriptor `i` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    AbckitCoreModule *(*importDescriptorGetImportingModule)(AbckitCoreImportDescriptor *i);

    /**
     * @brief Returns imported module for import descriptor `i`.
     * @return Pointer to the `AbckitCoreModule` that holds the module that the import `i` imports from.
     * @param [ in ] i - Import descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `i` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for import descriptor `i` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    AbckitCoreModule *(*importDescriptorGetImportedModule)(AbckitCoreImportDescriptor *i);

    /**
     * @brief Returns name of the import descriptor `i`.
     * @return Pointer to the `AbckitString` that holds the name of the imported entity.
     * @param [ in ] i - Import descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `i` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for import descriptor `i` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     * @note Allocates
     */
    AbckitString *(*importDescriptorGetName)(AbckitCoreImportDescriptor *i);

    /**
     * @brief Returns alias for import descriptor `i`.
     * @return Pointer to the `AbckitString` that holds the alias of the imported entity.
     * @param [ in ] i - Import descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `i` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for import descriptor `i` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     * @note Allocates
     */
    AbckitString *(*importDescriptorGetAlias)(AbckitCoreImportDescriptor *i);

    /* ========================================
     * ExportDescriptor
     * ======================================== */

    /**
     * @brief Returns binary file that the given export descriptor `e` is a part of.
     * @return Pointer to the `AbckitFile` that the export `e` is a part of.
     * @param [ in ] e - Export descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `e` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for export descriptor `e` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    AbckitFile *(*exportDescriptorGetFile)(AbckitCoreExportDescriptor *e);

    /**
     * @brief Returns exporting module for export descriptor `e`.
     * @return Pointer to the `AbckitCoreModule` that holds the module that the export `e` is a part of.
     * @param [ in ] e - Export descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `e` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for export descriptor `e` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    AbckitCoreModule *(*exportDescriptorGetExportingModule)(AbckitCoreExportDescriptor *e);

    /**
     * @brief Returns exporting module for export descriptor `e`.
     * @return Pointer to the `AbckitCoreModule` that holds the module that the export `e` exports from. For local
     * entity export equals to the exporting module. For re-exports may be different.
     * @param [ in ] e - Export descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `e` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for export descriptor `e` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     */
    AbckitCoreModule *(*exportDescriptorGetExportedModule)(AbckitCoreExportDescriptor *e);

    /**
     * @brief Returns name for export descriptor `e`.
     * @return Pointer to the `AbckitString` that holds the name of the exported entity.
     * @param [ in ] e - Export descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `e` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for export descriptor `i` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     * @note Allocates
     */
    AbckitString *(*exportDescriptorGetName)(AbckitCoreExportDescriptor *e);

    /**
     * @brief Returns alias for export descriptor `e`.
     * @return Pointer to the `AbckitString` that holds the alias of the exported entity.
     * @param [ in ] e - Export descriptor to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `e` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for export descriptor `i` emitted not for target
     * `ABCKIT_TARGET_ARK_TS_V1` or `ABCKIT_TARGET_JS`.
     * @note Allocates
     */
    AbckitString *(*exportDescriptorGetAlias)(AbckitCoreExportDescriptor *e);

    /* ========================================
     * Class
     * ======================================== */

    /**
     * @brief Returns binary file that the given class `klass` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     */
    AbckitFile *(*classGetFile)(AbckitCoreClass *klass);

    /**
     * @brief Returns module for class `klass`.
     * @return Pointer to the `AbckitCoreModule`.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     */
    AbckitCoreModule *(*classGetModule)(AbckitCoreClass *klass);

    /**
     * @brief Returns name for class `klass`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Allocates
     */
    AbckitString *(*classGetName)(AbckitCoreClass *klass);

    /**
     * @brief Tells if class is defined in the same binary or externally in another binary.
     * @return Returns `true` if given class `klass` is defined in another binary and `false` if defined locally.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classIsExternal)(AbckitCoreClass *klass);

    /**
     * @brief Tells if class `klass` is final.
     * @return Returns `true` if given class `klass` is final and `false` otherwise.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classIsFinal)(AbckitCoreClass *klass);

    /**
     * @brief Tells if class `klass` is abstract.
     * @return Returns `true` if given class `klass` is abstract and `false` otherwise.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classIsAbstract)(AbckitCoreClass *klass);

    /**
     * @brief Returns parent function for class `klass`.
     * @return Pointer to the `AbckitCoreFunction`.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     */
    AbckitCoreFunction *(*classGetParentFunction)(AbckitCoreClass *klass);

    /**
     * @brief Returns parent namespace for class `klass`.
     * @return Pointer to the `AbckitCoreNamespace`.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     */
    AbckitCoreNamespace *(*classGetParentNamespace)(AbckitCoreClass *klass);

    /**
     * @brief Returns super classes of class `klass`.
     * @return Pointer to the `AbckitCoreClass`.
     * @param [ in ] klass - Class to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     */
    AbckitCoreClass *(*classGetSuperClass)(AbckitCoreClass *klass);

    /**
     * @brief Enumerates methods of class `klass`, invoking callback `cb` for each method.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*classEnumerateMethods)(AbckitCoreClass *klass, void *data,
                                  bool (*cb)(AbckitCoreFunction *method, void *data));

    /**
     * @brief Enumerates annotations of class `klass`, invoking callback `cb` for each annotation.
     * @return false` if was early exited. Otherwise - `true`.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*classEnumerateAnnotations)(AbckitCoreClass *klass, void *data,
                                      bool (*cb)(AbckitCoreAnnotation *anno, void *data));

    /**
     * @brief Enumerates subclasses of class `klass`, invoking callback `cb` for each interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classEnumerateSubClasses)(AbckitCoreClass *klass, void *data,
                                     bool (*cb)(AbckitCoreClass *subClass, void *data));

    /**
     * @brief Enumerates interfaces that class `klass` implements, invoking callback `cb` for each interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classEnumerateInterfaces)(AbckitCoreClass *klass, void *data,
                                     bool (*cb)(AbckitCoreInterface *iface, void *data));

    /**
     * @brief Enumerates fields of class `klass`, invoking callback `cb` for each field.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] klass - Class to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classEnumerateFields)(AbckitCoreClass *klass, void *data,
                                 bool (*cb)(AbckitCoreClassField *field, void *data));

    /* ========================================
     * Interface
     * ======================================== */

    /**
     * @brief Returns binary file that the given interface `iface` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] iface - Interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     */
    AbckitFile *(*interfaceGetFile)(AbckitCoreInterface *iface);

    /**
     * @brief Returns owning module for interface `iface`.
     * @return Pointer to the `AbckitCoreModule`.
     * @param [ in ] iface - Interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     */
    AbckitCoreModule *(*interfaceGetModule)(AbckitCoreInterface *iface);

    /**
     * @brief Returns name for interface `iface`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] iface - Interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     * @note Allocates
     */
    AbckitString *(*interfaceGetName)(AbckitCoreInterface *iface);

    /**
     * @brief Tells if interface is defined in the same binary or externally in another binary.
     * @return Returns `true` if given interface `iface` is defined in another binary and `false` if defined locally.
     * @param [ in ] iface - Interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*interfaceIsExternal)(AbckitCoreInterface *iface);

    /**
     * @brief Returns parent namespace for interface `iface`.
     * @return Pointer to the `AbckitCoreNamespace`.
     * @param [ in ] iface - Interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     */
    AbckitCoreNamespace *(*interfaceGetParentNamespace)(AbckitCoreInterface *iface);

    /**
     * @brief Enumerates super interfaces of interface `iface`, invoking callback `cb` for each super interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*interfaceEnumerateSuperInterfaces)(AbckitCoreInterface *iface, void *data,
                                              bool (*cb)(AbckitCoreInterface *iface, void *data));

    /**
     * @brief Enumerates sub interfaces of interface `iface`, invoking callback `cb` for each super interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*interfaceEnumerateSubInterfaces)(AbckitCoreInterface *iface, void *data,
                                            bool (*cb)(AbckitCoreInterface *iface, void *data));

    /**
     * @brief Enumerates classes that implement interface `iface`, invoking callback `cb` for each super interface.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*interfaceEnumerateClasses)(AbckitCoreInterface *iface, void *data,
                                      bool (*cb)(AbckitCoreClass *klass, void *data));

    /**
     * @brief Enumerates methods of interface `iface`, invoking callback `cb` for each method.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*interfaceEnumerateMethods)(AbckitCoreInterface *iface, void *data,
                                      bool (*cb)(AbckitCoreFunction *func, void *data));

    /**
     * @brief Enumerates annotations of interface `iface`, invoking callback `cb` for each annotation.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*interfaceEnumerateAnnotations)(AbckitCoreInterface *iface, void *data,
                                          bool (*cb)(AbckitCoreAnnotation *anno, void *data));

    /**
     * @brief Enumerates fields of interface `iface`, invoking callback `cb` for each field.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] iface - Interface to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*interfaceEnumerateFields)(AbckitCoreInterface *iface, void *data,
                                     bool (*cb)(AbckitCoreInterfaceField *field, void *data));

    /* ========================================
     * Enum
     * ======================================== */

    /**
     * @brief Returns binary file that the given enum `enm` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] enm - Enum to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     */
    AbckitFile *(*enumGetFile)(AbckitCoreEnum *enm);

    /**
     * @brief Returns owning module for enum `enm`.
     * @return Pointer to the `AbckitCoreModule`.
     * @param [ in ] enm - Enum to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     */
    AbckitCoreModule *(*enumGetModule)(AbckitCoreEnum *enm);

    /**
     * @brief Returns name for enum `enm`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] enm - Enum to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for enum not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     * @note Allocates
     */
    AbckitString *(*enumGetName)(AbckitCoreEnum *enm);

    /**
     * @brief Tells if enum is defined in the same binary or externally in another binary.
     * @return Returns `true` if given class `enm` is defined in another binary and `false` if defined locally.
     * @param [ in ] enm - Enum to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for enum not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*enumIsExternal)(AbckitCoreEnum *enm);

    /**
     * @brief Returns parent namespace for interface `iface`.
     * @return Pointer to the `AbckitCoreNamespace`.
     * @param [ in ] iface - Interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `iface` is NULL.
     */
    AbckitCoreNamespace *(*enumGetParentNamespace)(AbckitCoreEnum *iface);

    /**
     * @brief Enumerates methods of enum `enm`, invoking callback `cb` for each method.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] enm - Enum to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for enum not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*enumEnumerateMethods)(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreFunction *func, void *data));

    /**
     * @brief Enumerates fields of enum `enm`, invoking callback `cb` for each field.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] enm - Enum to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `enm` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for enum not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*enumEnumerateFields)(AbckitCoreEnum *enm, void *data, bool (*cb)(AbckitCoreEnumField *field, void *data));

    /* ========================================
     * Module Field
     * ======================================== */

    /**
     * @brief Returns module for module field `field`.
     * @return Pointer to the `AbckitCoreModule`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitCoreModule *(*moduleFieldGetModule)(AbckitCoreModuleField *field);

    /**
     * @brief Returns name for module field `field`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for module not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    AbckitString *(*moduleFieldGetName)(AbckitCoreModuleField *field);

    /**
     * @brief Returns type for module field `field`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitType *(*moduleFieldGetType)(AbckitCoreModuleField *field);

    /**
     * @brief Returns value for module field `field`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitValue *(*moduleFieldGetValue)(AbckitCoreModuleField *field);

    /* ========================================
     * Namespace Field
     * ======================================== */

    /**
     * @brief Returns namespace for namespace field `field`.
     * @return Pointer to the `AbckitCoreNamespace`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitCoreNamespace *(*namespaceFieldGetNamespace)(AbckitCoreNamespaceField *field);

    /**
     * @brief Returns name for namespace field `field`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for namespace not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    AbckitString *(*namespaceFieldGetName)(AbckitCoreNamespaceField *field);

    /**
     * @brief Returns type for namespace field `field`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for namespace not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    AbckitType *(*namespaceFieldGetType)(AbckitCoreNamespaceField *field);

    /* ========================================
     * Class Field
     * ======================================== */

    /**
     * @brief Returns class for class field `field`.
     * @return Pointer to the `AbckitCoreClass`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitCoreClass *(*classFieldGetClass)(AbckitCoreClassField *field);

    /**
     * @brief Returns name for class field `field`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    AbckitString *(*classFieldGetName)(AbckitCoreClassField *field);

    /**
     * @brief Returns type for class field `field`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitType *(*classFieldGetType)(AbckitCoreClassField *field);

    /**
     * @brief Returns value for class field `field`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    AbckitValue *(*classFieldGetValue)(AbckitCoreClassField *field);

    /**
     * @brief Returns whether class field `field` is public.
     * @return `true` if field `field` is public.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classFieldIsPublic)(AbckitCoreClassField *field);

    /**
     * @brief Returns whether class field `field` is protected.
     * @return `true` if field `field` is protected.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classFieldIsProtected)(AbckitCoreClassField *field);

    /**
     * @brief Returns whether class field `field` is private.
     * @return `true` if field `field` is private.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classFieldIsPrivate)(AbckitCoreClassField *field);

    /**
     * @brief Returns whether class field `field` is internal.
     * @return `true` if field `field` is internal.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classFieldIsInternal)(AbckitCoreClassField *field);

    /**
     * @brief Returns whether class field `field` is static.
     * @return `true` if field `field` is static.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classFieldIsStatic)(AbckitCoreClassField *field);

    /**
     * @brief Returns whether class field `field` is final.
     * @return `true` if field `field` is final.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    bool (*classFieldIsFinal)(AbckitCoreClassField *field);

    /**
     * @brief Enumerates annotations of class field `field`, invoking callback `cb` for each annotation.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] field - Field to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for class not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*classFieldEnumerateAnnotations)(AbckitCoreClassField *field, void *data,
                                           bool (*cb)(AbckitCoreAnnotation *anno, void *data));

    /* ========================================
     * Interface Field
     * ======================================== */

    /**
     * @brief Returns interface for interface field `field`.
     * @return Pointer to the `AbckitCoreInterface`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitCoreInterface *(*interfaceFieldGetInterface)(AbckitCoreInterfaceField *field);

    /**
     * @brief Returns name for interface field `field`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitString *(*interfaceFieldGetName)(AbckitCoreInterfaceField *field);

    /**
     * @brief Returns type for interface field `field`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitType *(*interfaceFieldGetType)(AbckitCoreInterfaceField *field);

    /**
     * @brief Returns whether interface field `field` is readonly.
     * @return `true` if field `field` is readonly.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for interface not for target
     */
    bool (*interfaceFieldIsReadonly)(AbckitCoreInterfaceField *field);

    /**
     * @brief Enumerates annotations of interface field `field`, invoking callback `cb` for each annotation.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] field - Field to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*interfaceFieldEnumerateAnnotations)(AbckitCoreInterfaceField *field, void *data,
                                               bool (*cb)(AbckitCoreAnnotation *anno, void *data));

    /* ========================================
     * Enum Field
     * ======================================== */

    /**
     * @brief Returns enum for enum field `field`.
     * @return Pointer to the `AbckitCoreEnum`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitCoreEnum *(*enumFieldGetEnum)(AbckitCoreEnumField *field);

    /**
     * @brief Returns name for enum field `field`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for enum not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    AbckitString *(*enumFieldGetName)(AbckitCoreEnumField *field);

    /**
     * @brief Returns type for enum field `field`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitType *(*enumFieldGetType)(AbckitCoreEnumField *field);

    /**
     * @brief Returns value for enum field `field`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] field - Field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `field` is NULL.
     */
    AbckitValue *(*enumFieldGetValue)(AbckitCoreEnumField *field);

    /* ========================================
     * Function
     * ======================================== */

    /**
     * @brief Returns binary file that the given function `function` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     */
    AbckitFile *(*functionGetFile)(AbckitCoreFunction *function);

    /**
     * @brief Returns module for function `function`.
     * @return Pointer to the `AbckitCoreModule`.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     */
    AbckitCoreModule *(*functionGetModule)(AbckitCoreFunction *function);

    /**
     * @brief Returns name for function `function`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Allocates
     */
    AbckitString *(*functionGetName)(AbckitCoreFunction *function);

    /**
     * @brief Returns parent function for function `function`.
     * @return Pointer to the `AbckitCoreClass`.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V1`.
     */
    AbckitCoreFunction *(*functionGetParentFunction)(AbckitCoreFunction *function);

    /**
     * @brief Returns parent class for function `function`.
     * @return Pointer to the `AbckitCoreClass`.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V1`.
     */
    AbckitCoreClass *(*functionGetParentClass)(AbckitCoreFunction *function);

    /**
     * @brief Returns parent namespace for function `function`.
     * @return Pointer to the `AbckitCoreNamespace`.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V1`.
     */
    AbckitCoreNamespace *(*functionGetParentNamespace)(AbckitCoreFunction *function);

    /**
     * @brief Enumerates nested functions of function `func`, invoking callback `cb` for each nested function.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] func - Function to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*functionEnumerateNestedFunctions)(AbckitCoreFunction *func, void *data,
                                             bool (*cb)(AbckitCoreFunction *nestedFunc, void *data));

    /**
     * @brief Enumerates nested classes of function `func`, invoking callback `cb` for each nested class.
     * @return false` if was early exited. Otherwise - `true`.
     * @param [ in ] func - Function to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*functionEnumerateNestedClasses)(AbckitCoreFunction *func, void *data,
                                           bool (*cb)(AbckitCoreClass *nestedClass, void *data));

    /**
     * @brief Enumerates annotations of function `func`, invoking callback `cb` for each annotation.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] func - Function to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*functionEnumerateAnnotations)(AbckitCoreFunction *func, void *data,
                                         bool (*cb)(AbckitCoreAnnotation *anno, void *data));

    /**
     * @brief Creates graph from function `function`.
     * @return Pointer to the `AbckitGraph`.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Allocates
     */
    AbckitGraph *(*createGraphFromFunction)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is static.
     * @return Returns `true` if given function `function` is static and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     */
    bool (*functionIsStatic)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is constructor.
     * @return Returns `true` if given function `function` is constructor and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     */
    bool (*functionIsCtor)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is static constructor.
     * @return Returns `true` if given function `function` is static constructor and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionIsCctor)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is anonymous.
     * @return Returns `true` if given function `function` is anonymous and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionIsAnonymous)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is public.
     * @return Returns `true` if given function `function` is public and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionIsPublic)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is protected.
     * @return Returns `true` if given function `function` is protected and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionIsProtected)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is private.
     * @return Returns `true` if given function `function` is private and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionIsPrivate)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is internal.
     * @return Returns `true` if given function `function` is internal and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionIsInternal)(AbckitCoreFunction *function);

    /**
     * @brief Tells if function `function` is external.
     * @return Returns `true` if given function `function` is external and `false` otherwise.
     * @param [ in ] function - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionIsExternal)(AbckitCoreFunction *function);

    /**
     * @brief Enumerates parameters of function `func`, invoking callback `cb` for each parameter.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] func - Function to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    bool (*functionEnumerateParameters)(AbckitCoreFunction *func, void *data,
                                        bool (*cb)(AbckitCoreFunctionParam *param, void *data));

    /**
     * @brief Returns return type for function `func`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] func - Function to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `func` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for function not for target
     * `ABCKIT_TARGET_ARK_TS_V2`.
     */
    AbckitType *(*functionGetReturnType)(AbckitCoreFunction *func);

    /* ========================================
     * FunctionParam
     * ======================================== */

    /**
     * @brief Returns  type for function param `param`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] param - AbckitCoreFunctionParam to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `param` is NULL.
     */
    AbckitType *(*functionParamGetType)(AbckitCoreFunctionParam *param);

    /* ========================================
     * Annotation
     * ======================================== */

    /**
     * @brief Returns binary file that the given annotation `anno` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] anno - Annotation to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     */
    AbckitFile *(*annotationGetFile)(AbckitCoreAnnotation *anno);

    /**
     * @brief Returns name for Annotation `anno`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] anno - Annotation to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Allocates
     */
    AbckitString *(*annotationGetName)(AbckitCoreAnnotation *anno);

    /**
     * @brief Returns interface for annotation `anno`.
     * @return Pointer to the `AbckitCoreAnnotationInterface`.
     * @param [ in ] anno - Annotation to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     */
    AbckitCoreAnnotationInterface *(*annotationGetInterface)(AbckitCoreAnnotation *anno);

    /**
     * @brief Tell if annotation `anno` is external.
     * @return Return true if anno is external otherwise false.
     * @param [ in ] anno - Annotation to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     */
    bool (*annotationIsExternal)(AbckitCoreAnnotation *anno);

    /**
     * @brief Enumerates elements of the annotation `anno`, invoking the callback for each element.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] anno - Annotation to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*annotationEnumerateElements)(AbckitCoreAnnotation *anno, void *data,
                                        bool (*cb)(AbckitCoreAnnotationElement *ae, void *data));

    /* ========================================
     * AnnotationElement
     * ======================================== */

    /**
     * @brief Returns binary file that the given annotation element `ae` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] ae - Annotation element to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ae` is NULL.
     */
    AbckitFile *(*annotationElementGetFile)(AbckitCoreAnnotationElement *ae);

    /**
     * @brief Returns annotation for annotation element `ae`.
     * @return Pointer to the `AbckitCoreAnnotation`.
     * @param [ in ] ae - Annotation element to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ae` is NULL.
     */
    AbckitCoreAnnotation *(*annotationElementGetAnnotation)(AbckitCoreAnnotationElement *ae);

    /**
     * @brief Returns name for annotation element `ae`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] ae - Annotation element to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ae` is NULL.
     * @note Allocates
     */
    AbckitString *(*annotationElementGetName)(AbckitCoreAnnotationElement *ae);

    /**
     * @brief Returns value for annotation element `ae`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] ae - Annotation element to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ae` is NULL.
     */
    AbckitValue *(*annotationElementGetValue)(AbckitCoreAnnotationElement *ae);

    /* ========================================
     * AnnotationInterface
     * ======================================== */

    /**
     * @brief Returns binary file that the given annotation interface `ai` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] ai - Annotation interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     */
    AbckitFile *(*annotationInterfaceGetFile)(AbckitCoreAnnotationInterface *ai);

    /**
     * @brief Returns module for annotation interface `ai`.
     * @return Pointer to the `AbckitCoreModule`.
     * @param [ in ] ai - Annotation interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     */
    AbckitCoreModule *(*annotationInterfaceGetModule)(AbckitCoreAnnotationInterface *ai);

    /**
     * @brief Returns parent namespace for AnnotationInterface `ai`.
     * @return Pointer to the `AbckitCoreNamespace`.
     * @param [ in ] ai - AnnotationInterface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     */
    AbckitCoreNamespace *(*annotationInterfaceGetParentNamespace)(AbckitCoreAnnotationInterface *ai);

    /**
     * @brief Returns name for annotation interface `ai`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] ai - Annotation interface to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     * @note Allocates
     */
    AbckitString *(*annotationInterfaceGetName)(AbckitCoreAnnotationInterface *ai);

    /**
     * @brief Enumerates fields of annotation interface `ai`, invoking callback `cb` for each field.
     * @return `false` if was early exited. Otherwise - `true`.
     * @param [ in ] ai - Annotation interface to be inspected.
     * @param [ in, out ] data - Pointer to the user-defined data that will be passed to the callback `cb` each time
     * it is invoked.
     * @param [ in ] cb - Callback that will be invoked. Should return `false` on early exit and `true` when iterations
     * should continue.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ai` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cb` is NULL.
     */
    bool (*annotationInterfaceEnumerateFields)(AbckitCoreAnnotationInterface *ai, void *data,

                                               bool (*cb)(AbckitCoreAnnotationInterfaceField *fld, void *data));

    /* ========================================
     * AnnotationInterfaceField
     * ======================================== */

    /**
     * @brief Returns binary file that the given interface field `fld` is a part of.
     * @return Pointer to the `AbckitFile`.
     * @param [ in ] fld - Annotation interface field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `fld` is NULL.
     */
    AbckitFile *(*annotationInterfaceFieldGetFile)(AbckitCoreAnnotationInterfaceField *fld);

    /**
     * @brief Returns interface for interface field `fld`.
     * @return Pointer to the `AbckitCoreAnnotationInterface`.
     * @param [ in ] fld - Annotation interface field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `fld` is NULL.
     */
    AbckitCoreAnnotationInterface *(*annotationInterfaceFieldGetInterface)(AbckitCoreAnnotationInterfaceField *fld);

    /**
     * @brief Returns name for interface field `fld`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] fld - Annotation interface field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `fld` is NULL.
     * @note Allocates
     */
    AbckitString *(*annotationInterfaceFieldGetName)(AbckitCoreAnnotationInterfaceField *fld);

    /**
     * @brief Returns type for interface field `fld`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] fld - Annotation interface field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `fld` is NULL.
     */
    AbckitType *(*annotationInterfaceFieldGetType)(AbckitCoreAnnotationInterfaceField *fld);

    /**
     * @brief Returns default value for interface field `fld`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] fld - Annotation interface field to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `fld` is NULL.
     */
    AbckitValue *(*annotationInterfaceFieldGetDefaultValue)(AbckitCoreAnnotationInterfaceField *fld);
};

/**
 * @brief Instantiates non-modifying API for core Abckit types.
 * @return Instance of the `AbckitInspectApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitInspectApi const *AbckitGetInspectApiImpl(enum AbckitApiVersion version);

/**
 * @brief Struct that holds the pointers to the modifying API for core Abckit types.
 */
struct CAPI_EXPORT AbckitModifyApi {
    /* ========================================
     * Function
     * ======================================== */

    /**
     * @brief Sets graph for function `function`.
     * @return None.
     * @param [ in ] function - Function to be modified.
     * @param [ in ] graph - Graph to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `function` and `graph` are
     * differ.
     * @note Allocates
     */
    void (*functionSetGraph)(AbckitCoreFunction *function, AbckitGraph *graph);

    /* ========================================
     * Type
     * ======================================== */

    /**
     * @brief Creates type according to type id `id`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ]  id - Type id corresponding to the type being created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is `ABCKIT_TYPE_ID_INVALID`.
     * @note Allocates
     */
    AbckitType *(*createType)(AbckitFile *file, enum AbckitTypeId id);

    /**
     * @brief Creates reference type according to class `klass`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] klass - Class from which the type is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `klass` is NULL.
     * @note Allocates
     */
    AbckitType *(*createReferenceType)(AbckitFile *file, AbckitCoreClass *klass);

    /**
     * @brief Sets name for type `type`.
     * @return None.
     * @param [ in ] type - Type to be modified.
     * @param [ in ] name - Name to be set.
     * @param [ in ] len - Length of the name.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `name` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for dynamic type.
     */
    void (*typeSetName)(AbckitType *type, const char *name, size_t len);

    /**
     * @brief Sets rank for type `type`.
     * @return None.
     * @param [ in ] type - Type to be modified.
     * @param [ in ] rank - Rank to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `type` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for dynamic type.
     */
    void (*typeSetRank)(AbckitType *type, size_t rank);

    /**
     * @brief Creates union type according to the given types `types`.
     * @return Pointer to the `AbckitType`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] types - Types from which the union type is created.
     * @param [ in ] size - Size of the union type to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `types` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `size` is less than 2.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for dynamic type.
     */
    AbckitType *(*createUnionType)(AbckitFile *file, AbckitType **types, size_t size);

    /* ========================================
     * Value
     * ======================================== */

    /**
     * @brief Creates value item `AbckitValue` containing the given boolean value `value`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Boolean value from which value item is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitValue *(*createValueU1)(AbckitFile *file, bool value);

    /**
     * @brief Creates value item containing the given int value `value`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Int value from which value item is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitValue *(*createValueInt)(AbckitFile *file, int value);

    /**
     * @brief Creates value item containing the given double value `value`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Double value from which value item is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitValue *(*createValueDouble)(AbckitFile *file, double value);

    /**
     * @brief Creates value item containing the given C-style null-terminated string `value`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - C-style null-terminated string from which value item is created.
     * @param [ in ] len - length of `value`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Allocates
     */
    AbckitValue *(*createValueString)(AbckitFile *file, const char *value, size_t len);

    /**
     * @brief Creates value item containing the given method reference.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] method - Method from which value item is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `method` is NULL.
     * @note Allocates
     */
    AbckitValue *(*createValueMethod)(AbckitFile *file, AbckitCoreFunction *method);

    /**
     * @brief Creates value item containing the given record reference.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] record - Record from which value item is created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `record` is NULL.
     * @note Allocates
     */
    AbckitValue *(*createValueRecord)(AbckitFile *file, AbckitCoreClass *record);

    /**
     * @brief Creates literal array value item with size `size` from the given value items array `value`.
     * @return Pointer to the `AbckitValue`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Value items from which literal array item is created.
     * @param [ in ] size - Size of the literal array value item to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Allocates
     */
    AbckitValue *(*createLiteralArrayValue)(AbckitFile *file, AbckitValue **value, size_t size);

    /* ========================================
     * String
     * ======================================== */

    /**
     * @brief Creates string item containing the given C-style null-terminated string `value`.
     * @return Pointer to the `AbckitString`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - C-style null-terminated string to be set.
     * @param [ in ] len - length of `value`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Allocates
     */
    AbckitString *(*createString)(AbckitFile *file, const char *value, size_t len);

    /* ========================================
     * LiteralArray
     * ======================================== */

    /**
     * @brief Creates literal array with size `size` from the given literals array `value`.
     * @return Pointer to the `AbckitLiteralArray`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Literals from which literal array is created.
     * @param [ in ] size - Size of the literal array to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Allocates
     */
    AbckitLiteralArray *(*createLiteralArray)(AbckitFile *file, AbckitLiteral **value, size_t size);

    /* ========================================
     * Literal
     * ======================================== */

    /**
     * @brief Creates literal containing the given boolean value `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Boolean value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralBool)(AbckitFile *file, bool value);

    /**
     * @brief Creates literal containing the given byte value `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Byte value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralU8)(AbckitFile *file, uint8_t value);

    /**
     * @brief Creates literal containing the given short value `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Short value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralU16)(AbckitFile *file, uint16_t value);

    /**
     * @brief Creates literal containing the given оffset of method affiliate `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Offset of method affiliate.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralMethodAffiliate)(AbckitFile *file, uint16_t value);

    /**
     * @brief Creates literal containing the given integer value `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Integer value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralU32)(AbckitFile *file, uint32_t value);

    /**
     * @brief Creates literal containing the given long value `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Long value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralU64)(AbckitFile *file, uint64_t value);

    /**
     * @brief Creates literal containing the given float value `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Float value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralFloat)(AbckitFile *file, float value);

    /**
     * @brief Creates literal containing the given double value `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - Double value that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralDouble)(AbckitFile *file, double value);

    /**
     * @brief Creates literal containing the given literal array `litarr`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] litarr - Literal array that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `litarr` is NULL.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if invoked for target `ABCKIT_TARGET_JS`.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralLiteralArray)(AbckitFile *file, AbckitLiteralArray *litarr);

    /**
     * @brief Creates literal containing the given C-style null-terminated string `value`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] value - C-style null-terminated string that will be stored in the literal to be created.
     * @param [ in ] len - length of `value`.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `value` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralString)(AbckitFile *file, const char *value, size_t len);

    /**
     * @brief Creates literal containing the given function `function`.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @param [ in ] function - Function that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralMethod)(AbckitFile *file, AbckitCoreFunction *function);

    /**
     * @brief Creates literal containing null value.
     * @return Pointer to the `AbckitLiteral`.
     * @param [ in ] file - Binary file to be modified.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `file` is NULL.
     * @note Allocates
     */
    AbckitLiteral *(*createLiteralNullValue)(AbckitFile *file);
};

/**
 * @brief Instantiates modifying API for core Abckit types.
 * @return Instance of the `AbckitModifyApi` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitModifyApi const *AbckitGetModifyApiImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LIBABCKIT_METADATA_H */
