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

let module = ESValue.load('js_code')
let classCar = module.getProperty('Car');
let foo = module.getProperty('foo');
let classA = module.getProperty('A');

function test_newcall(): boolean {
    let color = "Cyan";
    let car = classCar.instantiate(color);
    let varibletest=car.getProperty('color').toString();
    return true;
}

function test_dyncall(): int {
    let a = classA.instantiate();
    let fake_res = foo.invoke(314);
    let b = a.getProperty('getB').invoke();
    let c = b.getProperty('propVar');
    return c.getProperty('foo').invoke(5).toNumber().toInt();
}

function test_dyncall_by_value(): int {
    let a = classA.instantiate();
    return foo.invoke(1).toNumber().toInt();
}
