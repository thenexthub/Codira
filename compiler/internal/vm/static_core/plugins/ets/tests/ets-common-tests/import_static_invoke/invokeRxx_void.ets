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

/*---
desc: invokeR0 failed
tags: []
---*/

class TestObj {
        testFunction(callback:(...args: Array<Object | string | number | undefined | null>) => Any) {
                callback('string', 123);
        }
}

let arg = (...args: Array<Object | string | number | undefined | null>) => { return ('HelloWorld') }

const methodArgs: FixedArray<Object> = new FixedArray<Object>(1);

methodArgs[0] = arg;

let obj = new TestObj();

let clsVal = Type.of(obj) as ClassType;

let method = clsVal.getMethodByName('testFunction');

method.invoke(obj, methodArgs);