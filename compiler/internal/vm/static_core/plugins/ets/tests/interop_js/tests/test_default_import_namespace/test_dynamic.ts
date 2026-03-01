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
let testAll = etsVm.getFunction('Ltest_static/ETSGLOBAL;', 'testAll');

namespace ns {
    export interface I {
        i: string
        foo(s:string):string 
    }
    export class A implements I{
        i: string = "1234"
        foo(s:string):string {
            return s
        }
    }

    export function bar(n:number):number {
        return n
    }

    export const a: A = new A()
}

export default ns;

function main() {
    testAll();
}

main();
