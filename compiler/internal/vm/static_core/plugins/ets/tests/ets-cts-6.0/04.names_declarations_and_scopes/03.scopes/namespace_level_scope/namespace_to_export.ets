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
desc: namespaces scope
tags: [not-a-test, compile-only]
---*/

export namespace ExternalSpace {
    let x1 = 1
    export let x2 = x1
    let x3 = x2
    export const y1 = x1 + 1
    export const y2 = y1
    const y3 = y2
    export function foo(): int { 
        return x2 + y2;
    }

    export namespace EmbeddedSpace {
        export let z1 = foo()
        export const z2 = y2
        export function bar(): int {
            return z1 + z2
        }
    }

    namespace EmbeddedSpaceNotExp {
        let z1 = foo()
        const z2 = y2
        function bar(): int {
            return z1 + z2
        }
    }
}
