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

// CC-OFFNXT(超大头文件[C++] Oversized Header File) huge header file

#ifndef LIBABCKIT_ISA_ISA_DYNAMIC_H
#define LIBABCKIT_ISA_ISA_DYNAMIC_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif

#include "../declarations.h"
#include "../api_version.h"

#ifdef __cplusplus
extern "C" {
#endif

enum AbckitIsaApiDynamicOpcode {
    ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CONSTANT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_PARAMETER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LOADSTRING,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_TRY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CATCHPHI,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_PHI,

    ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONENTER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONREJECT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONRESOLVE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCGENERATORREJECT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCGENERATORRESOLVE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG0,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS0,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARG1,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS1,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARGS2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLARGS3,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHIS3,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_COPYDATAPROPERTIES,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_COPYRESTARGS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEARRAYWITHBUFFER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEASYNCGENERATOROBJ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYARRAY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEEMPTYOBJECT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEGENERATOROBJ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEITERRESULTOBJ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHBUFFER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEOBJECTWITHEXCLUDEDKEYS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CREATEREGEXPWITHLITERAL,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEBUGGER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEC,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINECLASSWITHBUFFER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEGETTERSETTERBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEMETHOD,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DELOBJPROP,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_EQ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_EXP,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETASYNCITERATOR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETITERATOR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETNEXTPROPNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETPROPITERATOR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETRESUMEMODE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETTEMPLATEOBJECT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETUNMAPPEDARGS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GREATEREQ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_INC,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ISFALSE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ISIN,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ISTRUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_IF,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDBIGINT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFALSE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBAL,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDGLOBALVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDHOLE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDINFINITY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLEXVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDNAN,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDNULL,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYINDEX,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDOBJBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDSUPERBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDSUPERBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDSYMBOL,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTRUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDUNDEFINED,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LESS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LESSEQ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWLEXENV,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_NOTEQ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_POPLEXENV,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_RESUMEGENERATOR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURN,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_RETURNUNDEFINED,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SETGENERATORSTATE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SETOBJECTWITHPROTO,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STARRAYSPREAD,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STGLOBALVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STLEXVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYINDEX,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOBJBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYINDEX,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STRICTEQ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STRICTNOTEQ,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STSUPERBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STSUPERBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLSPREAD,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SUSPENDGENERATOR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_CONSTASSIGNMENT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_DELETESUPERPROPERTY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFNOTOBJECT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_IFSUPERNOTCORRECTCALL,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_PATTERNNONCOERCIBLE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_TONUMBER,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYLDGLOBALBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_TRYSTGLOBALBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_TYPEOF,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_APPLY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ASYNCFUNCTIONAWAITUNCAUGHT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_CALLINIT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_CREATEPRIVATEPROPERTY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINEFIELDBYINDEX,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINEFIELDBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINEPRIVATEPROPERTY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_DEFINESENDABLECLASS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDSENDABLECLASS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NOTIFYCONCURRENTRESULT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_TOPROPERTYKEY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDSENDABLEEXTERNALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDELDSENDABLEEXTERNALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDSENDABLELOCALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDELDSENDABLELOCALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_NEWSENDABLEENV,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDENEWSENDABLEENV,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_STSENDABLEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDESTSENDABLEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDSENDABLEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDELDSENDABLEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISTRUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_ISFALSE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDLAZYMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDELDLAZYMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_LDLAZYSENDABLEMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_WIDELDLAZYSENDABLEMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLRUNTIME_SUPERCALLFORWARDALLARGS,

    ABCKIT_ISA_API_DYNAMIC_OPCODE_CALLTHISRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEPROPERTYBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFIELDBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DEFINEFUNC,

    ABCKIT_ISA_API_DYNAMIC_OPCODE_ADD2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SUB2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_MUL2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_DIV2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_MOD2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_AND2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_OR2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_XOR2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_ASHR2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SHL2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SHR2,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_NEG,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_NOT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDA_STR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW,

    ABCKIT_ISA_API_DYNAMIC_OPCODE_DYNAMICIMPORT,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_GETMODULENAMESPACE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_INSTANCEOF,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDEXTERNALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDFUNCTION,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDLOCALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDNEWTARGET,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDPRIVATEPROPERTY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTHIS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTHISBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_LDTHISBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWLEXENVWITHNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJAPPLY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_NEWOBJRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STCONSTTOGLOBALRECORD,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYNAMEWITHNAMESET,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STOWNBYVALUEWITHNAMESET,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STPRIVATEPROPERTY,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STTHISBYNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STTHISBYVALUE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_STTOGLOBALRECORD,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLARROWRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_SUPERCALLTHISRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_TESTIN,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_NOTEXISTS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_THROW_UNDEFINEDIFHOLEWITHNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_TONUMERIC,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_CALLRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_CALLTHISRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_COPYRESTARGS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_CREATEOBJECTWITHEXCLUDEDKEYS,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_GETMODULENAMESPACE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDEXTERNALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDLEXVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDLOCALMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDOBJBYINDEX,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_LDPATCHVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_NEWLEXENV,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_NEWLEXENVWITHNAME,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_NEWOBJRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STLEXVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STMODULEVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STOBJBYINDEX,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STOWNBYINDEX,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_STPATCHVAR,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_SUPERCALLARROWRANGE,
    ABCKIT_ISA_API_DYNAMIC_OPCODE_WIDE_SUPERCALLTHISRANGE,
};

enum AbckitIsaApiDynamicConditionCode {
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE = 0,
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ, /* == */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE, /* != */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LT, /* < */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_LE, /* <= */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GT, /* > */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_GE, /* >= */
    // Unsigned integers.
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_B,  /* < */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_BE, /* <= */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_A,  /* > */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_AE, /* >= */
    // Compare result of bitwise AND with zero
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_EQ, /* (lhs AND rhs) == 0 */
    ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_TST_NE, /* (lhs AND rhs) != 0 */
};

/**
 * @brief Struct that holds the pointers to the API used to work with dynamic ISA.
 * @note Valid targets: `ABCKIT_TARGET_TS`, `ABCKIT_TARGET_JS`, `ABCKIT_TARGET_ARK_TS_V1`.
 */
struct CAPI_EXPORT AbckitIsaApiDynamic {
    /**
     * @brief Retruns Module for `inst`.
     * @return Pointer to `AbckitCoreModule`.
     * @param [ in ] inst - Inst to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not Intrinsic.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if Intrinsic opcode is not GETMODULENAMESPACE or
     * WIDE_GETMODULENAMESPACE.
     */
    AbckitCoreModule *(*iGetModule)(AbckitInst *inst);

    /**
     * @brief Sets Module for `inst`.
     * @return None.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] md - Module to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `md`  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not Intrinsic.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if Intrinsic opcode is not GETMODULENAMESPACE or
     * WIDE_GETMODULENAMESPACE.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `inst` and `md` differs.
     */
    void (*iSetModule)(AbckitInst *inst, AbckitCoreModule *md);

    /**
     * @brief Returns condition code of `inst`.
     * @return enum value of `AbckitIsaApiDynamicConditionCode`.
     * @param [ in ] inst - Inst to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is not Intrinsic.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` opcode is not IF.
     */
    enum AbckitIsaApiDynamicConditionCode (*iGetConditionCode)(AbckitInst *inst);

    /**
     * @brief Sets condition code of `inst`.
     * @return None.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] cc - Condition code to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cc` is CC_NONE.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` opcode is not IF.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` opcode is CC_NE or CC_EQ.
     */
    void (*iSetConditionCode)(AbckitInst *inst, enum AbckitIsaApiDynamicConditionCode cc);

    /**
     * @brief Returns opcode of `inst`.
     * @return `AbckitIsaApiDynamicOpcode` enum value.
     * @param [ in ] inst - Inst to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    enum AbckitIsaApiDynamicOpcode (*iGetOpcode)(AbckitInst *inst);

    /**
     * @brief Returns import descriptor of `inst`.
     * @return Pointer to `AbckitCoreImportDescriptor`.
     * @param [ in ] inst - Inst to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    AbckitCoreImportDescriptor *(*iGetImportDescriptor)(AbckitInst *inst);

    /**
     * @brief Sets import descriptor of `inst`.
     * @return None.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] id - Import descriptor to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `id` is NULL.
     */
    void (*iSetImportDescriptor)(AbckitInst *inst, AbckitCoreImportDescriptor *id);

    /**
     * @brief Returns export descriptor of `inst`.
     * @return Pointer to `AbckitCoreExportDescriptor`.
     * @param [ in ] inst - Inst to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     */
    AbckitCoreExportDescriptor *(*iGetExportDescriptor)(AbckitInst *inst);

    /**
     * @brief Sets export descriptor of `inst`.
     * @return None.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] id - Export descriptor to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `inst` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `ed` is NULL.
     */
    void (*iSetExportDescriptor)(AbckitInst *inst, AbckitCoreExportDescriptor *ed);

    /**
     * @brief Creates instruction with opcode LOAD_STRING. This instruction loads the string `str` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] str - String to load.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `str` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLoadString)(AbckitGraph *graph, AbckitString *str);

    /**
     * @brief Creates instruction with opcode LDNAN. This instruction loads the `nan` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdnan)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDINFINITY. This instruction loads the `infinity` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdinfinity)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDUNDEFINED. This instruction loads the `undefined` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdundefined)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDNULL. This instruction loads the `null` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdnull)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDSYMBOL. This instruction loads the object `Symbol` in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdsymbol)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDGLOBAL. This instruction loads the object `global` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdglobal)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDTRUE. This instruction loads `true` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdtrue)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDFALSE. This instruction loads `false` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdfalse)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDHOLE. This instruction loads `hole` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdhole)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDNEWTARGET. This instruction loads the implicit parameter `new.target` of
     * the current function into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdnewtarget)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDTHIS. This instruction loads `this` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdthis)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode POPLEXENV. This instruction pops the current lexical environment.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreatePoplexenv)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode GETUNMAPPEDARGS. This instruction loads the arguments object of the
     * current function into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGetunmappedargs)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode ASYNCFUNCTIONENTER. This instruction creates an async function object,
     * and store the object in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAsyncfunctionenter)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode LDFUNCTION. This instruction loads the current function object in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdfunction)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode DEBUGGER. This instruction pauses execution during debugging.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateDebugger)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode GETPROPITERATOR. This instruction loads `acc`'s for-in iterator into
     * `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGetpropiterator)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode GETITERATOR. This instruction executes GetIterator(acc, sync), and stores
     * the result into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGetiterator)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode GETASYNCITERATOR. This instruction executes GetIterator(acc, sync), and
     * stores the result into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGetasynciterator)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode LDPRIVATEPROPERTY. This instruction loads the property of `acc` of the
     * key obtained from the specified lexical position into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdprivateproperty)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode STPRIVATEPROPERTY. This instruction loads the property of `input0` of the
     * key obtained from the specified lexical position into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - ??????????.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateStprivateproperty)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1,
                                            AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode TESTIN. This instruction loads a token from the specified lexical
     * position, checks whether it is a property of the object stored in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateTestin)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode DEFINEFIELDBYNAME. This instruction defines a property named 'string` for
     * object `input0`, and stores value `acc` in `string`
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Value to be stored.
     * @param [ in ] string - Field name.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateDefinefieldbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                            AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode DEFINEPROPERTYBYNAME.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Value to be stored.
     * @param [ in ] string - Property name.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateDefinepropertybyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                               AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CREATEEMPTYOBJECT. This instruction creates an empty object, and stores
     * it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreateemptyobject)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode CREATEEMPTYARRAY. This instruction creates an empty array, and stores
     * it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreateemptyarray)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode CREATEGENERATOROBJ. This instruction creates a generator with the
     * function object `input0`, and stores it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing function object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreategeneratorobj)(AbckitGraph *graph, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CREATEITERRESULTOBJ. This instruction executes `CreateIterResultObject`
     * with arguments `value` `input0` and `done` `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing object.
     * @param [ in ] input0 - Inst containing boolean.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreateiterresultobj)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode CREATEOBJECTWITHEXCLUDEDKEYS. This instruction creates an object based on
     * object `input0` with excluded properties of the keys `input1`, `inputs[0]`, ..., `inputs[imm0-1]`, and stores
     * it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing object.
     * @param [ in ] input1 - Inst containing first `key`.
     * @param [ in ] imm0 - Number of optional insts containing `key`s.
     * @param [ in ] ... - Optional insts containing `key`s.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreateobjectwithexcludedkeys)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                       uint64_t imm0, ...);

    /**
     * @brief Creates instruction with opcode WIDE_CREATEOBJECTWITHEXCLUDEDKEYS. This instruction creates an object
     * based on object `input0` with excluded properties of the keys `input1`, `inputs[0]`, ..., `inputs[imm0-1]`, and
     * stores it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing object.
     * @param [ in ] input1 - Inst containing first `key`.
     * @param [ in ] imm0 - Number of optional insts containing `key`s.
     * @param [ in ] ... - Optional insts containing `key`s.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideCreateobjectwithexcludedkeys)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                           uint64_t imm0, ...);

    /**
     * @brief Creates instruction with opcode CREATEARRAYWITHBUFFER. This instruction creates an array using literal
     * array indexed by `literalArray`, and stores it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] literalArray - Literal array used in array creation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `literalArray` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreatearraywithbuffer)(AbckitGraph *graph, AbckitLiteralArray *literalArray);

    /**
     * @brief Creates instruction with opcode CREATEOBJECTWITHBUFFER. This instruction creates an object using literal
     * array indexed by `literalArray`, and stores it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] literalArray - Literal array used in object creation.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `literalArray` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreateobjectwithbuffer)(AbckitGraph *graph, AbckitLiteralArray *literalArray);

    /**
     * @brief Creates instruction with opcode NEWOBJAPPLY. This instruction creates an instance of `input0` with
     * arguments list `acc`, and stores the instance in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing parameter list.
     * @param [ in ] input0 - Inst containing class object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateNewobjapply)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode NEWOBJRANGE. This instruction invokes the constructor of `inputs[0]` with
     * arguments `inputs[1]`, ..., `inputs[argCount-1]` to create a class instance, and stores the instance in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] argCount - Number of insts containing class object and arguments.
     * @param [ in ] ... - Insts containing class object and arguments.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateNewobjrange)(AbckitGraph *graph, size_t argCount, ... /* in AbckitInst *inputs... */);

    /**
     * @brief Creates instruction with opcode WIDE_NEWOBJRANGE. This instruction invokes the constructor of `inputs[0]`
     * with arguments `inputs[1]`, ..., `inputs[argCount-1]` to create a class instance, and stores the instance in
     * `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] argCount - Number of insts containing class object and arguments.
     * @param [ in ] ... - Insts containing class object and arguments.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideNewobjrange)(AbckitGraph *graph, size_t argCount, ... /* in AbckitInst *inputs... */);

    /**
     * @brief Creates instruction with opcode NEWLEXENV. This instruction creates a lexical environment with `imm0`
     * slots, and stores it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateNewlexenv)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode WIDE_NEWLEXENV. This instruction creates a lexical environment with `imm0`
     * slots, and stores it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideNewlexenv)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode NEWLEXENVWITHNAME. This instruction creates a lexical environment with
     * `imm0` slots using the names of lexical variables stored in literal array `literalArray`, and stores the
     * created environment in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     * @param [ in ] literalArray - Literal array with names of lexical variables.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `literalArray` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateNewlexenvwithname)(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray);

    /**
     * @brief Creates instruction with opcode WIDE_NEWLEXENVWITHNAME. This instruction creates a lexical environment
     * with `imm0` slots using the names of lexical variables stored in literal array `literalArray`, and stores the
     * created environment in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     * @param [ in ] literalArray - Literal array with names of lexical variables.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `literalArray` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideNewlexenvwithname)(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray);

    /**
     * @brief Creates instruction with opcode CREATEASYNCGENERATOROBJ. This instruction creates an async generator
     * function object with the function object `input0`, and stores the generator in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing function object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCreateasyncgeneratorobj)(AbckitGraph *graph, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode ASYNCGENERATORRESOLVE. This instruction executes `AsyncGeneratorResolve`
     * with arguments `generator input0`, `value input1`, and `done input2`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing `generator`.
     * @param [ in ] input1 - Inst containing `value` object.
     * @param [ in ] input2 - Inst containing `done` boolean.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input2` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input2` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAsyncgeneratorresolve)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                AbckitInst *input2);

    /**
     * @brief Creates instruction with opcode ADD2. This instruction computes the binary operation `input0 + acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAdd2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode SUB2. This instruction computes the binary operation `input0 - acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateSub2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode MUL2. This instruction computes the binary operation `input0 * acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateMul2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode DIV2. This instruction computes the binary operation `input0 / acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateDiv2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode MOD2. This instruction computes the binary operation `input0 % acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateMod2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode EQ. This instruction computes the binary operation `input0 == acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateEq)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode NOTEQ. This instruction computes the binary operation `input0 != acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateNoteq)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode LESS. This instruction computes the binary operation `input0 < acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLess)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode LESSEQ. This instruction computes the binary operation `input0 <= acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLesseq)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode GREATER. This instruction computes the binary operation `input0 > acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGreater)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode GREATEREQ. This instruction computes the binary operation `input0 >= acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGreatereq)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode SHL2. This instruction computes the binary operation `input0 << acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateShl2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode SHR2. This instruction computes the binary operation `input0 >> acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateShr2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode ASHR2. This instruction computes the binary operation `input0 >>> acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAshr2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode AND2. This instruction computes the binary operation `input0 & acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAnd2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode OR2. This instruction computes the binary operation `input0 | acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateOr2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode XOR2. This instruction computes the binary operation `input0 ^ acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateXor2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode EXP. This instruction computes the binary operation `input0 ** acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateExp)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode TYPEOF. This instruction computes `typeof acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object to be inspected.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateTypeof)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode TONUMBER. This instruction executes `ToNumber` with argument `acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateTonumber)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode TONUMERIC. This instruction executes `ToNumeric` with argument `acc`, and
     * stores the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateTonumeric)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode NEG. This instruction computes `-acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateNeg)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode NOT. This instruction computes `!acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateNot)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode INC. This instruction computes `acc + 1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateInc)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode DEC. This instruction computes `acc - 1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing operand.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateDec)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode ISTRUE. This instruction computes `acc == true`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateIstrue)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode ISFALSE. This instruction computes `acc == false`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateIsfalse)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode ISIN. This instruction computes `input0 in acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateIsin)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode INSTANCEOF. This instruction computes `input0 instanceof acc`, and stores
     * the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateInstanceof)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode STRICTNOTEQ. This instruction computes `input0 !== acc`, and stores
     * the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStrictnoteq)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode STRICTEQ. This instruction computes `input0 === acc`, and stores
     * the result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStricteq)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_NOTIFYCONCURRENTRESULT. This instruction notifies runtime with
     * the return value of the underlying concurrent function. This instruction appears only in concurrent functions.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing concurrent function result.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeNotifyconcurrentresult)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINEFIELDBYVALUE. This instruction defines a property with
     * key `input0` for object `input1`, and stores the value of `acc` in it.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing value for a property.
     * @param [ in ] input0 - Inst containing key for property.
     * @param [ in ] input1 - Inst containing object to be modified.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeDefinefieldbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                        AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINEFIELDBYINDEX. This instruction defines a property with
     * key `imm0` for object `input0`, and stores the value of `acc` in it.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing value for a property.
     * @param [ in ] imm0 - Key for property.
     * @param [ in ] input0 - Inst containing object to be modified.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeDefinefieldbyindex)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                        AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_TOPROPERTYKEY. This instruction converts `acc` to property
     * key, if fails, throw an exception.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeTopropertykey)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_CREATEPRIVATEPROPERTY. This instruction creates `imm0`
     * symbols. Obtains the stored private method according to the literal array `literalArray`. If a private instance
     * method exists, creates an additional symbol ("method"). Puts all the created symbols at the end of the lexical
     * environment of the current class according to the creation sequence. This instruction appears only in the
     * class definition.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Number of symbols to create.
     * @param [ in ] literalArray - Literal array of symbols.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `literalArray` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeCreateprivateproperty)(AbckitGraph *graph, uint64_t imm0,
                                                           AbckitLiteralArray *literalArray);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINEPRIVATEPROPERTY. This instruction gets a token from the
     * specified lexical position, and appends it to object `input0` as a property.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing a value of a property.
     * @param [ in ] imm0 - Lexical environment layer number.
     * @param [ in ] imm0 - Slot number.
     * @param [ in ] imm0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeDefineprivateproperty)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                           uint64_t imm1, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_CALLINIT. This instruction sets the value of this as `input0`,
     * invokes the function object stored in `acc` with no argument.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeCallinit)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINESENDABLECLASS. This instruction creates a sendable class
     * object of `function` with literal `literalArray` and superclass `input0`, and stores it in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] function - Method of the constructor of the sendable class.
     * @param [ in ] literalArray - Literal array.
     * @param [ in ] imm0 - Number of formal parameters of method.
     * @param [ in ] input0 - Inst contatining superclass object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `function` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `literalArray` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile` containing `function` and `graph`
     * differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeDefinesendableclass)(AbckitGraph *graph, AbckitCoreFunction *function,
                                                         AbckitLiteralArray *literalArray, uint64_t imm0,
                                                         AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_LDSENDABLECLASS. This instruction loads the sendable class in
     * lexical environment `imm0` into `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Lexical environment.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeLdsendableclass)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_LDSENDABLEEXTERNALMODULEVAR.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Index
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeLdsendableexternalmodulevar)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDELDSENDABLEEXTERNALMODULEVAR.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Index
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeWideldsendableexternalmodulevar)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_NEWSENDABLEENV.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Number of slots in a shared lexical environment.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeNewsendableenv)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDENEWSENDABLEENV.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Number of slots in a shared lexical environment.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeWidenewsendableenv)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_STSENDABLEVAR.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeStsendablevar)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDESTSENDABLEVAR.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` and `AbckitInst` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeWidestsendablevar)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                       uint64_t imm1);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_LDSENDABLEVAR.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeLdsendablevar)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDELDSENDABLEVAR.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeWideldsendablevar)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_ISTRUE.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Input object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeIstrue)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_ISFALSE.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Input object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallruntimeIsfalse)(AbckitGraph *graph, AbckitInst *acc);

    // AbckitInst *(*iCreateCallruntimeLdlazymodulevar)(
    //     AbckitGraph *graph,
    //     uint64_t imm1);

    // AbckitInst *(*iCreateCallruntimeWideldlazymodulevar)(
    //     AbckitGraph *graph,
    //     uint64_t imm1);

    // AbckitInst *(*iCreateCallruntimeLdlazysendablemodulevar)(
    //     AbckitGraph *graph,
    //     uint64_t imm1);

    // AbckitInst *(*iCreateCallruntimeWideldlazysendablemodulevar)(
    //     AbckitGraph *graph,
    //     uint64_t imm1);

    // AbckitInst *(*iCreateCallruntimeSupercallforwardallargs)(
    //     AbckitGraph *graph,
    //     uint64_t imm1);

    /**
     * @brief Creates instruction with opcode THROW. This instruction throws the exception stored in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing exception.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrow)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode THROW_NOTEXISTS. This instruction throws the exception that the method is
     * undefined.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowNotexists)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode THROW_PATTERNNONCOERCIBLE. This instruction throws the exception that the
     * object is not coercible.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowPatternnoncoercible)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode THROW_DELETESUPERPROPERTY. This instruction throws the exception of
     * deleting the property of the superclass.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowDeletesuperproperty)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode THROW_CONSTASSIGNMENT. This instruction throws the exception of
     * assignment to the const variable `input0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing name of a const variable.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowConstassignment)(AbckitGraph *graph, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode THROW_IFNOTOBJECT. This instruction throws exception if `input0` is not an
     * object.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing name of a const variable.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowIfnotobject)(AbckitGraph *graph, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode THROW_UNDEFINEDIFHOLE. This instruction throws the exception that `input1`
     * is `undefined` if `input0` is hole.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Inst containing value of the object.
     * @param [ in ] input0 - Inst containing name of the object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowUndefinedifhole)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode THROW_IFSUPERNOTCORRECTCALL. This instruction throws the exception if
     * `super()` is not called correctly.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing class object.
     * @param [ in ] input0 - Error type.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowIfsupernotcorrectcall)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode THROW_UNDEFINEDIFHOLEWITHNAME. This instruction throws the exception that
     * `string` is `undefined` if `acc` is hole.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing value of the object.
     * @param [ in ] string - String for exception.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateThrowUndefinedifholewithname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);

    /**
     * @brief Creates instruction with opcode CALLARG0. This instruction invokes the function object stored in `acc`
     * with no argument.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing function object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallarg0)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode CALLARG1. This instruction invokes the function object stored in `acc`
     * with `input0` argument.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing argument.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallarg1)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CALLARGS2. This instruction invokes the function object stored in `acc`
     * with `input0` and `input1` arguments.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing first argument.
     * @param [ in ] input1 - Inst containing second argument.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallargs2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode CALLARGS3. This instruction invokes the function object stored in `acc`
     * with `input0`, `input1`, `input2` arguments.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing first argument.
     * @param [ in ] input1 - Inst containing second argument.
     * @param [ in ] input2 - Inst containing third argument.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input2` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input1` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input2` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallargs3)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                    AbckitInst *input2);

    /**
     * @brief Creates instruction with opcode CALLRANGE. This instruction invokes `acc` with arguments `inputs[0]`, ...,
     * `inputs[argCount-1]`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] argCount - Number of arguments.
     * @param [ in ] ... - Arguments.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallrange)(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode WIDE_CALLRANGE. This instruction invokes `acc` with arguments `inputs[0]`,
     * ..., `inputs[argCount-1]`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] argCount - Number of arguments.
     * @param [ in ] ... - Arguments.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideCallrange)(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode SUPERCALLSPREAD. This instruction invokes `acc`'s superclass constructor
     * with argument list `input0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing class object.
     * @param [ in ] acc - Inst containing parameter list.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateSupercallspread)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode APPLY.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with arguments list `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object .
     * @param [ in ] input1 - Arguments object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateApply)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode CALLTHIS0.
     * Sets the value of this as `input0`, invokes the function object stored in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallthis0)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode CALLTHIS1.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with argument `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @param [ in ] input1 - First argument.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallthis1)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode CALLTHIS2.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with arguments `input1`, `input2`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @param [ in ] input1 - First argument.
     * @param [ in ] input2 - Second argument.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input2` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallthis2)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                    AbckitInst *input2);

    /**
     * @brief Creates instruction with opcode CALLTHIS3.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with arguments `input1`, `input2`, and `input3`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @param [ in ] input1 - First argument.
     * @param [ in ] input2 - Second argument.
     * @param [ in ] input3 - Third argument.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input2` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input3  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallthis3)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                    AbckitInst *input2, AbckitInst *input3);

    /**
     * @brief Creates instruction with opcode CALLTHISRANGE.
     * Sets the value of this as first variadic argument, invokes the function object stored in `acc` with arguments
     * `...`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Function object.
     * @param [ in ] argCount - Number of arguments .
     * @param [ in ] ... - Object + arguments.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCallthisrange)(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode WIDE_CALLTHISRANGE.
     * Sets the value of this as first variadic argument, invokes the function object stored in `acc` with arguments
     * `...`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Function object.
     * @param [ in ] argCount - Number of arguments .
     * @param [ in ] ... - Object + arguments.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideCallthisrange)(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode SUPERCALLTHISRANGE.
     * Invokes super with arguments ...
     * This instruction appears only in non-arrow functions.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] argCount - Number of parameters.
     * @param [ in ] ... - Parameters.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateSupercallthisrange)(AbckitGraph *graph, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode WIDE_SUPERCALLTHISRANGE.
     * Invokes super with arguments ...
     * This instruction appears only in non-arrow functions.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] argCount - Number of parameters.
     * @param [ in ] ... - Parameters.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideSupercallthisrange)(AbckitGraph *graph, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode SUPERCALLARROWRANGE.
     * Invokes `acc`'s superclass constructor with arguments `...`.
     * This instruction appears only in arrow functions.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Class object.
     * @param [ in ] argCount - Number of parameters.
     * @param [ in ] ... - Parameters.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateSupercallarrowrange)(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode WIDE_SUPERCALLARROWRANGE.
     * Invokes `acc`'s superclass constructor with arguments `...`.
     * This instruction appears only in arrow functions.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Class object.
     * @param [ in ] argCount - Number of parameters.
     * @param [ in ] ... - Parameters.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `argCount` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideSupercallarrowrange)(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);

    /**
     * @brief Creates instruction with opcode DEFINEGETTERSETTERBYVALUE.
     * Defines accessors for `input0`'s property of the key `input1` with getter method `input2`
     * and setter method `input3`. If `input2` (`input3`) is undefined, then getter (setter) will not be set.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Whether to set a name for the accessor.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Property key value.
     * @param [ in ] input2 - Getter function object.
     * @param [ in ] input3 - Setter function object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input2`  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input3` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateDefinegettersetterbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                    AbckitInst *input1, AbckitInst *input2, AbckitInst *input3);

    /**
     * @brief Creates instruction with opcode DEFINEFUNC. Creates a function instance of `function`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] function - Pointer to AbckitFunction.
     * @param [ in ] imm0 - Number of form parameters.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if  AbckitCoreFunction *function  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateDefinefunc)(AbckitGraph *graph, AbckitCoreFunction *function, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode DEFINEMETHOD.
     * Creates a class method instance of `function` for the prototype `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Prototype object of the class.
     * @param [ in ] function - Method.
     * @param [ in ] imm0 - Number of formal parameters.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreFunction *function  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateDefinemethod)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *function,
                                       uint64_t imm0);

    /**
     * @brief Creates instruction with opcode DEFINECLASSWITHBUFFER.
     * Creates a class object of `function` with literal `literalArray` and superclass `input0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] function - Class constructor.
     * @param [ in ] literalArray - Literal array.
     * @param [ in ] imm0 - parameter count of `function`.
     * @param [ in ] input0 - Object containing parent class.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if  AbckitCoreFunction *function  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitLiteralArray *literalArray  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateDefineclasswithbuffer)(AbckitGraph *graph, AbckitCoreFunction *function,
                                                AbckitLiteralArray *literalArray, uint64_t imm0, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode RESUMEGENERATOR. Executes `GeneratorResume` for the generator `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Generator object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateResumegenerator)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode GETRESUMEMODE.
     * Gets the type of completion resumption value for the generator `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Generator object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGetresumemode)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode Gettemplateobject.
     * Executes `GetTemplateObject` with argument `acc` of type templateLiteral.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ]  `acc` - Argument object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGettemplateobject)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode GETNEXTPROPNAME.
     * Executes the `next()` method of the for-in iterator `input0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input0 - Iterator object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `input0` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGetnextpropname)(AbckitGraph *graph, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode DELOBJPROP. Delete the `input0`'s property of the key `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Key.
     * @param [ in ] input0 - Object to delete property from.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateDelobjprop)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode SUSPENDGENERATOR. Suspends the generator `input0` with value `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Generator object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateSuspendgenerator)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode ASYNCFUNCTIONAWAITUNCAUGHT.
     * Executes `AwaitExpression` with function object `input0` and value `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Function object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAsyncfunctionawaituncaught)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode COPYDATAPROPERTIES. Copies the properties of `acc` into `input0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to load properties from.
     * @param [ in ] input0 - Destintaion object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateCopydataproperties)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode STARRAYSPREAD.
     * Stores `acc` in spreading way into `input0`'s elements starting from the index `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStarrayspread)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode SETOBJECTWITHPROTO. Set `acc`'s `__proto__` property as `input0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Destination object.
     * @param [ in ] input0 - Object to store.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateSetobjectwithproto)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode LDOBJBYVALUE. Loads `input0`'s property of the key `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Key.
     * @param [ in ] input0 - Object to load from.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdobjbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode STOBJBYVALUE. Stores `acc` to `input0`'s property of the key `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destonation object.
     * @param [ in ] input1 - Key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStobjbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode STOWNBYVALUE. Stores `acc` to object `input0`'s property of the key
     * `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store .
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStownbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode LDSUPERBYVALUE. Loads the property of `input0`'s superclass of the key
     * acc.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Key .
     * @param [ in ] input0 - Object to load from.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdsuperbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode STSUPERBYVALUE.
     * Stores `acc` to the property of `input0`'s superclass of the key `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStsuperbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode LDOBJBYINDEX. Loads `acc`'s property of the key `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to load from.
     * @param [ in ] imm0 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdobjbyindex)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode WIDE_LDOBJBYINDEX. Loads `acc`'s property of the key `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to load from.
     * @param [ in ] imm0 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideLdobjbyindex)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode STOBJBYINDEX. Stores `acc` to `input0`'s property of the key `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store .
     * @param [ in ] input0 - Destination object .
     * @param [ in ] imm0 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateStobjbyindex)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode WIDE_STOBJBYINDEX. Stores `acc` to `input0`'s property of the key `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store .
     * @param [ in ] input0 - Destination object .
     * @param [ in ] imm0 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideStobjbyindex)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode STOWNBYINDEX. Stores `acc` to object `input0`'s property of the key
     * `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] imm0 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateStownbyindex)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode WIDE_STOWNBYINDEX. Stores `acc` to object `input0`'s property of the key
     * `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] imm0 - Index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideStownbyindex)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode Asyncfunctionresolve. Resolves the Promise object of `input0` with value
     * `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Promise object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAsyncfunctionresolve)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode ASYNCFUNCTIONREJECT. Rejects the Promise object of `input0` with value
     * `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Promise object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateAsyncfunctionreject)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode COPYRESTARGS. Copies the rest parameters.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - The order in which the remaining parameters in the parameter list start.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateCopyrestargs)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode WIDE_COPYRESTARGS. Copies the rest parameters.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - The order in which the remaining parameters in the parameter list start.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideCopyrestargs)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode LDLEXVAR.
     * Loads the value at the `imm1`-th slot of the lexical environment beyond level `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdlexvar)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode WIDE_LDLEXVAR.
     * Loads the value at the `imm1`-th slot of the lexical environment beyond level `imm0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideLdlexvar)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode STLEXVAR.
     * Stores `acc` in the `imm1`-th slot of the lexical environment beyond `imm0` level.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateStlexvar)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode WIDE_STLEXVAR.
     * Stores `acc` in the `imm1`-th slot of the lexical environment beyond `imm0` level.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` or `imm1` type overflow.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideStlexvar)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);

    /**
     * @brief Creates instruction with opcode GETMODULENAMESPACE. Executes GetModuleNamespace for the `md`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] md - Module descriptor.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `md` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateGetmodulenamespace)(AbckitGraph *graph, AbckitCoreModule *md);

    /**
     * @brief Creates instruction with opcode WIDE_GETMODULENAMESPACE. Executes GetModuleNamespace for the `md`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] md - Module descriptor.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `md` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideGetmodulenamespace)(AbckitGraph *graph, AbckitCoreModule *md);

    /**
     * @brief Creates instruction with opcode STMODULEVAR. Stores `acc` to the module variable in the `ed`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] ed - Destination export descriptor.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStmodulevar)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed);

    /**
     * @brief Creates instruction with opcode WIDE_STMODULEVAR. Stores `acc` to the module variable in the `ed`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] ed - Destination export descriptor.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideStmodulevar)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed);

    /**
     * @brief Creates instruction with opcode TRYLDGLOBALBYNAME. Loads the global variable of the name `string`.
     * If the global variable `string` does not exist, an exception is thrown.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] string - String containing name of global variable.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if  AbckitString *string  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateTryldglobalbyname)(AbckitGraph *graph, AbckitString *string);

    /**
     * @brief Creates instruction with opcode TRYSTGLOBALBYNAME. Stores `acc` to the global variable of the name
     * `string`. If the global variable `string` does not exist, an exception is thrown.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing name of global variable.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateTrystglobalbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);

    /**
     * @brief Creates instruction with opcode LDGLOBALVAR. Loads the global variable of the name `string`.
     * This variable must exist.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] string - String containing name of global variable.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if  AbckitString *string  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdglobalvar)(AbckitGraph *graph, AbckitString *string);

    /**
     * @brief Creates instruction with opcode STGLOBALVAR. Stores `acc` to the global variable of the name `string`.
     * This variable must exist.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing name of global variable.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStglobalvar)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);

    /**
     * @brief Creates instruction with opcode LDOBJBYNAME. Loads `acc`'s property of the key `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to load property from.
     * @param [ in ] string - String containing property key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdobjbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);

    /**
     * @brief Creates instruction with opcode STOBJBYNAME. Stores `acc` to `input0`'s property of the key `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStobjbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode STOWNBYNAME. Stores `acc` to `input0`'s property of the key `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStownbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode LDSUPERBYNAME. Load the property of `acc`'s superclass of the key
     * `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to load property from.
     * @param [ in ] string - String containing property key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning `acc` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdsuperbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);

    /**
     * @brief Creates instruction with opcode STSUPERBYNAME.
     * Stores `acc` to the property of `input0`'s superclass of the key `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitGraph` owning one of `AbckitInst` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStsuperbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode LDLOCALMODULEVAR. Loads the local module variable.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] ed - Export descriptor to load.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `ed` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdlocalmodulevar)(AbckitGraph *graph, AbckitCoreExportDescriptor *ed);

    /**
     * @brief Creates instruction with opcode WIDE_LDLOCALMODULEVAR. Loads the local module variable.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] ed - Export descriptor to load.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `ed` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideLdlocalmodulevar)(AbckitGraph *graph, AbckitCoreExportDescriptor *ed);

    /**
     * @brief Creates instruction with opcode LDEXTERNALMODULEVAR. Loads the external module variable.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] id - Import descriptor to load.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `id` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph`'s mode is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdexternalmodulevar)(AbckitGraph *graph, AbckitCoreImportDescriptor *id);

    /**
     * @brief Creates instruction with opcode WIDE_LDEXTERNALMODULEVAR. Loads the external module variable.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] id - Import descriptor to load.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `id` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateWideLdexternalmodulevar)(AbckitGraph *graph, AbckitCoreImportDescriptor *id);

    /**
     * @brief Creates instruction with opcode STOWNBYVALUEWITHNAMESET.
     * Stores `acc` to object `input0`'s property of the key `input1`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Property key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input1` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `id` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStownbyvaluewithnameset)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  AbckitInst *input1);

    /**
     * @brief Creates instruction with opcode STOWNBYNAMEWITHNAMESET.
     * Stores `acc` to `input0`'s property of the key `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `AbckitFile` owning `id` and `graph` differs.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateStownbynamewithnameset)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                 AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode LDBIGINT. Loads the BigInt instance defined by the `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] string - String containing value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     * @note Allocates
     */
    AbckitInst *(*iCreateLdbigint)(AbckitGraph *graph, AbckitString *string);

    /**
     * @brief Creates instruction with opcode LDTHISBYNAME. Loads this 's property of the key `string`, and stores it in
     * `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] string - String containing the key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateLdthisbyname)(AbckitGraph *graph, AbckitString *string);

    /**
     * @brief Creates instruction with opcode STTHISBYNAME. Stores `acc` to this's property of the key `string`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing attribute key value.
     * @param [ in ] string - String containing the key.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `string` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateStthisbyname)(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);

    /**
     * @brief Creates instruction with opcode LDTHISBYVALUE. Loads this's property of the key `acc`, and stores it in
     * `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing attribute key value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateLdthisbyvalue)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode STTHISBYVALUE. Stores `acc` to this's property of the key `input0`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing value.
     * @param [ in ] input0  - Inst containing attribute key value.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `input0` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateStthisbyvalue)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode WIDE_LDPATCHVAR. Load` the patch variable in the `imm0`-th slot into
     * `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] imm0 - Long value containing patch variable index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     */
    AbckitInst *(*iCreateWideLdpatchvar)(AbckitGraph *graph, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode WIDE_STPATCHVAR. Stores `acc` to the patch variable at the `imm0`-th slot.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing value.
     * @param [ in ] imm0 - Long value containing patch variable index.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     */
    AbckitInst *(*iCreateWideStpatchvar)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode DYNAMICIMPORT. Executes ImportCall with argument `acc`, and stores the
     * result in `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing argument for ImportCall.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateDynamicimport)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode ASYNCGENERATORREJECT. Executes the abstract operation AsyncGeneratorReject
     * with generator `input0` and exception `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing exception object.
     * @param [ in ] input0 - Inst containing generator object.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input0` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `input0` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateAsyncgeneratorreject)(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

    /**
     * @brief Creates instruction with opcode SETGENERATORSTATE. Sets the state of acc as B.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Inst containing generator object.
     * @param [ in ] imm0 - Long value that is generator status.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `acc` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if there is `imm0` type overflow.
     */
    AbckitInst *(*iCreateSetgeneratorstate)(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);

    /**
     * @brief Creates instruction with opcode RETURN. This instruction returns `acc`.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] acc - Instruction to be returned.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `acc` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `input` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateReturn)(AbckitGraph *graph, AbckitInst *acc);

    /**
     * @brief Creates instruction with opcode RETURNUNDEFINED.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateReturnundefined)(AbckitGraph *graph);

    /**
     * @brief Creates instruction with opcode IF.
     * @return Pointer to created `AbckitInst`.
     * @param [ in ] graph - Graph where instruction will be inserted.
     * @param [ in ] input - Instruction that will be compared to zero.
     * @param [ in ] cc - Condition code.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `graph` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `input` is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cc` is `ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE` or
     * if `cc` is not `ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ` or `ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE`.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if corresponding `AbckitFile`s owning `graph` and `input` are differ.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph` is not DYNAMIC.
     */
    AbckitInst *(*iCreateIf)(AbckitGraph *graph, AbckitInst *input, enum AbckitIsaApiDynamicConditionCode cc);
};

/**
 * @brief Instantiates API for working with dynamic ISA.
 * @return Instance of the `AbckitIsaApiDynamic` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitIsaApiDynamic const *AbckitGetIsaApiDynamicImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif

#endif  // LIBABCKIT_ISA_ISA_DYNAMIC_H
