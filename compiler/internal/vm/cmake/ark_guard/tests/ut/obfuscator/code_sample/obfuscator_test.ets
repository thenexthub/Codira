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
let field1 = 'hello';

function foo1() {
}

namespace ns1 {
    let nsField1 = 'hello';

    function nsFoo1() {
    }
}

class cl1 {
    clField1 = 'hello';

    cl1Foo1() {
    }

}

@interface Anno1 {
    element1: int;
}

@interface Anno2 {}

type type1 = long;
let field2: type1 = 1;

type type2 = float;
let field3: type2 = 1;

export interface BaseInterface {
    id: number
}

declare interface Interface1 extends BaseInterface {
    name: string
}

export class BaseClass {}

export abstract class Class01 extends BaseClass {}