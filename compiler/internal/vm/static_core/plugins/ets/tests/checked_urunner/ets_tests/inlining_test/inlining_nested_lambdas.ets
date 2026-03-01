/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//! CHECKER      Check inlining of nested lambda calls
//! RUN          force_jit: true, options: "--compiler-regex='.*test.*'", entry: "inlining_nested_lambdas.ETSGLOBAL::main"
//! EVENT        /Inline,inlining_nested_lambdas.ETSGLOBAL::test,inlining_nested_lambdas.%%lambda-lambda_invoke-0::<ctor>,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_nested_lambdas.ETSGLOBAL::test,inlining_nested_lambdas.%%lambda-lambda_invoke-1::<ctor>,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_nested_lambdas.ETSGLOBAL::test,inlining_nested_lambdas.%%lambda-lambda_invoke-1::invoke1,.*,VIRTUAL,SUCCESS/
//! METHOD       "inlining_nested_lambdas.ETSGLOBAL::test"
//! PASS_AFTER   "Inline"
//! INST         /CallResolvedVirtual.Inlined .* inlining_nested_lambdas.%%lambda-lambda_invoke-1::invoke1/
//! INST_NEXT    /CallResolvedVirtual .* std.core.Function1::invoke1/

// NOTE(compiler): lambda_invoke-0::invoke1 can be also inlined (now only `F2` is inlined into `test`)

//! CHECKER      Check inlining of nested lambda calls (AOT)
//! SKIP_IF      @architecture == "arm32"
//! RUN_PAOC     options: "--compiler-regex='.*test.*'"
//! EVENT        /Inline,inlining_nested_lambdas.ETSGLOBAL::test,inlining_nested_lambdas.%%lambda-lambda_invoke-0::<ctor>,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_nested_lambdas.ETSGLOBAL::test,inlining_nested_lambdas.%%lambda-lambda_invoke-1::<ctor>,.*,STATIC,SUCCESS/
//! EVENT_NEXT   /Inline,inlining_nested_lambdas.ETSGLOBAL::test,inlining_nested_lambdas.%%lambda-lambda_invoke-1::invoke1,.*,VIRTUAL,SUCCESS/
//! METHOD       "inlining_nested_lambdas.ETSGLOBAL::test"
//! PASS_AFTER   "Inline"
//! INST         /CallResolvedVirtual.Inlined .* inlining_nested_lambdas.%%lambda-lambda_invoke-1::invoke1/
//! INST_NEXT    /CallResolvedVirtual .* std.core.Function1::invoke1/
//! RUN          options: "", entry: "inlining_nested_lambdas.ETSGLOBAL::main"

function test(y: int): int {
    let F1 = (x: int) => { return x + 1 }
    let F2 = (x: int) => { return F1(x) + 10 }
    return F2(y);
}

function main(): void
{
    arktest.assertEQ(test(3), 14)
}
