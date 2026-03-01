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
desc: String literals consist of zero or more characters enclosed between single or double quotes.
params: examples from the specification
---*/

let b = 'Hello, world!'
let c = "Hello, world!"
let d = "\\"
let e = ""
let f = "don't do it"
let g = 'don\'t do it'
let h = 'don\u0027t do it'

function main(): void {
    arktest.assertTrue(b instanceof string)
    arktest.assertTrue(c instanceof string)
    arktest.assertTrue(d instanceof string)
    arktest.assertTrue(e instanceof string)
    arktest.assertTrue(f instanceof string)
    arktest.assertTrue(g instanceof string)
    arktest.assertTrue(h instanceof string)

	arktest.assertEQ(b.length, 13)
	arktest.assertEQ(c.length, 13)
	arktest.assertEQ(d.length, 1)
	arktest.assertEQ(e.length, 0)
	arktest.assertEQ(f.length, 11)
	arktest.assertEQ(g.length, 11)
	arktest.assertEQ(h.length, 11)
}
