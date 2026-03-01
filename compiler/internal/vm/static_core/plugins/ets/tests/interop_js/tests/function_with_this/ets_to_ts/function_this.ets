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

class Base {
    baseVal: number = 1
    getCurrent() {
        return this
    }
}

class Child extends Base {
    childVal: number = 2
    getSuper() {
        return super.getCurrent()
    }
}

interface FooInterface {
    foo: Function
}

let baseFunc = new Base().getCurrent
let childFunc = new Child().getSuper
let fooBaseObj: FooInterface = {
    foo: baseFunc
}
let fooChildObj: FooInterface = {
    foo: childFunc
}

function etsDoCallbackFunction(func: Function) {
    return func.unsafeCall()
}

function etsDoCallbackLambda(func: () => object) {
    return func()
}