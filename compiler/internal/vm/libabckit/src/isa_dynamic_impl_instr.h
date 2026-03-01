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

#ifndef LIBABCKIT_ISA_DYNAMIC_IMPL_INSTR_H
#define LIBABCKIT_ISA_DYNAMIC_IMPL_INSTR_H

#include <cassert>

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/ir_static.h"

#include "libabckit/src/macros.h"

#include <iostream>

namespace libabckit {

extern "C" AbckitIsaApiDynamicOpcode IgetDYNAMICOpcode(AbckitInst *inst);
extern "C" AbckitInst *IcreateDYNAMICLoadString(AbckitGraph *graph, AbckitString *str);
extern "C" AbckitInst *IcreateDYNAMICLdnan(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdinfinity(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdundefined(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdnull(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdsymbol(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdglobal(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdtrue(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdfalse(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdhole(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdnewtarget(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdthis(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICPoplexenv(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICGetunmappedargs(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionenter(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICLdfunction(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICDebugger(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICGetpropiterator(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICGetiterator(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICGetasynciterator(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICLdprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                       uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICStprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                       uint64_t imm1, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICTestin(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICDefinefieldbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                       AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICDefinepropertybyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                          AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCreateemptyobject(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICCreateemptyarray(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICCreategeneratorobj(AbckitGraph *graph, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCreateiterresultobj(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0,
                                                                  AbckitInst *input1, uint64_t imm0, ...);
extern "C" AbckitInst *IcreateDYNAMICWideCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0,
                                                                      AbckitInst *input1, uint64_t imm0, ...);
extern "C" AbckitInst *IcreateDYNAMICCreatearraywithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray);
extern "C" AbckitInst *IcreateDYNAMICCreateobjectwithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray);
extern "C" AbckitInst *IcreateDYNAMICNewobjapply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICNewobjrange(AbckitGraph *graph, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICWideNewobjrange(AbckitGraph *graph, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICNewlexenv(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICWideNewlexenv(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICNewlexenvwithname(AbckitGraph *graph, uint64_t imm0,
                                                       AbckitLiteralArray *literalArray);
extern "C" AbckitInst *IcreateDYNAMICWideNewlexenvwithname(AbckitGraph *graph, uint64_t imm0,
                                                           AbckitLiteralArray *literalArray);
extern "C" AbckitInst *IcreateDYNAMICCreateasyncgeneratorobj(AbckitGraph *graph, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICAsyncgeneratorresolve(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                           AbckitInst *input2);
extern "C" AbckitInst *IcreateDYNAMICAdd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICSub2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICMul2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICDiv2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICMod2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICEq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICNoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICLess(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICLesseq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICGreater(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICGreatereq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICShl2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICShr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICAshr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICAnd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICOr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICXor2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICExp(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICTypeof(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICTonumber(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICTonumeric(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICNeg(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICNot(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICInc(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICDec(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICIstrue(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICIsfalse(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICIsin(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICInstanceof(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICStrictnoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICStricteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeNotifyconcurrentresult(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefinefieldbyvalue(AbckitGraph *graph, AbckitInst *acc,
                                                                   AbckitInst *input0, AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefinefieldbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                                   AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeTopropertykey(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeCreateprivateproperty(AbckitGraph *graph, uint64_t imm0,
                                                                      AbckitLiteralArray *literalArray);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefineprivateproperty(AbckitGraph *graph, AbckitInst *acc,
                                                                      uint64_t imm0, uint64_t imm1, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeCallinit(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefinesendableclass(AbckitGraph *graph, AbckitCoreFunction *function,
                                                                    AbckitLiteralArray *literalArray, uint64_t imm0,
                                                                    AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeLdsendableclass(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeLdsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeWideldsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeNewsendableenv(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeWidenewsendableenv(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeStsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                              uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeWidestsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                                  uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeLdsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeWideldsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeIstrue(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICCallruntimeIsfalse(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICThrow(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICThrowNotexists(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICThrowPatternnoncoercible(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICThrowDeletesuperproperty(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICThrowConstassignment(AbckitGraph *graph, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICThrowIfnotobject(AbckitGraph *graph, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICThrowUndefinedifhole(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICThrowIfsupernotcorrectcall(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICThrowUndefinedifholewithname(AbckitGraph *graph, AbckitInst *acc,
                                                                  AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICCallarg0(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICCallarg1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCallargs2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICCallargs3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1, AbckitInst *input2);
extern "C" AbckitInst *IcreateDYNAMICCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICWideCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICSupercallspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICApply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICCallthis0(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCallthis1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICCallthis2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1, AbckitInst *input2);
// CC-OFFNXT(G.FUN.01-CPP) API function
extern "C" AbckitInst *IcreateDYNAMICCallthis3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1, AbckitInst *input2, AbckitInst *input3);
extern "C" AbckitInst *IcreateDYNAMICCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICWideCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICSupercallthisrange(AbckitGraph *graph, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICWideSupercallthisrange(AbckitGraph *graph, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);
extern "C" AbckitInst *IcreateDYNAMICWideSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...);
// CC-OFFNXT(G.FUN.01-CPP) API function
extern "C" AbckitInst *IcreateDYNAMICDefinegettersetterbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                               AbckitInst *input1, AbckitInst *input2,
                                                               AbckitInst *input3);
extern "C" AbckitInst *IcreateDYNAMICDefinefunc(AbckitGraph *graph, AbckitCoreFunction *function, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICDefinemethod(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *function,
                                                  uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICDefineclasswithbuffer(AbckitGraph *graph, AbckitCoreFunction *function,
                                                           AbckitLiteralArray *literalArray, uint64_t imm0,
                                                           AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICResumegenerator(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICGetresumemode(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICGettemplateobject(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICGetnextpropname(AbckitGraph *graph, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICDelobjprop(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICSuspendgenerator(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionawaituncaught(AbckitGraph *graph, AbckitInst *acc,
                                                                AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCopydataproperties(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICStarrayspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                   AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICSetobjectwithproto(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICLdobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICStobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICStownbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICLdsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICStsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                    AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICWideLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICWideStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                      uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICWideStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                      uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionresolve(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICCopyrestargs(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICWideCopyrestargs(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICWideLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICWideStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);
extern "C" AbckitInst *IcreateDYNAMICGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md);
extern "C" AbckitInst *IcreateDYNAMICWideGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md);
extern "C" AbckitInst *IcreateDYNAMICStmodulevar(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed);
extern "C" AbckitInst *IcreateDYNAMICWideStmodulevar(AbckitGraph *graph, AbckitInst *acc,
                                                     AbckitCoreExportDescriptor *ed);
extern "C" AbckitInst *IcreateDYNAMICTryldglobalbyname(AbckitGraph *graph, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICTrystglobalbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICLdglobalvar(AbckitGraph *graph, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICStglobalvar(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICLdobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICStobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                 AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICStownbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                 AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICLdsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICStsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                   AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed);
extern "C" AbckitInst *IcreateDYNAMICWideLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed);
extern "C" AbckitInst *IcreateDYNAMICLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id);
extern "C" AbckitInst *IcreateDYNAMICWideLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id);
extern "C" AbckitInst *IcreateDYNAMICStownbyvaluewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                             AbckitInst *input1);
extern "C" AbckitInst *IcreateDYNAMICStownbynamewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                            AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICLdbigint(AbckitGraph *graph, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICLdthisbyname(AbckitGraph *graph, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICStthisbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
extern "C" AbckitInst *IcreateDYNAMICLdthisbyvalue(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICStthisbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICWideLdpatchvar(AbckitGraph *graph, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICWideStpatchvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICDynamicimport(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICAsyncgeneratorreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
extern "C" AbckitInst *IcreateDYNAMICSetgeneratorstate(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
extern "C" AbckitInst *IcreateDYNAMICReturn(AbckitGraph *graph, AbckitInst *acc);
extern "C" AbckitInst *IcreateDYNAMICReturnundefined(AbckitGraph *graph);
extern "C" AbckitInst *IcreateDYNAMICIf(AbckitGraph *graph, AbckitInst *input, AbckitIsaApiDynamicConditionCode cc);
extern "C" AbckitIsaApiDynamicConditionCode IgetDYNAMICConditionCode(AbckitInst *inst);
extern "C" void IsetDYNAMICConditionCode(AbckitInst *inst, AbckitIsaApiDynamicConditionCode cc);
extern "C" AbckitCoreImportDescriptor *IgetImportDescriptor(AbckitInst *inst);
extern "C" void IsetImportDescriptor(AbckitInst *inst, AbckitCoreImportDescriptor *id);
extern "C" AbckitCoreExportDescriptor *IgetExportDescriptor(AbckitInst *inst);
extern "C" void IsetExportDescriptor(AbckitInst *inst, AbckitCoreExportDescriptor *ed);
extern "C" AbckitCoreModule *IgetModule(AbckitInst *inst);
extern "C" void IsetModule(AbckitInst *inst, AbckitCoreModule *md);
}  // namespace libabckit

#endif
