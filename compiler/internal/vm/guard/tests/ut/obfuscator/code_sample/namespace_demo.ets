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

namespace Foo {
    class Foo {
        name: string = "Foo";
    }

    export function foo(name: string) {
        return 'Foo' + name;
    }

    let var1 = 1;

    namespace Inner {
        let var2 = 2;
        export function foo(): string {
            let var3 = new Foo();
            return 'inner' + var2 + var3.name;
        }
    }

    export namespace External {
        export function foo(): string {
            return `${Inner.foo()}|${var1}|count ${Bar.getNowCount()}|${Bar.foo('External')}`;
        }
    }
}

namespace Bar {
    let count = 0;
    const base: int;

    static {
        count++;
        base = 12;
    }

    export function getNowCount(): int {
        let ret = count + base;
        count++;
        return ret;
    }

    export function foo(name: string): string {
        return 'Bar' + Foo.foo(name);
    }
}

namespace ns {
    export class A {
        foo(): string {
            return 'foo';
        }
    }
}

console.log('foo: ' + Foo.External.foo());
console.log('count: ' + Bar.getNowCount());
console.log(new ns.A().foo());

