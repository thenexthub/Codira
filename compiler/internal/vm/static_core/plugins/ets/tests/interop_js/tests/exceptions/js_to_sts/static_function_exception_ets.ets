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

let module1 = ESValue.load('static_function_exception')
let staticFunctionWithCustomException = module1.getProperty('staticFunctionWithCustomException');
let staticFunctionWithBuiltinException = module1.getProperty('staticFunctionWithBuiltinException');
let CustomError = module1.getProperty('CustomError');
let createError = module1.getProperty('createError');

let module2 = ESValue.load('ts_custom_exception')
let CustomErrorTypeA = module2.getProperty('CustomErrorTypeA');
let createCustomErrorA = module2.getProperty('createCustomErrorA');

//1 Static function throws user-defined exception E

//Test 1.1 catch and check
function CheckCustomException(): boolean {
    try {
        staticFunctionWithCustomException.invoke()
        return false
    } catch(e) {
        return e instanceof Error
    }

}

// Test 1.2 Check exception name
function CheckNameCustomException(): boolean {
    try {
        staticFunctionWithCustomException.invoke()
        return false
    } catch(e) {
        return e.name == 'CustomErrorA'

    }
}

// Test 1.3 Check exception message
function CheckMessageCustomException(): boolean {
    try {
        staticFunctionWithCustomException.invoke()
        return false
    } catch(e) {
        return e.message == 'This is a custom error(from static function)'
    }
}

// 2 Static function throws builtin exception

//Test 2.1 catch and check
function CheckBuiltinExceptionInStatic(): boolean {
    try {
        staticFunctionWithBuiltinException.invoke()
        return false
    } catch(e) {
        return e instanceof TypeError
    }

}

// Test 2.2 Check exception name
function CheckNameBuiltinExceptionInStatic(): boolean {
    try {
        staticFunctionWithBuiltinException.invoke()
        return false
    } catch(e) {
        return e.name == 'TypeError'
    }
}

// Test 2.3 Check exception message

function CheckMessageBuiltinExceptionInStatic(): boolean {
    try {
        staticFunctionWithBuiltinException.invoke()
        return false
    } catch(e) {
        return e.message == 'This is a TypeError'
    }
}

// 3 Throw imported custom error

function ThrowCustomError(): boolean {
    try {
        throw CustomError.instantiate('This is a custom error(from static function)').unwrap() as Error;
    } catch (e) {
        return true;
    }
}

// 4 Throw error from imported function

function CheckThrowNewError(): boolean {
    try {
        const myError = createError.invoke('NewError', 'New error message');
        throw myError.unwrap() as Error;
    } catch (e) {
        return true;
    }
}

// 5 Throwing an exception with an imported function and an imported type

function throwTSCustomExceptionWithImportType(): boolean {
    try {
        const myErrorA = createCustomErrorA.invoke('NewError', 500);
        throw myErrorA.unwrap() as Error;
    } catch (e) {
        return true;
    }
}
