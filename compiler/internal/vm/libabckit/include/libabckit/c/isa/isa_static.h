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

#ifndef LIBABCKIT_ISA_ISA_STATIC_H
#define LIBABCKIT_ISA_ISA_STATIC_H

#ifndef __cplusplus
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#else
#include <cstddef>
#include <cstdint>
#endif

#include "../metadata_core.h"
#include "../ir_core.h"
#include "../abckit.h"

#ifdef __cplusplus
extern "C" {
#endif

enum AbckitIsaApiStaticOpcode {
    ABCKIT_ISA_API_STATIC_OPCODE_INVALID,
    ABCKIT_ISA_API_STATIC_OPCODE_CONSTANT,
    ABCKIT_ISA_API_STATIC_OPCODE_PARAMETER,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADSTRING,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADTYPE,
    ABCKIT_ISA_API_STATIC_OPCODE_RETURN,
    ABCKIT_ISA_API_STATIC_OPCODE_TRY,
    ABCKIT_ISA_API_STATIC_OPCODE_CATCHPHI,
    ABCKIT_ISA_API_STATIC_OPCODE_PHI,

    /* Unary operations: */
    ABCKIT_ISA_API_STATIC_OPCODE_NEG,
    ABCKIT_ISA_API_STATIC_OPCODE_NOT,

    /* BinaryOperation: */
    ABCKIT_ISA_API_STATIC_OPCODE_CMP,
    ABCKIT_ISA_API_STATIC_OPCODE_ADD,
    ABCKIT_ISA_API_STATIC_OPCODE_SUB,
    ABCKIT_ISA_API_STATIC_OPCODE_MUL,
    ABCKIT_ISA_API_STATIC_OPCODE_DIV,
    ABCKIT_ISA_API_STATIC_OPCODE_MOD,
    ABCKIT_ISA_API_STATIC_OPCODE_SHL,
    ABCKIT_ISA_API_STATIC_OPCODE_SHR,
    ABCKIT_ISA_API_STATIC_OPCODE_ASHR,
    ABCKIT_ISA_API_STATIC_OPCODE_AND,
    ABCKIT_ISA_API_STATIC_OPCODE_OR,
    ABCKIT_ISA_API_STATIC_OPCODE_XOR,

    /* Cast: */
    ABCKIT_ISA_API_STATIC_OPCODE_CAST,

    /* Object manipulation: */
    ABCKIT_ISA_API_STATIC_OPCODE_NULLPTR,
    ABCKIT_ISA_API_STATIC_OPCODE_NEWARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_NEWOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_INITOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_STOREARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_STOREOBJECT,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADSTATIC,
    ABCKIT_ISA_API_STATIC_OPCODE_STORESTATIC,
    ABCKIT_ISA_API_STATIC_OPCODE_LENARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADCONSTARRAY,
    ABCKIT_ISA_API_STATIC_OPCODE_CHECKCAST,
    ABCKIT_ISA_API_STATIC_OPCODE_ISINSTANCE,
    ABCKIT_ISA_API_STATIC_OPCODE_ISNULLVALUE,
    ABCKIT_ISA_API_STATIC_OPCODE_LOADNULLVALUE,
    ABCKIT_ISA_API_STATIC_OPCODE_EQUALS,
    ABCKIT_ISA_API_STATIC_OPCODE_STRICTEQUALS,
    ABCKIT_ISA_API_STATIC_OPCODE_NULLCHECK,
    ABCKIT_ISA_API_STATIC_OPCODE_ETSISTRUE,
    ABCKIT_ISA_API_STATIC_OPCODE_ETSTYPEOF,
    ABCKIT_ISA_API_STATIC_OPCODE_ETSLDOBJBYNAME,
    /* Return: */
    ABCKIT_ISA_API_STATIC_OPCODE_RETURN_VOID,

    /* Calls: */
    ABCKIT_ISA_API_STATIC_OPCODE_CALL_STATIC,
    ABCKIT_ISA_API_STATIC_OPCODE_CALL_VIRTUAL,

    /* BinaryImmOperation: */
    ABCKIT_ISA_API_STATIC_OPCODE_ADDI,
    ABCKIT_ISA_API_STATIC_OPCODE_SUBI,
    ABCKIT_ISA_API_STATIC_OPCODE_MULI,
    ABCKIT_ISA_API_STATIC_OPCODE_DIVI,
    ABCKIT_ISA_API_STATIC_OPCODE_MODI,
    ABCKIT_ISA_API_STATIC_OPCODE_SHLI,
    ABCKIT_ISA_API_STATIC_OPCODE_SHRI,
    ABCKIT_ISA_API_STATIC_OPCODE_ASHRI,
    ABCKIT_ISA_API_STATIC_OPCODE_ANDI,
    ABCKIT_ISA_API_STATIC_OPCODE_ORI,
    ABCKIT_ISA_API_STATIC_OPCODE_XORI,

    /* Exceptions */
    ABCKIT_ISA_API_STATIC_OPCODE_THROW,

    /* Condition */
    ABCKIT_ISA_API_STATIC_OPCODE_IF,
};

enum AbckitIsaApiStaticConditionCode {
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE = 0,
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_EQ, /* == */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NE, /* != */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LT, /* < */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_LE, /* <= */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GT, /* > */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_GE, /* >= */
    // Unsigned integers.
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_B,  /* < */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_BE, /* <= */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_A,  /* > */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_AE, /* >= */
    // Compare result of bitwise AND with zero
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_EQ, /* (lhs AND rhs) == 0 */
    ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_TST_NE, /* (lhs AND rhs) != 0 */
};

/**
 * @brief Struct that holds the pointers to the API used to work with static ISA.
 * @note Valid targets: `ABCKIT_TARGET_ARK_TS_V2`.
 */
struct CAPI_EXPORT AbckitIsaApiStatic {
    /**
     * @brief Returns core class of `inst`.
     * @return AbckitCoreClass *.
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    AbckitCoreClass *(*iGetClass)(AbckitInst *inst /* in */);

    /**
     * @brief Sets `inst`'s class.
     * @return void .
     * @param [ in ] AbckitInst *inst .
     * @param [ in ]  AbckitCoreClass *klass .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreClass *klass  is NULL.
     */
    void (*iSetClass)(AbckitInst *inst /* in */, AbckitCoreClass *klass /* in */);

    /**
     * @brief Returns condition code.
     * @return enum AbckitIsaApiStaticConditionCode .
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    enum AbckitIsaApiStaticConditionCode (*iGetConditionCode)(AbckitInst *inst /* in */);

    /**
     * @brief Sets condition code.
     * @return void .
     * @param [ in ] AbckitInst *inst .
     * @param [ in ]  enum AbckitIsaApiStaticConditionCode cc .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    void (*iSetConditionCode)(AbckitInst *inst /* in */, enum AbckitIsaApiStaticConditionCode cc /* in */);

    /**
     * @brief Returns `inst`'s opcode.
     * @return enum AbckitIsaApiStaticOpcode .
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    enum AbckitIsaApiStaticOpcode (*iGetOpcode)(AbckitInst *inst /* in */);

    /* For Cast instructions */
    /**
     * @brief Sets target type 't' for 'inst'.
     * @return void .
     * @param [ in ] AbckitInst *inst .
     * @param [ in ]  enum AbckitTypeId t .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    void (*iSetTargetType)(AbckitInst *inst /* in */, enum AbckitTypeId t /* in */);

    /* For Cast instructions */
    /**
     * @brief Returns `inst`'s target type.
     * @return enum AbckitTypeId .
     * @param [ in ] AbckitInst *inst .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inst  is NULL.
     */
    enum AbckitTypeId (*iGetTargetType)(AbckitInst *inst /* in */);

    /**
     * @brief Creates `cmp` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateCmp)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `load string` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitString *str .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitString *str  is NULL.
     */
    AbckitInst *(*iCreateLoadString)(AbckitGraph *graph /* in */, AbckitString *str /* in */);

    /**
     * @brief Creates `return` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateReturn)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */);

    /**
     * @brief Creates `If` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @param [ in ] enum AbckitIsaApiStaticConditionCode cc .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `cc` is ABCKIT_ISA_API_STATIC_CONDITION_CODE_CC_NONE
     */
    AbckitInst *(*iCreateIf)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */,
                             enum AbckitIsaApiStaticConditionCode cc /* in */);

    /**
     * @brief Creates `Neg` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     */
    AbckitInst *(*iCreateNeg)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */);

    /**
     * @brief Creates `Not` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     */
    AbckitInst *(*iCreateNot)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */);

    /**
     * @brief Creates `Add` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateAdd)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Sub` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateSub)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Mul` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateMul)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Div` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateDiv)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Mod` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateMod)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Shl` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateShl)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Shr` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateShr)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `AShr` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateAShr)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `And` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateAnd)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Or` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateOr)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Xor` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     */
    AbckitInst *(*iCreateXor)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief Creates `Cast` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ] enum AbckitTypeId targetType .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input0->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if input1->graph is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if targetTypeId <= ABCKIT_TYPE_ID_INVALID
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if targetTypeId > ABCKIT_TYPE_ID_REFERENCE
     */
    AbckitInst *(*iCreateCast)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */,
                               enum AbckitTypeId targetType /* in */);

    /**
     * @brief Creates `NullPtr` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     */
    AbckitInst *(*gCreateNullPtr)(AbckitGraph *graph /* in */);

    /**
     * @brief Creates `NewArray` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreClass *inputClass .
     * @param [ in ] AbckitInst *inputSize .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreClass *inputClass  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputSize  is NULL.
     * @note Set `ABCKIT_STATUS_INTERNAL_ERROR` error if inputClass->owningModule  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_MODE` error if `graph>file->frontend` and `Mode::STATIC` are not the same.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph` and `inputSize->graph` are not the same.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph->file` and `inputClass->owningModule->file`
     * @note are not the same.
     */
    AbckitInst *(*iCreateNewArray)(AbckitGraph *graph /* in */, AbckitCoreClass *inputClass /* in */,
                                   AbckitInst *inputSize /* in */);

    /**
     * @brief Creates `NewObject` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreClass *inputClass .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreClass *inputClass  is NULL.
     * @note Set `ABCKIT_STATUS_INTERNAL_ERROR` error if inputClass->owningModule  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph->file` and `inputClass->owningModule->file`
     * @note are not the same.
     */
    AbckitInst *(*iCreateNewObject)(AbckitGraph *graph /* in */, AbckitCoreClass *inputClass /* in */);

    /**
     * @brief Creates `InitObject` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreFunction *function .
     * @param [ in ] size_t argCount .
     * @param  ... .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreFunction *function  is NULL.
     * @note Set `ABCKIT_STATUS_INTERNAL_ERROR` error if inputFunction->owningModule  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph->file` and `inputFunction->owningModule->file`
     * @note are not the same.
     */
    AbckitInst *(*iCreateInitObject)(AbckitGraph *graph /* in */, AbckitCoreFunction *function /* in */,
                                     size_t argCount /* in */, ... /* function params */);

    /**
     * @brief Creates `iCreateLoadObject` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph.
     * @param [ in ] AbckitInst *inputObj .
     * @param [ in ] AbckitString *filed .
     * @param [ in ] returnTypeId *returnType .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitType *field  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph` and `inputObj->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateLoadObject)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                     AbckitString *field /* in */, enum AbckitTypeId returnTypeId /* in */);

    /**
     * @brief Creates `StoreObject` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @param [ in ]  AbckitString *fieldId .
     * @param [ in ]  AbckitInst *value .
     * @param [ in ]  AbckitTypeId *typeId .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitString *fieldId  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *value  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `inputObj->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateStoreObject)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                      AbckitString *fieldId /* in */, AbckitInst *value /* in */,
                                      AbckitTypeId typeId /* in */);

    /**
     * @brief Creates `LoadArray` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arrayRef .
     * @param [ in ] AbckitInst *idx .
     * @param [ in ]  enum AbckitTypeId returnTypeId .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arrayRef  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *idx  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `arrayRef->graph`
     * @note and `idx->graph`are not the same.
     */
    AbckitInst *(*iCreateLoadArray)(AbckitGraph *graph /* in */, AbckitInst *arrayRef /* in */,
                                    AbckitInst *idx /* in */, enum AbckitTypeId returnTypeId /* in */);

    /**
     * @brief Creates `StoreArray` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arrayRef .
     * @param [ in ] AbckitInst *idx .
     * @param [ in ]  AbckitInst *value .
     * @param [ in ] enum AbckitTypeId valueTypeId .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arrayRef  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *idx  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *value  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `arrayRef->graph`,
     * @note `idx->graph` and `value->graph` are not the same.
     */
    AbckitInst *(*iCreateStoreArray)(AbckitGraph *graph /* in */, AbckitInst *arrayRef /* in */,
                                     AbckitInst *idx /* in */, AbckitInst *value /* in */,
                                     enum AbckitTypeId valueTypeId /* in */);

    /**
     * @brief Creates `StoreArrayWide` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arrayRef .
     * @param [ in ] AbckitInst *idx .
     * @param [ in ]  AbckitInst *value .
     * @param [ in ] enum AbckitTypeId valueTypeId .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arrayRef  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *idx  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *value  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `arrayRef->graph`,
     * @note `idx->graph` and `value->graph` are not the same.
     */
    AbckitInst *(*iCreateStoreArrayWide)(AbckitGraph *graph /* in */, AbckitInst *arrayRef /* in */,
                                         AbckitInst *idx /* in */, AbckitInst *value /* in */,
                                         enum AbckitTypeId valueTypeId /* in */);

    /**
     * @brief Creates `LenArray` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *arr .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *arr  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph` and `arr->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateLenArray)(AbckitGraph *graph /* in */, AbckitInst *arr /* in */);

    /**
     * @brief Creates `LoadConstArray` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitLiteralArray *literalArray .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitLiteralArray *literalArray  is NULL.
     */
    AbckitInst *(*iCreateLoadConstArray)(AbckitGraph *graph /* in */, AbckitLiteralArray *literalArray /* in */);

    /**
     * @brief Creates `CheckCast` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @param [ in ] AbckitType *targetType .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitType *targetType  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph` and `inputObj->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateCheckCast)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                    AbckitType *targetType /* in */);

    /**
     * @brief Creates `IsInstance` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @param [ in ] AbckitType *targetType .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitType *targetType  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph` and `inputObj->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateIsInstance)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                     AbckitType *targetType /* in */);

    /**
     * @brief iCreateLoadNullValue.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     */
    AbckitInst *(*iCreateLoadNullValue)(AbckitGraph *graph /* in */);

    /**
     * @brief Creates `ReturnVoid` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     */
    AbckitInst *(*iCreateReturnVoid)(AbckitGraph *graph /* in */);

    /**
     * @brief Creates `Equals` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note and `input1->graph` are not the same.
     */
    AbckitInst *(*iCreateEquals)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, AbckitInst *input1 /* in */);

    /**
     * @brief iCreateStrictEquals.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  AbckitInst *input1 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input1  is NULL.
     */
    AbckitInst *(*iCreateStrictEquals)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */,
                                       AbckitInst *input1 /* in */);

    /**
     * @brief Creates `CallStatic` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitCoreFunction *function .
     * @param [ in ] size_t argCount .
     * @param  ... .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreFunction *function  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `function->graph`
     * @note are not the same.
     * @note Set `ABCKIT_STATUS_INTERNAL_ERROR` error if `inputFunction->owningModule` is NULL
     */
    AbckitInst *(*iCreateCallStatic)(AbckitGraph *graph /* in */, AbckitCoreFunction *function /* in */,
                                     size_t argCount /* in */, ... /* function params */);

    /**
     * @brief Creates `CallVirtual` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @param [ in ] AbckitCoreFunction *function .
     * @param [ in ]  size_t argCount .
     * @param ... .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitCoreFunction *function  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `inputObj->graph`
     * @note are not the same.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph->file`, `inputFunction->owningModule->file`
     * @note are not the same.
     * @note Set `ABCKIT_STATUS_INTERNAL_ERROR` error if `inputFunction->owningModule` is NULL
     */
    AbckitInst *(*iCreateCallVirtual)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */,
                                      AbckitCoreFunction *function /* in */, size_t argCount /* in */,
                                      ... /* function params */);

    /**
     * @brief Creates `AddI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateAddI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `SubI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateSubI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `MulI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateMulI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `DivI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateDivI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `ModI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateModI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `ShlI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateShlI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `ShrI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateShrI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `AShrI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateAShrI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `AndI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateAndI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `OrI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateOrI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `XorI` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @param [ in ]  uint64_t imm .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `input0->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateXorI)(AbckitGraph *graph /* in */, AbckitInst *input0 /* in */, uint64_t imm /* in */);

    /**
     * @brief Creates `Throw` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *acc .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *acc  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `acc->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateThrow)(AbckitGraph *graph /* in */, AbckitInst *acc /* in */);

    /**
     * @brief Creates `IsUndefined` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *inputObj .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *inputObj  is NULL.
     * @note Set `ABCKIT_STATUS_WRONG_CTX` error if `graph`, `inputObj->graph`
     * @note are not the same.
     */
    AbckitInst *(*iCreateIsUndefined)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */);

    /**
     * @brief Creates `NullCheck` inst.
     * @return AbckitInst *.
     * @param [ in ] AbckitGraph *graph .
     * @param [ in ]  AbckitInst *input0 .
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitGraph *graph  is NULL.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if AbckitInst *input0  is NULL.
     */
    AbckitInst *(*iCreateNullCheck)(AbckitGraph *graph /* in */, AbckitInst *inputObj /* in */);
};

/**
 * @brief Instantiates API for for working with static ISA.
 * @return Instance of the `AbckitIsaApiStatic` struct with valid function pointers.
 * @param [ in ] version - Version of the API to instantiate.
 * @note Set `ABCKIT_STATUS_UNKNOWN_API_VERSION` error if `version` value is not in the `AbckitApiVersion` enum.
 */
CAPI_EXPORT struct AbckitIsaApiStatic const *AbckitGetIsaApiStaticImpl(enum AbckitApiVersion version);

#ifdef __cplusplus
}
#endif

#endif  // LIBABCKIT_ISA_ISA_STATIC_H
