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

let module = ESValue.load('dynamic_function_exception')
let dynamicFunctionWithUserException = module.getProperty('dynamicFunctionWithUserException');
let dynamicFunctionWithCustomObject = module.getProperty('dynamicFunctionWithCustomObject');
let dynamicFunctionWithBuiltinException = module.getProperty('dynamicFunctionWithBuiltinException');

// 1. Dynamic function throws user-defined exception E that extends Error (catch clause with E)

// Test 1.1 Catch exception and check Error
function CheckUserExceptionError(): boolean {
    try {
        dynamicFunctionWithUserException.invoke()
        return false
    } catch(e) {
        return e instanceof Error
    }

}

// Test 1.2 Check exception name
function CheckNameUserExceptionError(): boolean {
    try {
        dynamicFunctionWithUserException.invoke()
        return false
    } catch(e) {
         return (e as Error).name == 'CustomError1'

    }
}

// Test 1.3 Check exception message
function CheckMessageUserExceptionError(): boolean {
    try {
        dynamicFunctionWithUserException.invoke()
        return false
    } catch(e) {
         return (e as Error).message == 'This is a custom error'

    }
}

// 2 Dynamic function throws object that isn't extend Error

//Test 2.1 Catch exception and check Error
function CatchUserException(): boolean {
    try {
        dynamicFunctionWithCustomObject.invoke()
        return false

    } catch(e) {
        return e instanceof ESError && (e as ESError).name ==='UnknownError' &&
               e.message == 'This is a custom object error'
    }

}

//3 Dynamic function throws builtin exception E

//Test 3.1 Catch exception and check Error
function CatchBuiltinException(): boolean {
    try {
        dynamicFunctionWithBuiltinException.invoke()
        return false

    } catch(e) {
        return e instanceof TypeError
    }

}

// Test 3.2 Check exception name
function CheckNameBuiltinException(): boolean {
    try {
        dynamicFunctionWithBuiltinException.invoke()
        return false
    } catch(e) {
        return e.name ==='TypeError'
    }
}

// Test 3.3 Check exception message
function CheckMessageBuiltinException(): boolean {
    try {
        dynamicFunctionWithBuiltinException.invoke()
        return false
    } catch(e) {
        return e.message == 'This is a TypeError'
    }
}
