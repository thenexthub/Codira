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
desc: Safe access to field
assert: A field access in which objectReference contains ‘?.’ is called safe field access because it handles null safely.
---*/
final class B {
    public x: Int = 12345;
}

final class A {
  public bin: TNullableB = null

  constructor(initB: boolean = false) {
    if(!initB) return
    this.bin = new B
  }
}

type TNullableB = B | null
type TNullableA = A | null

function main() {
    let testX = new A;
    arktest.assertEQ(undefined, ((value: A|null): Int | undefined => { return value?.bin?.x })(testX))
    let testY = new A(false)
    arktest.assertEQ(undefined, ((value: A|null): Int | undefined => { return value?.bin?.x })(testY))
    let testZ = new A(true)
    arktest.assertEQ(12345, ((value: A|null): Int | undefined => { return value?.bin?.x })(testZ))
}
