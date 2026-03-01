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
desc: Template literals are of type string, which is a predefined reference type
---*/

function main() {
  // newlines only
  let t1 = `


`
  arktest.assertTrue( t1 instanceof string && t1 == "\n\n\n" )

  // empty string
  let t2 = ``
  arktest.assertTrue( t2 instanceof string && t2 == "" )

  // escape back-tick
  let t3 = `\``
  arktest.assertTrue( t3 instanceof string && t3 == "`" )

  // escape dollar sign
  let t4 = `\${2*2}`
  arktest.assertTrue( t4 instanceof string && t4 == "${2*2}" )

  // escape opening curly brace
  let t5 = `$\{2*2}`
  arktest.assertTrue( t5 instanceof string && t5 == "${2*2}" )

  // escape back-slash
  let t6 = `test \\node`
  arktest.assertTrue( t6 instanceof string && t6 == "test \\node" )
}
