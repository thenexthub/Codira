/*
 * Copyright (c)2025 Huawei Device Co., Ltd.
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

class A {
    public bar(v: int) {
        return v
    }

    public lambda = () => 1

    public static staticBar() {}
}

interface B {
    buzz()
}

class C implements B {
    buzz() {}
}


function foo(v: int) {
    return v
};

function returnFunctionName(a: Function): string {
    return a.name
}

function main() {
    let x = () => {}
    let a: A = new A()
    let b: B = new C()
    let f = true;
    let def = f ? () => { } : null

    arktest.assertEQ(foo.name, 'foo')
    arktest.assertEQ(x.name, 'x')
    arktest.assertEQ(a.bar.name, 'bar')
    arktest.assertEQ(a.lambda.name, 'lambda')
    arktest.assertEQ(A.staticBar.name, 'staticBar')
    arktest.assertEQ(b.buzz.name, 'buzz')
    // NOTE(dkofanov): #27259. Decide whether such case need to be supported (TS doesn't support).
    // arktest.assertEQ(def?.name, 'def')

    arktest.assertEQ(returnFunctionName(foo), 'foo')
    arktest.assertEQ(returnFunctionName(x), 'x')
    arktest.assertEQ(returnFunctionName(a.bar), 'bar')
    arktest.assertEQ(returnFunctionName(a.lambda), 'lambda')
    arktest.assertEQ(returnFunctionName(A.staticBar), 'staticBar')
    arktest.assertEQ(returnFunctionName(b.buzz), 'buzz')
    arktest.assertEQ(returnFunctionName(() => 1), '')
    if (def) {
        // NOTE(dkofanov): #27259. Decide whether such case need to be supported (TS doesn't support).
        // arktest.assertEQ(returnFunctionName(def), 'def')
    }
}
