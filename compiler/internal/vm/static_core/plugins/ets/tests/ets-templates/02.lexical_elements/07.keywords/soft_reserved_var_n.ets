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

/*---
desc: ->
    The following identifiers are also treated as soft keywords reserved for the future use,
    or currently used in TypeScript
expected_out: "Syntax error ESY0297: 'var' keyword is not supported. Use 'let' instead."
tags: [negative, compile-only]
---*/

let var = 0
var = 1
