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
desc: Infer type Floating-point literals 
---*/

let floatLiteral = 3.14f
let doubleLiteral = 3.14

arktest.assertTrue(floatLiteral instanceof float, "float literal not infer as float type")
arktest.assertFalse(floatLiteral instanceof double, "float literal infer as double type")

arktest.assertTrue(doubleLiteral instanceof double, "double literal not infer as double type")
arktest.assertFalse(doubleLiteral instanceof float, "double literal infer as float type")
