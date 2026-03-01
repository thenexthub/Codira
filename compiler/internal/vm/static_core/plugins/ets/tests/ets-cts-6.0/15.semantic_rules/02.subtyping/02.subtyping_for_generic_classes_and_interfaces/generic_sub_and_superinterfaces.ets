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
desc: Subtyping relation among the generic interface types with same parameters
---*/


interface C<T extends String, G extends Number> {
    name: T;
}

interface D<T extends String, G extends Number> extends C<T, G> {
    name: 'String2'
    id: G;
}

function main(): void {
    let instanceD: D<String, Number> = { name: 'String2', id: 2.0 }
    let instanceC: C<String, Number> = instanceD;
    arktest.assertTrue(instanceC instanceof D)
    arktest.assertTrue(instanceC.name instanceof String)
    arktest.assertEQ(instanceC.name, 'String2')
}
