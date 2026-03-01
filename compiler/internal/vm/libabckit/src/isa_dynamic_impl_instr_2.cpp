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

#include "isa_dynamic_impl_instr.h"
#include "scoped_timer.h"

namespace libabckit {

extern "C" AbckitIsaApiDynamicOpcode IgetDYNAMICOpcode(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, ABCKIT_ISA_API_DYNAMIC_OPCODE_INVALID);

    return IgetOpcodeDynamicStatic(inst);
}

extern "C" AbckitInst *IcreateDYNAMICLoadString(AbckitGraph *graph, AbckitString *str)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr)
    LIBABCKIT_BAD_ARGUMENT(str, nullptr)
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateLoadStringStatic(graph, str);
}

extern "C" AbckitInst *IcreateDYNAMICLdnan(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdnanStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdinfinity(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdinfinityStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdundefined(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdundefinedStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdnull(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdnullStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdsymbol(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdsymbolStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdglobal(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdglobalStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdtrue(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdtrueStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdfalse(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdfalseStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdhole(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdholeStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdnewtarget(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdnewtargetStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdthis(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdthisStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICPoplexenv(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynPoplexenvStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICGetunmappedargs(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynGetunmappedargsStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionenter(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynAsyncfunctionenterStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICLdfunction(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdfunctionStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICDebugger(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynDebuggerStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICGetpropiterator(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGetpropiteratorStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICGetiterator(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGetiteratorStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICGetasynciterator(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGetasynciteratorStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICLdprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                       uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynLdprivatepropertyStatic(graph, acc, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICStprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                       uint64_t imm1, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynStprivatepropertyStatic(graph, acc, imm0, imm1, input0);
}

extern "C" AbckitInst *IcreateDYNAMICTestin(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynTestinStatic(graph, acc, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICDefinefieldbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                       AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDefinefieldbynameStatic(graph, acc, string, input0);
}

extern "C" AbckitInst *IcreateDYNAMICDefinepropertybyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                          AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDefinepropertybynameStatic(graph, acc, string, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCreateemptyobject(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCreateemptyobjectStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICCreateemptyarray(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCreateemptyarrayStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICCreategeneratorobj(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCreategeneratorobjStatic(graph, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCreateiterresultobj(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCreateiterresultobjStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0,
                                                                  AbckitInst *input1, uint64_t imm0, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, imm0);

    auto *inst = IcreateDynCreateobjectwithexcludedkeysStatic(graph, input0, input1, imm0, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICWideCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0,
                                                                      AbckitInst *input1, uint64_t imm0, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, imm0);

    auto *inst = IcreateDynWideCreateobjectwithexcludedkeysStatic(graph, input0, input1, imm0, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICCreatearraywithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(literalArray, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCreatearraywithbufferStatic(graph, literalArray);
}

extern "C" AbckitInst *IcreateDYNAMICCreateobjectwithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(literalArray, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCreateobjectwithbufferStatic(graph, literalArray);
}

extern "C" AbckitInst *IcreateDYNAMICNewobjapply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynNewobjapplyStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICNewobjrange(AbckitGraph *graph, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr)
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr)

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynNewobjrangeStatic(graph, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICWideNewobjrange(AbckitGraph *graph, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynWideNewobjrangeStatic(graph, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICNewlexenv(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynNewlexenvStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICWideNewlexenv(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynWideNewlexenvStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICNewlexenvwithname(AbckitGraph *graph, uint64_t imm0,
                                                       AbckitLiteralArray *literalArray)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(literalArray, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynNewlexenvwithnameStatic(graph, imm0, literalArray);
}

extern "C" AbckitInst *IcreateDYNAMICWideNewlexenvwithname(AbckitGraph *graph, uint64_t imm0,
                                                           AbckitLiteralArray *literalArray)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(literalArray, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynWideNewlexenvwithnameStatic(graph, imm0, literalArray);
}

extern "C" AbckitInst *IcreateDYNAMICCreateasyncgeneratorobj(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCreateasyncgeneratorobjStatic(graph, input0);
}

extern "C" AbckitInst *IcreateDYNAMICAsyncgeneratorresolve(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                           AbckitInst *input2)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input2, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input2->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynAsyncgeneratorresolveStatic(graph, input0, input1, input2);
}

extern "C" AbckitInst *IcreateDYNAMICAdd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynAdd2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICSub2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynSub2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICMul2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynMul2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICDiv2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDiv2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICMod2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynMod2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICEq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynEqStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICNoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynNoteqStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICLess(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynLessStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICLesseq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynLesseqStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICGreater(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGreaterStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICGreatereq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGreatereqStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICShl2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynShl2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICShr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynShr2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICAshr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynAshr2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICAnd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynAnd2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICOr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynOr2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICXor2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynXor2Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICExp(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynExpStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICTypeof(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynTypeofStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICTonumber(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynTonumberStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICTonumeric(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynTonumericStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICNeg(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynNegStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICNot(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynNotStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICInc(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynIncStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICDec(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDecStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICIstrue(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynIstrueStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICIsfalse(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynIsfalseStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICIsin(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynIsinStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICInstanceof(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynInstanceofStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICStrictnoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynStrictnoteqStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICStricteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynStricteqStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeNotifyconcurrentresult(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeNotifyconcurrentresultStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefinefieldbyvalue(AbckitGraph *graph, AbckitInst *acc,
                                                                   AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeDefinefieldbyvalueStatic(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefinefieldbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                                   AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeDefinefieldbyindexStatic(graph, acc, imm0, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeTopropertykey(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeTopropertykeyStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeCreateprivateproperty(AbckitGraph *graph, uint64_t imm0,
                                                                      AbckitLiteralArray *literalArray)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(literalArray, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeCreateprivatepropertyStatic(graph, imm0, literalArray);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefineprivateproperty(AbckitGraph *graph, AbckitInst *acc,
                                                                      uint64_t imm0, uint64_t imm1, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeDefineprivatepropertyStatic(graph, acc, imm0, imm1, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeCallinit(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeCallinitStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeDefinesendableclass(AbckitGraph *graph, AbckitCoreFunction *function,
                                                                    AbckitLiteralArray *literalArray, uint64_t imm0,
                                                                    AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(function, nullptr);
    LIBABCKIT_BAD_ARGUMENT(literalArray, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);

    LIBABCKIT_WRONG_CTX(graph->file, function->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeDefinesendableclassStatic(graph, function, literalArray, imm0, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeLdsendableclass(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCallruntimeLdsendableclassStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeLdsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCallruntimeLdsendableexternalmodulevarStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeWideldsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCallruntimeWideldsendableexternalmodulevarStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeNewsendableenv(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCallruntimeNewsendableenvStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeWidenewsendableenv(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCallruntimeWidenewsendableenvStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeStsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                              uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeStsendablevarStatic(graph, acc, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeWidestsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                                  uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeWidestsendablevarStatic(graph, acc, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeLdsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCallruntimeLdsendablevarStatic(graph, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeWideldsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCallruntimeWideldsendablevarStatic(graph, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeIstrue(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeIstrueStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICCallruntimeIsfalse(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallruntimeIsfalseStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICThrow(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynThrowStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICThrowNotexists(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynThrowNotexistsStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICThrowPatternnoncoercible(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynThrowPatternnoncoercibleStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICThrowDeletesuperproperty(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynThrowDeletesuperpropertyStatic(graph);
}

}  // namespace libabckit
