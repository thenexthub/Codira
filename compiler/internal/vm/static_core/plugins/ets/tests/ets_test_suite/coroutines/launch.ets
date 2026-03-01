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

class MyPayload {
    public s: string;
    constructor(a: number) {
        this.s = `MyPayload.s=${a}`;
    }
}

const l0 = () => `l0`;
const l1 = (a0: string) => `l1: ${a0}`;
const l2 = (a0: string, a1: number, a2: MyPayload) => `l2: ${a0} ${a1} ${a2.s}`;

function f0() {
    return `f0`;
}

function f1(a0: string) {
    return `f1: ${a0}`;
}

function f2(a0: string, a1: number, a2: MyPayload) {
    return `f2: ${a0} ${a1} ${a2.s}`;
}

class C {
    static s0() {
        return `s0`;
    }

    static s1(a0: string) {
        return `s1: ${a0}`;
    }

    static s2(a0: string, a1: number, a2: MyPayload) {
        return `s2: ${a0} ${a1} ${a2.s}`;
    }
}

/**
 * This test verifies launching lambdas with and without arguments.
 * This function calls launch to execute l0, l1, and l2 functions, and verifies that their return values match expectations.
 */
function testLaunchLambdas(): void {
    // Launch l0 function with no arguments
    const l0Result = launch<string, () => string>(l0).Await();
    
    // Launch l1 function with one string argument 'l1_arg_0'
    const l1Result = launch<string, (a0: string) => string>(l1, 'l1_arg_0').Await();

    // Launch l2 function with three arguments: string, number, and a custom object
    const l2ResultJob: Job<string> = launch<
        string,
        (a0: string, a1: number, a2: MyPayload) => string
    >(l2, 'l2_arg_0', 2.73, new MyPayload(3.14));
    
    const l2Result = l2ResultJob.Await();

    // Assert that the results match the expected values
    arktest.assertEQ(l0Result, 'l0');
    arktest.assertEQ(l1Result, 'l1: l1_arg_0');
    arktest.assertEQ(l2Result, "l2: l2_arg_0 2.73 MyPayload.s=3.14");
}

/**
 * This test checks launching regular functions with and without arguments
 * This function calls launch to execute f0, f1, and f2 functions, and verifies their return values.
 */
function testLaunchFunctions(): void {
    // Launch f0 function with no arguments
    const f0ResultJob: Job<string> = launch<string, () => string>(f0);
    const f0Result = f0ResultJob.Await();

    // Launch f1 function with one string argument 'f1_arg_0'
    const f1ResultJob: Job<string> = launch<string, (a0: string) => string>(f1, 'f1_arg_0');
    const f1Result = f1ResultJob.Await();

    // Launch f2 function with three arguments: string, number, and a custom object
    const f2ResultJob: Job<string> = launch<
        string,
        (a0: string, a1: number, a2: MyPayload) => string
    >(f2, 'f2_arg_0', 2.73, new MyPayload(3.14));
    
    const f2Result = f2ResultJob.Await();

    // Assert that the results match the expected values
    arktest.assertEQ(f0Result, 'f0');
    arktest.assertEQ(f1Result, 'f1: f1_arg_0');
    arktest.assertEQ(f2Result, 'f2: f2_arg_0 2.73 MyPayload.s=3.14');
}

/**
 * Tests the behavior of the launch function when launching static methods.
 * This function calls launch to execute C.s0, C.s1, and C.s2 methods, and verifies their return values.
 */
function testLaunchStatic(): void {
    // Launch C.s0 static method with no arguments
    const s0Result = launch<string, () => string>(C.s0).Await();

    // Launch C.s1 static method with one string argument 'C.s1_arg_0'
    const s1Result = launch<string, (a0: string) => string>(C.s1, 'C.s1_arg_0').Await();

    // Launch C.s2 static method with three arguments: string, number, and a custom object
    const s2ResultJob: Job<string> = launch<
        string,
        (a0: string, a1: number, a2: MyPayload) => string
    >(C.s2, 'C.s2_arg_0', 2.73, new MyPayload(3.14));
    
    const s2Result = s2ResultJob.Await();

    // Assert that the results match the expected values
    arktest.assertEQ(s0Result, 's0');
    arktest.assertEQ(s1Result, 's1: C.s1_arg_0');
    arktest.assertEQ(s2Result, 's2: C.s2_arg_0 2.73 MyPayload.s=3.14');
}


function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.launch');
    testSuite.addTest('testLaunchLambdas', testLaunchLambdas);
    testSuite.addTest('testLaunchFunctions', testLaunchFunctions);
    testSuite.addTest('testLaunchStatic', testLaunchStatic);
    let res = testSuite.run();
    return res;
}
