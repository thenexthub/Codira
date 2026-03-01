/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
  files: [declare.ets]
---*/

import { I, A } from './declare.d.ets'

class B extends A<number>{
    foo(num: number): number {
        num = num + 1;
        return num;
    }
}

function bar1(obj: I<number>):number {
    return obj.foo(1);
}

function bar2(obj: I<number>):number {
    return obj.foo(1, 2);
}

function main() {
    let b = new B()
    arktest.assertEQ(bar1(b), 2);
    arktest.assertEQ(bar2(b), 2);
}
