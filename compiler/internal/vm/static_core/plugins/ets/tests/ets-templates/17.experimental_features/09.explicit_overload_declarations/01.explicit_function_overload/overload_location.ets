/*---
Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

{% for c in cases %}
/*---
desc: All overloaded functions must be in the same module or namespace scope. Otherwise, a compile-time error occurs.
tags: [{{c.tags}}]
---*/

import {foo1} from "./module.ets"

function foo2(a: int): int {
    return 2;
}

namespace N {
    export function fooN(a: long): int {
        return 3;
    }

    namespace M {
        export function fooM(a: number): int {
            return 4;
        }
    }

    overload goo { {{c.nlist}} };
}

overload foo { {{c.list}} };

function main(): void {
    foo(1);
}

{% endfor %}
