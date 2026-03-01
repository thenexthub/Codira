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
desc: Overload Ambient Function declarations
tags: [compile-only, not-a-test]
---*/

export function fun1(x: int): string { return x.toString(); }
export function fun2(x: string): string { return x; }
export overload fun {fun1, fun2}

export namespace N {
    export function foo1(x: number, y: string, z: Object[]): double {
        return (x * y.length * z.length).toDouble();
    }
    export function foo2(x: number, y: string): double {
        return (x * y.length).toDouble();
    }
    export overload foo {foo1, foo2}
}

export function bar1<U, V, R>(x: U, y: V): R { return JSON.stringify([x, y]) as R; }
export function bar2<U, R>(x: U): R { return JSON.stringify(x) as R; }
export overload bar {bar1, bar2}

export function baz1(y: string): string { return y + '1'; }
export function baz2(y: string, x?: int): string { return x ? x.toString() : y; }
export overload baz {baz1, baz2}
