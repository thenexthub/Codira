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

#ifndef CPP_ABCKIT_DYNAMIC_ISA_IMPL_H
#define CPP_ABCKIT_DYNAMIC_ISA_IMPL_H

#include "graph.h"
#include "file.h"
#include "literal_array.h"
#include "core/import_descriptor.h"
#include "core/module.h"
#include "config.h"

#include <memory>
#include <type_traits>

// NOLINTBEGIN(performance-unnecessary-value-param)
namespace abckit {

inline core::Module DynamicIsa::GetModule(Instruction inst) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitCoreModule *mdl = conf->cDynapi_->iGetModule(inst.GetView());
    CheckError(conf);
    return core::Module(mdl, conf, graph_.GetFile());
}

inline Instruction DynamicIsa::SetModule(Instruction inst, core::Module mdl) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    conf->cDynapi_->iSetModule(inst.GetView(), mdl.GetView());
    CheckError(conf);
    return inst;
}

inline AbckitIsaApiDynamicConditionCode DynamicIsa::GetConditionCode(Instruction inst) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitIsaApiDynamicConditionCode cc = conf->cDynapi_->iGetConditionCode(inst.GetView());
    CheckError(conf);
    return cc;
}

inline Instruction DynamicIsa::SetConditionCode(Instruction inst, AbckitIsaApiDynamicConditionCode cc) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    conf->cDynapi_->iSetConditionCode(inst.GetView(), cc);
    CheckError(conf);
    return inst;
}

inline AbckitIsaApiDynamicOpcode DynamicIsa::GetOpcode(Instruction inst) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitIsaApiDynamicOpcode opc = conf->cDynapi_->iGetOpcode(inst.GetView());
    CheckError(conf);
    return opc;
}

inline core::ImportDescriptor DynamicIsa::GetImportDescriptor(Instruction inst) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitCoreImportDescriptor *descr = conf->cDynapi_->iGetImportDescriptor(inst.GetView());
    CheckError(conf);
    return core::ImportDescriptor(descr, conf, graph_.GetFile());
}

inline Instruction DynamicIsa::SetImportDescriptor(Instruction inst, core::ImportDescriptor descr) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    conf->cDynapi_->iSetImportDescriptor(inst.GetView(), descr.GetView());
    CheckError(conf);
    return inst;
}

inline core::ExportDescriptor DynamicIsa::GetExportDescriptor(Instruction inst) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitCoreExportDescriptor *descr = conf->cDynapi_->iGetExportDescriptor(inst.GetView());
    CheckError(conf);
    return core::ExportDescriptor(descr, conf, graph_.GetFile());
}

inline Instruction DynamicIsa::SetExportDescriptor(Instruction inst, core::ExportDescriptor descr) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    conf->cDynapi_->iSetExportDescriptor(inst.GetView(), descr.GetView());
    CheckError(conf);
    return inst;
}

