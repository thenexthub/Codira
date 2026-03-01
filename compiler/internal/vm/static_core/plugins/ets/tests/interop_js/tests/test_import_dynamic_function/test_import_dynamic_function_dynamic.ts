/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;
let testImportFunction = etsVm.getFunction(
    'Ltest_import_dynamic_function_static/ETSGLOBAL;', 'testImportFunction'
);
let testFunctionAsArgument = etsVm.getFunction(
    'Ltest_import_dynamic_function_static/ETSGLOBAL;', 'testFunctionAsArgument'
);

export function returnMagic(): number {
    return 0x55aa;
}

export let returnMagicLambda = (): number => {
    return 0x55aa;
}

export function addTwo(a: number, b: number): number {
    return a + b;
}

export function addTwoLambda(a: number, b: number): number {
    return a + b;
}

export function addDefault(a: number, b: number = 0x7c00): number {
    return a + b;
}

export function addDefaultLambda(a: number, b: number = 0x7c00): number {
    return a + b;
}

export function addOptional(a: number, b?: number): number {
    if (b === undefined) {
        b = 0x7c00;
    }
    return a + b;
}

export function addOptionalLambda(a: number, b?: number): number {
    if (b === undefined) {
        b = 0x7c00;
    }
    return a + b;
}

export function addRest(...args: number[]): number {
    return args.reduce((acc, curr) => acc + curr, 0);
}

export function addRestLambda(...args: number[]): number {
    return args.reduce((acc, curr) => acc + curr, 0);
}

function main() {
    testImportFunction();
    testFunctionAsArgument(returnMagic);
}

main();
