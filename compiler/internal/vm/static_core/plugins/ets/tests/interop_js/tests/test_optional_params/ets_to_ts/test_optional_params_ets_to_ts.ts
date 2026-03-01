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

const etsVm = globalThis.gtest.etsVm;

let foo1 = etsVm.getFunction('Loptional_params/ETSGLOBAL;', 'foo1');
let foo2 = etsVm.getFunction('Loptional_params/ETSGLOBAL;', 'foo2');
let foo3 = etsVm.getFunction('Loptional_params/ETSGLOBAL;', 'foo3');
let fun1 = etsVm.getFunction('Loptional_params/ETSGLOBAL;', 'fun1');
let fun2 = etsVm.getFunction('Loptional_params/ETSGLOBAL;', 'fun2');
let fun3 = etsVm.getFunction('Loptional_params/ETSGLOBAL;', 'fun3');

function unreachable(): void {
    ASSERT_TRUE(false);
}

function main(): void {
    ASSERT_TRUE(foo1(1, 'str'));
    ASSERT_TRUE(foo2(1, 'str', true));
    ASSERT_TRUE(foo3(1, 'str', true, 2));

    ASSERT_TRUE(fun1(1, 'str'));
    ASSERT_TRUE(fun2(1, 'str', false));
    ASSERT_TRUE(fun3(1, 'str', false, 2));

    // test if the non-optional non-default parameter is lost
    // for function
    try {
        foo1();
        unreachable();
    } catch (e) {
        ASSERT_TRUE(e.message.includes('wrong argc'));
    }
}

main();
