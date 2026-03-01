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
let testImportErrorInstance = etsVm.getFunction(
    'Ltest_import_dynamic_error_static/ETSGLOBAL;', 'testImportErrorInstance'
);
let testImportCustomError = etsVm.getFunction(
    'Ltest_import_dynamic_error_static/ETSGLOBAL;', 'testImportCustomError'
);
let testImportCustomErrorInstance = etsVm.getFunction(
    'Ltest_import_dynamic_error_static/ETSGLOBAL;', 'testImportCustomErrorInstance'
);
let testPassCustomErrorAsArgument = etsVm.getFunction(
    'Ltest_import_dynamic_error_static/ETSGLOBAL;', 'testPassCustomErrorAsArgument'
);

export let errorInstance = new Error("errorInstance");

export class CustomError extends Error {
    CustomMessage: string
    constructor(message: string) {
        super(message);
        this.CustomMessage = message;
    }
}

export let customErrorInstance = new CustomError("customErrorInstance");

function main() {
    testImportErrorInstance();

    testImportCustomError();
    testImportCustomErrorInstance();
    testPassCustomErrorAsArgument(new CustomError("customErrorArgument"));
}

main();
