/*---
Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
desc: Safe method call instance method of a class
assert: A method call with ‘?.’ in objectReference is called a safe method call because it handles null safely.
---*/

class A {
    public foo(x: int): Int {
        return x * 2;
    }
}

function main(): void {
    let instance: A|null = (): A|null => { return null; }();
    arktest.assertTrue( instance?.foo(3) === undefined )
}
