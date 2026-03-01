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

extern "C" AbckitInst *IcreateDYNAMICThrowConstassignment(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynThrowConstassignmentStatic(graph, input0);
}

extern "C" AbckitInst *IcreateDYNAMICThrowIfnotobject(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynThrowIfnotobjectStatic(graph, input0);
}

extern "C" AbckitInst *IcreateDYNAMICThrowUndefinedifhole(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
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
    return IcreateDynThrowUndefinedifholeStatic(graph, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICThrowIfsupernotcorrectcall(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynThrowIfsupernotcorrectcallStatic(graph, acc, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICThrowUndefinedifholewithname(AbckitGraph *graph, AbckitInst *acc,
                                                                  AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynThrowUndefinedifholewithnameStatic(graph, acc, string);
}

extern "C" AbckitInst *IcreateDYNAMICCallarg0(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallarg0Static(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICCallarg1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynCallarg1Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCallargs2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1)
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
    return IcreateDynCallargs2Static(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICCallargs3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1, AbckitInst *input2)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input2, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input2->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallargs3Static(graph, acc, input0, input1, input2);
}

extern "C" AbckitInst *IcreateDYNAMICCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);
    auto *inst = IcreateDynCallrangeStatic(graph, acc, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICWideCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynWideCallrangeStatic(graph, acc, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICSupercallspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynSupercallspreadStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICApply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
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
    return IcreateDynApplyStatic(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICCallthis0(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynCallthis0Static(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCallthis1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1)
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
    return IcreateDynCallthis1Static(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICCallthis2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1, AbckitInst *input2)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input2, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input2->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallthis2Static(graph, acc, input0, input1, input2);
}

// CC-OFFNXT(G.FUN.01) This is function from public API
extern "C" AbckitInst *IcreateDYNAMICCallthis3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                               AbckitInst *input1, AbckitInst *input2, AbckitInst *input3)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input2, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input3, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input2->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input3->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynCallthis3Static(graph, acc, input0, input1, input2, input3);
}

extern "C" AbckitInst *IcreateDYNAMICCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynCallthisrangeStatic(graph, acc, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICWideCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynWideCallthisrangeStatic(graph, acc, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICSupercallthisrange(AbckitGraph *graph, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynSupercallthisrangeStatic(graph, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICWideSupercallthisrange(AbckitGraph *graph, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynWideSupercallthisrangeStatic(graph, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);

    auto *inst = IcreateDynSupercallarrowrangeStatic(graph, acc, argCount, args);
    va_end(args);
    return inst;
}

extern "C" AbckitInst *IcreateDYNAMICWideSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    std::va_list args;
    va_start(args, argCount);
    auto *inst = IcreateDynWideSupercallarrowrangeStatic(graph, acc, argCount, args);
    va_end(args);
    return inst;
}

// CC-OFFNXT(G.FUN.01) This is function from public API
extern "C" AbckitInst *IcreateDYNAMICDefinegettersetterbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                               AbckitInst *input1, AbckitInst *input2,
                                                               AbckitInst *input3)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input1, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input2, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input3, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input1->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input2->graph, nullptr);
    LIBABCKIT_WRONG_CTX(graph, input3->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDefinegettersetterbyvalueStatic(graph, acc, input0, input1, input2, input3);
}

extern "C" AbckitInst *IcreateDYNAMICDefinefunc(AbckitGraph *graph, AbckitCoreFunction *function, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(function, nullptr);
    LIBABCKIT_INTERNAL_ERROR(function->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, function->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDefinefuncStatic(graph, function, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICDefinemethod(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *function,
                                                  uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(function, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_INTERNAL_ERROR(function->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, function->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDefinemethodStatic(graph, acc, function, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICDefineclasswithbuffer(AbckitGraph *graph, AbckitCoreFunction *function,
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
    LIBABCKIT_INTERNAL_ERROR(function->owningModule, nullptr);
    LIBABCKIT_WRONG_CTX(graph->file, function->owningModule->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDefineclasswithbufferStatic(graph, function, literalArray, imm0, input0);
}

extern "C" AbckitInst *IcreateDYNAMICResumegenerator(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynResumegeneratorStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICGetresumemode(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGetresumemodeStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICGettemplateobject(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGettemplateobjectStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICGetnextpropname(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input0, nullptr);

    LIBABCKIT_WRONG_CTX(graph, input0->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGetnextpropnameStatic(graph, input0);
}

extern "C" AbckitInst *IcreateDYNAMICDelobjprop(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynDelobjpropStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICSuspendgenerator(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynSuspendgeneratorStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionawaituncaught(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynAsyncfunctionawaituncaughtStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCopydataproperties(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynCopydatapropertiesStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICStarrayspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                   AbckitInst *input1)
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
    return IcreateDynStarrayspreadStatic(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICSetobjectwithproto(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynSetobjectwithprotoStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICLdobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynLdobjbyvalueStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICStobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  AbckitInst *input1)
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
    return IcreateDynStobjbyvalueStatic(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICStownbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  AbckitInst *input1)
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
    return IcreateDynStownbyvalueStatic(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICLdsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynLdsuperbyvalueStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICStsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                    AbckitInst *input1)
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
    return IcreateDynStsuperbyvalueStatic(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynLdobjbyindexStatic(graph, acc, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICWideLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynWideLdobjbyindexStatic(graph, acc, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  uint64_t imm0)
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
    return IcreateDynStobjbyindexStatic(graph, acc, input0, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICWideStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                      uint64_t imm0)
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
    return IcreateDynWideStobjbyindexStatic(graph, acc, input0, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                  uint64_t imm0)
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
    return IcreateDynStownbyindexStatic(graph, acc, input0, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICWideStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                      uint64_t imm0)
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
    return IcreateDynWideStownbyindexStatic(graph, acc, input0, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionresolve(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynAsyncfunctionresolveStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICAsyncfunctionreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynAsyncfunctionrejectStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICCopyrestargs(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynCopyrestargsStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICWideCopyrestargs(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynWideCopyrestargsStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdlexvarStatic(graph, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICWideLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynWideLdlexvarStatic(graph, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynStlexvarStatic(graph, acc, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICWideStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynWideStlexvarStatic(graph, acc, imm0, imm1);
}

extern "C" AbckitInst *IcreateDYNAMICGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(md, nullptr);

    LIBABCKIT_WRONG_CTX(graph->file, md->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynGetmodulenamespaceStatic(graph, md);
}

extern "C" AbckitInst *IcreateDYNAMICWideGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(md, nullptr);

    LIBABCKIT_WRONG_CTX(graph->file, md->file, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynWideGetmodulenamespaceStatic(graph, md);
}

extern "C" AbckitInst *IcreateDYNAMICStmodulevar(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(ed, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynStmodulevarStatic(graph, acc, ed);
}

extern "C" AbckitInst *IcreateDYNAMICWideStmodulevar(AbckitGraph *graph, AbckitInst *acc,
                                                     AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(ed, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynWideStmodulevarStatic(graph, acc, ed);
}

extern "C" AbckitInst *IcreateDYNAMICTryldglobalbyname(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynTryldglobalbynameStatic(graph, string);
}

extern "C" AbckitInst *IcreateDYNAMICTrystglobalbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynTrystglobalbynameStatic(graph, acc, string);
}

extern "C" AbckitInst *IcreateDYNAMICLdglobalvar(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdglobalvarStatic(graph, string);
}

extern "C" AbckitInst *IcreateDYNAMICStglobalvar(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynStglobalvarStatic(graph, acc, string);
}

extern "C" AbckitInst *IcreateDYNAMICLdobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynLdobjbynameStatic(graph, acc, string);
}

extern "C" AbckitInst *IcreateDYNAMICStobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
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
    return IcreateDynStobjbynameStatic(graph, acc, string, input0);
}

extern "C" AbckitInst *IcreateDYNAMICStownbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
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
    return IcreateDynStownbynameStatic(graph, acc, string, input0);
}

extern "C" AbckitInst *IcreateDYNAMICLdsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynLdsuperbynameStatic(graph, acc, string);
}

extern "C" AbckitInst *IcreateDYNAMICStsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
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
    return IcreateDynStsuperbynameStatic(graph, acc, string, input0);
}

extern "C" AbckitInst *IcreateDYNAMICLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(ed, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdlocalmodulevarStatic(graph, ed);
}

extern "C" AbckitInst *IcreateDYNAMICWideLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(ed, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynWideLdlocalmodulevarStatic(graph, ed);
}

extern "C" AbckitInst *IcreateDYNAMICLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(id, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdexternalmodulevarStatic(graph, id);
}

extern "C" AbckitInst *IcreateDYNAMICWideLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(id, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynWideLdexternalmodulevarStatic(graph, id);
}

extern "C" AbckitInst *IcreateDYNAMICStownbyvaluewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                             AbckitInst *input1)
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
    return IcreateDynStownbyvaluewithnamesetStatic(graph, acc, input0, input1);
}

extern "C" AbckitInst *IcreateDYNAMICStownbynamewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
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
    return IcreateDynStownbynamewithnamesetStatic(graph, acc, string, input0);
}

extern "C" AbckitInst *IcreateDYNAMICLdbigint(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdbigintStatic(graph, string);
}

extern "C" AbckitInst *IcreateDYNAMICLdthisbyname(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynLdthisbynameStatic(graph, string);
}

extern "C" AbckitInst *IcreateDYNAMICStthisbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);
    LIBABCKIT_BAD_ARGUMENT(string, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynStthisbynameStatic(graph, acc, string);
}

extern "C" AbckitInst *IcreateDYNAMICLdthisbyvalue(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynLdthisbyvalueStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICStthisbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynStthisbyvalueStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICWideLdpatchvar(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynWideLdpatchvarStatic(graph, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICWideStpatchvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynWideStpatchvarStatic(graph, acc, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICDynamicimport(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynDynamicimportStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICAsyncgeneratorreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
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
    return IcreateDynAsyncgeneratorrejectStatic(graph, acc, input0);
}

extern "C" AbckitInst *IcreateDYNAMICSetgeneratorstate(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynSetgeneratorstateStatic(graph, acc, imm0);
}

extern "C" AbckitInst *IcreateDYNAMICReturn(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(acc, nullptr);

    LIBABCKIT_WRONG_CTX(graph, acc->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynReturnStatic(graph, acc);
}

extern "C" AbckitInst *IcreateDYNAMICReturnundefined(AbckitGraph *graph)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);

    return IcreateDynReturnundefinedStatic(graph);
}

extern "C" AbckitInst *IcreateDYNAMICIf(AbckitGraph *graph, AbckitInst *input, AbckitIsaApiDynamicConditionCode cc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(graph, nullptr);
    LIBABCKIT_BAD_ARGUMENT(input, nullptr);
    if (cc == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    LIBABCKIT_WRONG_CTX(graph, input->graph, nullptr);
    LIBABCKIT_WRONG_MODE(graph, Mode::DYNAMIC, nullptr);
    return IcreateDynIfStatic(graph, input, cc);
}

extern "C" AbckitIsaApiDynamicConditionCode IgetDYNAMICConditionCode(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE);

    if (IgetDYNAMICOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_IF) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        LIBABCKIT_LOG(DEBUG) << "Trying to get condition code not from 'If' instruction\n";
        return ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE;
    }

    return IgetConditionCodeDynamicStatic(inst);
}

extern "C" void IsetDYNAMICConditionCode(AbckitInst *inst, AbckitIsaApiDynamicConditionCode cc)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    if (cc == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NONE) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    if (IgetDYNAMICOpcode(inst) != ABCKIT_ISA_API_DYNAMIC_OPCODE_IF) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        LIBABCKIT_LOG(DEBUG) << "Trying to get condition code not from 'If' instruction\n";
        return;
    }

    bool ccDynamicResitiction =
        !((cc == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE) || (cc == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ));
    if (IsDynamic(inst->graph->function->owningModule->target) && ccDynamicResitiction) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        LIBABCKIT_LOG(DEBUG) << "Wrong condition code set for dynamic if\n";
        return;
    }

    IsetConditionCodeDynamicStatic(inst, cc);
}

extern "C" AbckitCoreImportDescriptor *IgetImportDescriptor(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetImportDescriptorStatic(inst);
}

extern "C" void IsetImportDescriptor(AbckitInst *inst, AbckitCoreImportDescriptor *id)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(id);

    IsetImportDescriptorStatic(inst, id);
}

extern "C" AbckitCoreExportDescriptor *IgetExportDescriptor(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetExportDescriptorStatic(inst);
}

extern "C" void IsetExportDescriptor(AbckitInst *inst, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(ed);

    IsetExportDescriptorStatic(inst, ed);
}

extern "C" AbckitCoreModule *IgetModule(AbckitInst *inst)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT(inst, nullptr);

    return IgetModuleStatic(inst);
}

extern "C" void IsetModule(AbckitInst *inst, AbckitCoreModule *md)
{
    LIBABCKIT_CLEAR_LAST_ERROR;
    LIBABCKIT_IMPLEMENTED;
    LIBABCKIT_TIME_EXEC;

    LIBABCKIT_BAD_ARGUMENT_VOID(inst);
    LIBABCKIT_BAD_ARGUMENT_VOID(md);

    LIBABCKIT_WRONG_CTX_VOID(inst->graph->file, md->file);
    IsetModuleStatic(inst, md);
}

}  // namespace libabckit
