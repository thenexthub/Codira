/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

let etsVm = globalThis.gtest.etsVm;
let testAll = etsVm.getFunction('Ltest_xref_object_static/ETSGLOBAL;', 'testAll');

export class Foo {
    constructor() {
        this.barProperty = 0x55aa;
    }

    barProperty: number;
    arrayLikeProperty: Object = {
        0: 0,
        1: 1,
        2: 2,
        3: 3,
        4: 4,
    };

    getBar(): number {
        return this.barProperty;
    }

    static getMagic(): number {
        return 0x55aa;
    }
}

export let fooInstance = new Foo();

export class FooError extends Error {
    constructor() {
        super("FooError.message");
        this.barProperty = 0x55aa;
    }

    barProperty: number;
    arrayProperty: Array<number> = [0, 1, 2, 3, 4];
}

export let fooErrorInstance = new FooError();


function main() {
    testAll();
}

main();
