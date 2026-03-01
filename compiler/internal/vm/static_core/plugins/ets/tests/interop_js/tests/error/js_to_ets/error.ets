/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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

let module = ESValue.load('error_js')
let throwNum = module.getProperty('throwNum');
let throwStr = module.getProperty('throwStr');
let throwObj = module.getProperty('throwObj');
let throwErrorSubClass = module.getProperty('throwErrorSubClass');
let throwError1 = module.getProperty('throwError1');
let throwError2 = module.getProperty('throwError2');
let throwError3 = module.getProperty('throwError3');
let throwEtsError = module.getProperty('throwEtsError');

function throwError(): Error {
    throw new Error('Error From ETS');
}

function checkEtsError(): boolean {
    try { throwEtsError.invoke(throwError); } catch (e) {
        return e.message === 'Error From ETS';
    }
    return false;
}

function checkNumError(): boolean {
    try { throwNum.invoke(); } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        return errObj.toNumber() === 0;
    }
    return false;
}

function checkStrError(): boolean {
    try { throwStr.invoke(); } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        return errObj.toString() === 'hello';
    }
    return false;
}

function checkObjError(): boolean {
    try { throwObj.invoke(); } catch (e) {
        let errObj: ESValue = (e as ESError).getValue();
        let a: ESValue = errObj.getProperty('a');
        let b: ESValue = errObj.getProperty('b');
        return (a.toNumber() === 1) && (b.toNumber() === 2);
    }
    return false;
}

function checkSubClassError(): boolean {
    let errorCaught: boolean = false;
    try { 
        throwErrorSubClass.invoke(); 
    } catch (e) {
        errorCaught = true;
        arktest.assertEQ(e instanceof Error, true);
    }
    return errorCaught;
}

function checkErrorData(): boolean {
    let isThrowErrorCaught: boolean = false;
    let errorThrown: Error | undefined = undefined;

    try { 
        throwError1.invoke(); 
    } catch (e) {
        isThrowErrorCaught = true;

        arktest.assertEQ(e instanceof Error, true);
        errorThrown = e;
    }

    let isGetMessageErrorCaught: boolean = false;

    try {
        // in static, the error message must be string
        // in the case, the message is an object in dynamic
        // so a runtime error will be thrown when accessing the message
        let msg = errorThrown?.message;
    } catch (e) {
        e = e as Error;
        arktest.assertEQ(e instanceof Error, true);

        isGetMessageErrorCaught = true;
        arktest.assertTrue(e.message.includes(`String expected`));
    }

    return isThrowErrorCaught && isGetMessageErrorCaught;
}

function checkErrorMsg(): boolean {
    try { throwError2.invoke(); } catch (e) {
        return e.message === 'null pointer error';
    }
    return false;
}

function checkErrorCause(): boolean {
    let errorCaught: boolean = false;
    try { 
        throwError3.invoke(); 
    } catch (e) {
        errorCaught = true;

        let cause = e.cause as Error;
        arktest.assertEQ(e.message, 'current error');
        arktest.assertEQ(cause.message, 'original error');
    }
    return errorCaught;
}

// Check Error
function checkError(): boolean {
    let cause: ErrorOptions = {cause: 'Test cause' as Object} as ErrorOptions
    let err = new Error('original error', cause);
    let orgName = err.name;
    let orgMsg = err.message;
    let orgCause = err.cause;
    err.name = 'new error name';
    err.message = 'new error message';
    err.cause = 'new error cause';

    return orgName === 'Error' && orgMsg === 'original error' &&
        orgCause === 'Test cause' && err.cause === 'new error cause' &&
        err.name === 'new error name' && err.message === 'new error message';
}