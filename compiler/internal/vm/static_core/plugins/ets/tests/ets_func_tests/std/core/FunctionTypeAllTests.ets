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

function main(): int {
    const suite = new arktest.ArkTestsuite('FunctionType & LambdaType API tests for SDK (using Type.of)');

    suite.addTest('FunctionType.getName returns empty', testGetNameEmpty);
    suite.addTest('FunctionType.getResultType', testGetResultType);
    suite.addTest('FunctionType.getParametersNum', testGetParametersNum);
    suite.addTest('FunctionType.getParameter types', testGetParameter);
    suite.addTest('LambdaType.getLiteral', testGetLiteral);
    suite.addTest('FunctionType optional parameters', testFunctionOptionalParameters);
    suite.addTest('LambdaType optional parameters', testLambdaOptionalParameters);

    return suite.run();
}

function dummyFunction(a: int, b: string): boolean {
    return true;
}

const dummyLambda = (x: int, y: string): boolean => x > 0;

function testGetNameEmpty() {
    const ft = Type.of(dummyFunction) as FunctionType;
    arktest.assertEQ('', ft.getName());
}

function testGetResultType() {
    const ft = Type.of(dummyFunction) as FunctionType;
    const res = ft.getResultType();
    arktest.assertTrue(res.equals(BooleanType.VAL));
}

function testGetParametersNum() {
    const ft = Type.of(dummyFunction) as FunctionType;
    arktest.assertEQ(2, ft.getParametersNum());
}

function testGetParameter() {
    const ft = Type.of(dummyFunction) as FunctionType;
    const p0 = ft.getParameter(0).getType();
    const p1 = ft.getParameter(1).getType();
    arktest.assertTrue(p0.equals(IntType.VAL));
    arktest.assertTrue(p1.equals(StringType.REF));
}

function testGetLiteral() {
    const lt = Type.of(dummyLambda) as LambdaType;
    arktest.assertEQ('(1: int, 2: string): boolean', lt.getLiteral());
}

function optionalFunc(a: int, b?: string): boolean {
    return true;
}

function testFunctionOptionalParameters() {
    const ft = Type.of(optionalFunc) as FunctionType;
    arktest.assertEQ(2, ft.getParametersNum());

    const p0 = ft.getParameter(0);
    const p1 = ft.getParameter(1);

    arktest.assertTrue(p0.getType().equals(IntType.VAL));
    arktest.assertTrue(p1.getType().equals(StringType.REF));
}

const optionalLambda = (x: int, y?: string): boolean => true;

function testLambdaOptionalParameters() {
    const lt = Type.of(optionalLambda) as LambdaType;
    arktest.assertEQ(2, lt.getParametersNum());

    const p0 = lt.getParameter(0);
    const p1 = lt.getParameter(1);

    arktest.assertTrue(p0.getType().equals(IntType.VAL));
    arktest.assertTrue(p1.getType().equals(StringType.REF));
}
