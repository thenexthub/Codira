/*---
Copyright (c) 2025 Huawei Device Co., Ltd.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
---*/

/*---
desc: module scope to export
tags: [not-a-test, compile-only]
---*/

export let x = 1;

const y = 2;

export function foo(): int {
    let y = x
    return y - 1;
}

function bar(): void {
    let b = x;
};

export class C {
    y: int = x + 1;
}

class Z {
    z: int;
}

export interface I {
    zzz(): int {
        let y = x
        return y * 3;
    }
}

export class D implements I {
    f: int = 4
}

interface J {
    zz(): int {
        let j = x
        return j * 3;
    }
}

class F implements J {
    f: int;
}

