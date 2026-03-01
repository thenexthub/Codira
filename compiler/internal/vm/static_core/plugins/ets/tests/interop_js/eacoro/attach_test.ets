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

class TestClass {

    static callJsBuiltinTest(): int {
        let global = JSRuntime.getGlobal();
        if (!global){
            console.log("No global");
            return 1;
        }

        try {
            let f1Lambda = (x: JSValue) => { JSRuntime.setPropertyInt(JSRuntime.getGlobal(), 'value1', 1);};
            let f2Lambda = (x: JSValue) => { JSRuntime.setPropertyInt(JSRuntime.getGlobal(), 'value2', 2);};
            let f1: JSValue = JSRuntime.newJSValueObject(f1Lambda);
            let f2: JSValue = JSRuntime.newJSValueObject(f2Lambda);
            
            let promiseProp = JSRuntime.getPropertyObject(JSRuntime.getGlobal(), 'Promise')
            let methodResolve = JSRuntime.getPropertyObject(promiseProp, 'resolve');
            let resolveRes = JSRuntime.invoke(promiseProp, methodResolve);
            let methodThen = JSRuntime.getPropertyObject(resolveRes, 'then');
            JSRuntime.invoke(resolveRes, methodThen, f1);
            JSRuntime.invoke(resolveRes, methodThen, f2);
        }
        catch (e) {
            console.log("Builtin function failed");
            return 1;
        }


        try {
            arktest.assertEQ(JSRuntime.getPropertyInt(JSRuntime.getGlobal(), 'value1'), 1);
            arktest.assertEQ(JSRuntime.getPropertyInt(JSRuntime.getGlobal(), 'value2'), 2);
        }
        catch(e) {
            console.log("Failed to modify values");
            return 1;
        }

        return 0;
    }

    static loadJsModuleTest(): int {
        let module = JSRuntime.loadModule("./attach_thread_test_helpers_js");
        if (module == undefined) {
            console.log("Failed to load module");
            return 1;
        }

        return 0;
    }

    static callJsFunctionTest(): int {
        let module = JSRuntime.loadModule("./attach_thread_test_helpers_js");
        let method = JSRuntime.getPropertyJSValue(module, 'returnTwo');
        let val = JSRuntime.invoke(module, method);
        try {
            arktest.assertEQ(val, 2);
        }
        catch(e) {
            console.log("callJsFunctionTest failed");
            return 1;
        }

        return 0;
    }    

    static callJsAsyncFunctionTest(): int {
        return TestClass.callJsAsyncFunction('simpleAsync');
    }

    static callJsAsyncFunctionWithAwaitTest(): int {
        return TestClass.callJsAsyncFunction('asyncWithAwait');
    }

    private static callJsAsyncFunction(funcName: string): int {
        let module = JSRuntime.loadModule("./attach_thread_test_helpers_js");

        let modified = false;

        let method = JSRuntime.getPropertyJSValue(module, funcName);
        let callRes = JSRuntime.invoke(module, method);
        let linker = Class.ofCaller()!.getLinker();
        let clsName = 'std.core.Promise';
        let p = JSRuntime.getValueObject(callRes, linker.loadClass(clsName, false)) as Promise<Object>;
        let p1 = p.then((x:Object): void => {
            modified = true;
        });

        waitForCompletion(() => p1);

        try {
            let value = JSRuntime.getPropertyInt(JSRuntime.getGlobal(), 'value');
            arktest.assertEQ(value, 7);
            arktest.assertEQ(modified, true);
        }
        catch(e) {
            console.log("Value modifications failed");
            return 1;
        }
        return 0;
    }
}