inline Instruction DynamicIsa::CreateLoadString(std::string_view string) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *abcLoadstring = conf->cDynapi_->iCreateLoadString(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(abcLoadstring, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode NEWOBJRANGE.
 * @return `Instruction`
 * @param [ in ] input0 - Class object.
 * @param [ in ] args - Number of insts containing arguments.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateNewobjrange(Instruction input0, Args... args) &&
{
    static_assert((... && std::is_convertible_v<Args, Instruction>),
                  "Expected abckit::Instruction instances at `instArgs` for CreateNewobjrange");

    auto *conf = graph_.GetApiConfig();
    AbckitInst *objrange = conf->cDynapi_->iCreateNewobjrange(graph_.GetResource(), 1 + sizeof...(args),
                                                              input0.GetView(), args.GetView()...);
    CheckError(conf);
    return Instruction(objrange, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode NEWOBJRANGE.
 * @return `Instruction`
 * @param [ in ] input0 - Class object.
 * @param [ in ] args - Number of insts containing arguments.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateWideNewobjrange(Instruction input0, Args... args) &&
{
    static_assert((... && std::is_convertible_v<Args, Instruction>),
                  "Expected abckit::Instruction instances at `instArgs` for CreateWideNewobjrange");

    auto *conf = graph_.GetApiConfig();
    AbckitInst *objrange = conf->cDynapi_->iCreateWideNewobjrange(graph_.GetResource(), 1 + sizeof...(args),
                                                                  input0.GetView(), args.GetView()...);
    CheckError(conf);
    return Instruction(objrange, conf, &graph_);
}

inline Instruction DynamicIsa::CreateSub2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *subInst = conf->cDynapi_->iCreateSub2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(subInst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdnan() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdnan(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdinfinity() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdinfinity(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdundefined() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdundefined(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdnull() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdnull(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdsymbol() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdsymbol(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdglobal() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdglobal(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdtrue() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdtrue(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdfalse() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdfalse(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdhole() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdhole(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdnewtarget() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdnewtarget(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdthis() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdthis(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreatePoplexenv() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreatePoplexenv(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGetunmappedargs() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGetunmappedargs(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAsyncfunctionenter() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateAsyncfunctionenter(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdfunction() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdfunction(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDebugger() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDebugger(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGetpropiterator(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGetpropiterator(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGetiterator(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGetiterator(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGetasynciterator(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGetasynciterator(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdprivateproperty(Instruction acc, uint64_t imm0, uint64_t imm1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdprivateproperty(graph_.GetResource(), acc.GetView(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStprivateproperty(Instruction acc, uint64_t imm0, uint64_t imm1,
                                                       Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateStprivateproperty(graph_.GetResource(), acc.GetView(), imm0, imm1, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateTestin(Instruction acc, uint64_t imm0, uint64_t imm1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateTestin(graph_.GetResource(), acc.GetView(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDefinefieldbyname(Instruction acc, std::string_view string, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst =
        conf->cDynapi_->iCreateDefinefieldbyname(graph_.GetResource(), acc.GetView(), abcStr, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDefinepropertybyname(Instruction acc, std::string_view string,
                                                          Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst =
        conf->cDynapi_->iCreateDefinepropertybyname(graph_.GetResource(), acc.GetView(), abcStr, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreateemptyobject() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCreateemptyobject(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreateemptyarray() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCreateemptyarray(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreategeneratorobj(Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCreategeneratorobj(graph_.GetResource(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreateiterresultobj(Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateCreateiterresultobj(graph_.GetResource(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode CREATEOBJECTWITHEXCLUDEDKEYS.
 * @return `Instruction`
 * @param [ in ] input0 - Inst containing object.
 * @param [ in ] input1 - Inst containing first `key`.
 * @param [ in ] imm0 - Number of optional insts containing `key`s.
 * @param [ in ] args - Optional insts containing `key`s.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateCreateobjectwithexcludedkeys(Instruction input0, Instruction input1, uint64_t imm0,
                                                                  Args... args) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCreateobjectwithexcludedkeys(graph_.GetResource(), input0.GetView(),
                                                                           input1.GetView(), imm0, (args.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode WIDE_CREATEOBJECTWITHEXCLUDEDKEYS.
 * @return `Instruction`
 * @param [ in ] input0 - Inst containing object.
 * @param [ in ] input1 - Inst containing first `key`.
 * @param [ in ] imm0 - Number of optional insts containing `key`s.
 * @param [ in ] args - Optional insts containing `key`s.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateWideCreateobjectwithexcludedkeys(Instruction input0, Instruction input1,
                                                                      uint64_t imm0, Args... args) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideCreateobjectwithexcludedkeys(
        graph_.GetResource(), input0.GetView(), input1.GetView(), imm0, (args.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreatearraywithbuffer(LiteralArray literalArray) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCreatearraywithbuffer(graph_.GetResource(), literalArray.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreateobjectwithbuffer(LiteralArray literalArray) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCreateobjectwithbuffer(graph_.GetResource(), literalArray.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateNewobjapply(Instruction acc, Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateNewobjapply(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateNewlexenv(uint64_t imm0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateNewlexenv(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideNewlexenv(uint64_t imm0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideNewlexenv(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateNewlexenvwithname(uint64_t imm0, LiteralArray literalArray) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateNewlexenvwithname(graph_.GetResource(), imm0, literalArray.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideNewlexenvwithname(uint64_t imm0, LiteralArray literalArray) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideNewlexenvwithname(graph_.GetResource(), imm0, literalArray.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCreateasyncgeneratorobj(Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCreateasyncgeneratorobj(graph_.GetResource(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAsyncgeneratorresolve(Instruction input0, Instruction input1,
                                                           Instruction input2) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateAsyncgeneratorresolve(graph_.GetResource(), input0.GetView(),
                                                                    input1.GetView(), input2.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAdd2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateAdd2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateMul2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateMul2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDiv2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDiv2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateMod2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateMod2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateEq(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateEq(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateNoteq(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateNoteq(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLess(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLess(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLesseq(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLesseq(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGreater(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGreater(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGreatereq(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGreatereq(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateShl2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateShl2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateShr2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateShr2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAshr2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateAshr2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAnd2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateAnd2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateOr2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateOr2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateXor2(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateXor2(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateExp(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateExp(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateTypeof(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateTypeof(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateTonumber(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateTonumber(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateTonumeric(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateTonumeric(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateNeg(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateNeg(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateNot(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateNot(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateInc(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateInc(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDec(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDec(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateIstrue(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateIstrue(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateIsfalse(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateIsfalse(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateIsin(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateIsin(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateInstanceof(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateInstanceof(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStrictnoteq(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStrictnoteq(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStricteq(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStricteq(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeNotifyconcurrentresult(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeNotifyconcurrentresult(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeDefinefieldbyvalue(Instruction acc, Instruction input0,
                                                                   Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeDefinefieldbyvalue(graph_.GetResource(), acc.GetView(),
                                                                            input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeDefinefieldbyindex(Instruction acc, uint64_t imm0,
                                                                   Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeDefinefieldbyindex(graph_.GetResource(), acc.GetView(), imm0,
                                                                            input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeTopropertykey(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeTopropertykey(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeCreateprivateproperty(uint64_t imm0, LiteralArray literalArray) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateCallruntimeCreateprivateproperty(graph_.GetResource(), imm0, literalArray.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeDefineprivateproperty(Instruction acc, uint64_t imm0, uint64_t imm1,
                                                                      Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeDefineprivateproperty(graph_.GetResource(), acc.GetView(),
                                                                               imm0, imm1, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeCallinit(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateCallruntimeCallinit(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeDefinesendableclass(core::Function function, LiteralArray literalArray,
                                                                    uint64_t imm0, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeDefinesendableclass(
        graph_.GetResource(), function.GetView(), literalArray.GetView(), imm0, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeLdsendableclass(uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeLdsendableclass(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeLdsendableexternalmodulevar(uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeLdsendableexternalmodulevar(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeWideldsendableexternalmodulevar(uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeWideldsendableexternalmodulevar(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeNewsendableenv(uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeNewsendableenv(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeWidenewsendableenv(uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeWidenewsendableenv(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeStsendablevar(Instruction acc, uint64_t imm0, uint64_t imm1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeStsendablevar(graph_.GetResource(), acc.GetView(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeWidestsendablevar(Instruction acc, uint64_t imm0, uint64_t imm1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateCallruntimeWidestsendablevar(graph_.GetResource(), acc.GetView(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeLdsendablevar(uint64_t imm0, uint64_t imm1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeLdsendablevar(graph_.GetResource(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeWideldsendablevar(uint64_t imm0, uint64_t imm1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeWideldsendablevar(graph_.GetResource(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeIstrue(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeIstrue(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallruntimeIsfalse(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallruntimeIsfalse(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrow(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateThrow(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowNotexists() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateThrowNotexists(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowPatternnoncoercible() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateThrowPatternnoncoercible(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowDeletesuperproperty() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateThrowDeletesuperproperty(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowConstassignment(Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateThrowConstassignment(graph_.GetResource(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowIfnotobject(Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateThrowIfnotobject(graph_.GetResource(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowUndefinedifhole(Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateThrowUndefinedifhole(graph_.GetResource(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowIfsupernotcorrectcall(Instruction acc, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateThrowIfsupernotcorrectcall(graph_.GetResource(), acc.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateThrowUndefinedifholewithname(Instruction acc, std::string_view string) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateThrowUndefinedifholewithname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallarg0(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallarg0(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallarg1(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallarg1(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallargs2(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateCallargs2(graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallargs3(Instruction acc, Instruction input0, Instruction input1,
                                               Instruction input2) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallargs3(graph_.GetResource(), acc.GetView(), input0.GetView(),
                                                        input1.GetView(), input2.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode CALLRANGE.
 * @return `Instruction`
 * @param [ in ] acc - Inst containing function object.
 * @param [ in ] instrs - Arguments.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateCallrange(Instruction acc, Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateCallrange(graph_.GetResource(), acc.GetView(), sizeof...(instrs), (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode WIDE_CALLRANGE.
 * @return `Instruction`
 * @param [ in ] acc - Inst containing function object.
 * @param [ in ] instrs - Arguments.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateWideCallrange(Instruction acc, Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideCallrange(graph_.GetResource(), acc.GetView(), sizeof...(instrs),
                                                            (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateSupercallspread(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateSupercallspread(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateApply(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateApply(graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallthis0(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallthis0(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallthis1(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateCallthis1(graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallthis2(Instruction acc, Instruction input0, Instruction input1,
                                               Instruction input2) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallthis2(graph_.GetResource(), acc.GetView(), input0.GetView(),
                                                        input1.GetView(), input2.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCallthis3(Instruction acc, Instruction input0, Instruction input1,
                                               Instruction input2, Instruction input3) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallthis3(graph_.GetResource(), acc.GetView(), input0.GetView(),
                                                        input1.GetView(), input2.GetView(), input3.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode CALLTHISRANGE.
 * Sets the value of this as first variadic argument, invokes the function object stored in `acc` with arguments
 * `...`.
 * @return `Instruction`
 * @param [ in ] acc - Function object.
 * @param [ in ] instrs - Object + arguments.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateCallthisrange(Instruction acc, Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCallthisrange(graph_.GetResource(), acc.GetView(), sizeof...(instrs),
                                                            (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode WIDE_CALLTHISRANGE.
 * Sets the value of this as first variadic argument, invokes the function object stored in `acc` with arguments
 * `...`.
 * @return `Instruction`
 * @param [ in ] acc - Function object.
 * @param [ in ] instrs - Object + arguments.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateWideCallthisrange(Instruction acc, Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideCallthisrange(graph_.GetResource(), acc.GetView(), sizeof...(instrs),
                                                                (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode SUPERCALLTHISRANGE.
 * Invokes super with arguments ...
 * This instruction appears only in non-arrow functions.
 * @return `Instruction`
 * @param [ in ] instrs - Parameters.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateSupercallthisrange(Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateSupercallthisrange(graph_.GetResource(), sizeof...(instrs), (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode WIDE_SUPERCALLTHISRANGE.
 * Invokes super with arguments ...
 * This instruction appears only in non-arrow functions.
 * @return `Instruction`
 * @param [ in ] instrs - Parameters.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateWideSupercallthisrange(Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateWideSupercallthisrange(graph_.GetResource(), sizeof...(instrs), (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode SUPERCALLARROWRANGE.
 * Invokes `acc`'s superclass constructor with arguments `...`.
 * This instruction appears only in arrow functions.
 * @return `Instruction`
 * @param [ in ] acc - Class object.
 * @param [ in ] instrs - Parameters.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateSupercallarrowrange(Instruction acc, Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateSupercallarrowrange(graph_.GetResource(), acc.GetView(),
                                                                  sizeof...(instrs), (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

/**
 * @brief Creates instruction with opcode WIDE_SUPERCALLARROWRANGE.
 * Invokes `acc`'s superclass constructor with arguments `...`.
 * This instruction appears only in arrow functions.
 * @return `Instruction`
 * @param [ in ] acc - Class object.
 * @param [ in ] instrs - Parameters.
 */
template <typename... Args>
inline Instruction DynamicIsa::CreateWideSupercallarrowrange(Instruction acc, Args... instrs) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideSupercallarrowrange(graph_.GetResource(), acc.GetView(),
                                                                      sizeof...(instrs), (instrs.GetView())...);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDefinegettersetterbyvalue(Instruction acc, Instruction input0, Instruction input1,
                                                               Instruction input2, Instruction input3) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDefinegettersetterbyvalue(
        graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView(), input2.GetView(), input3.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDefinefunc(core::Function function, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDefinefunc(graph_.GetResource(), function.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDefinemethod(Instruction acc, core::Function function, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateDefinemethod(graph_.GetResource(), acc.GetView(), function.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDefineclasswithbuffer(core::Function function, LiteralArray literalArray,
                                                           uint64_t imm0, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDefineclasswithbuffer(graph_.GetResource(), function.GetView(),
                                                                    literalArray.GetView(), imm0, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateResumegenerator(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateResumegenerator(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGetresumemode(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGetresumemode(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGettemplateobject(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGettemplateobject(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGetnextpropname(Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGetnextpropname(graph_.GetResource(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDelobjprop(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDelobjprop(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateSuspendgenerator(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateSuspendgenerator(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAsyncfunctionawaituncaught(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateAsyncfunctionawaituncaught(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCopydataproperties(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCopydataproperties(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStarrayspread(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateStarrayspread(graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateSetobjectwithproto(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateSetobjectwithproto(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdobjbyvalue(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdobjbyvalue(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStobjbyvalue(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateStobjbyvalue(graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStownbyvalue(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateStownbyvalue(graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdsuperbyvalue(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdsuperbyvalue(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStsuperbyvalue(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateStsuperbyvalue(graph_.GetResource(), acc.GetView(), input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdobjbyindex(Instruction acc, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdobjbyindex(graph_.GetResource(), acc.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideLdobjbyindex(Instruction acc, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideLdobjbyindex(graph_.GetResource(), acc.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStobjbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStobjbyindex(graph_.GetResource(), acc.GetView(), input0.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideStobjbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateWideStobjbyindex(graph_.GetResource(), acc.GetView(), input0.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStownbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStownbyindex(graph_.GetResource(), acc.GetView(), input0.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideStownbyindex(Instruction acc, Instruction input0, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateWideStownbyindex(graph_.GetResource(), acc.GetView(), input0.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAsyncfunctionresolve(Instruction acc, Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateAsyncfunctionresolve(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAsyncfunctionreject(Instruction acc, Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateAsyncfunctionreject(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateCopyrestargs(uint64_t imm0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateCopyrestargs(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideCopyrestargs(uint64_t imm0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideCopyrestargs(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdlexvar(uint64_t imm0, uint64_t imm1) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdlexvar(graph_.GetResource(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideLdlexvar(uint64_t imm0, uint64_t imm1) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideLdlexvar(graph_.GetResource(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStlexvar(Instruction acc, uint64_t imm0, uint64_t imm1) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStlexvar(graph_.GetResource(), acc.GetView(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideStlexvar(Instruction acc, uint64_t imm0, uint64_t imm1) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideStlexvar(graph_.GetResource(), acc.GetView(), imm0, imm1);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateGetmodulenamespace(core::Module md) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateGetmodulenamespace(graph_.GetResource(), md.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideGetmodulenamespace(core::Module md) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideGetmodulenamespace(graph_.GetResource(), md.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStmodulevar(Instruction acc, core::ExportDescriptor ed) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStmodulevar(graph_.GetResource(), acc.GetView(), ed.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideStmodulevar(Instruction acc, core::ExportDescriptor ed) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideStmodulevar(graph_.GetResource(), acc.GetView(), ed.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateTryldglobalbyname(std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    // NOTE(urandon): resolve duplicates for string
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateTryldglobalbyname(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateTrystglobalbyname(Instruction acc, std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateTrystglobalbyname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdglobalvar(std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateLdglobalvar(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStglobalvar(Instruction acc, std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateStglobalvar(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdobjbyname(Instruction acc, std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateLdobjbyname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStobjbyname(Instruction acc, std::string_view string, Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst =
        conf->cDynapi_->iCreateStobjbyname(graph_.GetResource(), acc.GetView(), abcStr, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStownbyname(Instruction acc, std::string_view string, Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst =
        conf->cDynapi_->iCreateStownbyname(graph_.GetResource(), acc.GetView(), abcStr, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdsuperbyname(Instruction acc, std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateLdsuperbyname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStsuperbyname(Instruction acc, std::string_view string, Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst =
        conf->cDynapi_->iCreateStsuperbyname(graph_.GetResource(), acc.GetView(), abcStr, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdlocalmodulevar(core::ExportDescriptor ed) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdlocalmodulevar(graph_.GetResource(), ed.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideLdlocalmodulevar(core::ExportDescriptor ed) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideLdlocalmodulevar(graph_.GetResource(), ed.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdexternalmodulevar(core::ImportDescriptor id) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdexternalmodulevar(graph_.GetResource(), id.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideLdexternalmodulevar(core::ImportDescriptor id) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideLdexternalmodulevar(graph_.GetResource(), id.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStownbyvaluewithnameset(Instruction acc, Instruction input0, Instruction input1) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStownbyvaluewithnameset(graph_.GetResource(), acc.GetView(),
                                                                      input0.GetView(), input1.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStownbynamewithnameset(Instruction acc, std::string_view string,
                                                            Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst =
        conf->cDynapi_->iCreateStownbynamewithnameset(graph_.GetResource(), acc.GetView(), abcStr, input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdbigint(std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateLdbigint(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdthisbyname(std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateLdthisbyname(graph_.GetResource(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStthisbyname(Instruction acc, std::string_view string) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitString *abcStr = conf->cMapi_->createString(graph_.GetFile()->GetResource(), string.data(), string.size());
    CheckError(conf);
    AbckitInst *inst = conf->cDynapi_->iCreateStthisbyname(graph_.GetResource(), acc.GetView(), abcStr);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateLdthisbyvalue(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateLdthisbyvalue(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateStthisbyvalue(Instruction acc, Instruction input0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateStthisbyvalue(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideLdpatchvar(uint64_t imm0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideLdpatchvar(graph_.GetResource(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateWideStpatchvar(Instruction acc, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateWideStpatchvar(graph_.GetResource(), acc.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateDynamicimport(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateDynamicimport(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateAsyncgeneratorreject(Instruction acc, Instruction input0) &&
{
    const ApiConfig *conf = graph_.GetApiConfig();
    AbckitInst *inst =
        conf->cDynapi_->iCreateAsyncgeneratorreject(graph_.GetResource(), acc.GetView(), input0.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateSetgeneratorstate(Instruction acc, uint64_t imm0) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateSetgeneratorstate(graph_.GetResource(), acc.GetView(), imm0);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateReturn(Instruction acc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateReturn(graph_.GetResource(), acc.GetView());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateReturnundefined() &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateReturnundefined(graph_.GetResource());
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

inline Instruction DynamicIsa::CreateIf(Instruction acc, enum AbckitIsaApiDynamicConditionCode cc) &&
{
    auto *conf = graph_.GetApiConfig();
    AbckitInst *inst = conf->cDynapi_->iCreateIf(graph_.GetResource(), acc.GetView(), cc);
    CheckError(conf);
    return Instruction(inst, conf, &graph_);
}

}  // namespace abckit

// NOLINTEND(performance-unnecessary-value-param)

#endif  // CPP_ABCKIT_DYNAMIC_ISA_IMPL_H
