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

let module = ESValue.load('main_js');
let Foo = module.getProperty('Foo');
let createFooClassFromTs = module.getProperty('createFooClassFromTs');

function EtsGlobalInitPlaceholder(): void {}

function Bar(): object {
    return createFooClassFromTs.invoke(JSRuntime.newJSValueInt(42));
}

function checkInteropNegative(): boolean {
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    EtsGlobalInitPlaceholder();
    try {
        launch<Any, () => Any>(Bar).Await();
    } catch (e) {
        if (e instanceof NoInteropContextError) {
            return true;
        }
    }
    return false;
}

function checkInteropPositive(): boolean {
    EtsGlobalInitPlaceholder();
    try {
        Bar();
    } catch (e) {
        return false;
    }
    return true;
}
