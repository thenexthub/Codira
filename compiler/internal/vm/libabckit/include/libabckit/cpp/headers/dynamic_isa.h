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

#ifndef CPP_ABCKIT_DYNAMIC_ISA_H
#define CPP_ABCKIT_DYNAMIC_ISA_H

#include "instruction.h"
#include "core/import_descriptor.h"
#include "config.h"

#include <cstdint>
#include <string_view>

namespace abckit {

/**
 * @brief DynamicIsa class containing API's for dynamic ISA manipulation
 */
class DynamicIsa final {
    /// @brief To access private constructor
    friend class abckit::Graph;

public:
    /**
     * @brief Deleted constructor
     * @param other
     */
    DynamicIsa(const DynamicIsa &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return DynamicIsa
     */
    DynamicIsa &operator=(const DynamicIsa &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     */
    DynamicIsa(DynamicIsa &&other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return DynamicIsa
     */
    DynamicIsa &operator=(DynamicIsa &&other) = delete;

    /**
     * @brief Destructor
     */
    ~DynamicIsa() = default;

    // Rvalue annotated so we can call it only in callchain context

    /**
     * @brief Creates instruction with opcode LOAD_STRING. This instruction loads the string `string` into `acc`.
     * @return Instruction
     * @param string to load
     */
    Instruction CreateLoadString(std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode SUB2. This instruction computes the binary operation `input0 - acc`, and
     * stores the result in returned instruction.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateSub2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Retruns Module for `inst`.
     * @return Instruction
     * @param [ in ] inst - Inst to be inspected.
     */
    core::Module GetModule(Instruction inst) &&;

    /**
     * @brief Sets Module for `inst`.
     * @return Instruction which is new state for inst.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] mdl - Module to be set.
     */
    Instruction SetModule(Instruction inst, core::Module mdl) &&;

    /**
     * @brief Returns condition code of `inst`.
     * @return enum value of `AbckitIsaApiDynamicConditionCode`.
     * @param [ in ] inst - Inst to be inspected.
     */
    AbckitIsaApiDynamicConditionCode GetConditionCode(Instruction inst) &&;

    /**
     * @brief Sets condition code of `inst`.
     * @return Instruction which is new state for inst.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] cc - Condition code to be set.
     */
    Instruction SetConditionCode(Instruction inst, enum AbckitIsaApiDynamicConditionCode cc) &&;

    /**
     * @brief Returns opcode of `inst`.
     * @return `AbckitIsaApiDynamicOpcode` enum value.
     * @param [ in ] inst - Inst to be inspected.
     */
    AbckitIsaApiDynamicOpcode GetOpcode(Instruction inst) &&;

    /**
     * @brief Returns import descriptor of `inst`.
     * @return ImportDescriptor
     * @param [ in ] inst - Inst to be inspected.
     */
    core::ImportDescriptor GetImportDescriptor(Instruction inst) &&;

    /**
     * @brief Sets import descriptor of `inst`.
     * @return Instruction which is new state for inst.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] descr - Import descriptor to be set.
     */
    Instruction SetImportDescriptor(Instruction inst, core::ImportDescriptor descr) &&;

    /**
     * @brief Returns export descriptor of `inst`.
     * @return `core::ExportDescriptor`
     * @param [ in ] inst - Inst to be inspected.
     */
    core::ExportDescriptor GetExportDescriptor(Instruction inst) &&;

    /**
     * @brief Sets export descriptor of `inst`.
     * @return Instruction which is new state for inst.
     * @param [ in ] inst - Inst to be modified.
     * @param [ in ] descr - Export descriptor to be set.
     */
    Instruction SetExportDescriptor(Instruction inst, core::ExportDescriptor descr) &&;

    /**
     * @brief Creates instruction with opcode LDNAN. This instruction loads the `nan` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdnan() &&;

    /**
     * @brief Creates instruction with opcode LDINFINITY. This instruction loads the `infinity` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdinfinity() &&;

    /**
     * @brief Creates instruction with opcode LDUNDEFINED. This instruction loads the `undefined` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdundefined() &&;

    /**
     * @brief Creates instruction with opcode LDNULL. This instruction loads the `null` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdnull() &&;

    /**
     * @brief Creates instruction with opcode LDSYMBOL. This instruction loads the object `Symbol` in `acc`.
     * @return Instruction
     */
    Instruction CreateLdsymbol() &&;

    /**
     * @brief Creates instruction with opcode LDGLOBAL. This instruction loads the object `global` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdglobal() &&;

    /**
     * @brief Creates instruction with opcode LDTRUE. This instruction loads `true` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdtrue() &&;

    /**
     * @brief Creates instruction with opcode LDFALSE. This instruction loads `false` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdfalse() &&;

    /**
     * @brief Creates instruction with opcode LDHOLE. This instruction loads `hole` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdhole() &&;

    /**
     * @brief Creates instruction with opcode LDNEWTARGET. This instruction loads the implicit parameter `new.target` of
     * the current function into `acc`.
     * @return Instruction
     */
    Instruction CreateLdnewtarget() &&;

    /**
     * @brief Creates instruction with opcode LDTHIS. This instruction loads `this` into `acc`.
     * @return Instruction
     */
    Instruction CreateLdthis() &&;

    /**
     * @brief Creates instruction with opcode POPLEXENV. This instruction pops the current lexical environment.
     * @return Instruction
     */
    Instruction CreatePoplexenv() &&;

    /**
     * @brief Creates instruction with opcode GETUNMAPPEDARGS. This instruction loads the arguments object of the
     * current function into `acc`.
     * @return Instruction
     */
    Instruction CreateGetunmappedargs() &&;

    /**
     * @brief Creates instruction with opcode ASYNCFUNCTIONENTER. This instruction creates an async function object,
     * and store the object in `acc`.
     * @return Instruction
     */
    Instruction CreateAsyncfunctionenter() &&;

    /**
     * @brief Creates instruction with opcode LDFUNCTION. This instruction loads the current function object in `acc`.
     * @return Instruction
     */
    Instruction CreateLdfunction() &&;

    /**
     * @brief Creates instruction with opcode DEBUGGER. This instruction pauses execution during debugging.
     * @return Instruction
     */
    Instruction CreateDebugger() &&;

    /**
     * @brief Creates instruction with opcode GETPROPITERATOR. This instruction loads `acc`'s for-in iterator into
     * `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     */
    Instruction CreateGetpropiterator(Instruction acc) &&;
    /**
     * @brief Creates instruction with opcode GETITERATOR. This instruction executes GetIterator(acc, sync), and stores
     * the result into `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     */
    Instruction CreateGetiterator(Instruction acc) &&;
    /**
     * @brief Creates instruction with opcode GETASYNCITERATOR. This instruction executes GetIterator(acc, sync), and
     * stores the result into `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     */
    Instruction CreateGetasynciterator(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode LDPRIVATEPROPERTY. This instruction loads the property of `acc` of the
     * key obtained from the specified lexical position into `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateLdprivateproperty(Instruction acc, uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode STPRIVATEPROPERTY. This instruction loads the property of `input0` of the
     * key obtained from the specified lexical position into `acc`.
     * @return Instruction
     * @param [ in ] acc - ??????????.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateStprivateproperty(Instruction acc, uint64_t imm0, uint64_t imm1, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode TESTIN. This instruction loads a token from the specified lexical
     * position, checks whether it is a property of the object stored in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateTestin(Instruction acc, uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode DEFINEFIELDBYNAME. This instruction defines a property named 'string` for
     * object `input0`, and stores value `acc` in `string`
     * @return Instruction
     * @param [ in ] acc - Value to be stored.
     * @param [ in ] string - Field name.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateDefinefieldbyname(Instruction acc, std::string_view string, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode DEFINEPROPERTYBYNAME.
     * @return Instruction
     * @param [ in ] acc - Value to be stored.
     * @param [ in ] string - Property name.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateDefinepropertybyname(Instruction acc, std::string_view string, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CREATEEMPTYOBJECT. This instruction creates an empty object, and stores
     * it in `acc`.
     * @return Instruction
     */
    Instruction CreateCreateemptyobject() &&;

    /**
     * @brief Creates instruction with opcode CREATEEMPTYARRAY. This instruction creates an empty array, and stores
     * it in `acc`.
     * @return Instruction
     */
    Instruction CreateCreateemptyarray() &&;

    /**
     * @brief Creates instruction with opcode CREATEGENERATOROBJ. This instruction creates a generator with the
     * function object `input0`, and stores it in `acc`.
     * @return Instruction
     * @param [ in ] input0 - Inst containing function object.
     */
    Instruction CreateCreategeneratorobj(Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CREATEITERRESULTOBJ. This instruction executes `CreateIterResultObject`
     * with arguments `value` `input0` and `done` `input1`.
     * @return Instruction
     * @param [ in ] input0 - Inst containing object.
     * @param [ in ] input1 - Inst containing boolean.
     */
    Instruction CreateCreateiterresultobj(Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode CREATEOBJECTWITHEXCLUDEDKEYS. This instruction creates an object based on
     * object `input0` with excluded properties of the keys `input1`, `inputs[0]`, ..., `inputs[imm0-1]`, and stores
     * it in `acc`.
     * @return Instruction
     * @param [ in ] input0 - Inst containing object.
     * @param [ in ] input1 - Inst containing first `key`.
     * @param [ in ] imm0 - Number of optional insts containing `key`s.
     * @param [ in ] args - Optional insts containing `key`s.
     */
    template <typename... Args>
    Instruction CreateCreateobjectwithexcludedkeys(Instruction input0, Instruction input1, uint64_t imm0,
                                                   Args... args) &&;

    /**
     * @brief Creates instruction with opcode WIDE_CREATEOBJECTWITHEXCLUDEDKEYS. This instruction creates an object
     * based on object `input0` with excluded properties of the keys `input1`, `inputs[0]`, ..., `inputs[imm0-1]`, and
     * stores it in `acc`.
     * @return Instruction
     * @param [ in ] input0 - Inst containing object.
     * @param [ in ] input1 - Inst containing first `key`.
     * @param [ in ] imm0 - Number of optional insts containing `key`s.
     * @param [ in ] args - Optional insts containing `key`s.
     */
    template <typename... Args>
    Instruction CreateWideCreateobjectwithexcludedkeys(Instruction input0, Instruction input1, uint64_t imm0,
                                                       Args... args) &&;

    /**
     * @brief Creates instruction with opcode CREATEARRAYWITHBUFFER. This instruction creates an array using literal
     * array indexed by `literalArray`, and stores it in `acc`.
     * @return Instruction
     * @param [ in ] literalArray - Literal array used in array creation.
     */
    Instruction CreateCreatearraywithbuffer(LiteralArray literalArray) &&;

    /**
     * @brief Creates instruction with opcode CREATEOBJECTWITHBUFFER. This instruction creates an object using literal
     * array indexed by `literalArray`, and stores it in `acc`.
     * @return Instruction
     * @param [ in ] literalArray - Literal array used in object creation.
     */
    Instruction CreateCreateobjectwithbuffer(LiteralArray literalArray) &&;

    /**
     * @brief Creates instruction with opcode NEWOBJAPPLY. This instruction creates an instance of `input0` with
     * arguments list `acc`, and stores the instance in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing parameter list.
     * @param [ in ] input0 - Inst containing class object.
     */
    Instruction CreateNewobjapply(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode NEWOBJRANGE. This instruction invokes the constructor of `inputs[0]` with
     * arguments `inputs[1]`, ..., `inputs[argCount-1]` to create a class instance, and stores the instance in `acc`.
     * @return Instruction
     * @param [ in ] input0 - Class object.
     * @param [ in ] args - Number of insts containing arguments.
     */
    template <typename... Args>
    Instruction CreateNewobjrange(Instruction input0, Args... args) &&;

    /**
     * @brief Creates instruction with opcode NEWOBJRANGE. This instruction invokes the constructor of `inputs[0]` with
     * arguments `inputs[1]`, ..., `inputs[argCount-1]` to create a class instance, and stores the instance in `acc`.
     * @return Instruction
     * @param [ in ] input0 - Class object.
     * @param [ in ] args - Number of insts containing arguments.
     */
    template <typename... Args>
    Instruction CreateWideNewobjrange(Instruction input0, Args... args) &&;

    /**
     * @brief Creates instruction with opcode NEWLEXENV. This instruction creates a lexical environment with `imm0`
     * slots, and stores it in `acc`.
     * @return Instruction
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     */
    Instruction CreateNewlexenv(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode WIDE_NEWLEXENV. This instruction creates a lexical environment with `imm0`
     * slots, and stores it in `acc`.
     * @return Instruction
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     */
    Instruction CreateWideNewlexenv(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode NEWLEXENVWITHNAME. This instruction creates a lexical environment with
     * `imm0` slots using the names of lexical variables stored in literal array `literalArray`, and stores the
     * created environment in `acc`.
     * @return Instruction
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     * @param [ in ] literalArray - Literal array with names of lexical variables.
     */
    Instruction CreateNewlexenvwithname(uint64_t imm0, LiteralArray literalArray) &&;

    /**
     * @brief Creates instruction with opcode WIDE_NEWLEXENVWITHNAME. This instruction creates a lexical environment
     * with `imm0` slots using the names of lexical variables stored in literal array `literalArray`, and stores the
     * created environment in `acc`.
     * @return Instruction
     * @param [ in ] imm0 - Number of slots in the lexical environment.
     * @param [ in ] literalArray - Literal array with names of lexical variables.
     */
    Instruction CreateWideNewlexenvwithname(uint64_t imm0, LiteralArray literalArray) &&;

    /**
     * @brief Creates instruction with opcode CREATEASYNCGENERATOROBJ. This instruction creates an async generator
     * function object with the function object `input0`, and stores the generator in `acc`.
     * @return Instruction
     * @param [ in ] input0 - Inst containing function object.
     */
    Instruction CreateCreateasyncgeneratorobj(Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode ASYNCGENERATORRESOLVE. This instruction executes `AsyncGeneratorResolve`
     * with arguments `generator input0`, `value input1`, and `done input2`.
     * @return Instruction
     * @param [ in ] input0 - Inst containing `generator`.
     * @param [ in ] input1 - Inst containing `value` object.
     * @param [ in ] input2 - Inst containing `done` boolean.
     */
    Instruction CreateAsyncgeneratorresolve(Instruction input0, Instruction input1, Instruction input2) &&;

    /**
     * @brief Creates instruction with opcode ADD2. This instruction computes the binary operation `input0 + acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateAdd2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode MUL2. This instruction computes the binary operation `input0 * acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateMul2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode DIV2. This instruction computes the binary operation `input0 / acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateDiv2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode MOD2. This instruction computes the binary operation `input0 % acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateMod2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode EQ. This instruction computes the binary operation `input0 == acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateEq(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode NOTEQ. This instruction computes the binary operation `input0 != acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateNoteq(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode LESS. This instruction computes the binary operation `input0 < acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateLess(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode LESSEQ. This instruction computes the binary operation `input0 <= acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateLesseq(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode GREATER. This instruction computes the binary operation `input0 > acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateGreater(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode GREATEREQ. This instruction computes the binary operation `input0 >= acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateGreatereq(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode SHL2. This instruction computes the binary operation `input0 << acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateShl2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode SHR2. This instruction computes the binary operation `input0 >> acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateShr2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode ASHR2. This instruction computes the binary operation `input0 >>> acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateAshr2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode AND2. This instruction computes the binary operation `input0 & acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateAnd2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode OR2. This instruction computes the binary operation `input0 | acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateOr2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode XOR2. This instruction computes the binary operation `input0 ^ acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateXor2(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode EXP. This instruction computes the binary operation `input0 ** acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing right operand.
     * @param [ in ] input0 - Inst containing left operand.
     */
    Instruction CreateExp(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode TYPEOF. This instruction computes `typeof acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object to be inspected.
     */
    Instruction CreateTypeof(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode TONUMBER. This instruction executes `ToNumber` with argument `acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     */
    Instruction CreateTonumber(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode TONUMERIC. This instruction executes `ToNumeric` with argument `acc`, and
     * stores the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     */
    Instruction CreateTonumeric(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode NEG. This instruction computes `-acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing operand.
     */
    Instruction CreateNeg(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode NOT. This instruction computes `!acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing operand.
     */
    Instruction CreateNot(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode INC. This instruction computes `acc + 1`.
     * @return Instruction
     * @param [ in ] acc - Inst containing operand.
     */
    Instruction CreateInc(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode DEC. This instruction computes `acc - 1`.
     * @return Instruction
     * @param [ in ] acc - Inst containing operand.
     */
    Instruction CreateDec(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode ISTRUE. This instruction computes `acc == true`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     */
    Instruction CreateIstrue(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode ISFALSE. This instruction computes `acc == false`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     */
    Instruction CreateIsfalse(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode ISIN. This instruction computes `input0 in acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateIsin(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode INSTANCEOF. This instruction computes `input0 instanceof acc`, and stores
     * the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateInstanceof(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode STRICTNOTEQ. This instruction computes `input0 !== acc`, and stores
     * the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateStrictnoteq(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode STRICTEQ. This instruction computes `input0 === acc`, and stores
     * the result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing object.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateStricteq(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_NOTIFYCONCURRENTRESULT. This instruction notifies runtime with
     * the return value of the underlying concurrent function. This instruction appears only in concurrent functions.
     * @return Instruction
     * @param [ in ] acc - Inst containing concurrent function result.
     */
    Instruction CreateCallruntimeNotifyconcurrentresult(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINEFIELDBYVALUE. This instruction defines a property with
     * key `input0` for object `input1`, and stores the value of `acc` in it.
     * @return Instruction
     * @param [ in ] acc - Inst containing value for a property.
     * @param [ in ] input0 - Inst containing key for property.
     * @param [ in ] input1 - Inst containing object to be modified.
     */
    Instruction CreateCallruntimeDefinefieldbyvalue(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINEFIELDBYINDEX. This instruction defines a property with
     * key `imm0` for object `input0`, and stores the value of `acc` in it.
     * @return Instruction
     * @param [ in ] acc - Inst containing value for a property.
     * @param [ in ] imm0 - Key for property.
     * @param [ in ] input0 - Inst containing object to be modified.
     */
    Instruction CreateCallruntimeDefinefieldbyindex(Instruction acc, uint64_t imm0, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_TOPROPERTYKEY. This instruction converts `acc` to property
     * key, if fails, throw an exception.
     * @return Instruction
     * @param [ in ] acc - Inst containing value.
     */
    Instruction CreateCallruntimeTopropertykey(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_CREATEPRIVATEPROPERTY. This instruction creates `imm0`
     * symbols. Obtains the stored private method according to the literal array `literalArray`. If a private instance
     * method exists, creates an additional symbol ("method"). Puts all the created symbols at the end of the lexical
     * environment of the current class according to the creation sequence. This instruction appears only in the
     * class definition.
     * @return Instruction
     * @param [ in ] imm0 - Number of symbols to create.
     * @param [ in ] literalArray - Literal array of symbols.
     */
    Instruction CreateCallruntimeCreateprivateproperty(uint64_t imm0, LiteralArray literalArray) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINEPRIVATEPROPERTY. This instruction gets a token from the
     * specified lexical position, and appends it to object `input0` as a property.
     * @return Instruction
     * @param [ in ] acc - Inst containing a value of a property.
     * @param [ in ] imm0 - Lexical environment layer number.
     * @param [ in ] imm1 - Slot number.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateCallruntimeDefineprivateproperty(Instruction acc, uint64_t imm0, uint64_t imm1,
                                                       Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_CALLINIT. This instruction sets the value of this as `input0`,
     * invokes the function object stored in `acc` with no argument.
     * @return Instruction
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing object.
     */
    Instruction CreateCallruntimeCallinit(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_DEFINESENDABLECLASS. This instruction creates a sendable class
     * object of `function` with literal `literalArray` and superclass `input0`, and stores it in `acc`.
     * @return Instruction
     * @param [ in ] function - Method of the constructor of the sendable class.
     * @param [ in ] literalArray - Literal array.
     * @param [ in ] imm0 - Number of formal parameters of method.
     * @param [ in ] input0 - Inst contatining superclass object.
     */
    Instruction CreateCallruntimeDefinesendableclass(core::Function function, LiteralArray literalArray, uint64_t imm0,
                                                     Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_LDSENDABLECLASS. This instruction loads the sendable class in
     * lexical environment `imm0` into `acc`.
     * @return Instruction
     * @param [ in ] imm0 - Lexical environment.
     */
    Instruction CreateCallruntimeLdsendableclass(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_LDSENDABLEEXTERNALMODULEVAR.
     * @return Instruction
     * @param [ in ] imm0 - Index
     */
    Instruction CreateCallruntimeLdsendableexternalmodulevar(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDELDSENDABLEEXTERNALMODULEVAR.
     * @return Instruction
     * @param [ in ] imm0 - Index
     */
    Instruction CreateCallruntimeWideldsendableexternalmodulevar(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDENEWSENDABLEENV.
     * @return Instruction
     * @param [ in ] imm0 - Number of variables.
     */
    Instruction CreateCallruntimeNewsendableenv(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDENEWSENDABLEENV.
     * @return Instruction
     * @param [ in ] imm0 - Number of variables.
     */
    Instruction CreateCallruntimeWidenewsendableenv(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_STSENDABLEVAR.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateCallruntimeStsendablevar(Instruction acc, uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDESTSENDABLEVAR.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateCallruntimeWidestsendablevar(Instruction acc, uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_LDSENDABLEVAR.
     * @return Instruction
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateCallruntimeLdsendablevar(uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_WIDELDSENDABLEVAR.
     * @return Instruction
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateCallruntimeWideldsendablevar(uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_ISTRUE.
     * @return Instruction
     * @param [ in ] acc - Input object.
     */
    Instruction CreateCallruntimeIstrue(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode CALLRUNTIME_ISFALSE.
     * @return Instruction
     * @param [ in ] acc - Input object.
     */
    Instruction CreateCallruntimeIsfalse(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode THROW. This instruction throws the exception stored in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing exception.
     */
    Instruction CreateThrow(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode THROW_NOTEXISTS. This instruction throws the exception that the method is
     * undefined.
     * @return Instruction
     */
    Instruction CreateThrowNotexists() &&;

    /**
     * @brief Creates instruction with opcode THROW_PATTERNNONCOERCIBLE. This instruction throws the exception that the
     * object is not coercible.
     * @return Instruction
     */
    Instruction CreateThrowPatternnoncoercible() &&;

    /**
     * @brief Creates instruction with opcode THROW_DELETESUPERPROPERTY. This instruction throws the exception of
     * deleting the property of the superclass.
     * @return Instruction
     */
    Instruction CreateThrowDeletesuperproperty() &&;

    /**
     * @brief Creates instruction with opcode THROW_CONSTASSIGNMENT. This instruction throws the exception of
     * assignment to the const variable `input0`.
     * @return Instruction
     * @param [ in ] input0 - Inst containing name of a const variable.
     */
    Instruction CreateThrowConstassignment(Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode THROW_IFNOTOBJECT. This instruction throws exception if `input0` is not an
     * object.
     * @return Instruction
     * @param [ in ] input0 - Inst containing name of a const variable.
     */
    Instruction CreateThrowIfnotobject(Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode THROW_UNDEFINEDIFHOLE. This instruction throws the exception that `input1`
     * is `undefined` if `input0` is hole.
     * @return Instruction
     * @param [ in ] input0 - Inst containing value of the object.
     * @param [ in ] input1 - Inst containing name of the object.
     */
    Instruction CreateThrowUndefinedifhole(Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode THROW_IFSUPERNOTCORRECTCALL. This instruction throws the exception if
     * `super()` is not called correctly.
     * @return Instruction
     * @param [ in ] acc - Inst containing class object.
     * @param [ in ] imm0 - Error type.
     */
    Instruction CreateThrowIfsupernotcorrectcall(Instruction acc, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode THROW_UNDEFINEDIFHOLEWITHNAME. This instruction throws the exception that
     * `string` is `undefined` if `acc` is hole.
     * @return Instruction
     * @param [ in ] acc - Inst containing value of the object.
     * @param [ in ] string - String for exception.
     */
    Instruction CreateThrowUndefinedifholewithname(Instruction acc, std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode CALLARG0. This instruction invokes the function object stored in `acc`
     * with no argument.
     * @return Instruction
     * @param [ in ] acc - Inst containing function object.
     */
    Instruction CreateCallarg0(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode CALLARG1. This instruction invokes the function object stored in `acc`
     * with `input0` argument.
     * @return Instruction
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing argument.
     */
    Instruction CreateCallarg1(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CALLARGS2. This instruction invokes the function object stored in `acc`
     * with `input0` and `input1` arguments.
     * @return Instruction
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing first argument.
     * @param [ in ] input1 - Inst containing second argument.
     */
    Instruction CreateCallargs2(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode CALLARGS3. This instruction invokes the function object stored in `acc`
     * with `input0`, `input1`, `input2` arguments.
     * @return Instruction
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] input0 - Inst containing first argument.
     * @param [ in ] input1 - Inst containing second argument.
     * @param [ in ] input2 - Inst containing third argument.
     */
    Instruction CreateCallargs3(Instruction acc, Instruction input0, Instruction input1, Instruction input2) &&;

    /**
     * @brief Creates instruction with opcode CALLRANGE. This instruction invokes `acc` with arguments `inputs[0]`, ...,
     * `inputs[argCount-1]`.
     * @return Instruction
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] instrs - Arguments.
     */
    template <typename... Args>
    Instruction CreateCallrange(Instruction acc, Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode WIDE_CALLRANGE. This instruction invokes `acc` with arguments `inputs[0]`,
     * ..., `inputs[argCount-1]`.
     * @return Instruction
     * @param [ in ] acc - Inst containing function object.
     * @param [ in ] instrs - Arguments.
     */
    template <typename... Args>
    Instruction CreateWideCallrange(Instruction acc, Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode SUPERCALLSPREAD. This instruction invokes `acc`'s superclass constructor
     * with argument list `input0`.
     * @return Instruction
     * @param [ in ] acc - Inst containing class object.
     * @param [ in ] input0 - Inst containing parameter list.
     */
    Instruction CreateSupercallspread(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode APPLY.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with arguments list `input1`.
     * @return Instruction
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object .
     * @param [ in ] input1 - Arguments object.
     */
    Instruction CreateApply(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode CALLTHIS0.
     * Sets the value of this as `input0`, invokes the function object stored in `acc`.
     * @return Instruction
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     */
    Instruction CreateCallthis0(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode CALLTHIS1.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with argument `input1`.
     * @return Instruction
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @param [ in ] input1 - First argument.
     */
    Instruction CreateCallthis1(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode CALLTHIS2.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with arguments `input1`, `input2`.
     * @return Instruction
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @param [ in ] input1 - First argument.
     * @param [ in ] input2 - Second argument.
     */
    Instruction CreateCallthis2(Instruction acc, Instruction input0, Instruction input1, Instruction input2) &&;

    /**
     * @brief Creates instruction with opcode CALLTHIS3.
     * Sets the value of this as `input0`,
     * invokes the function object stored in `acc` with arguments `input1`, `input2`, and `input3`.
     * @return Instruction
     * @param [ in ] acc - Function object.
     * @param [ in ] input0 - This object.
     * @param [ in ] input1 - First argument.
     * @param [ in ] input2 - Second argument.
     * @param [ in ] input3 - Third argument.
     */
    Instruction CreateCallthis3(Instruction acc, Instruction input0, Instruction input1, Instruction input2,
                                Instruction input3) &&;

    /**
     * @brief Creates instruction with opcode CALLTHISRANGE.
     * Sets the value of this as first variadic argument, invokes the function object stored in `acc` with arguments
     * `...`.
     * @return Instruction
     * @param [ in ] acc - Function object.
     * @param [ in ] instrs - Object + arguments.
     */
    template <typename... Args>
    Instruction CreateCallthisrange(Instruction acc, Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode WIDE_CALLTHISRANGE.
     * Sets the value of this as first variadic argument, invokes the function object stored in `acc` with arguments
     * `...`.
     * @return Instruction
     * @param [ in ] acc - Function object.
     * @param [ in ] instrs - Object + arguments.
     */
    template <typename... Args>
    Instruction CreateWideCallthisrange(Instruction acc, Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode SUPERCALLTHISRANGE.
     * Invokes super with arguments ...
     * This instruction appears only in non-arrow functions.
     * @return Instruction
     * @param [ in ] instrs - Parameters.
     */
    template <typename... Args>
    Instruction CreateSupercallthisrange(Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode WIDE_SUPERCALLTHISRANGE.
     * Invokes super with arguments ...
     * This instruction appears only in non-arrow functions.
     * @return Instruction
     * @param [ in ] instrs - Parameters.
     */
    template <typename... Args>
    Instruction CreateWideSupercallthisrange(Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode SUPERCALLARROWRANGE.
     * Invokes `acc`'s superclass constructor with arguments `...`.
     * This instruction appears only in arrow functions.
     * @return Instruction
     * @param [ in ] acc - Class object.
     * @param [ in ] instrs - Parameters.
     */
    template <typename... Args>
    Instruction CreateSupercallarrowrange(Instruction acc, Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode WIDE_SUPERCALLARROWRANGE.
     * Invokes `acc`'s superclass constructor with arguments `...`.
     * This instruction appears only in arrow functions.
     * @return Instruction
     * @param [ in ] acc - Class object.
     * @param [ in ] instrs - Parameters.
     */
    template <typename... Args>
    Instruction CreateWideSupercallarrowrange(Instruction acc, Args... instrs) &&;

    /**
     * @brief Creates instruction with opcode DEFINEGETTERSETTERBYVALUE.
     * Defines accessors for `input0`'s property of the key `input1` with getter method `input2`
     * and setter method `input3`. If `input2` (`input3`) is undefined, then getter (setter) will not be set.
     * @return Instruction
     * @param [ in ] acc - Whether to set a name for the accessor.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Property key value.
     * @param [ in ] input2 - Getter function object.
     * @param [ in ] input3 - Setter function object.
     */
    Instruction CreateDefinegettersetterbyvalue(Instruction acc, Instruction input0, Instruction input1,
                                                Instruction input2, Instruction input3) &&;

    /**
     * @brief Creates instruction with opcode DEFINEFUNC. Creates a function instance of `function`.
     * @return Instruction
     * @param [ in ] function - Pointer to AbckitFunction.
     * @param [ in ] imm0 - Number of form parameters.
     */
    Instruction CreateDefinefunc(core::Function function, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode DEFINEMETHOD.
     * Creates a class method instance of `function` for the prototype `acc`.
     * @return Instruction
     * @param [ in ] acc - Prototype object of the class.
     * @param [ in ] function - Method.
     * @param [ in ] imm0 - Number of formal parameters.
     */
    Instruction CreateDefinemethod(Instruction acc, core::Function function, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode DEFINECLASSWITHBUFFER.
     * Creates a class object of `function` with literal `literalArray` and superclass `input0`.
     * @return Instruction
     * @param [ in ] function - Class constructor.
     * @param [ in ] literalArray - Literal array.
     * @param [ in ] imm0 - parameter count of `function`.
     * @param [ in ] input0 - Object containing parent class.
     */
    Instruction CreateDefineclasswithbuffer(core::Function function, LiteralArray literalArray, uint64_t imm0,
                                            Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode RESUMEGENERATOR. Executes `GeneratorResume` for the generator `acc`.
     * @return Instruction
     * @param [ in ] acc - Generator object.
     */
    Instruction CreateResumegenerator(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode GETRESUMEMODE.
     * Gets the type of completion resumption value for the generator `acc`.
     * @return Instruction
     * @param [ in ] acc - Generator object.
     */
    Instruction CreateGetresumemode(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode Gettemplateobject.
     * Executes `GetTemplateObject` with argument `acc` of type templateLiteral.
     * @return Instruction
     * @param [ in ]  acc - Argument object.
     */
    Instruction CreateGettemplateobject(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode GETNEXTPROPNAME.
     * Executes the `next()` method of the for-in iterator `input0`.
     * @return Instruction
     * @param [ in ] input0 - Iterator object.
     */
    Instruction CreateGetnextpropname(Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode DELOBJPROP. Delete the `input0`'s property of the key `acc`.
     * @return Instruction
     * @param [ in ] acc - Key.
     * @param [ in ] input0 - Object to delete property from.
     */
    Instruction CreateDelobjprop(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode SUSPENDGENERATOR. Suspends the generator `input0` with value `acc`.
     * @return Instruction
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Generator object.
     */
    Instruction CreateSuspendgenerator(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode ASYNCFUNCTIONAWAITUNCAUGHT.
     * Executes `AwaitExpression` with function object `input0` and value `acc`.
     * @return Instruction
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Function object.
     */
    Instruction CreateAsyncfunctionawaituncaught(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode COPYDATAPROPERTIES. Copies the properties of `acc` into `input0`.
     * @return Instruction
     * @param [ in ] acc - Object to load properties from.
     * @param [ in ] input0 - Destintaion object.
     */
    Instruction CreateCopydataproperties(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode STARRAYSPREAD.
     * Stores `acc` in spreading way into `input0`'s elements starting from the index `input1`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Index.
     */
    Instruction CreateStarrayspread(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode SETOBJECTWITHPROTO. Set `acc`'s `__proto__` property as `input0`.
     * @return Instruction
     * @param [ in ] acc - Destination object.
     * @param [ in ] input0 - Object to store.
     */
    Instruction CreateSetobjectwithproto(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode LDOBJBYVALUE. Loads `input0`'s property of the key `acc`.
     * @return Instruction
     * @param [ in ] acc - Key.
     * @param [ in ] input0 - Object to load from.
     */
    Instruction CreateLdobjbyvalue(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode STOBJBYVALUE. Stores `acc` to `input0`'s property of the key `input1`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destonation object.
     * @param [ in ] input1 - Key.
     */
    Instruction CreateStobjbyvalue(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode STOWNBYVALUE. Stores `acc` to object `input0`'s property of the key
     * `input1`.
     * @return Instruction
     * @param [ in ] acc - Object to store .
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Key.
     */
    Instruction CreateStownbyvalue(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode LDSUPERBYVALUE. Loads the property of `input0`'s superclass of the key
     * acc.
     * @return Instruction
     * @param [ in ] acc - Key .
     * @param [ in ] input0 - Object to load from.
     */
    Instruction CreateLdsuperbyvalue(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode STSUPERBYVALUE.
     * Stores `acc` to the property of `input0`'s superclass of the key `input1`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Key.
     */
    Instruction CreateStsuperbyvalue(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode LDOBJBYINDEX. Loads `acc`'s property of the key `imm0`.
     * @return Instruction
     * @param [ in ] acc - Object to load from.
     * @param [ in ] imm0 - Index.
     */
    Instruction CreateLdobjbyindex(Instruction acc, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode WIDE_LDOBJBYINDEX. Loads `acc`'s property of the key `imm0`.
     * @return Instruction
     * @param [ in ] acc - Object to load from.
     * @param [ in ] imm0 - Index.
     */
    Instruction CreateWideLdobjbyindex(Instruction acc, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode STOBJBYINDEX. Stores `acc` to `input0`'s property of the key `imm0`.
     * @return Instruction
     * @param [ in ] acc - Object to store .
     * @param [ in ] input0 - Destination object .
     * @param [ in ] imm0 - Index.
     */
    Instruction CreateStobjbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode WIDE_STOBJBYINDEX. Stores `acc` to `input0`'s property of the key `imm0`.
     * @return Instruction
     * @param [ in ] acc - Object to store .
     * @param [ in ] input0 - Destination object .
     * @param [ in ] imm0 - Index.
     */
    Instruction CreateWideStobjbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode STOWNBYINDEX. Stores `acc` to object `input0`'s property of the key
     * `imm0`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] imm0 - Index.
     */
    Instruction CreateStownbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode WIDE_STOWNBYINDEX. Stores `acc` to object `input0`'s property of the key
     * `imm0`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] imm0 - Index.
     */
    Instruction CreateWideStownbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode Asyncfunctionresolve. Resolves the Promise object of `input0` with value
     * `acc`.
     * @return Instruction
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Promise object.
     */
    Instruction CreateAsyncfunctionresolve(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode ASYNCFUNCTIONREJECT. Rejects the Promise object of `input0` with value
     * `acc`.
     * @return Instruction
     * @param [ in ] acc - Value.
     * @param [ in ] input0 - Promise object.
     */
    Instruction CreateAsyncfunctionreject(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode COPYRESTARGS. Copies the rest parameters.
     * @return Instruction
     * @param [ in ] imm0 - The order in which the remaining parameters in the parameter list start.
     */
    Instruction CreateCopyrestargs(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode WIDE_COPYRESTARGS. Copies the rest parameters.
     * @return Instruction
     * @param [ in ] imm0 - The order in which the remaining parameters in the parameter list start.
     */
    Instruction CreateWideCopyrestargs(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode LDLEXVAR.
     * Loads the value at the `imm1`-th slot of the lexical environment beyond level `imm0`.
     * @return Instruction
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateLdlexvar(uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode WIDE_LDLEXVAR.
     * Loads the value at the `imm1`-th slot of the lexical environment beyond level `imm0`.
     * @return Instruction
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateWideLdlexvar(uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode STLEXVAR.
     * Stores `acc` in the `imm1`-th slot of the lexical environment beyond `imm0` level.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateStlexvar(Instruction acc, uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode WIDE_STLEXVAR.
     * Stores `acc` in the `imm1`-th slot of the lexical environment beyond `imm0` level.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] imm0 - Lexical environment level.
     * @param [ in ] imm1 - Slot number.
     */
    Instruction CreateWideStlexvar(Instruction acc, uint64_t imm0, uint64_t imm1) &&;

    /**
     * @brief Creates instruction with opcode GETMODULENAMESPACE. Executes GetModuleNamespace for the `md`.
     * @return Instruction
     * @param [ in ] md - Module descriptor.
     */
    Instruction CreateGetmodulenamespace(core::Module md) &&;

    /**
     * @brief Creates instruction with opcode WIDE_GETMODULENAMESPACE. Executes GetModuleNamespace for the `md`.
     * @return Instruction
     * @param [ in ] md - Module descriptor.
     */
    Instruction CreateWideGetmodulenamespace(core::Module md) &&;

    /**
     * @brief Creates instruction with opcode STMODULEVAR. Stores `acc` to the module variable in the `ed`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] ed - Destination export descriptor.
     */
    Instruction CreateStmodulevar(Instruction acc, core::ExportDescriptor ed) &&;

    /**
     * @brief Creates instruction with opcode WIDE_STMODULEVAR. Stores `acc` to the module variable in the `ed`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] ed - Destination export descriptor.
     */
    Instruction CreateWideStmodulevar(Instruction acc, core::ExportDescriptor ed) &&;

    /**
     * @brief Creates instruction with opcode TRYLDGLOBALBYNAME. Loads the global variable of the name `string`.
     * If the global variable `string` does not exist, an exception is thrown.
     * @return Instruction
     * @param [ in ] string to load
     */
    Instruction CreateTryldglobalbyname(std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode TRYSTGLOBALBYNAME. Stores `acc` to the global variable of the name
     * `string`. If the global variable `string` does not exist, an exception is thrown.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing name of global variable.
     */
    Instruction CreateTrystglobalbyname(Instruction acc, std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode LDGLOBALVAR. Loads the global variable of the name `string`.
     * This variable must exist.
     * @return Instruction
     * @param [ in ] string - String containing name of global variable.
     */
    Instruction CreateLdglobalvar(std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode STGLOBALVAR. Stores `acc` to the global variable of the name `string`.
     * This variable must exist.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing name of global variable.
     */
    Instruction CreateStglobalvar(Instruction acc, std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode LDOBJBYNAME. Loads `acc`'s property of the key `string`.
     * @return Instruction
     * @param [ in ] acc - Object to load property from.
     * @param [ in ] string - String containing property key.
     */
    Instruction CreateLdobjbyname(Instruction acc, std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode STOBJBYNAME. Stores `acc` to `input0`'s property of the key `string`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     */
    Instruction CreateStobjbyname(Instruction acc, std::string_view string, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode STOWNBYNAME. Stores `acc` to `input0`'s property of the key `string`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     */
    Instruction CreateStownbyname(Instruction acc, std::string_view string, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode LDSUPERBYNAME. Load the property of `acc`'s superclass of the key
     * `string`.
     * @return Instruction
     * @param [ in ] acc - Object to load property from.
     * @param [ in ] string - String containing property key.
     */
    Instruction CreateLdsuperbyname(Instruction acc, std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode STSUPERBYNAME.
     * Stores `acc` to the property of `input0`'s superclass of the key `string`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     */
    Instruction CreateStsuperbyname(Instruction acc, std::string_view string, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode LDLOCALMODULEVAR. Loads the local module variable.
     * @return Instruction
     * @param [ in ] ed - Export descriptor to load.
     */
    Instruction CreateLdlocalmodulevar(core::ExportDescriptor ed) &&;

    /**
     * @brief Creates instruction with opcode WIDE_LDLOCALMODULEVAR. Loads the local module variable.
     * @return Instruction
     * @param [ in ] ed - Export descriptor to load.
     */
    Instruction CreateWideLdlocalmodulevar(core::ExportDescriptor ed) &&;

    /**
     * @brief Creates instruction with opcode LDEXTERNALMODULEVAR. Loads the external module variable.
     * @return Instruction
     * @param [ in ] id - Import descriptor to load.
     */
    Instruction CreateLdexternalmodulevar(core::ImportDescriptor id) &&;

    /**
     * @brief Creates instruction with opcode WIDE_LDEXTERNALMODULEVAR. Loads the external module variable.
     * @return Instruction
     * @param [ in ] id - Import descriptor to load.
     */
    Instruction CreateWideLdexternalmodulevar(core::ImportDescriptor id) &&;

    /**
     * @brief Creates instruction with opcode STOWNBYVALUEWITHNAMESET.
     * Stores `acc` to object `input0`'s property of the key `input1`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] input0 - Destination object.
     * @param [ in ] input1 - Property key.
     */
    Instruction CreateStownbyvaluewithnameset(Instruction acc, Instruction input0, Instruction input1) &&;

    /**
     * @brief Creates instruction with opcode STOWNBYNAMEWITHNAMESET.
     * Stores `acc` to `input0`'s property of the key `string`.
     * @return Instruction
     * @param [ in ] acc - Object to store.
     * @param [ in ] string - String containing property key.
     * @param [ in ] input0 - Destination object.
     */
    Instruction CreateStownbynamewithnameset(Instruction acc, std::string_view string, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode LDBIGINT. Loads the BigInt instance defined by the `string`.
     * @return Instruction
     * @param [ in ] string - String containing value.
     */
    Instruction CreateLdbigint(std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode LDTHISBYNAME. Loads this 's property of the key `string`, and stores it in
     * `acc`.
     * @return Instruction
     * @param [ in ] string - String containing the key.
     */
    Instruction CreateLdthisbyname(std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode STTHISBYNAME. Stores `acc` to this's property of the key `string`.
     * @return Instruction
     * @param [ in ] acc - Inst containing attribute key value.
     * @param [ in ] string - String containing the key.
     */
    Instruction CreateStthisbyname(Instruction acc, std::string_view string) &&;

    /**
     * @brief Creates instruction with opcode LDTHISBYVALUE. Loads this's property of the key `acc`, and stores it in
     * `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing attribute key value.
     */
    Instruction CreateLdthisbyvalue(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode STTHISBYVALUE. Stores `acc` to this's property of the key `input0`.
     * @return Instruction
     * @param [ in ] acc - Inst containing value.
     * @param [ in ] input0  - Inst containing attribute key value.
     */
    Instruction CreateStthisbyvalue(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode WIDE_LDPATCHVAR. Load` the patch variable in the `imm0`-th slot into
     * `acc`.
     * @return Instruction
     * @param [ in ] imm0 - Long value containing patch variable index.
     */
    Instruction CreateWideLdpatchvar(uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode WIDE_STPATCHVAR. Stores `acc` to the patch variable at the `imm0`-th slot.
     * @return Instruction
     * @param [ in ] acc - Inst containing value.
     * @param [ in ] imm0 - Long value containing patch variable index.
     */
    Instruction CreateWideStpatchvar(Instruction acc, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode DYNAMICIMPORT. Executes ImportCall with argument `acc`, and stores the
     * result in `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing argument for ImportCall.
     */
    Instruction CreateDynamicimport(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode ASYNCGENERATORREJECT. Executes the abstract operation AsyncGeneratorReject
     * with generator `input0` and exception `acc`.
     * @return Instruction
     * @param [ in ] acc - Inst containing exception object.
     * @param [ in ] input0 - Inst containing generator object.
     */
    Instruction CreateAsyncgeneratorreject(Instruction acc, Instruction input0) &&;

    /**
     * @brief Creates instruction with opcode SETGENERATORSTATE. Sets the state of acc as B.
     * @return Instruction
     * @param [ in ] acc - Inst containing generator object.
     * @param [ in ] imm0 - Long value that is generator status.
     */
    Instruction CreateSetgeneratorstate(Instruction acc, uint64_t imm0) &&;

    /**
     * @brief Creates instruction with opcode RETURN. This instruction returns `acc`.
     * @return Instruction
     * @param [ in ] acc - Instruction to be returned.
     */
    Instruction CreateReturn(Instruction acc) &&;

    /**
     * @brief Creates instruction with opcode RETURNUNDEFINED.
     * @return Instruction
     */
    Instruction CreateReturnundefined() &&;

    /**
     * @brief Creates instruction with opcode IF.
     * @return Instruction
     * @param [ in ] acc - Instruction that will be compared to zero.
     * @param [ in ] cc - Condition code.
     */
    Instruction CreateIf(Instruction acc, enum AbckitIsaApiDynamicConditionCode cc) &&;

private:
    explicit DynamicIsa(const Graph &graph) : graph_(graph) {};
    const Graph &graph_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_DYNAMIC_ISA_H
